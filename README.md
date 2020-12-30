# GoPro-Multiple-Smart-Remote-ESP8266
This code should show how to control several GoPros with a NodeMCU V1.0 (ESP-12E) or a Wemos D1 mini. It is designed for my purposes on four cameras. (GoPro Hero 5 black, HERO 7 black and HERO 8 tested)

I use the code with a VB.net Windows Forms Application. In this I have named the four cameras with their own names and can assign the individual status of the cameras by referencing from IP to MAC. 

A big thank you to theContentMint for the inspiration for UDP and the port being used.
Another thank you goes to KonradIT for inspiration ever to start with an ESP8266.

# How to use:
Change the MAC addresses to yours for ```Cam1Mac``` to ```Cam8Mac```

<b>Open serial monitor and use the following commands:</b> <br>
```
help      - Shows this help<br>
info      - Shows infos<br>
wakeup    - sends Wake on LAN to each camera<br>
on        - Switches the smart remote on<br>
off       - Switches the smart remote off<br>
start     - Start recording<br>
stop      - Stop recording<br>
video     - Switches to video mode<br>
photo     - Switches to photo mode<br>
burst     - Switches to burst mode<br>
timelapse - Switches to timelapse mode<br>
power0    - Turns off all cameras
```
If your camera does not connect, connect a smart remote in the camera while RC is active.

# LCD on serial monitor - funny but useless with multiple cameras
For some fun, you can uncomment ```//#define PRINTLCD```. You will then see an LCD screen in the serial monitor. It looks like this:<br>
```
############################################################    
####         #################################### ##########    
###           ###  ########  ################ ### ### ######    
###           ##   ########  ################# ##### #######    
###           #    ########  ###################   #########    
###           #    ########  ################## #   ########    
###  ###      #    ########  ###############  # ##  #  #####    
###  ###      #    ########  ################## ### ########    
###  ###      ##   ########      ###############   #########    
###           ###  ########      ############# ##### #######    
####         ################################ ### ### ######    
################################################# ##########    
############################################################    
                                                                
  ##  ####   ####   ####           ###  ####                    
 ### ##  ## ##  ## ##  ##         #### ##  ##                   
  ## ##  ## ##  ## ##  ##        ## ## ##  ##                   
  ## ##  ##  ####  ##  ## ##### ##  ##  ####                    
  ## ##  ## ##  ## ##  ##       ###### ##  ##                   
  ## ##  ## ##  ## ##  ##           ## ##  ##                   
  ##  ####   ####   ####            ##  ####                    
                                                                
  ###########    #######                                        
  ###########   #########                                       
         ####  ####   ####                                      
        ####   ####   ####                                      
        ####   ####   ####                                      
       #####   ####   ####                                      
       ####    ####   ####                                      
       ####    ####   ####                                      
      #####    ####   ####                                      
      ####     ####   ####                                      
      ####     ####   ####                                      
     #####     ####   ####                                      
     ####      ####   ####                                      
     ####      ####   ####                                      
    #####      ####   ####                                      
    ####       ####   ####                                      
    ####        #########                                       
   #####         #######                                        
                                                                
############################################################    
                                                                
                               ###############    #######       
    ##  #   #      ##  ###     #             #   #       #      
   ###  #   #     ### ## ##    #         ### #  #  #####  #     
    ##  #   # #  #### ## ##   ##         ### #    #     #       
    ##  #####   ## ##  ####   ##         ### #      ###         
    ##  #   #   #####    ##   ##         ### #     #   #        
    ##  #   # #    ## ## ##    #         ### #       #          
   #### #   #      ##  ###     #             #      ###         
                               ###############       #          
```
# Additional informations
I use this program to control 4 cameras at the same time. To do this, I use software on the PC. You can find this under GoEasyPro, written in vb.net with MS Visual Studio.<br>
<b> For use with GoEasyPro, ```//#define GOEASYPRO``` must be uncommented </b><br><br>
Go to <a href="https://github.com/sepp89117/GoEasyPro">GoEasyPro</a>
