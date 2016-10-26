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

#ifndef PROXY_POLOLUALTIMU10DEVICE_H
#define PROXY_POLOLUALTIMU10DEVICE_H

#include <cstdint>
#include <iostream>
#include <string>

#include "Device.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class PololuAltImu10Device : public Device {
 public:
  PololuAltImu10Device(std::string const &);
  PololuAltImu10Device(PololuAltImu10Device const &) = delete;
  PololuAltImu10Device &operator=(PololuAltImu10Device const &) = delete;
  virtual ~PololuAltImu10Device();
  opendlv::proxy::AccelerometerReading ReadAccelerometer();
  opendlv::proxy::AltimeterReading ReadAltimeter();
  opendlv::proxy::CompassReading ReadCompass();
  opendlv::proxy::GyroscopeReading ReadGyroscope();

 private:
  void I2cWriteRegister(uint8_t, uint8_t);

  int16_t m_deviceFile;
};
}
}
}
} // opendlv::core::system::proxy

#endif
