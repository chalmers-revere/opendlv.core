# Controlling the FH16 Truck Using the PS3 Controller

This folder provides instructions on how to use the PS3 controller to control the FH16 truck. The demo is supposed to run on the truck PC. Before continuing with the instructions below, SSH to this computer.

## Check & Set Up Periphery

Before starting the usecase, make sure all periphery is working and sending data.

### PS3 Controller

#### Test

After connecting the controller, a device node will be created in `/dev/input`. To see which controller device nodes are available, run

    $ ls /dev/input/js*
    
After plugging in, the four LEDs left to the USB connector of the controller should be flashing. Press the round button in the center of the controller, labelled with the PlayStation logo, to turn on the controller. If the LEDs stop flashing at some point, the controller stopped working. Replug it and start again.

To check whether the controller is actually sending data, run

    $ cat /dev/input/js0
    
assuming that `js0` is the name of the device node corresponding to the controller. If this is the right node and the controller is working, there should be console output when you move the joysticks and press buttons. If you have more than one `js*` device node, check until you find the one which produces output.

#### Set Up

Check whether the device name matches the mapping in `docker-compose.yml`.  If the device name determined in the previous step does not match the **left** side of the mapping, change it accordingly.

Example: Suppose your PS3 controller is attached to `/dev/input/js1`, while in docker-compose the mapping says:

    devices:
        - /dev/input/js0:/dev/input/js0
        
In that case, change the left side of the mapping to `/dev/input/js0`:

    devices:
        - /dev/input/js1:/dev/input/js0
        
The right side of the mapping shall not be changed and should correspond to the `system-ps3controller.ps3controllerdevicenode` value in `configuration`.

### CAN Interface

#### Test

Connect the truck's CAN cable to the interface on the car PC. Make sure the truck sends CAN data, e.g. by turning on the engine. Check whether you get CAN data by running where ? needs to be replaced with the correct CAN network interface:

    $ cansniffer can?
    
There should be lots of output. If there is not, try with a different network interface number (`ifconfig -a` for the possible numbers).

#### Set Up

Check whether the device identifier corresponds to the `proxy-fh16.devicenode` value in `configuration`.

## Start the Usecase

To build the usecases' image and run the usecase, run:

    $ docker-compose up --build
    
To exit the containers of the usecase, hit `Ctrl+C`. Afterwards, to stop them, run:

    $ docker-compose stop
    
To actually remove the containers of the usecase, run

    $ docker-compose rm
