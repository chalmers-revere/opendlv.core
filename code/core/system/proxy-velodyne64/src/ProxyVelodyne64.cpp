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

#include <fstream>
#include <iostream>
#include <memory>

#include "opendavinci/odcore/base/KeyValueConfiguration.h"
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
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-velodyne64")
    , m_memoryName()
    , m_memorySize(0)
    , m_udpReceiverIP()
    , m_udpPort(0)
    , m_velodyneSharedMemory(NULL)
    , m_udpreceiver(NULL)
    , m_velodyne64decoder(NULL) {}

ProxyVelodyne64::~ProxyVelodyne64() {}

void ProxyVelodyne64::setUp() {
    m_memoryName = getKeyValueConfiguration().getValue< string >("proxy-velodyne64.sharedMemory.name");
    m_memorySize = getKeyValueConfiguration().getValue< uint32_t >("proxy-velodyne64.sharedMemory.size");
    m_velodyneSharedMemory = SharedMemoryFactory::createSharedMemory(m_memoryName, m_memorySize);

    m_udpReceiverIP = getKeyValueConfiguration().getValue< string >("proxy-velodyne64.udpReceiverIP");
    m_udpPort = getKeyValueConfiguration().getValue< uint32_t >("proxy-velodyne64.udpPort");
    m_udpreceiver = UDPFactory::createUDPReceiver(m_udpReceiverIP, m_udpPort);

    m_velodyne64decoder = shared_ptr< Velodyne64Decoder >(new Velodyne64Decoder(m_velodyneSharedMemory, getConference(), getKeyValueConfiguration().getValue< string >("proxy-velodyne64.calibration")));

    m_udpreceiver->setStringListener(m_velodyne64decoder.get());
    // Start receiving bytes.
    m_udpreceiver->start();
}

void ProxyVelodyne64::tearDown() {
    m_udpreceiver->stop();
    m_udpreceiver->setStringListener(NULL);
}

void ProxyVelodyne64::nextContainer(odcore::data::Container &){}

}
}
}
} // opendlv::core::system::proxy
