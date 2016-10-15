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

#include <opencv2/imgproc/imgproc.hpp>

#include "AxisCamera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

AxisCamera::AxisCamera(const string &name, const string &address, const string &username, const string &password, const uint32_t &width, const uint32_t &height, const string &calibrationFile, const bool &debug)
    : Camera(name, width, height)
    , m_capture(nullptr)
    , m_intrinsicCalibration()
    , m_extrinsicCalibration()
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

    // Try to read calibration file if camera is present.
    if (m_capture->isOpened()) {
        try {
            cv::FileStorage fileStorage(calibrationFile, cv::FileStorage::READ);
            fileStorage["camera_matrix"] >> m_extrinsicCalibration;
            fileStorage["distortion_coefficients"] >> m_intrinsicCalibration;
            fileStorage.release();
        }
        catch (cv::Exception &ex){
            const char *errorMessage = ex.what();
            cerr << "[proxy-camera-axis] Failed to read calibration file " << calibrationFile <<  ": " << errorMessage << endl;
        }
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
            if (!m_intrinsicCalibration.empty() && !m_extrinsicCalibration.empty()){
                cv::Mat tmpClone = m_image.clone();
                cv::undistort(tmpClone, m_image, m_extrinsicCalibration, m_intrinsicCalibration);
                tmpClone.release();
            }
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
