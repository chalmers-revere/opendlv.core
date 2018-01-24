# Copyright (C) 2018 Ola Benderius
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

cmake_minimum_required(VERSION 3.5)

project(opendlv-standardmessageset)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake.Modules" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${OPENDAVINCI_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})

include(ExternalProject)
include(FindJava REQUIRED)
include(FindodDataStructureGenerator REQUIRED)

ExternalProject_Add (libodvdopendlvstandardmessageset-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvstandardmessageset
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add (libodvdopendlvstandardmessageset
    DEPENDS libodvdopendlvstandardmessageset-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/ODVDOpenDLVStandardMessageSet.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvstandardmessageset"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvstandardmessageset-prefix/src/libodvdopendlvstandardmessageset-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvstandardmessageset-prefix/src/libodvdopendlvstandardmessageset-build --target ${MAKE_PACKAGE})
