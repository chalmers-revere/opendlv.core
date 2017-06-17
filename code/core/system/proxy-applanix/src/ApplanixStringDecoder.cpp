/**
 * proxy-applanix - Interface to GPS/IMU unit Applanix.
 * Copyright (C) 2016-2017 Christian Berger
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
    if (_buffer.good()) {
        uint16_t buffer = 0;
        _buffer.read((char *)(&(buffer)), sizeof(buffer));
        buffer = le32toh(buffer);
        m_payloadSize = buffer;

        m_foundHeader = true;
        m_buffering = true;

        // Skip GRP header.
        m_toRemove += ApplanixStringDecoder::GRP_HEADER_SIZE;
    }
}

opendlv::core::sensors::applanix::TimeDistance ApplanixStringDecoder::getTimeDistance(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::TimeDistance timedist;

    if (buffer.good()) {
        double time1            = 0;
        double time2            = 0;
        double distanceTag      = 0;
        uint8_t timeTypes       = 0;
        uint8_t distanceType    = 0;

        buffer.read((char *)(&(time1)), sizeof(time1));
        buffer.read((char *)(&(time2)), sizeof(time2));
        buffer.read((char *)(&(distanceTag)), sizeof(distanceTag));
        buffer.read((char *)(&(timeTypes)), sizeof(timeTypes));
        buffer.read((char *)(&(distanceType)), sizeof(distanceType));

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

        double lat          = 0;
        double lon          = 0;
        double alt          = 0;
        float vel_north     = 0;
        float vel_east      = 0;
        float vel_down      = 0;
        double roll         = 0;
        double pitch        = 0;
        double heading      = 0;
        double wander       = 0;
        float track         = 0;
        float speed         = 0;
        float arate_lon     = 0;
        float arate_trans   = 0;
        float arate_down    = 0;
        float accel_lon     = 0;
        float accel_trans   = 0;
        float accel_down    = 0;
        uint8_t alignment   = 0;
        uint8_t pad         = 0;

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

        float northposrms           = 0;
        float eastposrms            = 0;
        float downposrms            = 0;
        float northvelrms           = 0;
        float eastvelrms            = 0;
        float downvelrms            = 0;
        float rollrms               = 0;
        float pitchrms              = 0;
        float headingrms            = 0;
        float ellipsoidmajor        = 0;
        float ellipsoidminor        = 0;
        float ellipsoidorientation  = 0;
        uint16_t pad                = 0;

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

opendlv::core::sensors::applanix::GNSSReceiverChannelStatus ApplanixStringDecoder::getGNSSReceiverChannelStatus(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::GNSSReceiverChannelStatus gnss;

    if (buffer.good()) {
        uint16_t SV_PRN                     = 0;
        uint16_t channel_tracking_status    = 0;
        float SV_azimuth                    = 0;
        float SV_elevation                  = 0;
        float SV_L1_SNR                     = 0;
        float SV_L2_SNR                     = 0;

        buffer.read((char *)(&(SV_PRN)), sizeof(SV_PRN));
        SV_PRN = le16toh(SV_PRN);

        buffer.read((char *)(&(channel_tracking_status)), sizeof(channel_tracking_status));
        channel_tracking_status = le16toh(channel_tracking_status);

        buffer.read((char *)(&(SV_azimuth)), sizeof(SV_azimuth));
        buffer.read((char *)(&(SV_elevation)), sizeof(SV_elevation));
        buffer.read((char *)(&(SV_L1_SNR)), sizeof(SV_L1_SNR));
        buffer.read((char *)(&(SV_L2_SNR)), sizeof(SV_L2_SNR));

        gnss.setSV_PRN(SV_PRN);
        gnss.setChannel_tracking_status(channel_tracking_status);
        gnss.setSV_azimuth(SV_azimuth);
        gnss.setSV_elevation(SV_elevation);
        gnss.setSV_L1_SNR(SV_L1_SNR);
        gnss.setSV_L2_SNR(SV_L2_SNR);
    }

    return gnss;
}

opendlv::core::sensors::applanix::Grp3Data ApplanixStringDecoder::getGRP3(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp3Data g3Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        int8_t navigationSolutionStatus = 0;
        uint8_t numberSVTracked         = 0;
        uint16_t channelStatusByteCount = 0;
        float HDOP                      = 0;
        float VDOP                      = 0;
        float DGPS_correction_latency   = 0;
        uint16_t DGPS_reference_ID      = 0;
        uint32_t UTC_week_number        = 0;
        double UTC_time_offset          = 0;
        float GNSS_navigation_latency   = 0;
        float geoidal_separation        = 0;
        uint16_t GNSS_receiver_type     = 0;
        uint32_t GNSS_status            = 0;
        uint16_t pad                    = 0;

        buffer.read((char *)(&(navigationSolutionStatus)), sizeof(navigationSolutionStatus));
        buffer.read((char *)(&(numberSVTracked)), sizeof(numberSVTracked));
        buffer.read((char *)(&(channelStatusByteCount)), sizeof(channelStatusByteCount));
        channelStatusByteCount = le16toh(channelStatusByteCount);

        const uint8_t SIZE_OF_GNSS = 20;
        for (uint8_t i = 0; i < (channelStatusByteCount / SIZE_OF_GNSS); i++) {
            opendlv::core::sensors::applanix::GNSSReceiverChannelStatus gnss = getGNSSReceiverChannelStatus(buffer);
            g3Data.addTo_ListOfChannel_status(gnss);
        }

        buffer.read((char *)(&(HDOP)), sizeof(HDOP));
        buffer.read((char *)(&(VDOP)), sizeof(VDOP));
        buffer.read((char *)(&(DGPS_correction_latency)), sizeof(DGPS_correction_latency));
        buffer.read((char *)(&(DGPS_reference_ID)), sizeof(DGPS_reference_ID));
        DGPS_reference_ID = le16toh(DGPS_reference_ID);
        buffer.read((char *)(&(UTC_week_number)), sizeof(UTC_week_number));
        UTC_week_number = le32toh(UTC_week_number);
        buffer.read((char *)(&(UTC_time_offset)), sizeof(UTC_time_offset));
        buffer.read((char *)(&(GNSS_navigation_latency)), sizeof(GNSS_navigation_latency));
        buffer.read((char *)(&(geoidal_separation)), sizeof(geoidal_separation));
        buffer.read((char *)(&(GNSS_receiver_type)), sizeof(GNSS_receiver_type));
        GNSS_receiver_type = le16toh(GNSS_receiver_type);
        buffer.read((char *)(&(GNSS_status)), sizeof(GNSS_status));
        GNSS_status = le32toh(GNSS_status);

        buffer.read((char *)(&(pad)), sizeof(pad));

        // Set values.
        g3Data.setNavigation_solution_status(navigationSolutionStatus);
        g3Data.setNumber_sv_tracked(numberSVTracked);
        g3Data.setChannel_status_byte_count(channelStatusByteCount);

        g3Data.setHDOP(HDOP);
        g3Data.setVDOP(VDOP);
        g3Data.setDGPS_correction_latency(DGPS_correction_latency);
        g3Data.setDGPS_reference_ID(DGPS_reference_ID);
        g3Data.setUTC_week_number(UTC_week_number);
        g3Data.setUTC_time_offset(UTC_time_offset);
        g3Data.setGNSS_navigation_latency(GNSS_navigation_latency);
        g3Data.setGeoidal_separation(geoidal_separation);
        g3Data.setGNSS_receiver_type(GNSS_receiver_type);
        g3Data.setGNSS_status(GNSS_status);

        g3Data.setTimeDistance(timedist);
    }

    return g3Data;
}

opendlv::core::sensors::applanix::Grp4Data ApplanixStringDecoder::getGRP4(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp4Data g4Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        const uint16_t LENGTH_IMUDATA = 24;
        char imudata[LENGTH_IMUDATA];
        uint8_t datastatus  = 0;
        uint8_t imutype     = 0;
        uint8_t imurate     = 0;
        uint16_t imustatus  = 0;
        uint8_t pad         = 0;

        buffer.read(imudata, LENGTH_IMUDATA);
        buffer.read((char *)(&(datastatus)), sizeof(datastatus));
        buffer.read((char *)(&(imutype)), sizeof(imutype));
        buffer.read((char *)(&(imurate)), sizeof(imurate));
        buffer.read((char *)(&(imustatus)), sizeof(imustatus));
        imustatus = le16toh(imustatus);
        buffer.read((char *)(&(pad)), sizeof(pad));

        g4Data.setImudata(string(imudata, LENGTH_IMUDATA));
        g4Data.setDatastatus(datastatus);
        g4Data.setImutype(imutype);
        g4Data.setImurate(imurate);
        g4Data.setImustatus(imustatus);

        g4Data.setTimeDistance(timedist);
    }

    return g4Data;
}

opendlv::core::sensors::applanix::Grp10001Data ApplanixStringDecoder::getGRP10001(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp10001Data g10001Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        uint16_t GNSS_receiver_type = 0;
        uint32_t reserved           = 0;
        uint16_t byte_count         = 0;
        vector<char> GNSS_receiver_raw_data;
        uint8_t pad                 = 0;

        buffer.read((char *)(&(GNSS_receiver_type)), sizeof(GNSS_receiver_type));
        GNSS_receiver_type = le16toh(GNSS_receiver_type);
        buffer.read((char *)(&(reserved)), sizeof(reserved));
        buffer.read((char *)(&(byte_count)), sizeof(byte_count));
        byte_count = le16toh(byte_count);

        // Reserve storage.
        GNSS_receiver_raw_data.reserve(byte_count);
        buffer.read(&GNSS_receiver_raw_data[0], byte_count);

        // Read padding.
        for(uint8_t paddingToRead = 0; paddingToRead < ((m_payloadSize - TIME_DISTANCE_FIELD_SIZE - sizeof(GNSS_receiver_type) - sizeof(reserved) - sizeof(byte_count) - byte_count - ApplanixStringDecoder::GRP_FOOTER_SIZE)); paddingToRead++) {
            buffer.read((char *)(&(pad)), sizeof(pad));
        }

        g10001Data.setGNSS_receiver_type(GNSS_receiver_type);
        g10001Data.setGNSS_receiver_raw_data(string(&GNSS_receiver_raw_data[0], byte_count));

        g10001Data.setTimeDistance(timedist);
    }

    return g10001Data;
}

opendlv::core::sensors::applanix::Grp10002Data ApplanixStringDecoder::getGRP10002(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp10002Data g10002Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        const uint16_t LENGTH_IMUHEADER = 6;
        char imuheader[LENGTH_IMUHEADER];
        uint16_t byte_count = 0;
        vector<char> imu_raw_data;
        int16_t data_checksum = 0;
        uint8_t pad         = 0;

        buffer.read(imuheader, sizeof(imuheader));
        buffer.read((char *)(&(byte_count)), sizeof(byte_count));
        byte_count = le16toh(byte_count);

        // Reserve storage.
        imu_raw_data.reserve(byte_count);
        buffer.read(&imu_raw_data[0], byte_count);

        buffer.read((char *)(&(data_checksum)), sizeof(data_checksum));
        data_checksum = le16toh(data_checksum);

        // Read padding.
        for(uint8_t paddingToRead = 0; paddingToRead < ((m_payloadSize - TIME_DISTANCE_FIELD_SIZE - LENGTH_IMUHEADER - sizeof(byte_count) - byte_count - sizeof(data_checksum) - ApplanixStringDecoder::GRP_FOOTER_SIZE)); paddingToRead++) {
            buffer.read((char *)(&(pad)), sizeof(pad));
        }

        g10002Data.setImuheader(string(imuheader, LENGTH_IMUHEADER));
        g10002Data.setImu_raw_data(string(&imu_raw_data[0], byte_count));
        g10002Data.setDatachecksum(data_checksum);

        g10002Data.setTimeDistance(timedist);
    }

    return g10002Data;
}

opendlv::core::sensors::applanix::Grp10003Data ApplanixStringDecoder::getGRP10003(std::stringstream &buffer) {
    opendlv::core::sensors::applanix::Grp10003Data g10003Data;

    if (buffer.good()) {
        // Read timedist field.
        opendlv::core::sensors::applanix::TimeDistance timedist = getTimeDistance(buffer);

        uint32_t pps = 0;
        buffer.read((char *)(&(pps)), sizeof(pps));
        pps = le32toh(pps);

        uint16_t pad = 0;
        buffer.read((char *)(&(pad)), sizeof(pad));

        g10003Data.setPulsecount(pps);

        g10003Data.setTimeDistance(timedist);
    }

    return g10003Data;
}

opendlv::core::sensors::applanix::Grp10009Data ApplanixStringDecoder::getGRP10009(std::stringstream &buffer) {
    // Grp10009 message is identical to Grp10001. Thus, re-use the decoder and simply copy the data.
    opendlv::core::sensors::applanix::Grp10001Data g10001Data = getGRP10001(buffer);

    opendlv::core::sensors::applanix::Grp10009Data g10009Data;
    g10009Data.setGNSS_receiver_type(g10001Data.getGNSS_receiver_type());
    g10009Data.setGNSS_receiver_raw_data(g10001Data.getGNSS_receiver_raw_data());
    g10009Data.setTimeDistance(g10001Data.getTimeDistance());

    return g10009Data;
}

void ApplanixStringDecoder::nextString(std::string const &data) {
    // Add data to the end...
    m_buffer.seekp(0, std::ios_base::end);
    m_buffer.write(data.c_str(), data.size());

    // ...but always read from the beginning.
    m_buffer.seekg(m_toRemove, std::ios_base::beg);

    while ((static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove + ApplanixStringDecoder::GRP_HEADER_SIZE) < m_buffer.tellp()) {
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
                // Decode Applanix GRP2.
                opendlv::core::sensors::applanix::Grp2Data g2Data = getGRP2(m_buffer);

                Container c(g2Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP3 == m_nextApplanixMessage) {
                // Decode Applanix GRP3.
                opendlv::core::sensors::applanix::Grp3Data g3Data = getGRP3(m_buffer);

                Container c(g3Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP4 == m_nextApplanixMessage) {
                // Decode Applanix GRP4.
                opendlv::core::sensors::applanix::Grp4Data g4Data = getGRP4(m_buffer);

                Container c(g4Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP10001 == m_nextApplanixMessage) {
                // Decode Applanix GRP10001.
                opendlv::core::sensors::applanix::Grp10001Data g10001Data = getGRP10001(m_buffer);

                Container c(g10001Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP10002 == m_nextApplanixMessage) {
                // Decode Applanix GRP10002.
                opendlv::core::sensors::applanix::Grp10002Data g10002Data = getGRP10002(m_buffer);

                Container c(g10002Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP10003 == m_nextApplanixMessage) {
                // Decode Applanix GRP10003.
                opendlv::core::sensors::applanix::Grp10003Data g10003Data = getGRP10003(m_buffer);

                Container c(g10003Data);
                m_conference.send(c);
            }
            else if (ApplanixStringDecoder::GRP10009 == m_nextApplanixMessage) {
                // Decode Applanix GRP10009.
                opendlv::core::sensors::applanix::Grp10009Data g10009Data = getGRP10009(m_buffer);

                Container c(g10009Data);
                m_conference.send(c);
            }
            else {
                // Unknown message.
            }

            // Maintain internal buffer status.
            m_buffering = false;
            m_foundHeader = false;
            m_toRemove += m_payloadSize + ApplanixStringDecoder::GRP_FOOTER_SIZE;
            m_payloadSize = 0;
            m_nextApplanixMessage = ApplanixStringDecoder::UNKNOWN;
        }

        // Try decoding GRP? header.
        if (     !m_foundHeader
            && (   (static_cast<uint32_t>(m_buffer.tellp())
                 - (static_cast<uint32_t>(m_buffer.tellg()) + m_toRemove))
                    >= ApplanixStringDecoder::GRP_HEADER_SIZE
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
            buffer = le16toh(buffer);
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
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 3) ) {
                // Decode "$GRP3" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP3;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 4) ) {
                // Decode "$GRP4" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP4;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 10001) ) {
                // Decode "$GRP10001" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP10001;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 10002) ) {
                // Decode "$GRP10002" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP10002;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 10003) ) {
                // Decode "$GRP10003" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP10003;
            }
            else if ( (hdr.getGrpstart() == "$GRP") && (hdr.getGroupnum() == 10009) ) {
                // Decode "$GRP10009" messages.
                prepareReadingBuffer(m_buffer);

                // Define the next message to decode.
                m_nextApplanixMessage = ApplanixStringDecoder::GRP10009;
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
