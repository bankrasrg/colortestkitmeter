/*
  http.cpp - Webserver class

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "http.h"
#include "instr.h"

/* ================================================================================================== */

httpServer::httpServer(unsigned int port): TCPServer(port) {
  this->port = port;
}

void httpServer::begin() {
  this->TCPServer.begin();
  if (!sdCard::root.exists("INDEX.HTM")) {
    Serial.println("httpServer::begin failed to find file INDEX.HTM.");
  }
}

void httpServer::available() {
  EthernetClient client = this->TCPServer.available();
  if (client) {
    Serial.println("httpServer::available webclient connected");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // Store the received character in the buffer
        this->_httpRequest += c;
        if (c == '\n' && currentLineIsBlank) {
          Serial.print("httpServer::available _httpRequest: ");
          this->_httpRequest.toLowerCase();
          if ((this->_httpRequest.indexOf("get / ") >= 0) || (this->_httpRequest.indexOf("get /index.htm") >= 0)) {
            Serial.println("httpServer::available GET / or /index.htm");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();            
            this->setWebFile("INDEX.HTM");
          }
          else if (this->_httpRequest.indexOf("get /color.jpg") >= 0) {
            Serial.println("httpServer::available GET /color.jpg");
            this->setWebFile("COLOR.JPG");
            if (sdCard::root.exists(this->_webFile)) { 
              client.println("HTTP/1.1 200 OK");
              client.println();
            }      
          }
          else if (this->_httpRequest.indexOf("get /buffer.htm") >= 0) {
            Serial.println("httpServer::available GET /buffer.htm");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><title>WZ1295 - Instrument of Things</title></head>");
            client.println("<body>");
            client.println(instrMemory::colorSensorDataIndex);
            client.println("<br>");
            for (int i = 0; i < MEM_DEPTH; i++) {
              client.print(i);
              client.print(",");
              client.print(instrMemory::colorSensorDataRed[i]);
              client.print(",");
              client.print(instrMemory::colorSensorDataGreen[i]);
              client.print(",");
              client.print(instrMemory::colorSensorDataBlue[i]);
              client.print(",");
              client.print(instrMemory::colorSensorDataClear[i]);
              client.print(",");
              client.print(instrMemory::colorSensorDataColorTemp[i]);
              client.print(",");
              client.print(instrMemory::colorSensorDataLux[i]);
              client.println("<br>");
            }
            client.println("</body>");
            client.println("</html>");
            this->setWebFile("");
          }
          if (sdCard::root.exists(this->_webFile)) {
            Serial.println("httpServer::available _webFile exists");
            if (!sdCard::file.open(&sdCard::root, this->_webFile, O_READ)) {              
              Serial.print("httpServer::available failed to open file ");
              Serial.println(this->_webFile);
            }
            while (sdCard::file.available() > 0) {
              byte data = sdCard::file.read();
              client.write(data);
            }
            sdCard::file.close();
          }
          if ((this->_httpRequest.indexOf("get / ") >= 0) || (this->_httpRequest.indexOf("get /index.htm") >= 0)) {
            client.println("<script>");
            client.print("var idn = \"");
            client.print(instrMemory::instrIDN);
            client.println("\";");
            client.print("var ip = \"");
            client.print(ipToString(instrMemory::ip));
            client.println("\";");
            client.print("var snip = \"");
            client.print(ipToString(instrMemory::snip));
            client.println("\";");
            client.print("var gw = \"");
            client.print(ipToString(instrMemory::gw));
            client.println("\";");
            client.print("var dnsip = \"");
            client.print(ipToString(instrMemory::dnsip));
            client.println("\";");
            client.print("var r = \"");
            client.print(instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var g = \"");
            client.print(instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var b = \"");
            client.print(instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var c = \"");
            client.print(instrMemory::colorSensorDataClear[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var tc = \"");
            client.print(instrMemory::colorSensorDataColorTemp[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var i = \"");
            client.print(instrMemory::colorSensorDataLux[instrMemory::colorSensorDataIndex]);
            client.println("\";");
            client.print("var IntTime = \"");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0xFF) client.print("2.4");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0xF6) client.print("24");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0xEB) client.print("50");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0xD5) client.print("101");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0xC0) client.print("154");
            if (instrMemory::colorSensorDataIntTime[instrMemory::colorSensorDataIndex] == 0x00) client.print("700");
            client.println("\";");
            client.print("var Gain = \"");
            if (instrMemory::colorSensorDataGain[instrMemory::colorSensorDataIndex] == 0x00) client.print("1");
            if (instrMemory::colorSensorDataGain[instrMemory::colorSensorDataIndex] == 0x01) client.print("2");
            if (instrMemory::colorSensorDataGain[instrMemory::colorSensorDataIndex] == 0x02) client.print("16");
            if (instrMemory::colorSensorDataGain[instrMemory::colorSensorDataIndex] == 0x03) client.print("60");
            client.println("\";");
            
            client.println("document.getElementById(\"table_idn\").innerHTML = idn;");
            client.println("document.getElementById(\"table_ip\").innerHTML = ip;");
            client.println("document.getElementById(\"table_snip\").innerHTML = snip;");
            client.println("document.getElementById(\"table_gw\").innerHTML = gw");
            client.println("document.getElementById(\"table_dnsip\").innerHTML = dnsip;");            
            client.println("document.getElementById(\"table_r\").innerHTML = r;");
            client.println("document.getElementById(\"table_g\").innerHTML = g;");
            client.println("document.getElementById(\"table_b\").innerHTML = b;");
            client.println("document.getElementById(\"table_c\").innerHTML = c;");
            client.println("document.getElementById(\"table_tc\").innerHTML = tc;");
            client.println("document.getElementById(\"table_i\").innerHTML = i;");
            client.println("document.getElementById(\"table_IntTime\").innerHTML = IntTime;");
            client.println("document.getElementById(\"table_Gain\").innerHTML = Gain;");
            client.println("</script>");
            client.println("</body>");
            client.println("</html>");          
          }
          this->_httpRequest = "";
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    // Give the web client time to receive the data
    delay(1);
    client.stop();
    Serial.println("httpServer::available webclient disconnected"); 
  }
}

void httpServer::setWebFile(String s) {
  s.toCharArray(this->_webFile, 12);
}
