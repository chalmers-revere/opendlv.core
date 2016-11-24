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

#include <stdint.h>

#include <cstring>
#include <iostream>

#include <opencv2/highgui/highgui.hpp>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "OpenCVCamera.h"

#include "ProxyCamera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;

ProxyCamera::ProxyCamera(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-camera")
    , m_camera() {}

ProxyCamera::~ProxyCamera() {}

void ProxyCamera::setUp() {
    const string NAME = getKeyValueConfiguration().getValue< string >("proxy-camera.camera.name");
    const uint32_t ID = getKeyValueConfiguration().getValue< uint32_t >("proxy-camera.camera.id");
    const uint32_t WIDTH = getKeyValueConfiguration().getValue< uint32_t >("proxy-camera.camera.width");
    const uint32_t HEIGHT = getKeyValueConfiguration().getValue< uint32_t >("proxy-camera.camera.height");
    const uint32_t BPP = getKeyValueConfiguration().getValue< uint32_t >("proxy-camera.camera.bpp");
    const bool DEBUG = getKeyValueConfiguration().getValue< bool >("proxy-camera.camera.debug") == 1;
    const bool FLIPPED = getKeyValueConfiguration().getValue< uint32_t >("proxy-camera.camera.flipped") == 1;

    m_camera = unique_ptr< Camera >(new OpenCVCamera(NAME, ID, WIDTH, HEIGHT, BPP, DEBUG, FLIPPED));
    if (m_camera.get() == NULL) {
        cerr << "[" << getName() << "] No valid camera type defined." << endl;
    }
}

void ProxyCamera::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyCamera::body() {
    uint32_t captureCounter = 0;
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        if (m_camera.get() != NULL) {
            // Capture frame.
            odcore::data::image::SharedImage si = m_camera->capture();
            TimeStamp now;

            // Create container with meta-information about captured frame.
            Container c(si);
            c.setSampleTimeStamp(now);

            // Share container for recording.
            getConference().send(c);

            captureCounter++;
        }
    }
    cout << "[" << getName() << "] Captured " << captureCounter << " frames." << endl;
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
