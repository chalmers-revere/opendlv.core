/**
 * Copyright (C) 2015 Chalmers REVERE
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

#ifndef PROXY_PROXYANGLESENSOR_H
#define PROXY_PROXYANGLESENSOR_H

#include <memory>
#include <vector>
#include <deque>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * This class provides interface to an angle sensor through analogue voltage input reading.
 */
class ProxyAnglesensor : public odcore::base::module::TimeTriggeredConferenceClientModule {
   public:
    ProxyAnglesensor(int32_t const &, char **);

    ProxyAnglesensor(ProxyAnglesensor const &) = delete;

    ProxyAnglesensor &operator=(ProxyAnglesensor const &) = delete;

    virtual ~ProxyAnglesensor();

    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

   private:
    void setUp();

    void tearDown();

    void Calibrate();

    bool LoadCalibration();

    void SaveCalibration();

    uint16_t GetRawReading();

    float Analogue2Radians(uint16_t &);

    uint16_t m_pin;
  
    std::vector<float> m_convertConstants;

    std::deque<uint16_t> m_rawReadingMinMax;

    std::deque<float> m_anglesMinMax;

    std::string m_calibrationFile;

    bool m_debug;
};

}
}
}
} // opendlv::core::system::proxy

#endif
