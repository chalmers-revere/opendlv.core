This folder provides the instructions for testing proxy-camera, which can be executed from a docker-compose file to start all micro-services to record video streams from three OpenCV cameras with lossless H264 compression. It is assumed that git, Docker, and Docker Compose are installed and all the three cameras are properly connected. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/

### Prepare OpenDaVINCI Docker image

Clone the OpenDaVINCI source tree and get the latest update:

    $ git clone https://github.com/se-research/OpenDaVINCI
    
    $ git pull
    
Go to the docker folder and build OpenDaVINCI Docker image:

    $ make
    
### Prepare proxy-camera

proxy-camera is included in the feature.camera branch of the opendlv.core repository (https://github.com/chalmers-revere/opendlv.core). Clone the opendlv.core source and switch to the feature.camera branch:

    $ git clone https://github.com/chalmers-revere/opendlv.core
    
    $ git checkout feature.camera
    
    $ git pull
    
Go to opendlv.core/docker, build and create the Docker image seresearch/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest:

    $ make buildComplete
    
    $ make createDockerImage
    
### Test proxy-camera with Docker Compose

Copy the configuration file and docker-compose file docker-compose.yml, and the environment file .env in the 3camerasProxyCamera folder and paste them at ~/recordings. The environment file .env defines an environment variable CID which is referred to by the docker-compose file. CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111.  Change the current directory to ~/recordings and run Docker Compose (the first command grants access to your Xserver):

    $ xhost +
    
    $ docker-compose up

Then proxy-camera will start the recording with all the three cameras. To stop the recording, run

    $ docker-compose stop
    
Then remove all stopped containers:

    $ docker-compose rm

After the recording, the recording files are stored at ~/recordings. The three recording files from the three cameras are recorder.rec-WebCam1.h264, recorder.rec-WebCam2.h264, and recorder.rec-WebCam3.h264.

Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent, proxy-camera, and odrecorderh264 run with cid 123 instead of 111:

    $ CID=123 docker-compose up
    
Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

### Replay the recordings

In Terminal 1, run odsupercomponent as a Docker container:

    $ docker run -ti --rm --net=host -v ~/recordings:/opt/example seresearch/opendavinci-ubuntu-16.04-complete:latest /opt/od4/bin/odsupercomponent --cid=111 --verbose=1  --configuration=/opt/example/configuration
    
In Terminal 2, run odcockpit as a Docker container:

    $ docker run -ti --rm --net=host --ipc=host --user=root -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v ~/recordings:/opt/example seresearch/opendavinci-ubuntu-16.04-complete:latest /opt/od4/bin/odcockpit --cid=111
    
Open the SharedImageViewer plugin in odcockpit. Multiple windows can be started to replay multiple recordings separately in parallel.

In Terminal 3, run odplayerh264:

    $ docker run -ti --rm --net=host --ipc=host --user=root -v ~/recordings:/opt/example -w /opt/example seresearch/opendavinci-ubuntu-16.04-complete:latest /opt/od4/bin/odplayerh264 --cid=111
    
An alternative to odplayerh264 is to replay the recordings via the Player plugin in odcockpit. The recordings can be loaded from /opt/example via Player.

