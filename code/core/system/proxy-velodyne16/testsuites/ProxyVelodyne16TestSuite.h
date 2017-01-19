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
        , m_velodyneMemory(m) {}

    virtual void send(odcore::data::Container &c) const {
        m_counter++;
        if (m_counter == 2) { //We are only interested in the x, y, z, intensity values of points in the second Velodyne frame (Frame 1)
            if (c.getDataType() == odcore::data::SharedPointCloud::ID()) {
                odcore::data::SharedPointCloud velodyneFrame = c.getData< SharedPointCloud >();                                            //Get shared point cloud
                std::shared_ptr< odcore::wrapper::SharedMemory > vsm = SharedMemoryFactory::attachToSharedMemory(velodyneFrame.getName()); //Attach the shared point cloud to the shared memory

                if (vsm.get() != NULL) {
                    if (vsm->isValid()) {
                        //odcore::base::Lock l(vsm); //No need to lock shared resource as the test suite runs in a single thread. If the lock is enabled, the test suite will not terminate
                        if (velodyneFrame.getComponentDataType() == SharedPointCloud::FLOAT_T && (velodyneFrame.getNumberOfComponentsPerPoint() == 4) && (velodyneFrame.getUserInfo() == SharedPointCloud::XYZ_INTENSITY)) {
                            memcpy(m_velodyneMemory->getSharedMemory(), vsm->getSharedMemory(), velodyneFrame.getSize());
                            cout << "Copy decoded data in Frame 1 to a memory segment" << endl;
                        }
                    }
                }
            }
        }
    }

    mutable uint8_t m_counter;
    mutable std::shared_ptr< odcore::wrapper::SharedMemory > m_velodyneMemory;
};

class packetToByte : public odcore::io::conference::ContainerListener {
   public:
    packetToByte(std::shared_ptr< odcore::wrapper::SharedMemory > m1, std::shared_ptr< odcore::wrapper::SharedMemory > m2)
        : m_mcc(m2)
        , m_velodyne16decoder(m1, m_mcc, "../VLP-16.xml", false, 0, 0, 0, 1, 1) //The calibration file VLP-16.xml is automatically copied to the parent folder of the test suite binary
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

   private:
    MyContainerConference m_mcc;
    opendlv::core::system::proxy::Velodyne16Decoder m_velodyne16decoder;
};

class ProxyVelodyne16Test : public CxxTest::TestSuite {
   public:
    ProxyVelodyne16Test()
        : m_velodyneSharedMemory(SharedMemoryFactory::createSharedMemory(m_NAME, m_SIZE))
        , m_segment(SharedMemoryFactory::createSharedMemory(m_NAME2, m_SIZE)) {}

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
        getline(decompressedData, value);          //The .csv file after decompression has this format: "x, y, z, intensity,", while each line represents one point
        while (getline(decompressedData, value)) { //Read the csv file line by line and save x, y, z, intensity values of all points in Frame 1 to respective vectors for comparison with our Velodyne16 decoder
            stringstream lineStream(value);
            string cell;
            getline(lineStream, cell, ',');
            m_xDataV.push_back(stof(cell));
            getline(lineStream, cell, ',');
            m_yDataV.push_back(stof(cell));
            getline(lineStream, cell, ',');
            m_zDataV.push_back(stof(cell));
            getline(lineStream, cell, ',');
            m_intensityV.push_back(stof(cell));
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
            
            cout << "Before comparing:" << compare << endl;
            
            for (uint32_t vCounter = 0; vCounter < m_xDataV.size(); vCounter++) {
                if ((abs(velodyneRawData[vCounter*4] - m_xDataV[vCounter]) < 0.1f) && ((abs(velodyneRawData[vCounter*4 + 1] - m_yDataV[vCounter])) < 0.1f) &&
                ((abs(velodyneRawData[vCounter*4 + 2] - m_zDataV[vCounter])) < 0.1f) && ((abs(velodyneRawData[vCounter*4 + 3] - m_intensityV[vCounter])) < 1.0f)) {
                    compare++;
                }
            }
        }

        cout << "Number of points from VeloView: " << m_xDataV.size() << endl;
        cout << "Number of points matched from our Velodyne decoder: " << compare << endl;

        TS_ASSERT(static_cast<float>(compare)/static_cast<float>(m_xDataV.size())>0.98f);  //At least 98% of all the points of Frame 1 should be matched between Velodyne16Decoder and VeloView for the sample pcap file. 100% is not expected due to azimuth interpolation. Velodyne16Decoder takes the average of two reported azimuth values, while VeloView takes time stamp into account to give more precise but more expensive azimuth interpolation.
    }

   private:
    const uint32_t m_BUFFER_SIZE = 4000;
    const std::string m_NAME = "testVelodyne16SM"; //The name for the shared memory m_velodyneSharedMemory
    const std::string m_NAME2 = "sharedSegment16";   //The name for the shared memory segment
    const uint32_t m_SIZE = 480000;
    //The total size of the shared memory: MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * sizeof(float), where MAX_POINT_SIZE is the maximum number of points per frame (This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed), NUMBER_OF_COMPONENTS_PER_POIN=4 (x, y, z, intensity) Recommended values: MAX_POINT_SIZE=30000->ProxyVelodyne16.sharedMemory.size = 480000

    vector< float > m_xDataV;
    vector< float > m_yDataV;
    vector< float > m_zDataV;
    vector< float > m_intensityV;
    std::shared_ptr< odcore::wrapper::SharedMemory > m_velodyneSharedMemory; //This shared memory should be passed to the Velodyne16 decoder via the packetToByte class
    std::shared_ptr< odcore::wrapper::SharedMemory > m_segment;    //This shared memory should be passed to MyContainerConference class which then has write access to this shared memory
};

#endif /*PROXY_PROXYVELODYNE16_TESTSUITE_H*/
