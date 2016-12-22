This folder provides the instructions for replaying recordings from different sensors. Recorded videos are replayed in odcockpit, a visualization tool of OpenDaVINCI. It is assumed that Docker and and Docker Compose are installed. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/. In addition, the Docker image of opendlv.core is necessary for running this use case.

This folder contains:

- a docker-compose file that includes two micro-services, odsupercomponent and odcockpit. odsupercomponent is used for software component lifecycle management in OpenDaVINCI
- a Dockerfile specifying the Docker image to be used
- a configuration file for odsupercomponent
- an environment file .env which defines an environment variable CID that is referred to by the docker-compose file
- a car model file Car.objx and a simulation scenario file Track.scnx. They are not directly useful to this use case, however, these two files are required by the EnvironmentViewer plugin of odcockpit to visualize the point cloud

Here CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111.  In this folder, run Docker Compose (the first command grants access to your Xserver):

    $ xhost +
    
    $ docker-compose up --build

This will activate odsupercomponent and the visualization tool odcockpit. In the Player plugin, load the recording file located at ~/recordings and click the Play button to start the replay. If the recording contains a video in H264 format, the video can be replayed in the SharedImageViewer plugin. Multiple such plugins can be open to replay multiple videos from the recording in parallel. If the recording contains point cloud, the point cloud can be replayed in the EnvironmentViewer plugin. In EnvironmentViewer, unselect the stationary elements XYZAxes, Grid, Surroundings, AerialImage and the dynamic element EgoCar to have a clean background for the point cloud. By default, EnvironmentViewer uses free camera view which allows a user to do the following operations:

- Use **W**/**S** on the keyboard to zoom in and zoom out
- Use **A**/**D** on the keyboard to move the display window left and right
- Drag the vertical bar on the left to adjust the perspective (the same operation can also be performed in the display window with the same effect)
- Drag the horizontal bar at the bottom to rotate clockwise and counter-clockwise (the same operation can also be performed in the display window with the same effect)

The recorded data from other sources such as Applanix and WGS84 coordinate can be displayed in the LiveFeed plugin. To stop the video replay, run

    $ docker-compose stop
    
Then remove all stopped containers:

    $ docker-compose rm

Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent and odcockpit run with cid 123 instead of 111:

    $ CID=123 docker-compose up --build
    
Then CID=123 should also be used for docker-compose stop and docker-compose rm accordingly.

