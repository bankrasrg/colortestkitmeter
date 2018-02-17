/*
  instr.h - Library for VXI-11 instrument on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef instr_h
#define instr_h

#include <Arduino.h>
#include "rpc.h"
#include "portmap.h"
#include "vxi11.h"
#include "http.h"
#include "display.h"
#include "sdcard.h"
#include "sensors.h"

#define MEM_DEPTH 2048

/* ================================================================================================== */

class instrServer {
  public:
    instrServer();
    rpcPortmapServer portmapServer;
    rpcVXI11Server vxi11CoreServer;
    rpcVXI11Server vxi11AsyncServer;
    rpcVXI11Server vxi11IntrServer;
    httpServer webServer;
    displayDriver lcd;
    colorSensor rgbSensor;
    void begin(const int csPinSD);
    void available();
  private:
};

namespace instrMemory {
  extern uint16_t colorSensorDataRed[MEM_DEPTH];
  extern uint16_t colorSensorDataGreen[MEM_DEPTH];
  extern uint16_t colorSensorDataBlue[MEM_DEPTH];
  extern uint16_t colorSensorDataClear[MEM_DEPTH];
  extern uint16_t colorSensorDataColorTemp[MEM_DEPTH];
  extern uint16_t colorSensorDataLux[MEM_DEPTH];
  extern byte colorSensorDataIntTime[MEM_DEPTH];
  extern byte colorSensorDataGain[MEM_DEPTH];
  extern unsigned int colorSensorDataIndex;
  extern boolean colorSensorLED;
  extern byte colorSensorIntTime;
  extern byte colorSensorGain;
  extern boolean updateDisplay;
  extern void resetMemory();
  extern String instrIDN;
  extern byte ip[4];
  extern byte gw[4];
  extern byte snip[4];
  extern byte dnsip[4];
};

#endif

