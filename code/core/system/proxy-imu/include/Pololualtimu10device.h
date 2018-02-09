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
#include <vector>


#include <opendavinci/odcore/wrapper/Eigen.h>

#include "odvdopendlvstandardmessageset/GeneratedHeaders_ODVDOpenDLVStandardMessageSet.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class PololuAltImu10Device {
   public:
    PololuAltImu10Device(std::string const &, std::string const &, std::vector<double> const &, uint32_t const &, std::string const &, bool const &, bool &);

    PololuAltImu10Device(PololuAltImu10Device const &) = delete;

    PololuAltImu10Device &operator=(PololuAltImu10Device const &) = delete;

    virtual ~PololuAltImu10Device();

    void saveCalibrationFile();

    std::vector<float> GetAcceleration();

    opendlv::proxy::AccelerationReading ReadAccelerometer();

    float GetAltitude();

    opendlv::proxy::AltitudeReading ReadAltimeter();

    float GetTemperature();

    opendlv::proxy::TemperatureReading ReadTemperature();

    std::vector<float> GetMagneticField();

    opendlv::proxy::MagneticFieldReading ReadMagnetometer();

    std::vector<float> GetAngularVelocity();

    opendlv::proxy::AngularVelocityReading ReadGyroscope();

    bool IsInitialized() const;

   private:
    void I2cWriteRegister(uint8_t, uint8_t);

    void initCalibration();
    
    bool loadCalibrationFile();

    void accessLSM6();

    void accessLIS3();

    void accessLPS25();

    void initLSM6();

    void initLIS3();

    void initLPS25();

    void CalibrateMagnetometer(std::vector<float>*);

    void CalibrateAccelerometer(std::vector<float>*);

    void CalibrateGyroscope(std::vector<float>*);

    Eigen::Vector3f Rotate(Eigen::Vector3f, Eigen::Matrix3d);

    int16_t m_deviceFile;

    std::string m_addressType;

    uint8_t m_instrumentAdress[3];

    Eigen::Matrix3d m_rotationMatrix;

    std::string m_calibrationFile;

    bool const m_lockCalibration;

    float m_accelerometerMaxVal[3];

    float m_accelerometerMinVal[3];

    float m_magnetometerMaxVal[3];

    float m_magnetometerMinVal[3];

    float m_gyroscopeAvgVal[3];

    uint32_t const m_calibrationNumberOfSamples;

    bool m_initialized;

    bool m_debug;
};

namespace lis3RegAddr {
enum RegAddr {
    WHO_AM_I = 0x0F,

    CTRL_REG1 = 0x20,
    CTRL_REG2 = 0x21,
    CTRL_REG3 = 0x22,
    CTRL_REG4 = 0x23,
    CTRL_REG5 = 0x24,

    STATUS_REG = 0x27,
    OUT_X_L = 0x28,
    OUT_X_H = 0x29,
    OUT_Y_L = 0x2A,
    OUT_Y_H = 0x2B,
    OUT_Z_L = 0x2C,
    OUT_Z_H = 0x2D,
    TEMP_OUT_L = 0x2E,
    TEMP_OUT_H = 0x2F,
    INT_CFG = 0x30,
    INT_SRC = 0x31,
    INT_THS_L = 0x32,
    INT_THS_H = 0x33,
};
}
namespace lsm6RegAddr {
enum RegAddr {
    FUNC_CFG_ACCESS = 0x01,

    FIFO_CTRL1 = 0x06,
    FIFO_CTRL2 = 0x07,
    FIFO_CTRL3 = 0x08,
    FIFO_CTRL4 = 0x09,
    FIFO_CTRL5 = 0x0A,
    ORIENT_CFG_G = 0x0B,

    INT1_CTRL = 0x0D,
    INT2_CTRL = 0x0E,
    WHO_AM_I = 0x0F,
    CTRL1_XL = 0x10,
    CTRL2_G = 0x11,
    CTRL3_C = 0x12,
    CTRL4_C = 0x13,
    CTRL5_C = 0x14,
    CTRL6_C = 0x15,
    CTRL7_G = 0x16,
    CTRL8_XL = 0x17,
    CTRL9_XL = 0x18,
    CTRL10_C = 0x19,

    WAKE_UP_SRC = 0x1B,
    TAP_SRC = 0x1C,
    D6D_SRC = 0x1D,
    STATUS_REG = 0x1E,

    OUT_TEMP_L = 0x20,
    OUT_TEMP_H = 0x21,
    OUTX_L_G = 0x22,
    OUTX_H_G = 0x23,
    OUTY_L_G = 0x24,
    OUTY_H_G = 0x25,
    OUTZ_L_G = 0x26,
    OUTZ_H_G = 0x27,
    OUTX_L_XL = 0x28,
    OUTX_H_XL = 0x29,
    OUTY_L_XL = 0x2A,
    OUTY_H_XL = 0x2B,
    OUTZ_L_XL = 0x2C,
    OUTZ_H_XL = 0x2D,

    FIFO_STATUS1 = 0x3A,
    FIFO_STATUS2 = 0x3B,
    FIFO_STATUS3 = 0x3C,
    FIFO_STATUS4 = 0x3D,
    FIFO_DATA_OUT_L = 0x3E,
    FIFO_DATA_OUT_H = 0x3F,
    TIMESTAMP0_REG = 0x40,
    TIMESTAMP1_REG = 0x41,
    TIMESTAMP2_REG = 0x42,

    STEP_TIMESTAMP_L = 0x49,
    STEP_TIMESTAMP_H = 0x4A,
    STEP_COUNTER_L = 0x4B,
    STEP_COUNTER_H = 0x4C,

    FUNC_SRC = 0x53,

    TAP_CFG = 0x58,
    TAP_THS_6D = 0x59,
    INT_DUR2 = 0x5A,
    WAKE_UP_THS = 0x5B,
    WAKE_UP_DUR = 0x5C,
    FREE_FALL = 0x5D,
    MD1_CFG = 0x5E,
    MD2_CFG = 0x5F,
};
}
namespace lps25RegAddr {
enum regAddr {
    REF_P_XL = 0x08,
    REF_P_L = 0x09,
    REF_P_H = 0x0A,

    WHO_AM_I = 0x0F,

    RES_CONF = 0x10,

    CTRL_REG1 = 0x20,
    CTRL_REG2 = 0x21,
    CTRL_REG3 = 0x22,
    CTRL_REG4 = 0x23, // 25H

    STATUS_REG = 0x27,

    PRESS_OUT_XL = 0x28,
    PRESS_OUT_L = 0x29,
    PRESS_OUT_H = 0x2A,

    TEMP_OUT_L = 0x2B,
    TEMP_OUT_H = 0x2C,

    FIFO_CTRL = 0x2E,   // 25H
    FIFO_STATUS = 0x2F, // 25H

    RPDS_L = 0x39, // 25H
    RPDS_H = 0x3A, // 25H

    LPS25H_INTERRUPT_CFG = 0x24,
    LPS25H_INT_SOURCE = 0x25,
    LPS25H_THS_P_L = 0x30,
    LPS25H_THS_P_H = 0x31,
};
}
}
}
}
} // opendlv::core::system::proxy

#endif
