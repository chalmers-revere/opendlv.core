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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <iostream>
#include <sstream>
#include <string>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>

#include "odvdapplanix/GeneratedHeaders_ODVDApplanix.h"

#include "ApplanixStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

// see http://docs.ros.org/diamondback/api/applanix/html/applanix_8h_source.html
typedef struct GRPHDR_MSG_ {
    char grpstart[4];
    uint16_t groupnum;
    uint16_t bytecount; /* size includes footer, not header */
} GRPHDR_MSG;

typedef struct GRPFTR_MSG_ {
    uint16_t crc;
    char grpend[2];
} GRPFTR_MSG;

typedef struct GRP1DATA_MSG_ {
    char timedist[26];
    double lat;
    double lon;
    double alt;
    float vel_north;
    float vel_east;
    float vel_down;
    double roll;
    double pitch;
    double heading;
    double wander;
    float track;
    float speed;
    float arate_lon;
    float arate_trans;
    float arate_down;
    float accel_lon;
    float accel_trans;
    float accel_down;
    char alignment;
    char padding;
} GRP1DATA_MSG;


ApplanixStringDecoder::ApplanixStringDecoder(odcore::io::conference::ContainerConference &conference)
    : m_conference(conference)
    , m_buffer() {}

ApplanixStringDecoder::~ApplanixStringDecoder() {}

// TODO: To be tested and mapped to data structures as specified in ODVDApplanix.
void ApplanixStringDecoder::nextString(std::string const &data) {
    static bool foundHeader = false;
    static bool buffering = false;
    static uint32_t PAYLOAD_SIZE = 0;
    static uint32_t toRemove = 0;

    // TODO: Fill this data structure.
    opendlv::core::sensors::applanix::Grp1Data g1data;
    cout << g1data.toString() << endl;

cout << "S1 = " <<  m_buffer.str().size() << endl;
    const string old = m_buffer.str();
    const string newString = old + data;
    m_buffer.str(newString);
    string s = m_buffer.str();
cout << "S2 = " <<  m_buffer.str().size() << endl;

    while ( (s.size() > 8) && (toRemove < s.size())) {
        s = m_buffer.str();

        // Wait for more data.
        if (buffering && (s.size() < PAYLOAD_SIZE)) {
            break;
        }

        if (buffering && (s.size() >= PAYLOAD_SIZE)) {
            m_buffer.seekg(0);
            GRP1DATA_MSG msg;
            m_buffer.read(msg.timedist, sizeof(msg.timedist));
            m_buffer.read((char *)(&(msg.lat)), sizeof(msg.lat));
            m_buffer.read((char *)(&(msg.lon)), sizeof(msg.lon));
            m_buffer.read((char *)(&(msg.alt)), sizeof(msg.alt));
            m_buffer.read((char *)(&(msg.vel_north)), sizeof(msg.vel_north));
            m_buffer.read((char *)(&(msg.vel_east)), sizeof(msg.vel_east));
            m_buffer.read((char *)(&(msg.vel_down)), sizeof(msg.vel_down));
            m_buffer.read((char *)(&(msg.roll)), sizeof(msg.roll));
            m_buffer.read((char *)(&(msg.pitch)), sizeof(msg.pitch));
            m_buffer.read((char *)(&(msg.heading)), sizeof(msg.heading));
            m_buffer.read((char *)(&(msg.wander)), sizeof(msg.wander));
            m_buffer.read((char *)(&(msg.track)), sizeof(msg.track));
            m_buffer.read((char *)(&(msg.speed)), sizeof(msg.speed));

            cout.precision(10);
            cout << ""
                 << msg.lat << " "
                 << msg.lon << " "
                 << msg.alt << " "
                 << msg.roll << " "
                 << msg.pitch << " "
                 << msg.heading << " "
                 << msg.speed << " "
                 << endl;

            const string s2 = s.substr(PAYLOAD_SIZE);
            m_buffer.seekp(0);
            m_buffer.seekg(0);
            m_buffer.str(s2);

            buffering = false;
            foundHeader = false;
            PAYLOAD_SIZE = 0;
            toRemove = 0;
            s = m_buffer.str();
            continue;
        }

        if (!foundHeader && s.size() >= 8) {
            // Decode header.
            GRPHDR_MSG hdr;
            m_buffer.seekg(toRemove);
            m_buffer.read(hdr.grpstart, sizeof(hdr.grpstart));
            m_buffer.read((char *)(&(hdr.groupnum)), sizeof(hdr.groupnum));

            if ((string(hdr.grpstart, 4) == "$GRP")) {
                if (hdr.groupnum == 1) {
                    m_buffer.read((char *)(&(hdr.bytecount)), sizeof(hdr.bytecount));
                    PAYLOAD_SIZE = hdr.bytecount;

                    foundHeader = true;
                    buffering = true;

                    // Remove header.
                    const string s2 = s.substr(toRemove + 8);
                    m_buffer.seekp(0);
                    m_buffer.seekg(0);
                    m_buffer.str(s2);
                    s = m_buffer.str();
                } else {
                    toRemove++;
                }
            } else {
                // Nothing known found; discard one byte.
                toRemove++;
            }
        }
    }
}
}
}
}
} // opendlv::core::system::proxy
