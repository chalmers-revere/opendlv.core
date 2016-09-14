This folder provides the instructions for video recording with a single OpenCV camera. A docker-compose file is provided to start all micro-services to record video streams from the OpenCV camera with lossless H264 compression. It includes three services: odsupercomponent, opendlv-core-system-proxy-camera (or proxy-camera for short), and odrecorderh264. odsupercomponent is used for software component lifecycle management in OpenDaVINCI. proxy-camera activates the camera and odrecorderh264 records the video. It is assumed that git, Docker, and Docker Compose are installed and the camera is properly connected. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/.
    
### Prepare proxy-camera

proxy-camera is included in the opendlv.core repository (https://github.com/chalmers-revere/opendlv.core). Clone the opendlv.core source:

    $ git clone https://github.com/chalmers-revere/opendlv.core
    
    $ git pull
    
Go to opendlv.core/docker, build and create the Docker image seresearch/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest:

    $ make buildComplete
    
    $ make createDockerImage
    
### Use proxy-camera with Docker Compose

Go to the folder usecases/recordings.cameras.opencv.3. This folder contains a configuration file, a docker-compose file docker-compose.yml, and an environment file .env. The environment file .env defines an environment variable CID which is referred to by the docker-compose file. CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111.  Run Docker Compose:
    
    $ docker-compose up

Then proxy-camera will start the recording with the camera. To stop the recording, run

    $ docker-compose stop
    
Then remove all stopped containers:

    $ docker-compose rm

After the recording, the recording files are stored at ~/recordings, with recorder.rec-WebCam1.h264 being the actual recording with lossless H264 compression. 

Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent, proxy-camera, and odrecorderh264 run with cid 123 instead of 111:

    $ CID=123 docker-compose up
    
Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

