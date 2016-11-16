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

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <sys/ioctl.h>

#include <iostream>
#include <fstream>
#include <vector>

#include "Pololualtimu10device.h"

#include "opendavinci/odcore/strings/StringToolbox.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * Constructor for PololuAltImuV5 device interfacing through I2C.
 *
 */
PololuAltImu10Device::PololuAltImu10Device(std::string const &a_deviceName, std::vector<double> const &a_mountRotation, std::string &a_calibrationFile, bool const &a_lockCalibration, bool &a_debug)
    : m_deviceFile()
    , m_rotationMatrix()
    , m_calibrationFile(a_calibrationFile)
    , m_lockCalibration(a_lockCalibration)
    , m_magnetometerMaxVal{0,0,0}
    , m_magnetometerMinVal{0,0,0}
    // , m_heavyAcc{0,0,0}
    , m_initialized(false)
    , m_debug(a_debug)
{
    m_deviceFile = open(a_deviceName.c_str(), O_RDWR);
    if (m_deviceFile < 0) {
        std::cerr << "[Pololu Altimu] Failed to open the i2c bus." << std::endl;
        return;
    }

    std::cout << "[Pololu Altimu] I2C bus " << a_deviceName << " opened successfully."
              << std::endl;

    initLSM6();
    initLIS3();
    initLPS25();

    double roll = a_mountRotation[0];
    double pitch = a_mountRotation[1];
    double yaw = a_mountRotation[2];

    Eigen::Matrix3d rX;
    Eigen::Matrix3d rY;
    Eigen::Matrix3d rZ;

    rX <<
        1,  0,  0,
        0,  std::cos(roll), -std::sin(roll),
        0,  std::sin(roll), std::cos(roll);
    rY << 
        std::cos(pitch), 0,  std::sin(pitch),
        0,  1,  0,
        -std::sin(pitch),    0,  std::cos(pitch);
    rZ <<
        std::cos(yaw), -std::sin(yaw),    0,
        std::sin(yaw), std::cos(yaw), 0,
        0,  0,  1;
    m_rotationMatrix = rX*rY*rZ;
    if(m_debug){
        std::cout << "Rotation matrix: \n" << m_rotationMatrix << std::endl;
    }
    m_initialized = true;
}


PololuAltImu10Device::~PololuAltImu10Device() {
}

bool PololuAltImu10Device::loadCalibrationFile() {
    if(m_calibrationFile.empty()) {
        return 1;
    }
    std::ifstream file(m_calibrationFile, std::ifstream::in);
    if(file.is_open()){
        std::string line;
        while(std::getline(file, line)) {
            std::vector<std::string> strList = odcore::strings::StringToolbox::split(line, ' ');
            for(uint8_t i = 0; i < strList.size(); i++) { 
                std::cout << strList[i] << ",";
            }
            std::cout << std::endl;
            if(strList[0].compare("m_magnetometerMaxVal") == 0) {
                for(uint8_t i = 0; i < 3; i++) { 
                    m_magnetometerMaxVal[i] = std::stof(strList[i+1]);
                }
            } else if(strList[0].compare("m_magnetometerMinVal") == 0) {
                for(uint8_t i = 0; i < 3; i++) { 
                    m_magnetometerMinVal[i] = std::stof(strList[i+1]);
                }
            }
        }

        std::cout << "[Pololu Altimu] Loaded the calibration settings.";
        if(m_debug) {
            std::cout << "\nLoaded:\nm_magnetometerMaxVal(" << m_magnetometerMaxVal[0] << "," << m_magnetometerMaxVal[1] 
            << "," << m_magnetometerMaxVal[2] << ")\nm_magnetometerMinVal(" << m_magnetometerMinVal[0] << "," << m_magnetometerMinVal[1] 
            << "," << m_magnetometerMinVal[2] <<")";
        }
        std::cout << std::endl;
        file.close();
        return 0;
    } else {
        std::cout << "[Pololu Altimu] Could not load the calibration settings. Starting on fresh settings." << std::endl;
        file.close();
        return 1;
    }
}

void PololuAltImu10Device::saveCalibrationFile() {
    if(m_calibrationFile.empty() & m_lockCalibration) {
        return;
    }
    std::ofstream file(m_calibrationFile, std::ifstream::out);
    if(file.is_open()){
        file << "m_magnetometerMaxVal";
        for(uint8_t i = 0; i < 3; i++) {
            file << " " << m_magnetometerMaxVal[i];
        }
        file << std::endl;
        file << "m_magnetometerMinVal";
        for(uint8_t i = 0; i < 3; i++) {
            file << " " << m_magnetometerMinVal[i];
        }
        file << std::endl;
        std::cout << "[Pololu Altimu] Saved the calibration settings.";
        if(m_debug) {
            std::cout << "\nSaved:\nm_magnetometerMaxVal(" << m_magnetometerMaxVal[0] << "," << m_magnetometerMaxVal[1] 
            << "," << m_magnetometerMaxVal[2] << "),\nm_magnetometerMinVal(" << m_magnetometerMinVal[0] << "," << m_magnetometerMinVal[1] 
            << "," << m_magnetometerMinVal[2] <<")";
        }
        std::cout << std::endl;
    } else {
        std::cout << "[Pololu Altimu] Could not save the calibration settings." << std::endl;
    }
    file.flush();
    file.close();
}

void PololuAltImu10Device::accessLSM6() {
    uint8_t address = 0x6b;
    if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
        std::cerr << "[Pololu Altimu] Failed to acquire bus access or talk to slave device. (LSM6 / Accel /Gyro)"
                  << std::endl;
        return;
    }
}

void PololuAltImu10Device::accessLIS3() {
    uint8_t address = 0x1E;
    if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
        std::cerr << "[Pololu Altimu] Failed to acquire bus access or talk to slave device. (LIS3 / Magnetometer)"
                  << std::endl;
        return;
    }
}

void PololuAltImu10Device::accessLPS25() {
    uint8_t address = 0x5d;
    if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
        std::cerr << "[Pololu Altimu] Failed to acquire bus access or talk to slave device. (LPS25 / Pressure)"
                  << std::endl;
        return;
    }
}


void PololuAltImu10Device::initLSM6() {
    accessLSM6();
    // --------------- Accelerometer
    // CTRL1_XL : ODR_XL3, ODR_XL2, ODR_XL1, ODR_XL0, FS_XL1, FS_XL0, BW_XL1, BW_XL0
    // ODR = 1000 (1.66 kHz (high performance));
    // FS_XL = 00 (+/-2 g full scale)
    // Accelerometer full-scale selection. Default value: 00.
    // (00: ±2 g; 01: ±16 g; 10: ±4 g; 11: ±8 g)
    // Anti-aliasing filter bandwidth selection. value: 11
    // (00: 400 Hz; 01: 200 Hz; 10: 100 Hz; 11: 50 Hz)
    // 0100 00 11
    I2cWriteRegister(lsm6RegAddr::CTRL1_XL, 0x43);

    // --------------- Gyro
    // CTRL2_G : ODR_G3, ODR_G2, ODR_G1, ODR_G0, FS_G1, FS_G0, FS_125, 0
    // ODR = 0100 (104 Hz (normal mode)); FS_G = 00 (245 dps)
    I2cWriteRegister(lsm6RegAddr::CTRL2_G, 0x40);

    // Common
    // IF_INC = 1 (automatically increment register address)
    I2cWriteRegister(lsm6RegAddr::CTRL3_C, 0x04);
}

void PololuAltImu10Device::initLIS3() {
    accessLIS3();
    // 0x70 = 0b01110000
    // OM = 11 (ultra-high-performance mode for X and Y); DO = 100 (10 Hz ODR)
    I2cWriteRegister(lis3RegAddr::CTRL_REG1, 0x70);

    // 0x00 = 0b00000000
    // FS = 00 (+/- 4 gauss full scale)
    I2cWriteRegister(lis3RegAddr::CTRL_REG2, 0x00);

    // 0x00 = 0b00000000
    // MD = 00 (continuous-conversion mode)
    I2cWriteRegister(lis3RegAddr::CTRL_REG3, 0x00);

    // 0x0C = 0b00001100
    // OMZ = 11 (ultra-high-performance mode for Z)
    I2cWriteRegister(lis3RegAddr::CTRL_REG4, 0x0C);

    //Initial values for calibration
    if(loadCalibrationFile()) {
        accessLIS3();
        uint8_t buffer[] = {lis3RegAddr::OUT_X_L | 0x80};

        uint8_t status = write(m_deviceFile, buffer, 1);
        if (status != 1) {
            std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Magnetometer)" << std::endl;
        }

        uint8_t outBuffer[6];
        status = read(m_deviceFile, outBuffer, 6);
        if (status != 6) {
            std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Magnetometer)" << std::endl;
        }

        uint8_t xlm = outBuffer[0];
        uint8_t xhm = outBuffer[1];
        uint8_t ylm = outBuffer[2];
        uint8_t yhm = outBuffer[3];
        uint8_t zlm = outBuffer[4];
        uint8_t zhm = outBuffer[5];

        int16_t x = (int16_t)(xhm << 8 | xlm);
        int16_t y = (int16_t)(yhm << 8 | ylm);
        int16_t z = (int16_t)(zhm << 8 | zlm);

        //F
        //FS=±4 gauss  --> 6842 LSB/gauss
        float scaledX = static_cast< double >(x) / 6842.0;
        float scaledY = static_cast< double >(y) / 6842.0;
        float scaledZ = static_cast< double >(z) / 6842.0;

        m_magnetometerMinVal[0] = scaledX;
        m_magnetometerMinVal[1] = scaledY;
        m_magnetometerMinVal[2] = scaledZ;
        m_magnetometerMaxVal[0] = scaledX;
        m_magnetometerMaxVal[1] = scaledY;
        m_magnetometerMaxVal[2] = scaledZ;
    }

}

void PololuAltImu10Device::initLPS25() {
    accessLPS25();
    // 0xB0 = 0b10110000
    // PD = 1 (active mode);  ODR = 011 (12.5 Hz pressure & temperature output data rate)
    I2cWriteRegister(lps25RegAddr::CTRL_REG1, 0xB0);
}

void PololuAltImu10Device::I2cWriteRegister(uint8_t a_register, uint8_t a_value) {
    uint8_t buffer[2];
    buffer[0] = a_register;
    buffer[1] = a_value;

    uint8_t status = write(m_deviceFile, buffer, 2);

    if (status != 2) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus at register:" << a_register << ". status code: " << status << std::endl;
        std::cout << errno;
        return;
    }
}

opendlv::proxy::AccelerometerReading PololuAltImu10Device::ReadAccelerometer() {
    accessLSM6();
    uint8_t buffer[1];
    buffer[0] = lsm6RegAddr::OUTX_L_XL;

    uint8_t status = write(m_deviceFile, buffer, 1);
    if (status != 1) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Accelerometer)" << std::endl;
        return nullptr;
    }

    uint8_t outBuffer[6];
    status = read(m_deviceFile, outBuffer, 6);
    if (status != 6) {
        std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Accelerometer)" << std::endl;
    }

    uint8_t xla = outBuffer[0];
    uint8_t xha = outBuffer[1];
    uint8_t yla = outBuffer[2];
    uint8_t yha = outBuffer[3];
    uint8_t zla = outBuffer[4];
    uint8_t zha = outBuffer[5];

    int16_t x = xha;
    x = (x << 8) | xla;

    int16_t y = yha;
    y = (y << 8) | yla;

    int16_t z = zha;
    z = (z << 8) | zla;

    //FS = ±2 --> 0.061 mg/LSB -->
    static double gravityAccel = -9.81;
    //Hardcoded caliberation
    //TODO: better calibration
    float scaledX = ((0.061 * static_cast< double >(x)) / 1000) * gravityAccel;
    float scaledY = ((0.061 * static_cast< double >(y)) / 1000) * gravityAccel;
    float scaledZ = ((0.061 * static_cast< double >(z)) / 1000) * gravityAccel;


    Eigen::Vector3f rawReading(scaledX,scaledY,scaledZ);
    Eigen::Vector3f adjustedReading = Rotate(rawReading, m_rotationMatrix);

    float reading[] = {adjustedReading[0], adjustedReading[1], adjustedReading[2]};
    

    //For magnetometer calibration
    // for(uint8_t i = 0; i < 3; i++){
    //     m_heavyAcc[i] += 0.5f*(reading[i] - m_heavyAcc[i]);    
    // }



    // float reading[] = {scaledX, scaledY, scaledZ};
    opendlv::proxy::AccelerometerReading accelerometerReading(reading);
    return accelerometerReading;
}

opendlv::proxy::AltimeterReading PololuAltImu10Device::ReadAltimeter() {
    accessLPS25();

    // Pressure
    uint8_t buffer[] = {lps25RegAddr::PRESS_OUT_XL | (1 << 7)};

    uint8_t status = write(m_deviceFile, buffer, 1);
    if (status != 1) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Altimeter/Pressure)" << std::endl;
        return 0;
    }

    uint8_t outBuffer[3];
    status = read(m_deviceFile, outBuffer, 3);
    if (status != 3) {
        std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Altimeter/Pressure)" << std::endl;
    }

    uint8_t pxl = outBuffer[0];
    uint8_t pl = outBuffer[1];
    uint8_t ph = outBuffer[2];

    // combine bytes
    int32_t pressure_raw = (int32_t)ph << 16 | (uint16_t)pl << 8 | pxl;
    double pressure_bar = static_cast< double >(pressure_raw) / 4096;

    // converts pressure in mbar to altitude in meters, using 1976 US
    // Standard Atmosphere model (note that this formula only applies to a
    // height of 11 km, or about 36000 ft)
    //  If altimeter setting (QNH, barometric pressure adjusted to sea
    //  level) is given, this function returns an indicated altitude
    //  compensated for actual regional pressure; otherwise, it returns
    //  the pressure altitude above the standard pressure level of 1013.25
    //  mbar or 29.9213 inHg
    double altitude = (1 - pow(pressure_bar / 1013.25, 0.190263)) * 44330.8;

    opendlv::proxy::AltimeterReading altimeterReading(altitude);
    return altimeterReading;
}

opendlv::proxy::TemperatureReading PololuAltImu10Device::ReadTemperature() {
    accessLPS25();

    //Temperature
    uint8_t buffer[] = {lps25RegAddr::TEMP_OUT_L | (1 << 7)};
    uint8_t status = write(m_deviceFile, buffer, 1);
    if (status != 1) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Temperature)" << std::endl;
        return 0;
    }

    uint8_t outBuffer[2];
    status = read(m_deviceFile, outBuffer, 2);
    if (status != 2) {
        std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Temperature)" << std::endl;
    }

    uint8_t tl = outBuffer[0];
    uint8_t th = outBuffer[1];

    // combine bytes
    float temperature = 42.5 + ((int16_t)(th << 8 | tl)) / 480.0;

    opendlv::proxy::TemperatureReading temperatureReading(temperature);
    return temperatureReading;
}

opendlv::proxy::MagnetometerReading PololuAltImu10Device::ReadMagnetometer() {
    accessLIS3();
    uint8_t buffer[] = {lis3RegAddr::OUT_X_L | 0x80};

    uint8_t status = write(m_deviceFile, buffer, 1);
    if (status != 1) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Magnetometer)" << std::endl;
        return nullptr;
    }

    uint8_t outBuffer[6];
    status = read(m_deviceFile, outBuffer, 6);
    if (status != 6) {
        std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Magnetometer)" << std::endl;
    }

    uint8_t xlm = outBuffer[0];
    uint8_t xhm = outBuffer[1];
    uint8_t ylm = outBuffer[2];
    uint8_t yhm = outBuffer[3];
    uint8_t zlm = outBuffer[4];
    uint8_t zhm = outBuffer[5];

    int16_t x = (int16_t)(xhm << 8 | xlm);
    int16_t y = (int16_t)(yhm << 8 | ylm);
    int16_t z = (int16_t)(zhm << 8 | zlm);

    //F
    //FS=±4 gauss  --> 6842 LSB/gauss
    float scaledX = static_cast< double >(x) / 6842.0;
    float scaledY = static_cast< double >(y) / 6842.0;
    float scaledZ = static_cast< double >(z) / 6842.0;
    
    float reading[] = {scaledX, scaledY, scaledZ};
    if(!m_lockCalibration){
        CalibrateMagnetometer(reading);
    }

    Eigen::Vector3f rawReading(reading[0],reading[1],reading[2]);
    
    Eigen::Vector3f adjustedReading = Rotate(rawReading, m_rotationMatrix);
    adjustedReading.normalize();


    opendlv::proxy::MagnetometerReading magnetometerReading(reading);
    return magnetometerReading;
}

void PololuAltImu10Device::CalibrateMagnetometer(float* a_val)
{


    // std::cout << "Raw values: "<< a_val[0] << "," << a_val[1]<< "," <<a_val[2] <<  std::endl;
    for(uint8_t i = 0; i < 3; i++) {
        if(a_val[i] > m_magnetometerMaxVal[i]) {
            m_magnetometerMaxVal[i] = a_val[i];
        } else if(a_val[i] < m_magnetometerMinVal[i]) {
            m_magnetometerMinVal[i] = a_val[i];
        }
        //Hard iron calibration
        a_val[i] -= (m_magnetometerMinVal[i] + m_magnetometerMaxVal[i]) / 2.0f ;
        //Soft iron calibration
        // a_val[i]  = (a_val[i] - m_magnetometerMinVal[i]) / (m_magnetometerMaxVal[i] - m_magnetometerMinVal[i]) * 2 - 1;
    }
    // std::cout << "Calibrated values: "<< a_val[0] << "," << a_val[1]<< "," <<a_val[2] <<  std::endl;
    // std::cout << "Heading: " << 180 * atan2(a_val[1],a_val[0]) / M_PI << std::endl;

    //Tilt compensation
    // float roll = atan2(m_heavyAcc[1],m_heavyAcc[2]);
    // float pitch = atan2(-m_heavyAcc[0], m_heavyAcc[1]*sinf(roll)+m_heavyAcc[2]*cosf(roll));



    // a_val[0] = a_val[0]*cosf(pitch)+a_val[2]*sinf(pitch);
    // a_val[1] = a_val[0]*sinf(pitch)*sinf(roll) + a_val[1]*cosf(roll) - a_val[2]*sinf(roll)*cosf(pitch);
    // a_val[2] = -a_val[0]*cosf(roll)*sinf(pitch) + a_val[1]*sinf(roll) + a_val[2]*cosf(roll)*cosf(pitch);
    // std::cout << "Tilt compensation: "<< 180 * atan2(a_val[1],a_val[0]) / M_PI << " (Pitch, Roll): " << pitch << "," << roll <<std::endl;

}


opendlv::proxy::GyroscopeReading PololuAltImu10Device::ReadGyroscope() {
    accessLSM6();
    uint8_t buffer[] = {lsm6RegAddr::OUTX_L_G};

    uint8_t status = write(m_deviceFile, buffer, 1);
    if (status != 1) {
        std::cerr << "[Pololu Altimu] Failed to write to the i2c bus. (Gyroscope)" << std::endl;
        return nullptr;
    }

    uint8_t outBuffer[6];
    status = read(m_deviceFile, outBuffer, 6);
    if (status != 6) {
        std::cerr << "[Pololu Altimu] Failed to read to the i2c bus. (Gyroscope)" << std::endl;
    }

    uint8_t xla = outBuffer[0];
    uint8_t xha = outBuffer[1];
    uint8_t yla = outBuffer[2];
    uint8_t yha = outBuffer[3];
    uint8_t zla = outBuffer[4];
    uint8_t zha = outBuffer[5];

    int16_t x = xha;
    x = (x << 8) | xla;

    int16_t y = yha;
    y = (y << 8) | yla;

    int16_t z = zha;
    z = (z << 8) | zla;

    //a.x = ~a.x + 1;
    //a.y = ~a.y + 1;
    //a.z = ~a.z + 1;

    //FS = ±245 --> 8.75 mdps/LSB
    static double PI = 3.14159265359;
    //Calibration also needed.
    float scaledX = (((8.75 * static_cast< double >(x)) / 1000) / 180) * PI;
    float scaledY = (((8.75 * static_cast< double >(y)) / 1000) / 180) * PI;
    float scaledZ = (((8.75 * static_cast< double >(z)) / 1000) / 180) * PI;

    float reading[] = {scaledX, scaledY, scaledZ};
    opendlv::proxy::GyroscopeReading gyroscopeReading(reading);
    return gyroscopeReading;
}

bool PololuAltImu10Device::IsInitialized() const {
    return m_initialized;
}

Eigen::Vector3f PololuAltImu10Device::Rotate(Eigen::Vector3f a_v, Eigen::Matrix3d a_m)
{
    return (a_m*a_v.cast<double>()).cast<float>();
}


}
}
}
} // opendlv::core::system::proxy
