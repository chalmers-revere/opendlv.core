# proxy-anglesensor - Reading off the analogue input for anglesensors.
# Copyright (C) 2016 Christian Berger
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

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT (opendlv-core-system-proxy-anglesensor)

###########################################################################
# Set the search path for .cmake files.
SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake.Modules" ${CMAKE_MODULE_PATH})

# Add a local CMake module search path dependent on the desired installation destination.
# Thus, artifacts from the complete source build can be given precendence over any installed versions.
IF(UNIX)
    SET (CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
ENDIF()
IF(WIN32)
    SET (CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/CMake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})
ENDIF()

###########################################################################
# Include flags for compiling.
INCLUDE (CompileFlags)

###########################################################################
# Find and configure CxxTest.
INCLUDE (CheckCxxTestEnvironment)

###########################################################################
# Find OpenDaVINCI.
FIND_PACKAGE (OpenDaVINCI REQUIRED)

###########################################################################
# Find ODVDAnglesensor.
FIND_PACKAGE(ODVDOpenDLVStandardMessageSet REQUIRED)


###############################################################################
# Set header files from ODVDAnglesensor.
INCLUDE_DIRECTORIES (SYSTEM ${ODVDOPENDLVSTANDARDMESSAGESET_INCLUDE_DIRS})
# Set header files from OpenDaVINCI.
INCLUDE_DIRECTORIES (SYSTEM ${OPENDAVINCI_INCLUDE_DIRS})
# Set include directory.
INCLUDE_DIRECTORIES(include)

# Set libraries to link against.
set(LIBRARIES ${OPENDAVINCI_LIBRARIES}
              ${ODVDOPENDLVSTANDARDMESSAGESET_LIBRARIES})

###############################################################################
# Build this project.
FILE(GLOB_RECURSE thisproject-sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
ADD_LIBRARY (${PROJECT_NAME}-static STATIC ${thisproject-sources})
ADD_EXECUTABLE (${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/apps/${PROJECT_NAME}.cpp")
TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${PROJECT_NAME}-static ${LIBRARIES}) 

###############################################################################
# Enable CxxTest for all available testsuites.
IF(CXXTEST_FOUND)
    FILE(GLOB thisproject-testsuites "${CMAKE_CURRENT_SOURCE_DIR}/testsuites/*.h")
    
    FOREACH(testsuite ${thisproject-testsuites})
        STRING(REPLACE "/" ";" testsuite-list ${testsuite})

        LIST(LENGTH testsuite-list len)
        MATH(EXPR lastItem "${len}-1")
        LIST(GET testsuite-list "${lastItem}" testsuite-short)

        SET(CXXTEST_TESTGEN_ARGS ${CXXTEST_TESTGEN_ARGS} --world=${PROJECT_NAME}-${testsuite-short})
        CXXTEST_ADD_TEST(${testsuite-short}-TestSuite ${testsuite-short}-TestSuite.cpp ${testsuite})
        IF(UNIX)
            IF( (   ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
                 OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
                 OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly") )
                AND (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") )
                SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal -Wno-error=suggest-attribute=noreturn")
            ELSE()
                SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal")
            ENDIF()
        ENDIF()
        IF(WIN32)
            SET_SOURCE_FILES_PROPERTIES(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "")
        ENDIF()
        SET_TESTS_PROPERTIES(${testsuite-short}-TestSuite PROPERTIES TIMEOUT 3000)
        TARGET_LINK_LIBRARIES(${testsuite-short}-TestSuite ${PROJECT_NAME}-static ${LIBRARIES})
    ENDFOREACH()
ENDIF(CXXTEST_FOUND)

###############################################################################
# Install this project.
INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT opendlv-core)
INSTALL(TARGETS ${PROJECT_NAME}-static DESTINATION lib COMPONENT opendlv-core)
INSTALL(FILES man/${PROJECT_NAME}.1 DESTINATION man/man1 COMPONENT opendlv-core)

# Install header files.
INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION include/opendlv-core-proxy COMPONENT opendlv-core)

