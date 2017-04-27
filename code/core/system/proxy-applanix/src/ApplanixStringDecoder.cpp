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

void ApplanixStringDecoder::prepareReadingBuffer(std::stringstream &_buffer) {
    const uint32_t GRP_HEADER_SIZE = 8;

    if (_buffer.good()) {
        uint16_t buffer = 0;
        _buffer.read((char *)(&(buffer)), sizeof(buffer));
        buffer = le32toh(buffer);
        m_payloadSize = buffer;

        m_foundHeader = true;
        m_buffering = true;

        // Skip GRP header.
        m_toRemove += GRP_HEADER_SIZE;
    }
}

opendlv::core::sensors::applanix::TimeDistance ApplanixStringDecoder::getTimeDistance(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::TimeDistance timedist;

    if (buffer.good()) {
        double time1 = 0;
        double time2 = 0;
        double distanceTag = 0;
        uint8_t timeTypes = 0;
        uint8_t distanceType = 0;

        m_buffer.read((char *)(&(time1)), sizeof(time1));
        m_buffer.read((char *)(&(time2)), sizeof(time2));
        m_buffer.read((char *)(&(distanceTag)), sizeof(distanceTag));
        m_buffer.read((char *)(&(timeTypes)), sizeof(timeTypes));
        m_buffer.read((char *)(&(distanceType)), sizeof(distanceType));

        timedist.setTime1(time1);
        timedist.setTime2(time2);
        timedist.setDistanceTag(distanceTag);
        timedist.setTime1Type(static_cast<opendlv::core::sensors::applanix::TimeDistance::TimeType>(timeTypes & 0x0F));
        timedist.setTime2Type(static_cast<opendlv::core::sensors::applanix::TimeDistance::TimeType>(timeTypes & 0xF0));
        timedist.setDistanceType(static_cast<opendlv::core::sensors::applanix::TimeDistance::DistanceType>(distanceType));
    }

    return timedist;
}

opendlv::core::sensors::applanix::Grp1Data ApplanixStringDecoder::getGRP1(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp1Data g1Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

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
        uint8_t alignment = 0;
        char pad = 0;

        buffer.read((char *)(&(lat)), sizeof(lat));
        buffer.read((char *)(&(lon)), sizeof(lon));
        buffer.read((char *)(&(alt)), sizeof(alt));
        buffer.read((char *)(&(vel_north)), sizeof(vel_north));
        buffer.read((char *)(&(vel_east)), sizeof(vel_east));
        buffer.read((char *)(&(vel_down)), sizeof(vel_down));
        buffer.read((char *)(&(roll)), sizeof(roll));
        buffer.read((char *)(&(pitch)), sizeof(pitch));
        buffer.read((char *)(&(heading)), sizeof(heading));
        buffer.read((char *)(&(wander)), sizeof(wander));
        buffer.read((char *)(&(track)), sizeof(track));
        buffer.read((char *)(&(speed)), sizeof(speed));
        buffer.read((char *)(&(arate_lon)), sizeof(arate_lon));
        buffer.read((char *)(&(arate_trans)), sizeof(arate_trans));
        buffer.read((char *)(&(arate_down)), sizeof(arate_down));
        buffer.read((char *)(&(accel_lon)), sizeof(accel_lon));
        buffer.read((char *)(&(accel_trans)), sizeof(accel_trans));
        buffer.read((char *)(&(accel_down)), sizeof(accel_down));

        buffer.read((char *)(&(alignment)), sizeof(alignment));
        buffer.read((char *)(&(pad)), sizeof(pad));

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
        g1Data.setTimeDistance(timedist);
        g1Data.setAlignment(alignment);
    }

    return g1Data;
}

opendlv::core::sensors::applanix::Grp2Data ApplanixStringDecoder::getGRP2(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp2Data g2Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        float northposrms = 0;
        float eastposrms = 0;
        float downposrms = 0;
        float northvelrms = 0;
        float eastvelrms = 0;
        float downvelrms = 0;
        float rollrms = 0;
        float pitchrms = 0;
        float headingrms = 0;
        float ellipsoidmajor = 0;
        float ellipsoidminor = 0;
        float ellipsoidorientation = 0;
        uint16_t pad = 0;

        buffer.read((char *)(&(northposrms)), sizeof(northposrms));
        buffer.read((char *)(&(eastposrms)), sizeof(eastposrms));
        buffer.read((char *)(&(downposrms)), sizeof(downposrms));
        buffer.read((char *)(&(northvelrms)), sizeof(northvelrms));
        buffer.read((char *)(&(eastvelrms)), sizeof(eastvelrms));
        buffer.read((char *)(&(downvelrms)), sizeof(downvelrms));
        buffer.read((char *)(&(rollrms)), sizeof(rollrms));
        buffer.read((char *)(&(pitchrms)), sizeof(pitchrms));
        buffer.read((char *)(&(headingrms)), sizeof(headingrms));
        buffer.read((char *)(&(ellipsoidmajor)), sizeof(ellipsoidmajor));
        buffer.read((char *)(&(ellipsoidminor)), sizeof(ellipsoidminor));
        buffer.read((char *)(&(ellipsoidorientation)), sizeof(ellipsoidorientation));

        buffer.read((char *)(&(pad)), sizeof(pad));

        g2Data.setNorthposrms(northposrms);
        g2Data.setEastposrms(eastposrms);
        g2Data.setDownposrms(downposrms);
        g2Data.setNorthvelrms(northvelrms);
        g2Data.setEastvelrms(eastvelrms);
        g2Data.setDownvelrms(downvelrms);
        g2Data.setRollrms(rollrms);
        g2Data.setPitchrms(pitchrms);
        g2Data.setHeadingrms(headingrms);
        g2Data.setEllipsoidmajor(ellipsoidmajor);
        g2Data.setEllipsoidminor(ellipsoidminor);
        g2Data.setEllipsoidorientation(ellipsoidorientation);
        g2Data.setTimeDistance(timedist);
    }

    return g2Data;
}

void ApplanixStringDecoder::nextString(std::string const &data) {
    // Add data to the end...
    m_buffer.seekp(0, std::ios_base::end);
    m_buffer.write(data.c_str(), data.size());

    // ...but always read from the beginning.
    m_buffer.seekg(m_toRemove, std::ios_base::beg);

    const uint32_t GRP_HEADER_SIZE = 8;
    const uint32_t GPR_FOOTER_SIZE = 4;
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

            if (ApplanixStringDecoder::GRP1 == m_nextApplanixMessage) {
                // Decode Applanix GRP1.
                opendlv::core::sensors::applanix::Grp1Data g1Data = getGRP1(m_buffer);

                Container c(g1Data);
                m_conference.send(c);

                // Create generic message.
                opendlv::data::environment::WGS84Coordinate wgs84(g1Data.getLat(), g1Data.getLon());
                Container c2(wgs84);
                m_conference.send(c2);
            }
            else if (ApplanixStringDecoder::GRP2 == m_nextApplanixMessage) {
                // Decode Applanix GRP1.
                opendlv::core::sensors::applanix::Grp2Data g2Data = getGRP2(m_buffer);

                Container c(g2Data);
                m_conference.send(c);
            }
            else {
                // Unknown message.
            }

            // Maintain internal buffer status.
            m_buffering = false;
            m_foundHeader = false;
            m_toRemove += m_payloadSize + GPR_FOOTER_SIZE;
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

            if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 1) ) {
                // Decode "$GRP1" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP1;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 2) ) {
                // Decode "$GRP2" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP2;
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
