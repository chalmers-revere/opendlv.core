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
#include <sstream>
#include <string>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>
#include <opendavinci/odcore/strings/StringToolbox.h>


#include "odvdopendlvstandardmessageset/GeneratedHeaders_ODVDOpenDLVStandardMessageSet.h"

#include "ProxyAnglesensor.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyAnglesensor::ProxyAnglesensor(int32_t const &a_argc, char **a_argv)
    : TimeTriggeredConferenceClientModule(
      a_argc, a_argv, "proxy-anglesensor")
    , m_pin()
    , m_convertConstants()
    , m_rawReadingMinMax()
    , m_anglesMinMax()
    , m_calibrationFile()
    , m_debug() {
}

ProxyAnglesensor::~ProxyAnglesensor() {
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyAnglesensor::body() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
    const bool setUpCalibration = (kv.getValue<int32_t>("proxy-anglesensor.setUpCalibration") == 1);
    if(setUpCalibration || LoadCalibration()) {
        Calibrate();
    }
    m_convertConstants.push_back((m_rawReadingMinMax.at(0)+m_rawReadingMinMax.at(1))/2.0f);
    m_convertConstants.push_back((m_anglesMinMax.at(1)-(m_anglesMinMax.at(0)+m_anglesMinMax.at(1))/2.0f)/(m_rawReadingMinMax.at(1)-(m_rawReadingMinMax.at(0)+m_rawReadingMinMax.at(1))/2.0f));
    m_convertConstants.push_back((m_anglesMinMax.at(0)+m_anglesMinMax.at(1))/2.0f);
    
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        uint16_t rawReading = GetRawReading();
        float angle = Analogue2Radians(rawReading);
        
        opendlv::proxy::AngleReading reading(angle);
        odcore::data::Container readingContainer(reading);
        getConference().send(readingContainer);

        if(m_debug) {
            std::cout << "[Proxy Anglesensor]" << reading.toString() << std::endl;
        }
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void ProxyAnglesensor::setUp() {
    odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

    std::string pinString = kv.getValue<std::string>("proxy-anglesensor.pin");
    m_pin = std::stoi(pinString);
    
    const std::string anglesMinMaxString = kv.getValue<std::string>("proxy-anglesensor.anglesminmax");
    std::vector<std::string> anglesMinMax = odcore::strings::StringToolbox::split(anglesMinMaxString, ',');
    for (uint8_t i = 0; i < 2; i++) {
        m_anglesMinMax.push_back(std::stof(anglesMinMax.at(i))*static_cast<float>(M_PI)/180.0f);
    }
    if (m_anglesMinMax.at(1) < m_anglesMinMax.at(0)) {
        m_anglesMinMax.push_back(m_anglesMinMax.at(0));
        m_anglesMinMax.pop_front();
    }


    m_calibrationFile = kv.getValue<std::string>("proxy-anglesensor.calibrationfile");

    m_debug = (kv.getValue<int32_t>("proxy-anglesensor.debug") == 1);
}

void ProxyAnglesensor::tearDown() {
    SaveCalibration();
}

void ProxyAnglesensor::Calibrate() {
    std::cout << "[Proxy Anglesensor] Starting calibration. Finding min and max values of analogue input. Try to turn the angle sensors to min and max angles."<< std::endl;
    uint16_t min = GetRawReading();
    uint16_t max = GetRawReading();
    const uint32_t calibrationIterations = 400;
    uint32_t i = 0;
    while((i < calibrationIterations) && (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING)) {
        uint16_t rawReading = GetRawReading();
        if(rawReading < min) {
            min = rawReading;
        } else if (rawReading > max) {
            max = rawReading;
        }
        std::cout << "[Proxy Anglesensor] (" << i << "/" << calibrationIterations <<") [min,max]: [" << min << "," << max << "]" << std::endl;
        i++;
    }
    m_rawReadingMinMax.push_back(min);
    m_rawReadingMinMax.push_back(max);
}

bool ProxyAnglesensor::LoadCalibration() {
    if(m_calibrationFile.empty()) {
        return true;
    }
    std::ifstream file(m_calibrationFile, std::ifstream::in);
    if(file.is_open()){
        std::string line;
        while(std::getline(file, line)) {
            std::vector<std::string> strList = odcore::strings::StringToolbox::split(line, ' ');
            // for(uint8_t i = 0; i < strList.size(); i++) { 
            //     std::cout << strList[i] << ",";
            // }
            std::cout << std::endl;
            if(strList[0].compare("m_rawReadingMinMax") == 0) {
                for(uint8_t i = 0; i < 2; i++) { 
                    m_rawReadingMinMax.push_back(std::stof(strList[i+1]));
                }
            }
        }
        std::cout << "[Pololu Anglesensor] Loaded the calibration settings.";
        if(m_debug) {
            std::cout << "\nLoaded:\nm_rawReadingMinMax(" << m_rawReadingMinMax.at(0) << "," << m_rawReadingMinMax.at(1) << ")";
        }
        std::cout << std::endl;
        file.close();
        return false;
    } else {
        std::cout << "[Pololu Anglesensor] Could not load the calibration settings. Starting on fresh settings." << std::endl;
        file.close();
        return true;
    }
}

void ProxyAnglesensor::SaveCalibration() {
    if(m_calibrationFile.empty()) {
        return;
    }
    std::ofstream file(m_calibrationFile, std::ifstream::out);
    if(file.is_open()){
        file << "m_rawReadingMinMax";
        for(uint8_t i = 0; i < 2; i++) {
            file << " " << m_rawReadingMinMax.at(i);
        }
        file << std::endl;
        std::cout << "[Pololu Anglesensor] Saved the calibration settings.";
        if(m_debug) {
            std::cout << "\nSaved:\nm_rawReadingMinMax(" << m_rawReadingMinMax.at(0) << "," << m_rawReadingMinMax.at(1) << ")";
        }
        std::cout << std::endl;
    } else {
        std::cout << "[Pololu Anglesensor] Could not save the calibration settings." << std::endl;
    }
    file.flush();
    file.close();
}

uint16_t ProxyAnglesensor::GetRawReading() {
    std::string filename = "/sys/bus/iio/devices/iio:device0/in_voltage" + std::to_string(m_pin) + "_raw";

    std::ifstream file(filename, std::ifstream::in);
    std::string line;
    if(file.is_open()){
        std::getline(file, line);
        uint16_t rawReadings = std::stoi(line);
        file.close();
        return rawReadings;
    } else {
        std::cerr << "[Proxy Anglesensor] Could not read from analogue input. (pin: " << m_pin << ", filename: " << filename << ")" << std::endl;
    }
    return std::numeric_limits<int>::quiet_NaN();
}

float ProxyAnglesensor::Analogue2Radians(uint16_t &a_val) {
    float val = (a_val - m_convertConstants.at(0))*m_convertConstants.at(1) + m_convertConstants.at(2);
    return val;
}



}
}
}
} // opendlv::core::system::proxy
