/**
 * proxy-velodyne - Interface to VLP-16
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
#include "opendavinci/odcore/io/protocol/PCAPProtocol.h"
#include "opendavinci/odcore/base/Mutex.h"
#include "velodyneListener16.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include <opendavinci/odcore/io/udp/UDPReceiver.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include "UDPReceiveBytes.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

        using namespace std;
        using namespace odcore::wrapper;
        using namespace odcore::base::module;
        /**
         * This class is a skeleton to send driving commands to Hesperia-light's vehicle driving dynamics simulation.
         */
        class ProxyVelodyne16 : public odcore::base::module::TimeTriggeredConferenceClientModule {
            private:
                /**
                 * "Forbidden" copy constructor. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the copy constructor.
                 *
                 * @param obj Reference to an object of this class.
                 */
                ProxyVelodyne16(const ProxyVelodyne16 &/*obj*/);

                /**
                 * "Forbidden" assignment operator. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the assignment operator.
                 *
                 * @param obj Reference to an object of this class.
                 * @return Reference to this instance.
                 */
                ProxyVelodyne16& operator=(const ProxyVelodyne16 &/*obj*/);

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
                const std::string NAME = "pointCloud";
                const uint32_t MAX_POINT_SIZE=30000;  //The maximum number of points per frame is set based on the observation of the first 100 frames of the pcap file imeangowest.pcap. This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed.
                const uint8_t NUMBER_OF_COMPONENTS_PER_POINT = 4; // How many components do we have per vector?
                const uint32_t SIZE_PER_COMPONENT = sizeof(float);
                const uint32_t SIZE = MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * SIZE_PER_COMPONENT; // What is the total size of the shared memory? 
                const string RECEIVER = "127.0.0.1";
                //const string RECEIVER = "192.168.3.255";
                const uint32_t PORT = 2368;
                const uint32_t CONSUME = 5000; //the number of bytes to be sent to the PCAP decoder each time
                
                odcore::io::protocol::PCAPProtocol m_pcap;
                std::shared_ptr<SharedMemory> VelodyneSharedMemory;
                VelodyneListener16 m_vListener;
                std::shared_ptr<odcore::io::udp::UDPReceiver> udpreceiver;
                opendlv::core::system::proxy::UDPReceiveBytes handler; 
                odcore::base::Mutex rfb; 
        };
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYVELODYNE_H*/
