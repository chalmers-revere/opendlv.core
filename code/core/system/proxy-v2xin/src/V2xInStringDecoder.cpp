/**
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

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <opendavinci/odcore/data/Container.h>

#include "V2xInStringDecoder.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

V2xInStringDecoder::V2xInStringDecoder(
    odcore::io::conference::ContainerConference &a_conference,
    std::vector<uint32_t> a_filterSenderIds, 
    std::vector<uint32_t> a_filterMessageIds, bool a_isVerbose) : 
  m_conference(a_conference),
  m_buffer(),
  m_filterMessageIds(a_filterMessageIds),
  m_filterSenderIds(a_filterSenderIds),
  m_isVerbose(a_isVerbose)
{
}

V2xInStringDecoder::~V2xInStringDecoder()
{
}

void V2xInStringDecoder::nextString(std::string const &a_message)
{
  if (a_message.length() < 4) {
    return;
  }

  uint32_t senderId;
  std::vector<unsigned char> bytes(a_message.begin(), a_message.begin() + 4);
  memcpy(&senderId, &bytes[0], 4);
  
  if (a_message.length() == 4) {
    if (m_isVerbose) {
      std::cout << "Received ping from " << senderId << std::endl;
    }
    return;
  }
  
  if (std::find(m_filterSenderIds.begin(), m_filterSenderIds.end(), 
        senderId) == m_filterSenderIds.end()) {
    return;
  }

  std::string const containerString = a_message.substr(4);
  std::istringstream inputStream(containerString);
  
  odcore::data::Container container;
  inputStream >> container;

  if (m_isVerbose) {
    std::cout << "Received container " << (uint32_t)container.getDataType() << ", sent at " << container.getSentTimeStamp().getYYYYMMDD_HHMMSSms() << ", received at " << container.getReceivedTimeStamp().getYYYYMMDD_HHMMSSms() << ", delta = " << (container.getReceivedTimeStamp() - container.getSentTimeStamp()).toMicroseconds() << "us." << std::endl;
  }

  uint32_t messageId = container.getDataType();
  if (std::find(m_filterMessageIds.begin(), m_filterMessageIds.end(), 
        messageId) == m_filterMessageIds.end()) {
    return;
  }

  if (m_isVerbose) {
    std::cout << "Received and injected message " << messageId << " from "
      << senderId << std::endl;
  }
  container.setSenderStamp(senderId);
  m_conference.send(container);
}

}
}
}
}
