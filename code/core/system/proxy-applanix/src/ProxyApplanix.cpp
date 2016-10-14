/**
 * proxy-applanix - Interface to GPS/IMU unit Applanix.
 * Copyright (C) 2016 Christian Berger
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

#include <sstream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/io/tcp/TCPFactory.h>

#include "ProxyApplanix.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::io::tcp;

ProxyApplanix::ProxyApplanix(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-applanix")
    , m_applanix()
    , m_applanixStringDecoder() {}

ProxyApplanix::~ProxyApplanix() {}

void ProxyApplanix::setUp() {
    const string APPLANIX_IP = getKeyValueConfiguration().getValue< std::string >("proxy-applanix.ip");
    const uint32_t APPLANIX_PORT = getKeyValueConfiguration().getValue< uint32_t >("proxy-applanix.port");

    // Separating string decoding for GPS messages received from Applanix unit from this class.
    // Therefore, we need to pass the getConference() reference to the other instance so that it can send containers.
    m_applanixStringDecoder = std::unique_ptr< ApplanixStringDecoder >(new ApplanixStringDecoder(getConference()));

    try {
        m_applanix = shared_ptr< TCPConnection >(TCPFactory::createTCPConnectionTo(APPLANIX_IP, APPLANIX_PORT));
        m_applanix->setRaw(true);

        // m_applanixStringDecoder is handling data from the Applanix unit.
        m_applanix->setStringListener(m_applanixStringDecoder.get());
        m_applanix->start();
    } catch (string &exception) {
        stringstream sstrWarning;
        sstrWarning << "[" << getName() << "] Could not connect to Applanix: " << exception << endl;
        toLogger(odcore::data::LogMessage::LogLevel::WARN, sstrWarning.str());
    }
}

void ProxyApplanix::tearDown() {
    if (m_applanix.get() != NULL) {
        m_applanix->stop();
        m_applanix->setStringListener(NULL);
    }
}

void ProxyApplanix::nextContainer(odcore::data::Container & /*c*/) {}
}
}
}
} // opendlv::core::system::proxy
