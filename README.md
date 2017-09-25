# opendlv-core

## Building on a native Linux:

    mkdir build && cd build
    PATH=/opt/od4/bin:$PATH cmake -D OPENDAVINCI_DIR=/opt/od4 -D CMAKE_INSTALL_PREFIX=/opt/opendlv.core ..

## Building using a Docker builder:

    cd docker
    make buildComplete
    make createDockerImage

The resulting Docker image chalmersrevere/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest contains
the OpenDLV core binaries on the latest OpenDaVINCI framework running on Ubuntu 16.04 LTS.

Note that cmake is required to build opendlv.core for both native Linux and Docker building processes.
## Run the resulting Docker image:

    docker run -ti --rm --net host --user odv chalmersrevere/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest /bin/bash

