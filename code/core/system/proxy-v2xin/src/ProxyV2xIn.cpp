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

#include <stdint.h>

#include <iostream>
#include <sstream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "ProxyV2xIn.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyV2xIn::ProxyV2xIn(const int &argc, char **argv)
  : DataTriggeredConferenceClientModule(argc, argv, "proxy-v2xin")
    , m_v2xIn()
    , m_v2xInStringDecoder()
{
}

ProxyV2xIn::~ProxyV2xIn()
{
}

void ProxyV2xIn::setUp()
{
  //std::string const v2xInAddress = getKeyValueConfiguration().getValue<std::string>("proxy-v2xin.address");
  uint32_t const v2xInPort = getKeyValueConfiguration().getValue<uint32_t>("proxy-v2xin.port");

  std::vector<uint32_t> filterSenderIds;
  std::string const filterSenderIdsString = 
    getKeyValueConfiguration().getValue<std::string>(
        "proxy-v2xin.filter-sender-ids");
  auto const filterSenderIdsStringVec = 
    odcore::strings::StringToolbox::split(filterSenderIdsString, ',');
  for (std::string const filterSenderIdString : filterSenderIdsStringVec) {
    uint32_t const filterSenderId = std::stoul(filterSenderIdString);
    filterSenderIds.push_back(filterSenderId);
  }

  std::vector<uint32_t> filterMessageIds;
  std::string const filterMessageIdsString = 
    getKeyValueConfiguration().getValue<std::string>(
        "proxy-v2xin.filter-message-ids");
  auto const filterMessageIdsStringVec = 
    odcore::strings::StringToolbox::split(filterMessageIdsString, ',');
  for (std::string const filterMessageIdString : filterMessageIdsStringVec) {
    uint32_t const filterMessageId = std::stoul(filterMessageIdString);
    filterMessageIds.push_back(filterMessageId);
  }

  m_v2xInStringDecoder = std::unique_ptr<V2xInStringDecoder>(
      new V2xInStringDecoder(getConference(), filterSenderIds, 
        filterMessageIds, isVerbose()));

  try {
    m_v2xIn = std::shared_ptr<odcore::io::udp::UDPReceiver>(
        odcore::io::udp::UDPFactory::createUDPReceiver("0.0.0.0", v2xInPort));

    m_v2xIn->setStringListener(m_v2xInStringDecoder.get());
    m_v2xIn->start();

    if (isVerbose()) {
      std::cout << "Listening on port: " << v2xInPort << std::endl;
    }
  } catch (std::string &exception) {
    std::stringstream info;
    info << "[" << getName() << "] Could not connect to V2xIn: " << exception 
      << std::endl;
    toLogger(odcore::data::LogMessage::LogLevel::INFO, info.str());
  }
}

void ProxyV2xIn::tearDown()
{
  if (m_v2xIn.get() != nullptr) {
    m_v2xIn->stop();
    m_v2xIn->setStringListener(nullptr);
  }
}

void ProxyV2xIn::nextContainer(odcore::data::Container &)
{
}

}
}
}
}
