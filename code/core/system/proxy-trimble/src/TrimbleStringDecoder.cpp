/**
 * proxy-trimble - Interface to GPS/IMU unit Trimble.
 * Copyright (C) 2016 Chalmers REVERE
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

#include <cmath>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>
#include <opendlv/data/environment/Point3.h>
#include <opendlv/data/environment/WGS84Coordinate.h>

#include "odvdtrimble/GeneratedHeaders_ODVDTrimble.h"

#include "TrimbleStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::data;

TrimbleStringDecoder::TrimbleStringDecoder(odcore::io::conference::ContainerConference &conference, bool debug)
    : m_conference(conference)
    , m_buffer() 
    , m_debug(debug) {}

TrimbleStringDecoder::~TrimbleStringDecoder() {}

void TrimbleStringDecoder::nextString(string const &s) {
    static bool hasOldWGS84 = false;
    static opendlv::data::environment::WGS84Coordinate oldWGS84;
    double timestamp = 0;
    double latitude = 0;
    double longitude = 0;
    float altitude = 0;
    float northHeading = 0;
    float speed = 0;
    uint8_t latitudeDirection = 0;
    uint8_t longitudeDirection = 0;
    uint8_t satelliteCount = 0;
    bool hasHeading = false;
    bool hasRtk = false;

    bool gotGpgga = false;
//    bool gotGpvtg = false;

    vector< string > messages;

    istringstream ss1(s);
    string msg;
    while (getline(ss1, msg, '$')) {
        messages.push_back(msg);
    }

    if (messages.empty()) {
        return;
    }

    for (auto message : messages) {
        if (message.empty()) {
            continue;
        }

        vector< string > fields;

        istringstream ss2(message);
        string field;
        while (getline(ss2, field, ',')) {
            fields.push_back(field);
        }

        if (fields.empty()) {
            continue;
        }

        string type = fields.at(0);
        if(m_debug){
            for(auto qq : fields){
              cout << qq << ", ";
            }
            cout << endl;
        }

        if (type == "GPGGA") {
            // TODO: FIX AND REMOVE try.
            try {
                gotGpgga = true;

                timestamp = stod(fields.at(1));

                latitude = stod(fields.at(2));
                latitudeDirection = fields.at(3)[0];

                longitude = stod(fields.at(4));
                longitudeDirection = fields.at(5)[0];

                // Convert from format dd mm,mmmm
                latitude = latitude / 100.0;
                longitude = longitude / 100.0;
                latitude = static_cast< int >(latitude) + (latitude - static_cast< int >(latitude)) * 100.0 / 60.0;
                longitude = static_cast< int >(longitude) + (longitude - static_cast< int >(longitude)) * 100.0 / 60.0;

                // 0: Non valid, 1: GPS fix, 2: DGPS fix, 4: RTK fix int, 5: RTK float int
                const int gpsQuality = stoi(fields.at(6));
                hasRtk = (gpsQuality == 4 || gpsQuality == 5);

                satelliteCount = stoi(fields.at(7));

                //   float hdop = stof(fields.at(8));

                altitude = stof(fields.at(9));

                //string altitudeUnit = fields.at(10);

                //    float geoidSeparation = stof(fields.at(11));
                //    string geodSeparationUnit = fields.at(12);
            }
            catch (...) {
                cout << "WARNING: Read error for GPGGA." << endl;
            }
        }
        else if (type == "GPVTG") {
// The Trimble unit does not support heading without two antennas.
//            try {
//                gotGpvtg = true;

//                string headingStr = fields.at(1);

//                // cout << "HeadingStr: " << headingStr << endl;

//                if (headingStr.empty()) {
//                    northHeading = 0.0f;
//                    hasHeading = false;
//                }
//                else {
//                    northHeading = stod(headingStr) * M_PI / 180.0;
//                    hasHeading = true;
//                }

//                // Convert to m/s.
//                speed = stof(fields.at(7)) / 3.6f;
//            }
//            catch (...) {
//                cout << "WARNING: Read error for GPVTG." << endl;
//            }
        } else if (type == "GPHDT") {
        } else if (type == "GPRMC") {
        } else {
            cout << "[proxy-trimble] WARNING: Unknown packet type. " << type << endl;
        }
    }
    if(m_debug){
        cout << "[proxy-trimble] GPS sending signals : Latitude : " << latitude << setprecision(19)
              << " Longitude : " << setprecision(19) << longitude << " Heading: " << northHeading << " hasRtk: " << hasRtk << endl;
    }

    if (gotGpgga/* && gotGpvtg*/) {
        // Calculate heading using two consecutive GPS position.
        opendlv::data::environment::WGS84Coordinate wgs84(latitude, longitude);
        if (hasOldWGS84) {
            // Set oldWGS84 coordinate as reference, transform wgs84 into Cartesian frame, and compute heading.
            opendlv::data::environment::Point3 p = oldWGS84.transform(wgs84);
            northHeading = p.getAngleXY() - M_PI*0.5;
            hasHeading = true;
            oldWGS84 = wgs84;
        }
        hasOldWGS84 = true;


        opendlv::core::sensors::trimble::GpsReading gps(timestamp,
                                                        latitude,
                                                        longitude,
                                                        altitude,
                                                        northHeading,
                                                        speed,
                                                        latitudeDirection,
                                                        longitudeDirection,
                                                        satelliteCount,
                                                        hasHeading,
                                                        hasRtk);
        Container c(gps);
        m_conference.send(c);

        // Distribute WGS84 coordinate.
        Container c2(wgs84);
        m_conference.send(c2);
    }
}
}
}
}
} // opendlv::core::system::proxy
