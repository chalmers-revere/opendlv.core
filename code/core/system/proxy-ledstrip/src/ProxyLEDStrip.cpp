/**
 * proxy-ledstrip - Interface to the LED strip.
 * Copyright (C) 2016 Chalmers Revere
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
#include <stdint.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/wrapper/SerialPort.h>
#include <opendavinci/odcore/wrapper/SerialPortFactory.h>

#include "odvdopendlvstandardmessageset/GeneratedHeaders_ODVDOpenDLVStandardMessageSet.h"

#include "ProxyLEDStrip.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odcore::wrapper;

ProxyLEDStrip::ProxyLEDStrip(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-ledstrip")
    , m_angle(0.0f)
    , m_R(0)
    , m_G(255)
    , m_B(0) {}

ProxyLEDStrip::~ProxyLEDStrip() {}

void ProxyLEDStrip::setUp() {}

void ProxyLEDStrip::tearDown() {}

void ProxyLEDStrip::nextContainer(odcore::data::Container &c) {
    if (c.getDataType() == opendlv::logic::action::AimPoint::ID()) {
        auto aimPoint = c.getData<opendlv::logic::action::AimPoint>();
        m_angle = aimPoint.getAzimuthAngle();
        cout << "[" << getName() << "] Direction azimuth: " << m_angle << std::endl;
    }
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyLEDStrip::body() {
    const string SERIAL_PORT = "/dev/ttyUSB0";
    //const string SERIAL_PORT = "/dev/ttyACM0"; // this is for the Arduino Uno with spare LED strip
    const uint32_t BAUD_RATE = 115200;

    shared_ptr< SerialPort > serial;
    try {
        serial = shared_ptr< SerialPort >(SerialPortFactory::createSerialPort(SERIAL_PORT, BAUD_RATE));
    }
    catch (string &exception) {
        cerr << "[" << getName() << "] Serial port could not be created: " << exception << endl;
        return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
    }

    // variable for debugging purposes
    //const float increment=0.05;
    // boolean flags for debugging purposes
    //bool test=false, sign=false;

    uint8_t focus = 1;
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        CLOG2 << endl
              << "[" << getName() << "] Start while loop" << endl;

/*
        // debug code
        if(test) { // this is for the Arduino Uno with spare LED strip
            CLOG2<<"angle: "<<m_angle<<" rad"<<std::endl;
            if(sign) {m_angle+=increment;CLOG2<<"adding "<<increment<<endl;}
            else {m_angle-=increment;CLOG2<<"subtracting "<<increment<<endl;}
        }
*/

        // capping the max angle at 45 deg = 0.785398 rad
        if (std::fabs(m_angle) >= 0.785398f) {
            if (m_angle >= 0.0f) {
                m_angle = 0.785398f;
                // for debugging purposes
                //sign=false;
            }
            else {
                m_angle = -0.785398f;
                // for debugging purposes
                //sign=true;
            }
        }

        CLOG2 << "[" << getName() << "] angle: " << m_angle << " rad" << endl;

        // Construct Arduino frame to control the LED strip
        vector< uint8_t > ledRequest;

        /* 
         * "focus" represents the centre of the LED section to be powered. 
         * It is obtained through the transformation of the direction 
         * of the movement angle (capped between [-45,45] deg) 
         * into a percentage value
         */
        focus = round(-m_angle / (45.0f / 180.0f * static_cast< float >(M_PI)) * 50.0f) + 50.0f;

        uint8_t section_size = 50; // the size of the LED section to be powered
        uint8_t fade = 10;         // the number of LEDs to be dimmed at the edge of the lighted section
        uint8_t checksum = 0;      // checksum resulting from the bitwise xor of the payload bytes

/*
        if(sign) focus--;
        else focus++;

        if(focus>=146) {
            sign=true;
        }
        else if(focus<1) {
            sign=false;
        }
*/

        checksum = focus ^ section_size ^ fade ^ m_R ^ m_G ^ m_B;

        uint8_t R = m_R, G = m_G, B = m_B;

        // set the color to green
        if (std::fabs(focus - 50) <= 10) {
            R = 0;
            G = 255;
            B = 0;
        }
        else { // set the color to orange
            R = 200;
            G = 50;
            B = 5;
            // light orange: (180,65,10)
        }

        // Message header: 0xFEDE
        // Message size: 9 bytes
        ledRequest.push_back(0xFE);
        ledRequest.push_back(0xDE);
        ledRequest.push_back(focus);
        ledRequest.push_back(section_size);
        ledRequest.push_back(fade);
        ledRequest.push_back(R);
        ledRequest.push_back(G);
        ledRequest.push_back(B);
        ledRequest.push_back(checksum);

        string command(ledRequest.begin(), ledRequest.end());

        CLOG2 << "[" << getName() << "] Frame : ";
        for (uint8_t i = 0; i < command.size(); ++i) {
            cout << (uint16_t)command.at(i) << " ";
        }
        CLOG2 << " : frame size = " << command.size()
              << " {focus: " << (uint16_t)focus << ", section size: " << (uint16_t)section_size << ", fade: " << (uint16_t)fade
              << ", R: " << (uint16_t)m_R << ", G: " << (uint16_t)m_G << ", B: " << (uint16_t)m_B << "}"
              << ", checksum: " << (uint16_t)checksum << endl;

        serial->send(command);
        cout << "[" << getName() << "] Frame sent." << endl;
    }
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
