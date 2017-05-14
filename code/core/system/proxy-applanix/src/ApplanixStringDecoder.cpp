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
    , m_buffer()
    , m_foundHeader(false)
    , m_buffering(false)
    , m_payloadSize(0)
    , m_toRemove(0)
    , m_nextApplanixMessage(ApplanixStringDecoder::UNKNOWN) {}

ApplanixStringDecoder::~ApplanixStringDecoder() {}

void ApplanixStringDecoder::nextString(std::string const &data) {
    // Add data to the end...
    m_buffer.seekp(0, std::ios_base::end);
    m_buffer.write(data.c_str(), data.size());

    // ...but always read from the beginning.
    m_buffer.seekg(m_toRemove, std::ios_base::beg);

    const uint32_t GRP_HEADER_SIZE = 8;
    while ((static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove + GRP_HEADER_SIZE) < m_buffer.tellp()) {
        // Wait for more data if put pointer is smaller than expected buffer fill level.
        if (     m_buffering
            && (   (static_cast<uint32_t>(m_buffer.tellp())
                 - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove))
                    < m_payloadSize
               )
           ) {
            break;
        }

        // Enough data available to decode the requested GRP.
        if (     m_buffering
            && (   (static_cast<uint32_t>(m_buffer.tellp())
                 - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove))
                    >= m_payloadSize
               )
           ) {
            // Go to where we need to read from.
            m_buffer.seekg(m_toRemove, std::ios_base::beg);

            // Decode Applanix GRP1.
            if (ApplanixStringDecoder::GRP1 == m_nextApplanixMessage) {
                // Create sensor specific data struct.
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

                // Create generic message.
                opendlv::data::environment::WGS84Coordinate wgs84(lat, lon);
                Container c2(wgs84);
                m_conference.send(c2);
            }
            else {
                // Unknown message.
            }

            // Maintain internal buffer status.
            m_buffering = false;
            m_foundHeader = false;
            m_toRemove += m_payloadSize;
            m_payloadSize = 0;
            m_nextApplanixMessage = ApplanixStringDecoder::UNKNOWN;
        }

        // Try decoding GRP? header.
        if (     !m_foundHeader
            && (   (static_cast<uint32_t>(m_buffer.tellp())
                 - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove))
                    >= GRP_HEADER_SIZE
               )
           ) {
            // Go to where we need to read from.
            m_buffer.seekg(m_toRemove, ios::beg);

            // Decode GRP header.
            opendlv::core::sensors::applanix::internal::GrpHdrMsg hdr;

            char grpstart[4];
            m_buffer.read(grpstart, sizeof(grpstart));
            hdr.setGrpstart(string(grpstart, 4));

            uint16_t buffer = 0;
            m_buffer.read((char *)(&(buffer)), sizeof(buffer));
            buffer = le32toh(buffer);
            hdr.setGroupnum(buffer);

            // Decode "$GRP1" messages.
            if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 1) ) {
                buffer = 0;
                m_buffer.read((char *)(&(buffer)), sizeof(buffer));
                buffer = le32toh(buffer);
                hdr.setBytecount(buffer);
                m_payloadSize = hdr.getBytecount();

                m_foundHeader = true;
                m_buffering = true;

                // Skip GRP header.
                m_toRemove += GRP_HEADER_SIZE;

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP1;
            }
            else {
                // Nothing known found; skip this byte and try again.
                m_toRemove++;
            }
        }
    }

    // Discard unused data from buffer but avoid copying data too often.
    const uint32_t MAX_BUFFER = 32768;
    if ( (m_buffer.tellg() > 0) && (m_toRemove > MAX_BUFFER) ) {
        const string s = m_buffer.str().substr(m_buffer.tellg());
        m_buffer.clear();
        m_buffer.str(s);
        m_buffer.seekp(0, ios::end);
        m_buffer.seekg(0, ios::beg);
        m_toRemove = 0;
    }
}
}
}
}
} // opendlv::core::system::proxy
