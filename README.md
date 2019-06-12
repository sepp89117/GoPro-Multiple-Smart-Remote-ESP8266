# GoPro-Multiple-Smart-Remote-ESP8266
This code should show how to control several GoPros with a NodeMCU V1.0 (ESP-12E). It is designed for my purposes on four cameras. (GoPro Hero 5 black tested)

A big thank you to theContentMint for the inspiration for UDP and the port being used.
Another thank you goes to KonradIT for inspiration ever to start with an ESP8266.

# TODO
The command CM (change mode) does not work. I would like to be able to set all cameras to default mode or other modes.

# How to use:
First of all, no changes in the code are necessary. Only when the claims grow, changes must be made.

Open serialmonitor and use the following commands:
'
<rc1> //to start softAP, after that, you can connect cameras
"<rc0>" //to stop softAP
"<sh1>" //start shutter
"<sh0>" //stop shutter
"<pw0>" //power off all cams
'
all without quotes
