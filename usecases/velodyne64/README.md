This folder provides the instructions for testing proxy-velodyne, a program which decodes a live stream from a 64-layer Velodyne lidar (HDL-64E). This tutorial assumes that git, Docker, and Docker Compose are installed. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/

### Prepare the OpenDaVINCI test environment

The test of proxy-velodyne requires a tool that simulates live Velodyne data and a tool for visualization. Both tools are provided in veloviz branch of OpenDaVINCI(http://www.opendavinci.org), an open source development architecture for virtual, networked, and cyber-physical system infrastructures.

Clone the OpenDaVINCI source tree and get the latest update:

    $ git clone https://github.com/se-research/OpenDaVINCI
    
    $ git pull

Switch to the veloviz branch and get the latest update:

    $ git checkout veloviz
    
    $ git pull
    
Merge the master branch into veloviz:

    $ git merge master
    
Manually resolve the conflicts after the merge. Then build OpenDaVINCI by following the tutorial at: http://opendavinci.readthedocs.io/en/latest/installation.html. Here we assume that OpenDaVINCI is installed at /opt/od3.

After OpenDaVINCI is successfully built and installed, copy Car.objx and Track.scnx in this folder and paste them into /opt/od3/bin. These two files are used by the visualizatioan tool odcockpit at /opt/od3/bin.

Put the configuration file in this folder at ~/Downloads/proxyVelodyneTest. This configuration file will be used by odsupercomponent of OpenDaVINCI for software component lifecycle management.

The veloviz branch of OpenDaVINCI provides a tool PcapReplay which reads a .pcap Velodyne recording file and sends out the decoded Velodyne data to simulate live Velodyne stream. By default, PcapReplay is not automatically installed at /opt/od3/bin. Therefore, we need to manually build PcapReplay. Go to the OpenDaVINCI source folder and change the current directory to automotive/velodyne/PcapReplay. Open and edit PcapReplay.cpp in the src folder by replacing "../../velodyneReadFile/build/atwall.pcap" with "xxx.pcap", where xxx is the name of the .pcap file to be used. Build PcapReplay as follows:

    $ mkdir build && cd build
    
    $ cmake -D CMAKE_INSTALL_PREFIX=/opt/od3 ..
    
    $ make
    
Put a xxx.pcap recording file in the same folder as the PcapReplay binary.

### Prepare proxy-velodyne

proxy-velodyne is included in the feature.velodyne branch of the opendlv.core repository (https://github.com/chalmers-revere/opendlv.core). Clone the opendlv.core source and switch to the feature.velodyne branch:

    $ git clone https://github.com/chalmers-revere/opendlv.core
    
    $ git checkout feature.velodyne
    
    $ git pull
    
Go to opendlv.core/docker, build and create the Docker image seresearch/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest:

    $ make buildComplete
    
    $ make createDockerImage
    
The proxy-velodyne binary opendlv-core-system-proxy-velodyne will be found at opendlv.core/docker/builds/opendlv-core-on-opendavinci-ubuntu-16.04-complete-feature.velodyne/opt/opendlv.core/bin. The Velodyne decoder integrated in proxy-velodyne requires an external calibration file. Put the calibration file db.xml in this folder to the same folder as the proxy-velodyne binary.

### Test proxy-velodyne

In Terminal 1, go to /opt/od3/bin, and run odsupercomponent:

    $ LD_LIBRARY_PATH=/opt/od3/lib ./odsupercomponent --cid=111 --configuration=~/Downloads/proxyVelodyneTest/configuration
    
In Terminal 2, go to /opt/od3/bin, and run odcockpit:

    $ LD_LIBRARY_PATH=/opt/od3/lib ./odcockpit --cid=111
    
Open the EnvironmentViewer plugin in odcockpit. Find the configuration window SceneGraph. In the drop-down menu below, unselect the objects that you do not want to see during the visualization.

In Terminal 3, go to the directory of proxy-velodyne and run proxy-velodyne:

    $ LD_LIBRARY_PATH=/opt/od3/lib ./opendlv-core-system-proxy-velodyne --cid=111 --freq=30
    
In Terminal 4, go to the directory of PcapReplay (OpenDaVINCI/automotive/velodyne/PcapReplay/build) and run PcapReplay:

    $ ./PcapReplay --cid=111 --freq=30
    
Then the Velodyne data will be visualized as 3D point cloud frames in EnvironmentViewer of odcockpit. Adjust the frequency of proxy-velodyne and PcapReplay carefully to obtain the best frame rate. Note that too low frequency of proxy-velodyne may lead to buffer overflow.

### Test proxy-velodyne with Docker

Follow the same instruction for Terminal 1, 2, and 4 above. In Terminal 3, run proxy-velodyne in a Docker container instead:

    $ docker run -ti --rm --net=host --ipc=host --user=1000 seresearch/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest /opt/opendlv.core/bin/opendlv-core-system-proxy-velodyne --cid=111 --freq=30

### Test proxy-velodyne with Docker Compose

Put docker-compose.yml, Dockerfile and the environment file .env in this folder to ~/Downloads/proxyVelodyneTest. Switch to that folder and run:

    $ docker-compose up
    
This will start odsupercomponent and proxy-velodyne. The environment file .env defines an environment variable CID which is referred to by the docker-compose file. CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111. Then run odcockpit and PcapReplay by following the same instruction above. Note that sudo should be used while running odcockpit, as root user is specified for proxy-velodyne in the docker-compose file.

To stop the test, run

    $ docker-compose stop
    
Remove the stopped containers:

    $ docker-compose rm
    
Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent and proxy-velodyne run with cid 123 instead of 111:

    $ CID=123 docker-compose up

This means that odcockpit and PcapReplay should also run with cid=123. Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.
