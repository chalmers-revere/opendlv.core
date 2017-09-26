/**
 * Velodyne64Decoder is used to decode Velodyne HDL-64E data realized with OpenDaVINCI
 * Copyright (C) 2016 Hang Yin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <cmath>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <array>

#include "opendavinci/GeneratedHeaders_OpenDaVINCI.h"
#include "opendavinci/generated/odcore/data/SharedPointCloud.h"
#include "opendavinci/odcore/base/Lock.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"

#include "velodyne64Decoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

Velodyne64Decoder::Velodyne64Decoder(const std::shared_ptr< SharedMemory > m,
odcore::io::conference::ContainerConference &c, const string &s)
    : m_pointIndex(0)
    , m_startID(0)
    , m_previousAzimuth(0.0)
    , m_upperBlock(true)
    , m_distance(0.0)
    , m_velodyneSharedMemory(m)
    , m_segment(NULL)
    , m_conference(c)
    , m_spc()
    , m_rotCorrection()
    , m_vertCorrection()
    , m_distCorrection()
    , m_vertOffsetCorrection()
    , m_horizOffsetCorrection()
    , m_calibration(s) {
    //Initial setup of the shared point cloud (N.B. The size and width of the shared point cloud depends on the number of points of a frame, hence they are not set up in the constructor)
    m_spc.setName(m_velodyneSharedMemory->getName()); // Name of the shared memory segment with the data.
    m_spc.setHeight(1); // We have just a sequence of vectors.
    m_spc.setNumberOfComponentsPerPoint(m_NUMBER_OF_COMPONENTS_PER_POINT);
    m_spc.setComponentDataType(SharedPointCloud::FLOAT_T); // Data type per component.
    m_spc.setUserInfo(SharedPointCloud::XYZ_INTENSITY);

    //Create memory for temporary storage of point cloud data for each frame
    m_segment = (float *)malloc(m_SIZE);

    //Load calibration data from the calibration file
    string line;
    ifstream in(m_calibration);
    if (!in.is_open()){
        cout << "Calibration file not found." << endl;
    }
    std::array<int, 5> counter = {{0, 0, 0, 0, 0}}; //corresponds to the index of the five calibration values
    std::array<bool, 5> found = {{false, false, false, false, false}};

    while (getline(in, line)) {
        string tmp; // strip whitespaces from the beginning
        for (unsigned int i = 0; i < line.length(); i++) {

            if ((line[i] == '\t' || line[i] == ' ') && tmp.size() == 0) {
            } else {
                if (line[i] == '<') {
                    if (found[0]) {
                        m_rotCorrection[counter[0]] = atof(tmp.c_str());
                        counter[0]++;
                        found[0] = false;
                        continue;
                    }

                    if (found[1]) {
                        m_vertCorrection[counter[1]] = atof(tmp.c_str());
                        counter[1]++;
                        found[1] = false;
                        continue;
                    }

                    if (found[2]) {
                        m_distCorrection[counter[2]] = atof(tmp.c_str());
                        counter[2]++;
                        found[2] = false;
                        continue;
                    }

                    if (found[3]) {
                        m_vertOffsetCorrection[counter[3]] = atof(tmp.c_str());
                        counter[3]++;
                        found[3] = false;
                        continue;
                    }

                    if (found[4]) {
                        m_horizOffsetCorrection[counter[4]] = atof(tmp.c_str());
                        counter[4]++;
                        found[4] = false;
                        continue;
                    }
                    tmp += line[i];
                } else {
                    tmp += line[i];
                }
            }

            if (tmp == "<rotCorrection_>") {
                found[0] = true;
                tmp = "";
            } else if (tmp == "<vertCorrection_>") {
                found[1] = true;
                tmp = "";
            } else if (tmp == "<distCorrection_>") {
                found[2] = true;
                tmp = "";
            } else if (tmp == "<vertOffsetCorrection_>") {
                found[3] = true;
                tmp = "";
            } else if (tmp == "<horizOffsetCorrection_>") {
                found[4] = true;
                tmp = "";
            } else {
            }
        }
    }
}

Velodyne64Decoder::~Velodyne64Decoder() {
    free(m_segment);
}

float Velodyne64Decoder::toRadian(float angle) {
    return angle * static_cast<float>(M_PI) / 180.0f;
}

//Update the shared point cloud when a complete scan is completed.
void Velodyne64Decoder::sendSharedPointCloud(const float &oldAzimuth, const float &newAzimuth) {
    if (newAzimuth < oldAzimuth) {
        if (m_velodyneSharedMemory->isValid()) {
            Lock l(m_velodyneSharedMemory);
            memcpy(m_velodyneSharedMemory->getSharedMemory(), m_segment, m_SIZE);
            //Set the size and width of the shared point cloud of the current frame
            m_spc.setSize(m_SIZE); // Size in raw bytes.
            m_spc.setWidth(m_pointIndex);                                                      // Number of points.
            Container c(m_spc);
            m_conference.send(c);
        }
        m_pointIndex = 0;
        m_startID = 0;
    }
}

void Velodyne64Decoder::nextString(const string &payload) {
    if (payload.length() == 1206) {
        //Decode HDL-64E data
        uint32_t position = 0; //position specifies the starting position to read from the 1206 bytes

        //The payload of a HDL-64E packet consists of 12 blocks with 100 bytes each. Decode each block separately.
        static uint8_t firstByte, secondByte;
        static uint32_t dataValue;
        for (int index = 0; index < 12; index++) {
            //Decode header information: 2 bytes
            firstByte = (uint8_t)(payload.at(position));
            secondByte = (uint8_t)(payload.at(position + 1));
            dataValue = ntohs(firstByte * 256 + secondByte);
            if (dataValue == 0xddff) {
                m_upperBlock = false; //Lower block
            } else {
                m_upperBlock = true; //upper block
            }

            //Decode rotational information: 2 bytes
            firstByte = (uint8_t)(payload.at(position + 2));
            secondByte = (uint8_t)(payload.at(position + 3));
            dataValue = ntohs(firstByte * 256 + secondByte);
            float azimuth = static_cast< float >(dataValue / 100.0);
            sendSharedPointCloud(m_previousAzimuth, azimuth); //Send a complete scan as one frame
            m_previousAzimuth = azimuth;
            position += 4;

            if (m_pointIndex < m_MAX_POINT_SIZE) {
                //Decode distance information and intensity of each sensor in a block
                for (int counter = 0; counter < 32; counter++) {
                    //Decode distance: 2 bytes
                    static uint8_t sensorID(0);
                    if (m_upperBlock){
                        sensorID = counter;
                    }
                    else{
                        sensorID = counter + 32;
                    }
                    firstByte = (uint8_t)(payload.at(position));
                    secondByte = (uint8_t)(payload.at(position + 1));
                    dataValue = ntohs(firstByte * 256 + secondByte);
                    m_distance = static_cast< float >(dataValue * 0.2f / 100.0f) + m_distCorrection[sensorID] / 100.0f;
                    if (m_distance > 1.0f) {
                        static float xyDistance, xData, yData, zData, intensity;
                        xyDistance = m_distance * cos(toRadian(m_vertCorrection[sensorID]));
                        xData = xyDistance * sin(toRadian(azimuth - m_rotCorrection[sensorID])) - m_horizOffsetCorrection[sensorID] / 100.0f * cos(toRadian(azimuth - m_rotCorrection[sensorID]));
                        yData = xyDistance * cos(toRadian(azimuth - m_rotCorrection[sensorID])) + m_horizOffsetCorrection[sensorID] / 100.0f * sin(toRadian(azimuth - m_rotCorrection[sensorID]));
                        zData = m_distance * sin(toRadian(m_vertCorrection[sensorID])) + m_vertOffsetCorrection[sensorID] / 100.0f;
                        //Decode intensity: 1 byte
                        uint8_t intensityInt = (uint8_t)(payload.at(position + 2));
                        intensity = (float)intensityInt;

                        //Store coordinate information of each point to the malloc memory
                        m_segment[m_startID] = xData;
                        m_segment[m_startID + 1] = yData;
                        m_segment[m_startID + 2] = zData;
                        m_segment[m_startID + 3] = intensity;

                        m_pointIndex++;
                        m_startID += m_NUMBER_OF_COMPONENTS_PER_POINT;
                    }
                    position += 3;

                    if (m_pointIndex >= m_MAX_POINT_SIZE) {
                        position += 3 * (31 - counter); //Discard the points of the current frame when the preallocated shared memory is full; move the position to be read in the 1206 bytes
                        break;
                    }
                }
            } else {
                position += 96; //32*3
            }
        }
    }
}
}
}
}
} // opendlv::core::system::proxy
