<p align="center"><img src ="http://makers.sigfox.com/img/sigfox-logo-black.svg" width="300"></p>

# SIGFOX HidnSeek Tutorial
This is a SIGFOX tutorial for getting started with the [HidnSeek GPS Locator](https://www.hidnseek.fr/). This is a device based upon the ATMega328p with features including:

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/hidnseek.png" width="600"></p>

* GPS Locality
* 3 Axis Accelerometer
* Temperature and Pressure Sensors
* [SIGFOX](https://sigfox.com) Enabled Module (TD 1207)
* Arduino IDE Compatible

Using the SIGFOX network, the device can broadcast it's payload to the SIGFOX Cloud, where it can be interpreted by a user's server allowing for both Uplink and Downlink messages to be sent. As the network is currently undergoing global rollout, please check our [coverage map](http://www.sigfox.com/coverage) for details on availability in your region!

## Getting started
This tutorial assumes that you are familiar with the [Arduino IDE](https://www.arduino.cc/en/Main/Software) and are using, at least version **1.6.4**.

### Setting Up

Open up the Arduino IDE and make sure that there are no devices currently connected to your machine.

#### Locating the Board

First of all, you will need to add the board to the Arduino IDE using the **Additional Boards Manager**. You can find this by clicking on the Arduino IDE, *Preferences* button. You'll see a text field titled, *Additional Boards Manager URLs*. Within this field you should add the following link:

https://hidnseek.github.io/hidnseek/package_hidnseek_boot_index.json

This allows the Arduino IDE to automatically find the index files required to build on the HidnSeek Device.

#### Installing the Board

You will now need to install the board. You can do this by navigating to the **Board Manager**, located under *Tools* -> *Board* -> *Board Manager*. Locate and install USBaspLoader HidnSeek.

Restart the Arduino IDE and check that the board has correctly installed.

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/arduino_install.png" width="700"></p>

#### Installing Drivers (Windows Only)

There is an up-to-date link to the Windows drivers provided by HidnSeek.

http://www.protostack.com/download/USBasp-win-driver-x86-x64-v3.0.7.zip

The driver will work with Windows XP through to Windows 10 (both 32 and 64 bit editions).

#### Preparing the Board for Uploading

In order to prepare the HidnSeek for the uploading of a sketch, **the board must be placed into DFU mode (Device Firmware Update)**.

If you open the case of the HidnSeek (There are 4 plastic connectors around the edge of the device), you will notice a bank of exposed connections. Short the connections labelled R & G and the device will enter DFU mode.

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/HidnSeek_Board.png" width="450"></p>

Once the device has entered DFU mode, a red LED will flash rapidly for approximately 30 seconds. During this period of time, a new sketch may be uploaded to the device. If you miss this window, you will have to repeat the process for entering DFU mode.

## Detecting Movement and Transmitting Alert over SIGFOX

This example shows how to use the HidnSeek library to detect movement on the device, counting the number of times the device has moved since the last transmission and send the data over the SIGFOX network.

*The device is limited to transmit a maximum of approximately 6 times an hour (140~ messages per day) based upon the limitations of the EU Regulations (ETSI 300-220) on the unlicensed 868MHz frequency.*

This could be used as an alarm or a trigger based upon movement such as a door or window opening.

*Please note - the use of isReady() is unideal as it is inefficient.*

## Getting a GPS Fix and Transmitting Location/Timestamp over SIGFOX

This example demonstrates how you can use the HidnSeek's GPS to broadcast the device's location at intervals of once an hour over the SIGFOX network. This sketch is a modification of the Firmware for the HidnSeek as much of the behaviour requires the related sketch functions for battery, GPS, etc. control.

The sports mode and other additional functionality of the HidnSeek have been removed to streamline the sketch and make it easier to understand functionality.

### Checking Your Data over Serial

In order to determine if the GPS data you are collecting is correct, you can use the device's serial port to communicate with a host machine. You will need to use a Serial to USB cable and connect it to the HidnSeek via the GND, RX, TX and 3.3 V connector pads.

### Retrieving Data from the SIGFOX Cloud

This step is slightly more complicated as we're required to set up an server to communicate with the SIGFOX Cloud and to retrieve any of the messages sent by the HidnSeek.

In the interest of existing tutorials/guides for managing SIGFOX callbacks, it would be useful to check out our [Talking Plant Tutorial](https://www.hackster.io/18194/sigfox-talking-plant-0d21bc?ref=platform&ref_id=7860_trending___&offset=0) where it's shown to you how to use node.js to host a server with the purpose of receiving sensor data.

If you haven't already, you will need to activate the device at [backend.sigfox.com](https://backend.sigfox.com). This is our device/network management portal where you can set device callbacks, configure downlink messages, etc.

### Learn More

If you're interested in finding out more about SIGFOX, our technology and the dev kits that are available, you can head to our [website](http://makers.sigfox.com) to learn more!

#### References

* Reference Manual - https://github.com/Bucknalla/Sigfox-Hidnseek-Tutorial/blob/master/Resources/HidnSeek_Manual.pdf
* HidnSeek Original Repository - https://github.com/hidnseek/hidnseek

###### Credit

* Stephane D - HidnSeek
