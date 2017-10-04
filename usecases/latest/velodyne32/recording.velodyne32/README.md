This folder provides the instructions for recording HDL-32E lidar data. A docker-compose file is provided to start all micro-services to decode HDL-32E packets and record HDL-32E data. It includes three services: odsupercomponent, opendlv-core-system-proxy-velodyne32 (or proxy-velodyne32 for short), and odrecorderh264. odsupercomponent is used for software component lifecycle management in OpenDaVINCI. proxy-velodyne32 listens to HDL-32E packets and decodes them in real time. odrecorderh264 is used for recording. This tutorial assumes that git, Docker, and Docker Compose are installed. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/. In addition, the Docker image of opendlv.core is necessary for running this use case.

HDL-32E sends out data as UDP packets via an Ethernet cable. The current IP address of HDL-32E is assigned via DHCP. Run `nmap 10.42.42.0/24` to check the IP addresses of all devices in the network, including HDL-32E. Then ping the IP address of HDL-32E to check if it is successfully connected to the network. The network configuration of HDL-32E can be changed by following the data sheet provided together with HDL-32E.

The HDL-32E decoder supports two types of point clouds: shared point cloud (SPC) and compact point cloud (CPC). SPC contains either (1) the xyz coordinate of each point and its intensity value, or (2) the distance, azimuth, vertical angle, and intensity vaue of each point. CPC is a compressed version of SPC, squeezing one HDL-32E frame into three UDP packets. A detailed dedcription of CPC can be found in opendlv.core/code/core/system/proxy-velodyne32. In the configuration file in this use case folder, proxy-velodyne32.pointCloudOption specifies which point cloud is enabled: 0: SPC only, 1: CPC only, 2: both SPC and CPC. This use case folder also contains:

- a Dockerfile specifying the Docker images to be used
- an environment file .env which defines an environment variable CID that is referred to by the docker-compose file
- a HDL-32E calibration file HDL-32E.xml required by the HDL-32E decoder while SPC is enabled

Here CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111. In this folder, run Docker Compose:
    
    $ docker-compose up --build

This will activate odsupercomponent, proxy-velodyne32 and odrecorderh264. To stop the recording, run

    $ docker-compose stop
    
Remove the stopped containers:

    $ docker-compose rm
    
After the recording, the recording files are stored at ~/recordings, including

- a .rec file which stores all OpenDaVINCI containers, including CPC if CPC is enabled
- a .rec.mem file which stores SPC if SPC is enabled. This file is empty if SPC is not enabled

The recording file format is CID-xxx-odrecorderh264_yyy, where xxx is the cid number and yyy is the timestamp.
    
Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes all micro-services run with cid 123 instead of 111:

    $ CID=123 docker-compose up --build

Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

