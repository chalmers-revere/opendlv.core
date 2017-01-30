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

#ifndef CORE_TOOL_VIDEOCAPTURE_HPP_
#define CORE_TOOL_VIDEOCAPTURE_HPP_


#include <stdint.h>

#include <memory>
#include <string>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opendavinci/GeneratedHeaders_OpenDaVINCI.h>
#include <opendavinci/odcore/wrapper/SharedMemory.h>


namespace opendlv {
namespace core {
namespace tool {

class VideoCapture
{
 public:
  VideoCapture(const std::string &sourcename, const std::string &filename, const uint32_t &width, const uint32_t &height, const bool &debug);
  VideoCapture(VideoCapture const &) = delete;
  VideoCapture &operator=(VideoCapture const &) = delete;
  virtual ~VideoCapture();

  odcore::data::image::SharedImage capture();

 private:
  /**
  * This method is responsible to copy the image from the
  * specific camera driver to the shared memory.
  *
  * @param dest Pointer where to copy the data.
  * @param size Number of bytes to copy.
  * @return true if the data was successfully copied.
  */
  virtual bool copyImageTo(char *dest, const uint32_t &size);
  virtual bool isValid() const;
  virtual bool captureFrame();

  const std::string getSourcename() const;
  uint32_t getWidth() const;
  uint32_t getHeight() const;
  uint32_t getSize() const;

 private:
  odcore::data::image::SharedImage m_sharedImage;
  std::shared_ptr<odcore::wrapper::SharedMemory> m_sharedMemory;

  std::string m_sourcename;
  std::string m_filename;
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_size;

  std::unique_ptr<cv::VideoCapture> m_capture;
  cv::Mat m_image;
  bool m_debug;
};

} // tools
} // core
} // opendlv

#endif
