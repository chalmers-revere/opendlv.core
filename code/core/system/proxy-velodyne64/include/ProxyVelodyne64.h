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

#ifndef PROXY_PROXYVELODYNE64_H
#define PROXY_PROXYVELODYNE64_H

#include <memory>

#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "velodyne64Decoder.h"
#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::wrapper;

class ProxyVelodyne64 : public odcore::base::module::DataTriggeredConferenceClientModule {
   private:
    ProxyVelodyne64(const ProxyVelodyne64 & /*obj*/) = delete;
    ProxyVelodyne64 &operator=(const ProxyVelodyne64 & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyVelodyne64(const int &argc, char **argv);

    virtual ~ProxyVelodyne64();

    //odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();
    virtual void nextContainer(odcore::data::Container &);

   private:
    void setUp();
    void tearDown();
    
   private:
    string m_memoryName;    //Name of the shared memory
    uint32_t m_memorySize;  //The total size of the shared memory: MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * sizeof(float), where MAX_POINT_SIZE is the maximum number of points per frame (This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed), NUMBER_OF_COMPONENTS_PER_POIN=4 (x, y, z, intensity) Recommended values: MAX_POINT_SIZE=101000->ProxyVelodyne64.sharedMemory.size = 1616000 in the configuration file.
    string m_udpReceiverIP; //"0.0.0.0" to listen to all network interfaces
    uint32_t m_udpPort;     //2368 for velodyne

    std::shared_ptr< SharedMemory > m_velodyneSharedMemory;
    std::shared_ptr< odcore::io::udp::UDPReceiver > m_udpreceiver;
    std::shared_ptr< opendlv::core::system::proxy::Velodyne64Decoder > m_velodyne64decoder;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYVELODYNE64_H*/
