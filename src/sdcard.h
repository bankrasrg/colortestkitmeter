/*
  sdcard.h - Various functions and classes

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef sdcard_h
#define sdcard_h

#include <Arduino.h>
#include <SdFat.h>
#include "misc.h"

/* ================================================================================================== */

class instrConfigFile {
  public:
    instrConfigFile();
    boolean readConfigValue(String configParam);
  private:
    String _configParam;
    String _configValue;
    boolean _configValueBytes();
};

/* ================================================================================================== */

namespace sdCard {
  extern Sd2Card card;
  extern SdVolume volume;
  extern SdFile root;
  extern SdFile file;
  extern instrConfigFile configFile;
  extern boolean begin(const int csPinSD);
};

#endif

