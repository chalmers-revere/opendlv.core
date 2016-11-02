/**
 * proxy-relays - Interface to relays.
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

#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include <odvdvehicle/GeneratedHeaders_ODVDVehicle.h>

#include "Gpio.h"
#include "ProxyRelay.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

ProxyRelay::ProxyRelay(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-relay"),
    m_gpio()
{
}

ProxyRelay::~ProxyRelay() 
{
}

void ProxyRelay::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  std::string const path = kv.getValue<std::string>("proxy-relay.gpioSystemPath");

  std::string const valuesString = 
      kv.getValue<std::string>("proxy-relay.values");

  std::vector<bool> values;

  std::vector<std::string> valuesVector = 
      odcore::strings::StringToolbox::split(valuesString, ',');
  for (auto valueString : valuesVector) {
    bool value = static_cast<bool>(std::stoi(valueString));
    values.push_back(value);
  }

  std::string const pinsString = 
      kv.getValue<std::string>("proxy-relay.gpio.pins");

  std::vector<uint16_t> pins;

  std::vector<std::string> pinsVector = 
      odcore::strings::StringToolbox::split(pinsString, ',');
  for (auto pinString : pinsVector) {
    uint16_t pin = std::stoi(pinString);
    pins.push_back(pin);
  }

  m_gpio = std::unique_ptr<Gpio>(new Gpio(path, values, pins));
}

void ProxyRelay::tearDown()
{
}

void ProxyRelay::nextContainer(odcore::data::Container &a_container)
{
  if (a_container.getDataType() == opendlv::proxy::RelayRequest::ID()) {
    opendlv::proxy::RelayRequest request = 
        a_container.getData<opendlv::proxy::RelayRequest>();

    std::string deviceId = request.getDeviceId();

    if (deviceId != getIdentifier()) {
      return;
    }

    bool relayValue = request.getRelayValue();
    uint8_t relayIndex = request.getRelayIndex();

    m_gpio->SetValue(relayIndex, relayValue);
  }
}

}
}
}
} // opendlv::core::system::proxy
