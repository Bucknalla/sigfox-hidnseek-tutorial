<p align="center"><img src ="http://makers.sigfox.com/img/sigfox-logo-black.svg" width="300"></p>

# SIGFOX HidnSeek Tutorial
This is a SIGFOX tutorial for getting started with the [HidnSeek GPS Locator](https://www.hidnseek.fr/). This is a device based upon the ATMega328p with features including:

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/hidnseek.png" width="300"></p>

* GPS Locality
* 3 Axis Accelerometer
* Temperature and Pressure Sensors
* [SIGFOX](https://sigfox.com) Enabled Module (TD 1207)
* Arduino IDE Compatible

Using the SIGFOX network, the device can broadcast it's payload to the SIGFOX Cloud, where it can be interpreted by a user's server allowing for both Uplink and Downlink messages to be sent. As the network is currently undergoing global rollout, please check our [coverage map](http://www.sigfox.com/coverage) for details on availability in your region!

## Getting started
This tutorial assumes that you are familiar with the Arduino IDE (https://www.arduino.cc/en/Main/Software) and are running at least version **1.6.4**.

### Setting Up



#### Installing Drivers

First of all, you will need to add the board to the Arduino IDE using the **Additional Boards Manager**. You can find this by clicking on the Arduino IDE, *Preferences* button. You'll see a text field titled, *Additional Boards Manager URLs*. Within this field you should add the following link:

https://hidnseek.github.io/hidnseek/package_hidnseek_boot_index.json

This allows the Arduino IDE to automatically find the index files required to build for the HidnSeek Device.

#### Installing Boards

You will now need to install the board. You can do this by navigating to the **Board Manager**, located under *Tools*, then *Board*. Locate and install USBaspLoader HidnSeek.

Restart the Arduino IDE and check that the board has correctly installed.

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/hidnseek_board.png" width="300"></p>

#### Preparing the Board for Uploading

In order to prepare the HidnSeek for the uploading of a sketch, **the board must be placed into DFU mode (Device Firmware Update)**. This can be activated by one of two different ways.

##### 1. Accelerometer

Using the 3-Axis Accelerometer within the device, it is possible to place the device into DFU mode.

##### 2. Shorting R & G Pins

If you open the case of the HidnSeek (There are 4 plastic connectors around the edge of the device), you will notice a bank of exposed connections. Short the connections labelled R & G and the device will enter DFU mode.

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/hidnseek_board.png" width="300"></p>

Once the device has entered DFU mode, a red LED will flash rapidly for approximately 30 seconds. During this period of time, a new sketch may be uploaded to the device. If you miss this window, you will have to repeat the process for entering DFU mode.

## Getting a GPS Fix

This example demonstrates how you can use the HidnSeek's GPS to broadcast the device's location at intervals of once an hour over the SIGFOX network. This can be increased up to approximately 6 times an hour (140~ messages per day) based upon the limitations of the EU Regulations (ETSI 300-220) on the unlicensed 868MHz frequency.

We'll use the red LED on board to indicate when the device is transmitting it's location.

#### Outputting to the Serial Terminal

For convenience, we'll output the GPS location to the Arduino's built-in Serial Monitor for the time being. You can launch the serial monitor by selecting *Tools*, then *Serial Monitor* from the Arduino IDE menu.

#### Retrieving Data from the SIGFOX Cloud

This step is slightly more complicated as we're required to set up an server to communicate with the SIGFOX Cloud and to retrieve any of the messages sent by the HidnSeek.

## Detecting Movement

#### Using the Accelerometer

This next example uses the

## Learn More

If you're interested in finding out more about SIGFOX, our technology and the dev kits that are available, you can head to our [website](http://makers.sigfox.com) to learn more!
