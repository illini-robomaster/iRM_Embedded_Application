/**************************************************************************
 *  Copyright (C) 2018 RoboMaster.
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
 * @author  RoboMaster
 * @date    2017-10
 * @file    crc_check.h
 * @brief   CRC check for referee system and tx2
 * @log     2018-04-18 nickelliang
 */

#ifndef _CRC_CHECK_H_
#define _CRC_CHECK_H_

#include "stm32f4xx_hal.h"

/**
 * CRC8 checksum calculation
 *
 * @param  pchMessage Message to check
 * @param  dwLength   Length of the message
 * @param  ucCRC8     Initialized checksum
 * @return            CRC checksum
 */
static uint8_t get_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength, uint8_t ucCRC8);

/**
 * Verify CRC8
 *
 * @param  pchMessage Message to verify
 * @param  dwLength   Length = Data + Checksum
 * @return            1 for true, 0 for false
 */
uint8_t verify_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);

/**
 * Append CRC8 to the end of message
 *
 * @param  pchMessage Message to calculate CRC and append
 * @param  dwLength   Length = Data + Checksum
 */
void append_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);

/**
 * CRC16 checksum calculation
 *
 * @param  pchMessage Message to check
 * @param  dwLength   Length of the message
 * @param  wCRC       Initialized checksum
 * @return            CRC checksum
 */
static uint16_t get_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength, uint16_t wCRC);

/**
 * Verify CRC16
 *
 * @param  pchMessage Message to verify
 * @param  dwLength   Length = Data + Checksum
 * @return            1 for true, 0 for false
 */
uint8_t verify_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);

/**
 * Append CRC16 to the end of message
 *
 * @param  pchMessage Message to calculate CRC and append
 * @param  dwLength   Length = Data + Checksum
 */
void append_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);

#endif
