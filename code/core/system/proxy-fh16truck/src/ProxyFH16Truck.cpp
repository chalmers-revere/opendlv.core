/**
 * proxy-fh16truck - Interface to FH16 truck.
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

#include "odvdfh16truck/GeneratedHeaders_ODVDFH16Truck.h"
#include "odvdvehicle/GeneratedHeaders_ODVDVehicle.h"
#include "fh16mapping/GeneratedHeaders_fh16mapping.h"

#include "ProxyFH16Truck.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;

ProxyFH16Truck::ProxyFH16Truck(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-fh16truck") {}

ProxyFH16Truck::~ProxyFH16Truck() {}

void ProxyFH16Truck::setUp() {}

void ProxyFH16Truck::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyFH16Truck::body() {
    // TODO: Listen for messages of type opendlv::proxy::ActuationRequest to map them on the appropriate FH16 truck messages.
    opendlv::proxy::ActuationRequest ar;
    (void)ar;

    opendlv::proxy::reverefh16::AccelerationRequest ar2;
    (void)ar2;

    canmapping::opendlv::proxy::reverefh16::AccelerationRequest mar2;
    (void)mar2;

    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        cout << "Inside the main processing loop." << endl;
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
