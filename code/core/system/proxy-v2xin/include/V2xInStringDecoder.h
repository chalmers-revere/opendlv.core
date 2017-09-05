/**
 * Copyright (C) 2017 Ola Benderius
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

#ifndef PROXY_V2XINSTRINGDECODER_H
#define PROXY_V2XINSTRINGDECODER_H

#include <vector>

#include <opendavinci/odcore/io/StringListener.h>
#include <opendavinci/odcore/io/conference/ContainerConference.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class V2xInStringDecoder : public odcore::io::StringListener {
  public:
    V2xInStringDecoder(odcore::io::conference::ContainerConference &, 
        std::vector<uint32_t>, std::vector<uint32_t>, bool);
    V2xInStringDecoder(V2xInStringDecoder const &) = delete;
    V2xInStringDecoder &operator=(V2xInStringDecoder const &) = delete;
    virtual ~V2xInStringDecoder();

    virtual void nextString(const std::string &s);

  private:
    odcore::io::conference::ContainerConference &m_conference;
    std::stringstream m_buffer;
    std::vector<uint32_t> m_filterMessageIds;
    std::vector<uint32_t> m_filterSenderIds;
    bool m_isVerbose;
};

}
}
}
}

#endif
