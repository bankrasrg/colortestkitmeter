/*
  portmap.cpp - Library for port mapper on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "portmap.h"

/* ================================================================================================== */

portmapPacker::portmapPacker() : rpcPacker() {
}

void portmapPacker::pack_mapping(sPortmap &pmap) {
  this->pack_uint(pmap.prog);
  this->pack_uint(pmap.vers);
  this->pack_uint(pmap.prot);
  this->pack_uint(pmap.port);
}

void portmapPacker::pack_call_args(sCallArgs &cargs) {
  this->pack_uint(cargs.prog);
  this->pack_uint(cargs.vers);
  this->pack_uint(cargs.proc);
  this->pack_opaque(cargs.args);
}

/* ================================================================================================== */

portmapUnpacker::portmapUnpacker() : rpcUnpacker() {
}

sPortmap portmapUnpacker::unpack_mapping() {
  sPortmap pmap;
  pmap.prog = this->unpack_uint();
  pmap.vers = this->unpack_uint();
  pmap.prot = this->unpack_uint();
  pmap.port = this->unpack_uint();
  return pmap;
}

sCallResult portmapUnpacker::unpack_call_result() {
  sCallResult cres;  
  cres.port = this->unpack_uint();
  cres.res = this->unpack_opaque();
  return cres;
}

/* ================================================================================================== */

rpcPortmapServer::rpcPortmapServer(unsigned prog, unsigned int vers, unsigned int port): TCPServer(port), UDPServer() {
  this->prog = prog;
  this->vers = vers;
  this->port = port;
  this->lastxid = 0;
  for (int i = 0; i < PMAPLIST_SIZE; i++)
    this->_pmaplist[i] = {0, 0, 0, 0};
}

void rpcPortmapServer::begin() {
  this->TCPServer.begin();
  this->UDPServer.begin(port);
}

void rpcPortmapServer::available() {
  EthernetClient client = this->TCPServer.available();

  if (client) {
    Serial.println("rpcPortmapServer: TCP client connected");
    this->portmapUnpack.reset();
    
    while (client.connected()) {
      if (client.available())
        this->portmapUnpack.append_buffer(client.read());
      else
        break;
    }
  
    Serial.println("rpcPortmapServer::available received data");
    String s = this->portmapUnpack.get_buffer();
    for (int i = 0; i < s.length(); i++) {
      Serial.print(s.charAt(i));
      if (i % 2 > 0)
        Serial.print(" ");
    }
    Serial.println("");  

    this->portmapPack.reset();

    unsigned int xid;
    unsigned int prog;
    unsigned int vers;
    unsigned int proc;
    sAuth cred;
    sAuth verf;
    
    this->portmapUnpack.unpack_callheader(&xid, &prog, &vers, &proc, &cred, &verf);   

    if (prog != this->prog) {
      this->reply_PROG_UNAVAIL();
      return;
    }
    if (vers != this->vers) {
      this->reply_PROG_MISMATCH();
      return;
    }
    
    verf.flavor = AUTH_NULL;   
    verf.stuff = this->portmapPack.make_auth(AUTH_NULL);
    this->portmapPack.pack_replyheader(xid, verf);

    boolean res = false;
    
    switch (proc) {
      case PMAPPROC_NULL: 
        res = this->handle_PMAPPROC_NULL();
        break;
      case PMAPPROC_SET:
        res = this->handle_PMAPPROC_SET();
        break;
      case PMAPPROC_UNSET:
        res = this->handle_PMAPPROC_UNSET();
        break;
      case PMAPPROC_GETPORT:
        res = this->handle_PMAPPROC_GETPORT();
        break;
      case PMAPPROC_DUMP:
        res = this->handle_PMAPPROC_DUMP();
        break;
      case PMAPPROC_CALLIT:
        res = this->handle_PMAPPROC_CALLIT();
        break;
      default:
        this->portmapPack.pack_uint(PROC_UNAVAIL);
    }
        
    if (!res) {     
      reply_GARBAGE_ARGS(xid);
      return;
    }

    this->portmapPack.pack_recordheader();
    
    // Move the following to a more generic function!?
    s = this->portmapPack.get_buffer();
    char c[50]; // 50 bytes data
    for (int i = 0; i < s.length(); i += 2) {
      byte b1 = s.charAt(i);
      byte b2 = s.charAt(i + 1);   
      if (b1 > 0x39)
        b1 -= 7;
      if (b2 > 0x39)
        b2 -= 7;
      c[i/2] = (((b1 & 0x0F) << 4) | (b2 & 0x0F));   
    }
    unsigned int n = s.length() / 2;
    client.write(c, n);
    
    client.stop();
    Serial.println("rpcPortmapServer: TCP client disconnected");
  }
}

String rpcPortmapServer::registerServer(String s) {
  Serial.println("rpcPortmapServer::registerServer received data");
  for (int i = 0; i < s.length(); i++) {
    Serial.print(s.charAt(i));
    if (i % 2 > 0)
      Serial.print(" ");
  }
  Serial.println("");
    
  this->portmapUnpack.set_buffer(s);
  this->portmapPack.reset();
     
  unsigned int xid = this->portmapUnpack.unpack_uint();
  this->portmapPack.pack_uint(xid);
    
  int mtype = this->portmapUnpack.unpack_enum();
  if (mtype != CALL) {     
    return ""; // Do not reply if not called.
  }
  else {
    this->portmapPack.pack_uint(REPLY);
    if (this->portmapUnpack.unpack_uint() != RPCVERSION) {
      this->portmapPack.pack_uint(MSG_DENIED);
      this->portmapPack.pack_uint(RPC_MISMATCH);
      this->portmapPack.pack_uint(RPCVERSION);
      this->portmapPack.pack_uint(RPCVERSION); // twice?
    }
    else {
      this->portmapPack.pack_uint(MSG_ACCEPTED);
      this->portmapPack.pack_auth(AUTH_NULL, this->portmapPack.make_auth(AUTH_NULL));
      unsigned int prog = this->portmapUnpack.unpack_uint();
      Serial.print("rpcPortmapServer::registerServer prog = ");
      Serial.println(prog);
      if (prog != this->prog) {
        Serial.print("rpcPortmapServer::registerServer expected prog = ");
        Serial.println(this->prog);
        this->portmapPack.pack_uint(PROG_UNAVAIL);
      }
      else {
        vers = this->portmapUnpack.unpack_uint();
        Serial.print("rpcPortmapServer::registerServer vers = ");
        Serial.println(vers);
        if (vers != this->vers) {
          Serial.print("rpcPortmapServer::registerServer expected vers = ");
          Serial.println(this->vers);
          this->portmapPack.pack_uint(PROG_MISMATCH);
          this->portmapPack.pack_uint(this->vers);
          this->portmapPack.pack_uint(this->vers); // twice?
        }
        else {
          unsigned int proc = this->portmapUnpack.unpack_uint();
          Serial.print("rpcPortmapServer::registerServer proc = ");
          Serial.println(proc);
          sAuth cred = this->portmapUnpack.unpack_auth();
          sAuth verf = this->portmapUnpack.unpack_auth();
          boolean result = false;
            
          // Only accept PMAPPROC_SET            
          switch (proc) {
            case PMAPPROC_SET:
              result = this->handle_PMAPPROC_SET();
              break;
            default:
              this->portmapPack.pack_uint(PROC_UNAVAIL);
          }
        
          if (!result) {
            // In case handle_X fails due to too few or too many arguments, return the following instead:
            Serial.println("rpcPortmapServer::registerServer procedure failed!");
            this->portmapPack.reset();
            this->portmapPack.pack_uint(xid);
            this->portmapPack.pack_uint(REPLY);
            this->portmapPack.pack_uint(MSG_ACCEPTED);
            this->portmapPack.pack_auth(AUTH_NULL, this->portmapPack.make_auth(AUTH_NULL));
            this->portmapPack.pack_uint(GARBAGE_ARGS);
          }
        }
      }
    }
  }
  return this->portmapPack.get_buffer();
}
  
boolean rpcPortmapServer::handle_PMAPPROC_NULL() {
  return true;
}

boolean rpcPortmapServer::handle_PMAPPROC_SET() {
  sPortmap p = this->portmapUnpack.unpack_mapping();
  if (!this->portmapUnpack.done()) {
    Serial.println("rpcPortmapServer::handle_PMAPPROC_SET: data remaining in buffer!");
    return false;
  }
  int n = -1;
  for (int i = 0; i < PMAPLIST_SIZE; i++) {
    if (this->_pmaplist[i].port == p.port) {
      // If the port is not bound (in use) it should be unregistered before accepting the new mapping. How to check if the port is bound?
      this->portmapPack.pack_uint(PROG_UNAVAIL); // What error code to pack?
      Serial.println("rpcPortmapServer::handle_PMAPPROC_SET: port already registered!");
      return true;
    }
    if ((this->_pmaplist[i].port == 0) && (n < 0))
      n = i;
  }
  if (n < 0) {
    this->portmapPack.pack_uint(PROG_UNAVAIL); // What error code to pack?
    Serial.println("rpcPortmapServer::handle_PMAPPROC_SET: pmaplist is full!");
    return true;
  }
  this->_pmaplist[n] = p;
  this->portmapPack.pack_uint(SUCCESS);  
  return true;  
}

boolean rpcPortmapServer::handle_PMAPPROC_UNSET() {
  return true;
}

boolean rpcPortmapServer::handle_PMAPPROC_GETPORT() {
  sPortmap p = this->portmapUnpack.unpack_mapping();
  if (!this->portmapUnpack.done()) {
    Serial.println("rpcPortmapServer::handle_PMAPPROC_SET: data remaining in buffer!");
    return false;
  }
  for (int i = 0; i < PMAPLIST_SIZE; i++) {
    if ((this->_pmaplist[i].prog == p.prog) && (this->_pmaplist[i].vers == p.vers) && (this->_pmaplist[i].prot == p.prot)) {
      this->portmapPack.pack_uint(this->_pmaplist[i].port); // What error code to pack?
      Serial.println("rpcPortmapServer::handle_PMAPPROC_GETPORT: port found");
      return true;
    }
  }
  return false;
}

boolean rpcPortmapServer::handle_PMAPPROC_DUMP() {
  return true;
}

boolean rpcPortmapServer::handle_PMAPPROC_CALLIT() {
  return true;
}

void rpcPortmapServer::reply_PROG_UNAVAIL() {
  this->portmapPack.pack_uint(PROG_UNAVAIL);
}

void rpcPortmapServer::reply_PROG_MISMATCH() {
  this->portmapPack.pack_uint(PROG_MISMATCH);
  this->portmapPack.pack_uint(this->vers);
  this->portmapPack.pack_uint(this->vers);
}

void rpcPortmapServer::reply_GARBAGE_ARGS(unsigned int xid) {
  this->portmapPack.reset();
  this->portmapPack.pack_uint(xid);
  this->portmapPack.pack_uint(REPLY);
  this->portmapPack.pack_uint(MSG_ACCEPTED);
  this->portmapPack.pack_auth(AUTH_NULL, this->portmapPack.make_auth(AUTH_NULL));
  this->portmapPack.pack_uint(GARBAGE_ARGS);
}
