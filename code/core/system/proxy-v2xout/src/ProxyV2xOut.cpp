/**
 * Copyright (C) 2017 Ola Benderius
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


#include <cstring>
#include <iostream>
#include <sstream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "ProxyV2xOut.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyV2xOut::ProxyV2xOut(const int &argc, char **argv)
  : TimeTriggeredConferenceClientModule(argc, argv, "proxy-v2xout")
    , m_v2xOut()
    , m_filterMessageIds()
    , m_senderId()
    , m_isPinging()
{
}

ProxyV2xOut::~ProxyV2xOut()
{
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyV2xOut::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() 
      == odcore::data::dmcp::ModuleStateMessage::RUNNING) {

    if (m_isPinging) {
      std::string data = getBinaryString(m_senderId);
      m_v2xOut->send(data);
    }
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
    
std::string ProxyV2xOut::getBinaryString(uint32_t a_i) const
{
  std::vector<unsigned char> bytes(4);
  memcpy(&bytes[0], &a_i, 4);

  std::stringstream stringStream;
  for (uint32_t i = 0; i < 4; i++) {
    stringStream << bytes[i];
  }
  std::string data = stringStream.str();

  return data;
}

void ProxyV2xOut::setUp()
{
  std::string const address = getKeyValueConfiguration().getValue<std::string>("proxy-v2xout.address");
  uint32_t const port = getKeyValueConfiguration().getValue<uint32_t>("proxy-v2xout.port");
  m_senderId = getKeyValueConfiguration().getValue<uint32_t>("proxy-v2xout.sender-id");
  m_isPinging = getKeyValueConfiguration().getValue<bool>("proxy-v2xout.pinging");

  std::string const filterMessageIdsString = 
    getKeyValueConfiguration().getValue<std::string>(
        "proxy-v2xout.filter-message-ids");
  auto const filterMessageIdsStringVec = 
    odcore::strings::StringToolbox::split(filterMessageIdsString, ',');
  for (std::string const filterMessageIdString : filterMessageIdsStringVec) {
    uint32_t const filterMessageId = std::stoul(filterMessageIdString);
    m_filterMessageIds.push_back(filterMessageId);
  }

  try {
    m_v2xOut = std::shared_ptr<odcore::io::udp::UDPSender>(
        odcore::io::udp::UDPFactory::createUDPSender(address, port));
  } catch (std::string &exception) {
    std::stringstream info;
    info << "[" << getName() << "] Failed to create UDPSender: " << exception 
      << std::endl;
    toLogger(odcore::data::LogMessage::LogLevel::INFO, info.str());
  }
}

void ProxyV2xOut::tearDown()
{
}

void ProxyV2xOut::nextContainer(odcore::data::Container &a_container)
{
  uint32_t messageId = a_container.getDataType();
  if (std::find(m_filterMessageIds.begin(), m_filterMessageIds.end(), 
        messageId) == m_filterMessageIds.end()) {
    return;
  } 
      
  std::string senderIdString = getBinaryString(m_senderId);

  std::ostringstream stringStream;
  stringStream << a_container;
  std::string messageString = stringStream.str();

  std::string data = senderIdString + messageString;
  m_v2xOut->send(data);

  if (isVerbose()) {
    std::cout << "Broadcasting message " << messageId << " with sender id " 
      << m_senderId << std::endl;
  }
}

}
}
}
}
