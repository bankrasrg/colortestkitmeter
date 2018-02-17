/*
  portmap.h - Library for port mapper on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef portmap_h
#define portmap_h

#include <Arduino.h>
#include "rpc.h"

struct sPortmap {
  unsigned int prog;
  unsigned int vers;
  unsigned int prot;
  unsigned int port;
};

struct sCallArgs {
  unsigned int prog;
  unsigned int vers;
  unsigned int proc;
  String args;
};

struct sCallResult {
  unsigned int port;
  String res; 
};

/* ================================================================================================== */

class portmapPacker: public rpcPacker {
  public:
    portmapPacker();
    void pack_mapping(sPortmap &pmap);
    template<int N>
    void pack_pmaplist(sPortmap (&l)[N]) {
      for (int i = 0; i < N; i++) {
        this->pack_uint(1);
        this->pack_mapping(l[i]);
      }
      this->pack_uint(0);
    };
    void pack_call_args(sCallArgs &cargs);
};

/* ================================================================================================== */

class portmapUnpacker: public rpcUnpacker {
  public:
    portmapUnpacker();
    sPortmap unpack_mapping();
    template<int N, typename T>
    T (&unpack_pmaplist(T (&p)[N]))[N] {
      for (int i = 0; i < N; i++) {
        unsigned int x = this->unpack_uint();
        if (x == 0) {
          break;
        }
        else {
          p[i] = this->unpack_mapping();
        }       
      }
      return p;
    }    
    sCallResult unpack_call_result();
};

/* ================================================================================================== */

class rpcPortmapServer {
  public:
    rpcPortmapServer(unsigned int prog, unsigned int vers, unsigned int port);
    unsigned int prog;
    unsigned int vers;
    unsigned int port;
    unsigned int lastxid;
    portmapPacker portmapPack;
    portmapUnpacker portmapUnpack;
    EthernetServer TCPServer;
    EthernetUDP UDPServer;
    void begin();
    void available();
    String registerServer(String s);
    boolean handle_PMAPPROC_NULL();
    boolean handle_PMAPPROC_SET();
    boolean handle_PMAPPROC_UNSET();
    boolean handle_PMAPPROC_GETPORT();
    boolean handle_PMAPPROC_DUMP();
    boolean handle_PMAPPROC_CALLIT();
    void reply_PROG_UNAVAIL();
    void reply_PROG_MISMATCH();
    void reply_GARBAGE_ARGS(unsigned int xid);
  private:
    sPortmap _pmaplist[PMAPLIST_SIZE];
};

#endif

