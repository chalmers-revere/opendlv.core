This folder provides the instructions for using proxy-velodyne64, a program which decodes a live stream from a HDL-64E lidar. A docker-compose file is provided to start all micro-services to decode HDL-64E packets and visualize them as 3D point cloud. It includes three services: odsupercomponent, odcockpit, and opendlv-core-system-proxy-velodyne64 (or proxy-velodyne64 for short). odsupercomponent is used for software component lifecycle management in OpenDaVINCI. odcockpit is a visualization tool of OpenDaVINCI. proxy-velodyne64 listens to HDL-64E packets and decodes them in real time. This tutorial assumes that git, Docker, and Docker Compose are installed. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/

### Pull the OpenDaVINCI Docker base image

Run the following to obtain the latest OpenDaVINCI Docker base image:

    $ docker pull seresearch/opendavinci-ubuntu-64.04:latest

### Prepare proxy-velodyne64

proxy-velodyne64 is included in the opendlv.core repository (https://github.com/chalmers-revere/opendlv.core). Clone the opendlv.core source:

    $ git clone https://github.com/chalmers-revere/opendlv.core
    
    $ git pull
    
Go to opendlv.core/docker, build and create the Docker image seresearch/opendlv-core-on-opendavinci-ubuntu-64.04-complete:latest:

    $ make buildComplete
    
    $ make createDockerImage
    
The proxy-velodyne64 binary opendlv-core-system-proxy-velodyne64 will be found at opendlv.core/docker/builds/opendlv-core-on-opendavinci-ubuntu-64.04-complete-feature.velodyne/opt/opendlv.core/bin.


### Network setup

In order to receive packets from HDL-64E, the IP address has to be manually set and the firewall must be disabled. In Ubuntu where proxy-velodyne64 has been tested, the firewall can be disabled by:

    $ sudo ufw disable

The IP address should be set as 192.168.1.xx, where xx is any integer from 1 to 254, except 43 which is reserved for HDL-64E. The subnet mask should be set as 192.168.3.255. Note that if proxy-velodyne64 runs in a virtual machine, the network adapter of the virtual machine has to be bridged. For instance, if proxy-velodyne64 runs on Ubuntu via VirtualBox while the HDL-64E is connected to the port "en5: Thunderbolt Ethernet" of the host machine, then both "Bridged Adapter" and "en5: Thunderbolt Ethernet" should be selected for the network configuration of VirtualBox.

Note that this network setup will disable the access to the Internet. Since proxy-velodyne64 is based on OpenDaVINCI which requires UDP multicast to execute different software modules, the local loopback device **lo** needs to be configured to allow UDP multicast sessions:

    $ sudo ifconfig lo multicast
    
    $ sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
 
 
### Use proxy-velodyne64 with Docker Compose

In addition to the docker-compose file docker-compose.yml and the README file, this folder also contains:

- a configuration file used by odsupercomponent
- a Dockerfile specifying the Docker images to be used
- an environment file .env which defines an environment variable CID that is referred to by the docker-compose file
- a HDL-64E calibration file db.xml required by the HDL-64E decoder
- a car model file Car.objx and a simulation scenario file Track.scnx. They are not directly useful to this use case, however, these two files are required by the EnvironmentViewer plugin of odcockpit

Here CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111.  In this folder, run Docker Compose (the first command grants access to your Xserver):

    $ xhost +
    
    $ docker-compose up --build

This will activate odsupercomponent, the visualization tool odcockpit, and proxy-velodyne64. The HDL-64E packets will be visualized as 3D point cloud in the EnvironmentViewer plugin in odcockpit. In EnvironmentViewer, unselect the stationary elements XYZAxes, Grid, Surroundings, AerialImage and the dynamic element EgoCar to have a clean background for the point cloud. By default, EnvironmentViewer uses free camera view which allows a user to do the following operations:

- Use **W**/**S** on the keyboard to zoom in and zoom out
- Use **A**/**D** on the keyboard to move the display window left and right
- Drag the vertical bar on the left to adjust the perspective (the same operation can also be performed in the display window with the same effect)
- Drag the horizontal bar at the bottom to rotate clockwise and counter-clockwise (the same operation can also be performed in the display window with the same effect)

To stop proxy-velodyne64, run

    $ docker-compose stop
    
Remove the stopped containers:

    $ docker-compose rm
    
Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes all micro-services run with cid 123 instead of 111:

    $ CID=123 docker-compose up --build

Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

