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

CanMessageDataStore::CanMessageDataStore(shared_ptr< automotive::odcantools::CANDevice > canDevice)
    : automotive::odcantools::MessageToCANDataStore(canDevice)
    , m_dataStoreMutex()
    , m_enabled(false)
    , m_overridden(false)
    , m_overrideToggleStatus(true) {
}

void CanMessageDataStore::add(odcore::data::Container &container) {
    odcore::base::Lock l(m_dataStoreMutex);

    /*
  if (container.getDataType() == opendlv::proxy::ControlState::ID()) {
    opendlv::proxy::ControlState controlState = container.getData<opendlv::proxy::ControlState>();
    bool enabledPrevious = m_enabled;

    m_enabled = controlState.getIsAutonomous();

    if (m_overridden && !m_enabled && enabledPrevious) {
      m_overridden = false;
      std::cout << "Reset override flag" << std::endl;
    }

    std::cout << "Enable: " << m_enabled << std::endl;
  }
  else if (container.getDataType() == opendlv::proxy::ControlOverrideState::ID()) {
    opendlv::proxy::ControlOverrideState controlOverrideState = container.getData<opendlv::proxy::ControlOverrideState>();

    bool isOverridden = controlOverrideState.getIsOverridden();
    if (isOverridden) {
      // When override is once set to true, can only be reset when receiving certain ControlState
      m_overridden = true;
    }

    std::cout << "Overridden: " << m_overridden << std::endl;
  }
  else
*/
    // TODO: m_enabled needs to be controlled externally.
    m_enabled = true;

    if (container.getDataType() == opendlv::proxy::ActuationRequest::ID()) {
        opendlv::proxy::ActuationRequest actuationRequest = container.getData< opendlv::proxy::ActuationRequest >();

        const bool isValid = actuationRequest.getIsValid();
        if (!isValid) {
            return;
        }

        const float acceleration = actuationRequest.getAcceleration();
        if (acceleration < 0.0f) {
            opendlv::proxy::reverefh16::BrakeRequest brakeRequest;
            brakeRequest.setEnableRequest(m_enabled);
            brakeRequest.setBrake(acceleration);

            odcore::data::Container brakeRequestContainer(brakeRequest);
            canmapping::opendlv::proxy::reverefh16::BrakeRequest brakeRequestMapping;
            automotive::GenericCANMessage genericCanMessage = brakeRequestMapping.encode(brakeRequestContainer);
            m_canDevice->write(genericCanMessage);
        } else {
            opendlv::proxy::reverefh16::AccelerationRequest accelerationRequest;
            accelerationRequest.setEnableRequest(m_enabled);

            // TODO: map requested acceleration value to acceleration pedal position
            accelerationRequest.setAccelerationPedalPosition(acceleration);

            odcore::data::Container accelerationRequestContainer(accelerationRequest);
            canmapping::opendlv::proxy::reverefh16::AccelerationRequest accelerationRequestMapping;
            automotive::GenericCANMessage genericCanMessage = accelerationRequestMapping.encode(accelerationRequestContainer);
            m_canDevice->write(genericCanMessage);
        }

        const float steering = actuationRequest.getSteering();
        opendlv::proxy::reverefh16::SteeringRequest steeringRequest;
        steeringRequest.setEnableRequest(m_enabled);
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
