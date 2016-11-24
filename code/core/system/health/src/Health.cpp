/**
 * health - Component to check the health of the runtime environment.
 * Copyright (C) 2016 Christian Berger
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

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/base/Thread.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "odvdvehicle/generated/opendlv/system/HealthStatus.h"

#include "Health.h"

namespace opendlv {
namespace core {
namespace system {

using namespace std;
using namespace odcore::base;

Health::Health(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "health")
    , m_healthScript()
    , m_sleep(60) {}

Health::~Health() {}

void Health::setUp() {
    m_healthScript = getKeyValueConfiguration().getValue< std::string >("health.script");
    m_sleep = getKeyValueConfiguration().getValue< int32_t >("health.sleep");
    m_sleep = (m_sleep < 0) ? 60 : m_sleep;
}

void Health::tearDown() {}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Health::body() {
    while (getModuleStateAndWaitForRemainingTimeInTimeslice() ==
    odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        // Open the script for execution.
        FILE *fp = ::popen(m_healthScript.c_str(), "r");
        if (NULL == fp) {
            cerr << "[" << getName() << "] Could not run " << m_healthScript << std::endl;
        } else {
            opendlv::system::HealthStatus hs;
            char buffer[4096];
            // Read the script's output line by line.
            while (::fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
                std::string s(buffer);
                std::vector< std::string > tokens = odcore::strings::StringToolbox::split(s, '=');
                if (2 == tokens.size()) {
                    // Remove trailing newline.
                    s = tokens.at(1);
                    s = s.substr(0, s.size() - 1);
                    hs.putTo_MapOfStatus(tokens.at(0), s);
                }
            }
            pclose(fp);

            auto pairOfIterators = hs.iteratorPair_MapOfStatus();
            auto it = pairOfIterators.first;
            while (it != pairOfIterators.second) {
                std::cout << it->first << ":" << it->second << std::endl;
                it++;
            }

            // Broadcast HealthStatus.
            odcore::data::Container c(hs);
            getConference().send(c);

            // Slowing down check.
            odcore::base::Thread::usleepFor(m_sleep * 1000 * 1000);
        }
    }

    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
}
}
} // opendlv::core::system
