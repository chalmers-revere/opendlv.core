# OpenDLV - Software for driverless vehicles realized with OpenDaVINCI
# Copyright (C) 2016 Chalmers REVERE
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

if(CXXTEST_FOUND)
  file(GLOB thisproject-testsuites "${CMAKE_CURRENT_SOURCE_DIR}/testsuites/*.hpp")

  foreach(testsuite ${thisproject-testsuites})
    string(REPLACE "/" ";" testsuite-list ${testsuite})

    list(LENGTH testsuite-list len)
    math(EXPR lastItem "${len}-1")
    list(GET testsuite-list "${lastItem}" testsuite-short)

    set(CXXTEST_TESTGEN_ARGS ${CXXTEST_TESTGEN_ARGS} --world=${PROJECT_NAME}-${testsuite-short})
    CXXTEST_ADD_TEST(${testsuite-short}-TestSuite ${testsuite-short}-TestSuite.cpp ${testsuite})
    if(UNIX)
      if(   ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
         OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
         OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly") )
        set_source_files_properties(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal -Wno-error=suggest-attribute=noreturn -Wno-switch-default")
      else()
        set_source_files_properties(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "-Wno-effc++ -Wno-float-equal -Wno-switch-default")
      endif()
    endif()
    if(WIN32)
      set_source_files_properties(${testsuite-short}-TestSuite.cpp PROPERTIES COMPILE_FLAGS "")
    endif()
    set_tests_properties(${testsuite-short}-TestSuite PROPERTIES TIMEOUT 3000)
    target_link_libraries(${testsuite-short}-TestSuite ${PROJECT_NAME}lib-static ${LIBRARIES})
  endforeach()
endif(CXXTEST_FOUND)

