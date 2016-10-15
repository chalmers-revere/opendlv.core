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

#ifndef AXISCAMERA_H_
#define AXISCAMERA_H_

#include <memory>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Camera.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

using namespace std;

/**
 * This class wraps an Axis camera and captures its data into a shared memory segment.
 */
class AxisCamera : public Camera {
   private:
    /**
     * "Forbidden" copy constructor. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the copy constructor.
     *
     * @param obj Reference to an object of this class.
     */
    AxisCamera(const AxisCamera & /*obj*/);

    /**
     * "Forbidden" assignment operator. Goal: The compiler should warn
     * already at compile time for unwanted bugs caused by any misuse
     * of the assignment operator.
     *
     * @param obj Reference to an object of this class.
     * @return Reference to this instance.
     */
    AxisCamera &operator=(const AxisCamera & /*obj*/);

   public:
    /**
     * Constructor.
     *
     * @param name Name of the shared memory segment.
     * @param address IP/Port to AxisCamera.
     * @param username.
     * @param password.
     * @param width Expected image width.
     * @param height Expected image height.
     * @param calibrationFile Name of a .yml file containing the intrinsic and extrinsic calibration parameters.
     * @param debug Show live image feed.
     */
    AxisCamera(const string &name, const string &address, const string &username, const string &password, const uint32_t &width, const uint32_t &height, const string &calibrationFile, const bool &debug);
    virtual ~AxisCamera();

   private:
    virtual bool copyImageTo(char *dest, const uint32_t &size);
    virtual bool isValid() const;
    virtual bool captureFrame();

   private:
    std::unique_ptr<cv::VideoCapture> m_capture;
    cv::Mat m_intrinsicCalibration;
    cv::Mat m_extrinsicCalibration;
    cv::Mat m_image;
    bool m_debug;
};
}
}
}
} // opendlv::core::system::proxy

#endif /*AXISCAMERA_H_*/
