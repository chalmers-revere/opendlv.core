/**
 * UDPPacketReceiver is used to receive UDP packets
 * Copyright (C) 2015 Christian Berger and Hang Yin
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
 
#ifndef UDPPACKETRECEIVER_H_
#define UDPPACKETRECEIVER_H_

#include "opendavinci/odcore/base/Mutex.h"
#include <opendavinci/odcore/io/PacketListener.h>
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerListener.h"
#include "opendavinci/odcore/io/conference/ContainerObserver.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

    // This class will handle packets received via a UDP socket.
    class UDPPacketReceiver : public odcore::io::PacketListener,
                              public odcore::io::conference::ContainerObserver {
    private:
        /**
         * "Forbidden" copy constructor. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the copy constructor.
         */
        UDPPacketReceiver(const UDPPacketReceiver &);

        /**
         * "Forbidden" assignment operator. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the assignment operator.
         */
        UDPPacketReceiver& operator=(const UDPPacketReceiver &);

    public:
        /**
         * Constructor.
         */
        UDPPacketReceiver();

        virtual ~UDPPacketReceiver();
        
        virtual void setContainerListener(odcore::io::conference::ContainerListener *cl);
        // Your class needs to implement the method void void nextPacket(const odcore::io::Packet &p).
        virtual void nextPacket(const odcore::io::Packet &p);
    
    private:
        /**
         * This method is used to pass received data thread-safe
         * to the registered ContainerListener.
         */
        void invokeContainerListener(odcore::data::Container &c);
        odcore::base::Mutex m_containerListenerMutex;
        odcore::io::conference::ContainerListener *m_containerListener;
    };
}
}
}
} // opendlv::core::system::proxy
#endif /*UDPPACKETRECEIVER_H_*/

