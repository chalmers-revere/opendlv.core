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

// Libraries required for i2c-bus
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <unistd.h>


#include "Pololualtimu10device.h"

#include "opendavinci/odcore/wrapper/Eigen.h"
#include "opendavinci/odcore/strings/StringToolbox.h"

namespace opendlv {
namespace core {
namespace system{
namespace proxy{

/**
 * Constructor for PololuAltImuV5 device interfacing through I2C.
 *
 */
PololuAltImu10Device::PololuAltImu10Device(std::string const &a_deviceName, 
  std::string const &a_adressType)
    : m_deviceFile()
    , m_addressType()
    , m_instrumentAdress()
    , m_initialized(false)
{
  m_deviceFile = open(a_deviceName.c_str(), O_RDWR);
  if (m_deviceFile < 0) {
    std::cerr << "[Pololu altIMU v5] Failed to open the i2c bus." << std::endl;
    return;
  }

  std::cout << "[Pololu altIMU v5] I2C bus " << a_deviceName 
      << " opened successfully." << std::endl;

  if (a_adressType.compare("high")) {
    m_addressType = a_adressType;
    m_instrumentAdress.push_back(0x1e);
    m_instrumentAdress.push_back(0x5d);
    m_instrumentAdress.push_back(0x6b);
  } else if (a_adressType.compare("low")) {
    m_addressType = a_adressType;
    m_instrumentAdress.push_back(0x1c);
    m_instrumentAdress.push_back(0x5c);
    m_instrumentAdress.push_back(0x6a);
  } else {
    std::cerr 
        << "[Pololu altIMU v5] Address type invalid. Must be either high xor low." 
        << std::endl; 
  }

  initLSM6();
  initLIS3();
  initLPS25();

  m_initialized = true;
}


PololuAltImu10Device::~PololuAltImu10Device() {}

void PololuAltImu10Device::accessLSM6()
{
  uint8_t address = m_instrumentAdress.at(2);
  if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
    std::cerr << "[Pololu altIMU v5] Failed to acquire bus access or talk to slave"
        << "device. (LSM6 - Accelerometer and Gyroscope)" << std::endl;
    return;
  }
}

void PololuAltImu10Device::accessLIS3()
{
  uint8_t address = m_instrumentAdress.at(0);
  if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
    std::cerr << "[Pololu altIMU v5] Failed to acquire bus access or talk to slave"
        << "device. (LIS3 - Magnetometer)" << std::endl;
    return;
  }
}

void PololuAltImu10Device::accessLPS25()
{
  uint8_t address = m_instrumentAdress.at(1);
  if (ioctl(m_deviceFile, I2C_SLAVE, address) < 0) {
    std::cerr << "[Pololu altIMU v5] Failed to acquire bus access or talk to slave"
        << "device. (LPS25 - Pressure)" << std::endl;
    return;
  }
}


void PololuAltImu10Device::initLSM6()
{
  accessLSM6();
  // --------------- Accelerometer
  // CTRL1_XL : ODR_XL3, ODR_XL2, ODR_XL1, ODR_XL0, FS_XL1, FS_XL0, BW_XL1, 
  // BW_XL0
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

void PololuAltImu10Device::initLIS3()
{
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

}

void PololuAltImu10Device::initLPS25()
{
    accessLPS25();
    // 0xB0 = 0b10110000
    // PD = 1 (active mode);
    // ODR = 011 (12.5 Hz pressure & temperature output data rate)
    I2cWriteRegister(lps25RegAddr::CTRL_REG1, 0xB0);
}

void PololuAltImu10Device::I2cWriteRegister(uint8_t a_register, uint8_t a_value)
{
  uint8_t buffer[2];
  buffer[0] = a_register;
  buffer[1] = a_value;

  uint8_t status = write(m_deviceFile, buffer, 2);

  if (status != 2) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus at register:" 
        << a_register << ". status code: " << status << std::endl;
    std::cout << errno;
    return;
  }
}

std::vector<float> PololuAltImu10Device::GetAcceleration()
{
  accessLSM6();
  uint8_t buffer[1];
  buffer[0] = lsm6RegAddr::OUTX_L_XL;

  uint8_t status = write(m_deviceFile, buffer, 1);
  if (status != 1) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus. " 
      << "(Accelerometer)" << std::endl;
  }

  uint8_t outBuffer[6];
  status = read(m_deviceFile, outBuffer, 6);
  if (status != 6) {
    std::cerr << "[Pololu altIMU v5] Failed to read to the i2c bus. "
        << "(Accelerometer)" << std::endl;
  }

  uint8_t xla = outBuffer[0];
  uint8_t xha = outBuffer[1];
  uint8_t yla = outBuffer[2];
  uint8_t yha = outBuffer[3];
  uint8_t zla = outBuffer[4];
  uint8_t zha = outBuffer[5];

  int16_t x = xha;
  int16_t y = yha;
  int16_t z = zha;

  x = (x << 8) | xla;
  y = (y << 8) | yla;
  z = (z << 8) | zla;

  //FS = ±2 --> 0.061 mg/LSB -->
  const double GRAVITY_ACC= -9.82;
  const double CONSTANT = 0.061 / 1000 * GRAVITY_ACC;
  
  float scaledX = CONSTANT * static_cast<double>(x);
  float scaledY = CONSTANT * static_cast<double>(y);
  float scaledZ = CONSTANT * static_cast<double>(z);

  return std::vector<float>{scaledX, scaledY, scaledZ};
}

opendlv::proxy::AccelerometerReading PololuAltImu10Device::ReadAccelerometer()
{
  std::vector<float> reading = GetAcceleration();
  opendlv::proxy::AccelerometerReading accelerometerReading(
      reading.at(0), reading.at(1), reading.at(2));
  return accelerometerReading;
}

float PololuAltImu10Device::GetPressure() {
  accessLPS25();
  // Pressure
  uint8_t buffer[] = {lps25RegAddr::PRESS_OUT_XL | (1 << 7)};

  uint8_t status = write(m_deviceFile, buffer, 1);
  if (status != 1) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus. (Altimeter)" 
        << std::endl;
    return 0;
  }

  uint8_t outBuffer[3];
  status = read(m_deviceFile, outBuffer, 3);
  if (status != 3) {
    std::cerr << "[Pololu altIMU v5] Failed to read to the i2c bus. (Altimeter)" 
        << std::endl;
  }

  uint8_t pxl = outBuffer[0];
  uint8_t pl = outBuffer[1];
  uint8_t ph = outBuffer[2];

  // combine bytes
  int32_t pressure_raw = (int8_t)ph << 16 | (uint16_t)pl << 8 | pxl;
  float pressure_bar = pressure_raw/4096.0f;

  // converts pressure in mbar to altitude in meters, using 1976 US
  // Standard Atmosphere model (note that this formula only applies to a
  // height of 11 km, or about 36000 ft)
  //  If altimeter setting (QNH, barometric pressure adjusted to sea
  //  level) is given, this function returns an indicated altitude
  //  compensated for actual regional pressure; otherwise, it returns
  //  the pressure altitude above the standard pressure level of 1013.25
  //  mbar or 29.9213 inHg
  // float altitude = 
  //     static_cast<float>((1 - pow(pressure_bar / 1013.25, 0.190263)) * 44330.8);
  return pressure_bar;
}

opendlv::proxy::BarometerReading PololuAltImu10Device::ReadBarometer()
{
  float pressure = GetPressure();
  opendlv::proxy::BarometerReading barometerReading(pressure);
  return barometerReading;
}

float PololuAltImu10Device::GetTemperature()
{
  accessLPS25();

  //Temperature
  uint8_t buffer[] = {lps25RegAddr::TEMP_OUT_L | (1 << 7)};
  uint8_t status = write(m_deviceFile, buffer, 1);
  if (status != 1) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus. (Temperature)"
        << std::endl;
    return 0;
  }

  uint8_t outBuffer[2];
  status = read(m_deviceFile, outBuffer, 2);
  if (status != 2) {
    std::cerr << "[Pololu altIMU v5] Failed to read to the i2c bus. (Temperature)"
        << std::endl;
  }

  uint8_t tl = outBuffer[0];
  uint8_t th = outBuffer[1];

  // combine bytes and conversion to celsius
  float temperature = 42.5 + ((int16_t)(th << 8 | tl)) / 480.0;
  return temperature;
}

opendlv::proxy::ThermometerReading PololuAltImu10Device::ReadThermometer()
{
  float temperature = GetTemperature();
  opendlv::proxy::ThermometerReading thermometerReading(temperature);
  return thermometerReading;
}

std::vector<float> PololuAltImu10Device::GetMagneticField() {
  accessLIS3();
  uint8_t buffer[] = {lis3RegAddr::OUT_X_L | 0x80};

  uint8_t status = write(m_deviceFile, buffer, 1);
  if (status != 1) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus. "
        << "(Magnetometer)" << std::endl;
  }

  uint8_t outBuffer[6];
  status = read(m_deviceFile, outBuffer, 6);
  if (status != 6) {
    std::cerr << "[Pololu altIMU v5] Failed to read to the i2c bus. (Magnetometer)"
        << std::endl;
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
  float scaledX = x / 6842.0f;
  float scaledY = y / 6842.0f;
  float scaledZ = z / 6842.0f;
  return std::vector<float>{scaledX,scaledY,scaledZ};
}

opendlv::proxy::MagnetometerReading PololuAltImu10Device::ReadMagnetometer()
{
  std::vector<float> reading = GetMagneticField();
  opendlv::proxy::MagnetometerReading magnetometerReading(reading.at(0),
      reading.at(1), reading.at(2));
  return magnetometerReading;
}

std::vector<float> PololuAltImu10Device::GetAngularVelocity()
{
  accessLSM6();
  uint8_t buffer[] = {lsm6RegAddr::OUTX_L_G};

  uint8_t status = write(m_deviceFile, buffer, 1);
  if (status != 1) {
    std::cerr << "[Pololu altIMU v5] Failed to write to the i2c bus. (Gyroscope)"
        << std::endl;
  }

  uint8_t outBuffer[6];
  status = read(m_deviceFile, outBuffer, 6);
  if (status != 6) {
    std::cerr << "[Pololu altIMU v5] Failed to read to the i2c bus. (Gyroscope)"
        << std::endl;
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

  const double CONSTANT = 8.75 * M_PI / (1000 * 180);

  float scaledX = CONSTANT * static_cast<double>(x);
  float scaledY = CONSTANT * static_cast<double>(y);
  float scaledZ = CONSTANT * static_cast<double>(z);
  return std::vector<float> {scaledX,scaledY,scaledZ}; 
}


opendlv::proxy::GyroscopeReading PololuAltImu10Device::ReadGyroscope()
{
  std::vector<float> reading = GetAngularVelocity();
  opendlv::proxy::GyroscopeReading gyroscopeReading(reading.at(0), 
      reading.at(1), reading.at(2));
  return gyroscopeReading;
}

bool PololuAltImu10Device::IsInitialized() const
{
  return m_initialized;
}

}
}
}
} // opendlv::core::system::proxy
