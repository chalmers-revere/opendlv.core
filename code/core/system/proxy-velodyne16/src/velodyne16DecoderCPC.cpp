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

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "opendavinci/GeneratedHeaders_OpenDaVINCI.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "velodyne16DecoderCPC.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

Velodyne16DecoderCPC::Velodyne16DecoderCPC(odcore::io::conference::ContainerConference &c)
    : m_pointIndex(0)
    , m_previousAzimuth(0.0)
    , m_currentAzimuth(0.0)
    , m_nextAzimuth(0.0)
    , m_deltaAzimuth(0.0)
    , m_distance(0.0)
    , m_velodyneContainer(c)
    , m_startAzimuth(0.0)
    , m_distanceStringStream("")
    , m_isStartAzimuth(true) {
    //Distance values for each 16 sensors with the same azimuth are ordered based on vertical angle,
    //from -15 to 15 degress, with increment 2--sensor IDs: 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15
    m_sensorOrderIndex[0] = 0;
    m_sensorOrderIndex[1] = 2;
    m_sensorOrderIndex[2] = 4;
    m_sensorOrderIndex[3] = 6;
    m_sensorOrderIndex[4] = 8;
    m_sensorOrderIndex[5] = 10;
    m_sensorOrderIndex[6] = 12;
    m_sensorOrderIndex[7] = 14;
    m_sensorOrderIndex[8] = 1;
    m_sensorOrderIndex[9] = 3;
    m_sensorOrderIndex[10] = 5;
    m_sensorOrderIndex[11] = 7;
    m_sensorOrderIndex[12] = 9;
    m_sensorOrderIndex[13] = 11;
    m_sensorOrderIndex[14] = 13;
    m_sensorOrderIndex[15] = 15;
}

Velodyne16DecoderCPC::~Velodyne16DecoderCPC() {}

//Update the shared point cloud when a complete scan is completed.
void Velodyne16DecoderCPC::sendCompactPointCloud() {   
    CompactPointCloud cpc(m_startAzimuth,m_previousAzimuth,m_ENTRIES_PER_AZIMUTH,m_distanceStringStream.str());    
    Container c(cpc);
    m_velodyneContainer.send(c);

    m_pointIndex = 0;
    m_startAzimuth = m_currentAzimuth;
    m_isStartAzimuth=false;
    m_distanceStringStream.str("");
}

void Velodyne16DecoderCPC::nextString(const string &payload) {
    if (payload.length() == 1206) {
        //Decode VLP-16 data
        uint32_t position = 0; //position specifies the starting position to read from the 1206 bytes

        //The payload of a VLP-16 packet consists of 12 blocks with 100 bytes each. Decode each block separately.
        static uint8_t firstByte, secondByte;
        static uint16_t dataValue;
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
                sendCompactPointCloud(); //Send a complete scan as one frame
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
                                sendCompactPointCloud(); //Send a complete scan as one frame
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
                    m_16Sensors[sensorID] = ntohs(firstByte * 256 + secondByte)/5;
                                
                    if(sensorID==15){
                        for(uint8_t index=0;index<16;index++){
                            m_distanceStringStream.write((char*)(&m_16Sensors[m_sensorOrderIndex[index]]),2);
                        }
                    }

                    m_pointIndex++;
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
