/**
 * proxy-v2v - Interface to V2V.
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
#include <opendavinci/odcore/io/udp/UDPFactory.h>

#include "odvdv2v/GeneratedHeaders_ODVDV2V.h"

#include "ProxyV2V.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;

ProxyV2V::ProxyV2V(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-v2v")
    , m_initialised(false)
    , m_udpsender()
    , m_udpreceiver()
    , m_debug() {}

ProxyV2V::~ProxyV2V() {}

void ProxyV2V::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

    const string RECEIVER = "0.0.0.0";
    const uint32_t RECEIVERPORT = kv.getValue< uint32_t >("proxy-v2v.listenPort");
    try {
        m_udpreceiver = shared_ptr< odcore::io::udp::UDPReceiver >(odcore::io::udp::UDPFactory::createUDPReceiver(RECEIVER, RECEIVERPORT));
        m_udpreceiver->setPacketListener(this);
        m_udpreceiver->start();
    }
    catch (std::string &exception) {
        cerr << "[" << getName() << "] Error while creating UDP receiver:  " << exception << endl;
    }


    const string TARGET = kv.getValue< std::string >("proxy-v2v.comboxIp");
    const uint32_t TARGETPORT = kv.getValue< uint32_t >("proxy-v2v.comboxPort");
    try {
        m_udpsender = std::shared_ptr< odcore::io::udp::UDPSender >(odcore::io::udp::UDPFactory::createUDPSender(TARGET, TARGETPORT));
    }
    catch (std::string &exception) {
        cerr << "[" << getName() << "] Error while creating UDP sender:  " << exception << endl;
    }
    m_debug = (kv.getValue<int32_t>("proxy-v2v.debug") == 1);
    m_initialised = true;
}

void ProxyV2V::tearDown() {
    m_udpreceiver->stop();
    m_udpreceiver->setPacketListener(NULL);
}

void ProxyV2V::nextPacket(const odcore::data::Packet &p) {
    if(m_debug) {
        cout << "[" << getName() << "] Received a packet from " << p.getSender() << ", "
             << "with " << p.getData().length()
             // << " bytes containing '"
             // << p.getData() << "'"
             << endl;
    }

    opendlv::proxy::V2vReading nextMessage;
    nextMessage.setSize(p.getData().length());
    nextMessage.setData(p.getData());

    odcore::data::Container c(nextMessage);
    getConference().send(c);
}

void ProxyV2V::nextContainer(odcore::data::Container &c) {
    if (!m_initialised) {
        return;
    }
    if (c.getDataType() == opendlv::proxy::V2vRequest::ID()) {
        if (m_debug) {
            cout << "[" << getName() << "] Got an outbound message" << endl;
        }

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
