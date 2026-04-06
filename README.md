# puppy-drone
Autonomous drone Application for Zephyr and Puppy SoC  

## Getting and compiling

In order to compile the program, you will need to follow Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) until the ```pip install west``` command. Then, instead of ```west init ~/zephyrproject```, you type ```west init -m https://github.com/caninos-loucos/puppy-drone``` and follow until ```west sdk install```.  

After West is set up, you will be able to compile using the command ```west build -p -b poodle-toy-v1 --sysbuild```  

## Flashing

To flash the program on your board, you can plug in your Poodle Toy board with an USB cable, press the reset button and send the command ```mcumgr image upload --conntype=serial --connstring='dev=/dev/ttyUSB0,baud=115200'``` in the next 2 seconds, while the red LED is on.  
`mcumgr` can only be invoked in the Zephyr Python Virtual environment, so if you can't find the program, you must first install Zephyr through the getting started guide. If you don't want to do that, you can download [AuTerm](https://github.com/thedjnK/AuTerm/releases/tag/v0.36a-pre), which has a graphical interface, then select the serial port where your board is (generally /dev/ttyUSB0 when on Linux) and select speed of 115200. Then you go to the MCUmgr tab, click on the "Connect" button and, if successful, select the file you want to flash your device with. Then you do the process of resetting the board.  
