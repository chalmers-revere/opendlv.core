/**
 * Copyright (C) 2017 openKorp
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
#include <ctype.h>
#include <iostream>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "odvdimu/GeneratedHeaders_ODVDIMU.h"

#include "Pololualtimu10device.h"
#include "ProxyIMU.h"

namespace opendlv {
namespace core {
namespace system{
namespace proxy{


ProxyIMU::ProxyIMU(int32_t const &a_argc, char **a_argv)
    : TimeTriggeredConferenceClientModule(
      a_argc, a_argv, "proxy-miniature-imu")
    , m_device()
{
}

ProxyIMU::~ProxyIMU() {
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyIMU::body() {
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() 
      == odcore::data::dmcp::ModuleStateMessage::RUNNING) {

    auto gyroscopeReading = m_device->ReadGyroscope();
    odcore::data::Container gyroscopeContainer(gyroscopeReading);
    gyroscopeContainer.setSenderStamp(getIdentifier());
    getConference().send(gyroscopeContainer);

    auto accelerometerReading = m_device->ReadAccelerometer();
    odcore::data::Container accelerometerContainer(accelerometerReading);
    accelerometerContainer.setSenderStamp(getIdentifier());
    getConference().send(accelerometerContainer);

    auto magnetometerReading = m_device->ReadMagnetometer();
    odcore::data::Container magnetometerContainer(magnetometerReading);
    magnetometerContainer.setSenderStamp(getIdentifier());
    getConference().send(magnetometerContainer);

    auto barometerReading = m_device->ReadBarometer();
    odcore::data::Container barometerContainer(barometerReading);
    barometerContainer.setSenderStamp(getIdentifier());
    getConference().send(barometerContainer);

    auto thermometerReading = m_device->ReadThermometer();
    odcore::data::Container thermometerContainer(thermometerReading);
    thermometerContainer.setSenderStamp(getIdentifier());
    getConference().send(thermometerContainer);

    if (isVerbose()) {
      std::cout << "[" << getName() << "] Sending IMU data: \n"
          << gyroscopeReading.toString() << "\n"
          << accelerometerReading.toString() << "\n"
          << magnetometerReading.toString() << "\n"
          << barometerReading.toString() << "\n"
          << thermometerReading.toString() << std::endl;
    }
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void ProxyIMU::setUp() {
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  std::string const type = kv.getValue<std::string>(getName() + ".type");

  if (type.compare("pololu.altimu10") == 0) {
    std::string const deviceNode = 
        kv.getValue< std::string >(getName() + ".pololu.altimu10.device_node");
    std::string const addressType = 
        kv.getValue<std::string>(getName() + ".pololu.altimu10.address_type");
    if (addressType.compare("high") || addressType.compare("low")) {
      m_device = std::unique_ptr<PololuAltImu10Device>(
          new PololuAltImu10Device(deviceNode, addressType));
    } else {
      std::cerr 
          << "[" << getName() 
          << "] Address type invalid. Must be either high xor low." 
          << std::endl; 
    }
    std::cout << "[" << getName() << "] Successfully initiated.";
  }
  if (m_device.get() == nullptr) {
    std::cerr << "[" << getName() << "] No valid device driver defined."
        << std::endl;
  }
}

void ProxyIMU::tearDown() {}

}
}
}
} 
