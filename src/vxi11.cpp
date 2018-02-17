/*
  vxi11.cpp - Library for VXI-11 servers on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "vxi11.h"
#include "instr.h"

/* ================================================================================================== */

vxi11Packer::vxi11Packer() {
}

void vxi11Packer::pack_device_link(int link) {
  this->pack_int(link);
}

void vxi11Packer::pack_device_error(int error) {
  this->pack_int(error);
}

void vxi11Packer::pack_create_link_resp(int error, int link, unsigned int abort_port, unsigned int max_recv_size) {
  this->pack_int(error);
  this->pack_int(link);
  this->pack_uint(abort_port);
  this->pack_uint(max_recv_size);
}

void vxi11Packer::pack_device_write_resp(int error, unsigned int data_size) {
  this->pack_int(error);
  this->pack_uint(data_size);
}

void vxi11Packer::pack_device_read_resp(int error, unsigned int reason, String data) {
  this->pack_int(error);
  this->pack_uint(reason);
  this->pack_opaque(data);
}

void vxi11Packer::pack_device_readstb_resp(int error, byte stb) {
  this->pack_int(error);
  this->pack_uint(stb);  
}

void vxi11Packer::pack_device_docmd_resp(int error, String data) {
  this->pack_int(error);  
  this->pack_opaque(data);
}

/* ================================================================================================== */

vxi11Unpacker::vxi11Unpacker() {
}

void vxi11Unpacker::unpack_device_link(int *link) {
  *link = this->unpack_int();
}

void vxi11Unpacker::unpack_create_link_parms(int *id, boolean *lock_device, unsigned int *lock_timeout, String *device) {
  *id = this->unpack_int();
  *lock_device = this->unpack_bool();
  *lock_timeout = this->unpack_uint();
  *device = this->unpack_string();
}

void vxi11Unpacker::unpack_device_write_parms(int *link, unsigned int *timeout, unsigned int *lock_timeout, byte *flags, String *data) {
  *link = this->unpack_int();
  *timeout = this->unpack_uint();
  *lock_timeout = this->unpack_uint();
  *flags = this->unpack_uint();
  *data = this->unpack_opaque();
}

void vxi11Unpacker::unpack_device_read_parms(int *link, unsigned int *data_size, unsigned int *timeout, unsigned int *lock_timeout, byte *flags, char *termchar) {
  *link = this->unpack_int();
  *data_size = this->unpack_uint();
  *timeout = this->unpack_uint();
  *lock_timeout = this->unpack_uint();
  *flags = this->unpack_uint();
  *termchar = this->unpack_uint();
}

void vxi11Unpacker::unpack_device_generic_parms(int *link, byte *flags, unsigned int *timeout, unsigned int *lock_timeout) {
  *link = this->unpack_int();
  *flags = this->unpack_uint();  
  *timeout = this->unpack_uint();
  *lock_timeout = this->unpack_uint();
}

void vxi11Unpacker::unpack_device_remote_func_parms(unsigned int *host_addr, unsigned int *host_port, unsigned int *prog_num, unsigned int *prog_vers, int *prog_family) {
  *host_addr = this->unpack_uint();  
  *host_port = this->unpack_uint();  
  *prog_num = this->unpack_uint();  
  *prog_vers = this->unpack_uint();  
  *prog_family = this->unpack_int();  
}
  
void vxi11Unpacker::unpack_device_enable_srq_parms(int *link, boolean *enable, String *handle) {
  *link = this->unpack_int();
  *enable = this->unpack_bool();
  *handle = this->unpack_opaque();
}

void vxi11Unpacker::unpack_device_lock_parms(int *link, byte *flags, unsigned int *lock_timeout) {
  *link = this->unpack_int();
  *flags = this->unpack_uint();  
  *lock_timeout = this->unpack_uint();
}

void vxi11Unpacker::unpack_device_docmd_parms(int *link, byte *flags, unsigned int *timeout, unsigned int *lock_timeout, int *cmd, boolean *network_order, int *data_size, String *data_in) {
  *link = this->unpack_int();
  *flags = this->unpack_uint();  
  *timeout = this->unpack_uint();
  *lock_timeout = this->unpack_uint();  
  *cmd = this->unpack_int();
  *network_order = this->unpack_bool();
  *data_size = this->unpack_int();
  *data_in = this->unpack_opaque();
}

/* ================================================================================================== */

rpcVXI11Server::rpcVXI11Server(unsigned prog, unsigned int vers, unsigned int port): rpcTCPServer(prog, vers, port), vxi11Packer(), vxi11Unpacker() {
  this->prog = prog;
  this->vers = vers;
  this->port = port;
  this->lastxid = 0;
}

String rpcVXI11Server::get_data_buffer() {
  return this->_data_buffer;
}

void rpcVXI11Server::set_data_buffer(String s) {
  this->_data_buffer = s;
}
  
boolean rpcVXI11Server::handle_CREATE_LINK() {
  int id;
  boolean lock_device;
  unsigned int lock_timeout;
  String device;  
  this->unpack_create_link_parms(&id, &lock_device, &lock_timeout, &device);
  
  int link = 0;
  unsigned int max_recv_size = 1024;
  this->pack_create_link_resp(ERR_NO_ERROR, link, this->port, max_recv_size);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_WRITE() {
  int link;
  unsigned int timeout;
  unsigned int lock_timeout;
  byte flags;
  String data;
  this->unpack_device_write_parms(&link, &timeout, &lock_timeout, &flags, &data);
  
  this->pack_device_write_resp(ERR_NO_ERROR, data.length());
  
  if (data.equalsIgnoreCase("*IDN?")) {
    this->set_data_buffer(instrMemory::instrIDN);
  }
  if (data.equalsIgnoreCase(":CHAN1:DATA?")) {
    String dataString = String(instrMemory::colorSensorDataRed[instrMemory::colorSensorDataIndex], DEC);
    dataString = dataString + "," + String(instrMemory::colorSensorDataGreen[instrMemory::colorSensorDataIndex], DEC);
    dataString = dataString + "," + String(instrMemory::colorSensorDataBlue[instrMemory::colorSensorDataIndex], DEC);
    dataString = dataString + "," + String(instrMemory::colorSensorDataClear[instrMemory::colorSensorDataIndex], DEC);
    dataString = dataString + "," + String(instrMemory::colorSensorDataColorTemp[instrMemory::colorSensorDataIndex], DEC);
    dataString = dataString + "," + String(instrMemory::colorSensorDataLux[instrMemory::colorSensorDataIndex], DEC);
    this->set_data_buffer(dataString);
  }
  if (data.equalsIgnoreCase(":CHAN1:LED?")) {
    if (instrMemory::colorSensorLED)
      this->set_data_buffer("1");
    else
      this->set_data_buffer("0");  
  }
  if (data.equalsIgnoreCase(":CHAN1:LED ON") || data.equalsIgnoreCase(":CHAN1:LED 1")) {
    instrMemory::colorSensorLED = true;
    this->set_data_buffer("1");  
  }
  if (data.equalsIgnoreCase(":CHAN1:LED OFF") || data.equalsIgnoreCase(":CHAN1:LED 0")) {
    instrMemory::colorSensorLED = false;
    this->set_data_buffer("0");  
  }
  if (data.equalsIgnoreCase(":CHAN1:ITIM?")) {
    if (instrMemory::colorSensorIntTime == 0xFF) this->set_data_buffer("2.4");
    if (instrMemory::colorSensorIntTime == 0xF6) this->set_data_buffer("24");
    if (instrMemory::colorSensorIntTime == 0xEB) this->set_data_buffer("50");
    if (instrMemory::colorSensorIntTime == 0xD5) this->set_data_buffer("101");
    if (instrMemory::colorSensorIntTime == 0xC0) this->set_data_buffer("154");
    if (instrMemory::colorSensorIntTime == 0x00) this->set_data_buffer("700");
  }
  if (data.equalsIgnoreCase(":CHAN1:ITIM 2.4")) {
    instrMemory::colorSensorIntTime = 0xFF;
    this->set_data_buffer("2.4");
  }  
  if (data.equalsIgnoreCase(":CHAN1:ITIM 24")) {
    instrMemory::colorSensorIntTime = 0xF6;
    this->set_data_buffer("24");
  }  
  if (data.equalsIgnoreCase(":CHAN1:ITIM 50")) {
    instrMemory::colorSensorIntTime = 0xEB;
    this->set_data_buffer("50");
  }  
  if (data.equalsIgnoreCase(":CHAN1:ITIM 101")) {
    instrMemory::colorSensorIntTime = 0xD5;
    this->set_data_buffer("101");
  }  
  if (data.equalsIgnoreCase(":CHAN1:ITIM 154")) {
    instrMemory::colorSensorIntTime = 0xC0;
    this->set_data_buffer("154");
  }  
  if (data.equalsIgnoreCase(":CHAN1:ITIM 700")) {
    instrMemory::colorSensorIntTime = 0x00;
    this->set_data_buffer("700");
  }    
  if (data.equalsIgnoreCase(":CHAN1:GAIN?")) {
    if (instrMemory::colorSensorGain == 0x00) this->set_data_buffer("1");
    if (instrMemory::colorSensorGain == 0x01) this->set_data_buffer("4");
    if (instrMemory::colorSensorGain == 0x02) this->set_data_buffer("16");
    if (instrMemory::colorSensorGain == 0x03) this->set_data_buffer("60");
  }
  if (data.equalsIgnoreCase(":CHAN1:GAIN 1")) {
    instrMemory::colorSensorGain = 0x00;
    this->set_data_buffer("1");
  }  
  if (data.equalsIgnoreCase(":CHAN1:GAIN 2")) {
    instrMemory::colorSensorGain = 0x01;
    this->set_data_buffer("2");
  }
  if (data.equalsIgnoreCase(":CHAN1:GAIN 16")) {
    instrMemory::colorSensorGain = 0x02;
    this->set_data_buffer("16");
  }
  if (data.equalsIgnoreCase(":CHAN1:GAIN 60")) {
    instrMemory::colorSensorGain = 0x03;
    this->set_data_buffer("60");
  }
  if (data.equalsIgnoreCase(":LAN:IPAD?")) {
    this->set_data_buffer(ipToString(instrMemory::ip));  
  }
  if (data.equalsIgnoreCase(":LAN:SMAS?")) {
    this->set_data_buffer(ipToString(instrMemory::snip));  
  }
  if (data.equalsIgnoreCase(":LAN:GAT?")) {
    this->set_data_buffer(ipToString(instrMemory::gw));  
  }
  if (data.equalsIgnoreCase(":LAN:DNS?")) {
    this->set_data_buffer(ipToString(instrMemory::dnsip));  
  }
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_READ() {
  int link;
  unsigned int data_size;
  unsigned int timeout;
  unsigned int lock_timeout;
  byte flags;
  char term_char;
  this->unpack_device_read_parms(&link, &data_size, &timeout, &lock_timeout, &flags, &term_char);
  unsigned int reason = 4;
  this->pack_device_read_resp(ERR_NO_ERROR, reason, this->get_data_buffer());  
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_READSTB() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  unsigned int timeout;
  this->unpack_device_generic_parms(&link, &flags, &lock_timeout, &timeout);
  byte stb = 0x04; // arbitrary value for test
  this->pack_device_readstb_resp(ERR_NO_ERROR, stb);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_TRIGGER() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  unsigned int timeout;
  this->unpack_device_generic_parms(&link, &flags, &lock_timeout, &timeout);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_CLEAR() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  unsigned int timeout;
  this->unpack_device_generic_parms(&link, &flags, &lock_timeout, &timeout);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_REMOTE() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  unsigned int timeout;
  this->unpack_device_generic_parms(&link, &flags, &lock_timeout, &timeout);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_LOCAL() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  unsigned int timeout;
  this->unpack_device_generic_parms(&link, &flags, &lock_timeout, &timeout);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_LOCK() {
  int link;
  byte flags;
  unsigned int lock_timeout;
  this->unpack_device_lock_parms(&link, &flags, &lock_timeout);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_UNLOCK() {
  int link;
  this->unpack_device_link(&link);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_ENABLE_SRQ() {
  int link;
  boolean enable;
  String handle;
  this->unpack_device_enable_srq_parms(&link, &enable, &handle);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_DOCMD() {
  int link;
  byte flags;
  unsigned int timeout;
  unsigned int lock_timeout;
  int cmd;
  boolean network_order;
  int data_size;
  String data_in;  
  this->unpack_device_docmd_parms(&link, &flags, &timeout, &lock_timeout, &cmd, &network_order, &data_size, &data_in);

  this->pack_device_docmd_resp(ERR_NO_ERROR, this->get_data_buffer());
  return true;
}

boolean rpcVXI11Server::handle_DESTROY_LINK() {
  int link;
  this->unpack_device_link(&link);

  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_CREATE_INTR_CHAN() {
  int link;
  byte flags;
  unsigned int timeout;
  unsigned int lock_timeout;
  int cmd;
  boolean network_order;
  int data_size;
  String data_in;  
  this->unpack_device_docmd_parms(&link, &flags, &timeout, &lock_timeout, &cmd, &network_order, &data_size, &data_in);
  
  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DESTROY_INTR_CHAN() {
  // nothing to unpack
  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_ABORT() {
  // unpack ...
  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

boolean rpcVXI11Server::handle_DEVICE_INTR_SRQ() {
  // unpack ...
  this->pack_device_error(ERR_NO_ERROR);
  return true;
}

