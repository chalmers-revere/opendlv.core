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
#include <fstream>
#include <string>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "odvdanglesensor/GeneratedHeaders_ODVDAnglesensor.h"
// #include <odvdanglesensor/GeneratedHeaders_ODVDAnglesensor.h>

#include "ProxyAnglesensor.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyAnglesensor::ProxyAnglesensor(int32_t const &a_argc, char **a_argv)
    : TimeTriggeredConferenceClientModule(
      a_argc, a_argv, "proxy-anglesensor")
    , m_pins()
    , m_scaleValue()
    , m_debug() {

    std::string path = std::string("/sys/devices/platform/bone_capemgr/slots");
    std::ofstream exportFile(path, std::ofstream::out);
    exportFile << "BB-ADC";
    exportFile.close();
}

ProxyAnglesensor::~ProxyAnglesensor() {
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyAnglesensor::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {

    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void ProxyAnglesensor::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

    m_scaleValue = kv.getValue<float>("proxy-anglesensor.reading2voltage");
    std::string pinString = kv.getValue<std::string>("proxy-anglesensor.pins");
    std::vector<std::string> pinStringVector = odcore::strings::StringToolbox::split(pinString, ',');
    for(std::string pin : pinStringVector) {
        m_pins.push_back(std::stoi(pin));
    }
    m_debug = (kv.getValue< int32_t >("proxy-anglesensor.debug") == 1);
        
    /*
    }
    double roll = kv.getValue<double>("proxy-imu.mount.roll")*M_PI/180.0;
    double pitch = kv.getValue<double>("proxy-imu.mount.pitch")*M_PI/180.0;
    double yaw = kv.getValue<double>("proxy-imu.mount.yaw")*M_PI/180.0;
    std::vector<double> const mountRotation({roll, pitch, yaw});
    std::string const type = kv.getValue<std::string>("proxy-imu.type");
    std::string calibrationFile = "";
    bool const lockCalibration = (kv.getValue< int32_t >("proxy-imu.lockcalibration") == 1);
    try {
        calibrationFile = kv.getValue<std::string>("proxy-imu.calibrationfile");
    }
    catch(...) {
        calibrationFile = "";
    }

    if (type.compare("pololu.altimu10") == 0) {
        std::string const deviceNode =
        kv.getValue< std::string >("proxy-imu.pololu.altimu10.device_node");

        m_device = std::unique_ptr<PololuAltImu10Device>(new PololuAltImu10Device(deviceNode, mountRotation, calibrationFile, lockCalibration, m_debug));
    }

    if (m_device.get() == nullptr) {
        std::cerr << "[proxy-imu] No valid device driver defined."
                  << std::endl;
    }
    */
}

void ProxyAnglesensor::tearDown() {
}

std::vector<uint16_t> ProxyAnglesensor::GetRawReadings()
{
    std::vector<uint16_t> rawReadings;

    std::string path = std::string("/sys/bus/iio/devices/iio:device0/"); 
    for (uint16_t pin : m_pins) {


        std::string filename = path + "in_voltage" + std::to_string(pin) + "_raw";

        std::ifstream file(filename, std::ifstream::in);
        std::string line;
        if(file.is_open()){
            std::getline(file, line);
            uint16_t value = std::stoi(line);
            rawReadings.push_back(value);
        } else {
            std::cerr << "[ProxyAnglesensor] Could not read from analogue input. (pin: " << pin << ", filename: " << filename << ")" << std::endl;
        }

        file.close();
    }
    if(m_debug){
        std::cout << "[ProxyAnglesensor]";
        std::vector<uint16_t>::iterator it = rawReadings.begin();
        for (uint16_t pin : m_pins) {
            std::cout << " Pin " << pin << ": " << *it;
            it++;
        }
        std::cout << std::endl;
    }
    return rawReadings;
}



}
}
}
} // opendlv::core::system::proxy
