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

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <array>

#include "opendavinci/generated/odcore/data/SharedPointCloud.h"
#include "opendavinci/odcore/base/Lock.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"
#include <opendavinci/odcore/data/TimeStamp.h>

#include "velodyne32Decoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

void Velodyne32Decoder::readCalibrationFile(){
    //Load calibration data from the calibration file
    //HDL-32E has 32 channels/sensors. Each sensor has a specific vertical angle, which can be read from
    //m_verticalAngle[sensor ID] is specified in the calibration file.
    string line;
    ifstream in(m_calibration);
    if (!in.is_open()){
        cout << "Calibration file not found." << endl;
    }
    uint8_t counter = 0; //corresponds to the index of the vertical angle of each beam
    bool found = false;

    while (getline(in, line) && counter < 32) {
        string tmp; // strip whitespaces from the beginning
        for (uint8_t i = 0; i < line.length(); i++) {

            if ((line[i] == '\t' || line[i] == ' ') && tmp.size() == 0) {
            } else {
                if (line[i] == '<') {

                    if (found) {
                        m_verticalAngle[counter] = atof(tmp.c_str());

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

void Velodyne32Decoder::index32sensorIDs() {
    //Distance values for each 32 sensors with the same azimuth are ordered based on vertical angle,
    //from -30.67 to 10.67 degrees, with alternating increment 1.33 and 1.34--sensor IDs: -30.67, -29.33, -28, -26.66, -25.33, -24, -22.67, -21.33, -20, -18.67, -17.33, -16, -14.67, -13.33, -12, -10.67, -9.33, -8, -6.66, -5.33, -4, -2.67, -1.33, 0, 1.33, 2.67, 4, 5.33, 6.67, 8, 9.33, 10.67
    readCalibrationFile();
    std::array<float, 32> orderedVerticalAngle;
    for (uint8_t i = 0; i < 32; i++) {
        m_32SensorsNoIntensity[i] = 0;
        m_32SensorsWithIntensity[i] = 0;
        orderedVerticalAngle[i] = m_verticalAngle[i];
    }
    //Order the vertical angles of 32 sensor IDs with increasing value
    for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t j = i; j < 32; j++) {
            if (orderedVerticalAngle[j] < orderedVerticalAngle[i]) {
                float temp = orderedVerticalAngle[j];
                orderedVerticalAngle[j] = orderedVerticalAngle[i];
                orderedVerticalAngle[i] = temp;
            }
        }
    }
    //Find the sensor IDs in the odered list of vertical angles
    for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t j = 0; j < 32; j++) {
            if (abs(orderedVerticalAngle[i] - m_verticalAngle[j]) < 0.1f) {
                m_sensorOrderIndex[i] = j;
                break;
            }
        }
    }      
}

void Velodyne32Decoder::setupIntensityMaskCPC(uint8_t &numberOfBitsForIntensity, uint8_t &intensityPlacement) {
    if (numberOfBitsForIntensity != 0) {
        m_mask = 0xFFFF;
        if (intensityPlacement == 0) {//higher bits for intensity
            m_mask = m_mask >> numberOfBitsForIntensity;
        } else {
            m_mask = m_mask << numberOfBitsForIntensity;
        }
    }
}

Velodyne32Decoder::Velodyne32Decoder(const std::shared_ptr< SharedMemory > m,
odcore::io::conference::ContainerConference &c, const string &s, const bool &withCPC, const uint8_t &SPCOption, const uint8_t &CPCIntensityOption, const uint8_t &numberOfBitsForIntensity, const uint8_t &intensityPlacement, const uint8_t &distanceEncoding)
    : m_SPCOption(SPCOption)
    , m_CPCIntensityOption(CPCIntensityOption)
    , m_numberOfBitsForIntensity(numberOfBitsForIntensity)
    , m_intensityPlacement(intensityPlacement)
    , m_mask(0)
    , m_distanceEncoding(distanceEncoding)
    , m_pointIndexSPC(0)
    , m_pointIndexCPC(0)
    , m_startID(0)
    , m_previousAzimuth(0.0)
    , m_currentAzimuth(0.0)
    , m_distance(0.0)
    , m_velodyneSharedMemory(m)
    , m_segment(NULL)
    , m_conference(c)
    , m_spc()
    , m_verticalAngle()
    , m_calibration(s)
    , m_withSPC(true)
    , m_withCPC(withCPC)
    , m_startAzimuth(0.0)
    , m_distanceStringStreamNoIntensityPart1("")
    , m_distanceStringStreamNoIntensityPart2("")
    , m_distanceStringStreamNoIntensityPart3("")
    , m_distanceStringStreamWithIntensityPart1("")
    , m_distanceStringStreamWithIntensityPart2("")
    , m_distanceStringStreamWithIntensityPart3("")
    , m_isStartAzimuth(true)
    , m_sensorOrderIndex()
    , m_32SensorsNoIntensity()
    , m_32SensorsWithIntensity() {
    //Initial setup of the shared point cloud (N.B. The size and width of the shared point cloud depends on the number of points of a frame, hence they are not set up in the constructor)
    m_spc.setName(m_velodyneSharedMemory->getName()); // Name of the shared memory segment with the data.
    m_spc.setHeight(1); // We have just a sequence of vectors.
    m_spc.setNumberOfComponentsPerPoint(m_NUMBER_OF_COMPONENTS_PER_POINT);
    m_spc.setComponentDataType(SharedPointCloud::FLOAT_T); // Data type per component.
    if (m_SPCOption == 0) {
        m_spc.setUserInfo(SharedPointCloud::XYZ_INTENSITY);
    } else {
        m_spc.setUserInfo(SharedPointCloud::POLAR_INTENSITY);
    }

    //Create memory for temporary storage of point cloud data for each frame
    m_segment = (float *)malloc(m_SIZE);
    if (m_segment == NULL) {
        throw bad_alloc();
    }
    index32sensorIDs();
    if (m_withCPC && m_numberOfBitsForIntensity > 0) {
        setupIntensityMaskCPC(m_numberOfBitsForIntensity, m_intensityPlacement);
    }
}

Velodyne32Decoder::Velodyne32Decoder(odcore::io::conference::ContainerConference &c, const string &s, const uint8_t &CPCIntensityOption, const uint8_t &numberOfBitsForIntensity, const uint8_t &intensityPlacement, const uint8_t &distanceEncoding)
    : m_SPCOption(0)
    , m_CPCIntensityOption(CPCIntensityOption)
    , m_numberOfBitsForIntensity(numberOfBitsForIntensity)
    , m_intensityPlacement(intensityPlacement)
    , m_mask()
    , m_distanceEncoding(distanceEncoding)
    , m_pointIndexSPC(0)
    , m_pointIndexCPC(0)
    , m_startID(0)
    , m_previousAzimuth(0.0)
    , m_currentAzimuth(0.0)
    , m_distance(0.0)
    , m_velodyneSharedMemory()
    , m_segment(NULL)
    , m_conference(c)
    , m_spc()
    , m_verticalAngle()
    , m_calibration(s)
    , m_withSPC(false)
    , m_withCPC(true)
    , m_startAzimuth(0.0)
    , m_distanceStringStreamNoIntensityPart1("")
    , m_distanceStringStreamNoIntensityPart2("")
    , m_distanceStringStreamNoIntensityPart3("")
    , m_distanceStringStreamWithIntensityPart1("")
    , m_distanceStringStreamWithIntensityPart2("")
    , m_distanceStringStreamWithIntensityPart3("")
    , m_isStartAzimuth(true)
    , m_sensorOrderIndex()
    , m_32SensorsNoIntensity()
    , m_32SensorsWithIntensity() {
    index32sensorIDs();
    setupIntensityMaskCPC(m_numberOfBitsForIntensity, m_intensityPlacement);
}

Velodyne32Decoder::~Velodyne32Decoder() {
    if (m_withSPC) {
        free(m_segment);
    }
}

//Send 3 CPC containers for each complete scan, 12+11+9 layers' combination
void Velodyne32Decoder::sendCPC (bool noIntensity) { 
    TimeStamp now;
    if (noIntensity) {
        CompactPointCloud cpc1(m_startAzimuth, m_previousAzimuth, 12, m_distanceStringStreamNoIntensityPart1.str(), 0, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c1(cpc1);
        c1.setSampleTimeStamp(now);
        m_conference.send(c1);
        CompactPointCloud cpc2(m_startAzimuth, m_previousAzimuth, 11, m_distanceStringStreamNoIntensityPart2.str(), 0, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c2(cpc2);
        c2.setSampleTimeStamp(now);
        m_conference.send(c2);
        CompactPointCloud cpc3(m_startAzimuth, m_previousAzimuth, 9, m_distanceStringStreamNoIntensityPart3.str(), 0, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c3(cpc3);
        c3.setSampleTimeStamp(now);
        m_conference.send(c3);
    } else {
        CompactPointCloud cpc4(m_startAzimuth, m_previousAzimuth, 12, m_distanceStringStreamWithIntensityPart1.str(), m_numberOfBitsForIntensity, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c4(cpc4);
        c4.setSampleTimeStamp(now);
        m_conference.send(c4);
        CompactPointCloud cpc5(m_startAzimuth, m_previousAzimuth, 11, m_distanceStringStreamWithIntensityPart2.str(), m_numberOfBitsForIntensity, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c5(cpc5);
        c5.setSampleTimeStamp(now);
        m_conference.send(c5);
        CompactPointCloud cpc6(m_startAzimuth, m_previousAzimuth, 9, m_distanceStringStreamWithIntensityPart3.str(), m_numberOfBitsForIntensity, static_cast< CompactPointCloud::INTENSITY_PLACEMENT >(m_intensityPlacement), static_cast< CompactPointCloud::DISTANCE_ENCODING >(m_distanceEncoding));    
        Container c6(cpc6);
        c6.setSampleTimeStamp(now);
        m_conference.send(c6);
    }
}

//Update the shared or compact point cloud when a complete scan is completed.
void Velodyne32Decoder::sendPointCloud() {  
    //Send shared point cloud
    if (m_withSPC) {
        if (m_velodyneSharedMemory->isValid()) {
            Lock l(m_velodyneSharedMemory);
            memcpy(m_velodyneSharedMemory->getSharedMemory(), m_segment, m_SIZE);
            //Set the size and width of the shared point cloud of the current frame
            m_spc.setSize(m_SIZE); // Size in raw bytes.
            m_spc.setWidth(m_pointIndexSPC); // Number of points.
            TimeStamp now;
            Container c(m_spc);
            c.setSampleTimeStamp(now);
            m_conference.send(c);     
        }
        m_pointIndexSPC = 0;
        m_startID = 0;
    }
    //Send compact point cloud (format: start azimuth, end azimuth, entries per azimuth, distances, number if bits for intensity, intensity placement, distance decoding)
    if (m_withCPC) {
        if (m_CPCIntensityOption == 0) {//No intensity
            sendCPC (true);
        } else if (m_CPCIntensityOption == 1) {//With intensity
            sendCPC (false);
        } else{//Send both CPCs with and without intensity
             sendCPC (true);
             sendCPC (false);
        }
        m_pointIndexCPC = 0;
        m_startAzimuth = m_currentAzimuth;
        m_isStartAzimuth = false;
        m_distanceStringStreamNoIntensityPart1.str("");
        m_distanceStringStreamNoIntensityPart2.str("");
        m_distanceStringStreamNoIntensityPart3.str("");
        m_distanceStringStreamWithIntensityPart1.str("");
        m_distanceStringStreamWithIntensityPart2.str("");
        m_distanceStringStreamWithIntensityPart3.str("");
    }
}

void Velodyne32Decoder::nextString(const string &payload) {
    if (payload.length() == 1206) {
        //Decode VLP-32 data
        uint32_t position = 0; //position specifies the starting position to read from the 1206 bytes

        //The payload of a VLP-32 packet consists of 12 blocks with 100 bytes each. Decode each block separately.
        static uint8_t firstByte, secondByte, thirdByte;//two bytes for distance and one byte for intensity
        static uint16_t dataValue;
        for (uint8_t blockID = 0; blockID < 12; blockID++) {
            //Skip the flag: 0xEEFF for upper block or 0xDDFF for lower block (2 bytes)
            position += 2;

            //Decode azimuth information: 2 bytes. Swap the two bytes, change to decimal, and divide it by 100.
            firstByte = (uint8_t)(payload.at(position));
            secondByte = (uint8_t)(payload.at(position + 1));
            dataValue = ntohs(firstByte * 256 + secondByte);
            m_currentAzimuth = static_cast< float >(dataValue / 100.0f);

            if (m_currentAzimuth < m_previousAzimuth) {
                sendPointCloud(); //Send a complete scan as one frame
            }
            m_previousAzimuth = m_currentAzimuth;
            position += 2;

            //Only decode the data if the maximum number of points of the current frame has not been reached
            if (m_pointIndexSPC < m_MAX_POINT_SIZE || m_pointIndexCPC < m_MAX_POINT_SIZE) {
                //Decode distance information and intensity of each beam/channel in a block
                for (uint8_t counter = 0; counter < 32; counter++) {

                    uint8_t sensorID = counter;
                    //Decode distance: 2 bytes. Swap the bytes
                    firstByte = (uint8_t)(payload.at(position));
                    secondByte = (uint8_t)(payload.at(position + 1));
                    thirdByte = (uint8_t)(payload.at(position + 2));//original intensity value
                    
                    if (m_withSPC && m_pointIndexSPC < m_MAX_POINT_SIZE) {
                        dataValue = ntohs(firstByte * 256 + secondByte);
                        m_distance = dataValue / 500.0f; //2mm-->/1000 for meter
                        
                        if (m_distance > 1.0f) {
                            if (m_SPCOption == 0) {//xyz+intensity
                                static float xyDistance, xData, yData, zData;
                                //Compute x, y, z cooridnate
                                xyDistance = m_distance * cos(m_verticalAngle[sensorID] * toRadian);
                                xData = xyDistance * sin(m_currentAzimuth * toRadian);
                                yData = xyDistance * cos(m_currentAzimuth * toRadian);
                                zData = m_distance * sin(m_verticalAngle[sensorID] * toRadian);

                                //Store coordinate information of each point to the malloc memory
                                m_segment[m_startID] = xData;
                                m_segment[m_startID + 1] = yData;
                                m_segment[m_startID + 2] = zData;
                                m_segment[m_startID + 3] = (float)thirdByte;//intensity

                            } else {//distance+azimuth+vertical angle+intensity
                                m_segment[m_startID] = m_distance;
                                m_segment[m_startID + 1] = m_currentAzimuth;
                                m_segment[m_startID + 2] = m_verticalAngle[sensorID];
                                m_segment[m_startID + 3] = (float)thirdByte;//intensity; 
                            }
                            m_pointIndexSPC++;
                            m_startID += m_NUMBER_OF_COMPONENTS_PER_POINT;
                        }   
                    }
                    
                    if (m_withCPC && m_pointIndexCPC < m_MAX_POINT_SIZE) {
                        if (m_CPCIntensityOption == 0 || m_CPCIntensityOption == 2) {
                            //Store distance with resolution 2mm in an array of uint16_t type
                            m_32SensorsNoIntensity[sensorID] = ntohs(firstByte * 256 + secondByte);
                            if (m_distanceEncoding == 0) {
                                m_32SensorsNoIntensity[sensorID] = m_32SensorsNoIntensity[sensorID] / 5;  //Store distance with resolution 1cm instead
                            }
                            
                            if (sensorID == 31) {
                                for (uint8_t index = 0; index < 32; index++) {
                                    m_32SensorsNoIntensity[m_sensorOrderIndex[index]] = htons(m_32SensorsNoIntensity[m_sensorOrderIndex[index]]);
                                    if (index ==0 || index % 3 == 1) {//Layer 0, 1, 4, 7..., i.e., in addition to Layer 0, every 3rd layer from Layer 1 and resulting in 12 layers
                                        m_distanceStringStreamNoIntensityPart1.write((char*)(&m_32SensorsNoIntensity[m_sensorOrderIndex[index]]),2);
                                    } else if (index == 2 || index % 3 == 0) {//Layer 2, 3, 6, 9..., i.e., in addition to Layer 2, every 3rd layer from Layer 3 and resulting in 11 layers
                                        m_distanceStringStreamNoIntensityPart2.write((char*)(&m_32SensorsNoIntensity[m_sensorOrderIndex[index]]),2);
                                    } else {//Layer 5, 8, 11..., i.e., every 3rd layer from Layer 5 and resulting in 9 layers
                                        m_distanceStringStreamNoIntensityPart3.write((char*)(&m_32SensorsNoIntensity[m_sensorOrderIndex[index]]),2);
                                    }
                                }
                            }
                            m_pointIndexCPC++;    
                        }
                        
                        if (m_CPCIntensityOption == 1 || m_CPCIntensityOption == 2) {
                            //Store distance with resolution 2mm in an array of uint16_t type
                            uint16_t distance = ntohs(firstByte * 256 + secondByte);
                            if (m_distanceEncoding == 0) {
                                distance = distance / 5; //Store distance with resolution 1cm instead
                            }
                            
                            uint16_t intensityLevel = thirdByte;
                            if (m_intensityPlacement == 0) {//higher bits for intensity
                                if (distance <= m_mask) {
                                    distance = distance & m_mask; //Reserve higher n bits for intensity
                                    intensityLevel = intensityLevel >> (8 - m_numberOfBitsForIntensity);
                                    m_32SensorsWithIntensity[sensorID] = (intensityLevel << (16 - m_numberOfBitsForIntensity) ) + distance;
                                } else {//m_mask determines the number of bits for the covered distance. Distance longer than that should return 0.
                                    m_32SensorsWithIntensity[sensorID] = 0;
                                }
                            } else {//lower bits for intensity
                                distance = distance & m_mask; //Reserve lower n bits for intensity
                                intensityLevel = intensityLevel >> (8 - m_numberOfBitsForIntensity);
                                m_32SensorsWithIntensity[sensorID] = distance + intensityLevel;//(16-n) bits for distance + n bits for intensity
                            }

                            if (sensorID == 31) {
                                for (uint8_t index = 0; index < 32; index++) {
                                    m_32SensorsWithIntensity[m_sensorOrderIndex[index]] = htons(m_32SensorsWithIntensity[m_sensorOrderIndex[index]]);
                                    if (index ==0 || index % 3 == 1) {//Layer 0, 1, 4, 7..., i.e., in addition to Layer 0, every 3rd layer from Layer 1 and resulting in 12 layers
                                        m_distanceStringStreamWithIntensityPart1.write((char*)(&m_32SensorsWithIntensity[m_sensorOrderIndex[index]]),2);
                                    } else if (index == 2 || index % 3 == 0) {//Layer 2, 3, 6, 9..., i.e., in addition to Layer 2, every 3rd layer from Layer 3 and resulting in 11 layers
                                        m_distanceStringStreamWithIntensityPart2.write((char*)(&m_32SensorsWithIntensity[m_sensorOrderIndex[index]]),2);
                                    } else {//Layer 5, 8, 11..., i.e., every 3rd layer from Layer 5 and resulting in 9 layers
                                        m_distanceStringStreamWithIntensityPart3.write((char*)(&m_32SensorsWithIntensity[m_sensorOrderIndex[index]]),2);
                                    }
                                }
                            }
                            if (m_CPCIntensityOption == 1) {
                                m_pointIndexCPC++; 
                            }
                        }
                         
                        
                    }
                    
                    position += 3;

                    if ((m_withCPC && m_pointIndexCPC >= m_MAX_POINT_SIZE) || (!m_withCPC && m_pointIndexSPC >= m_MAX_POINT_SIZE)) {
                        position += 3 * (31 - counter); //Discard the points of the current frame when the preallocated shared memory is full; move the position to be read in the 1206 bytes
                        cout << "More than 70000 points." << endl; 
                        break;
                    }
                }
            } else {
                position += 96; //32*3(bytes), skip one block
            }
        }
        //Ignore the last 6 bytes: 4 bytes GPS time stamp, 2 blank bytes
    }
}
}
}
}
} // opendlv::core::system::proxy
