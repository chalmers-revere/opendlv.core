/**
 * Copyright (C) 2015 Chalmers REVERE
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

#ifndef LIDAR_LIDARSTRINGDECODER_HPP_
#define LIDAR_LIDARSTRINGDECODER_HPP_

#include <sstream>

#include <opendavinci/odcore/io/StringListener.h>
#include <opendavinci/odcore/io/conference/ContainerConference.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * This class decodes the received string.
 */
class SickStringDecoder : public odcore::io::StringListener {
 public:
  SickStringDecoder(odcore::io::conference::ContainerConference &, const double&, const double&, const double&);
  SickStringDecoder(SickStringDecoder const &) = delete;
  SickStringDecoder &operator=(SickStringDecoder const &) = delete;
  virtual ~SickStringDecoder();

  virtual void nextString(const std::string &);

 private:
  void convertToDistances();
  bool tryDecode();

 private:
  odcore::io::conference::ContainerConference &m_conference;

  bool m_header;
  bool m_startConfirmed;

  double m_position[3];
  unsigned char m_measurements[1000];
  unsigned char m_startResponse[10];
  unsigned char m_measurementHeader[7];
  unsigned char m_centimeterResponse[44];
  std::stringstream m_buffer;
};

}
}
}
}

#endif
