/**
 * Velodyne16Decoder is used to decode VLP-16 data realized with OpenDaVINCI
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

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "opendavinci/generated/odcore/data/SharedPointCloud.h"
#include "opendavinci/odcore/base/Lock.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"

#include "velodyne16Decoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

Velodyne16Decoder::Velodyne16Decoder(const std::shared_ptr< SharedMemory > m,
odcore::io::conference::ContainerConference &c, const string &s)
    : m_pointIndex(0)
    , m_startID(0)
    , m_previousAzimuth(0.0)
    , m_currentAzimuth(0.0)
    , m_nextAzimuth(0.0)
    , m_deltaAzimuth(0.0)
    , m_distance(0.0)
    , m_velodyneSharedMemory(m)
    , m_segment(NULL)
    , m_velodyneContainer(c)
    , m_spc()
    , m_calibration(s)
    , firstPacket(false)
    , receiveFirstPacket(0)
    , sendFrame(0) {
    //Initial setup of the shared point cloud (N.B. The size and width of the shared point cloud depends on the number of points of a frame, hence they are not set up in the constructor)
    m_spc.setName(m_velodyneSharedMemory->getName()); // Name of the shared memory segment with the data.
    m_spc.setHeight(1); // We have just a sequence of vectors.
    m_spc.setNumberOfComponentsPerPoint(m_NUMBER_OF_COMPONENTS_PER_POINT);
    m_spc.setComponentDataType(SharedPointCloud::FLOAT_T); // Data type per component.
    m_spc.setUserInfo(SharedPointCloud::XYZ_INTENSITY);

    //Create memory for temporary storage of point cloud data for each frame
    m_segment = (float *)malloc(m_SIZE);

    //Load calibration data from the calibration file
    //VLP-16 has 16 channels/sensors. Each sensor has a specific vertical angle, which can be read from
    //m_vertCorrection[sensor ID] is specified in the calibration file.
    string line;
    ifstream in(m_calibration);
    if (!in.is_open()){
        cout << "Calibration file not found." << endl;
    }
    uint8_t counter = 0; //corresponds to the index of the vertical angle of each beam
    bool found = false;

    while (getline(in, line) && counter < 16) {
        string tmp; // strip whitespaces from the beginning
        for (uint8_t i = 0; i < line.length(); i++) {

            if ((line[i] == '\t' || line[i] == ' ') && tmp.size() == 0) {
            } else {
                if (line[i] == '<') {

                    if (found) {
                        m_vertCorrection[counter] = atof(tmp.c_str());

                        counter++;
                        found = false;
                        continue;
                    }
                    tmp += line[i];
                } else {
                    tmp += line[i];
                }
            }

            if (tmp == "<vertCorrection_>") {
                found = true;
                tmp = "";
            } else {
            }
        }
    }
}

Velodyne16Decoder::~Velodyne16Decoder() {
    free(m_segment);
}

//Update the shared point cloud when a complete scan is completed.
void Velodyne16Decoder::sendSharedPointCloud() {
    TimeStamp t2;
    sendFrame=t2.toMicroseconds();
    int64_t processTime=sendFrame-receiveFirstPacket;
    cout<<processTime<<endl;
    firstPacket=false;
    
    if (m_velodyneSharedMemory->isValid()) {
        Lock l(m_velodyneSharedMemory);
        memcpy(m_velodyneSharedMemory->getSharedMemory(), m_segment, m_SIZE);
        //Set the size and width of the shared point cloud of the current frame
        m_spc.setSize(m_SIZE); // Size in raw bytes.
        m_spc.setWidth(m_pointIndex);                                                      // Number of points.
        Container c(m_spc);
        m_velodyneContainer.send(c);
        
    }
    m_pointIndex = 0;
    m_startID = 0;
}

void Velodyne16Decoder::nextString(const string &payload) {
    if (payload.length() == 1206) {
        if(!firstPacket){
            firstPacket=true;
            TimeStamp t1;
            receiveFirstPacket=t1.toMicroseconds();
        }
        //Decode VLP-16 data
        uint32_t position = 0; //position specifies the starting position to read from the 1206 bytes

        //The payload of a VLP-16 packet consists of 12 blocks with 100 bytes each. Decode each block separately.
        static uint8_t firstByte, secondByte;
        static uint32_t dataValue;
        for (uint8_t blockID = 0; blockID < 12; blockID++) {
            //Skip the flag: 0xFFEE(2 bytes)
            position += 2;

            //Decode azimuth information: 2 bytes. Swap the two bytes, change to decimal, and divide it by 100. Due to azimuth interpolation, the azimuth of blocks 1-11 is already decoded in the middle of the previous block.
            if(blockID == 0){
                firstByte = (uint8_t)(payload.at(position));
                secondByte = (uint8_t)(payload.at(position + 1));
                dataValue = ntohs(firstByte * 256 + secondByte);
                m_currentAzimuth = static_cast< float >(dataValue / 100.0);
            }
            else{
                m_currentAzimuth = m_nextAzimuth;
                if(m_currentAzimuth > 360.0f){
                    m_currentAzimuth -= 360.0f;
                }
            }
            if (m_currentAzimuth < m_previousAzimuth) {
                sendSharedPointCloud(); //Send a complete scan as one frame
            }
            m_previousAzimuth = m_currentAzimuth;
            position += 2;

            //Only decode the data if the maximum number of points of the current frame has not been reached
            if (m_pointIndex < m_MAX_POINT_SIZE) {
                //Decode distance information and intensity of each beam/channel in a block, which contains two firing sequences
                for (uint8_t counter = 0; counter < 32; counter++) {
                    //Interpolate azimuth value
                    if (counter == 16) {
                        if (blockID < 11) {
                            position += 50; //3*16+2, move the pointer to the azimuth bytes of the next data block
                            firstByte = (uint8_t)(payload.at(position));
                            secondByte = (uint8_t)(payload.at(position + 1));
                            dataValue = ntohs(firstByte * 256 + secondByte);
                            m_nextAzimuth = static_cast< float >(dataValue / 100.0);
                            position -= 50; //reset pointer
                            if (m_nextAzimuth < m_currentAzimuth) {
                                m_nextAzimuth += 360.0f;
                            }
                            m_deltaAzimuth = (m_nextAzimuth - m_currentAzimuth) / 2.0f;
                            m_currentAzimuth += m_deltaAzimuth;
                        } else {
                            m_currentAzimuth += m_deltaAzimuth;
                        }
                        if (m_currentAzimuth > 360.0f) {
                            m_currentAzimuth -= 360.0f;
                            if (m_currentAzimuth < m_previousAzimuth) {
                                sendSharedPointCloud(); //Send a complete scan as one frame
                            }
                        }
                        m_previousAzimuth = m_currentAzimuth;
                    }

                    uint8_t sensorID = counter;
                    if (counter > 15) {
                        sensorID = counter - 16;
                    }
                    //Decode distance: 2 bytes. Swap the bytes, change to decimal, and divide it by 500
                    firstByte = (uint8_t)(payload.at(position));
                    secondByte = (uint8_t)(payload.at(position + 1));
                    dataValue = ntohs(firstByte * 256 + secondByte);
                    m_distance = dataValue / 500.0; //*2mm-->/1000 for meter

                    //Discard distances shorter than 1m
                    if (m_distance > 1.0f) {
                        static float xyDistance, xData, yData, zData, intensity;
                        //Compute x, y, z cooridnate
                        xyDistance = m_distance * cos(m_vertCorrection[sensorID] * toRadian);
                        xData = xyDistance * sin(m_currentAzimuth * toRadian);
                        yData = xyDistance * cos(m_currentAzimuth * toRadian);
                        zData = m_distance * sin(m_vertCorrection[sensorID] * toRadian);
                        //Get intensity/reflectivity: 1 byte
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
                        //cout<<"Point overflow!"<<endl;
                        break;
                    }
                }
            } else {
                position += 96; //32*3(bytes), skip one block
            }
        }
        //Ignore the last 6 bytes: 4 bytes timestamp and 2 factory bytes
    }
}
}
}
}
} // opendlv::core::system::proxy
