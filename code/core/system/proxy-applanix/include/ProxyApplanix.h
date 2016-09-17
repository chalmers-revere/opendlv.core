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

#ifndef PROXY_PROXYAPPLANIX_H
#define PROXY_PROXYAPPLANIX_H

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/io/tcp/TCPConnection.h>

#include "ApplanixStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

/**
 * Interface to GPS/IMU unit Applanix.
 */
class ProxyApplanix : public odcore::base::module::DataTriggeredConferenceClientModule {
   private:
    ProxyApplanix(const ProxyApplanix & /*obj*/) = delete;
    ProxyApplanix &operator=(const ProxyApplanix & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyApplanix(const int &argc, char **argv);

    virtual ~ProxyApplanix();
    virtual void nextContainer(odcore::data::Container &c);

   private:
    void setUp();
    void tearDown();

   private:
    std::shared_ptr< odcore::io::tcp::TCPConnection > m_applanix;
    std::unique_ptr< ApplanixStringDecoder > m_applanixStringDecoder;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYAPPLANIX_H*/
