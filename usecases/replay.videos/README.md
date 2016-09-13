This folder provides the instructions for replaying one or multiple recorded videos with H264 lossless compression. Recorded videos are replayed in odcockpit, a visualization tool of OpenDaVINCI together with odplayerh264 that decodes videos in H264 format. It is assumed that Docker and and Docker Compose are installed. To install Docker, follow the tutorial: https://docs.docker.com/engine/installation/linux/ubuntulinux/

### Replay video recordings

Copy the configuration file and docker-compose file docker-compose.yml, and the environment file .env in this folder and paste them at ~/recordings. The environment file .env defines an environment variable CID which is referred to by the docker-compose file. CID is a user-defined environment variable that specifies the cid of the UDP session established by odsupercomponent. In .env CID has the value 111, thus in docker-compose.yml "${CID}" resolves to 111.  Change the current directory to ~/recordings and run Docker Compose (the first command grants access to your Xserver):

    $ xhost +
    
    $ docker-compose up

This will activate odsupercomponent and the visualization tool odcockpit. Open the SharedImageViewer plugin in odcockpit. Multiple windows can be started to replay multiple recordings separately in parallel.

In a new terminal, run odplayerh264:

    $ docker run -ti --rm --net=host --ipc=host --user=root -v ~/recordings:/opt/example -w /opt/example seresearch/opendavinci-ubuntu-16.04-complete:latest /opt/od4/bin/odplayerh264 --cid=111
  
If there are video recordings at ~/recordings, they will be replayed in the SharedImageViewer window(s). An alternative to odplayerh264 is to replay the recordings via the Player plugin in odcockpit. The recordings can be loaded from /opt/example via Player.

To stop the video replay, run

    $ docker-compose stop
    
Then remove all stopped containers:

    $ docker-compose rm

Note that the value of CID defined in .env can be manually overwritten by preceding the docker-compose command with CID=xxx, where xxx is the cid number. For instance, the following command makes odsupercomponent and odcockpit run with cid 123 instead of 111:

    $ CID=123 docker-compose up
    
Consequently, odplayerh264 has to run with cid=123 as well.

