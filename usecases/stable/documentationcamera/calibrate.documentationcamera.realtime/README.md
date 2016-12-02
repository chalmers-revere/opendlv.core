This folder provides the instructions for viewing runtime video feed with a single OpenCV camera. This use case is particularly used for adjusting the mounting position and angle of a documentation camera in the car. A docker-compose file is provided to start all micro-services to display runtime video feed. It includes three services: odsupercomponent, health, and opendlv-core-system-proxy-camera (or proxy-camera for short). odsupercomponent is used for software component lifecycle management in OpenDaVINCI. health checks the status of device nodes. proxy-camera activates the camera and enables a pop-up video display window. It is assumed that git, Docker, and Docker Compose are installed and the camera is properly connected. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/.
    
### Prepare proxy-camera

proxy-camera is included in the opendlv.core repository (https://github.com/chalmers-revere/opendlv.core). Clone the opendlv.core source:

    $ git clone https://github.com/chalmers-revere/opendlv.core
    
    $ git pull
    
Go to opendlv.core/docker, build and create the Docker image seresearch/opendlv-core-on-opendavinci-ubuntu-16.04-complete:latest:

    $ make buildComplete
    
    $ make createDockerImage
    
### Use proxy-camera with Docker Compose

Go to the folder usecases/calibrate.documentationcamera. This folder contains a checkHealth.sh script, a configuration file, a docker-compose file docker-compose.yml, and an environment file .env. The environment file .env defines an environment variable CID which is referred to by the docker-compose file. CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 201, thus in docker-compose.yml "${CID}" resolves to 201.  Run Docker Compose:
    
    $ docker-compose up --build

Then proxy-camera will start a pop-up window that displays live video feed from the camera. To stop the camera display (e.g., when the camera mounting position calibration is completed), run

    $ docker-compose stop
    
Then remove all stopped containers:

    $ docker-compose rm

Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent, proxy-camera, and odrecorderh264 run with cid 123 instead of 201:

    $ CID=123 docker-compose up
    
Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

Finally, note that this use case assumes that the camera is mounted upside down. Hence video images are flipped for that reason. The configuration file in this folder includes a parameter proxy-camera.camera.flipped which is set to 1. In order to disable flipped images, change its value to 0.

