/**
 * health - Component to check the health of the runtime environment.
 * Copyright (C) 2016 Christian Berger
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

#ifndef HEALTH_H
#define HEALTH_H

#include <string>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace core {
namespace system {

using namespace std;

/**
 * This component checks the health of the runtime environment.
 */
class Health : public odcore::base::module::TimeTriggeredConferenceClientModule {
   public:
    Health(int32_t const &, char **);
    Health(Health const &) = delete;
    Health &operator=(Health const &) = delete;
    virtual ~Health();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

   private:
    void setUp();
    void tearDown();

   private:
    std::string m_healthScript;
    int32_t m_sleep;
};
}
}
} // opendlv::core::system

#endif /*HEALTH_H*/
