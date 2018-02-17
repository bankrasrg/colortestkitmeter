/*
  vxi11.h - Library for VXI-11 servers on Arduino

  WIZnet Connect the Magic 2014 Design Challenge
  Project Registration Number: WZ1295
*/

#ifndef vxi11_h
#define vxi11_h

#include <Arduino.h>
#include "rpc.h"

/* ================================================================================================== */

class vxi11Packer: virtual public rpcPacker {
  public:
    vxi11Packer();
    void pack_device_link(int link);
    void pack_device_error(int error);
    void pack_create_link_resp(int error, int link, unsigned int abort_port, unsigned int max_recv_size);
    void pack_device_write_resp(int error, unsigned int data_size);
    void pack_device_read_resp(int error, unsigned int reason, String data);
    void pack_device_readstb_resp(int error, byte stb);
    void pack_device_docmd_resp(int error, String data);
};

/* ================================================================================================== */

class vxi11Unpacker: virtual public rpcUnpacker {
  public:
    vxi11Unpacker();
    void unpack_device_link(int *id);
    void unpack_create_link_parms(int *id, boolean *lock_device, unsigned int *lock_timeout, String *device);
    void unpack_device_write_parms(int *id, unsigned int *timeout, unsigned int *lock_timeout, byte *flags, String *data);
    void unpack_device_read_parms(int *id, unsigned int *data_size, unsigned int *timeout, unsigned int *lock_timeout, byte *flags, char *termchar);
    void unpack_device_generic_parms(int *id, byte *flags, unsigned int *timeout, unsigned int *lock_timeout);
    void unpack_device_remote_func_parms(unsigned int *host_addr, unsigned int *host_port, unsigned int *prog_num, unsigned int *prog_vers, int *prog_family);
    void unpack_device_enable_srq_parms(int *id, boolean *enable, String *handle);
    void unpack_device_lock_parms(int *id, byte *flags, unsigned int *lock_timeout);
    void unpack_device_docmd_parms(int *id, byte *flags, unsigned int *timeout, unsigned int *lock_timeout, int *cmd, boolean *network_order, int *data_size, String *data_in);
};

/* ================================================================================================== */

class rpcVXI11Server: public rpcTCPServer, public vxi11Packer, public vxi11Unpacker {
  public:
    rpcVXI11Server(unsigned int prog, unsigned int vers, unsigned int port);
    unsigned int prog;
    unsigned int vers;
    unsigned int port;
    unsigned int lastxid;
    String get_data_buffer();
    void set_data_buffer(String s);
    boolean handle_CREATE_LINK();
    boolean handle_DEVICE_WRITE();
    boolean handle_DEVICE_READ();
    boolean handle_DEVICE_READSTB();
    boolean handle_DEVICE_TRIGGER();
    boolean handle_DEVICE_CLEAR();
    boolean handle_DEVICE_REMOTE();
    boolean handle_DEVICE_LOCAL();
    boolean handle_DEVICE_LOCK();
    boolean handle_DEVICE_UNLOCK();
    boolean handle_DEVICE_ENABLE_SRQ();
    boolean handle_DEVICE_DOCMD();
    boolean handle_DESTROY_LINK();
    boolean handle_CREATE_INTR_CHAN();
    boolean handle_DESTROY_INTR_CHAN();
    boolean handle_DEVICE_ABORT();
    boolean handle_DEVICE_INTR_SRQ();
  private:
    String _data_buffer;
};

#endif

