/**
 * ps3controller - Using a PS3 controller to accelerate, brake, and steer a vehicle.
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

#ifndef PS3CONTROLLER_H
#define PS3CONTROLLER_H

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace core {
namespace system {

using namespace std;

/**
 * Using a PS3 controller to accelerate, brake, and steer a vehicle.
 */
class PS3Controller : public odcore::base::module::TimeTriggeredConferenceClientModule {
   private:
    PS3Controller(const PS3Controller & /*obj*/) = delete;
    PS3Controller &operator=(const PS3Controller & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    PS3Controller(const int &argc, char **argv);

    virtual ~PS3Controller();

   private:
    void setUp();
    void tearDown();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();
};
}
}
} // opendlv::core::system

#endif /*PS3CONTROLLER_H*/
