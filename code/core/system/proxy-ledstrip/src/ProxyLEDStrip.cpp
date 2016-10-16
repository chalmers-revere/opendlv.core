/**
 * proxy-ledstrip - Interface to the LED strip.
 * Copyright (C) 2016 Chalmers Revere
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

#include <stdint.h>

#include <iostream>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/io/Packet.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>

#include "odvdv2v/GeneratedHeaders_ODVDV2V.h"

#include "ProxyLEDStrip.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;

ProxyLEDStrip::ProxyLEDStrip(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-ledstrip")
    , m_udpsender()
    , m_udpreceiver() {}

ProxyLEDStrip::~ProxyLEDStrip() {}

void ProxyLEDStrip::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

    const string RECEIVER = "0.0.0.0";
    const uint32_t RECEIVERPORT = kv.getValue< uint32_t >("proxy-ledstrip.listenPort");
    try {
        m_udpreceiver = shared_ptr< odcore::io::udp::UDPReceiver >(odcore::io::udp::UDPFactory::createUDPReceiver(RECEIVER, RECEIVERPORT));
        m_udpreceiver->setPacketListener(this);
        m_udpreceiver->start();
    }
    catch (std::string &exception) {
        cerr << "[" << getName() << "] Error while creating UDP receiver:  " << exception << endl;
    }


    const string TARGET = kv.getValue< std::string >("proxy-ledstrip.comboxIp");
    const uint32_t TARGETPORT = kv.getValue< uint32_t >("proxy-ledstrip.comboxPort");
    try {
        m_udpsender = std::shared_ptr< odcore::io::udp::UDPSender >(odcore::io::udp::UDPFactory::createUDPSender(TARGET, TARGETPORT));
    }
    catch (std::string &exception) {
        cerr << "[" << getName() << "] Error while creating UDP sender:  " << exception << endl;
    }
}

void ProxyLEDStrip::tearDown() {
    m_udpreceiver->stop();
    m_udpreceiver->setPacketListener(NULL);
}

void ProxyLEDStrip::nextPacket(const odcore::io::Packet &p) {
    cout << "[" << getName() << "] Received a packet from " << p.getSender() << ", "
         << "with " << p.getData().length()
         // << " bytes containing '"
         // << p.getData() << "'"
         << endl;

    opendlv::proxy::V2vReading nextMessage;
    nextMessage.setSize(p.getData().length());
    nextMessage.setData(p.getData());

    odcore::data::Container c(nextMessage);
    getConference().send(c);
}

void ProxyLEDStrip::nextContainer(odcore::data::Container &c) {
    if (c.getDataType() == opendlv::proxy::V2vRequest::ID()) {
        cout << "[" << getName() << "] Got an outbound message" << std::endl;

        opendlv::proxy::V2vRequest message = c.getData< opendlv::proxy::V2vRequest >();
        try {
            m_udpsender->send(message.getData());
        }
        catch (string &exception) {
            cerr << "[" << getName() << "] Data could not be sent: " << exception << endl;
        }
    }
}
}
}
}
} // opendlv::core::system::proxy
