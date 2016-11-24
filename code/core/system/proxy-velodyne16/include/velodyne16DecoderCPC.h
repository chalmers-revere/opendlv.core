/**
 * Velodyne16DecoderCPC is used to decode VLP-16 data and send Compact Point Cloud realized with OpenDaVINCI
 * Copyright (C) 2016 Hang Yin
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

#ifndef VELODYNE16DECODERCPC_H_
#define VELODYNE16DECODERCPC_H_

#include <sstream>

#include "opendavinci/odcore/data/Container.h"
#include <opendavinci/odcore/io/StringListener.h>
#include "opendavinci/generated/odcore/data/CompactPointCloud.h"
#include "opendavinci/odcore/wrapper/half_float.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
using namespace odcore::wrapper;

// This class will handle bytes received via a UDP socket.
class Velodyne16DecoderCPC : public odcore::io::StringListener {
   private:
    /**
         * "Forbidden" copy constructor. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the copy constructor.
         */
    Velodyne16DecoderCPC(const Velodyne16DecoderCPC &);

    /**
         * "Forbidden" assignment operator. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the assignment operator.
         */
    Velodyne16DecoderCPC &operator=(const Velodyne16DecoderCPC &);

   public:
    /**
         * Constructor.
         */
    Velodyne16DecoderCPC(odcore::io::conference::ContainerConference &);

    virtual ~Velodyne16DecoderCPC();

    virtual void nextString(const std::string &s);

   private:
    void sendCompactPointCloud(const float &oldAzimuth, const float &newAzimuth);
   private:
    const uint32_t m_MAX_POINT_SIZE = 30000; //the maximum number of points per frame. This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed.

    uint32_t m_pointIndex;
    float m_previousAzimuth;
    float m_deltaAzimuth;
    float m_distance;
    odcore::io::conference::ContainerConference &m_velodyneContainer;
    float m_startAzimuth;
    const uint8_t m_ENTRIES_PER_AZIMUTH = 16;//For VLP-16, there are 16 points per azimuth
    std::stringstream m_distanceStringStream; //The string stream with distance values for all points of one frame
    bool m_isStartAzimuth;  //Indicate if an azimuth is the starting azimuth of a new frame
    uint8_t m_sensorOrderIndex[16];//Specify the order for each 16 points in the string with distance values
    half m_16Sensors[16];//Store the distance values of the current 16 sensors
};
}
}
}
} // opendlv::core::system::proxy
#endif /*VELODYNE16DECODERCPC_H_*/
