/**
 * proxy-relays - Interface to relays.
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

#include <cmath>
#include <stdint.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>

#include "ProxyRelay.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

ProxyRelay::ProxyRelay(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-relay") {}

ProxyRelay::~ProxyRelay() {}

void ProxyRelay::setUp() {}

void ProxyRelay::tearDown() {}

void ProxyRelay::nextContainer(odcore::data::Container &) {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyRelay::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        cout << "[" << getName() << "] Frame sent." << endl;
    }
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
