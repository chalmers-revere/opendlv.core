/**
 * proxy-fh16truck - Interface to FH16 truck.
 * Copyright (C) 2016 Christian Berger
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

#ifndef PROXY_PROXYFH16TRUCK_H
#define PROXY_PROXYFH16TRUCK_H

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include <opendavinci/odcore/base/FIFOQueue.h>
#include <opendavinci/odcore/base/Mutex.h>
#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/reflection/CSVFromVisitableVisitor.h>
#include <odcantools/GenericCANMessageListener.h>
#include <fh16mapping/GeneratedHeaders_fh16mapping.h>

namespace automotive {
class GenericCANMessage;
}
namespace automotive {
namespace odcantools {
class CANDevice;
}
}
namespace odtools {
namespace recorder {
class Recorder;
}
}

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class CanMessageDataStore;

using namespace std;

/**
 * Interface to FH16 truck.
 */
class ProxyFH16Truck : public odcore::base::module::TimeTriggeredConferenceClientModule,
    public automotive::odcantools::GenericCANMessageListener {
    public:
        ProxyFH16Truck(int32_t const &, char **);
        ProxyFH16Truck(ProxyFH16Truck const &) = delete;
        ProxyFH16Truck &operator=(ProxyFH16Truck const &) = delete;
        virtual ~ProxyFH16Truck();

        virtual void nextGenericCANMessage(const automotive::GenericCANMessage &gcm);

    private:
        void setUp();
        void tearDown();
        odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

        void setUpRecordingGenericCANMessage(const std::string &timeStampForFileName);
        void setUpRecordingMappedGenericCANMessage(const std::string &timeStampForFileName);

        odcore::data::Container addCANTimeStamp(odcore::data::Container &c, const odcore::data::TimeStamp &ts);
        void dumpASCData(const automotive::GenericCANMessage &gcm);
        void dumpCSVData(odcore::data::Container &c);

        // method from opendlv disabled as not relevant for ps3controller
        //void handleBeacons();

        void disableCANRequests();

        odcore::base::FIFOQueue m_fifoGenericCanMessages;
        std::unique_ptr<odtools::recorder::Recorder> m_recorderGenericCanMessages;

        odcore::base::FIFOQueue m_fifoMappedCanMessages;
        std::unique_ptr<odtools::recorder::Recorder> m_recorderMappedCanMessages;

        std::shared_ptr<automotive::odcantools::CANDevice> m_device;
        std::unique_ptr<CanMessageDataStore> m_canMessageDataStore;

        canmapping::CanMapping m_revereFh16CanMessageMapping;

        odcore::data::TimeStamp m_startOfRecording;
        std::shared_ptr<std::fstream> m_ASCfile;
        std::map<uint32_t, std::shared_ptr<std::fstream>> m_mapOfCSVFiles;
        std::map<uint32_t, std::shared_ptr<odcore::reflection::CSVFromVisitableVisitor>> m_mapOfCSVVisitors;
};
} // proxy
} // system
} // core
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYFH16TRUCK_H*/
