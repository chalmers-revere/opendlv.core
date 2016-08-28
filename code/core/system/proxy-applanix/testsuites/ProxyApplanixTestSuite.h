/**
 * proxy-applanix - Interface to GPS/IMU unit Applanix.
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

#ifndef PROXY_PROXYAPPLANIX_TESTSUITE_H
#define PROXY_PROXYAPPLANIX_TESTSUITE_H

#include "cxxtest/TestSuite.h"

#include <memory>

#include <opendavinci/odcore/io/conference/ContainerConference.h>
#include <opendavinci/odcore/io/conference/ContainerConferenceFactory.h>

#include "odvdapplanix/GeneratedHeaders_ODVDApplanix.h"

// Include local header files.
#include "../include/ProxyApplanix.h"
#include "../include/ApplanixStringDecoder.h"

using namespace std;
using namespace odcore::io::conference;
using namespace opendlv::core::system::proxy;

class MyContainerConference : public ContainerConference {
   public:
    MyContainerConference() : ContainerConference(), m_callCounter(0), m_g1data() {}
    virtual void send(odcore::data::Container &container) const {
        m_callCounter++;
        if (container.getDataType() == opendlv::core::sensors::applanix::Grp1Data::ID()) {
            m_g1data = container.getData<opendlv::core::sensors::applanix::Grp1Data>();
cout << m_g1data.toString() << endl;
        }
    }
    mutable uint32_t m_callCounter;
    mutable opendlv::core::sensors::applanix::Grp1Data m_g1data;
};

class ProxyApplanixTest : public CxxTest::TestSuite {
   public:
    void setUp() {}

    void tearDown() {}

    void testApplication() {
        MyContainerConference mcc;
        ApplanixStringDecoder asd(mcc);

        fstream data("../2016-08-28-Applanix.dump", ios::binary | ios::in);

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
                asd.nextString(s);
            }
            overallCounter++;
        }
        data.close();
    }
};

#endif /*PROXY_PROXYAPPLANIX_TESTSUITE_H*/
