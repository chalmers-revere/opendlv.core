/**
 * proxy-trimble - Interface to GPS/IMU unit Trimble.
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

#ifndef PROXY_PROXYTRIMBLE_TESTSUITE_H
#define PROXY_PROXYTRIMBLE_TESTSUITE_H

#include "cxxtest/TestSuite.h"

#include <memory>

#include <opendavinci/odcore/io/conference/ContainerConference.h>
#include <opendavinci/odcore/io/conference/ContainerConferenceFactory.h>

#include "odvdtrimble/GeneratedHeaders_ODVDTrimble.h"

// Include local header files.
#include "../include/ProxyTrimble.h"
#include "../include/TrimbleStringDecoder.h"

using namespace std;
using namespace odcore::io::conference;
using namespace opendlv::core::system::proxy;

class MyContainerConference : public ContainerConference {
   public:
    MyContainerConference() : ContainerConference(), m_callCounter(0), m_gps() {}
    virtual void send(odcore::data::Container &container) const {
        m_callCounter++;
        if (container.getDataType() == opendlv::core::sensors::trimble::GpsReading::ID()) {
            m_gps = container.getData<opendlv::core::sensors::trimble::GpsReading>();
            cout << m_gps.toString() << endl;
        }
    }
    mutable uint32_t m_callCounter;
    mutable opendlv::core::sensors::trimble::GpsReading m_gps;
};

class ProxyTrimbleTest : public CxxTest::TestSuite {
   public:
    void setUp() {}

    void tearDown() {}

    void testApplication() {
        MyContainerConference mcc;
        const bool DEBUG = true;
        TrimbleStringDecoder tsd(mcc,DEBUG);

        // TODO: Add Trimble data dump.
        fstream data("../2016-08-28-Trimble.dump", ios::binary | ios::in);

        uint32_t overallCounter = 0;
        while (overallCounter < 50) {
            uint32_t count = 0;
            stringstream sstr;
            while (data.good()) {
                char c = data.get();
                sstr.write(&c, sizeof(c));

                if (++count == 15) break;
            }
            const string s = sstr.str();
            if (s.size() > 0) {
                tsd.nextString(s);
                if (mcc.m_callCounter == 1) {
//                    TS_ASSERT_DELTA(mcc.m_gps.getLatitude(), X1, 1e-8);
//                    TS_ASSERT_DELTA(mcc.m_gps.getLongitude(), Y1, 1e-8);
                }
                if (mcc.m_callCounter == 2) {
//                    TS_ASSERT_DELTA(mcc.m_gps.getLatitude(), X2, 1e-8);
//                    TS_ASSERT_DELTA(mcc.m_gps.getLongitude(), Y2, 1e-8);
                }
                if (mcc.m_callCounter == 3) {
//                    TS_ASSERT_DELTA(mcc.m_gps.getLatitude(), X3, 1e-8);
//                    TS_ASSERT_DELTA(mcc.m_gps.getLongitude(), Y3, 1e-8);
                }
            }
            overallCounter++;
        }
        data.close();
    }
};

#endif /*PROXY_PROXYTRIMBLE_TESTSUITE_H*/
