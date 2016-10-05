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

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include "velodyne64Decoder.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include <opendavinci/odcore/io/udp/UDPReceiver.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

    using namespace std;
    using namespace odcore::wrapper;

    class ProxyVelodyne64 : public odcore::base::module::TimeTriggeredConferenceClientModule {
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
        
        odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

       private:
        void setUp();
        void tearDown();
        const std::string NAME = "pointCloud";
        const uint32_t MAX_POINT_SIZE=101000;  //the maximum number of points per frame. This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed.
        const uint8_t NUMBER_OF_COMPONENTS_PER_POINT = 4; // How many components do we have per vector?
        const uint32_t SIZE_PER_COMPONENT = sizeof(float);
        const uint32_t SIZE = MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * SIZE_PER_COMPONENT; // What is the total size of the shared memory? 
        const string RECEIVER = "0.0.0.0";
        const uint32_t PORT = 2368;
        
        std::shared_ptr<SharedMemory> VelodyneSharedMemory;
        std::shared_ptr<odcore::io::udp::UDPReceiver> udpreceiver;
        opendlv::core::system::proxy::velodyne64Decoder v64d;
    };
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYVELODYNE64_H*/
