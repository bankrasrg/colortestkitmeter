/*
  sensors.h - 

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include <Adafruit_TCS34725.h>

/* ================================================================================================== */

class colorSensor: public Adafruit_TCS34725 {
  public:
    colorSensor();
    void initialize();
    void singleMeasurement();
};

#endif

