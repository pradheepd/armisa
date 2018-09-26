#include "systemc.h"
#include "PeripheralDefs.h"

static unsigned char *SData;

bool SReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(data,&SData[addr-SRAM_START_ADDR],datalen);

    return true;
}

bool SWriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(&SData[addr-SRAM_START_ADDR],data, datalen);

    return true;
}


SRAM::SRAM(const char *name): sc_module(name), slaveb("sram"){

    slaveb.register_Slave_Addr(SRAM_START_ADDR, SRAM_START_ADDR+SRAM_SIZE);
    slaveb.register_read_cb(SReadCB);
    slaveb.register_write_cb(SWriteCB);

    SData = (unsigned char *)malloc(SRAM_SIZE);

}