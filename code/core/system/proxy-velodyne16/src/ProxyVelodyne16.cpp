/**
 * proxy-velodyne - Interface to VLP-16.
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

#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"
#include "automotivedata/GeneratedHeaders_AutomotiveData.h"
#include "opendavinci/odcore/base/Lock.h"

#include "ProxyVelodyne16.h"


namespace opendlv {
namespace core {
namespace system {
namespace proxy {

        using namespace std;
        using namespace odcore::base;
        using namespace odcore::data;
        using namespace odcore::wrapper;
        using namespace odcore::base::module;
        using namespace odcore::io::udp;

        ProxyVelodyne16::ProxyVelodyne16(const int32_t &argc, char **argv) :
            TimeTriggeredConferenceClientModule(argc, argv, "ProxyVelodyne16"),
            m_pcap(),
            VelodyneSharedMemory(SharedMemoryFactory::createSharedMemory(NAME, SIZE)),
            m_vListener(VelodyneSharedMemory,getConference()),
            udpreceiver(UDPFactory::createUDPReceiver(RECEIVER, PORT)),
            handler(m_pcap),
            rfb(){}

        ProxyVelodyne16::~ProxyVelodyne16() {}

        void ProxyVelodyne16::setUp() {
            m_pcap.setContainerListener(&m_vListener);
            udpreceiver->setStringListener(&handler);
            // Start receiving bytes.
            udpreceiver->start();
        }

        void ProxyVelodyne16::tearDown() {
            udpreceiver->stop();
            udpreceiver->setStringListener(NULL);
        }

        // This method will do the main data processing job.
        //While running this module, adjust the frequency to get desired frame rate of the replay. Note that too low frame rate may lead to buffer overflow!
        odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyVelodyne16::body() {
            while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING){
                while(handler.getBuffer().size()>CONSUME){
                    odcore::base::Lock l(rfb);
                    m_pcap.nextString(handler.getBuffer().substr(0,CONSUME));
                    handler.consume(CONSUME);
                } 
            }
            return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
        }
}
}
}
} // opendlv::core::system::proxy

