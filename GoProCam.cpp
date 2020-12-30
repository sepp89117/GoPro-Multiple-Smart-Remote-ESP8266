#include "GoProCam.h"


GoProCam::GoProCam(uint8_t* mac) {
  for (int i = 0; i < 6; i++){
	_mac[i] = mac[i];
  }
}

void GoProCam::setIp(uint32_t ip) {
  _ip = ip;
}

void GoProCam::resetIp() {
  _ip = 0;
}

void GoProCam::setMac(uint8_t* mac) {
  for (int i = 0; i < 6; i++){
	_mac[i] = mac[i];
  }
}

uint32_t GoProCam::getIp() {
  return _ip;
}

uint8_t* GoProCam::getMac() {
  return _mac;
}
