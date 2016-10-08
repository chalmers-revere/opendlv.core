/**
 * proxy-sick - Interface to Sick.
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

#include "ProxySick.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;

ProxySick::ProxySick(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-sick") {}

ProxySick::~ProxySick() {}

void ProxySick::setUp() {}

void ProxySick::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxySick::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        cout << "Inside the main processing loop." << endl;
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
