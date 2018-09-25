#include "systemc.h"
#include "PeripheralDefs.h"

char *FData;

bool FReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(data,&FData[addr],datalen);

    return true;
}

bool FWriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(&FData[addr],data, datalen);

    return true;
}


FLASH::FLASH(const char *name): sc_module(name), slaveb("Flash"){

    slaveb.register_Slave_Addr(FLASH_START_ADDR, FLASH_START_ADDR+FLASH_SIZE);
    slaveb.register_read_cb(FReadCB);
    slaveb.register_write_cb(FWriteCB);

}