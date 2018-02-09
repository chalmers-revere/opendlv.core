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
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

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

        opendlv::proxy::AngularVelocityReading gyroscopeReading = m_device->ReadGyroscope();
        odcore::data::Container gyroscopeContainer(gyroscopeReading);
        getConference().send(gyroscopeContainer);

        opendlv::proxy::AccelerationReading accelerometerReading = m_device->ReadAccelerometer();
        odcore::data::Container accelerometerContainer(accelerometerReading);
        getConference().send(accelerometerContainer);

        opendlv::proxy::MagneticFieldReading magnetometerReading = m_device->ReadMagnetometer();
        odcore::data::Container magnetometerContainer(magnetometerReading);
        getConference().send(magnetometerContainer);

        opendlv::proxy::AltitudeReading altimeterReading = m_device->ReadAltimeter();
        odcore::data::Container altimeterContainer(altimeterReading);
        getConference().send(altimeterContainer);

        opendlv::proxy::TemperatureReading temperatureReading = m_device->ReadTemperature();
        odcore::data::Container temperatureContainer(temperatureReading);
        getConference().send(temperatureContainer);


        if (m_debug) {
            std::cout << gyroscopeReading.toString() << ", "
            << accelerometerReading.toString() << ", "
            << magnetometerReading.toString() << ", "
            << altimeterReading.toString() << ", "
            << temperatureReading.toString() << std::endl;
        }
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void ProxyIMU::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
    double roll = kv.getValue<double>("proxy-imu.mount.roll")*M_PI/180.0;
    double pitch = kv.getValue<double>("proxy-imu.mount.pitch")*M_PI/180.0;
    double yaw = kv.getValue<double>("proxy-imu.mount.yaw")*M_PI/180.0;
    std::vector<double> const mountRotation({roll, pitch, yaw});
    std::string const type = kv.getValue<std::string>("proxy-imu.type");
    uint32_t const calibrationNumberOfSamples = kv.getValue<uint32_t>("proxy-imu.calibration_number_of_samples");
    bool const lockCalibration = (kv.getValue< int32_t >("proxy-imu.lockcalibration") == 1);
    m_debug = (kv.getValue< int32_t >("proxy-imu.debug") == 1);
    std::string const calibrationFile = kv.getValue<std::string>("proxy-imu.calibration_file");

    if (type.compare("pololu.altimu10") == 0) {
        std::string const deviceNode = kv.getValue< std::string >("proxy-imu.pololu.altimu10.device_node");
        std::string const addressType = kv.getValue<std::string>("proxy-imu.pololu.altimu10.address_type");
        if(addressType.compare("high") || addressType.compare("low")) {
            m_device = std::unique_ptr<PololuAltImu10Device>(new PololuAltImu10Device(deviceNode, addressType, mountRotation, calibrationNumberOfSamples, calibrationFile, lockCalibration, m_debug));
        } else {
            std::cerr << "[proxy-imu] Address type invalid. Must be either high xor low." << std::endl; 
        }
        std::cout << "[proxy-imu] Successfully initiated.";
    }
    if (m_device.get() == nullptr) {
        std::cerr << "[proxy-imu] No valid device driver defined."
                  << std::endl;
    }
}

void ProxyIMU::tearDown() {
    m_device->saveCalibrationFile();
}
}
}
}
} // opendlv::core::system::proxy
