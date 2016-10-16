/**
 * proxy-v2v - Interface to V2V.
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

#include <stdint.h>

#include <iostream>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "ProxyV2V.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;

ProxyV2V::ProxyV2V(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-v2v") {}

ProxyV2V::~ProxyV2V() {}

void ProxyV2V::setUp() {
//    const string NAME = getKeyValueConfiguration().getValue< string >("proxy-camera-axis.name");
//        cerr << "[" << getName() << "] No valid camera type defined." << endl;
}

void ProxyV2V::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyV2V::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    }
//    cout << "[" << getName() << "] Captured " << captureCounter << " frames." << endl;
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
