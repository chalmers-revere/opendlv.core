#!/bin/bash

# completeBuild.sh - Script to build opendlv.core.
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

UID_AS=$1

cat <<EOF > /opt/run-cmake.sh
#!/bin/bash
cd /opt/build

echo "[opendlv.core Docker builder] Complete build."
cmake -E remove_directory .
PATH=/opt/od4/bin:$PATH cmake -D OPENDAVINCI_DIR=/opt/od4 -D PACKAGING_ENABLED=NO -D CMAKE_INSTALL_PREFIX=/opt/output /opt/sources
PATH=/opt/od4/bin:$PATH make -j4
EOF

chmod 755 /opt/run-cmake.sh
chown $UID_AS:$UID_AS /opt/run-cmake.sh

su -m `getent passwd $UID_AS|cut -f1 -d":"` -c /opt/run-cmake.sh

