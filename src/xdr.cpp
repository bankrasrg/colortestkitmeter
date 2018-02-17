/*
  xdr.cpp - Library for XDR on Arduino
  Implements a subset of Sun XDR (eXternal Data Representation)
  Based on Python 2.7 xdrlib.py (/usr/lib/python2.7/xdrlib.py)
  See: RFC 1014

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "xdr.h"

/* ================================================================================================== */

xdrPacker::xdrPacker() {
  reset();
}

void xdrPacker::reset() {
  // Reset the buffer
  this->_buf = "";
}

void xdrPacker::set_buffer(String s) {
  // Replace the buffer by string s.
  this->_buf = s;
}

void xdrPacker::append_buffer(String s) {
  // Append string s to the string in the buffer.
  this->_buf += s;
}

String xdrPacker::get_buffer() {
  return this->_buf;
}

unsigned int xdrPacker::get_buflen() {
  // Get the size of the string in the buffer (in characters).
  return this->_buf.length();
}

void xdrPacker::pack_uint(unsigned int x) {
  // Pack unsigned int value as string containing value in unsigned long format (4 bytes)
  // Int variables store 2 bytes on Arduino Uno and 4 bytes on Arduino Due
  // Long variables store 4 bytes on Arduino
  unsigned long l = (unsigned long) x;
  String v = String(l, HEX);
  int n = v.length();
  if (n < 8) {
    for (int i = 0; i < 8 - n; i++) {
      v = "0" + v;
    }
  }
  if (n > 8)
    v = v.substring(n - 8, n);
  v.toUpperCase();
  this->_buf += v;
}

void xdrPacker::pack_int(int x) {
  // Pack int value as string containing value in long format (4 bytes)
  // Int variables store 2 bytes on Arduino Uno and 4 bytes on Arduino Due
  // Long variables store 4 bytes on Arduino
  long l = (long) x;
  String v = String(l, HEX);
  int n = v.length();
  if (n < 8) {
    for (int i = 0; i < 8 - n; i++) {
      v = "0" + v;
    }
  }
  if (n > 8)
    v = v.substring(n - 8, n);
  v.toUpperCase();
  this->_buf += v;  
}

void xdrPacker::pack_enum(int x) {
  pack_int(x);
}

void xdrPacker::pack_bool(boolean x) {
  if (x) {
    this->_buf += "0001";
  }
  else {
    this->_buf += "0000";
  }  
}

void xdrPacker::pack_uhyper(double x) {
  // Double variables store 4 bytes on Arduino Uno and 8 bytes on Arduino Due
  // Hyper variables store 8 bytes
}

void xdrPacker::pack_hyper(double x) {
  pack_uhyper(x);
}

void xdrPacker::pack_float(float x) {
  // Float variables store 4 bytes
}

void xdrPacker::pack_double(double x) {
  // Double variables store 4 bytes on Arduino Uno and 8 bytes on Arduino Due
}

void xdrPacker::pack_fstring(int n, String s) {
  if (n < 0) {
    // Raise error, but how?
  }
  int z = ((n + 3) / 4) * 4;

  for (int i = 0; i < s.length(); i ++) {   
    byte b = s.charAt(i);
    String v = String(b, HEX);
    v.toUpperCase();
    this->_buf += v;
  }

  for (int i = 0; i < (z - n); i++) {
    this->_buf += "00";
  }
}

void xdrPacker::pack_fopaque(int n, String s) {
  pack_fstring(n, s);
}

void xdrPacker::pack_string(String s) {
  // Pack string
  unsigned int n = s.length();
  pack_uint(n);
  pack_fstring(n, s);  
}

void xdrPacker::pack_opaque(String s) {
  pack_string(s);  
}

void xdrPacker::pack_bytes(String s) {
  pack_string(s);  
}

/* ================================================================================================== */

xdrUnpacker::xdrUnpacker() {
  reset();
}

void xdrUnpacker::reset() {
  // Reset the buffer
  set_buffer("");
}

void xdrUnpacker::set_buffer(String s) {
  // Replace the buffer by string d and reset the position counter.
  this->_buf = s;
  this->_pos = 0;
}

void xdrUnpacker::append_buffer(byte b) {
  // Append a byte b to the string in the buffer. The position counter is not reset.
  char c = ((b & 0xF0) >> 4) + 0x30;
  if (c > 0x39)
    c += 7;
  this->_buf += c;
  c = (b & 0x0F) + 0x30;
  if (c > 0x39)
    c += 7;
  this->_buf += c;
}

unsigned int xdrUnpacker::get_position() {
  // Return the position counter for the buffer.
  return this->_pos; 
}

void xdrUnpacker::set_position(unsigned int n) {
  // Set the position counter for the buffer to value n.
  this->_pos = n;
}

String xdrUnpacker::get_buffer() {
  // Get the string from the buffer.
  return this->_buf;
}

unsigned int xdrUnpacker::get_buflen() {
  // Get the size of the string in the buffer (in characters).
  return this->_buf.length();
}

boolean xdrUnpacker::done() {
  // Check if the position counter has reached the end of the string in the buffer.
  return !(this->_pos < this->_buf.length());
}

unsigned int xdrUnpacker::unpack_uint() {
  String d = (this->_buf).substring(this->_pos, this->_pos + 8);
  this->_pos += 8;
  if (d.length() < 8) {
    // Raise error, but how?
  }
  unsigned int x = 0;
  int v = 0;
  for (int i = 0; i < 8; i++) {
    char c = d.charAt(i);
    if (c >= '0' && c <= '9')
      v = (c - '0');
    else
      v = (c - 'A' + 10);
    x *= 16;
    x += v;
  }
  return x;
}

int xdrUnpacker::unpack_int() {
  String d = (this->_buf).substring(this->_pos, this->_pos + 8);
  this->_pos += 8;
  if (d.length() < 8) {
    // Raise error, but how?
  }
  int x = 0;
  int v = 0;
  for (int i = 0; i < 8; i++) {
    char c = d.charAt(i);
    if (c >= '0' && c <= '9')
      v = (c - '0');
    else
      v = (c - 'A' + 10);
    x *= 16;
    x += v;
  }
  if (d.charAt(0) < '8') {
    return x;
  }
  else {
    return -((x ^ 0xFFFF) + 1);
  }
}

int xdrUnpacker::unpack_enum() {
  return unpack_int();
}

boolean xdrUnpacker::unpack_bool() {
  return (boolean)unpack_uint();
}

double xdrUnpacker::unpack_uhyper() {
  unsigned int h = unpack_uint();
  unsigned int l = unpack_uint();
  //return (((double)h << 32) | l);
  return 0;
}

double xdrUnpacker::unpack_hyper() {
  double x = unpack_uhyper();
  if (x >= 0x8000000000000000L)
    x -= 0x10000000000000000L;
  return x;
}

float xdrUnpacker::unpack_float() {

}

double xdrUnpacker::unpack_double() {

}

String xdrUnpacker::unpack_fstring(int n) { 
  if (n < 0) {
    Serial.println("xdrUnpacker::unpack_fstring error.");
    // Raise error, but how?
  }
  int i = this->_pos;

  int j = i + ((n + 3) / 4) * 8;
  if (j > (this->_buf).length()) {
    Serial.println("xdrUnpacker::unpack_fstring error.");
    // Raise error, but how?
  }
  this->_pos = j;

  String s;
  for (int x = 0; x < 2 * n; x += 2) {
    byte b1 = (this->_buf).charAt(i + x);
    byte b2 = (this->_buf).charAt(i + x + 1);
    if (b1 > 0x39)
      b1 -= 7;
    if (b2 > 0x39)
      b2 -= 7;
    char c = ((b1 & 0x0F) << 4) ^ (b2 & 0x0F);
    s += c;  
  }
   
  return s;
}

String xdrUnpacker::unpack_fopaque(int n) {
  return unpack_fstring(n);
}  

String xdrUnpacker::unpack_string() {
  unsigned int n = unpack_uint();
  return unpack_fstring(n);
}

String xdrUnpacker::unpack_opaque() {
  return unpack_string();
}

String xdrUnpacker::unpack_bytes() {
  return unpack_string();
}

