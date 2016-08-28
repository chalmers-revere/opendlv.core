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
    : m_conference(conference) {}

ApplanixStringDecoder::~ApplanixStringDecoder() {}

// TODO: To be tested and mapped to data structures as specified in ODVDApplanix.
void ApplanixStringDecoder::nextString(std::string const &data) {
    // TODO: Fill this data structure.
    opendlv::core::sensors::applanix::Grp1Data g1data;
    cout << g1data.toString() << endl;

    bool foundHeader = false;
    bool buffering = false;
    uint32_t PAYLOAD_SIZE = 0;
    uint32_t toRemove = 0;
    string s = data;
    stringstream sstr(s);
    while (s.size() > 8) {
        s = sstr.str();
        if (buffering && (s.size() >= PAYLOAD_SIZE)) {
            sstr.seekg(0);
            GRP1DATA_MSG msg;
            sstr.read(msg.timedist, sizeof(msg.timedist));
            sstr.read((char *)(&(msg.lat)), sizeof(msg.lat));
            sstr.read((char *)(&(msg.lon)), sizeof(msg.lon));
            sstr.read((char *)(&(msg.alt)), sizeof(msg.alt));
            sstr.read((char *)(&(msg.vel_north)), sizeof(msg.vel_north));
            sstr.read((char *)(&(msg.vel_east)), sizeof(msg.vel_east));
            sstr.read((char *)(&(msg.vel_down)), sizeof(msg.vel_down));
            sstr.read((char *)(&(msg.roll)), sizeof(msg.roll));
            sstr.read((char *)(&(msg.pitch)), sizeof(msg.pitch));
            sstr.read((char *)(&(msg.heading)), sizeof(msg.heading));
            sstr.read((char *)(&(msg.wander)), sizeof(msg.wander));
            sstr.read((char *)(&(msg.track)), sizeof(msg.track));
            sstr.read((char *)(&(msg.speed)), sizeof(msg.speed));

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
            buffering = false;
            foundHeader = false;
            const string s2 = s.substr(PAYLOAD_SIZE);
            sstr.seekp(0);
            sstr.seekg(0);
            sstr.str(s2);
            continue;
        }

        if (!foundHeader && s.size() >= 8) {
            // Decode header.
            GRPHDR_MSG hdr;
            sstr.seekg(toRemove);
            sstr.read(hdr.grpstart, sizeof(hdr.grpstart));
            sstr.read((char *)(&(hdr.groupnum)), sizeof(hdr.groupnum));

            if ((string(hdr.grpstart, 4) == "$GRP")) {
                if (hdr.groupnum == 1) {
                    sstr.read((char *)(&(hdr.bytecount)), sizeof(hdr.bytecount));
                    PAYLOAD_SIZE = hdr.bytecount;

                    foundHeader = true;
                    buffering = true;

                    // Remove header.
                    const string s2 = s.substr(toRemove + 8);
                    sstr.seekp(0);
                    sstr.seekg(0);
                    sstr.str(s2);
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
