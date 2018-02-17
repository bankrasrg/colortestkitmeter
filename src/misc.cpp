/*
  misc.cpp - Various functions and classes

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "misc.h"

/* ================================================================================================== */

byte getCharValue(char c) { 
 if (c >= '0' && c <= '9')
   return (byte)(c - '0');
 else
   return (byte)(c - 'A' + 10);
}

/* ================================================================================================== */

String ipToString(byte i[4]) { 
  String s = String(i[0], DEC) + ":" + String(i[1], DEC) + ":" + String(i[2], DEC) + ":" + String(i[3], DEC);
  return s;
}
