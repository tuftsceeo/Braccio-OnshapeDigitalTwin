# Braccio-OnshapeDigitalTwin
Implementation of an Onshape digital twin for the Arduino Braccio++ robot arm. We can use the Onshape interface to place checkpoints that define the path of the end effector as well as the gripper configuration allowing pick and place operations.
Accomanying Onshape document: [Click here to follow](https://cad.onshape.com/documents/9a453d9e3d5cb8f5b9b51eba/w/0248d727b4bba4c0097425b7/e/e23e520ed8ef3f8834a241ca)

![pickandplace](https://github.com/tuftsceeo/Braccio-OnshapeDigitalTwin/blob/main/img/pickandplace.gif)s

## How this works
The code works by making https GET requests to the onshape API to fetch information about each checkpoint position and configuration. It calculates the angles required to get to that position and then sets the robot arm to those joint angles. If monitoring is enabled it makes POST requests to set mate values for the onshape model to represent the physical state.


## Instructions
### Install Dependencies
This code requires the following arduino libraries to be installed:
* [ArduinoJson](https://arduinojson.org/)
* [BraccioIK](https://github.com/tuftsceeo/Braccio-InverseKinematics/tree/main/BraccioIK)
* [Braccio++](https://github.com/arduino-libraries/Arduino_Braccio_plusplus)
* [ArduinoOnshape](https://github.com/tuftsceeo/OnshapeArduino)

ArduinoJson and Braccio++ can be installed from the arduino library manager whereas BraccioIK and Braccio++ can be installed from the linked github repositories. They can be downloaded as a .zip file and use Arduino IDE to select the option "Add .zip libary" under sketch - include library.
### Setup Onshape
Open the [linked onshape document](https://cad.onshape.com/documents/9a453d9e3d5cb8f5b9b51eba/w/0248d727b4bba4c0097425b7/e/e23e520ed8ef3f8834a241ca) for this assembly and make a copy to edit. Then obtain your key to the onshape API [here](https://dev-portal.onshape.com/keys) (write this down for later).
### Setup Arduino code
once assembled, connect your Arduino Braccio++ robot arm to your computer and run the ObtainMinMax code in main. Open the serial monitor and you will see the value of each joint (the arm should be disengaged and free to move around). We need to get the physical min and max angle possible at each joint so move the joint to its minimum and maximum position repeating this for each joint.Write it down for later. This will look something like this: 

```
float motorMin[4] = {0, 84, 55, 38.19};
float motorMax[4] = {360, 225, 256, 251.76};
```
These are the angles for base, shoulder, elbow, and wrist respectively. We also need to find the home position of each joint this is the position where the Braccio is point straight up with the gripper open. (The original home position considers a closed gripper but we define the homed gripper as open).

```
float homePos[6] = {230.92, 159.63, 147.26, 152.15, 150.81, 180.42};
```

Open the BraccioOnshape code in ArduinoIDE which is the main code. We need to change these fields to reflect your settings at the top of the code:

![settings](https://github.com/tuftsceeo/Braccio-OnshapeDigitalTwin/blob/main/img/settings.png)

### Add SSL certificate
Follow the [instructions on the arduino website](https://support.arduino.cc/hc/en-us/articles/360016119219-How-to-add-certificates-to-Wifi-Nina-Wifi-101-Modules-) to add Onshape's SSL certificate to your Braccio's nano RP2040.

### Run code
We can now upload the code to the Arduino and run it. By default, the code waits for you to connect to the serial monitor so you must be open the serial monitor to run it. This can be changed in the code if not required but it provides useful information while sending the requests.

## Using the program

We can insert upto 10 checkpoints, while inserting the checkpoint you can set the following configuration parameters:
* **configuration**: The number for which order the checkpoint appears in the sequence
* **gripper configuration**: Whether the gripper is closed or open
* **gripper delay**: Time(in seconds) to wait at each checkpoint

![gripper](https://github.com/tuftsceeo/Braccio-OnshapeDigitalTwin/blob/main/img/gripper.gif)

Click generate when done.The checkpoint can be moved around and arranged as required within the green dome representing the range of motion.

![dome](https://github.com/tuftsceeo/Braccio-OnshapeDigitalTwin/blob/main/img/dome.gif)

Once the path is set, the enter button on the Braccio board can be pressed to execute the path.

## Research

**This program was part of a submission for Onshape undergraduate poster competition in 2022**

![OnshapePoster](https://github.com/tuftsceeo/Braccio-OnshapeDigitalTwin/blob/main/img/OnshapePoster.jpg)
