# GoPro-Multiple-Smart-Remote-ESP8266
This code should show how to control several GoPros with a NodeMCU V1.0 (ESP-12E). It is designed for my purposes on four cameras. (GoPro Hero 5 black and HERO 7 black tested)

I use the code with a VB.net Windows Forms Application. In this I have named the four cameras with their own names and can assign the individual status of the cameras by referencing from IP to MAC. 

A big thank you to theContentMint for the inspiration for UDP and the port being used.
Another thank you goes to KonradIT for inspiration ever to start with an ESP8266.

# TODO
Set Time; Keep Cams Wifi-AP alive;

# How to use:
Change the MAC addresses to yours for Cam1Mac to Cam8Mac

Open serial monitor and use the following commands: <br>
<br>
&#60;rc1> //to start softAP, after that, you can connect cameras <br>
&#60;rc0> //to stop softAP <br>
&#60;sh1> //start shutter <br>
&#60;sh0> //stop shutter <br>
&#60;pw0> //power off all cams

If your camera does not connect, connect a smart remote in the camera while RC is active.

# Additional informations
I use this program to control 4 cameras at the same time. To do this, I use software on the PC. You can find this under GoEasyPro, written in vb.net with MS Visual Studio.

Go to <a href="https://github.com/sepp89117/GoEasyPro">GoEasyPro</a>
