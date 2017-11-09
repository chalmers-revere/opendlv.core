/**
 * imu-calibration - Tool to find calibration matrices of IMU sensor
 * Copyright (C) 2016 Chalmers Revere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <ctype.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//include <logger.h>??



#include "movesteppermotor.hpp"

namespace opendlv {
namespace core {
namespace tool {

// find log file
// find IMU ID

for (i=1 20 i++) {
// find steady position
// find sensor data acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z
// calculate for each mean over duration of steady position
// save sensor-data-steady(i)
}

// genetic algorithm to solve cost function with Sum ( acc-g ) == 0
// save calibration matrix M_acc
// genetic algorithm to solve cost function with Sum ( gyr-g ) == 0
// save calibration matrix M_gyr


// find rotation
// find sensor data acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z, mag_x, mag_y, mag_z
// low pass filter data
// derivate mag
for (i=1 20 i++) {
// pick points of mag(t) and mag'(t)
// perform singular value decomposition (SVD) for each point (eq. 3.23)
// save d(k)
}
// calculate mean of d
// save S_mag o_mag (eq. 3.14 - 3.21)


for (i=1 3 i++) {
// find phase-x/y/z
// find sensor data acc_x, acc_y, acc_z, mag_x, mag_y, mag_z
// derivate mag
// solve cost functions (eq. 3.24-3.26)
// save T_mag(i)

// derivate acc
// solve cost functions (eq. 3.24-3.26)
// save T_R_acc(i)

// derivate gyr
// solve cost functions (eq. 3.27-3.30)
// save T_R_gyr(i)
}
// save T_mag, T_acc, T_gyr

// save all Matrices to IMU ID

} // tool
} // core
} // opendlv
