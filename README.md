<p align="center"><img src ="http://makers.sigfox.com/img/sigfox-logo-black.svg" width="300"></p>

# Sigfox HidnSeek Tutorial
This is a Sigfox tutorial for getting started with the [HidnSeek GPS Locator](https://www.hidnseek.fr/). This is a device based upon the ATMega328p with features including:

<p align="center"><img src ="https://raw.githubusercontent.com/Bucknalla/Sigfox-Hidnseek-Tutorial/master/Resources/images/hidnseek.png" width="300"></p>


* GPS Locality
* 3 Axis Accelerometer
* Temperature and Pressure Sensors
* [SIGFOX](https://sigfox.com) Enabled Module (TD 1207)
* Arduino compatible

Using the Sigfox network, the device can broadcast it's payload to the Sigfox Cloud, where it can be interpreted by a user's server allowing for both Uplink and Downlink messages to be sent. As the network is currently undergoing global rollout, please check our [coverage map](http://www.sigfox.com/coverage) for details on availability in your region!

## Getting started
This tutorial assumes that you are familiar with the Arduino IDE (https://www.arduino.cc/en/Main/Software) and are running at least version **1.6.4**.

### Setting Up



#### Installing Drivers

#### Installing Boards

## Getting a GPS Fix

This example demonstrates how you can use the HidnSeek's GPS to broadcast the device's location at intervals of once an hour. This can be increased up to approximately 6 times an hour (140~ messages per day) based upon the limitations of the EU Regulations (ETSI 300-220) on the unlicensed 868MHz frequency.
#### Blinking whilst Acquiring

## Detecting Movement

#### Using the Accelerometer

## Learn More

If you're interested in finding out more about Sigfox, LPWAN and the dev kits that are available, you can head to our [website](http://makers.sigfox.com) to learn more!
