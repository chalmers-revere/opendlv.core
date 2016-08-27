/**
 * proxy-applanix - Interface to GPS/IMU unit Applanix.
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

#ifndef PROXY_APPLANIX_H
#define PROXY_APPLANIX_H

#include <string>
#include <vector>

#include "opendavinci/odcore/opendavinci.h"

namespace opendlv {
namespace core {
namespace system {
namespace proxy {

    using namespace std;

    /**
     * Interface to GPS/IMU unit Applanix.
     */
    class ProxyApplanix {
        private:
            ProxyApplanix(const ProxyApplanix &/*obj*/) = delete;
            ProxyApplanix& operator=(const ProxyApplanix &/*obj*/) = delete;

        public:
            /**
             * Constructor.
             */
            ProxyApplanix();

            virtual ~ProxyApplanix();

            /**
             * This method runs odfilter.
             *
             * @param argc Number of command line arguments.
             * @param argv Command line arguments.
             * @return 0 if the filter is successful, 1 if both, the keep and the drop parameters are specified.
             */
            int32_t run(const int32_t &argc, char **argv);

        private:
            void parseAdditionalCommandLineParameters(const int &argc, char **argv);

            /**
             * This method returns a sorted vector with unique numerical values
             * extracted from a comma-separated list of numbers.
             *
             * @param s Comma-separated list of numbers.
             * @return vector containing sorted unique numerical values.
             */
            vector<uint32_t> getListOfNumbers(const string &s);

        private:
            vector<uint32_t> m_keep;
            vector<uint32_t> m_drop;
    };

} } } } // opendlv::core::system::proxy

#endif /*PROXY_APPLANIX_H*/
