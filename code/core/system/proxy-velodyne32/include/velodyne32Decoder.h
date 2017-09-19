/**
 * Velodyne32Decoder is used to decode HDL-32E data realized with OpenDaVINCI
 * Copyright (C) 2017 Hang Yin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef VELODYNE32DECODER_H_
#define VELODYNE32DECODER_H_

#include <cmath>
#include <memory>
#include <array>

#include "opendavinci/generated/odcore/data/SharedPointCloud.h"
#include "opendavinci/generated/odcore/data/CompactPointCloud.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include <opendavinci/odcore/io/StringListener.h>
#include "automotivedata/generated/cartesian/Constants.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
using namespace odcore::wrapper;

// This class will handle bytes received via a UDP socket.
class Velodyne32Decoder : public odcore::io::StringListener {
   private:
    /**
         * "Forbidden" copy constructor. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the copy constructor.
         */
    Velodyne32Decoder(const Velodyne32Decoder &);

    /**
         * "Forbidden" assignment operator. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the assignment operator.
         */
    Velodyne32Decoder &operator=(const Velodyne32Decoder &);

   public:
    /**
         * Constructor.
     * @param m shared memory for SPC
     * @param c container conference 
     * @param s name of the calibration file
     * @param withCPC if CPC is included together with SPC
     * @param SPCOption polar or cartesian coordinate
     * @param CPCIntensityOption with or without intensity
     * @param numberOfBitsForIntensity number of bits reserved for intensity
     * @param intensityPlacement higher or lower bits for intensity
     * @param distanceEncoding use cm or 2mm for distance encoding
     */  
    Velodyne32Decoder(const std::shared_ptr< SharedMemory > m,
odcore::io::conference::ContainerConference &c, const string &s, const bool &withCPC, const uint8_t &SPCOption, const uint8_t &CPCIntensityOption, const uint8_t &numberOfBitsForIntensity, const uint8_t &intensityPlacement, const uint8_t &distanceEncoding);
    
    /**
         * Constructor.
     * @param c container conference 
     * @param s name of the calibration file
     * @param CPCIntensityOption with or without intensity
     * @param numberOfBitsForIntensity number of bits reserved for intensity
     * @param intensityPlacement higher or lower bits for intensity
     * @param distanceEncoding use cm or 2mm for distance encoding
     */
    Velodyne32Decoder(odcore::io::conference::ContainerConference &c, const string &s, const uint8_t &CPCIntensityOption, const uint8_t &numberOfBitsForIntensity, const uint8_t &intensityPlacement, const uint8_t &distanceEncoding);

    virtual ~Velodyne32Decoder();

    virtual void nextString(const std::string &s);

   private:
    void readCalibrationFile();
    void index32sensorIDs();
    void setupIntensityMaskCPC(uint8_t &, uint8_t &);
    void sendCPC(bool);
    void sendPointCloud();
   private:
    const uint32_t m_MAX_POINT_SIZE = 70000; //the maximum number of points per frame. This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed.
    const uint32_t m_SIZE_PER_COMPONENT = sizeof(float);
    const uint8_t m_NUMBER_OF_COMPONENTS_PER_POINT = 4;  //4 components per vector: (1) cartesian: xyz+intensity; (2) polar: distance+azimuth+vertical angle+intensity
    const uint32_t m_SIZE = m_MAX_POINT_SIZE * m_NUMBER_OF_COMPONENTS_PER_POINT * m_SIZE_PER_COMPONENT; //the total size of the shared memory

    uint8_t m_SPCOption; //0: xyz+intensity; 1: distance+azimuth+vertical angle+intensity
    uint8_t m_CPCIntensityOption; //Only used when CPC is enabled. 0: without intensity; 1: with intensity; 2: send a CPC container twice, one with intensity, and the other without intensity
    uint8_t m_numberOfBitsForIntensity; //Range 0-7. Only used when CPC is enabled
    uint8_t m_intensityPlacement;  //0: higher bits; 1: lower bits
    uint16_t m_mask;  //for combining distance and intensity in 16 bits
    uint8_t m_distanceEncoding; //0: cm; 1: 2mm
    uint32_t m_pointIndexSPC; //current number of points of the current frame for shared point cloud 
    uint32_t m_pointIndexCPC; //current number of points of the current frame for compact point cloud
    uint32_t m_startID;
    float m_previousAzimuth;
    float m_currentAzimuth;
    float m_distance;
    std::shared_ptr< SharedMemory > m_velodyneSharedMemory; //shared memory for shared point cloud
    float *m_segment;  //temporary memory for transferring data of each frame to the shared memory
    odcore::io::conference::ContainerConference &m_conference;
    odcore::data::SharedPointCloud m_spc; //shared point cloud
    std::array<float, 32> m_verticalAngle; //Vertical angle of each sensor beam
    string m_calibration;  //name of the calibration file for HDL-32E
    const float toRadian = static_cast<float>(M_PI) / 180.0f;  //degree to radian
    bool m_withSPC;  //if SPC is expected
    bool m_withCPC;  //if CPC is expected
        
    //For compact point cloud:
    float m_startAzimuth;
    const uint8_t m_ENTRIES_PER_AZIMUTH = 11;//For HDL-32E, there are 32 points per azimuth
    std::stringstream m_distanceStringStreamNoIntensityPart1; //Layer 0, 1, 4, 7..., i.e., in addition to Layer 0, every 3rd layer from Layer 1 and resulting in 12 layers, without intensity
    std::stringstream m_distanceStringStreamNoIntensityPart2; //Layer 2, 3, 6, 9..., i.e., in addition to Layer 2, every 3rd layer from Layer 3 and resulting in 11 layers, without intensity
    std::stringstream m_distanceStringStreamNoIntensityPart3; //Layer 5, 8, 11..., i.e., every 3rd layer from Layer 5 and resulting in 9 layers, without intensity
    std::stringstream m_distanceStringStreamWithIntensityPart1; //Similar to m_distanceStringStreamNoIntensityPart1, with intensity
    std::stringstream m_distanceStringStreamWithIntensityPart2; //Similar to m_distanceStringStreamNoIntensityPart2, with intensity
    std::stringstream m_distanceStringStreamWithIntensityPart3; //Similar to m_distanceStringStreamNoIntensityPart3, with intensity
    bool m_isStartAzimuth;  //Indicate if an azimuth is the starting azimuth of a new frame
    std::array<uint8_t, 32> m_sensorOrderIndex; //Specify the sensor ID order for each 32 points with increasing vertical angle for CPC and SPC
    std::array<uint16_t, 32> m_32SensorsNoIntensity; //Store the distance values of the current 32 sensors for CPC without intensity
    std::array<uint16_t, 32> m_32SensorsWithIntensity; //Store the distance values of the current 32 sensors for CPC with intensity
};
}
}
}
} // opendlv::core::system::proxy
#endif /*VELODYNE32DECODER_H_*/
