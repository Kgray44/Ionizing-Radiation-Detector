# Ionizing-Radiation-Detector
This code is for building an Ionizing Radiation Detector using Arduino!

Check out the tutorial for this code here:
https://www.hackster.io/k-gray/ionizing-radiation-detector-a0a782

# How to code the Microcontroller

What you need:
* USB-C cable
* PC

## Step 1
First, download the Arduino IDE from here:
https://downloads.arduino.cc/arduino-ide

## Step 2
Once you have the IDE downloaded, download this Ionizing Radiation Detector code by clicking [here](https://github.com/Kgray44/Ionizing-Radiation-Detector/archive/refs/heads/main.zip).

## Step 3
Once the code is downloaded, open the downloaded folder and click on the "geiger.ino" file.  It will automatically open in the Arduino IDE.  Be patient though, as this may take up to a minute to open.

## Step 4
You have to select the correct microcontroller and USB port before you are able to program.  Click on *tools / board / Arduino AVR Boards / Arduino Leonardo*.  That sets the board to Leonardo (Atmega32-U4).  Now, click on *tools / port / '/dev/cu/usbmodem14101'* (''this is the usual port on MacOS for a Leonardo).

## Step 5
Now, plug the USB cable into your PC and into the Leonardo.

## Step 6
Click the arrow (upload) button to the top left of the sketch, and it will program the board!

## Step 7
If you click the magnifying glass icon to the top right of the sketch, it will open the Serial Monitor.  This is great for debugging!
