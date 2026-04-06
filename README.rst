# Zephyr Drone Application for the Puppy microprocessor

This is an application for testing the upcoming Puppy Drone hardware with the Caninos Loucos Puppy microprocessor.

### Getting and compiling

In order to compile the program, you will need to follow Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) until the ```pip install west``` command. Then, instead of ```west init ~/zephyrproject```, you type ```west init -m https://github.com/caninos-loucos/puppy-drone``` and follow until ```west sdk install```.

After West is set up, you will be able to compile using the command ```west build -p -b poodle-toy-v1 --sysbuild```

### Flashing

To flash the program on your board, you can use ```mcumgr image upload --conntype=serial --connstring='dev=/dev/ttyUSB0,baud=115200'``` while in Zephyr's Python Virtual Environment.
