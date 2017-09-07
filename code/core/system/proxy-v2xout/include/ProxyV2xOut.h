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

#ifndef PROXY_PROXYV2XOUT_H
#define PROXY_PROXYV2XOUT_H

#include <linux/if_ether.h>

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class ProxyV2xOut : public odcore::base::module::DataTriggeredConferenceClientModule {
  public:
    ProxyV2xOut(int32_t const &, char **);
    ProxyV2xOut(ProxyV2xOut const &) = delete;
    ProxyV2xOut &operator=(ProxyV2xOut const &) = delete;
    virtual ~ProxyV2xOut();

    virtual void nextContainer(odcore::data::Container &);

  private:
    void setUp();
    void tearDown();
    std::string getBinaryString(uint32_t) const;

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
    std::vector<uint32_t> m_filterMessageIds;
    uint32_t m_senderId;

    int m_interfaceIndex;
    unsigned char m_sourceAddress[ETH_ALEN];
    int m_rawEthernetSocket;
};

}
}
}
}

#endif
