#ifndef GOPROCAM_H
#define GOPROCAM_H

#include <ESP8266WiFi.h>

class GoProCam {
  
  public:
  	GoProCam();
    GoProCam(uint8_t* mac);
  	
    void setIp(uint32_t ip);
    void resetIp();
  	void setMac(uint8_t* mac);
  	
  	uint32_t getIp();
  	uint8_t* getMac();

  private:
  	uint8_t _mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  	uint32_t _ip = 0;	
};

#endif

/*
|| @changelog
|| | 1.0 2020-12-13 - Sebastian Balzer : Initial Release
|| | 1.1 2021-01-03 - Sebastian Balzer : Changed for-loop to memcpy function
*/
