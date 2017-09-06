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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PROXY_PROXYV2XOUT_H
#define PROXY_PROXYV2XOUT_H

#include <memory>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/io/udp/UDPSender.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class ProxyV2xOut : public odcore::base::module::TimeTriggeredConferenceClientModule {
  public:
    ProxyV2xOut(int32_t const &, char **);
    ProxyV2xOut(ProxyV2xOut const &) = delete;
    ProxyV2xOut &operator=(ProxyV2xOut const &) = delete;
    virtual ~ProxyV2xOut();

    virtual void nextContainer(odcore::data::Container &);
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

  private:
    void setUp();
    void tearDown();
    std::string getBinaryString(uint32_t) const;

  private:
    std::shared_ptr<odcore::io::udp::UDPSender> m_v2xOut;
    std::vector<uint32_t> m_filterMessageIds;
    uint32_t m_senderId;
    bool m_isPinging;
};

}
}
}
}

#endif
