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
