# opendlv-core

This repository contains the minimum requirements for compiling and running
any OpenDLV microservice, in particular the middleware OpenDaVINCI and the
OpenDLV Standard Message set.

## Building using a Docker builder:

    cd docker
    make

The resulting Docker image chalmersrevere/opendlv-core:latest and 
chalmersrevere/opendlv-core-dev:latest contains the OpenDLV core runtime 
binaries and development binaries respectively.

