/**
 * proxy-ledstrip - Interface to the LED strip.
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

#ifndef PROXY_PROXYV2V_H
#define PROXY_PROXYV2V_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/io/PacketListener.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>
#include <opendavinci/odcore/io/udp/UDPSender.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

/**
 * Interface to a V2V unit.
 */
class ProxyLEDStrip : public odcore::base::module::DataTriggeredConferenceClientModule,
                 public odcore::io::PacketListener {
   private:
    ProxyLEDStrip(const ProxyLEDStrip & /*obj*/) = delete;
    ProxyLEDStrip &operator=(const ProxyLEDStrip & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyLEDStrip(const int &argc, char **argv);

    virtual ~ProxyLEDStrip();

   public:
    virtual void nextPacket(const odcore::io::Packet &p);
    virtual void nextContainer(odcore::data::Container &c);

   private:
    void setUp();
    void tearDown();

   private:
    shared_ptr< odcore::io::udp::UDPSender > m_udpsender;
    shared_ptr< odcore::io::udp::UDPReceiver > m_udpreceiver;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYV2V_H*/
