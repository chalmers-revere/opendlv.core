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

#include <iostream>


#include <opendavinci/odcore/base/Lock.h>
#include <opendavinci/odcore/wrapper/SharedMemoryFactory.h>

#include "videocapture.hpp"

namespace opendlv {
namespace core {
namespace tool {


VideoCapture::VideoCapture(const std::string &sourcename, const std::string &filepath, const uint32_t &width, const uint32_t &height, const bool &debug)
  : m_sharedImage()
  , m_sharedMemory()
  , m_sourcename(sourcename)
  , m_filename(filepath)
  , m_width(width)
  , m_height(height)
  , m_size(0)
  , m_capture(nullptr)
  , m_image()
  , m_debug(debug)
{

  const uint8_t BPP = 3;
  m_size = width * height * BPP;

  m_sharedMemory = odcore::wrapper::SharedMemoryFactory::createSharedMemory(sourcename, m_size);
  m_sharedImage.setName(sourcename);
  m_sharedImage.setSize(m_size);
  m_sharedImage.setWidth(width);
  m_sharedImage.setHeight(height);
  //Is the BPP really 3?, isn't it channels per pixels?
  m_sharedImage.setBytesPerPixel(BPP);

  m_capture.reset(new cv::VideoCapture(filepath));
  if (m_capture->isOpened()) {
    m_capture->set(CV_CAP_PROP_FRAME_WIDTH, width);
    m_capture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
    std::cout << "[tools-camerareplay] Successfully opened '"<< filepath << "'." << std::endl;
  } else {
    std::cerr << "[tools-camerareplay] Could not open file: '" << filepath << "'" << std::endl;
  }
}

VideoCapture::~VideoCapture()
{
  if (m_capture != nullptr) {
    m_capture->release();
    m_capture = nullptr;
  }
  m_image.release();
}

const std::string VideoCapture::getSourcename() const {
  return m_sourcename;
}

uint32_t VideoCapture::getWidth() const {
  return m_width;
}

uint32_t VideoCapture::getHeight() const {
  return m_height;
}

uint32_t VideoCapture::getSize() const {
  return m_size;
}

bool VideoCapture::isValid() const {
  return ((m_capture != nullptr) && m_capture->isOpened());
}

odcore::data::image::SharedImage VideoCapture::capture() {
  if (isValid()) {
    if (captureFrame()) {
      if (m_sharedMemory.get() && m_sharedMemory->isValid()) {
        odcore::base::Lock l(m_sharedMemory);
        copyImageTo(static_cast<char*>(m_sharedMemory->getSharedMemory()), m_size);
      }
    }
  }
  return m_sharedImage;
}

bool VideoCapture::captureFrame() {
  bool retVal = false;
  if (m_capture != nullptr && m_capture->read(m_image)) {
    retVal = true;
  }
  return retVal;
}


bool VideoCapture::copyImageTo(char *dest, const uint32_t &size) {
  bool retVal = false;
  if ((dest != NULL) && (size > 0)) {
    ::memcpy(dest, m_image.data, size);
    if (m_debug) {
      cv::imshow("[Video feed]", m_image);
      cv::waitKey(10);
    }
    retVal = true;
  }
  return retVal;
}

} // tool
} // core
} // opendlv
