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
 * @date    2018-05-23
 * @file    bsp_sdio.h
 * @brief   Board support package for SD card.
 * @log     2018-05-23 nickelliang
 */

#ifndef _BSP_SDIO_H_
#define _BSP_SDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

// #include "bsp_driver_sd.h"
// #include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "sdio.h"
// #include "main.h"
#include "sd_diskio.h"

#if 0

void sdio_init(void);
void sdio_queue_init(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
