/*
  rpc.cpp - Library for VXI-11 on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#include "rpc.h"

/* ================================================================================================== */

rpcPacker::rpcPacker() : xdrPacker() {
}

String rpcPacker::make_auth(int flavor) {
  String s;
  switch (flavor) {
    case AUTH_NULL: 
      s = "";
      break;
    case AUTH_UNIX:
      s = "";
      break;
    case AUTH_SHORT:
      s = "";
      break;
    case AUTH_DES:
      s = "";
      break;
    default:
      s = "";
  }
  return s;
}

void rpcPacker::pack_auth(sAuth &a) {
  this->pack_enum(a.flavor);
  this->pack_opaque(a.stuff);  
}

void rpcPacker::pack_auth(int flavor, String stuff) {
  this->pack_enum(flavor);
  this->pack_opaque(stuff);  
}

void rpcPacker::pack_recordheader() {
  unsigned int buflen = this->get_buflen();
  if ((buflen % 2) > 0)
    Serial.println("rpcPacker::pack_recordheader Odd number of characters in buffer!");
  if (buflen < 8)
    Serial.println("rpcPacker::pack_recordheader No fragment data in buffer!");
  unsigned int fraglen = buflen / 2;
  unsigned int rmheader = (fraglen ^ 0x80000000);
  String s = this->get_buffer();
  this->reset();  
  this->pack_uint(rmheader);
  this->append_buffer(s);
}

void rpcPacker::pack_callheader(unsigned int xid, unsigned int prog, unsigned int vers, unsigned int proc, sAuth cred, sAuth verf) {
  this->pack_uint(xid);
  this->pack_enum(CALL);
  this->pack_uint(RPCVERSION);
  this->pack_uint(prog);
  this->pack_uint(vers);
  this->pack_uint(proc);
  this->pack_auth(cred);
  this->pack_auth(verf);
}

void rpcPacker::pack_replyheader(unsigned int xid, sAuth verf) {
  this->pack_uint(xid);
  this->pack_enum(REPLY);
  this->pack_uint(MSG_ACCEPTED);
  this->pack_auth(verf);
  this->pack_enum(SUCCESS);
}

void rpcPacker::pack_mapping(sPortmap2 &pmap) {
  this->pack_uint(pmap.prog);
  this->pack_uint(pmap.vers);
  this->pack_uint(pmap.prot);
  this->pack_uint(pmap.port);
}

/* ================================================================================================== */

rpcUnpacker::rpcUnpacker() : xdrUnpacker() {
}

sAuth rpcUnpacker::unpack_auth() {
  sAuth a;
  a.flavor = this->unpack_enum();
  a.stuff = this->unpack_opaque();
  return a;
}

void rpcUnpacker::unpack_recordheader() {
  unsigned int rmheader = this->unpack_uint();
  boolean lastfrag = (rmheader && 0x80000000);
  if (!lastfrag)
    Serial.println("rpcUnpacker::unpack_recordheader Last fragment bit not set in record marking header!");
  unsigned int fraglen = (rmheader & 0x7FFFFFFF);
  unsigned int buflen = this->get_buflen();
  if (((fraglen + 4) * 2) != buflen)
    Serial.print("rpcUnpacker::unpack_recordheader Mismatch fragment size in record marking header with received data size!"); 
}

void rpcUnpacker::unpack_callheader(unsigned int *xid, unsigned int *prog, unsigned int *vers, unsigned int *proc, sAuth *cred, sAuth *verf) {
  this->unpack_recordheader();
 
  *xid = this->unpack_uint();
  Serial.print("rpcUnpacker::unpack_callheader xid = ");
  Serial.println(*xid);

  int mtype = this->unpack_enum();
  Serial.print("rpcUnpacker::unpack_callheader mtype = ");
  Serial.println(mtype);
  if (mtype != CALL)
    Serial.println("rpcUnpacker::unpack_callheader Message is not of type CALL!");

  unsigned int rpcvers = this->unpack_uint();
  Serial.print("rpcUnpacker::unpack_callheader rpcvers = ");
  Serial.println(rpcvers);
  if (rpcvers != RPCVERSION)
    Serial.println("rpcUnpacker::unpack_callheader Message is of bad RPC version!");
    
  *prog = this->unpack_uint();
  *vers = this->unpack_uint();
  *proc = this->unpack_uint();
  *cred = this->unpack_auth();
  *verf = this->unpack_auth();
  
  Serial.print("rpcUnpacker::unpack_callheader prog = ");
  Serial.println(*prog);
  Serial.print("rpcUnpacker::unpack_callheader vers = ");
  Serial.println(*vers);
  Serial.print("rpcUnpacker::unpack_callheader proc = ");
  Serial.println(*proc);
  
}

void rpcUnpacker::unpack_replyheader(unsigned int *xid, sAuth *verf) {
  this->unpack_recordheader();
  
  *xid = this->unpack_uint();
  // NOT FINISHED YET ...
  *verf = this->unpack_auth();
}

/* ================================================================================================== */

rpcServer::rpcServer(unsigned int prog, unsigned int vers, unsigned int prot, unsigned int port) {
  this->prog = prog;
  this->vers = vers;
  this->prot = prot;
  this->port = port;
  this->lastxid = 0;
}

String rpcServer::intentions() {
  sPortmap2 pmaplist;
  pmaplist.prog = this->prog;
  pmaplist.vers = this->vers;
  pmaplist.prot = this->prot;
  pmaplist.port = this->port;
  sAuth cred;
  sAuth verf;

  unsigned int xid = this->lastxid + 1;
  this->lastxid = xid;
  cred.flavor = 0; // mkcred()
  cred.stuff = "";  
  verf.flavor = 0; // mkverf()
  verf.stuff = "";
  
  this->rpcPacker::reset();
  this->pack_callheader(xid, PMAP_PROG, PMAP_VERS, PMAPPROC_SET, cred, verf);
  this->pack_mapping(pmaplist);

  String s = this->rpcPacker::get_buffer();

  Serial.println("rpcServer::intentions sent data");
  for (int i = 0; i < s.length(); i++) {
    Serial.print(s.charAt(i));
    if (i % 2 > 0)
      Serial.print(" ");
  }
  Serial.println("");

  return s;
}

boolean rpcServer::registered(String s) {  
  Serial.println("rpcServer::registered received data");
  for (int i = 0; i < s.length(); i++) {
    Serial.print(s.charAt(i));
    if (i % 2 > 0)
      Serial.print(" ");
  }
  Serial.println("");
  
  this->rpcUnpacker::set_buffer(s);
  
  unsigned int xid = this->unpack_uint();
  if (xid != this->lastxid) {
    Serial.print("rpcServer::registered Wrong xid in reply!");
  }

  int mtype = this->unpack_enum();
  if (mtype != REPLY) {
    Serial.print("rpcServer::registered Message type is not REPLY!");
  }

  int stat = this->unpack_enum();
  if (stat == MSG_DENIED) {
    stat = this->unpack_enum();
    if (stat == RPC_MISMATCH) {
      unsigned int low = this->unpack_uint();
      unsigned int high = this->unpack_uint();
      Serial.print("MSG_DENIED: RPC_MISMATCH: ");
      Serial.print(low);
      Serial.print(", ");
      Serial.println(high);
    }
    if (stat == AUTH_ERROR) {
      stat = this->unpack_uint();
      Serial.print("MSG_DENIED: AUTH_ERROR: ");
      Serial.println(stat);
    }
    Serial.print("MSG_DENIED: ");
    Serial.println(stat);
  }
  else if (stat != MSG_ACCEPTED) {
    Serial.print("rpcServer::registered Message status is neither MSG_DENIED nor MSG_ACCEPTED!");
  }
  
  sAuth verf = this->unpack_auth();

  stat = this->unpack_uint();
  Serial.print("rpcServer::registered stat = ");
  Serial.println(stat);
  boolean result = false;
  switch (stat) {
    case PROG_UNAVAIL:
      Serial.println("Call failed: PROG_UNAVAIL");
      break;
    case PROG_MISMATCH:
      Serial.print("Call failed: PROG_MISMATCH: ");
      Serial.print(this->unpack_uint());
      Serial.print(", ");
      Serial.println(this->unpack_uint());
      break;
    case PROC_UNAVAIL:
      Serial.println("Call failed: PROC_UNAVAIL");
      break;
    case GARBAGE_ARGS:
      Serial.println("GARBAGE_ARGS");
      break;
    case SUCCESS:
      result = true;
      break;
    default:
      Serial.print("Call failed: ");
      Serial.println(stat);
      break;
  }
  return result;
}

void rpcServer::reply_PROG_UNAVAIL() {
  this->pack_uint(PROG_UNAVAIL);
}

void rpcServer::reply_PROG_MISMATCH() {
  this->pack_uint(PROG_MISMATCH);
  this->pack_uint(this->vers);
  this->pack_uint(this->vers);
}

void rpcServer::reply_GARBAGE_ARGS(unsigned int xid) {
  this->rpcPacker::reset();
  this->pack_uint(xid);
  this->pack_uint(REPLY);
  this->pack_uint(MSG_ACCEPTED);
  this->pack_auth(AUTH_NULL, this->make_auth(AUTH_NULL));
  this->pack_uint(GARBAGE_ARGS);
}

/* ================================================================================================== */

rpcTCPServer::rpcTCPServer(unsigned int prog, unsigned int vers, unsigned int port): rpcServer(prog, vers, IPPROTO_TCP, port), TCPServer(port) {
}

void rpcTCPServer::begin(String s) {
  if (this->registered(s))
    this->TCPServer.begin();
  else
    Serial.print("rpcTCPServer registration failed!");
}

void rpcTCPServer::available() {
  EthernetClient client = this->TCPServer.available();

  if (client) {
    Serial.println("rpcTCPServer: TCP client connected");
    this->rpcUnpacker::reset();
    
    while (client.connected()) {
      if (client.available())
        this->rpcUnpacker::append_buffer(client.read());
      else
        break;
    }
  
    Serial.println("rpcTCPServer::available received data");
    String s = this->rpcUnpacker::get_buffer();
    for (int i = 0; i < s.length(); i++) {
      Serial.print(s.charAt(i));
      if (i % 2 > 0)
        Serial.print(" ");
    }
    Serial.println("");  

    this->rpcPacker::reset();

    unsigned int xid;
    unsigned int prog;
    unsigned int vers;
    unsigned int proc;
    sAuth cred;
    sAuth verf;
    
    this->unpack_callheader(&xid, &prog, &vers, &proc, &cred, &verf);   

    if (prog != this->prog) {
      this->reply_PROG_UNAVAIL();
      return;
    }
    if (vers != this->vers) {
      this->reply_PROG_MISMATCH();
      return;
    }
    
    verf.flavor = AUTH_NULL;   
    verf.stuff = this->make_auth(AUTH_NULL);
    this->pack_replyheader(xid, verf);

    boolean res = false;
    
    switch (proc) {
      case CREATE_LINK: 
        res = this->handle_CREATE_LINK();
        break;
      case DEVICE_WRITE: 
        res = this->handle_DEVICE_WRITE();
        break;
      case DEVICE_READ: 
        res = this->handle_DEVICE_READ();
        break;
      case DEVICE_READSTB: 
        res = this->handle_DEVICE_READSTB();
        break;
      case DEVICE_TRIGGER: 
        res = this->handle_DEVICE_TRIGGER();
        break;
      case DEVICE_CLEAR: 
        res = this->handle_DEVICE_CLEAR();
        break;
      case DEVICE_REMOTE:
        res = this->handle_DEVICE_REMOTE();
        break;
      case DEVICE_LOCAL:
        res = this->handle_DEVICE_LOCAL();
        break;
      case DEVICE_LOCK:
        res = this->handle_DEVICE_LOCK();
        break;
      case DEVICE_UNLOCK:
        res = this->handle_DEVICE_UNLOCK();
        break;
      case DEVICE_ENABLE_SRQ:
        res = this->handle_DEVICE_ENABLE_SRQ();
        break;
      case DEVICE_DOCMD:
        res = this->handle_DEVICE_DOCMD();
        break;
      case DESTROY_LINK:
        res = this->handle_DESTROY_LINK();
        break;       
      case CREATE_INTR_CHAN:
        res = this->handle_CREATE_INTR_CHAN();
        break;
      case DESTROY_INTR_CHAN:
        res = this->handle_DESTROY_INTR_CHAN();
        break;
      case DEVICE_ABORT:
        res = this->handle_DEVICE_ABORT();
        break;
      case DEVICE_INTR_SRQ:
        res = this->handle_DEVICE_INTR_SRQ();
        break;
      default:
        this->pack_uint(PROC_UNAVAIL);
    }
        
    if (!res) {     
      this->reply_GARBAGE_ARGS(xid);
      return;
    }

    this->pack_recordheader();
    
    // Move the following to a more generic function!
    s = this->rpcPacker::get_buffer();
    char c[500];
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
  
    if (res && (proc == DESTROY_LINK)) {
      client.stop();
      Serial.println("rpcTCPServer: TCP client disconnected");
    }
  }
}

/* ================================================================================================== */

rpcUDPServer::rpcUDPServer(unsigned int prog, unsigned int vers, unsigned int port) : rpcServer(prog, vers, IPPROTO_UDP, port), UDPServer() {
}

void rpcUDPServer::begin(String s) {
  if (this->registered(s))
    this->UDPServer.begin(port);
  else
    Serial.print("rpcUDPServer registration failed!");   
}

void rpcUDPServer::available() {
}


