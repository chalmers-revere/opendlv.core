/**
 * proxy-camera-axis - Interface to network cameras from Axis.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <string>

//#include "opencv2/imgproc/imgproc_c.h"
//#include "opencv2/imgproc/imgproc.hpp"

//#include <opencv2/core/core.hpp>

#include "AxisCamera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

AxisCamera::AxisCamera(const string &name, const string &address, const string &username, const string &password, const uint32_t &width, const uint32_t &height, const uint32_t &bpp, const bool &debug)
    : Camera(name, width, height, bpp)
    , m_capture(nullptr)
    , m_image()
    , m_debug(debug) {

    const string VIDEO_STREAM_ADDRESS =
        string("http://") + username + ":"
                          + password + "@"
                          + address
                          + "/axis-cgi/mjpg/video.cgi?user=" 
                          + username + "&password="
                          + password + "&channel=0&.mjpg";

    m_capture.reset(new cv::VideoCapture(VIDEO_STREAM_ADDRESS));
    if (m_capture->isOpened()) {
        m_capture->set(CV_CAP_PROP_FRAME_WIDTH, width);
        m_capture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
    } else {
        cerr << "[proxy-camera-axis] Could not open camera '" << VIDEO_STREAM_ADDRESS << "'" << endl;
    }
}

AxisCamera::~AxisCamera() {
    if (m_capture != nullptr) {
        m_capture->release();
        m_capture = nullptr;
    }
}

bool AxisCamera::isValid() const {
    return ((m_capture != nullptr) && m_capture->isOpened());
}

bool AxisCamera::captureFrame() {
    bool retVal = false;
    if (m_capture != nullptr) {
        if (m_capture->read(m_image)) {
            retVal = true;
        }
    }
    return retVal;
}

bool AxisCamera::copyImageTo(char *dest, const uint32_t &size) {
    bool retVal = false;

    if ((dest != NULL) && (size > 0)) {
        ::memcpy(dest, m_image.data, size);

        if (m_debug) {
            cv::imshow("[proxy-camera-axis]", m_image);
            cv::waitKey(10);
        }

        retVal = true;
    }

    return retVal;
}
}
}
}
} // opendlv::core::system::proxy
