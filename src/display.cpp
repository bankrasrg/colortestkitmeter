/*
  display.cpp - 

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "display.h"
#include "instr.h"

/* ================================================================================================== */

displayDriver::displayDriver(): LiquidCrystal_I2C(LCD_ADDR, LCD_EN, LCD_RW, LCD_RS, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_BL, LCD_BLPOL) {
}

void displayDriver::initialize() {
  // Display a start-up message
  this->begin(20, 4);
  this->home();
  this->setBacklight(1);
  this->setCursor(0, 0);   // column 0, row 0
  this->print("WIZnet 2014");
  this->setCursor(0, 1);   // column 0, row 1
  this->print("Connect the Magic");
  this->setCursor(0, 2);   // column 0, row 2
  this->print("Design Challenge");
  this->setCursor(0, 3);   // column 0, row 3
  this->print("Entry WZ1295");
  delay(2000);
}

void displayDriver::clear() {
  // Clear the display
  for (int i = 0; i < 4; i++) {
    this->setCursor(0, i); // column 0, row i
    this->print("                    ");
  }
  this->setCursor(0, 0);   // column 0, row 0
}

void displayDriver::update() {
  // Display the last measurement result
  if (instrMemory::updateDisplay) {
    this->setCursor(0, 0);   // column 0, row 0
    this->print(" ---   WZ1295   --- ");
    
    this->setCursor(0, 1);   // column 0, row 1
    
    this->print("R 0x");
    String hexString = String(instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex], HEX);
    int s = hexString.length();
    for (int i = 0; i < 4 - s; i++) {
     this->print("0");  
    }
    hexString.toUpperCase();
    this->print(hexString);

    this->print("  C 0x");
    hexString = String(instrMemory::colorSensorDataClear[instrMemory::colorSensorDataIndex], HEX);
    s = hexString.length();
    for (int i = 0; i < 4 - s; i++) {
     this->print("0");  
    }
    hexString.toUpperCase();
    this->print(hexString);
    
    this->setCursor(0, 2);   // column 0, row 2

    this->print("G 0x");
    hexString = String(instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex], HEX);
    s = hexString.length();
    for (int i = 0; i < 4 - s; i++) {
     this->print("0");  
    }
    hexString.toUpperCase();
    this->print(hexString);

    this->print("  Tc ");
    hexString = String(instrMemory::colorSensorDataColorTemp[instrMemory::colorSensorDataIndex], DEC);
    s = hexString.length();
    for (int i = 0; i < 4 - s; i++) {
     this->print("0");  
    }
    this->print(hexString);
    this->print(" K");

    this->setCursor(0, 3);   // column 0, row 3

    this->print("B 0x");
    hexString = String(instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex], HEX);
    s = hexString.length();
    for (int i = 0; i < 4 - s; i++) {
     this->print("0");  
    }
    hexString.toUpperCase();
    this->print(hexString);

    this->print("  I ");
    hexString = String(instrMemory::colorSensorDataLux[instrMemory::colorSensorDataIndex], DEC);
    s = hexString.length();
    for (int i = 0; i < 5 - s; i++) {
     this->print("0");  
    }
    this->print(hexString);
    this->print(" lx");
  }  
}





 
