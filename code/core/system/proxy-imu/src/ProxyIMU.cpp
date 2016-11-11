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

#include <cmath>
#include <cstring>
#include <ctype.h>
#include <iostream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "odvdimu/GeneratedHeaders_ODVDIMU.h"

#include "Pololualtimu10device.h"
#include "ProxyIMU.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyIMU::ProxyIMU(int32_t const &a_argc, char **a_argv)
    : TimeTriggeredConferenceClientModule(
      a_argc, a_argv, "proxy-imu")
    , m_device()
    , m_debug() {
}

ProxyIMU::~ProxyIMU() {
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyIMU::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {

        auto gyroscopeReading = m_device->ReadGyroscope();
        odcore::data::Container gyroscopeContainer(gyroscopeReading);
        getConference().send(gyroscopeContainer);

        auto accelerometerReading = m_device->ReadAccelerometer();
        odcore::data::Container accelerometerContainer(accelerometerReading);
        getConference().send(accelerometerContainer);

        auto magnetometerReading = m_device->ReadCompass();
        odcore::data::Container compassContainer(magnetometerReading);
        getConference().send(compassContainer);

        auto altimeterReading = m_device->ReadAltimeter();
        odcore::data::Container altimeterContainer(altimeterReading);
        getConference().send(altimeterContainer);

        auto temperatureReading = m_device->ReadTemperature();
        odcore::data::Container temperatureContainer(temperatureReading);
        getConference().send(temperatureContainer);


        if (m_debug) {
            std::cout << gyroscopeReading.toString() << ", "
            << accelerometerReading.toString() << ", "
            << magnetometerReading.toString() << ", "
            << altimeterReading.toString() << ","
            << temperatureReading.toString() << std::endl;
        }
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void ProxyIMU::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

    std::string const type = kv.getValue<std::string>("proxy-imu.type");
    std::string calibrationFile = "";
    try {
        calibrationFile = kv.getValue<std::string>("proxy-imu.calibrationfile");
    }
    catch(...) {
        calibrationFile = "";
    }

    if (type.compare("pololu.altimu10") == 0) {
        std::string const deviceNode =
        kv.getValue< std::string >("proxy-imu.pololu.altimu10.device_node");

        m_device = std::unique_ptr<PololuAltImu10Device>(new PololuAltImu10Device(deviceNode, calibrationFile));
        m_device->loadCalibrationFile();
    }

    if (m_device.get() == nullptr) {
        std::cerr << "[proxy-imu] No valid device driver defined."
                  << std::endl;
    }

    m_debug = (kv.getValue< int32_t >("proxy-imu.debug") == 1);


}

void ProxyIMU::tearDown() {
    m_device->saveCalibrationFile();
}
}
}
}
} // opendlv::core::system::proxy
