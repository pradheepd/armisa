#include "systemc.h"

/*class forward_interface {
public:
    virtual bool ReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)=0;
    virtual bool WriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)=0;
};

class backward_interface {
public:
    virtual void ReadCB(bool, unsigned char*)=0;
    virtual bool WriteCB(bool)=0;
};

typedef void (backward_interface::*Callback_Read_bw)(bool, unsigned char*);
typedef void (backward_interface::*Callback_Write_bw)(bool);

typedef bool (forward_interface::*Callback_Read_fw)(unsigned int addr, unsigned char *data, unsigned int datalen);
typedef bool (forward_interface::*Callback_Write_fw)(unsigned int addr, unsigned char *data, unsigned int datalen);
*/
#ifndef __PROCESSORBUS_H
#define __PROCESSORBUS_H

typedef void (*Callback_Read_bw)(bool, unsigned char*);
typedef void (*Callback_Write_bw)(bool);

typedef bool (*Callback_Read_fw)(unsigned int addr, unsigned char *data, unsigned int datalen);
typedef bool (*Callback_Write_fw)(unsigned int addr, unsigned char *data, unsigned int datalen);

typedef enum {CMD_READ=0,CMD_WRITE}BUSCMD;

class ProcessorBus_base : public sc_interface
{
    char *name;
    public:
    ProcessorBus_base(const char * nm);
    virtual bool write(unsigned int addr, unsigned char *data, unsigned int datalen) = 0;
    virtual bool read(unsigned int addr, unsigned char *data, unsigned int datalen) = 0;
    virtual const char *Name();
};

class ProcessorBus_Slave_base : public sc_interface
{
    char *name;
    public:
    ProcessorBus_Slave_base(const char * nm);
    virtual bool register_Slave_Addr(sc_dt::uint64 start, sc_dt::uint64 end) = 0;
    virtual bool bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err)=0;
    virtual void register_read_cb(Callback_Read_fw)=0;
    virtual void register_write_cb(Callback_Write_fw)=0;
    virtual const char *Name();
};

class ProcessorBus_Master_b : public ProcessorBus_base
{
    public:
    ProcessorBus_Master_b(const char * nm);
    virtual bool write(unsigned int addr, unsigned char *data, unsigned int datalen);
    virtual bool read(unsigned int addr, unsigned char *data, unsigned int datalen);
};

class ProcessorBus_Slave_b : public ProcessorBus_Slave_base
{
    int connectid;
    public:
    ProcessorBus_Slave_b(const char * nm);
    virtual bool register_Slave_Addr(sc_dt::uint64 start, sc_dt::uint64 end);
    virtual bool bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err);
    virtual void register_read_cb(Callback_Read_fw);
    virtual void register_write_cb(Callback_Write_fw);
};

class ProcessorBus_Master_nb : public ProcessorBus_base
{
    public:
    ProcessorBus_Master_nb(const char *);
    virtual bool write(unsigned int addr, unsigned char *data, unsigned int datalen);
    virtual bool read(unsigned int addr, unsigned char *data, unsigned int datalen);
    virtual void register_read_cb(Callback_Read_bw);
    virtual void register_write_cb(Callback_Write_bw);

};

class ProcessorBus_Slave_nb : public ProcessorBus_Slave_base
{
    int connectid;
    public:
    ProcessorBus_Slave_nb(const char * nm);
    virtual bool register_Slave_Addr(sc_dt::uint64 start, sc_dt::uint64 end);
    virtual bool bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err);
    virtual void register_read_cb(Callback_Read_fw);
    virtual void register_write_cb(Callback_Write_fw);
};

#endif
