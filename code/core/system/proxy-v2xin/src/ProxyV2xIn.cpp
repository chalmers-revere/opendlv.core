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
#include <sys/select.h>
#include <sys/socket.h>

#include <stdint.h>

#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "ProxyV2xIn.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyV2xIn::ProxyV2xIn(const int &argc, char **argv)
  : TimeTriggeredConferenceClientModule(argc, argv, "proxy-v2xin")
    , m_v2xInStringDecoder()
    , m_interfaceIndex(0)
    , m_sourceAddress()
    , m_rawEthernetSocket(-1)
{
}

ProxyV2xIn::~ProxyV2xIn()
{
}

void ProxyV2xIn::setUp()
{
  std::string const networkAdapterName = getKeyValueConfiguration().getValue<std::string>("proxy-v2xin.adapter");

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

  // Creating the raw socket to receive data.
  {

    if (!((m_rawEthernetSocket = ::socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)) {
        struct ifreq bufferInterface;
        ::memset(&bufferInterface, 0, sizeof(bufferInterface));

        ::strncpy(bufferInterface.ifr_name, networkAdapterName.c_str(), IFNAMSIZ);
        if (!(::ioctl(m_rawEthernetSocket, SIOCGIFINDEX, &bufferInterface) < 0)) {
            m_interfaceIndex = bufferInterface.ifr_ifindex;

            if (!(::ioctl(m_rawEthernetSocket, SIOCGIFHWADDR, &bufferInterface) < 0)) {
                ::memcpy((void*)m_sourceAddress, (void*)(bufferInterface.ifr_hwaddr.sa_data), ETH_ALEN);

                struct sockaddr_ll socketDetails;
                ::memset((void*)&socketDetails, 0, sizeof(socketDetails));
                socketDetails.sll_family = AF_PACKET;
                socketDetails.sll_ifindex = m_interfaceIndex;
                socketDetails.sll_protocol = htons(ETH_P_ALL);
                if(::bind(m_rawEthernetSocket, (struct sockaddr*)&socketDetails, sizeof (socketDetails)) == -1) {
                    std::stringstream warning;
                    warning << "[" << getName() << "] Could not bind socket: " << ::strerror(errno) << std::endl;
                    toLogger(odcore::data::LogMessage::LogLevel::WARN, warning.str());
                    ::close(m_rawEthernetSocket);
                    m_rawEthernetSocket = -1;
                }
                else {
                    std::cout << "[" << getName() << "]: Ready." << std::endl;
                }
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

void ProxyV2xIn::tearDown()
{}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyV2xIn::body() {
  if (m_rawEthernetSocket > -1) {
    fd_set rfds;
    struct timeval timeout;

    const uint16_t BUFFER_MAX = 4096;
    char *buffer = new char[BUFFER_MAX];
    int32_t nbytes = 0;
    while (getModuleState() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(m_rawEthernetSocket, &rfds);

        ::select(m_rawEthernetSocket + 1, &rfds, NULL, NULL, &timeout);
        if (FD_ISSET(m_rawEthernetSocket, &rfds)) {
            nbytes = ::recv(m_rawEthernetSocket, buffer, BUFFER_MAX, MSG_DONTWAIT);
            if (nbytes > 14) {
                uint16_t foundGeoProtocol = *(reinterpret_cast<uint16_t*>(buffer+6+6));
                foundGeoProtocol = ntohs(foundGeoProtocol);

                if (foundGeoProtocol == ProxyV2xIn::GEO_NETWORKING) {
std::cout << getName() << " Received " << nbytes << ", protocol: 0x" << std::hex << (uint32_t)foundGeoProtocol << std::dec << std::endl;
                    // Pass string for decoding the payload.
                    const std::string incomingData(buffer+6+6+2, nbytes-(+6+6+2));
                    if (m_v2xInStringDecoder.get() != nullptr) {
                        m_v2xInStringDecoder->nextString(incomingData);
                    }
                }
            }
        }
    }
    delete [] buffer;

  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}


}
}
}
}
