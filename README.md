# _Infinite Firmware_

This is the repository for the Infinite Automation firmware. It includes a version of their Arduino based firmware which has been converted to use ESP-IDF with Arduino as a component to ensure a smooth transition.

## Installation
Use this [tutorial](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md) to setup VS code and configure ESP-IDF. Once configured you can clone this repository and then use the ESP-IDF tools to build, flash and monitor the device.

The process for installing Arduino as an ESP-IDF component can be found [here](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/esp-idf_component.html).

## Tests
This repository includes some tests written in python that can test the setup process, logging in a user and controlling the device. These tests include command line arguments for tasks such as:

- clean: 
  - Boolean: True/False Clear the flash and perform a fresh install. 
- restart: 
  - Boolean: True/False Reboot the device before running the tests. 
- token_key: 
  - Android: "android_token" if the app was used to configure device.
  - Custom: "custom_token" if test_setup.py was run.


These tests are located in the **test** directory and can be run either directly from pytest:

```
pytest -s --verbose test_setup.py clean=True restart=True

pytest test_android.py

pytest test_control.py token_key=android_token
```

Or using the make file which includes pre-configured commands for the most common test cases.

```
# Clean install, setup and test control as user created during setup.
make test-all

# Perform clean install and custom user setup.
make test-setup-clean

# Login as user created after android setup and test control.
make test-control-android
```

### Command Line Helpers
The **test** directory also includes a cli for often used commands.
```
python cli.py erase-flash

python cli.py flash-firmware

# Decode a command hex value into the command type, switch selection, lock and status
python cli.py decode-cmd --cmd=1E

# Generate the Hex and Binary representation for a command from text inputs.
python cli.py generate-command --cmd=switch --switch=a --status=on

# Send Hex command to device.
python cli.py send-command --cmd=1E

# Get token for future requests after performing setup on Android device
python cli.py login-android

# For help with any of the commands:
python cli.py [command] --help

```
## Folder contents

The project **infinite_firmware** contains multiple C++ source and header files each encapsulating specific functionality. The entry to the program is [main.cpp](main/main.cpp).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

```
infinite_firmware
├─ .gitignore
├─ CMakeLists.txt
├─ components
│  └─ arduino
├─ dependencies.lock
├─ main
│  ├─ BroadcastManager.cpp
│  ├─ BroadcastManager.h
│  ├─ Buzzer.cpp
│  ├─ Buzzer.h
│  ├─ CMakeLists.txt
│  ├─ component.mk
│  ├─ CurrentSensor.cpp
│  ├─ CurrentSensor.h
│  ├─ FilesManager.cpp
│  ├─ FilesManager.h
│  ├─ Globals.cpp
│  ├─ Globals.h
│  ├─ HLW8012.cpp
│  ├─ HLW8012.h
│  ├─ main.cpp
│  ├─ main.h
│  ├─ MqttHandler.cpp
│  ├─ MqttHandler.h
│  ├─ NTPClient.cpp
│  ├─ NTPClient.h
│  ├─ Ota.cpp
│  ├─ Ota.h
│  ├─ PubSubClient.cpp
│  ├─ PubSubClient.h
│  ├─ RealTimeClock.cpp
│  ├─ RealTimeClock.h
│  ├─ RelayController.cpp
│  ├─ RelayController.h
│  ├─ Scheduler.cpp
│  ├─ Scheduler.h
│  ├─ ServerComm.cpp
│  ├─ ServerComm.h
│  ├─ SwitchInput.cpp
│  ├─ SwitchInput.h
│  ├─ SystemConfiguration.cpp
│  ├─ SystemConfiguration.h
│  ├─ UserHandler.cpp
│  ├─ UserHandler.h
│  ├─ Watchdog.cpp
│  ├─ Watchdog.h
│  ├─ WifiManager.cpp
│  ├─ WifiManager.h
│  └─ WifiWebPages.h
├─ test
│ ├─ cli.py
│ ├─ config.py
│ ├─ conftest.py
│ ├─ kvstore.py
│ ├─ logger.py
│ ├─ Makefile
│ ├─ test_android.py
│ ├─ test_control.py
│ ├─ test_setup.py
│ ├─ requirements.txt
│ └─ utils.py
├─ Makefile
├─ partitions.csv
├─ make_img.ps1
├─ README.md
├─ sdkconfig
└─ sdkconfig.old
```