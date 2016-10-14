/**
 * proxy-velodyne16 - Interface to VLP-16.
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

#include <fstream>
#include <iostream>
#include <memory>

#include "ProxyVelodyne16.h"
#include "opendavinci/odcore/base/KeyValueConfiguration.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"


namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace odcore::wrapper;
using namespace odcore::io::udp;

ProxyVelodyne16::ProxyVelodyne16(const int32_t &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "ProxyVelodyne16")
    , memoryName()
    , memorySize(0)
    , udpReceiverIP()
    , udpPort(0)
    , VelodyneSharedMemory(NULL)
    , udpreceiver(NULL)
    , v16dSp(NULL) {}

ProxyVelodyne16::~ProxyVelodyne16() {}

void ProxyVelodyne16::setUp() {
    memoryName = getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.sharedMemory.name");
    memorySize = getKeyValueConfiguration().getValue< uint32_t >("ProxyVelodyne16.sharedMemory.size");
    VelodyneSharedMemory = SharedMemoryFactory::createSharedMemory(memoryName, memorySize);

    udpReceiverIP = getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.udpReceiverIP");
    udpPort = getKeyValueConfiguration().getValue< uint32_t >("ProxyVelodyne16.udpPort");
    udpreceiver = UDPFactory::createUDPReceiver(udpReceiverIP, udpPort);

    v16dSp = shared_ptr< velodyne16Decoder >(new velodyne16Decoder(VelodyneSharedMemory, getConference(), getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.calibration")));

    udpreceiver->setStringListener(v16dSp.get());
    // Start receiving bytes.
    udpreceiver->start();
}

void ProxyVelodyne16::tearDown() {
    udpreceiver->stop();
    udpreceiver->setStringListener(NULL);
}

// This method will do the main data processing job.
odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyVelodyne16::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    }
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
