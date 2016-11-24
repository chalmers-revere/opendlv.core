/**
 * proxy-sick - Interface to Sick.
 * Copyright (C) 2016 Chalmers REVERE
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

#ifndef PROXY_PROXYSICK_H
#define PROXY_PROXYSICK_H

#include <stdint.h>

#include <memory>
#include <string>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/wrapper/SerialPort.h>

#include "SickStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

/**
 * Interface to Sick.
 */
class ProxySick : public odcore::base::module::TimeTriggeredConferenceClientModule {
   private:
    ProxySick(const ProxySick & /*obj*/) = delete;
    ProxySick &operator=(const ProxySick & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxySick(const int &argc, char **argv);

    virtual ~ProxySick();

   private:
    void setUp();
    void tearDown();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

   private:
    void status();
    void startScan();
    void stopScan();
    void settingsMode();
    void setCentimeterMode();
    void setBaudrate38400();
    void setBaudrate500k();
    void openSerialPort(std::string, uint32_t);
   private:
    std::shared_ptr<odcore::wrapper::SerialPort> m_sick;
    std::unique_ptr<SickStringDecoder> m_sickStringDecoder;
    std::string m_serialPort;
    uint32_t m_baudRate;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYSICK_H*/
