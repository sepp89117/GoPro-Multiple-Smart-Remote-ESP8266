#include "GoProCam.h"

GoProCam::GoProCam(uint8_t* mac) {
  memcpy(_mac, mac, 6);
}

void GoProCam::setIp(uint32_t ip) {
  _ip = ip;
}

void GoProCam::resetIp() {
  _ip = 0;
}

void GoProCam::setMac(uint8_t* mac) {
  memcpy(_mac, mac, 6);
}

uint32_t GoProCam::getIp() {
  return _ip;
}

uint8_t* GoProCam::getMac() {
  return _mac;
}

void GoProCam::setCamTimeGotMillis(unsigned long cMillis) {
  _camTimeGotMillis = cMillis;
}

void GoProCam::setCamTime(uint8_t* camTime) {
  memcpy(_camTime, camTime, 7);
}

const char *GoProCam::getTimeString() {
  if (_camTimeGotMillis > 0) {
    char *dt = new char[21];
    char buf[21];
    unsigned long camTimeSeconds = ((unsigned long)(_camTime[3] * 86400000) +
                                    (unsigned long)(_camTime[4] * 3600000) +
                                    (unsigned long)(_camTime[5] * 60000) +
                                    (unsigned long)(_camTime[6] * 1000) +
                                    (unsigned long)(millis() - _camTimeGotMillis)) 
                                    / 1000;

    uint8_t days = (float)camTimeSeconds / 60.0f / 60.0f / 24.0f;
    uint8_t hours = (float)(camTimeSeconds - (days * 86400)) / 60.0f / 60.0f;
    uint8_t minutes = (float)(camTimeSeconds - (days * 86400) - (hours * 3600)) / 60.0f;
    uint8_t seconds = camTimeSeconds - (days * 86400) - (hours * 3600) - (minutes * 60);

    snprintf(buf,
             sizeof(buf),
             "%d-%02d-%02d, %02d:%02d:%02d",
             (int)_camTime[0] * 256 + (int)_camTime[1], //Y
             (int)_camTime[2], //M
             (int)days, //D
             (int)hours, //h
             (int)minutes, //m
             (int)seconds //s
            );

    strcpy(dt, buf);

    return dt;
  } else {
    return "n/a";
  }
}

void GoProCam::setBattLevel(uint8_t percent){
  _battLevel = percent;
}
uint8_t GoProCam::getBattLevel(){
  return _battLevel;
}
