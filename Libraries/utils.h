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

#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @ingroup library
 * @defgroup utils Utils
 * @{
 */

#define sign(x) (x < 0 ? -1 : (x == 0 ? 0 : 1))

#define min(x, y) (x < y ? x : y)
#define max(x, y) (x > y ? x : y)

/**
 * @brief clip int32_t number into [-lim, lim]
 * @param data pointer to the number to be clipped
 * @param lim  absolute limit range
 * @return clipped value
 */
int32_t abs_limit(int32_t *data, int32_t lim);

/**
 * @brief clip floating point number into [-lim, lim]
 * @param data pointer to the number to be clipped
 * @param lim  absolute limit range
 * @return clipped value
 */
float fabs_limit(float *data, float lim);

/**
 * @brief force an integer to be in range [low_lim high_lim]
 * @param data      pointer to the integer to be clipped
 * @param low_lim   lower limit value
 * @param high_lim  upper limit value
 * @return clipped value
 */
int32_t clip_to_range(int32_t *data, int32_t low_lim, int32_t high_lim);

/**
 * @brief force a float to be in range [low_lim high_lim]
 * @param data      pointer to the float to be clipped
 * @param low_lim   lower limit value
 * @param high_lim  upper limit value
 * @return clipped value
 */
float fclip_to_range(float *data, float low_lim, float high_lim);

/**
 * @brief normalize two variables on cartesian plane
 * @param vx    pointer to the first variable
 * @param vy    pointer to the second variable
 * @return none
 */
void normalize_2d(float *vx, float *vy);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
