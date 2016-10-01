/**
 * UDPPacketReceiver is used to receive UDP packets
 * Copyright (C) 2015 Christian Berger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <iostream>
#include "opendavinci/odcore/base/Lock.h"
#include <opendavinci/odcore/io/PacketListener.h>
#include <opendavinci/odcore/io/Packet.h>
#include "opendavinci/GeneratedHeaders_OpenDaVINCI.h"
#include "UDPPacketReceiver.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
    using namespace std;
    using namespace odcore::base;
    using namespace odcore::io::conference;
    using namespace odcore::data;

    UDPPacketReceiver::UDPPacketReceiver() :
        m_containerListenerMutex(),
        m_containerListener(NULL){}

    UDPPacketReceiver::~UDPPacketReceiver() {
        setContainerListener(NULL);
    }
    
    void UDPPacketReceiver::setContainerListener(ContainerListener* listener) {
        Lock l(m_containerListenerMutex);
        m_containerListener = listener;
    }
    
    void UDPPacketReceiver::invokeContainerListener(Container &c) {
        Lock l(m_containerListenerMutex);
        if (m_containerListener != NULL) {
            m_containerListener->nextContainer(c);
        }
    }
    
    void UDPPacketReceiver::nextPacket(const odcore::io::Packet &p) {
        //cout << "Received a packet from " << p.getSender() << ", "
        //     << "with " << p.getData().length() << " bytes containing '"
        //     << c << "'" << endl;
        if(p.getData().length()==1206){
            pcap::PacketHeader ph;
            pcap::Packet packet(ph,p.getData());
            Container c(packet);
            invokeContainerListener(c);
        }
    }
}
}
}
} // opendlv::core::system::proxy
