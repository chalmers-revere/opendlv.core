/**
 * proxy-velodyne32 - Interface to HDL-32E
 * Copyright (C) 2017 Hang Yin
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

#ifndef PROXY_PROXYVELODYNE32_H
#define PROXY_PROXYVELODYNE32_H

#include <memory>

#include "opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>
#include "velodyne32Decoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::wrapper;

class ProxyVelodyne32 : public odcore::base::module::DataTriggeredConferenceClientModule {
   private:
    /**
     * "Forbidden" copy constructor. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the copy constructor.
     *
     * @param obj Reference to an object of this class.
     */
    ProxyVelodyne32(const ProxyVelodyne32 & /*obj*/);

    /**
     * "Forbidden" assignment operator. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the assignment operator.
     *
     * @param obj Reference to an object of this class.
     * @return Reference to this instance.
     */
    ProxyVelodyne32 &operator=(const ProxyVelodyne32 & /*obj*/);

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyVelodyne32(const int32_t &argc, char **argv);

    virtual ~ProxyVelodyne32();

    virtual void nextContainer(odcore::data::Container &);

   private:
    virtual void setUp();
    virtual void tearDown();
    
   private:
    uint8_t m_pointCloudOption;  //0: shared point cloud (SPC) only; 1: compact point cloud (CPC) only; 2: both SPC and CPC
    uint8_t m_SPCOption; //0: xyz+intensity; 1: distance+azimuth+vertical angle+intensity
    uint8_t m_CPCIntensityOption; //Only used when CPC is enabled. 0: without intensity; 1: with intensity; 2: send a CPC container twice, one with intensity, and the other without intensity
    uint8_t m_numberOfBitsForIntensity; //Range 0-7. Only used when CPC is enabled
    uint8_t m_intensityPlacement; //0: higher bits; 1: lower bits
    uint8_t m_distanceEncoding; //0: cm; 1: 2mm
    
    string m_memoryName;   //Name of the shared memory
    uint32_t m_memorySize; //The total size of the shared memory: MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * sizeof(float), where MAX_POINT_SIZE is the maximum number of points per frame (This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed), NUMBER_OF_COMPONENTS_PER_POIN=3 (azimuth, distance, intensity) Recommended values: MAX_POINT_SIZE=1120000

    string m_udpReceiverIP; //"0.0.0.0" to listen to all network interfaces
    uint32_t m_udpPort;     //2368 for velodyne

    std::shared_ptr< SharedMemory > m_velodyneSharedMemory;
    std::shared_ptr< odcore::io::udp::UDPReceiver > m_udpreceiver;
    std::shared_ptr< opendlv::core::system::proxy::Velodyne32Decoder > m_velodyne32decoder;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYVELODYNE_H*/
