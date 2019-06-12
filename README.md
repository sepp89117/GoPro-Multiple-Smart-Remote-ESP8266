# GoPro-Multiple-Smart-Remote-ESP8266
This code should show how to control several GoPros with a NodeMCU V1.0 (ESP-12E). It is designed for my purposes on four cameras. (GoPro Hero 5 black tested)

A big thank you to theContentMint for the inspiration for UDP and the port being used.
Another thank you goes to KonradIT for inspiration ever to start with an ESP8266.

# TODO
The command CM (change mode) does not work. I would like to be able to set all cameras to default mode or other modes.

# How to use:
First of all, no changes in the code are necessary. Only when the claims grow, changes must be made.

Open serialmonitor and use the following commands: <br>
<br>
&#60;rc1> //to start softAP, after that, you can connect cameras <br>
&#60;rc0> //to stop softAP <br>
&#60;sh1> //start shutter <br>
&#60;sh0> //stop shutter <br>
&#60;pw0> //power off all cams

# Additional informations
M1BSSID is only to be changed if you want to reference the status message of the camera to the MAC address.
When the camera sends ST, the status message with the MAC address is written in the serialmonitor. If the MAC address is not known to the program, the message of the camera will not be written.
Unfortunately I can only find out which IP sends the message. The IP is not fixed, so variable. Therefore, one has to refer to the IP with the MAC address and you can say that if this IP writes, it is this MAC. I hope this is understandable... 
The same goes for M2BSSID and so on.
