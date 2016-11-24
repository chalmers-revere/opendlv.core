/**
 * proxy-trimble - Interface to GPS/IMU unit Trimble.
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

#include "ProxyTrimble.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::io::tcp;

ProxyTrimble::ProxyTrimble(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-trimble")
    , m_trimble()
    , m_trimbleStringDecoder() {}

ProxyTrimble::~ProxyTrimble() {}

void ProxyTrimble::setUp() {
    const string TRIMBLE_IP = getKeyValueConfiguration().getValue< std::string >("proxy-trimble.ip");
    const uint32_t TRIMBLE_PORT = getKeyValueConfiguration().getValue< uint32_t >("proxy-trimble.port");
    const bool DEBUG = (getKeyValueConfiguration().getValue< uint32_t >("proxy-trimble.debug") == 1);

    // Separating string decoding for GPS messages received from Trimble unit from this class.
    // Therefore, we need to pass the getConference() reference to the other instance so that it can send containers.
    m_trimbleStringDecoder = std::unique_ptr< TrimbleStringDecoder >(new TrimbleStringDecoder(getConference(),DEBUG));

    try {
        m_trimble = shared_ptr< TCPConnection >(TCPFactory::createTCPConnectionTo(TRIMBLE_IP, TRIMBLE_PORT));
        m_trimble->setRaw(true);

        // m_trimbleStringDecoder is handling data from the Trimble unit.
        m_trimble->setStringListener(m_trimbleStringDecoder.get());
        m_trimble->start();
    } catch (string &exception) {
        stringstream info;
        info << "[" << getName() << "] Could not connect to Trimble: " << exception << endl;
        toLogger(odcore::data::LogMessage::LogLevel::INFO, info.str());
    }
}

void ProxyTrimble::tearDown() {
    if (m_trimble.get() != NULL) {
        m_trimble->stop();
        m_trimble->setStringListener(NULL);
    }
}

void ProxyTrimble::nextContainer(odcore::data::Container & /*c*/) {}
}
}
}
} // opendlv::core::system::proxy
