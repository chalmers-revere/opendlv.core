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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef PROXY_TRIMBLESTRINGDECODER_H
#define PROXY_TRIMBLESTRINGDECODER_H

#include <sstream>

#include <opendavinci/odcore/io/StringListener.h>
#include <opendavinci/odcore/io/conference/ContainerConference.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * This class decodes data from the Trimble unit.
 */
class TrimbleStringDecoder : public odcore::io::StringListener {
   private:
    TrimbleStringDecoder(TrimbleStringDecoder const &) = delete;
    TrimbleStringDecoder &operator=(TrimbleStringDecoder const &) = delete;

   public:
    TrimbleStringDecoder(odcore::io::conference::ContainerConference &, bool);
    virtual ~TrimbleStringDecoder();

    virtual void nextString(const std::string &s);

   private:
    odcore::io::conference::ContainerConference &m_conference;
    std::stringstream m_buffer;
    bool m_debug;
};
}
}
}
} // opendlv::core::system::proxy

#endif
