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
#include <opendlv/data/environment/WGS84Coordinate.h>

#include "odvdapplanix/GeneratedHeaders_ODVDApplanix.h"

#include "ApplanixStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::data;

ApplanixStringDecoder::ApplanixStringDecoder(odcore::io::conference::ContainerConference &conference)
    : m_conference(conference)
    , m_buffer() {}

ApplanixStringDecoder::~ApplanixStringDecoder() {}

void ApplanixStringDecoder::nextString(std::string const &data) {
    static bool foundHeader = false;
    static bool buffering = false;
    static uint32_t PAYLOAD_SIZE = 0;
    static uint32_t toRemove = 0;

    const string old = m_buffer.str();
    const string newString = old + data;
    m_buffer.str(newString);
    string s = m_buffer.str();

    while ((s.size() > 8) && ((toRemove + 8) < s.size())) {
        s = m_buffer.str();

        // Wait for more data.
        if (buffering && (s.size() < PAYLOAD_SIZE)) {
            break;
        }

        // Enough data available to decode GRP1.
        if (buffering && (s.size() >= PAYLOAD_SIZE)) {
            m_buffer.seekg(0);
            opendlv::core::sensors::applanix::Grp1Data g1Data;

            char timedist[26];
            m_buffer.read(timedist, sizeof(timedist));

            double lat = 0;
            double lon = 0;
            double alt = 0;
            float vel_north = 0;
            float vel_east = 0;
            float vel_down = 0;
            double roll = 0;
            double pitch = 0;
            double heading = 0;
            double wander = 0;
            float track = 0;
            float speed = 0;
            float arate_lon = 0;
            float arate_trans = 0;
            float arate_down = 0;
            float accel_lon = 0;
            float accel_trans = 0;
            float accel_down = 0;

            m_buffer.read((char *)(&(lat)), sizeof(lat));
            m_buffer.read((char *)(&(lon)), sizeof(lon));
            m_buffer.read((char *)(&(alt)), sizeof(alt));
            m_buffer.read((char *)(&(vel_north)), sizeof(vel_north));
            m_buffer.read((char *)(&(vel_east)), sizeof(vel_east));
            m_buffer.read((char *)(&(vel_down)), sizeof(vel_down));
            m_buffer.read((char *)(&(roll)), sizeof(roll));
            m_buffer.read((char *)(&(pitch)), sizeof(pitch));
            m_buffer.read((char *)(&(heading)), sizeof(heading));
            m_buffer.read((char *)(&(wander)), sizeof(wander));
            m_buffer.read((char *)(&(track)), sizeof(track));
            m_buffer.read((char *)(&(speed)), sizeof(speed));
            m_buffer.read((char *)(&(arate_lon)), sizeof(arate_lon));
            m_buffer.read((char *)(&(arate_trans)), sizeof(arate_trans));
            m_buffer.read((char *)(&(arate_down)), sizeof(arate_down));
            m_buffer.read((char *)(&(accel_lon)), sizeof(accel_lon));
            m_buffer.read((char *)(&(accel_trans)), sizeof(accel_trans));
            m_buffer.read((char *)(&(accel_down)), sizeof(accel_down));

            g1Data.setTimedist(string(timedist, 26));
            g1Data.setLat(lat);
            g1Data.setLon(lon);
            g1Data.setAlt(alt);
            g1Data.setVel_north(vel_north);
            g1Data.setVel_east(vel_east);
            g1Data.setVel_down(vel_down);
            g1Data.setRoll(roll);
            g1Data.setPitch(pitch);
            g1Data.setHeading(heading);
            g1Data.setWander(wander);
            g1Data.setTrack(track);
            g1Data.setSpeed(speed);
            g1Data.setArate_lon(arate_lon);
            g1Data.setArate_trans(arate_trans);
            g1Data.setArate_down(arate_down);
            g1Data.setAccel_lon(accel_lon);
            g1Data.setAccel_trans(accel_trans);
            g1Data.setAccel_down(accel_down);

            Container c(g1Data);
            m_conference.send(c);

            opendlv::data::environment::WGS84Coordinate wgs84(lat, opendlv::data::environment::WGS84Coordinate::NORTH, lon, opendlv::data::environment::WGS84Coordinate::EAST);
// TODO: After upgrading OpenDaVINCI's master with the updated WGS84Coordinate, remove the previous line and activate the following.
//            opendlv::data::environment::WGS84Coordinate wgs84(lat, lon);
            Container c2(wgs84);
            m_conference.send(c2);

            // Reset internal buffer.
            const uint32_t length = s.size();
            const string s2 = s.substr(PAYLOAD_SIZE, length);

            m_buffer.seekp(0);
            m_buffer.seekg(0);
            m_buffer.str(s2);

            buffering = false;
            foundHeader = false;
            PAYLOAD_SIZE = 0;
            toRemove = 0;
            s = m_buffer.str();
        }

        // Try decoding GRP1 header.
        if (!foundHeader && (s.size() >= 8)) {
            // Decode GRP header.
            opendlv::core::sensors::applanix::internal::GrpHdrMsg hdr;

            m_buffer.seekg(toRemove);

            char grpstart[4];
            m_buffer.read(grpstart, sizeof(grpstart));
            hdr.setGrpstart(string(grpstart, 4));

            uint16_t buffer = 0;
            m_buffer.read((char *)(&(buffer)), sizeof(buffer));
            buffer = le32toh(buffer);
            hdr.setGroupnum(buffer);

            if (hdr.getGrpstart() == "$GRP") {
                if (hdr.getGroupnum() == 1) {
                    buffer = 0;
                    m_buffer.read((char *)(&(buffer)), sizeof(buffer));
                    buffer = le32toh(buffer);
                    hdr.setBytecount(buffer);
                    PAYLOAD_SIZE = hdr.getBytecount();

                    foundHeader = true;
                    buffering = true;

                    // Remove GRP header.
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
