/*
  xdr.h - Library for XDR on Arduino
  Implements a subset of Sun XDR (eXternal Data Representation)
  Based on Python 2.7 xdrlib.py (/usr/lib/python2.7/xdrlib.py)
  See: RFC 1014

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef xdr_h
#define xdr_h

#include <Arduino.h>

// Class xdrError has not been implemented.
// Class xdrConversionError has not been implemented.

/* ================================================================================================== */

class xdrPacker {
  public:
    xdrPacker();
    void reset();
    void set_buffer(String s);
    void append_buffer(String s);
    String get_buffer();
    unsigned int get_buflen();
    void pack_uint(unsigned int x);
    void pack_int(int x);
    void pack_enum(int x);
    void pack_bool(boolean x);
    void pack_uhyper(double x);
    void pack_hyper(double x);
    void pack_float(float x);
    void pack_double(double x);
    void pack_fstring(int n, String s);
    void pack_fopaque(int n, String s);
    void pack_string(String s);
    void pack_opaque(String s);
    void pack_bytes(String s);
    // Function pack_list has not been implemented.
    // Function pack_farray has not been implemented.
    // Function pack_array has not been implemented.
  private:
    String _buf;
};

/* ================================================================================================== */

class xdrUnpacker {
  public:
    xdrUnpacker();
    void reset();
    void set_buffer(String s);
    void append_buffer(byte b);
    unsigned int get_position();
    void set_position(unsigned int n);
    String get_buffer();
    unsigned int get_buflen();
    boolean done();
    unsigned int unpack_uint();
    int unpack_int();
    int unpack_enum();
    boolean unpack_bool();
    double unpack_uhyper();
    double unpack_hyper();
    float unpack_float();
    double unpack_double();
    String unpack_fstring(int n);
    String unpack_fopaque(int n);
    String unpack_string();
    String unpack_opaque();
    String unpack_bytes();
    // Function unpack_list has not been implemented.
    // Function unpack_farray has not been implemented.
    // Functino unpack_array has not been implemented.
  private:
    String _buf;
    int _pos;
};

#endif

