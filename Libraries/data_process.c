/**************************************************************************
 *  Copyright (C) 2018 
 *  Illini RoboMaster @ University of Illinois at Urbana-Champaign.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

/**
 * @author  Nickel_Liang <nickelliang>
 * @date    2018-04-19
 * @file    data_process.c
 * @brief   Process uart received data in DMA buffer
 * @log     2018-05-26 nickelliang
 */

#include <stdio.h>
#include <string.h>

#include "data_process.h"

data_process_t* data_process_init(UART_HandleTypeDef *huart, osMutexId mutex, uint32_t fifo_size, uint16_t buffer_size, uint8_t sof, dispatcher_func_t dispatcher_func, void *source_struct, osMutexId tx_mutex, packer_func_t packer_func) {
    if ((huart == NULL) || (mutex == NULL) || (buffer_size == 0) || (fifo_size == 0) || (dispatcher_func == NULL) || (source_struct == NULL) || (tx_mutex == NULL) || (packer_func == NULL)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Invalid parameter.");
        return NULL;
    }

    data_process_t *source  = (data_process_t*)pvPortMalloc(sizeof(data_process_t));   /* Initialize a data process instance */
    if (source == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Unable to allocate data process object.");
        return NULL;
    }

    source->huart           = huart;
    source->buff_size       = buffer_size;
    source->read_index      = 0;
    source->source_struct   = source_struct;
    source->dispatcher_func = dispatcher_func;
    source->sof             = sof;
    source->data_len        = 0;
    source->packer_func     = packer_func;

    source->data_fifo       = fifo_s_create(fifo_size, mutex);
    if (source->data_fifo == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Unable to allocate FIFO for data process object.");
        free(source);
        return NULL;
    }

    source->buff[0]         = (uint8_t*)pvPortMalloc(2 * source->buff_size);
    if (source->buff[0] == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Unable to allocate DMA buffer for data process object.");
        fifo_s_destory(source->data_fifo);
        free(source);
        return NULL;
    }
    source->buff[1]         = source->buff[0] + source->buff_size;
#ifdef DEBUG
    BSP_DEBUG;
    printf("source->buff[0] 0x%08x ", source->buff[0]);
    printf("source->buff[1] 0x%08x \r\n", source->buff[1]);
#endif

    source->frame_packet    = (uint8_t*)pvPortMalloc(fifo_size);
    if (source->frame_packet == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Unable to allocate frame packet buffer for data process object.");
        free(source->buff[0]);
        fifo_s_destory(source->data_fifo);
        free(source);
        return NULL;
    }

    source->transmit_fifo   = fifo_s_create(fifo_size, tx_mutex);
    if (source->transmit_fifo == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Unable to allocate transmit FIFO for data process object.");
        free(source->frame_packet);
        free(source->buff[0]);
        fifo_s_destory(source->data_fifo);
        free(source);
        return NULL;
    }

    return source;
}

uint8_t data_process_rx(data_process_t *source) {
    if (!buffer_to_fifo(source)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Buffer to FIFO error.");
        return 0;
    }
    if (!fifo_to_struct(source)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "FIFO to struct error.");
        return 0;
    }
    return 1;
}

uint8_t data_process_tx(data_process_t *source) {
    uint32_t count = fifo_used_count(source->transmit_fifo);
    uint8_t buffer[count];
    if (count != 0) {
        if (fifo_s_gets(source->transmit_fifo, buffer, count) != count) {
            bsp_error_handler(__FUNCTION__, __LINE__, "Failed to get enough FIFO data.");
            return 0;
        }
        uart_tx_blocking(source->huart, buffer, count); // @todo Try DMA TX
    }
    return 1;
}

static uint8_t buffer_to_fifo(data_process_t *source) {
    if (source == NULL) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Invalid parameter.");
        return 0;
    }

    int32_t write_len;                  /* Helper to calculate how many bytes to write */
    uint8_t *buff = source->buff[0];    /* Source data buffer starting addr */
    uint16_t write_index;               /* Helper to store where our new head is */

#ifdef DEBUG
    int32_t total_write_len;
#endif

    /* @todo Following condition should be atomic. Actually whole function should be atomic */
    uint8_t dma_memory_target = dma_current_memory_target(source->huart->hdmarx->Instance);
    uint16_t dma_remain_space = dma_current_data_counter(source->huart->hdmarx->Instance);

    /* Calculate correct write index - where we stop writting */
    if (dma_memory_target == 0)     // Writing in buffer 1
        write_index = source->buff_size - dma_remain_space;
    else                            // Writing in buffer 2
        write_index = source->buff_size * 2 - dma_remain_space;

    /* Unnecessary check @todo delete if never triggered */
    if (write_index > source->buff_size * 2) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Write index exceed limit. Logic error.");
        return 0;
    }

    /* Handle wrap around condition */
    if (write_index < source->read_index) {
        /* Last time we left in buffer 2 and now head is in buffer 1. Wrap around needed. */
#ifdef DEBUG
        total_write_len = source->buff_size * 2 - source->read_index + write_index;
#endif
        /* Read index points to where we left last time, so read till the end of second buffer */
        write_len = source->buff_size * 2 - source->read_index;
        if (write_len < 0) {
            bsp_error_handler(__FUNCTION__, __LINE__, "Write length is negative.");
            return 0;
        }
        if (write_len != fifo_s_puts(source->data_fifo, &buff[source->read_index], write_len)) {
            printf("write index: %d\r\n", write_index);
            printf("memory target: %d\r\n", dma_memory_target);
            bsp_error_handler(__FUNCTION__, __LINE__, "FIFO overflow, need to resize.");
            return 0;
        }
        source->read_index = 0;
    }
    else {
        /* Head haven't wrap around yet. Simple direct write. */
#ifdef DEBUG
        total_write_len = write_index - source->read_index;
#endif
    }

#ifdef DEBUG
        BSP_DEBUG;
        printf("Total write len: %d\r\n", total_write_len);
        printf("Mem target:      %u\r\n", dma_memory_target);
        printf("Mem remain:      %d\r\n", dma_remain_space);
        printf("Write index:     %d\r\n", write_index);
        printf("\r\n");
#endif

    /* General write condition */
    write_len = write_index - source->read_index;
    if (write_len != fifo_s_puts(source->data_fifo, &buff[source->read_index], write_len)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "FIFO overflow, need to resize.");
        return 0;
    }
    source->read_index = write_index;
    return 1;
}

static uint8_t fifo_to_struct(data_process_t *source) {
    uint8_t byte = 0;
    uint8_t func_ret = 0;
    uint8_t flag = 0;

    while (!fifo_is_empty(source->data_fifo)) {
        byte = fifo_s_peek(source->data_fifo, 0); // Peek head
#ifdef DEBUG
        printf("Byte: %02x\r\n", byte);
        printf("Used: %d\r\n", source->data_fifo->used);
        printf("Free: %d\r\n", source->data_fifo->free);
#endif
        if (byte != source->sof) {
            fifo_s_get(source->data_fifo);
            continue;
        }

        if (process_header(source) && process_frame(source)) {
            source->dispatcher_func(source->source_struct, source);
            flag = 1;
        }
    }
#ifdef DEBUG
    BSP_DEBUG;
    printf("End of loop.\r\n");
#endif
    return flag;
}

uint8_t data_to_fifo(uint16_t cmdid, uint8_t *data, uint16_t length, data_process_t *source) {
    uint16_t frame_length = DATA_PROCESS_HEADER_LEN + DATA_PROCESS_CMD_LEN + length + DATA_PROCESS_CRC16_LEN;
    uint8_t buffer[frame_length];
    uint8_t sof             = source->sof;
    uint16_t data_length    = length;
    uint8_t seq             = 0;

    /* Construct data header */
    buffer[0]   = sof;
    memcpy(&buffer[1], (uint8_t*)&data_length, sizeof(uint16_t));
    buffer[3]   = seq;
    append_crc8_check_sum(buffer, DATA_PROCESS_HEADER_LEN);

    /* Construct data frame */
    memcpy(&buffer[DATA_PROCESS_HEADER_LEN], (uint8_t*)&cmdid, DATA_PROCESS_CMD_LEN);
    memcpy(&buffer[DATA_PROCESS_HEADER_LEN + DATA_PROCESS_CMD_LEN], data, data_length);
    append_crc16_check_sum(buffer, frame_length);

    /* Put data into fifo */
    if (fifo_s_puts(source->transmit_fifo, buffer, frame_length) != frame_length) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Failed to put data into FIFO.");
        return 0;
    }

    return 1;
}

static uint8_t process_header(data_process_t *source) {
    if (fifo_s_gets(source->data_fifo, (uint8_t*)(source->frame_packet), DATA_PROCESS_HEADER_LEN) != DATA_PROCESS_HEADER_LEN) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Failed to get enough FIFO data.");
        return 0;
    }
    source->data_len = (uint16_t)((source->frame_packet[2] << 8) | source->frame_packet[1]);
    if (source->data_len > DATA_PROCESS_MAX_DATA_LEN) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Data length exceed maximum.");
        return 0;
    }
    if (!verify_crc8_check_sum(source->frame_packet, DATA_PROCESS_HEADER_LEN)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "CRC8 check failed.");
        return 0;
    }
    return 1;
}

static uint8_t process_frame(data_process_t *source) {
    uint16_t frame_length = DATA_PROCESS_CMD_LEN + source->data_len + DATA_PROCESS_CRC16_LEN;
    if (fifo_s_gets(source->data_fifo, (uint8_t*)(source->frame_packet + DATA_PROCESS_HEADER_LEN), frame_length) != frame_length) {
        bsp_error_handler(__FUNCTION__, __LINE__, "Failed to get enough FIFO data.");
        return 0;
    }
    if (!verify_crc16_check_sum(source->frame_packet, DATA_PROCESS_HEADER_LEN + frame_length)) {
        bsp_error_handler(__FUNCTION__, __LINE__, "CRC16 check failed.");
        return 0;
    }
    return 1;
}
