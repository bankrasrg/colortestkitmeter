/*
  display.h - 

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef display_h
#define display_h

#include <Arduino.h>

#include <Wire.h>
#include <LCD.h>
#include <I2CIO.h>
#include <LiquidCrystal_I2C.h>

// Define parameters of LCD
#define LCD_ADDR  0x20 // 0100<A2><A1><A0> = 0x20
#define LCD_EN    2
#define LCD_RW    1
#define LCD_RS    0
#define LCD_D4    4
#define LCD_D5    5
#define LCD_D6    6
#define LCD_D7    7
#define LCD_BL    3
#define LCD_BLPOL POSITIVE

/* ================================================================================================== */

class displayDriver: public LiquidCrystal_I2C {
  public:
    displayDriver();
    void initialize();
    void clear();
    void update();
  private:
};

#endif

