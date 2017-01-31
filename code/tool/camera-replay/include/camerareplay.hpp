/**
 * camera-projection - Tool to find projection matrix of camera.
 * Copyright (C) 2016 Chalmers Revere
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

#ifndef CORE_TOOL_CAMERAREPLAY_HPP_
#define CORE_TOOL_CAMERAREPLAY_HPP_

#include <map>
#include <memory>
// #include <opendavinci/odcore/wrapper/Eigen.h>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/data/Container.h>

// #include "opencv2/video/tracking.hpp"
// #include "opencv2/imgproc/imgproc.hpp"
// #include "opencv2/highgui/highgui.hpp"

#include "videocapture.hpp"

namespace opendlv {
namespace core {
namespace tool {

class CameraReplay
: public odcore::base::module::TimeTriggeredConferenceClientModule{
 public:
  CameraReplay(int32_t const &, char **);
  CameraReplay(CameraReplay const &) = delete;
  CameraReplay &operator=(CameraReplay const &) = delete;
  virtual ~CameraReplay();

 private:
  void setUp();
  void tearDown();

  odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

 private:
  std::unique_ptr<VideoCapture> m_videoCapture;
};

} // tools
} // core
} // opendlv

#endif
