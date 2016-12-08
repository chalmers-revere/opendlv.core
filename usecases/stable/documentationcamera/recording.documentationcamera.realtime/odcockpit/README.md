This folder provides the instructions for monitoring a running recording session with a documentation camera. To start the recording session, follow the instructions in the folder /opendlv.core/usecases/recording.documentationcamera. By default the recording runs in "headless mode", i.e., without display. Using the micro-service odcockpit encapsulated in the docker-compose file in this folder, the video being recorded can be graphically monitored in real time. While the recording is in progress, start odcockpit in a Docker container (the first command grants access to your Xserver):

    $ xhost +
    
    $ docker-compose up --build

Open the SharedImageViewer plugin in odcockpit and double click the recording session name therein. The video will be visible.

To stop odcockpit and remove the stopped Docker container, run

    $ docker-compose stop

    $ docker-compose rm
