/*
  colortestkitmeter.ino - Main Arduino sketch
  
  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295

  Software requirements:
    Arduino IDE version 1.5.6
    Wiznet / WIZ_Ethernet_Library (https://github.com/Wiznet/WIZ_Ethernet_Library)
    New LiquidCrystal library 1.2.1 (https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home)
    Sdfatlib FAT16/FAT32 Arduino library for SD/SDHC cards 20131225 (https://code.google.com/p/sdfatlib/)
    Adafruit TCS34725 Arduino library (https://github.com/adafruit/Adafruit_TCS34725)    
    
  The following changes need to be made to the libraries:
  1. Select the WIZ550io module in the file w5100.h (libraries/Ethernet/utility/w5100.h):
    #define W5500_ETHERNET_SHIELD
    #define WIZ550io_WITH_MACADDRESS
  2. Reduce the SPI clock frequency for the WIZ550io module in the file w5500.cpp (libraries/Ethernet/utility/w5500.cpp):
    SPI.setClockDivider(SPI_CS, 3); // 28 Mhz 
    //SPI.setClockDivider(SPI_CS, 2); // 42 Mhz
  3. The liquid crystal library uses the function digitalPinToTimer() to check if the backlight pin is connected to a digital
     output or PWM output. This function is not working in the most current version of the Arduino IDE, when compiling for Arduino
     Due. Disable the use of this function in the file LiquidCrystal.cpp (libraries/LiquidCrystal/LiquidCrystal.cpp):
    if (true)     
    //if(digitalPinToTimer(_backlightPin) != NOT_ON_TIMER)
  4. The liquid crystal library uses the function _BV(i) to obtain a byte with a 1 shifted left to position i. This function is defined
     for AVR (hardware/tools/avr/avr/include/avr/sfr_defs.h) and apparently not defined for SAM3X. Add the definition of the function to 
     file FastIO.h (libraries/LiquidCrystal/FastIO.h):
    #define _BV(bit) (1 << (bit))
  5. Change the configuration of the Sdfatlib library to use the Arduino SPI library in SdFatConfig.h (libraries/SdFat/SdFatConfig.h):
    #define USE_ARDUINO_SPI_LIBRARY 1
  6. Ensure the correct libraries are used instead of the libraries delivered with the Arduino IDE. Move the following to a backup location:
    /opt/arduino-1.5.6/libraries/Ethernet
    /opt/arduino-1.5.6/libraries/SD
  7. Disable the library delay.h in the Adafruit TCS34725 library (libraries/Adafruit_TCS34725/Adafruit_TCS34725.cpp):
    //#include <util/delay.h>
  
  This sketch is designed for an Arduino Due with the following peripherals:
  1. SD card module attached to the SPI bus:
    MISO - pin 74 (SPI MISO)
    MOSI - pin 75 (SPI MOSI)
    CLK  - pin 76 (SPI SCK)
    CS   - pin 4  (SPI CS1)
  2. WIZ550io module attached to the SPI bus: 
    MISO - pin 74 (SPI MISO)
    MOSI - pin 75 (SPI MOSI)
    CLK  - pin 76 (SPI SCK)
    CS   - pin 10 (SPI CS0)
    INT  - pin 46 (interrupt from module, active low)
    RST  - pin 48 (reset module, active low)
    RDY  - pin 50 (module ready)
  3. A third module could be attached to the SPI bus:
    MISO - pin 74 (SPI MISO)
    MOSI - pin 75 (SPI MOSI)
    CLK  - pin 76 (SPI SCK)
    CS   - pin 52 (SPI CS2)
  4. LCD module (note: 5V logic levels) attached to the I2C bus (address 0x20):
    SDA  - pin 20 (I2C TWI1 SDA)
    SCL  - pin 21 (I2C TWI1 SCL)
  5. Adafruit TCS34725 breakout board attached to the I2C bus (address 0x29):
    SDA  - pin 20 (I2C TWI1 SDA)
    SCL  - pin 21 (I2C TWI1 SCL)
*/

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <LCD.h>
#include <I2CIO.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_TCS34725.h>
#include "instr.h"

// Use an RS232 terminal for debugging output of the sketch
const boolean SerialDebugging = true;

// Define the Arduino pins to use to select chips on the SPI bus.
const int SPI_CS_SD = 4;
const int SPI_CS_WIZ550io = 10;  // Note: the CS pin is actually defined in the w5500.cpp file of the WIZ_Ethernet_Library.
const int SPI_CS_DISABLE = 52;

// The ready, reset and interrupt pins of the Wiz550io module are not defined/used in the w5500 driver.
const int INT_WIZ550io = 46;
const int RST_WIZ550io = 48;
const int RDY_WIZ550io = 50;

// Declaration of the instrument server objects
instrServer deviceServer;

/* ================================================================================================== */

void setup() {
  // Start the serial line if needed for debugging and wait for communication
  if (SerialDebugging) {
    Serial.begin(115200);  // 115200 baud
    while (!Serial) {
      delay(1);
    }
    Serial.flush();
  }

  // Set the chip select pins for the SPI bus as output
  pinMode(SPI_CS_SD, OUTPUT);  
  //pinMode(SPI_CS_WIZ550io, OUTPUT); // Do not uncomment
  pinMode(SPI_CS_DISABLE, OUTPUT);
  
  // Start and configurure the SPI bus for the third module
  digitalWrite(SPI_CS_DISABLE, HIGH);
  //SPI.begin(SPI_CS_DISABLE);
  //SPI.setClockDivider(SPI_CS_DISABLE, 84);     // 84 MHz / 84 = 1 MHz
  //SPI.setDataMode(SPI_CS_DISABLE, SPI_MODE0);  // Mode 0: CPOL = 0, CPHA = 0
  //SPI.setBitOrder(SPI_CS_DISABLE, MSBFIRST);

  // Define the ready, reset and interrupt pins of the Wiz550io module.
  pinMode(INT_WIZ550io, INPUT);
  pinMode(RST_WIZ550io, OUTPUT);
  pinMode(RDY_WIZ550io, INPUT);  
  
  // Hardware reset of the WIZ550io, min. 0.5 ms low and 50 ms high
  digitalWrite(RST_WIZ550io, LOW);
  delay(1);
  digitalWrite(RST_WIZ550io, HIGH);  
  delay(100);

  // Turn the amber LED 'L' on the Arduino board off
  pinMode(LED_BUILTIN, OUTPUT); // pin 13
  digitalWrite(LED_BUILTIN, LOW);
  
  // Start the instrument  
  deviceServer.begin(SPI_CS_SD);
}  

/* ================================================================================================== */
  
void loop() {
  // Check if any clients are available for the instrument and process their requests.
  deviceServer.available();
}
