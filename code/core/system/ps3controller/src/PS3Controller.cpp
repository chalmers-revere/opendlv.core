/**
 * ps3controller - Using a PS3 controller to accelerate, brake, and steer a vehicle.
 * Copyright (C) 2016 Christian Berger
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

#include <stdint.h>

#include <iostream>

#include "PS3Controller.h"

namespace opendlv {
namespace core {
namespace system {

using namespace std;
using namespace odcore::base;

PS3Controller::PS3Controller(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "ps3controller") {}

PS3Controller::~PS3Controller() {}

void PS3Controller::setUp() {}

void PS3Controller::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode PS3Controller::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        cout << "Inside the main processing loop." << endl;
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
} // opendlv::core::system
