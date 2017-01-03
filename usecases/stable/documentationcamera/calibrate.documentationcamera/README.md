# Displaying the Live Image of a Camera

This folder provides the instructions for viewing the live image video of a V4L camera. This can be used for adjusting the mounting of a camera in the car and needs to be run on a system with a screen. The following micro-services are included: odsupercomponent, health, proxy-camera. odsupercomponent is used for software component lifecycle management in OpenDaVINCI. health checks the status of device nodes. proxy-camera activates the camera and displays a pop-up window with the live image.
    
## Setup Camera Device

Camera devices are listed in `/dev`. To check whether the camera is successfully attached, do

    $ ls /dev/video*
    
which should give you the list of attached cameras. If the attached camera is not `/dev/video0`, modify the left side of the device mapping of `proxy-camera` in `docker-compose.yml`. For example if your camera device is `/dev/video2`, change the mapping from 

    devices:
        - /dev/video0:/dev/video0
        
to

    devices:
        - /dev/video2:/dev/video0
        
The right side of the mapping shall not be changed.
    
## Start the usecase

To start the usecase, run
    
    $ docker-compose up --build

This should open a new window that displays the live video image from the camera. To stop, hit `Ctrl+C` in the terminal window. To leave the system in a clean state, stop all the containers of the usecase and remove them:

    $ docker-compose stop && docker-compose rm

## Troubleshooting

### Flipped Camera Image

It is assumed that the camera is mounted upside down, thus the video images are flipped before displaying. To disable flipping, set `proxy-camera.camera.flipped` in `configuration` to `0`.

### odsupercomponent

If the `odsupercomponent` service fails to start, try altering the `CID` in the file `.env`.

### Camera Image

If there are problems, try

    $ ffplay -i /dev/video0
    
where `/dev/video0` is the video device you want to test. This will also open a window with the live image of the camera and might help precluding potential error sources.
