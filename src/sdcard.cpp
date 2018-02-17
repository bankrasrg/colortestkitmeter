/*
  sdcard.cpp - Various functions and classes

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "sdcard.h"
#include "instr.h"

/* ================================================================================================== */

instrConfigFile::instrConfigFile() {
}

boolean instrConfigFile::readConfigValue(String configParam) {
  byte configFileByte = 0;
  String configFileLine = "";
  if (!sdCard::file.open(&sdCard::root, "CONFIG.INI", O_READ)) {              
    Serial.print("instrConfigFile::readConfigValue failed to open file CONFIG.INI");
    return false;
  }
  while (sdCard::file.available() > 0) {
    byte configFileByte = sdCard::file.read();
    if (configFileByte == 10) {
      Serial.println(configFileLine);
      if (configParam == configFileLine.substring(0, configFileLine.indexOf("="))) {
        this->_configParam = configParam;
        this->_configValue = configFileLine.substring(configFileLine.indexOf("=") + 1, configFileLine.length());
        sdCard::file.close();
        this->_configValueBytes();
        return true;
      }
      else {
        configFileLine = "";  
      }
    }
    else {
      configFileLine += char(configFileByte);
    }
  }
  // Parameter not found in configuration file.
  sdCard::file.close();
  return false;
}

boolean instrConfigFile::_configValueBytes() {
  if (this->_configParam == "ip" || this->_configParam == "gateway" || this->_configParam == "subnet" || this->_configParam == "nameserver") {
    int separatorIndex = 0;
    int separatorPosition[] = {};
    for (int i = 0; i < this->_configValue.length(); i++) {
      if (this->_configValue.charAt(i) == '.') {
        separatorPosition[separatorIndex] = i;
        separatorIndex++;
      }
    }
    if (separatorIndex == 3) {
      if (this->_configParam == "ip") {
        instrMemory::ip[0] = this->_configValue.substring(0, separatorPosition[0]).toInt();
        instrMemory::ip[1] = this->_configValue.substring(separatorPosition[0] + 1, separatorPosition[1]).toInt();          
        instrMemory::ip[2] = this->_configValue.substring(separatorPosition[1] + 1, separatorPosition[2]).toInt();          
        instrMemory::ip[3] = this->_configValue.substring(separatorPosition[2] + 1, this->_configValue.length()).toInt();
        return true;
      }
      else {
        if (this->_configParam == "gateway") {
          instrMemory::gw[0] = this->_configValue.substring(0, separatorPosition[0]).toInt();
          instrMemory::gw[1] = this->_configValue.substring(separatorPosition[0] + 1, separatorPosition[1]).toInt();          
          instrMemory::gw[2] = this->_configValue.substring(separatorPosition[1] + 1, separatorPosition[2]).toInt();          
          instrMemory::gw[3] = this->_configValue.substring(separatorPosition[2] + 1, this->_configValue.length()).toInt();
          return true;
        }
        else {
          if (this->_configParam == "subnet") {
            instrMemory::snip[0] = this->_configValue.substring(0, separatorPosition[0]).toInt();
            instrMemory::snip[1] = this->_configValue.substring(separatorPosition[0] + 1, separatorPosition[1]).toInt();          
            instrMemory::snip[2] = this->_configValue.substring(separatorPosition[1] + 1, separatorPosition[2]).toInt();          
            instrMemory::snip[3] = this->_configValue.substring(separatorPosition[2] + 1, this->_configValue.length()).toInt(); 
            return true;
          }
          else {
            if (this->_configParam == "nameserver") {
              instrMemory::dnsip[0] = this->_configValue.substring(0, separatorPosition[0]).toInt();
              instrMemory::dnsip[1] = this->_configValue.substring(separatorPosition[0] + 1, separatorPosition[1]).toInt();          
              instrMemory::dnsip[2] = this->_configValue.substring(separatorPosition[1] + 1, separatorPosition[2]).toInt();          
              instrMemory::dnsip[3] = this->_configValue.substring(separatorPosition[2] + 1, this->_configValue.length()).toInt(); 
              return true;
            }
          }
        }
      }
    }
  }  
  // Incorrect configParam or incorrect configValue
  return false;
}

/* ================================================================================================== */

namespace sdCard {
  Sd2Card card;
  SdVolume volume;
  SdFile root;
  SdFile file;
  instrConfigFile configFile;

  boolean begin(const int csPinSD) {
    if (!card.init(csPinSD, SPI_HALF_SPEED)) {
      Serial.println("sdFileSystem::begin failed to initialize the card.");
      return false;
    }
    else {
      if (!volume.init(&card)) {
        Serial.println("sdFileSystem::begin failed to find FAT16/FAT32 partition.");
        return false;
      }
      else {
        root.close();
        if (!root.openRoot(&volume)) {
          Serial.println("sdFileSystem::begin failed to open root partition.");
          return false;
        }
        else {          
          if (root.exists("CONFIG.INI")) {            
            if (configFile.readConfigValue("ip")) { // && configFile.readConfigValue("gateway") && configFile.readConfigValue("subnet") && configFile.readConfigValue("nameserver")) {
              return true;
            }
            else {
              Serial.println("sdFileSystem::begin failed to read all configuration parameters.");
              return false;
            }
          }
          else {
            Serial.println("sdFileSystem::begin failed to find file CONFIG.INI.");
            return false;
          }  
        }
      }
    }
  }
  
}
