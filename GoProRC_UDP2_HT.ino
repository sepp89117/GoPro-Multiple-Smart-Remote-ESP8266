#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <TimedAction.h>

//--------------------- defines ---------------------------------------------------------------------------
#define MAX_CMD_LENGTH 60
//#define DEBUG
//#define DEBUG_RX
//#define DEBUG_RX2
//
//--------------------- set MAC for NodeMCU 1.0 (ESP-12E) with 2.5.2 core ---------------------------------
uint8_t ap_mac[] = {0x84, 0xF3, 0xEB, 0xE4, 0x23, 0xDD}; // MAC-Adsress of my Smart-Remote
extern "C" void preinit() {
#include "user_interface.h"
  wifi_set_opmode(SOFTAP_MODE);
  wifi_set_macaddr(SOFTAP_IF, ap_mac);
}
//
//--------------------- heart beat declarations -----------------------------------------------------------
uint8_t stdMsg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
long rateI = 0;                                  // durable counter 1
long rateI2 = 0;                                 // durable counter 2
long rateI3 = 1;                                 // durable counter 3
long wtCount = 4;                                // "wt is send" counter
unsigned long previousMillis1 = 0;               // will store last time mac1 was shown
unsigned long previousMillis2 = 0;               // will store last time mac2 was shown
unsigned long previousMillis3 = 0;               // will store last time mac3 was shown
unsigned long previousMillis4 = 0;               // will store last time mac4 was shown
const long interval = 1500;                     // interval at which to send cmd (milliseconds)
//
//--------------------- HT Threads ------------------------------------------------------------------------
TimedAction heartBeatThread = TimedAction(1500, heartBeat);
TimedAction SubSerialMonitorCommandThread = TimedAction(50, SubSerialMonitorCommand);
TimedAction client_statusThread = TimedAction(2000, client_status); // for assignment of IP to MAC showFreeHeap
TimedAction showFreeHeapThread = TimedAction(2000, showFreeHeap);

//
//--------------------- GoPro MAC declarations ------------------------------------------------------------
uint8_t M1BSSID[] = {0x04, 0x41, 0x69, 0x4F, 0x0F, 0x4B};
uint8_t M2BSSID[] = {0x04, 0x41, 0x69, 0x5E, 0x4A, 0x33};
uint8_t M3BSSID[] = {0x04, 0x41, 0x69, 0x5F, 0x11, 0x39};
uint8_t M4BSSID[] = {0x04, 0x41, 0x69, 0x5F, 0x72, 0x39};
IPAddress M1IP(0, 0, 0, 0);
IPAddress M2IP(0, 0, 0, 0);
IPAddress M3IP(0, 0, 0, 0);
IPAddress M4IP(0, 0, 0, 0);
//
//--------------------- Cam-Commands ----------------------------------------------------------------------
uint8_t PW0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x50, 0x57, 0x00}; // power off
uint8_t SH1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x53, 0x48, 0x01}; // shutter 1
uint8_t SH0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x53, 0x48, 0x00}; // shutter 0
uint8_t CM0[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x4D, 0x01}; // change mode <- not working
uint8_t wt[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x77, 0x74}; // wait
uint8_t pw[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x70, 0x77}; // hold power on
uint8_t st[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x73, 0x74}; // status?
uint8_t cv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x63, 0x76}; // cam version
uint8_t OO[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x4F, 0x4F}; // UNKNOWN
uint8_t se[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x73, 0x65}; // UNKNOWN
uint8_t lc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x6C, 0x63}; // Display image
//
//--------------------- other declarations ----------------------------------------------------------------
const unsigned int plocalPort = 8383;            // Port der Fernbedienung
const unsigned int cremotePort = 8484;           // Port der Kamera
const unsigned int wifiChannel = 1;             // Channel of my Smart-Remote = 1
const char *ssid = "HERO-RC-A1111425435131";    // SSID of my Smart-Remote "HERO-RC-A1111425435131"
boolean conn = false;                           // indicator if AP is on
struct station_info *stat_info;
byte packetBuffer[1024];                        // buffer to hold incoming and outgoing packets
int previousClients = 0;                        // will store last client count
unsigned char oldNumber_client = 0;             // will store last client count
unsigned long oldTime = 0;                      // will store last client count show time
uint32_t memcurr = 0;
uint32_t memlast = 0;
uint32_t counter = 0;
IPAddress ip(10, 71, 79, 1);                    // IP of my Smart-Remote
IPAddress gateway(10, 71, 79, 1);               // GW of my Smart-Remote
IPAddress subnet(255, 255, 255, 0);             // SM of my Smart-Remote
//
//--------------------- instances -------------------------------------------------------------------------
WiFiUDP Udp;
//
//--------------------- program ---------------------------------------------------------------------------
void setup() {
  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  Serial.begin(115200);
  while (!Serial); // wait for serial attach
  WiFi.mode(WIFI_AP); // Set WiFi in AP mode
  WiFi.hostname("ESP_E423DD"); // Hostname of my Smart-Remote
  //setup is done
  Serial.flush();
  Serial.println("");
  Serial.println("Setup done.");
}

void loop() {
  //Serial.flush();
  showFreeHeapThread.check();

  // Check for a command from the Serial Monitor
  SubSerialMonitorCommandThread.check();

  // Check client status
  if (conn == true) {
    //let the heart beating
    heartBeatThread.check();
    reciveReq();
    client_statusThread.check();
  }
}

void startAP() {
  rateI = 0; // durable counter 1 reset
  rateI2 = 0; // durable counter 2 reset
  rateI3 = 1; // durable counter 3 reset
  wtCount = 4; // "wt is send" counter reset
  WiFi.softAPConfig(ip, gateway, subnet);

  //Start AP
  WiFi.softAP(ssid, "", wifiChannel);

  // Start UDP
  Udp.begin(plocalPort);
  
#ifdef DEBUG
  Serial.print("<rcMAC>");
  Serial.print(WiFi.softAPmacAddress());
  Serial.println("</rcMAC>");

  Serial.print("<rcSSID>");
  Serial.print(ssid);
  Serial.println("</rcSSID>");

  Serial.print("<rcIP>");
  Serial.print(WiFi.softAPIP());
  Serial.println("</rcIP>");
#endif

  conn = true;

  Serial.print("<rcOn>");
  Serial.print(conn);
  Serial.println("</rcOn>");
}

void stopAP() {
  Udp.stop();
  WiFi.softAPdisconnect(true);
  conn = false;

  Serial.print("<rcOn>");
  Serial.print(conn);
  Serial.println("</rcOn>");
}

void client_status() {
  unsigned char number_client;
  struct ip4_addr *IPaddress;
  IPAddress address;
  struct station_info *stat_info;

  number_client = wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();

  while (stat_info != NULL) {
    uint8_t* clientBSSID = stat_info->bssid;
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    if (memcmp(clientBSSID, M1BSSID, 6) == 0) {
      M1IP = address;
    } else if (memcmp(clientBSSID, M2BSSID, 6) == 0) {
      M2IP = address;
    } else if (memcmp(clientBSSID, M3BSSID, 6) == 0) {
      M3IP = address;
    } else if (memcmp(clientBSSID, M4BSSID, 6) == 0) {
      M4IP = address;
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
}

void showFreeHeap() {
  memcurr = ESP.getFreeHeap();
  Serial.printf("FREEHeap: %d; DIFF %d\n", memcurr, memcurr - memlast);
  memlast = memcurr;
}

byte ReadSerialMonitorString(char* sString) {
  // Declarations
  byte nCount;
  nCount = 0;
  if (Serial.available() > 0)
  {
    Serial.setTimeout(20);
    nCount = Serial.readBytes(sString, MAX_CMD_LENGTH);
  }
  // Terminate the string
  sString[nCount] = 0;
  return nCount;
}

void sendReq(uint8_t* req, int noBytes) {
  struct ip4_addr *IPaddress;
  IPAddress address;
  stat_info = wifi_softap_get_station_info();

  while (stat_info != NULL) {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

  Udp.beginPacket(address, cremotePort);
  Udp.write(req, noBytes);
  Udp.endPacket();
    
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info();
}


void reciveReq() {
  int noBytes = Udp.parsePacket();

  if (noBytes) {
    String received_command = "";
    unsigned long currentMillis = millis();

    // We've received a packet, read the data from it
    Udp.read(packetBuffer, noBytes); // read the packet into the buffer

    //HEX 2 String
    for (int i = 1; i <= noBytes; i++) {
      received_command = received_command + char(packetBuffer[i - 1]);
    }

    if (Udp.remoteIP() == M1IP) {
      if (currentMillis - previousMillis1 >= 300) {
        if (received_command.indexOf("st") >= 0) {
          Serial.print("<st>");
          serialPrintHex(packetBuffer, noBytes);
          Serial.print("</st>@");
          serialPrintMac((uint8_t*)M1BSSID);
          Serial.println();
          previousMillis1 = currentMillis;
        } else if (received_command.indexOf("pw") >= 0 || received_command.indexOf("wt") >= 0) {
          Serial.print("<pw1>");
          serialPrintMac((uint8_t*)M1BSSID);
          Serial.println("</pw1>");
          previousMillis1 = currentMillis;
        }
      }
    } else if (Udp.remoteIP() == M2IP) {
      if (currentMillis - previousMillis2 >= 300) {
        if (received_command.indexOf("st") >= 0) {
          Serial.print("<st>");
          serialPrintHex(packetBuffer, noBytes);
          Serial.print("</st>@");
          serialPrintMac((uint8_t*)M2BSSID);
          Serial.println();
          previousMillis2 = currentMillis;
        } else if (received_command.indexOf("pw") >= 0 || received_command.indexOf("wt") >= 0) {
          Serial.print("<pw1>");
          serialPrintMac((uint8_t*)M2BSSID);
          Serial.println("</pw1>");
          previousMillis2 = currentMillis;
        }
      }
    } else if (Udp.remoteIP() == M3IP) {
      if (currentMillis - previousMillis3 >= 300) {
        if (received_command.indexOf("st") >= 0) {
          Serial.print("<st>");
          serialPrintHex(packetBuffer, noBytes);
          Serial.print("</st>@");
          serialPrintMac((uint8_t*)M3BSSID);
          Serial.println();
          previousMillis3 = currentMillis;
        } else if (received_command.indexOf("pw") >= 0 || received_command.indexOf("wt") >= 0) {
          Serial.print("<pw1>");
          serialPrintMac((uint8_t*)M3BSSID);
          Serial.println("</pw1>");
          previousMillis3 = currentMillis;
        }
      }
    } else if (Udp.remoteIP() == M4IP) {
      if (currentMillis - previousMillis4 >= 300) {
        if (received_command.indexOf("st") >= 0) {
          Serial.print("<st>");
          serialPrintHex(packetBuffer, noBytes);
          Serial.print("</st>@");
          serialPrintMac((uint8_t*)M4BSSID);
          Serial.println();
          previousMillis4 = currentMillis;
        } else if (received_command.indexOf("pw") >= 0 || received_command.indexOf("wt") >= 0) {
          Serial.print("<pw1>");
          serialPrintMac((uint8_t*)M4BSSID);
          Serial.println("</pw1>");
          previousMillis4 = currentMillis;
        }
      }
    }
  }
}

void SubSerialMonitorCommand() {
  // Declarations
  char sString[MAX_CMD_LENGTH + 1];

  // Check for command from Serial Monitor
  int nLen = ReadSerialMonitorString(sString);
  if (nLen > 0)
  {
    String str(sString);

    if (str.indexOf("<rc1>") >= 0) {
      //start softAP
      startAP();
    } else if (str.indexOf("<rc0>") >= 0) {
      //stop softAP
      stopAP();
    } else if (str.indexOf("<sh1>") >= 0) {
      //send record command
      sendReq(SH1, 14);
      delay(200);
      sendReq(SH1, 14);
      delay(200);
      sendReq(SH1, 14);
      delay(200);
      sendReq(SH1, 14);
      delay(200);
      sendReq(SH1, 14);
    } else if (str.indexOf("<sh0>") >= 0) {
      //send stop recording command
      sendReq(SH0, 14);
      delay(200);
      sendReq(SH0, 14);
      delay(200);
      sendReq(SH0, 14);
      delay(200);
      sendReq(SH0, 14);
      delay(200);
      sendReq(SH0, 14);
    } else if (str.indexOf("<st>") >= 0) {
      sendReq(st, 13);
      delay(200);
      sendReq(st, 13);
      delay(200);
      sendReq(st, 13);
      delay(200);
      //sendReq(st, 13);
    } else if (str.indexOf("<se>") >= 0) {
      sendReq(se, 13);
      delay(200);
    } else if (str.indexOf("<oo>") >= 0) {
      sendReq(OO, 13);
      delay(200);
    } else if (str.indexOf("<cm0>") >= 0) {
      if (rateI >= 255) {
        rateI2++;
        rateI = 0;
      }
      if (rateI2 >= 255) {
        rateI3++;
        rateI2 = 0;
      }
      if (rateI3 >= 255) {
        rateI = 0;
        rateI2 = 0;
        rateI3 = 1;
      }
      CM0[8] = (uint8_t)rateI3;
      CM0[9] = (uint8_t)rateI2;
      CM0[10] = (uint8_t)rateI;
      rateI++;

      sendReq(CM0, 14);
      delay(200);
    } else if (str.indexOf("<pw0>") >= 0) {
      sendReq(PW0, 14);
      delay(200);
      sendReq(PW0, 14);
      delay(200);
      sendReq(PW0, 14);
      delay(500);
      sendReq(PW0, 14);
      delay(200);
      sendReq(PW0, 14);
      delay(200);
      sendReq(PW0, 14);
      delay(500);
    } else if (str.indexOf("???") >= 0) {
      //send whoAmI
      Serial.println("GPRC");
    } else {
      //undefiniert
      Serial.println("unknown command");
      unsigned long oldTime = 0;
    }
  }
}

void heartBeat() {

  if (rateI >= 255) {
    rateI2++;
    rateI = 0;
  }
  if (rateI2 >= 255) {
    rateI3++;
    rateI2 = 0;
  }
  if (rateI3 >= 255) {
    rateI = 0;
    rateI2 = 0;
    rateI3 = 1;
  }

  stdMsg[8] = (uint8_t)rateI3;
  stdMsg[9] = (uint8_t)rateI2;
  stdMsg[10] = (uint8_t)rateI;

  if (wtCount >= 3) {
    stdMsg[11] = {0x70}; // p
    stdMsg[12] = {0x77}; // w

    //send 4x PW without counting
    sendReq(stdMsg, 13);
    delay(230);

    sendReq(stdMsg, 13);
    delay(230);

    sendReq(stdMsg, 13);
    delay(230);

    sendReq(stdMsg, 13);
    delay(230);

    wtCount = 0;
    rateI++;
    return;
  } else {
    stdMsg[11] = {0x77}; // w
    stdMsg[12] = {0x74}; // t

    //send WT
    sendReq(stdMsg, 13);
    rateI++;
    wtCount++;
  }
}

void serialPrintHex(uint8_t msg[], int noBytes) {
  String received_command = "";
  for (int i = 1; i <= noBytes; i++) {
    Serial.print(msg[i - 1], HEX);
    received_command = received_command + char(msg[i - 1]);
    if (i % 44 == 0) {
      Serial.println();
    } else Serial.print(" ");
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
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}

void mac2str(const uint8_t* ptr, char* string) {
  sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
  return;
}
