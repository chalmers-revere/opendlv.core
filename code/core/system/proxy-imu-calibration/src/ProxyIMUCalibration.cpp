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

#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>
#include <opendavinci/odcore/wrapper/Eigen.h>

#include "odvdimu/GeneratedHeaders_ODVDIMU.h"

#include "ProxyIMUCalibration.h"

namespace opendlv {
namespace core {
namespace system{
namespace proxy{


ProxyIMUCalibration::ProxyIMUCalibration(int32_t const &a_argc, char **a_argv)
  : DataTriggeredConferenceClientModule(a_argc, a_argv, "proxy-imu-calibration"),
  m_calibratedId(),
  m_uncalibratedId()
{
}

ProxyIMUCalibration::~ProxyIMUCalibration()
{
}

void ProxyIMUCalibration::nextContainer(odcore::data::Container &a_c) {
  if (a_c.getSenderStamp() != m_uncalibratedId) {
    return;
  }

  if (a_c.getDataType() == opendlv::proxy::AccelerometerReading::ID()) {
    auto accelerometerReading = a_c.getData<opendlv::proxy::AccelerometerReading>();
    std::cout << "Got uncalibrated accelerometerReading: " << accelerometerReading.toString() << std::endl;

    // Do stuff to the reading.

    odcore::data::Container accelerometerContainer(accelerometerReading);
    accelerometerContainer.setSenderStamp(m_calibratedId);
    getConference().send(accelerometerContainer);
  }
  
  if (a_c.getDataType() == opendlv::proxy::GyroscopeReading::ID()) {
    auto gyroscopeReading = a_c.getData<opendlv::proxy::GyroscopeReading>();
    std::cout << "Got uncalibrated gyroscopeReading: " << gyroscopeReading.toString() << std::endl;

    // Do stuff to the reading.

    odcore::data::Container gyroscopeContainer(gyroscopeReading);
    gyroscopeContainer.setSenderStamp(m_calibratedId);
    getConference().send(gyroscopeContainer);
  }

  if (a_c.getDataType() == opendlv::proxy::MagnetometerReading::ID()) {
    auto magnetometerReading = a_c.getData<opendlv::proxy::MagnetometerReading>();
    std::cout << "Got uncalibrated magnetometerReading: " << magnetometerReading.toString() << std::endl;

    // Do stuff to the reading.

    odcore::data::Container magnetometerContainer(magnetometerReading);
    magnetometerContainer.setSenderStamp(m_calibratedId);
    getConference().send(magnetometerContainer);
  }

  if (a_c.getDataType() == opendlv::proxy::BarometerReading::ID()) {
    auto barometerReading = a_c.getData<opendlv::proxy::BarometerReading>();
    std::cout << "Got uncalibrated barometerReading: " << barometerReading.toString() << std::endl;

    // Do stuff to the reading.

    odcore::data::Container barometerContainer(barometerReading);
    barometerContainer.setSenderStamp(m_calibratedId);
    getConference().send(barometerContainer);
  }

  if (a_c.getDataType() == opendlv::proxy::ThermometerReading::ID()) {
    auto thermometerReading = a_c.getData<opendlv::proxy::ThermometerReading>();
    std::cout << "Got uncalibrated thermometerReading: " << thermometerReading.toString() << std::endl;

    // Do stuff to the reading.

    odcore::data::Container thermometerContainer(thermometerReading);
    thermometerContainer.setSenderStamp(m_calibratedId);
    getConference().send(thermometerContainer);
  }
}

void ProxyIMUCalibration::setUp() 
{
  m_uncalibratedId = getKeyValueConfiguration().getValue<uint32_t>("proxy-imu-calibration.uncalibrated_id");
  m_calibratedId = getKeyValueConfiguration().getValue<uint32_t>("proxy-imu-calibration.calibrated_id");
}

void ProxyIMUCalibration::tearDown()
{
}

}
}
}
} 
