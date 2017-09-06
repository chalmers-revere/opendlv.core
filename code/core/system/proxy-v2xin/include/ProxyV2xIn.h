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

#ifndef PROXY_PROXYV2XIN_H
#define PROXY_PROXYV2XIN_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>

#include "V2xInStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class ProxyV2xIn : public odcore::base::module::DataTriggeredConferenceClientModule {
  public:
    ProxyV2xIn(int32_t const &, char **);
    ProxyV2xIn(ProxyV2xIn const &) = delete;
    ProxyV2xIn &operator=(ProxyV2xIn const &) = delete;

    virtual ~ProxyV2xIn();
    virtual void nextContainer(odcore::data::Container &);

  private:
    void setUp();
    void tearDown();

  private:
    std::shared_ptr<odcore::io::udp::UDPReceiver> m_v2xIn;
    std::unique_ptr<V2xInStringDecoder> m_v2xInStringDecoder;
};

}
}
}
}

#endif
