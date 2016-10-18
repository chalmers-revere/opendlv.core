/**
 * proxy-velodyne16 - Interface to VLP-16
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

#ifndef PROXY_PROXYVELODYNE16_H
#define PROXY_PROXYVELODYNE16_H

#include <memory>

#include "opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "velodyne16Decoder.h"
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::wrapper;

class ProxyVelodyne16 : public odcore::base::module::TimeTriggeredConferenceClientModule {
   private:
    /**
     * "Forbidden" copy constructor. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the copy constructor.
     *
     * @param obj Reference to an object of this class.
     */
    ProxyVelodyne16(const ProxyVelodyne16 & /*obj*/);

    /**
     * "Forbidden" assignment operator. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the assignment operator.
     *
     * @param obj Reference to an object of this class.
     * @return Reference to this instance.
     */
    ProxyVelodyne16 &operator=(const ProxyVelodyne16 & /*obj*/);

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyVelodyne16(const int32_t &argc, char **argv);

    virtual ~ProxyVelodyne16();

    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

   private:
    virtual void setUp();
    virtual void tearDown();
    
   private:
    string m_memoryName;   //Name of the shared memory
    uint32_t m_memorySize; //The total size of the shared memory: MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * sizeof(float), where MAX_POINT_SIZE is the maximum number of points per frame (This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed), NUMBER_OF_COMPONENTS_PER_POIN=4 (x, y, z, intensity) Recommended values: MAX_POINT_SIZE=30000->ProxyVelodyne64.sharedMemory.size = 480000 in the configuration file.

    string m_udpReceiverIP; //"0.0.0.0" to listen to all network interfaces
    uint32_t m_udpPort;     //2368 for velodyne

    std::shared_ptr< SharedMemory > m_velodyneSharedMemory;
    std::shared_ptr< odcore::io::udp::UDPReceiver > m_udpreceiver;
    std::shared_ptr< opendlv::core::system::proxy::velodyne16Decoder > m_velodyne16decoder;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYVELODYNE_H*/
