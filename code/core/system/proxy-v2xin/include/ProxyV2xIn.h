/**
 * Copyright (C) 2017 Ola Benderius, Christian Berger
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

#ifndef PROXY_PROXYV2XIN_H
#define PROXY_PROXYV2XIN_H

#include <linux/if_ether.h>

#include <memory>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>

#include "V2xInStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class ProxyV2xIn : public odcore::base::module::TimeTriggeredConferenceClientModule {
  public:
    ProxyV2xIn(int32_t const &, char **);
    ProxyV2xIn(ProxyV2xIn const &) = delete;
    ProxyV2xIn &operator=(ProxyV2xIn const &) = delete;

    virtual ~ProxyV2xIn();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

  private:
    enum ETHERTYPES {
        GEO_NETWORKING = 0x8947
    };

    union rawEthernetFrame {
        struct {
            struct ethhdr header;
            unsigned char data[ETH_DATA_LEN];
        } field;
        unsigned char rawBuffer[ETH_FRAME_LEN];
    };

  private:
    void setUp();
    void tearDown();

  private:
    std::unique_ptr<V2xInStringDecoder> m_v2xInStringDecoder;

    int m_interfaceIndex;
    unsigned char m_sourceAddress[ETH_ALEN];
    int m_rawEthernetSocket;
};

}
}
}
}

#endif
