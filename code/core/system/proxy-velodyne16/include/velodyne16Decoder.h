/**
 * velodyne16Decoder is used to decode VLP-16 data realized with OpenDaVINCI
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
 
#ifndef VELODYNE16DECODER_H_
#define VELODYNE16DECODER_H_

#include <memory>

#include <opendavinci/odcore/io/StringListener.h>
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/wrapper/SharedMemory.h"
#include "opendavinci/generated/odcore/data/SharedPointCloud.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {
    using namespace odcore::wrapper;

    // This class will handle bytes received via a UDP socket.
    class velodyne16Decoder : public odcore::io::StringListener {
    private:
        /**
         * "Forbidden" copy constructor. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the copy constructor.
         */
        velodyne16Decoder(const velodyne16Decoder &);

        /**
         * "Forbidden" assignment operator. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the assignment operator.
         */
        velodyne16Decoder& operator=(const velodyne16Decoder &);

    public:
        /**
         * Constructor.
         */
        velodyne16Decoder(std::shared_ptr<SharedMemory>,odcore::io::conference::ContainerConference&);

        virtual ~velodyne16Decoder();
        
        void sendSPC(const float &oldAzimuth, const float &newAzimuth);
        
        virtual void nextString(const std::string &s);
    
    private:
        const uint32_t MAX_POINT_SIZE=30000;  //the maximum number of points per frame. This upper bound should be set as low as possible, as it affects the shared memory size and thus the frame updating speed.
        const uint32_t SIZE_PER_COMPONENT = sizeof(float);
        const uint8_t NUMBER_OF_COMPONENTS_PER_POINT = 4; // How many components do we have per vector?
        const uint32_t SIZE = MAX_POINT_SIZE * NUMBER_OF_COMPONENTS_PER_POINT * SIZE_PER_COMPONENT; // What is the total size of the shared memory?    
        const float PI=3.14159;
        
        uint32_t pointIndex;
        uint32_t startID;
        float previousAzimuth;
        float deltaAzimuth;
        float distance;
        std::shared_ptr<SharedMemory> VelodyneSharedMemory;//shared memory for the shared point cloud
        float* segment;//temporary memory for transferring data of each frame to the shared memory
        odcore::io::conference::ContainerConference& velodyneFrame;
        odcore::data::SharedPointCloud spc;//shared point cloud
        float vertCorrection[16];  //Vertal angle of each sensor beam  
    };
}
}
}
} // opendlv::core::system::proxy
#endif /*VELODYNE16DECODER_H_*/

