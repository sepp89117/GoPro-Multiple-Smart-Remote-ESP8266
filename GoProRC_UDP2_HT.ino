#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "TimedTask.h"
#include "GoProCam.h"

//---If you are using GoEasyPro, remove the comment from the following line
//#define GOEASYPRO

//---If print LCD on serial monitor, remove the comment from the following line
//#define PRINTLCD

//--------------------- GoPro MAC, IP and RSSI declarations ------------------------------------------------------------
//---Change these to yours, set the last three digits to 0x00 for Macs that are not in use
uint8_t Cam1Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam2Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam3Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam4Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam5Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam6Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam7Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
uint8_t Cam8Mac[6] = {0x04, 0x41, 0x69, 0x00, 0x00, 0x00};
//---Don't change the rest---

const int maxCams = 8;
uint8_t numConnected = 0;
uint8_t newConnected = 0;
uint8_t registeredCams = 0;
GoProCam cams[maxCams] = {GoProCam(Cam1Mac), GoProCam(Cam2Mac), GoProCam(Cam3Mac), GoProCam(Cam4Mac),
                          GoProCam(Cam5Mac), GoProCam(Cam6Mac), GoProCam(Cam7Mac), GoProCam(Cam8Mac)
                         };
uint8_t emptyMac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t *ignoreMacArray[4] = {emptyMac, emptyMac, emptyMac, emptyMac};
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
uint8_t lowCounter = 0;                                  // durable counter 1
uint8_t highCounter = 0;                                 // durable counter 2
uint8_t cmdIndicator = 0;                                // "wt is send" counter
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
uint8_t OO1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x4F, 0x01}; // used by rc, keeps udp connected
uint8_t wt[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x77, 0x74}; // wifi
uint8_t st[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74}; // status request
uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x63, 0x05}; // get status display (w=60px, h=75px, 1bpp)

//--------------------------- List of commands, supported by Hero5 black ---------------------------
//--- GET (with byte(8) = 0)
//uint8_t cc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63}; // cam capabilities
//uint8_t cm[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6d}; // cam mode
//uint8_t cv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x76}; // cam version (response contains firmware version an model name)
//uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x63}; // lcd (get status display (w=60px, h=75px, 1bpp))
//uint8_t pv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x76}; // preview
//uint8_t se[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x65}; // settings
//uint8_t sh[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x68}; // shutter
//uint8_t st[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74}; // status
//uint8_t um[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74}; // usb mode
//uint8_t vs[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x73}; // protocol version
//--- GET (with byte(8) = 1)
//uint8_t cv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x63, 0x76}; // cam version (response contains MAC an SSID)
//uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x6C, 0x63}; // lcd (get status display (w=60px, h=75px, 1bpp))
//uint8_t pw[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x77}; // power
//uint8_t se[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x73, 0x65}; // settings
//uint8_t sh[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x73, 0x68}; // shutter
//uint8_t vs[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x76, 0x73}; // protocol version
//uint8_t wt[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x77, 0x74}; // wifi?
//--- SET (with byte(8) = 0) (last byte is parameter)
//uint8_t CM[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x4d, 0x01}; // cam mode
//uint8_t OO[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x4F, 0x01}; // one on one
//uint8_t PV[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x56, 0x01}; // preview
//uint8_t PW[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x57, 0x01}; // power
//uint8_t SA[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x41, 0x01}; // ---???--- maybe Shutter Auto?
//uint8_t SH[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x48, 0x01}; // shutter
//uint8_t UM[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x4d, 0x01}; // usb mode
//--- SET (with byte(8) = 1) (last byte is parameter)
//uint8_t OO[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4F, 0x4F, 0x01}; // one on one
//uint8_t PW[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x50, 0x57, 0x01}; // power
//uint8_t SH[]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x53, 0x48, 0x01}; // shutter
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
  
  for (uint8_t i = 0; i < maxCams; i++) { //count registered cams
    uint8_t* mac = cams[i].getMac();
    if (mac[3] != 0x0 || mac[4] != 0x0 || mac[5] != 0x0) registeredCams++;
  }

  //setup is done
  Serial.flush();
  Serial.println();
#ifndef GOEASYPRO
  Serial.println();
  printHelp();

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

void printHelp() {
  Serial.println();
  Serial.println("--------------------- HELP ---------------------");
  Serial.println("Use the following commands:");
  Serial.println("help        - Shows this help");
  Serial.println("info        - Shows infos");
  Serial.println("wakeup      - Wakes up cameras that are in deep sleep (power0 sent)");
  Serial.println("on          - Switches the smart remote on");
  Serial.println("off         - Switches the smart remote off");
  Serial.println("start       - Start recording");
  Serial.println("stop        - Stop recording");
  Serial.println("video       - Switches to video mode");
  Serial.println("photo       - Switches to photo mode");
  Serial.println("burst       - Switches to burst mode");
  Serial.println("timelapse   - Switches to timelapse mode");
  Serial.println("power0      - Turns off all cameras (deep sleep)");
  Serial.println("------------------------------------------------");
  Serial.println();
}

void printInfo() {
  Serial.println();
  Serial.println("--------------------- INFO ---------------------");

  if (conn) Serial.println("RC activated!");
  else Serial.println("RC is off!");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("RC IP address: ");
  Serial.println(myIP);

  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  Serial.printf("RC MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Cams registered: ");
  Serial.println(registeredCams);

  Serial.print("Cams connected: ");
  Serial.println(numConnected);
  Serial.println("------------------------------------------------");
  Serial.println();
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

  conn = true;

#ifdef GOEASYPRO
  Serial.print("<rcOn>");
  Serial.print(1);
  Serial.println("</rcOn>");
#else
  printInfo();
#endif
}

void stopAP() {
  newConnected = 0;
  numConnected = 0;
  Udp.stop();
  WiFi.softAPdisconnect(true);

  WiFi.mode(WIFI_STA); // Set WiFi in STA mode

  conn = false;

  for (uint8_t i = 0; i < maxCams; i++) {
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
  Serial.println("Remote turned off");
#endif
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  newConnected++; //sets todo for loop()
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  for (uint8_t i = 0; i < maxCams; i++) {
    if (memcmp(evt.mac, cams[i].getMac(), 6) == 0) {
      if (cams[i].getIp() != 0) {
        cams[i].resetIp();
        Serial.print("Cam ");
        Serial.print(i + 1);
        Serial.println(" disconnected from AP");
        Serial.println();
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
    bool unkwMac = true; //indicates if cam is unknown
    uint8_t* clientMac = stat_info->bssid;
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    for (uint8_t x = 0; x < maxCams; x++) {
      if (memcmp(clientMac, cams[x].getMac(), 6) == 0) {
        unkwMac = false;
        IPAddress xAdr(cams[x].getIp());

        if (xAdr != address) {
          cams[x].setIp((uint32_t)address);
          Serial.print("Cam ");
          Serial.print(x + 1);
          Serial.println(" connected to AP");
          Serial.println();

          newConnected--;
          numConnected++;
        }
        break; //Mac found, exit for-loop
      }
    }

    if (unkwMac) {
      bool ignore = false;
      for (uint8_t i = 0; i < 4; i++) {
        if (memcmp(clientMac, ignoreMacArray[i], 6) == 0) {
          ignore = true;
          break;
        }
      }

      if (!ignore) {
        Serial.println("Unknown cam detected.");
        Serial.print("Pair with MAC: ");
        serialPrintMac(clientMac);
        Serial.println(" ? [Y/n]");
        Serial.println();

        while (!Serial.available()); //wait for user input
        int read = Serial.read();
        if (read == 89) { //Y
          if (regNewCam(clientMac, address)) {
            numConnected++;
            Serial.print("New cam ");
            Serial.print(registeredCams);
            Serial.println(" successfully connected to AP");
            Serial.println();
          } else {
            Serial.println("Can't connect new cam to AP");
            Serial.println("Cam will be ignored!");
            Serial.println();

            ignoreMac(clientMac);
          }
        } else { //other inputs
          Serial.println("Cam will be ignored!");
          Serial.println();

          ignoreMac(clientMac);
        }
        newConnected--;
      }
    }

    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
}

bool regNewCam(uint8_t* mac, uint32_t ip) {
  if (registeredCams < 8) {
    cams[registeredCams].setMac(mac);
    cams[registeredCams].setIp(ip);
    registeredCams++;
    return true;
  } else {
    return false;
  }
}

void ignoreMac(uint8_t* mac) {
  for (uint8_t i = 0; i < 4; i++) {
    if (memcmp(emptyMac, ignoreMacArray[i], 6) == 0) {
      memcpy(ignoreMacArray[i], mac, 6);
      break;
    }
  }
}

byte ReadSerialMonitorString(char* sString) {
  uint8_t nCount = 0;

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

  for (uint8_t i = 0; i < numConnected; i++) {
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

    for (uint8_t i = 0; i < maxCams; i++) {
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
          } else if (strstr_P(inCmd, PSTR("lc")) != NULL) {
#ifdef PRINTLCD
            serialPrintLc(packetBuffer);
#endif
          }
        }
#endif

        break;
      }
    }
  }
}

void serialPrintLc(uint8_t* lcBuffer) {
  for (uint8_t y = 74; y > -1; y--) {

    //Remove empty lines to reduce the height of the graphic
    if (y == 74 ||
        y == 60 || y == 59 || y == 58 || y == 57 || y == 56 || y == 55 ||
        y == 45 || y == 44 || y == 43 || y == 42 || y == 41 ||
        y == 21 || y == 20 || y == 19 || y == 18 || y == 17 ||
        y == 14 || y == 13 ||
        y == 2 || y == 1 || y == 0) {
      continue;
    }

    //print # on serial monitor for each pixel
    for (uint8_t x = 0; x < 8; x++) {
      for (uint8_t b = 0; b < 8; b++) {
        if (x == 0 && b == 0) Serial.println();

        if (bitRead(lcBuffer[(x + (y * 8)) + 15], 7 - b))Serial.print("#");
        else Serial.print(" ");
      }
    }
  }
  Serial.println();
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
#define GOEASYPRO
      heartBeatThread.setInterval(600);
      //send whoAmI for GoEasyPro
      Serial.println("GPRC");

    } else if (strstr_P(sString, PSTR("help")) != NULL) {
      printHelp();

    } else if (strstr_P(sString, PSTR("info")) != NULL) {
      printInfo();

    } else if (strstr_P(sString, PSTR("wakeup")) != NULL) {
      wakeCams();

    } else {
      //Serial.println("unknown command");
    }
  }
}

void heartBeat() {
  //rc sends 1x OO0, 1x OO1, 5x lc, 1x st
  if (cmdIndicator == 0) {

#if defined(GOEASYPRO) || defined(PRINTLCD)
    sendToCam(lc, 14);
#else
    sendToCam(wt, 13);
#endif

    cmdIndicator++;
  } else if (cmdIndicator == 1) {
    sendToCam(st, 13);

    cmdIndicator++;
  } else if (cmdIndicator >= 2) {
    //sendToCam(OO0, 14); //Not really necessary
    sendToCam(OO1, 14); //Necessary to keep the cam connected 

    cmdIndicator = 0;
  }
}

void wakeCams() { //Wakes up cameras that are in deep sleep ("power0" sent). They are apparently off, but looking for the wifi of the remote
  if (conn) {
    Serial.println("Wake up cams!");
    Udp.stop();
    WiFi.softAPdisconnect(true);

    delay(3000);

    WiFi.softAP(ssid, NULL, wifiChannel, 0, maxCams); //ssid, NULL, wifiChannel, 0, maxCams
    Udp.begin(rcUdpPort);
  } else {
    startAP();
  }
}

void serialPrintHex(uint8_t msg[], int numBytes) {
  for (uint8_t i = 0; i < numBytes; i++) {
    Serial.print(msg[i], HEX);
    if (i != numBytes - 1) Serial.print(" ");
  }
}

void serialPrintMac(uint8_t* bssid) {
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print(bssid[i], HEX);
    if (i < 5) {
      Serial.print(':');
    }
  }
}
