/**
 * Copyright (C) 2017 Chalmers Revere
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

#ifndef PROXY_PROXYIMUCALIBRATION_H
#define PROXY_PROXYIMUCALIBRATION_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class ProxyIMUCalibration : public odcore::base::module::DataTriggeredConferenceClientModule {
   public:
    ProxyIMUCalibration(int32_t const &, char **);
    ProxyIMUCalibration(ProxyIMUCalibration const &) = delete;
    ProxyIMUCalibration &operator=(ProxyIMUCalibration const &) = delete;
    virtual ~ProxyIMUCalibration();
    virtual void nextContainer(odcore::data::Container &); 

   private:
    void setUp();
    void tearDown();

    uint32_t m_calibratedId;
    uint32_t m_uncalibratedId;
};

}
}
}
}

#endif

