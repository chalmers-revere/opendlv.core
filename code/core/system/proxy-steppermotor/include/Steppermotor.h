/**
 * proxy-steppermotor - Interface to steppermotor
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

#ifndef PROXY_STEPPERMOTOR_H
#define PROXY_STEPPERMOTOR_H


#include <memory>
#include <string>
#include <vector>
#include <utility>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * Interface to steppermotor
 */
class Steppermotor : public odcore::base::module::TimeTriggeredConferenceClientModule {
 public:
  Steppermotor(const int &, char **);
  Steppermotor(const Gpio &) = delete;
  Steppermotor &operator=(const Steppermotor &) = delete;
  virtual ~Steppermotor();
  virtual void nextContainer(odcore::data::Container &);

 private:
  void setUp();
  void tearDown();
  virtual odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

  void OpenSteppermotor();
  void CloseSteppermotor();
  void Reset();
  void SetDirection(uint16_t const, std::string);
  std::string GetDirection(uint16_t const) const;
  void SetValue(uint16_t const, bool const);
  bool GetValue(uint16_t const) const;

  bool m_debug;
  bool m_initialised;
  std::vector<std::pair<bool, std::string>> m_initialValuesDirections;
  std::string m_path;
  std::vector<uint16_t> m_pins;
};

}
}
}
}
#endif
