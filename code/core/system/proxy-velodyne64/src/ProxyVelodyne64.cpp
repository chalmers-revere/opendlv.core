/**
 * proxy-velodyne64 - Interface to Velodyne HDL-64E.
 * Copyright (C) 2016 Hang Yin
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

#include <iostream>
#include <fstream>
#include <memory>

#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"

#include "ProxyVelodyne64.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

    using namespace odcore::wrapper;
    using namespace odcore::io::udp;

    ProxyVelodyne64::ProxyVelodyne64(const int &argc, char **argv)
        : TimeTriggeredConferenceClientModule(argc, argv, "ProxyVelodyne64"),
            VelodyneSharedMemory(SharedMemoryFactory::createSharedMemory(NAME, SIZE)),
            udpreceiver(UDPFactory::createUDPReceiver(RECEIVER, PORT)),
            v64d(VelodyneSharedMemory,getConference()){}

    ProxyVelodyne64::~ProxyVelodyne64() {}

    void ProxyVelodyne64::setUp() {
        udpreceiver->setPacketListener(&v64d);
        // Start receiving bytes.
        udpreceiver->start();
    }

    void ProxyVelodyne64::tearDown() {
        udpreceiver->stop();
        udpreceiver->setStringListener(NULL);
    }

    // This method will do the main data processing job.
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyVelodyne64::body() {
        while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        }
        return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
    }
}
}
}
} // opendlv::core::system::proxy
