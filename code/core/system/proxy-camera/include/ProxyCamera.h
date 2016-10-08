/**
 * proxy-camera - Interface to OpenCV-based cameras.
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

#ifndef PROXY_PROXYCAMERA_H
#define PROXY_PROXYCAMERA_H

#include <map>
#include <memory>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

#include "Camera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

/**
 * Interface to cameras.
 */
class ProxyCamera : public odcore::base::module::TimeTriggeredConferenceClientModule {
   private:
    ProxyCamera(const ProxyCamera & /*obj*/) = delete;
    ProxyCamera &operator=(const ProxyCamera & /*obj*/) = delete;

   public:
    /**
     * Constructor.
     *
     * @param argc Number of command line arguments.
     * @param argv Command line arguments.
     */
    ProxyCamera(const int &argc, char **argv);

    virtual ~ProxyCamera();

   private:
    void setUp();
    void tearDown();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

   private:
    unique_ptr< Camera > m_camera;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*PROXY_PROXYCAMERA_H*/
