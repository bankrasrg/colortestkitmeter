/*
  rpc.h - Library for RPC on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef rpc_h
#define rpc_h

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "xdr.h"

#define RPCVERSION 2 // (rpcvers)

// RPC constants servers
#define PMAP_PROG 0x0186A0 // (prog)
#define PMAP_VERS 2        // (vers)
#define PMAP_PORT 111      // (port)

// Portmap procedures (proc)
#define PMAPPROC_NULL    0
#define PMAPPROC_SET     1
#define PMAPPROC_UNSET   2
#define PMAPPROC_GETPORT 3
#define PMAPPROC_DUMP    4
#define PMAPPROC_CALLIT  5

// Protocols (prot)
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

// Message type (mtype)
#define CALL  0
#define REPLY 1

// Authentication flavors (flavor)
#define AUTH_NULL  0 // No authentication
#define AUTH_UNIX  1 // Unix style (uid + gids)
#define AUTH_SHORT 2 // Short hand unix style
#define AUTH_DES   3 // DES style (encrypted timestamp)

// Reply status (stat)
#define MSG_ACCEPTED 0
#define MSG_DENIED   1

// Accept status (astat)
#define SUCCESS       0 // RPC executed successfully
#define PROG_UNAVAIL  1 // Remote hasn’t exported program
#define PROG_MISMATCH 2 // Remote can’t support version #
#define PROC_UNAVAIL  3 // Program can’t support procedure
#define GARBAGE_ARGS  4 // Procedure can’t decode params
#define SYSTEM_ERR    5 // E.g. memory allocation failure

// Reject status (rstat)
#define RPC_MISMATCH 0 // RPC version number != RPCVERSION
#define AUTH_ERROR   1 // Remote can’t authenticate caller

// Authentication status at remote end (auth)
#define AUTH_OK           0 // Success
#define AUTH_BADCRED      1 // Bad credential (seal broken)
#define AUTH_REJECTEDCRED 2 // Client must begin new session
#define AUTH_BADVERF      3 // Bad verifier (seal broken)
#define AUTH_REJECTEDVERF 4 // Verifier expired or replayed
#define AUTH_TOOWEAK      5 // Rejected for security reasons

// Memory allocation for portmap list
#define PMAPLIST_SIZE 8

// RPC constants servers
#define DEVICE_CORE_PROG 0x0607AF
#define DEVICE_CORE_VERS 1
#define DEVICE_CORE_PORT 50001

#define DEVICE_ASYNC_PROG 0x0607B0
#define DEVICE_ASYNC_VERS 1
#define DEVICE_ASYNC_PORT 50002

#define DEVICE_INTR_PROG 0x0607B1
#define DEVICE_INTR_VERS 1
#define DEVICE_INTR_PORT 50003

// VXI-11 Core procedures (proc)
#define CREATE_LINK       10
#define DEVICE_WRITE      11
#define DEVICE_READ       12
#define DEVICE_READSTB    13
#define DEVICE_TRIGGER    14
#define DEVICE_CLEAR      15
#define DEVICE_REMOTE     16
#define DEVICE_LOCAL      17
#define DEVICE_LOCK       18
#define DEVICE_UNLOCK     19
#define DEVICE_ENABLE_SRQ 20
#define DEVICE_DOCMD      22
#define DESTROY_LINK      23
#define CREATE_INTR_CHAN  25
#define DESTROY_INTR_CHAN 26

// VXI-11 Async procedures (proc)
#define DEVICE_ABORT      1

// VXI-11 Intr procedures (proc)
#define DEVICE_INTR_SRQ   30

// Error states
#define ERR_NO_ERROR                      0
#define ERR_SYNTAX_ERROR                  1
#define ERR_DEVICE_NOT_ACCESSIBLE         3
#define ERR_INVALID_LINK_IDENTIFIER       4
#define ERR_PARAMETER_ERROR               5
#define ERR_CHANNEL_NOT_ESTABLISHED       6
#define ERR_OPERATION_NOT_SUPPORTED       8
#define ERR_OUT_OF_RESOURCES              9
#define ERR_DEVICE_LOCKED_BY_ANOTHER_LINK 11
#define ERR_NO_LOCK_HELD_BY_THIS_LINK     12
#define ERR_IO_TIMEOUT                    15
#define ERR_IO_ERROR                      17
#define ERR_INVALID_ADDRESS               21
#define ERR_ABORT                         23
#define ERR_CHANNEL_ALREADY_ESTABLISHED   29

// Flags
#define OP_FLAG_WAIT_BLOCK   1
#define OP_FLAG_END          8
#define OP_FLAG_TERMCHAR_SET 128
#define RX_REQCNT            1
#define RX_CHR               2
#define RX_END               4

struct sAuth {
  int flavor;
  String stuff;
};

struct sPortmap2 {
  unsigned int prog;
  unsigned int vers;
  unsigned int prot;
  unsigned int port;
};

/* ================================================================================================== */

class rpcPacker: virtual public xdrPacker {
  public:
    rpcPacker();
    String make_auth(int flavor);
    void pack_auth(sAuth &a);
    void pack_auth(int flavor, String stuff);
    // pack_auth_unix
    void pack_recordheader();
    void pack_callheader(unsigned int xid, unsigned int prog, unsigned int vers, unsigned int proc, sAuth cred, sAuth verf);
    void pack_replyheader(unsigned int xid, sAuth verf);
    void pack_mapping(sPortmap2 &pmap);
};

/* ================================================================================================== */

class rpcUnpacker: virtual public xdrUnpacker {  
  public:
    rpcUnpacker();
    sAuth unpack_auth();
    void unpack_recordheader();
    void unpack_callheader(unsigned int *xid, unsigned int *prog, unsigned int *vers, unsigned int *proc, sAuth *cred, sAuth *verf);   
    void unpack_replyheader(unsigned int *xid, sAuth *verf);
};

/* ================================================================================================== */

class rpcServer: virtual public rpcPacker, virtual public rpcUnpacker {
  public:
    rpcServer(unsigned int prog, unsigned int vers, unsigned int prot, unsigned int port);
    unsigned int prog;
    unsigned int vers;
    unsigned int prot;
    unsigned int port;
    unsigned int lastxid;
    String intentions();
    boolean registered(String s);
    // unregister
    // turn_around
    // Function rpcAddPackers has not been implemented.
    virtual boolean handle_CREATE_LINK() = 0;
    virtual boolean handle_DEVICE_WRITE() = 0;
    virtual boolean handle_DEVICE_READ() = 0;
    virtual boolean handle_DEVICE_READSTB() = 0;
    virtual boolean handle_DEVICE_TRIGGER() = 0;
    virtual boolean handle_DEVICE_CLEAR() = 0;
    virtual boolean handle_DEVICE_REMOTE() = 0;
    virtual boolean handle_DEVICE_LOCAL() = 0;
    virtual boolean handle_DEVICE_LOCK() = 0;
    virtual boolean handle_DEVICE_UNLOCK() = 0;
    virtual boolean handle_DEVICE_ENABLE_SRQ() = 0;
    virtual boolean handle_DEVICE_DOCMD() = 0;
    virtual boolean handle_DESTROY_LINK() = 0;
    virtual boolean handle_CREATE_INTR_CHAN() = 0;
    virtual boolean handle_DESTROY_INTR_CHAN() = 0;
    virtual boolean handle_DEVICE_ABORT() = 0;
    virtual boolean handle_DEVICE_INTR_SRQ() = 0;
    void reply_PROG_UNAVAIL();
    void reply_PROG_MISMATCH();
    void reply_GARBAGE_ARGS(unsigned int xid);
};

/* ================================================================================================== */

class rpcTCPServer: public rpcServer {
  public:
    rpcTCPServer(unsigned int prog, unsigned int vers, unsigned int port);
    EthernetServer TCPServer;
    void begin(String s);
    void available();
    // connect
    // loop
    // session
    // forkingloop
    // forksession
  private:
};

/* ================================================================================================== */

class rpcUDPServer: public rpcServer {
  public:
    rpcUDPServer(unsigned int prog, unsigned int vers, unsigned int port);
    EthernetUDP UDPServer;
    void begin(String s);
    void available();
    // connect
    // loop
    // session
};

/* ================================================================================================== */

// Class rpcError has not been implemented.
// Class rpcBadFormat has not been implemented.
// Class rpcBadVersion has not been implemented.
// Class rpcGarbadeArgs has not been implemented.
// Class rpcUnpackError has not been implemented.

// Class rpcClient has not been implemented.
// Class rpcRawTCPClient has not been implemented.
// Class rpcRawUDPClient has not been implemented.
// Class rpcRawBroadcastUDPClient has not been implemented.

#endif

