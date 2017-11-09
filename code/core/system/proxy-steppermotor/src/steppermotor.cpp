/**
 * proxy-steppermotor - Interface to steppermotor.
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

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "Steppermotor.h"

namespace opendlv {
namespace core {
namespace system{
namespace proxy{

Steppermotor::Steppermotor(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-steppermotor")
    , m_debug()
    , m_initialised()
    , m_initialValuesDirections()
    , m_path()
    , m_pins()
{
}

Steppermotor::~Steppermotor() 
{
}

void Steppermotor::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  m_debug = (kv.getValue<int32_t>("proxy-steppermotor.debug") == 1);

  m_path = kv.getValue<std::string>("proxy-steppermotor.systemPath");

  std::string const pinsString = 
      kv.getValue<std::string>("proxy-steppermotor.pins");
  std::vector<std::string> pinsVector = 
      odcore::strings::StringToolbox::split(pinsString, ',');


  std::string const initialValuesString = 
      kv.getValue<std::string>("proxy-steppermotor.values");

  std::vector<std::string> initialValuesVector = 
      odcore::strings::StringToolbox::split(initialValuesString, ',');
  
  std::string const initialDirectionsString =
      kv.getValue<std::string>("proxy-steppermotor.directions");

  std::vector<std::string> initialDirectionsVector =
      odcore::strings::StringToolbox::split(initialDirectionsString, ',');

  if (pinsVector.size() == initialValuesVector.size() 
      && pinsVector.size() == initialDirectionsVector.size()) {
    for (uint32_t i = 0; i < pinsVector.size(); i++) {
      uint16_t pin = std::stoi(pinsVector.at(i));
      bool value = static_cast<bool>(std::stoi(initialValuesVector.at(i)));
      std::string direction = initialDirectionsVector.at(i);
      if (direction.compare("out") == 0 || direction.compare("in") == 0) {
        m_pins.push_back(pin);
        m_initialValuesDirections.push_back(std::make_pair(value, direction));
      } else {
        cerr << "[" << getName() << "] " << "Invalid direction for pin " 
            << pin << "." << std::endl;
      }
    }
    if (m_debug) {
      std::cout << "[" << getName() << "] " << "Initialised pins: ";
      for (auto pin : m_pins) {
        std:: cout << pin << " ";
      }
      std::cout << "(Value, direction): ";
      for (auto pair : m_initialValuesDirections) {
        std::cout << "(" << pair.first << "," << pair.second << ") ";
      }
      std::cout << std::endl;
    }
  } else {
    cerr << "[" << getName() 
        << "] Number of pins do not equals to number of values or directions" 
        << std::endl;
  }

  Steppermotor();

  m_initialised = true;
}

void Steppermotor::tearDown()
{
  CloseSteppermotor();
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Steppermotor::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    for (auto pin : m_pins) {
      // std::string direction = GetDirection(pin);
      bool value = GetValue(pin);
      opendlv::proxy::ToggleReading::ToggleState state;
      if (value) {
        state = opendlv::proxy::ToggleReading::On;
      } else {
        state = opendlv::proxy::ToggleReading::Off;
      }
      opendlv::proxy::ToggleReading reading(pin, state);
      odcore::data::Container c(reading);
      getConference().send(c);
    }
    if (m_debug) {
      std::cout << "Number of pins: " << m_pins.size() << std::endl;
      for (auto pin : m_pins) {
        std::cout << "[" << getName() << "] Pin: " << pin 
            << " Direction: " << GetDirection(pin) 
            << " Value: " << GetValue(pin) 
            << "." << std::endl;
      }
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Steppermotor::nextContainer(odcore::data::Container &a_container)
{
  if (!m_initialised) {
    return;
  }
  (void) a_container;
  if (a_container.getDataType() == opendlv::proxy::ToggleRequest::ID()) {
    opendlv::proxy::ToggleRequest request = 
        a_container.getData<opendlv::proxy::ToggleRequest>();
    uint16_t pin = request.getPin();
    bool value = request.getState();
    if (GetDirection(pin).compare("out") == 0) {
      SetValue(pin, value);
    } else {
      cerr << "[" << getName() << "] The requested pin " << pin
          << " is read-only." 
          << std::endl;
    }
  }
}

void Steppermotor::OpenSteppermotor()
{
  std::string filename = m_path + "/export";
  std::ofstream exportFile(filename, std::ofstream::out);
  
  if (exportFile.is_open()) {
    for (auto pin : m_pins) {
      exportFile << pin;
      exportFile.flush();
    }
    Reset();
  } else {
    cerr << "[" << getName() << "] Could not open " << filename << "." 
        << std::endl;
  }
  exportFile.close();
}

void Steppermotor::CloseSteppermotor()
{
  std::string filename = m_path + "/unexport";
  std::ofstream unexportFile(filename, std::ofstream::out);
  
  if (unexportFile.is_open()) {
    for (auto pin : m_pins) {
      unexportFile << pin;
      unexportFile.flush();
    }
  } else {
    cerr << "[" << getName() << "] Could not open " << filename << "." 
        << std::endl;
  }
  unexportFile.close();
}

void Steppermotor::Reset()
{
  for (uint16_t i = 0; i < m_pins.size(); i++) {
    uint16_t pin = m_pins[i];
    bool initialValue = m_initialValuesDirections[i].first;
    std::string initialDirection = m_initialValuesDirections[i].second;
    SetDirection(pin, initialDirection);
    if (initialDirection.compare("out") == 0) {
      SetValue(pin, initialValue);
    }
  }
}

void Steppermotor::SetDirection(uint16_t const a_pin, std::string const a_str)
{
  std::string steppermotorDirectionFilename = m_path + "/steppermotor" + std::to_string(a_pin) 
      + "/direction";

  std::ofstream steppermotorDirectionFile(steppermotorDirectionFilename, std::ofstream::out);
  if (steppermotorDirectionFile.is_open()) {
    steppermotorDirectionFile << a_str;
    steppermotorDirectionFile.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << steppermotorDirectionFilename 
        << "." << std::endl;
  }

  steppermotorDirectionFile.close();
}

std::string Steppermotor::GetDirection(uint16_t const a_pin) const
{
  std::string steppermotorDirectionFilename = m_path + "/steppermotor" + std::to_string(a_pin) 
      + "/direction";
  std::string line;

  std::ifstream steppermotorDirectionFile(steppermotorDirectionFilename, std::ifstream::in);
  if (steppermotorDirectionFile.is_open()) {
    std::getline(steppermotorDirectionFile, line);
    std::string direction = line;
    steppermotorDirectionFile.close();
    return direction;
  } else {
    cerr << "[" << getName() << "] Could not open " << steppermotorDirectionFilename 
        << "." << std::endl;
    steppermotorDirectionFile.close();
    return "";
  }
}

void Steppermotor::SetValue(uint16_t const a_pin, bool const a_value)
{
  std::string steppermotorValueFilename = 
      m_path + "/steppermotor" + std::to_string(a_pin) + "/value";

  std::ofstream steppermotorValueFile(steppermotorValueFilename, std::ofstream::out);
  if (steppermotorValueFile.is_open()) {
    steppermotorValueFile << static_cast<uint16_t>(a_value);
    steppermotorValueFile.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << steppermotorValueFilename 
        << "." << std::endl;
  }
  steppermotorValueFile.close();
}

bool Steppermotor::GetValue(uint16_t const a_pin) const
{
  std::string steppermotorValueFilename = 
      m_path + "/steppermotor" + std::to_string(a_pin) + "/value";
  std::string line;

  std::ifstream steppermotorValueFile(steppermotorValueFilename, std::ifstream::in);
  if (steppermotorValueFile.is_open()) {
    std::getline(steppermotorValueFile, line);
    bool value = (std::stoi(line) == 1);
    steppermotorValueFile.close();
    return value;
  } else {
    cerr << "[" << getName() << "] Could not open " << steppermotorValueFilename 
        << "." << std::endl;
    steppermotorValueFile.close();
    return NULL;
  }
}


void Steppermotor::MoveSteps(uint16_t a_step a_dir) const
{
StepPin;
DirectionPin;

StepState=GetValue(StepPin);
DirectionState=GetValue(DirectionPin);


// check if direction is right
if (DirectionState==a_dir) {
} else {
SetValue=!DirectionState;
}


// change high/low for number of steps
for (uint_32 i=0 i<a_step ++i) {
SetValue=!StepState;
usleep(10);
}

}


}
}
}
}
