#include "systemc.h"
#include "PeripheralDefs.h"

static unsigned char *IData;

bool IReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(data,&IData[addr-INTRNAL_START_ADDR],datalen);

    return true;
}

bool IWriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(&IData[addr-INTRNAL_START_ADDR],data, datalen);

    return true;
}


INTRNAL::INTRNAL(const char *name): sc_module(name), slaveb("InternalMem"){

    slaveb.register_Slave_Addr(INTRNAL_START_ADDR, INTRNAL_START_ADDR+INTRNAL_SIZE);
    slaveb.register_read_cb(IReadCB);
    slaveb.register_write_cb(IWriteCB);

    IData = (unsigned char *)malloc(INTRNAL_SIZE);

    unsigned int val = START_SP_ADDR;
    
    IWriteCB(VT_IN_SP_VAL_ADDR, (unsigned char *)&val, 4);

    val = START_PC_ADDR;

    IWriteCB(VT_IN_SP_VAL_ADDR, (unsigned char *)&val, 4);

}