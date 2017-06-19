/**
 * proxy-applanix - Interface to GPS/IMU unit Applanix.
 * Copyright (C) 2016-2017 Christian Berger
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef PROXY_APPLANIXSTRINGDECODER_H
#define PROXY_APPLANIXSTRINGDECODER_H

#include <sstream>

#include <opendavinci/odcore/io/StringListener.h>
#include <opendavinci/odcore/io/conference/ContainerConference.h>

#include "odvdapplanix/GeneratedHeaders_ODVDApplanix.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * This class decodes data from the Applanix unit.
 */
class ApplanixStringDecoder : public odcore::io::StringListener {
   private:
        enum GRP_SIZES {
            GRP_HEADER_SIZE             = 8,
            GRP_FOOTER_SIZE             = 4,
            TIME_DISTANCE_FIELD_SIZE    = 26,
        };

        enum ApplanixMessages {
            UNKNOWN                     = 0,
            GRP1                        = 1,
            GRP2                        = 2,
            GRP3                        = 3,
            GRP4                        = 4,
            GRP10001                    = 10001,
            GRP10002                    = 10002,
            GRP10003                    = 10003,
            GRP10009                    = 10009,
        };

   private:
    ApplanixStringDecoder(ApplanixStringDecoder const &) = delete;
    ApplanixStringDecoder &operator=(ApplanixStringDecoder const &) = delete;

   public:
    ApplanixStringDecoder(odcore::io::conference::ContainerConference &);
    virtual ~ApplanixStringDecoder();

    virtual void nextString(const std::string &s);

   private:
    void prepareReadingBuffer(std::stringstream &buffer);
    opendlv::core::sensors::applanix::TimeDistance getTimeDistance(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp1Data getGRP1(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp2Data getGRP2(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp3Data getGRP3(std::stringstream &buffer);
    opendlv::core::sensors::applanix::GNSSReceiverChannelStatus getGNSSReceiverChannelStatus(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp4Data getGRP4(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp10001Data getGRP10001(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp10002Data getGRP10002(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp10003Data getGRP10003(std::stringstream &buffer);
    opendlv::core::sensors::applanix::Grp10009Data getGRP10009(std::stringstream &buffer);

   private:
    odcore::io::conference::ContainerConference &m_conference;
    std::stringstream m_buffer;
    bool m_foundHeader;
    bool m_buffering;
    uint32_t m_payloadSize;
    uint32_t m_toRemove;
    ApplanixMessages m_nextApplanixMessage;

};
}
}
}
} // opendlv::core::system::proxy

#endif
