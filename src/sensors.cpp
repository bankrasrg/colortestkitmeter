/*
  sensors.cpp - 

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "sensors.h"
#include "instr.h"

/* ================================================================================================== */

colorSensor::colorSensor(): Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X) {
}

void colorSensor::initialize() {
  if (!this->begin()) {
    Serial.println("colorSensor::initialize Failed to initialize the TCS34725 sensor.");
  }
  this->setInterrupt(!instrMemory::colorSensorLED);  // Switch the onboard LED on as default setting
  this->setIntegrationTime((tcs34725IntegrationTime_t)instrMemory::colorSensorIntTime);
  this->setGain((tcs34725Gain_t)instrMemory::colorSensorGain);
}

void colorSensor::singleMeasurement() {
  this->setInterrupt(!instrMemory::colorSensorLED);
  this->setIntegrationTime((tcs34725IntegrationTime_t)instrMemory::colorSensorIntTime);
  this->setGain((tcs34725Gain_t)instrMemory::colorSensorGain);
  instrMemory::colorSensorDataIndex++;
  if (instrMemory::colorSensorDataIndex >= MEM_DEPTH) {
    instrMemory::colorSensorDataIndex = 0;
  }
  this->getRawData(&instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex], \
                   &instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex], \
                   &instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex], \
                   &instrMemory::colorSensorDataClear[instrMemory::colorSensorDataIndex]);
  instrMemory::colorSensorDataColorTemp[instrMemory::colorSensorDataIndex] = \
    this->calculateColorTemperature(instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex], \
                                    instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex], \
                                    instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex]);
  instrMemory::colorSensorDataLux[instrMemory::colorSensorDataIndex] = \
    this->calculateLux(instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex], \
                       instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex], \
                       instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex]);  
  instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] = 0;
  instrMemory::colorSensorDataGain[instrMemory::colorSensorDataIndex] = 0; 
}


