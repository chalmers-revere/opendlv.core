/**
 * proxy-fh16truck - Interface to cameras.
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
#include <ctype.h>
#include <cstring>
#include <cmath>
#include <iostream>
#include <vector>

#include "opendavinci/odcore/base/KeyValueConfiguration.h"
#include "opendavinci/odcore/data/Container.h"
#include "opendavinci/odcore/data/TimeStamp.h"
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "opencv2/highgui/highgui.hpp"
#include "OpenCVCamera.h"

#ifdef HAVE_UEYE
    #include "uEyeCamera.h"
#endif

#include "ProxyCamera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;
using namespace odcore::base;
using namespace odcore::data;
using namespace odtools::recorder;

ProxyCamera::ProxyCamera(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-camera"),
    m_recorder(),
    m_camera(),
    m_startOfRecording() {}

ProxyCamera::~ProxyCamera() {}

void ProxyCamera::setUp() {
    // Get configuration data.
    KeyValueConfiguration kv = getKeyValueConfiguration();

    // Create built-in recorder.
    const bool useRecorder = kv.getValue<uint32_t>("proxy-camera.useRecorder") == 1;
    const string CAMERA_NAME = getKeyValueConfiguration().getValue<string>("proxy-camera.camera.name");
    if (useRecorder) {
        // URL for storing containers.
        stringstream recordingURL;
        m_startOfRecording = TimeStamp();
        recordingURL << "file://" << "CID-" << getCID() << "-" << getName() << "-" << CAMERA_NAME << "_" << m_startOfRecording.getYYYYMMDD_HHMMSS_noBlank() << ".rec";
        // Size of memory segments.
        const uint32_t MEMORY_SEGMENT_SIZE = getKeyValueConfiguration().getValue<uint32_t>("global.buffer.memorySegmentSize");
        // Number of memory segments.
        const uint32_t NUMBER_OF_SEGMENTS = getKeyValueConfiguration().getValue<uint32_t>("global.buffer.numberOfMemorySegments");
        // Run recorder in asynchronous mode to allow real-time recording in background.
        const bool THREADING = true;
        // Dump shared images and shared data?
        const bool DUMP_SHARED_DATA = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.recorder.dumpshareddata") == 1;

        m_recorder = unique_ptr<Recorder>(new Recorder(recordingURL.str(), MEMORY_SEGMENT_SIZE, NUMBER_OF_SEGMENTS, THREADING, DUMP_SHARED_DATA));
    }

    // Create the camera grabber.
    const string NAME = getKeyValueConfiguration().getValue<string>("proxy-camera.camera.name");
    string TYPE = getKeyValueConfiguration().getValue<string>("proxy-camera.camera.type");
    std::transform(TYPE.begin(), TYPE.end(), TYPE.begin(), ::tolower);
    const uint32_t ID = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.camera.id");
    const uint32_t WIDTH = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.camera.width");
    const uint32_t HEIGHT = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.camera.height");
    const uint32_t BPP = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.camera.bpp");
    const bool DEBUG = getKeyValueConfiguration().getValue<uint32_t>("proxy-camera.debug") == 1;

    if (TYPE.compare("opencv") == 0) {
        m_camera = unique_ptr<Camera>(new OpenCVCamera(NAME, ID, WIDTH, HEIGHT, BPP, DEBUG));
    }
    if (TYPE.compare("ueye") == 0) {
#ifdef HAVE_UEYE
        m_camera = unique_ptr<Camera>(new uEyeCamera(NAME, ID, WIDTH, HEIGHT, BPP, DEBUG));
#endif
    }

    if (m_camera.get() == NULL) {
        cerr << "No valid camera type defined." << endl;
    }
}

void ProxyCamera::tearDown() {}

void ProxyCamera::distribute(Container c) {
    // Store data to recorder.
    if (m_recorder.get() != NULL) {
        // Time stamp data before storing.
        c.setSampleTimeStamp(TimeStamp());
        m_recorder->store(c);
    }

    // Share data.
    getConference().send(c);
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode ProxyCamera::body() {
    // TODO: Remove me.
    // Test whether OpenCV is found and linked correctly.
    uint32_t captureCounter = 0;
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        // Capture frame.
        if (m_camera.get() != NULL) {
            odcore::data::image::SharedImage si = m_camera->capture();
            Container c(si);
            distribute(c);

            captureCounter++;
        }
    }
    cout << "Proxy-camera: Captured " << captureCounter << " frames." << endl;
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
}
} // opendlv::core::system::proxy
