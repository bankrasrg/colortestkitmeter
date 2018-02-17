/*
  instr.cpp - Library for VXI-11 instrument on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "instr.h"

/* ================================================================================================== */

instrServer::instrServer():
  portmapServer(PMAP_PROG, PMAP_VERS, PMAP_PORT),
  vxi11CoreServer(DEVICE_CORE_PROG, DEVICE_CORE_VERS, DEVICE_CORE_PORT),
  vxi11AsyncServer(DEVICE_ASYNC_PROG, DEVICE_ASYNC_VERS, DEVICE_ASYNC_PORT),
  vxi11IntrServer(DEVICE_INTR_PROG, DEVICE_INTR_VERS, DEVICE_INTR_PORT),
  webServer(HTTP_PORT) {
}

void instrServer::begin(const int csPinSD) {
  // Reset the instrument memory
  instrMemory::resetMemory();
  
  // Initialize the SD card
  sdCard::begin(csPinSD);

  // Start ethernet
  IPAddress ip(instrMemory::ip[0], instrMemory::ip[1], instrMemory::ip[2], instrMemory::ip[3]);
  IPAddress dnsip(instrMemory::dnsip[0], instrMemory::dnsip[1], instrMemory::dnsip[2], instrMemory::dnsip[3]);
  IPAddress gw(instrMemory::gw[0], instrMemory::gw[1], instrMemory::gw[2], instrMemory::gw[3]);
  IPAddress snip(instrMemory::snip[0], instrMemory::snip[1], instrMemory::snip[2], instrMemory::snip[3]);
  Ethernet.begin(ip, dnsip, gw, snip);

  // Start the portmap server
  Serial.println("Begin portmapServer ...");
  this->portmapServer.begin();

  // Start the VXI11 servers
  Serial.println("Begin vxi11CoreServer ...");
  this->vxi11CoreServer.begin(this->portmapServer.registerServer(this->vxi11CoreServer.intentions()));
  Serial.println("Begin vxi11AsyncServer ...");
  this->vxi11AsyncServer.begin(this->portmapServer.registerServer(this->vxi11AsyncServer.intentions()));
  Serial.println("Begin vxi11IntrServer ...");
  this->vxi11IntrServer.begin(this->portmapServer.registerServer(this->vxi11IntrServer.intentions()));

  // Start the web server
  Serial.println("Begin webServer ...");
  this->webServer.begin();

  // Initialize the LCD display
  Serial.println("Initialize LCD display ...");
  this->lcd.initialize();
  
  // Initialize the color sensor  
  Serial.println("Initialize color sensor ...");
  this->rgbSensor.initialize();
}

void instrServer::available() {
  this->portmapServer.available();
  this->vxi11CoreServer.available();
  this->vxi11AsyncServer.available();
  this->vxi11IntrServer.available();
  this->webServer.available();
  this->rgbSensor.singleMeasurement();
  instrMemory::updateDisplay = true;
  this->lcd.update();
}

namespace instrMemory {
  uint16_t colorSensorDataRed[MEM_DEPTH];
  uint16_t colorSensorDataGreen[MEM_DEPTH];
  uint16_t colorSensorDataBlue[MEM_DEPTH];
  uint16_t colorSensorDataClear[MEM_DEPTH];
  uint16_t colorSensorDataColorTemp[MEM_DEPTH];
  uint16_t colorSensorDataLux[MEM_DEPTH];
  byte colorSensorDataIntTime[MEM_DEPTH];
  byte colorSensorDataGain[MEM_DEPTH];
  unsigned int colorSensorDataIndex = 0;
  boolean colorSensorLED = true;
  byte colorSensorIntTime = 0x00;
  byte colorSensorGain = 0x00;
  boolean updateDisplay = false;
  void resetMemory() {
    for (int i = 0; i < MEM_DEPTH; i++) {
      colorSensorDataRed[i] = 0;
      colorSensorDataGreen[i] = 0;
      colorSensorDataBlue[i] = 0;
      colorSensorDataClear[i] = 0;
      colorSensorDataColorTemp[i] = 0;  
      colorSensorDataLux[i] = 0;
      colorSensorDataIntTime[i] = 0;
      colorSensorDataGain[i] = 0;
    }
    colorSensorDataIndex = 0;
  }

  String instrIDN = "WZ1295,2014,0002,140802b";

  // The IP and MAC address of the instrument will be read from a configuration file in flash memory (SD card).
  // In case the file does not exist or the flash memory cannot be read, default values will be used.
  byte ip[4] = {192, 168, 1, 91};
  byte gw[4] = {192, 168, 1, 1};
  byte snip[4] = {255, 255, 255, 0};
  byte dnsip[4] = {192, 168, 1, 1};
}
