/**
 * proxy-relays - Interface to relays.
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

#ifndef PROXY_PROXYRELAY_H
#define PROXY_PROXYRELAY_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class Gpio;

/**
 * Interface to GPIO relays.
 */
class ProxyRelay : public odcore::base::module::DataTriggeredConferenceClientModule {
   private:
    ProxyRelay(const ProxyRelay &) = delete;
    ProxyRelay &operator=(const ProxyRelay &) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyRelay(const int &, char **);

    virtual ~ProxyRelay();

   public:
    virtual void nextContainer(odcore::data::Container &);

   private:
    void setUp();
    void tearDown();

    std::unique_ptr<Gpio> m_gpio;
};

}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYRELAY_H*/
