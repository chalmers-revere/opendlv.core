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
 
 
/*
    List of buttons and axes.
    Almost each button has an associated axis, 
    in order to sense the pressure applied to the button.
    The buttons generate a binary event, 
    the axes return a value between MIN_AXES_VALUE and MAX_AXES_VALUE.

    To familiarize with the controller buttons/axes and their values, 
    it is advised to use the code and sample program available at https://github.com/drewnoakes/joystick

    Button 4 & Axis 8 = BUTTON ARROW UP
    Button 6 & Axis 10 = BUTTON ARROW DOWN
    Button 7 & Axis ? = BUTTON ARROW LEFT
    Button 5 & Axis 9 = BUTTON ARROW RIGHT

    Button 1 = LEFT ANALOG BUTTON
    Axis 0 = LEFT ANALOG STICK X
    Axis 1 = LEFT ANALOG STICK Y
    Button 2 = RIGHT ANALOG BUTTON
    Axis 2 = RIGHT ANALOG STICK X
    Axis 3 = RIGHT ANALOG STICK Y

    Button 14 & Axis 18 = BUTTON X
    Button 13 & Axis 17 = BUTTON CIRCLE
    Button 12 & Axis 16 = BUTTON TRIANGLE
    Button 15 & Axis 19 = BUTTON SQUARE

    Button 10 & Axis 14 = BUTTON L1
    Button 8 & Axis 12 = BUTTON L2
    Button 11 & Axis 15 = BUTTON R1
    Button 9 & Axis 13 = BUTTON R2

    Button 0 = BUTTON SELECT
    Button 3 = BUTTON START
    Button 16 = BUTTON PS

    Axes 23, 24, 25 = GYROSCOPE AXES
*/

    class PS3Controller : public odcore::base::module::TimeTriggeredConferenceClientModule {
        public:
            PS3Controller(int32_t const &, char **);
            PS3Controller(PS3Controller const &) = delete;
            PS3Controller &operator=(PS3Controller const &) = delete;
            virtual ~PS3Controller();

        private:
            void setUp();
            void tearDown();
            odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();
            void sendActuationRequest(double, double, bool);

        private:
            /** Minimum value of axes range */
            static const short MIN_AXES_VALUE = -32768;
            /** Minimum value of axes range */
            static const short MAX_AXES_VALUE = 32767;

            int m_ps3controllerDevice;
            int *m_axes;
            
            double m_ACCELERATION_MIN = 0;
            double m_ACCELERATION_MAX = 0;
            double m_DECELERATION_MIN = 0;
            double m_DECELERATION_MAX = 0;
            double m_STEERING_MIN = 0;
            double m_STEERING_MAX = 0;
    };
}
}
} // opendlv::core::system

#endif /*PS3CONTROLLER_H*/
