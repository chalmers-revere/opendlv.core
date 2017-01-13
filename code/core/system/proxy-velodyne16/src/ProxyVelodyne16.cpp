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
#include <stdexcept>

#include "ProxyVelodyne16.h"
#include "opendavinci/odcore/base/KeyValueConfiguration.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"


namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::wrapper;
using namespace odcore::io::udp;

ProxyVelodyne16::ProxyVelodyne16(const int32_t &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-velodyne16")
    , m_pointCloudOption(0)
    , m_CPCIntensityOption(0)
    , m_numberOfBitsForIntensity(0)
    , m_distanceEncoding(1)
    , m_memoryName()
    , m_memorySize(0)
    , m_udpReceiverIP()
    , m_udpPort(0)
    , m_velodyneSharedMemory(NULL)
    , m_udpreceiver(NULL)
    , m_velodyne16decoder(NULL) {}

ProxyVelodyne16::~ProxyVelodyne16() {}

void ProxyVelodyne16::setUp() {
    m_udpReceiverIP = getKeyValueConfiguration().getValue< string >("proxy-velodyne16.udpReceiverIP");
    m_udpPort = getKeyValueConfiguration().getValue< uint32_t >("proxy-velodyne16.udpPort");
    m_udpreceiver = UDPFactory::createUDPReceiver(m_udpReceiverIP, m_udpPort);

    m_pointCloudOption = getKeyValueConfiguration().getValue< uint16_t >("proxy-velodyne16.pointCloudOption");
    cout << "Point cloud option (0: SPC only; 1: CPC only; 2: both):" << +m_pointCloudOption << endl;
    m_CPCIntensityOption = getKeyValueConfiguration().getValue< uint16_t >("proxy-velodyne16.CPCIntensityOption");
    cout << "CPC intensity option ( 0: without intensity; 1: with intensity; 2: both) :" << +m_CPCIntensityOption << endl;
    m_numberOfBitsForIntensity = getKeyValueConfiguration().getValue< uint16_t >("proxy-velodyne16.numberOfBitsForIntensity");
    cout << "Number of bits for intensity in CPC:" << +m_numberOfBitsForIntensity << endl;
    m_distanceEncoding = getKeyValueConfiguration().getValue< uint16_t >("proxy-velodyne16.distanceEncoding");
    cout << "Distance encoding (0: cm; 1: 2mm):" << +m_distanceEncoding << endl;
    
    if (m_pointCloudOption == 0 || m_pointCloudOption == 2) {
        m_memoryName = getKeyValueConfiguration().getValue< string >("proxy-velodyne16.sharedMemory.name");
        m_memorySize = getKeyValueConfiguration().getValue< uint32_t >("proxy-velodyne16.sharedMemory.size");
        m_velodyneSharedMemory = SharedMemoryFactory::createSharedMemory(m_memoryName, m_memorySize);
        if (m_pointCloudOption == 0) {
            m_velodyne16decoder = shared_ptr< Velodyne16Decoder >(new Velodyne16Decoder(m_velodyneSharedMemory, getConference(), getKeyValueConfiguration().getValue< string >("proxy-velodyne16.calibration"), false, m_CPCIntensityOption, m_numberOfBitsForIntensity, m_distanceEncoding));
        }
        else {
            m_velodyne16decoder = shared_ptr< Velodyne16Decoder >(new Velodyne16Decoder(m_velodyneSharedMemory, getConference(), getKeyValueConfiguration().getValue< string >("proxy-velodyne16.calibration"), true, m_CPCIntensityOption, m_numberOfBitsForIntensity, m_distanceEncoding));
        }
    }
    else if (m_pointCloudOption == 1){
        m_velodyne16decoder = shared_ptr< Velodyne16Decoder >(new Velodyne16Decoder(getConference(), getKeyValueConfiguration().getValue< string >("proxy-velodyne16.calibration"), m_CPCIntensityOption, m_numberOfBitsForIntensity, m_distanceEncoding));
    }
    else {
        throw invalid_argument( "Invalid point cloud option! 0: shared point cloud (SPC) only; 1: compact point cloud (CPC) only; 2: both SPC and CPC" );
    }
    
    m_udpreceiver->setStringListener(m_velodyne16decoder.get());
    // Start receiving bytes.
    m_udpreceiver->start();
}

void ProxyVelodyne16::tearDown() {
    m_udpreceiver->stop();
    m_udpreceiver->setStringListener(NULL);
}

void ProxyVelodyne16::nextContainer(odcore::data::Container &){}

}
}
}
} // opendlv::core::system::proxy
