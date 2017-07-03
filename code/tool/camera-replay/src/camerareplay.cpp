/**
 * camera-replay - Tool to replay a video file as camera feed.
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

#include <ctype.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <opencv2/highgui/highgui.hpp>

#include "camerareplay.hpp"

namespace opendlv {
namespace core {
namespace tool {


CameraReplay::CameraReplay(int32_t const &a_argc, char **a_argv)
    : odcore::base::module::TimeTriggeredConferenceClientModule(
      a_argc, a_argv, "core-tool-camera-replay")
    , m_videoCapture()
{
}

CameraReplay::~CameraReplay()
{
}

void CameraReplay::setUp()
{ 
  auto kv = getKeyValueConfiguration();
  const std::string SOURCENAME = kv.getValue<std::string>("core-tool-camera-replay.sourcename");
  const std::string FILEPATH = kv.getValue<std::string>("core-tool-camera-replay.filepath");
  const uint32_t WIDTH = kv.getValue< uint32_t >("core-tool-camera-replay.width");
  const uint32_t HEIGHT = kv.getValue< uint32_t >("core-tool-camera-replay.height");
  const bool DEBUG = kv.getValue< bool >("core-tool-camera-replay.debug");

  m_videoCapture = std::unique_ptr<VideoCapture>(new VideoCapture(SOURCENAME, FILEPATH, WIDTH, HEIGHT, DEBUG));
  if (m_videoCapture.get() == NULL) {
    std::cerr << "[" << getName() << "] No valid video file defined." << std::endl;
  }
}

void CameraReplay::tearDown()
{
}


odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode CameraReplay::body(){
  uint32_t captureCounter = 0;
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() ==
  odcore::data::dmcp::ModuleStateMessage::RUNNING){
    if (m_videoCapture.get() != NULL) {
      // Capture frame.
      odcore::data::image::SharedImage si = m_videoCapture->capture();
      odcore::data::TimeStamp now;

      // Create container with meta-information about captured frame.
      odcore::data::Container c(si);
      c.setSampleTimeStamp(now);

      // Share container for recording.
      getConference().send(c);
      captureCounter++;
    }
  }
  std::cout << "[" << getName() << "] Captured " << captureCounter << " frames." << std::endl;
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

} // tool
} // core
} // opendlv
