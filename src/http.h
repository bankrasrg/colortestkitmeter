/*
  http.h - Webserver class

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef http_h
#define http_h

#include <Arduino.h>
#include <Ethernet.h>
#include "sdcard.h"
#include "misc.h"

// constants web server
#define HTTP_PORT 80      // port

/* ================================================================================================== */

class httpServer {
  public:
    httpServer(unsigned int port);
    unsigned int port;
    EthernetServer TCPServer;
    void begin();
    void available();
    void setWebFile(String s);
  private:
    char _webFile[13];
    String _httpRequest;
};

#endif

