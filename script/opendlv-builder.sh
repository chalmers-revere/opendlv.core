#!/bin/bash

# Copyright (C) 2017 Christian Berger
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

BASE_IMAGE=$1
IMAGE=$2


# IDEA: Output mode: (1) docker image, (2) binaries, (3) packages
# TODO: Fix ccache, volume for build files
# TODO: Abort if compilation (or somethig else) fails

REPOSITORY=seresearch
IMAGE_PATH=$REPOSITORY/$IMAGE
BASE_IMAGE_PATH=$REPOSITORY/$BASE_IMAGE

SOURCE_PATH=/opt/sources

VERSION=`cat $SOURCE_PATH/VERSION`

DOCKER_NETWORK_NAME=nw_$IMAGE

DOCKER_SOCK="/var/run/docker.sock"
if [ ! -e "$DOCKER_SOCK" ]
then
	echo "$DOCKER_SOCK not found. If running in container, please link this file from the host system, or make sure that docker is running."
  exit 1
fi

docker network inspect $DOCKER_NETWORK_NAME || docker network create $DOCKER_NETWORK_NAME

mkdir /opt/build /opt/output

cat <<EOF > /opt/run-cmake.sh
#!/bin/bash
cd /opt/build

echo "[opendlv-builder] Complete build."
cmake -E remove_directory .
PATH=/opt/od4/bin:$PATH cmake -D OPENDAVINCI_DIR=/opt/od4 -D PACKAGING_ENABLED=NO -D CMAKE_INSTALL_PREFIX=/opt/output $SOURCE_PATH
PATH=/opt/od4/bin:$PATH make -j4
EOF

chmod +x /opt/run-cmake.sh
/opt/run-cmake.sh

docker network rm $DOCKER_NETWORK_NAME

ADD_CMDS=""
if [ -e "/opt/output/bin" ]
then
  ADD_CMDS="$ADD_CMDS\nADD bin /usr/bin" 
fi
if [ -e "/opt/output/lib" ]
then
  ADD_CMDS="$ADD_CMDS\nADD lib /usr/lib" 
fi
if [ -e "/opt/output/share" ]
then
  ADD_CMDS="$ADD_CMDS\nADD share /usr/share" 
fi

cat <<EOF > /opt/output/Dockerfile
FROM $BASE_IMAGE_PATH
MAINTAINER Christian Berger "christian.berger@gu.se"
$ADD_CMDS
EOF
	
cd /opt/output 
docker build -t $IMAGE_PATH:latest .
docker tag $IMAGE_PATH:latest $IMAGE_PATH:$VERSION


