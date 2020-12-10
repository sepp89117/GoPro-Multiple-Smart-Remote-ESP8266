#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "TimedTask.h"

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

uint8_t *CamMacArray[] = {Cam1Mac, Cam2Mac, Cam3Mac, Cam4Mac, Cam5Mac, Cam6Mac, Cam7Mac, Cam8Mac};
const int maxCams = 8;
uint8_t CamIPs[maxCams][4];
//

//--------------------- defines ---------------------------------------------------------------------------
#define MAX_CMD_LENGTH 60
//

//--------------------- set MAC for NodeMCU 1.0 (ESP-12E) with 2.5.2 core ---------------------------------
uint8_t ap_mac[] = {0x84, 0xF3, 0xEB, 0xE4, 0x23, 0xDD}; // MAC-Adsress of Smart-Remote
extern "C" void preinit() {
#include "user_interface.h"
  wifi_set_opmode(SOFTAP_MODE);
  wifi_set_macaddr(SOFTAP_IF, ap_mac);
}
//

//--------------------- heart beat declarations -----------------------------------------------------------
long rateI = 0;                                  // durable counter 1
long rateI2 = 0;                                 // durable counter 2
long wtCount = 0;                                // "wt is send" counter
//

//--------------------- HT Threads ------------------------------------------------------------------------
TimedAction heartBeatThread = TimedAction(700, heartBeat);
TimedAction SubSerialMonitorCommandThread = TimedAction(50, SubSerialMonitorCommand);
//

//--------------------- WiFi events ------------------------------------------------------------
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
int toStationConnected = 0;
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
uint8_t PA[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x41, 0x02, 0x01}; // switch mode by rc (incremental)
uint8_t OO0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4F, 0x4F, 0x00}; // used by rc, keeps udp connected
uint8_t OO1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4F, 0x4F, 0x01}; // used by rc, keeps udp connected
uint8_t wt[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x77, 0x74}; // wifi
uint8_t pw[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x77}; // power
uint8_t st[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74}; // status request
uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x6C, 0x63, 0x05}; // get status display (w=60px, h=75px, 1bpp)
//

//--------------------- other declarations ----------------------------------------------------------------
const unsigned int plocalPort = 8383;           // Port der Fernbedienung
const unsigned int cremotePort = 8484;          // Port der Kamera
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

  //setup is done
  Serial.flush();
  Serial.println("");
  Serial.println("Ready!");
}

void loop() {
  yield();

  SubSerialMonitorCommandThread.check();

  if (conn) {
    if (toStationConnected > 0) {
      clientStatus();
    }

    receiveFromCam();
    heartBeatThread.check();
  }
}

void startAP() {
  WiFi.mode(WIFI_AP); // Set WiFi in AP mode
  WiFi.hostname("ESP_E423DD"); // Hostname of my Smart-Remote

  rateI = 0; // durable counter 1 reset
  rateI2 = 0; // durable counter 2 reset
  wtCount = 0; // "wt is send" counter reset
  WiFi.softAPConfig(ip, gateway, subnet);

  //Start AP
  WiFi.softAP(ssid, "", wifiChannel);

  //Start UDP
  Udp.begin(plocalPort);

  conn = true;

  Serial.print("<rcOn>");
  Serial.print(conn);
  Serial.println("</rcOn>");
}

void stopAP() {
  Udp.stop();
  WiFi.softAPdisconnect(true);
  conn = false;
  WiFi.mode(WIFI_STA); // Set WiFi in STA mode

  Serial.print("<rcOn>");
  Serial.print(conn);
  Serial.println("</rcOn>");
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  toStationConnected++; //sets todo for loop()
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  uint8_t empty[] = {0, 0, 0, 0};

  for (int i = 0; i < maxCams; i++) {
    if (memcmp(evt.mac, CamMacArray[i], 6) == 0) {
      IPAddress iAdr(CamIPs[i]);
      if (memcmp(empty, CamIPs[i], 4) != 0) {
        for (int j = 0; j < 4; j++) {
          CamIPs[i][j] = 0x0;
        }
        Serial.print("Cam ");
        Serial.print(i);
        Serial.println(" disconnected from AP");
      }
      break;
    }
  }
}

void clientStatus() {
  struct ip4_addr *IPaddress;
  IPAddress address;
  struct station_info *stat_info;

  stat_info = wifi_softap_get_station_info();

  while (stat_info != NULL) {
    uint8_t* clientMac = stat_info->bssid;
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    for (int i = 0; i < maxCams; i++) {
      if (memcmp(clientMac, CamMacArray[i], 6) == 0) {
        IPAddress iAdr(CamIPs[i]);

        if (iAdr != address) {
          for (int j = 0; j < 4; j++) {
            CamIPs[i][j] = address[j];
          }
          toStationConnected--;
          Serial.print("Cam ");
          Serial.print(i);
          Serial.println(" connected to AP");
        }
        break;
      }
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
}

byte ReadSerialMonitorString(char* sString) {
  byte nCount;
  nCount = 0;
  if (Serial.available() > 0) {
    Serial.setTimeout(50);
    nCount = Serial.readBytes(sString, MAX_CMD_LENGTH);
  }
  sString[nCount] = 0; //String terminator
  return nCount;
}

void sendToCam(uint8_t* req, int noBytes) {
  heartBeatThread.reset();
  SubSerialMonitorCommandThread.disable();

  struct ip4_addr *IPaddress;
  IPAddress address;
  stat_info = wifi_softap_get_station_info();

  req[9] = (uint8_t)rateI2;
  req[10] = (uint8_t)rateI;

  while (stat_info != NULL) {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    Udp.beginPacket(address, cremotePort);
    Udp.write(req, noBytes);
    Udp.endPacket();
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();

  //count up
  if (rateI >= 255) {
    rateI2++;
    rateI = 0;
  }
  if (rateI2 >= 255) {
    rateI2 = 0;
  }
  rateI++;

  SubSerialMonitorCommandThread.enable();
}

void receiveFromCam() {
  int noBytes = Udp.parsePacket();

  if (noBytes) {
    char inCmd[3];

    Udp.read(packetBuffer, noBytes); // read the packet into the buffer

    inCmd[0] = packetBuffer[11];
    inCmd[1] = packetBuffer[12];
    inCmd[2] = 0; //terminate string

    uint8_t* theBssid;
    int camIndex = 0;

    for (int i = 0; i < maxCams; i++) {
      IPAddress iAdr(CamIPs[i]);
      if (Udp.remoteIP() == iAdr) {
        theBssid = CamMacArray[i];
        camIndex = i;
        break;
      }
    }

    if (packetBuffer[13] == 0x1) {
      // illegal command for camera
      Serial.print("<illegal command \"");
      Serial.print(inCmd);
      Serial.print("\" in ");
      serialPrintHex(packetBuffer, noBytes);
      Serial.println(">");
    } else {
      if (strstr_P(inCmd, PSTR("lc")) != NULL) { //Screen for RC
        Serial.print("<lc>");
        serialPrintHex(packetBuffer, noBytes);
        Serial.print("</lc>@");
        serialPrintMac((uint8_t*)theBssid);
        Serial.println();
      } else if (strstr_P(inCmd, PSTR("st")) != NULL) {
        Serial.print("<st>");
        serialPrintHex(packetBuffer, noBytes);
        Serial.print("</st>@");
        serialPrintMac((uint8_t*)theBssid);
        Serial.println();
      } else if (strstr_P(inCmd, PSTR("pw")) != NULL || strstr_P(inCmd, PSTR("wt")) != NULL) {
        Serial.print("<pw1>");
        serialPrintMac((uint8_t*)theBssid);
        Serial.println("</pw1>");
      }
      //        else {
      //          Serial.print("<uknw>");
      //          serialPrintHex(packetBuffer, noBytes);
      //          Serial.println("</uknw>");
      //        }
    }
  }
}

void SubSerialMonitorCommand() {
  char sString[MAX_CMD_LENGTH + 1];

  // Check for command from Serial Monitor
  if (ReadSerialMonitorString(sString) > 0) {
    //String str(sString);

    if (strstr_P(sString, PSTR("<rc1>")) != NULL) { //strstr_P keeps sString in flash; PSTR avoid ram using
      //start softAP
      startAP();

    } else if (strstr_P(sString, PSTR("<rc0>")) != NULL) {
      //stop softAP
      stopAP();

    } else if (strstr_P(sString, PSTR("<sh1>")) != NULL) {
      //send record command
      sendToCam(SH1, 14);
      delay(50);

    } else if (strstr_P(sString, PSTR("<sh0>")) != NULL) {
      //send stop recording command
      sendToCam(SH0, 14);
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmv>")) != NULL) {
      sendToCam(CMv, 14); //change mode to video
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmp>")) != NULL) {
      sendToCam(CMp, 14); //change mode to photo
      delay(50);

    } else if (strstr_P(sString, PSTR("<cmb>")) != NULL) {
      sendToCam(CMb, 14); //change mode to burst
      delay(50);

    } else if (strstr_P(sString, PSTR("<cml>")) != NULL) {
      sendToCam(CMl, 14); //change mode to timelapse
      delay(50);

    } else if (strstr_P(sString, PSTR("<pa>")) != NULL) {
      sendToCam(PA, 15); //change mode by rc
      delay(50);

    } else if (strstr_P(sString, PSTR("<pw0>")) != NULL) {
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
  if (wtCount == 0) {
    sendToCam(lc, 14);
    delay(50);

    wtCount++;
  } else if (wtCount == 1) {
    sendToCam(st, 13);
    delay(50);

    wtCount++;
  } else if (wtCount >= 2) {
    sendToCam(OO0, 14);
    delay(250);
    sendToCam(OO1, 14);
    delay(50);

    wtCount = 0;
  }
}

void serialPrintHex(uint8_t msg[], int noBytes) {
  for (int i = 0; i < noBytes; i++) {
    Serial.print(msg[i], HEX);
    if (i != noBytes - 1) Serial.print(" ");
  }
}

void serialPrintHex(char msg[], int noBytes) {
  for (int i = 0; i < noBytes; i++) {
    Serial.print(msg[i], HEX);
    if (i != noBytes - 1) Serial.print(" ");
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

String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + "." + \
         String(ipAddress[1]) + "." + \
         String(ipAddress[2]) + "." + \
         String(ipAddress[3]);
}
