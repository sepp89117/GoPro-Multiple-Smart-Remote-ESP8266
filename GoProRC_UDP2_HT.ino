#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "TimedTask.h"
#include "GoProCam.h"

//---If you are using GoEasyPro, remove the comment from the following line
//#define GOEASYPRO

//--------------------- GoPro MAC, IP and RSSI declarations ------------------------------------------------------------
//---change these to yours
uint8_t Cam1Mac[6] = {0x04, 0x41, 0x69, 0x4F, 0x0F, 0x4B};
uint8_t Cam2Mac[6] = {0x04, 0x41, 0x69, 0x5E, 0x4A, 0x33};
uint8_t Cam3Mac[6] = {0x04, 0x41, 0x69, 0x5F, 0x11, 0x39};
uint8_t Cam4Mac[6] = {0x04, 0x41, 0x69, 0x5F, 0x72, 0x39};
uint8_t Cam5Mac[6] = {0x04, 0x41, 0x69, 0x0, 0x0, 0x0};
uint8_t Cam6Mac[6] = {0x04, 0x41, 0x69, 0x0, 0x0, 0x0};
uint8_t Cam7Mac[6] = {0x04, 0x41, 0x69, 0x0, 0x0, 0x0};
uint8_t Cam8Mac[6] = {0x04, 0x41, 0x69, 0x0, 0x0, 0x0};
//---don't change the rest---

const int maxCams = 8;
int numConnected = 0;
int newConnected = 0;
GoProCam cams[maxCams] = {GoProCam(Cam1Mac), GoProCam(Cam2Mac), GoProCam(Cam3Mac), GoProCam(Cam4Mac),
                          GoProCam(Cam5Mac), GoProCam(Cam6Mac), GoProCam(Cam7Mac), GoProCam(Cam8Mac)
                         };
//

//--------------------- defines ---------------------------------------------------------------------------
#define MAX_CMD_LENGTH 60
//

//--------------------- set MAC for NodeMCU 1.0 (ESP-12E) with 2.5.2 core ---------------------------------
uint8_t ap_mac[] = {0x84, 0xF3, 0xEB, 0xE4, 0x23, 0xDD}; // MAC-Adsress of Smart-Remote (Strangely, first byte has to be 0x84 to get 0x86 as a result)
extern "C" void preinit() {
#include "user_interface.h"
  wifi_set_opmode(SOFTAP_MODE);
  wifi_set_macaddr(SOFTAP_IF, ap_mac);
}
//

//--------------------- heart beat declarations -----------------------------------------------------------
long lowCounter = 0;                                  // durable counter 1
long highCounter = 0;                                 // durable counter 2
long cmdIndicator = 0;                                // "wt is send" counter
//

//--------------------- HT Threads ------------------------------------------------------------------------
#ifdef GOEASYPRO
TimedAction heartBeatThread = TimedAction(600, heartBeat);
#else
TimedAction heartBeatThread = TimedAction(1000, heartBeat);
#endif
//

//--------------------- WiFi events ------------------------------------------------------------
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
//

//--------------------- Cam-Commands ----------------------------------------------------------------------
uint8_t PW0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x50, 0x57, 0x00}; // power off
uint8_t SH1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x53, 0x48, 0x02}; // shutter start
uint8_t SH0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x53, 0x48, 0x00}; // shutter off
uint8_t CMv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x00}; // change mode (0: 'video')
uint8_t CMp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x01}; // change mode (1: 'photo')
uint8_t CMb[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x02}; // change mode (2: 'burst')
uint8_t CMl[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x03}; // change mode (3: 'timelapse')
uint8_t CMd[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x06}; // change mode (6: 'default mode')
//uint8_t OO0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4F, 0x4F, 0x00}; // used by rc, keeps udp connected
uint8_t OO1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4F, 0x4F, 0x01}; // used by rc, keeps udp connected
uint8_t wt[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x77, 0x74}; // wifi
//uint8_t pw[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x77}; // power
uint8_t st[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74}; // status request
uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x6C, 0x63, 0x05}; // get status display (w=60px, h=75px, 1bpp)
//

//--------------------- other declarations ----------------------------------------------------------------
const unsigned int rcUdpPort = 8383;           // Port der Fernbedienung
const unsigned int camUdpPort = 8484;          // Port der Kamera
const unsigned int wifiChannel = 1;             // Channel of my Smart-Remote = 1
const char *ssid = "HERO-RC-A1111425435131";    // SSID of my Smart-Remote
boolean conn = false;                           // indicator if AP is on
struct station_info *stat_info;
uint8_t packetBuffer[1024];                     // buffer to hold incoming and outgoing packets
IPAddress ip(10, 71, 79, 1);                    // IP of my Smart-Remote
IPAddress gateway(10, 71, 79, 1);               // GW of my Smart-Remote
IPAddress subnet(255, 255, 255, 0);             // SM of my Smart-Remote
//

//--------------------- instances -------------------------------------------------------------------------
WiFiUDP Udp;
WiFiClient _wifi_client;
//

//--------------------- program ---------------------------------------------------------------------------
void setup() {
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA); // Set WiFi in STA mode
  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  //setup is done
  Serial.flush();
  Serial.println("");
  #ifndef GOEASYPRO
  Serial.println("");
  Serial.println("Ready! Use the following commands:");
  Serial.println("on          - Switches the smart remote on");
  Serial.println("off         - Switches the smart remote off");
  Serial.println("start       - Start recording");
  Serial.println("stop        - Stop recording");
  Serial.println("video       - Switches to video mode");
  Serial.println("photo       - Switches to photo mode");
  Serial.println("burst       - Switches to burst mode");
  Serial.println("timelapse   - Switches to timelapse mode");
  Serial.println("power0      - Turns off all cameras");
  Serial.println();
  #endif
}

void loop() {
  receiveFromSerial();

  if (newConnected > 0) {
    getAssignedIp();
  }

  if (numConnected > 0) {
    heartBeatThread.check();
  }
}

void startAP() {
  WiFi.mode(WIFI_AP); // Set WiFi in AP mode
  WiFi.hostname("ESP_E423DD"); // Hostname of my Smart-Remote

  lowCounter = 0; // durable counter 1 reset
  highCounter = 0; // durable counter 2 reset
  cmdIndicator = 0; // "wt is send" counter reset
  WiFi.softAPConfig(ip, gateway, subnet);

  //Start AP
  WiFi.softAP(ssid, NULL, wifiChannel, 0, maxCams); //ssid, NULL, wifiChannel, 0, maxCams

  //Start UDP
  Udp.begin(rcUdpPort);

#ifdef GOEASYPRO
  Serial.print("<rcOn>");
  Serial.print(1);
  Serial.println("</rcOn>");
#else
  Serial.println("RC startetd");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("RC IP address: ");
  Serial.println(myIP);
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  Serial.printf("RC MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
#endif
}

void stopAP() {
  newConnected = 0;
  numConnected = 0;
  Udp.stop();
  WiFi.softAPdisconnect(true);

  WiFi.mode(WIFI_STA); // Set WiFi in STA mode
  _wifi_client.stop();

  for (int i = 0; i < maxCams; i++) {
    if (cams[i].getIp() != 0) {
      cams[i].resetIp();
      Serial.print("Cam ");
      Serial.print(i + 1);
      Serial.println(" disconnected from AP");
    }
  }

#ifdef GOEASYPRO
  Serial.print("<rcOn>");
  Serial.print(0);
  Serial.println("</rcOn>");
#else
  Serial.println("Remote off");
#endif
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  newConnected++; //sets todo for loop()
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  for (int i = 0; i < maxCams; i++) {
    if (memcmp(evt.mac, cams[i].getMac(), 6) == 0) {
      if (cams[i].getIp() != 0) {
        cams[i].resetIp();
        Serial.print("Cam ");
        Serial.print(i + 1);
        Serial.println(" disconnected from AP");
      }
      break;
    }
  }
}

void getAssignedIp() {
  struct ip4_addr *IPaddress;
  IPAddress address;
  struct station_info *stat_info;

  stat_info = wifi_softap_get_station_info();

  while (stat_info != NULL) {
    uint8_t* clientMac = stat_info->bssid;
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;


    for (int x = 0; x < maxCams; x++) {
      if (memcmp(clientMac, cams[x].getMac(), 6) == 0) {
        IPAddress xAdr(cams[x].getIp());

        if (xAdr != address) {
          cams[x].setIp((uint32_t)address);
          Serial.print("Cam ");
          Serial.print(x + 1);
          Serial.println(" connected to AP");

          newConnected--;
          numConnected++;
        }
        break;
      }
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
}

byte ReadSerialMonitorString(char* sString) {
  byte nCount = 0;

  if (Serial.available() > 0) {
    Serial.setTimeout(50);
    nCount = Serial.readBytes(sString, MAX_CMD_LENGTH);
  }
  sString[nCount] = 0; //String terminator
  return nCount;
}

void sendToCam(uint8_t* req, int numBytes) {
  struct ip4_addr *IPaddress;
  IPAddress address;
  stat_info = wifi_softap_get_station_info();

  req[9] = (uint8_t)highCounter;
  req[10] = (uint8_t)lowCounter;

  while (stat_info != NULL) {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    Udp.beginPacket(address, camUdpPort);
    Udp.write(req, numBytes);
    Udp.endPacket();
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();

  //count up
  if (lowCounter >= 255) {
    highCounter++;
    lowCounter = 0;
  }
  if (highCounter >= 255) {
    highCounter = 0;
  }
  lowCounter++;

  for (int i = 0; i < numConnected; i++) {
    receiveFromCam();
  }
}

void receiveFromCam() {
  yield();
  unsigned long receiveStart = millis();

  int numBytes = Udp.parsePacket();

  while (!numBytes && 350 > millis() - receiveStart) { //350 is the receive timeout
    yield();
    numBytes = Udp.parsePacket();
  }

  if (numBytes) {
    char inCmd[3];

    Udp.read(packetBuffer, numBytes); // read the packet into the buffer

    inCmd[0] = packetBuffer[11];
    inCmd[1] = packetBuffer[12];
    inCmd[2] = 0; //terminate string

    for (int i = 0; i < maxCams; i++) {
      IPAddress iAdr(cams[i].getIp());
      if (Udp.remoteIP() == iAdr) {

#ifdef GOEASYPRO
        if (packetBuffer[13] == 0x1) {
          // illegal command for camera
          Serial.print("<illegal command \"");
          Serial.print(inCmd);
          Serial.print("\" in ");
          serialPrintHex(packetBuffer, numBytes);
          Serial.println(">");
        } else {
          Serial.print("<");
          Serial.print(inCmd);
          Serial.print(">");
          serialPrintHex(packetBuffer, numBytes);
          Serial.print("</");
          Serial.print(inCmd);
          Serial.print(">@");
          serialPrintMac((uint8_t*)cams[i].getMac());
          Serial.println();
        }
#else

        if (packetBuffer[13] == 0x1) {
          // illegal command for camera
          Serial.print("Cam ");
          Serial.print(i + 1);
          Serial.println(" has sent: ");
          Serial.print("Illegal command \"");
          Serial.print(inCmd);
          Serial.print("\", Buffer: ");
          serialPrintHex(packetBuffer, numBytes);
          Serial.println("");
        } else {
          if (strstr_P(inCmd, PSTR("st")) != NULL) {
            Serial.print("Camera ");
            Serial.print(i + 1);
            Serial.println(" has sent: ");

            Serial.print("Mode: ");
            switch (packetBuffer[14]) { //mode
              case 0x0: //video mode
                Serial.println("video mode");
                break;
              case 0x1: //photo mode
                Serial.println("photo mode");
                break;
              case 0x2: //burst mode
                Serial.println("burst mode");
                break;
              case 0x3: //timelapse mode
                Serial.println("timelapse mode");
                break;
            }

            Serial.print("State: ");
            switch (packetBuffer[15]) { //state
              case 0x0: //standby
                Serial.println("standby");
                break;
              case 0x1: //recording
                Serial.println("recording");
                break;
            }

            Serial.println();
          }
        }
#endif

        break;
      }
    }
  }
}

void receiveFromSerial() {
  char sString[MAX_CMD_LENGTH + 1];

  // Check for command from Serial Monitor
  if (ReadSerialMonitorString(sString) > 0) {

    if (strstr_P(sString, PSTR("<rc1>")) != NULL || strstr_P(sString, PSTR("on")) != NULL) { //strstr_P keeps sString in flash; PSTR avoid ram using
      //start softAP
      startAP();

    } else if (strstr_P(sString, PSTR("<rc0>")) != NULL || strstr_P(sString, PSTR("off")) != NULL) {
      //stop softAP
      stopAP();

    } else if (strstr_P(sString, PSTR("<sh1>")) != NULL || strstr_P(sString, PSTR("start")) != NULL) {
      //send record command
      sendToCam(SH1, 14);
      delay(50);

    } else if (strstr_P(sString, PSTR("<sh0>")) != NULL || strstr_P(sString, PSTR("stop")) != NULL) {
      //send stop recording command
      sendToCam(SH0, 14);
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmv>")) != NULL || strstr_P(sString, PSTR("video")) != NULL) {
      sendToCam(CMv, 14); //change mode to video
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmp>")) != NULL || strstr_P(sString, PSTR("photo")) != NULL) {
      sendToCam(CMp, 14); //change mode to photo
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmb>")) != NULL || strstr_P(sString, PSTR("burst")) != NULL) {
      sendToCam(CMb, 14); //change mode to burst
      delay(50);

    } else if (strstr_P(sString, PSTR("<cml>")) != NULL || strstr_P(sString, PSTR("timelapse")) != NULL) {
      sendToCam(CMl, 14); //change mode to timelapse
      delay(50);

    } else if (strstr_P(sString, PSTR("<pw0>")) != NULL || strstr_P(sString, PSTR("power0")) != NULL) {
      sendToCam(PW0, 14);
      delay(200);
      sendToCam(PW0, 14);
      delay(200);
      sendToCam(PW0, 14);
      delay(500);
      sendToCam(PW0, 14);
      delay(200);
      sendToCam(PW0, 14);
      delay(200);
      sendToCam(PW0, 14);

    } else if (strstr_P(sString, PSTR("???")) != NULL) {
      //send whoAmI
      Serial.println("GPRC");

    } else {
      //undefiniert
      Serial.println("unknown command");
    }
  }
}

void heartBeat() {
  //rc sends 1x OO0, 1x OO1, 5x lc, 1x st
  if (cmdIndicator == 0) {

#ifdef GOEASYPRO
    sendToCam(lc, 14);
#else
    sendToCam(wt, 13);
#endif

    cmdIndicator++;
  } else if (cmdIndicator == 1) {
    sendToCam(st, 13);

    cmdIndicator++;
  } else if (cmdIndicator >= 2) {
    //sendToCam(OO0, 14);
    sendToCam(OO1, 14);

    cmdIndicator = 0;
  }
}

void serialPrintHex(uint8_t msg[], int numBytes) {
  for (int i = 0; i < numBytes; i++) {
    Serial.print(msg[i], HEX);
    if (i != numBytes - 1) Serial.print(" ");
  }
}

void serialPrintMac(uint8_t* bssid) {
  Serial.print(bssid[0], HEX); Serial.print(":");
  Serial.print(bssid[1], HEX); Serial.print(":");
  Serial.print(bssid[2], HEX); Serial.print(":");
  Serial.print(bssid[3], HEX); Serial.print(":");
  Serial.print(bssid[4], HEX); Serial.print(":");
  Serial.print(bssid[5], HEX); Serial.print("");
}
