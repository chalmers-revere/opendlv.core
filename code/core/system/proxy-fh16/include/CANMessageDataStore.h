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

#ifndef PROXY_CANMESSAGEDATASTORE_H
#define PROXY_CANMESSAGEDATASTORE_H

#include <memory>

#include <odcantools/MessageToCANDataStore.h>
#include <opendavinci/odcore/base/Mutex.h>

namespace automotive {
namespace odcantools {
class CANDevice;
}
}

namespace odcore {
namespace data {
class Container;
}
}

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

/**
 * This class maps selected messages to CAN messages.
 */
class CanMessageDataStore : public automotive::odcantools::MessageToCANDataStore {
   public:
    CanMessageDataStore(std::shared_ptr< automotive::odcantools::CANDevice > canDevice, bool readOnly);
    virtual void add(odcore::data::Container &container);

   private:
    void writeToCANDevice(const automotive::GenericCANMessage&);
    odcore::base::Mutex m_dataStoreMutex;
    bool m_readOnlyMode;
    bool m_enabled;
    bool m_overridden;
    bool m_overrideToggleStatus;
};

} // proxy
} // system
} // core
} // opendlv

#endif // CANMESSAGEDATASTOREFH16
