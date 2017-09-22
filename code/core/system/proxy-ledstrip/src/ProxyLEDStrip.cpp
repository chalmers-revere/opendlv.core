/**
 * proxy-ledstrip - Interface to the LED strip.
 * Copyright (C) 2016 Chalmers Revere
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

#include <cmath>
#include <stdint.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "opendavinci/odcore/base/KeyValueConfiguration.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/wrapper/SerialPortFactory.h"
#include "opendavinci/odcore/base/Lock.h"

#include "odvdopendlvdatamodel/generated/opendlv/model/Direction.h"
#include "odvdopendlvdatamodel/generated/opendlv/perception/StimulusDirectionOfMovement.h"

#include "ProxyLEDStrip.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {


ProxyLEDStrip::ProxyLEDStrip(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-ledstrip")
    , m_angle(0.0f)
    , m_mutex()
    , m_serialPort()
    , m_activeLedSize()
    , m_fadeSize()
    , m_R(0)
    , m_G(255)
    , m_B(0) {}

ProxyLEDStrip::~ProxyLEDStrip() {}

void ProxyLEDStrip::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  // "/dev/ttyUSB0";
  const std::string SERIAL_PORT = kv.getValue<std::string>(getName() + ".serialport"); 
  const uint32_t BAUD_RATE = kv.getValue<uint32_t>(getName() + ".baudrate");    
  // the size of the LED section to be powered
  m_activeLedSize = kv.getValue<uint32_t>(getName() + ".activeledsize");
  // the number of LEDs to be dimmed at the edge of the lighted section
  m_fadeSize = kv.getValue<uint32_t>(getName() + ".fadesize"); ;
  try {
    m_serialPort = std::shared_ptr<odcore::wrapper::SerialPort>(odcore::wrapper::SerialPortFactory::createSerialPort(SERIAL_PORT, BAUD_RATE));
  }
  catch (std::string &exception) {
    std::cerr << "[" << getName() << "] Serial port could not be created: " << exception << std::endl;
  }
}

void ProxyLEDStrip::tearDown() {}

void ProxyLEDStrip::nextContainer(odcore::data::Container &a_c) {
  if (a_c.getDataType() 
      == opendlv::perception::StimulusDirectionOfMovement::ID()) {
    opendlv::perception::StimulusDirectionOfMovement stimulusDirectionOfMovement = a_c.getData< opendlv::perception::StimulusDirectionOfMovement >();
    opendlv::model::Direction direction = stimulusDirectionOfMovement.getDesiredDirectionOfMovement();
    odcore::base::Lock l(m_mutex);
    m_angle = direction.getAzimuth();
    if (isVerbose()) {
      std::cout << "[" << getName() << "] Direction azimuth: " << m_angle << std::endl;
    }
  }
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyLEDStrip::body() {
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    odcore::base::Lock l(m_mutex);

    // capping the max angle at 45 deg = 0.785398 rad
    if (std::fabs(m_angle) >= 0.785398f) {
      if (m_angle >= 0.0f) {
        m_angle = 0.785398f;
      }
      else {
        m_angle = -0.785398f;
      }
    }


    // Construct Arduino frame to control the LED strip
    std::vector<uint8_t> ledRequest;
    /* 
     * "focus" represents the centre of the LED section to be powered. 
     * It is obtained through the transformation of the direction 
     * of the movement angle (capped between [-45,45] deg) 
     * into a percentage value
     */
    uint8_t focus = std::round(-m_angle / (45.0f / 180.0f * static_cast< float >(M_PI)) * 50.0f) + 50.0f;


    uint8_t checksum = 0;      // checksum resulting from the bitwise xor of the payload bytes
    checksum = focus ^ m_activeLedSize ^ m_fadeSize ^ m_R ^ m_G ^ m_B;

    uint8_t R = m_R;
    uint8_t G = m_G;
    uint8_t B = m_B;

    // set the color to green
    if (std::fabs(focus - 50) <= 10) {
      R = 0;
      G = 255;
      B = 0;
    }
    else { // set the color to orange
      R = 200;
      G = 50;
      B = 5;
      // light orange: (180,65,10)
    }

    // Message header: 0xFEDE
    // Message size: 9 bytes
    ledRequest.push_back(0xFE);
    ledRequest.push_back(0xDE);
    ledRequest.push_back(focus);
    ledRequest.push_back(m_activeLedSize);
    ledRequest.push_back(m_fadeSize);
    ledRequest.push_back(R);
    ledRequest.push_back(G);
    ledRequest.push_back(B);
    ledRequest.push_back(checksum);

    std::string command(ledRequest.begin(), ledRequest.end());
    m_serialPort->send(command);

    if (isVerbose()) {
      std::cout << "[" << getName() << "] angle: " << m_angle << " rad." << std::endl;
      std::cout << "[" << getName() << "] Frame sent: ";
      for (uint8_t i = 0; i < command.size(); ++i) {
        std::cout << (uint16_t) command.at(i) << " ";
      }
      std::cout << " : frame size = " << command.size()
          << " {focus: " << (uint16_t) focus 
          << ", section size: " << (uint16_t) m_activeLedSize 
          << ", fade: " << (uint16_t) m_fadeSize
          << ", R: " << (uint16_t) m_R 
          << ", G: " << (uint16_t) m_G 
          << ", B: " << (uint16_t) m_B << "}"
          << ", checksum: " << (uint16_t) checksum << std::endl;
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}


}
}
}
} // opendlv::core::system::proxy
