/**
 * proxy-velodyne16 - Interface to Velodyne VLP-16.
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

#ifndef PROXY_PROXYVELODYNE16_TESTSUITE_H
#define PROXY_PROXYVELODYNE16_TESTSUITE_H

#include "cxxtest/TestSuite.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "opendavinci/generated/odcore/data/SharedPointCloud.h"
#include "opendavinci/generated/odcore/data/pcap/GlobalHeader.h"
#include "opendavinci/generated/odcore/data/pcap/Packet.h"
#include "opendavinci/generated/odcore/data/pcap/PacketHeader.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/io/conference/ContainerConference.h"
#include "opendavinci/odcore/io/conference/ContainerListener.h"
#include "opendavinci/odcore/io/protocol/PCAPProtocol.h"
#include "opendavinci/odcore/wrapper/CompressionFactory.h"
#include "opendavinci/odcore/wrapper/DecompressedData.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "opendavinci/odcore/wrapper/SharedMemoryFactory.h"

#include "../include/velodyne16Decoder.h"

using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::io::protocol;
using namespace odcore::wrapper;

class MyContainerConference : public odcore::io::conference::ContainerConference {
   public:
    MyContainerConference(std::shared_ptr< odcore::wrapper::SharedMemory > m)
        : ContainerConference()
        , m_counter(0)
        , m_velodyneMemory(m)
        , m_numberOfSPCPoints(0) {}

    virtual void send(odcore::data::Container &c) const {
        m_counter++;
        if (m_counter == 2) { //We are only interested in the azimuth, distance, and intensity values of points in the second Velodyne frame (Frame 1)
            if (c.getDataType() == odcore::data::SharedPointCloud::ID()) {
                odcore::data::SharedPointCloud velodyneFrame = c.getData< SharedPointCloud >();                                            //Get shared point cloud
                std::shared_ptr< odcore::wrapper::SharedMemory > vsm = SharedMemoryFactory::attachToSharedMemory(velodyneFrame.getName()); //Attach the shared point cloud to the shared memory

                if (vsm.get() != NULL) {
                    if (vsm->isValid()) {
                        //odcore::base::Lock l(vsm); //No need to lock shared resource as the test suite runs in a single thread. If the lock is enabled, the test suite will not terminate
                        if (velodyneFrame.getComponentDataType() == SharedPointCloud::FLOAT_T && (velodyneFrame.getNumberOfComponentsPerPoint() == 3) && (velodyneFrame.getUserInfo() == SharedPointCloud::POLAR_INTENSITY)) {
                            memcpy(m_velodyneMemory->getSharedMemory(), vsm->getSharedMemory(), velodyneFrame.getSize());
                            cout << "Copy decoded data in Frame 1 to a memory segment" << endl;
                            m_numberOfSPCPoints = velodyneFrame.getWidth();
                        }
                    }
                }
            }
        }
    }
    uint32_t getNumberOfSPCPoints() {
        return m_numberOfSPCPoints;
    }
    
    mutable uint8_t m_counter;
    mutable std::shared_ptr< odcore::wrapper::SharedMemory > m_velodyneMemory;
    mutable uint32_t m_numberOfSPCPoints;
};

class packetToByte : public odcore::io::conference::ContainerListener {
    public:
        packetToByte(std::shared_ptr< odcore::wrapper::SharedMemory > m1, std::shared_ptr< odcore::wrapper::SharedMemory > m2)
        : m_mcc(m2)
        , m_velodyne16decoder(m1, m_mcc, "../VLP-16.xml", false, 0, 0, 1) //The calibration file VLP-16.xml is automatically copied to the parent folder of the test suite binary
        {}

        ~packetToByte() {}

        virtual void nextContainer(odcore::data::Container &c) {
            if (c.getDataType() == odcore::data::pcap::Packet::ID()) {
                // Here, we have a valid packet.

                //Extract payload from the packet
                pcap::Packet packet = c.getData< pcap::Packet >();
                pcap::PacketHeader packetHeader = packet.getHeader();
                if (packetHeader.getIncl_len() == 1248) {
                    string payload = packet.getPayload();
                    payload = payload.substr(42, 1206); //Remove the 42-byte Ethernet header
                    m_velodyne16decoder.nextString(payload);     //Let the Velodyne16 decoder decode the 1206-byte payload
                }
            }
        }
        
        uint32_t getNumberOfSPCPoints() {
            return m_mcc.getNumberOfSPCPoints();
        }
    
   private:
        MyContainerConference m_mcc;
        opendlv::core::system::proxy::Velodyne16Decoder m_velodyne16decoder;
        uint32_t m_numberOfSPCPoints;
};

class ProxyVelodyne16Test : public CxxTest::TestSuite {
   public:
    ProxyVelodyne16Test()
        : m_velodyneSharedMemory(SharedMemoryFactory::createSharedMemory(m_NAME, m_SIZE))
        , m_segment(SharedMemoryFactory::createSharedMemory(m_NAME2, m_SIZE)) {
        //Distance values are reordered with increasing vertical angles while storing SPC and CPC
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

    ~ProxyVelodyne16Test() {}

    void readCsvFile() {                                              //Read a .csv file that is compressed into a .zip file
        fstream fin("../sampleVeloViewFrame1.zip", ios::binary | ios::in); //Read xyz and intensity values of points in Frame 1 exported from VeloView. These values exist in a zip file in the parent folder
        TS_ASSERT(fin.is_open());
        std::shared_ptr< odcore::wrapper::DecompressedData > dd = odcore::wrapper::CompressionFactory::getContents(fin);
        fin.close();
        vector< string > entries = dd->getListOfEntries();
        std::shared_ptr< istream > stream = dd->getInputStreamFor(entries.at(0));
        stringstream decompressedData; //Store the decompressed data from the zip file
        if (stream.get()) {
            char c;
            while (stream->get(c)) {
                decompressedData << c;
            }
        }
        string value;
        getline(decompressedData, value);          //The .csv file after decompression has this format: "azimuth, distance, intensity,", while each line represents one point
        while (getline(decompressedData, value)) { //Read the csv file line by line and save x, y, z, intensity values of all points in Frame 1 to respective vectors for comparison with our Velodyne16 decoder
            stringstream lineStream(value);
            string cell;
            getline(lineStream, cell, ',');
            m_azimuthVeloView.push_back(stof(cell));
            getline(lineStream, cell, ',');
            m_distanceVeloView.push_back(stof(cell));
            getline(lineStream, cell, '\n');
            m_intensityVeloView.push_back(stof(cell));
        }
    }


    void testVelodyneDecodingFromFile() {
        readCsvFile();
        PCAPProtocol pcap;                     //Use the PCAP decoder of OpenDaVINCI to read the .pcap recording
        packetToByte p2b(m_velodyneSharedMemory, m_segment); //This class extracts Velodyne payload from pcap packets
        pcap.setContainerListener(&p2b);       //Set the packetToByte class as the container listener of the PCAP decoder

        fstream lidarStream("../sampleShort.pcap", ios::binary | ios::in); //A sample .pcap file containing several Velodyne frames in the parent folder
        TS_ASSERT(lidarStream.is_open());
        char *buffer = new char[m_BUFFER_SIZE + 1];
        while (lidarStream.good()) {
            lidarStream.read(buffer, m_BUFFER_SIZE * sizeof(char));
            string s(buffer, m_BUFFER_SIZE);
            pcap.nextString(s); //Read bytes from the .pcap file and feed them to the PCAP decoder
        }
        lidarStream.close();
        pcap.setContainerListener(NULL);

        cout << "File read complete." << endl;
        delete[] buffer;
        
        uint32_t compare = 0; //Number of points matched between VeloView and our Velodyne decoder
        if (m_segment->isValid()) {
            float *velodyneRawData = static_cast< float * >(m_segment->getSharedMemory()); //the shared memory "m_segment" should already contain Velodyne data of Frame 1 decoded by our Velodyne16 decoder and stored in the send method of the MyContainerConference class
            uint32_t numberOfPointsSPC = p2b.getNumberOfSPCPoints();
            cout << "Number of points from SPC: " << numberOfPointsSPC << endl;
            uint32_t numberOfAzimuths = numberOfPointsSPC / 16;
            uint32_t startID = 0;
            float azimuth[16],distance[16],intensity[16];
            
            //Put back the original sensor ID order for each 16 sensors
            for (uint32_t counter = 0; counter < numberOfAzimuths; counter++) {
                for (uint8_t sensorIndex = 0; sensorIndex < 16; sensorIndex++) {
                    azimuth[m_sensorOrderIndex[sensorIndex]] = velodyneRawData[startID];
                    distance[m_sensorOrderIndex[sensorIndex]] = velodyneRawData[startID + 1];
                    intensity[m_sensorOrderIndex[sensorIndex]] = velodyneRawData[startID + 2];
                    startID += 3;
                }
                for (uint8_t i = 0; i < 16; i++) {
                    m_azimuthSPC.push_back(azimuth[i]);
                    m_distanceSPC.push_back(distance[i]);
                    m_intensitySPC.push_back(intensity[i]);
                }
            }
            
            cout << "Before comparing:" << compare << endl;
            
            uint32_t spcCounter = 0;
            //VeloView drops points with distance less than 1m, while our Velodyne decoder keeps all points. For each point in
            //the csv file exported from VeloView, there must be a corresponding point in the SPC.
            for (uint32_t i = 0; i < m_distanceVeloView.size(); i++) {
                for (uint32_t j = spcCounter; j < numberOfPointsSPC; j++) {
                    if ((abs(m_azimuthSPC[j] - m_azimuthVeloView[i] / 100.0f) < 0.15f) && (abs(m_distanceSPC[j] - m_distanceVeloView[i]) < 0.1f) && (abs(m_intensitySPC[j] - m_intensityVeloView[i]) < 0.1f)) {
                        spcCounter = j;
                        compare++;
                        break;
                    }
                }
            }
        }

        cout << "Number of points from VeloView: " << m_distanceVeloView.size() << endl;
        cout << "Number of points matched from our Velodyne decoder: " << compare << endl;

        TS_ASSERT(static_cast< float >(compare) / static_cast< float >(m_distanceVeloView.size()) > 0.99f);  //At least 98% of all the points of Frame 1 should be matched between SPC and VeloView for the sample pcap file. 100% is not expected due to azimuth interpolation. SPC takes the average of two reported azimuth values, while VeloView takes time stamp into account to give more precise but more expensive azimuth interpolation.
    }

   private:
    const uint32_t m_BUFFER_SIZE = 4000;
    const std::string m_NAME = "testVelodyne16SM"; //The name for the shared memory m_velodyneSharedMemory
    const std::string m_NAME2 = "sharedSegment16";   //The name for the shared memory segment
    const uint32_t m_SIZE = 360000;
    //The total size of the shared memory: MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * sizeof(float), where MAX_POINT_SIZE is the maximum number of points per frame (This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed), NUMBER_OF_COMPONENTS_PER_POIN=3 (azimuth,distance,intensity) Recommended values: MAX_POINT_SIZE=30000->ProxyVelodyne16.sharedMemory.size = 360000

    vector< float > m_azimuthVeloView;
    vector< float > m_distanceVeloView;
    vector< float > m_intensityVeloView;
    vector< float > m_azimuthSPC;
    vector< float > m_distanceSPC;
    vector< float > m_intensitySPC;
    std::shared_ptr< odcore::wrapper::SharedMemory > m_velodyneSharedMemory; //This shared memory should be passed to the Velodyne16 decoder via the packetToByte class
    std::shared_ptr< odcore::wrapper::SharedMemory > m_segment;    //This shared memory should be passed to MyContainerConference class which then has write access to this shared memory
    uint16_t m_sensorOrderIndex[16];
};

#endif /*PROXY_PROXYVELODYNE16_TESTSUITE_H*/
