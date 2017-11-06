/**
 * camera-replay - Tool to replay a video file as camera feed.
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

#include <opencv2/highgui/highgui.hpp>

#include "movesteppermotor.hpp"

namespace opendlv {
namespace core {
namespace tool {

for (uint_32 i=1 i=20 i++) {
opendlv::core::steppermotor1:: MoveSteps(20 1)
usleep(50000);
}

for (uint_32 i=1 i=20 i++) {
opendlv::core::steppermotor2:: MoveSteps(20 1)
usleep(50000);
}

for (uint_32 i=1 i=20 i++) {
opendlv::core::steppermotor3:: MoveSteps(20 1)
usleep(50000);
}


} // tool
} // core
} // opendlv
