/**
 * Copyright (C) 2016 Chalmers REVERE
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <iostream>

#include <fh16mapping/GeneratedHeaders_fh16mapping.h>
#include <odcantools/CANDevice.h>
#include <opendavinci/odcore/base/Lock.h>
#include <opendavinci/odcore/data/Container.h>

#include <odvdvehicle/GeneratedHeaders_ODVDVehicle.h> // for ActuationRequest

#include "CANMessageDataStore.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

CanMessageDataStore::CanMessageDataStore(shared_ptr<automotive::odcantools::CANDevice> a_canDevice, bool a_enableActuationBrake, bool a_enableActuationSteering, bool a_enableActuationThrottle)
    : automotive::odcantools::MessageToCANDataStore(a_canDevice)
    , m_dataStoreMutex()
    , m_enabledActuationBrake(a_enableActuationBrake)
    , m_enabledActuationSteering(a_enableActuationSteering)
    , m_enabledActuationThrottle(a_enableActuationThrottle)
{
}

void CanMessageDataStore::add(odcore::data::Container &a_container) {
    odcore::base::Lock l(m_dataStoreMutex);

    if (a_container.getDataType() == opendlv::proxy::ActuationRequest::ID()) {
        auto actuationRequest = a_container.getData<opendlv::proxy::ActuationRequest>();

        bool const isValid = actuationRequest.getIsValid();

        float const acceleration = actuationRequest.getAcceleration();
        if (acceleration < 0.0f) {
            bool brakeEnabled = false;
            if (m_enabledActuationBrake && isValid) {
                brakeEnabled = true;
            }

            opendlv::proxy::reverefh16::BrakeRequest brakeRequest;
            brakeRequest.setEnableRequest(brakeEnabled);

            float const max_deceleration = 2.0f;
            if (acceleration < -max_deceleration) {
                if (brakeEnabled) {
                  std::cout << "WARNING: Deceleration was limited to " 
                    << max_deceleration << ". This should never happen, and "
                    << "may be a safety violating behaviour!" 
                    << std::endl;
                }
                brakeRequest.setBrake(-max_deceleration);
            } else {
                brakeRequest.setBrake(acceleration);
            }

            odcore::data::Container brakeRequestContainer(brakeRequest);
            canmapping::opendlv::proxy::reverefh16::BrakeRequest brakeRequestMapping;
            automotive::GenericCANMessage genericCanMessage = brakeRequestMapping.encode(brakeRequestContainer);
            m_canDevice->write(genericCanMessage);
        } else {
            bool throttleEnabled = false;
            if (m_enabledActuationThrottle && isValid) {
                throttleEnabled = true;
            }

            opendlv::proxy::reverefh16::AccelerationRequest accelerationRequest;
            accelerationRequest.setEnableRequest(throttleEnabled);
            accelerationRequest.setAccelerationPedalPosition(acceleration);

            odcore::data::Container accelerationRequestContainer(accelerationRequest);
            canmapping::opendlv::proxy::reverefh16::AccelerationRequest accelerationRequestMapping;
            automotive::GenericCANMessage genericCanMessage = accelerationRequestMapping.encode(accelerationRequestContainer);
            m_canDevice->write(genericCanMessage);
        }
            
        bool steeringEnabled = false;
        if (m_enabledActuationSteering && isValid) {
            steeringEnabled = true;
        }

        const float steering = actuationRequest.getSteering();
        opendlv::proxy::reverefh16::SteeringRequest steeringRequest;
        steeringRequest.setEnableRequest(steeringEnabled);
        steeringRequest.setSteeringRoadWheelAngle(steering);

        // Must be 33.535 to disable deltatorque.
        steeringRequest.setSteeringDeltaTorque(33.535);
        odcore::data::Container steeringRequestContainer(steeringRequest);

        canmapping::opendlv::proxy::reverefh16::SteeringRequest steeringRequestMapping;
        automotive::GenericCANMessage genericCanMessage = steeringRequestMapping.encode(steeringRequestContainer);
        m_canDevice->write(genericCanMessage);
    }
}

} // proxy
} // system
} // core
} // opendlv
