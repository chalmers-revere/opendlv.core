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
    , m_memoryName()
    , m_memorySize(0)
    , m_udpReceiverIP()
    , m_udpPort(0)
    , m_velodyneSharedMemory(NULL)
    , m_udpreceiver(NULL)
    , m_velodyne16decoder(NULL) {}

ProxyVelodyne16::~ProxyVelodyne16() {}

void ProxyVelodyne16::setUp() {
    m_memoryName = getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.sharedMemory.name");
    m_memorySize = getKeyValueConfiguration().getValue< uint32_t >("ProxyVelodyne16.sharedMemory.size");
    m_velodyneSharedMemory = SharedMemoryFactory::createSharedMemory(m_memoryName, m_memorySize);

    m_udpReceiverIP = getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.udpReceiverIP");
    m_udpPort = getKeyValueConfiguration().getValue< uint32_t >("ProxyVelodyne16.udpPort");
    m_udpreceiver = UDPFactory::createUDPReceiver(m_udpReceiverIP, m_udpPort);

    m_velodyne16decoder = shared_ptr< Velodyne16Decoder >(new Velodyne16Decoder(m_velodyneSharedMemory, getConference(), getKeyValueConfiguration().getValue< string >("ProxyVelodyne16.calibration")));

    m_udpreceiver->setStringListener(m_velodyne16decoder.get());
    // Start receiving bytes.
    m_udpreceiver->start();
}

void ProxyVelodyne16::tearDown() {
    m_udpreceiver->stop();
    m_udpreceiver->setStringListener(NULL);
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
