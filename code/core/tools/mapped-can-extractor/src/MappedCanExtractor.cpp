/**
 * odrecintegrity - Tool for checking the integrity of recorded data
 * Copyright (C) 2014 - 2015 Christian Berger
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

#include <fstream>
#include <iostream>
#include <string>

#include <opendavinci/odcore/serialization/Serializable.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/reflection/Message.h>
#include <opendavinci/odcore/reflection/MessagePrettyPrinterVisitor.h>
#include <odvdfh16truck/GeneratedHeaders_ODVDFH16Truck.h>
#include <odvdfh16truck/GeneratedHeaders_ODVDFH16Truck_Helper.h>

#include "MappedCanExtractor.h"

namespace mappedcanextractor {

    using namespace std;
    using namespace odcore;
    using namespace odcore::base;
    using namespace odcore::data;
    using namespace odcore::reflection;

    MappedCanExtractor::MappedCanExtractor() {}

    MappedCanExtractor::~MappedCanExtractor() {}

    int32_t MappedCanExtractor::run(const int32_t &argc, char **argv) {
        enum RETURN_CODE { SUCCESS = 0,
                           FILE_COULD_NOT_BE_OPENED = 255 };

        RETURN_CODE retVal = SUCCESS;

        if (argc == 2) {
            const string FILENAME(argv[1]);
            fstream fin;
            fin.open(FILENAME.c_str(), ios_base::in|ios_base::binary);

            if (fin.good()) {
                // Determine file size.
                fin.seekg(0, fin.end);
                int32_t length = fin.tellg();
                fin.seekg(0, fin.beg);

                int32_t oldPercentage = -1;
                while (fin.good()) {
                    Container c;
                    fin >> c;

                    if (fin.gcount() > 0) {
                        int32_t currPos = fin.tellg();

                        // If the data is from SHARED_IMAGE, skip the raw data from the shared memory segment.
                        cout << "[MappedCanExtractor]: Found data type '" << c.getDataType() << "'." << endl;

                        bool successfullyMapped = false;
                        Message msg = GeneratedHeaders_ODVDFH16Truck_Helper::__map(c, successfullyMapped);
                        if (successfullyMapped) {
                            MessagePrettyPrinterVisitor mppv;
                            msg.accept(mppv);
                            mppv.getOutput(cout);
                        }

                        float percentage = (float)(currPos*100.0)/(float)length;

                        if ( ((int32_t)percentage % 5 == 0) && ((int32_t)percentage != oldPercentage) ) {
                            cout << "[MappedCanExtractor]: " << percentage << "% (" << currPos << "/" << length << " bytes processed)." << endl;
                            oldPercentage = (int32_t)percentage;
                        }
                    }
                }
            }
            else {
                retVal = FILE_COULD_NOT_BE_OPENED;
            }
        }

        return retVal;
    }

} // mappedcanextractor

