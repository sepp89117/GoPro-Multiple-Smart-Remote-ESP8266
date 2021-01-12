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
    void setCamTimeGotMillis(unsigned long cMillis);
    void setCamTime(uint8_t* camTime);
    void setBattLevel(uint8_t percent);

    uint32_t getIp();
    uint8_t* getMac();
    const char *getTimeString();
    uint8_t getBattLevel();

  private:
    uint8_t _mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t _ip = 0;
    unsigned long _camTimeGotMillis = 0;
    uint8_t _camTime[7] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}; //Yh, Yl, M, D, h, m, s
    uint8_t _battLevel = 0;
};

#endif

/*
  || @changelog
  || | 1.0 2020-12-13 - Sebastian Balzer : Initial Release
  || | 1.1 2021-01-03 - Sebastian Balzer : Changed for-loop to memcpy function
  || | 1.2 2021-01-12 - Sebastian Balzer : Added DateTime and BattLevel
*/
