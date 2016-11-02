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

#ifndef PROXY_RELAY_GPIO_H_
#define PROXY_RELAY_GPIO_H_

#include <string>
#include <vector>

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

class Gpio {
 public:
  Gpio(std::string, std::vector<bool>, std::vector<uint16_t>);
  Gpio(Gpio const &) = delete;
  Gpio &operator=(Gpio const &) = delete;
  virtual ~Gpio();
  bool IsActive(uint16_t const) const;
  void Reset();
  void SetValue(uint16_t const, bool const);

 private:
  std::string m_path;
  std::vector<bool> m_initialValues;
  std::vector<uint16_t> m_pins;
};

}
}
}
} // opendlv::core::system::proxy

#endif
