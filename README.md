# GoPro-Multiple-Smart-Remote-ESP8266
This sketch provides the simultaneous control of up to 8 GoPros with the serial monitor. The status of all cameras is continuously displayed. <br>
It can also be used with GoEasyPro.<br>
GoPro Hero 5 black, HERO 7 black and HERO 8 tested

A big thank you to theContentMint for the inspiration for UDP and the port being used.
Another thank you goes to KonradIT for inspiration ever to start with an ESP8266.<br>
<br>
Suggestions are always welcome

# How to use:
Change the MAC addresses to yours for ```Cam1Mac``` to ```Cam8Mac```<br>
Unprogrammed cameras are recognized and you are asked whether they should be paired. However, this is only temporary until the power supply is interrupted or the module is reset.<br>
With the pairing request, the mac of the cam is displayed. So the mac of the cam can be found out in a simple way.<br><br>

<b>Open serial monitor and use the following commands:</b> <br>
```
help      - Shows this help
info      - Shows infos
wakeup    - Wakes up cameras that are in deep sleep (power0 sent)
on        - Switches the smart remote on
off       - Switches the smart remote off
start     - Start recording
stop      - Stop recording
video     - Switches to video mode
photo     - Switches to photo mode
burst     - Switches to burst mode
timelapse - Switches to timelapse mode
power0    - Turns off all cameras (deep sleep)
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
