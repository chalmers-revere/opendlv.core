/**
 * Copyright (C) 2017 Ola Benderius, Christian Berger
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

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "ProxyV2xOut.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyV2xOut::ProxyV2xOut(const int &argc, char **argv)
  : DataTriggeredConferenceClientModule(argc, argv, "proxy-v2xout")
    , m_filterMessageIds()
    , m_senderId()
    , m_interfaceIndex(0)
    , m_sourceAddress()
    , m_rawEthernetSocket(-1)
{
}

ProxyV2xOut::~ProxyV2xOut()
{
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
  m_senderId = getKeyValueConfiguration().getValue<uint32_t>("proxy-v2xout.sender-id");
  std::string const networkAdapterName = getKeyValueConfiguration().getValue<std::string>("proxy-v2xout.adapter");

  std::string const filterMessageIdsString = getKeyValueConfiguration().getValue<std::string>("proxy-v2xout.filter-message-ids");
  auto const filterMessageIdsStringVec = odcore::strings::StringToolbox::split(filterMessageIdsString, ',');
  for (std::string const filterMessageIdString : filterMessageIdsStringVec) {
    uint32_t const filterMessageId = std::stoul(filterMessageIdString);
    m_filterMessageIds.push_back(filterMessageId);
  }

  // Creating the raw socket to send data.
  {

    if (!((m_rawEthernetSocket = ::socket(AF_PACKET, SOCK_RAW, htons(ProxyV2xOut::GEO_NETWORKING))) < 0)) {
        struct ifreq bufferInterface;
        ::memset(&bufferInterface, 0, sizeof(bufferInterface));

        ::strncpy(bufferInterface.ifr_name, networkAdapterName.c_str(), IFNAMSIZ);
        if (!(::ioctl(m_rawEthernetSocket, SIOCGIFINDEX, &bufferInterface) < 0)) {
            m_interfaceIndex = bufferInterface.ifr_ifindex;

            if (!(::ioctl(m_rawEthernetSocket, SIOCGIFHWADDR, &bufferInterface) < 0)) {
                ::memcpy((void*)m_sourceAddress, (void*)(bufferInterface.ifr_hwaddr.sa_data), ETH_ALEN);
            }
            else {
                std::stringstream warning;
                warning << "[" << getName() << "] Could not get interface address: " << ::strerror(errno) << std::endl;
                toLogger(odcore::data::LogMessage::LogLevel::WARN, warning.str());
                ::close(m_rawEthernetSocket);
                m_rawEthernetSocket = -1;
            }
        }
        else {
            std::stringstream warning;
            warning << "[" << getName() << "] Could not get interface index: " << ::strerror(errno) << std::endl;
            toLogger(odcore::data::LogMessage::LogLevel::WARN, warning.str());
            ::close(m_rawEthernetSocket);
            m_rawEthernetSocket = -1;
        }
    }
    else {
        std::stringstream warning;
        warning << "[" << getName() << "] Failed to create raw socket: " << ::strerror(errno) << std::endl;
        toLogger(odcore::data::LogMessage::LogLevel::WARN, warning.str());
        m_rawEthernetSocket = -1;
    }
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
  if (!(m_rawEthernetSocket < 0)) {
    unsigned char ethernetBroadcast[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    union ProxyV2xOut::rawEthernetFrame frameToSend;
    ::memcpy(frameToSend.field.header.h_dest, ethernetBroadcast, ETH_ALEN);
    ::memcpy(frameToSend.field.header.h_source, m_sourceAddress, ETH_ALEN);
    frameToSend.field.header.h_proto = htons(ProxyV2xOut::GEO_NETWORKING);
    ::memcpy(frameToSend.field.data, data.c_str(), data.length());

    struct sockaddr_ll sendToDetails;
    ::memset((void*)&sendToDetails, 0, sizeof(sendToDetails));
    sendToDetails.sll_family = PF_PACKET;
    sendToDetails.sll_halen = ETH_ALEN;
    sendToDetails.sll_ifindex = m_interfaceIndex;
    ::memcpy((void*)(sendToDetails.sll_addr), (void*)ethernetBroadcast, ETH_ALEN);

    const unsigned int frameLength = data.length() + ETH_HLEN;
    if (!(::sendto(m_rawEthernetSocket, frameToSend.rawBuffer, frameLength, 0, (struct sockaddr*)&sendToDetails, sizeof(sendToDetails)) > 0)) {
        std::stringstream warning;
        warning << "[" << getName() << "] Error sending raw frame: " << ::strerror(errno) << std::endl;
        toLogger(odcore::data::LogMessage::LogLevel::WARN, warning.str());
    }
    else {
        if (isVerbose()) {
            std::cout << "Broadcasting message " << messageId << " with sender id " << m_senderId << std::endl;
        }
    }
  }
}

}
}
}
}
