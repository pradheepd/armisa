#include "ProcessorBus.h"

// flash size in kbs
#define FLASH_SIZE  64 * 1024

// sram size in kbs
#define SRAM_SIZE   20 * 1024

//flash start address
#define FLASH_START_ADDR    0x8000

//sram start address
#define SRAM_START_ADDR     0x20000000

//internal mem size
#define INTRNAL_SIZE        1024

//internal start addr
#define INTRNAL_START_ADDR  0x00000000

//startup stack pointer address
#define START_SP_ADDR       SRAM_START_ADDR

//startup PC address
#define START_PC_ADDR       FLASH_START_ADDR

//vector Macros

#define VT_IN_SP_VAL_ADDR      0x0
#define VT_RESET               0x4
#define VT_SOFTWARE_INTR       0x8

// Declare all peripherals here

//internal memory to store vector table etc
class INTRNAL : public sc_module{
    ProcessorBus_Slave_b slaveb;
public :
    INTRNAL(const char *);
};

//flash
class FLASH : public sc_module{
    ProcessorBus_Slave_b slaveb;
public :
    FLASH(const char *);
    bool Load_Program(const char*);
};

//sram
class SRAM : public sc_module{
    ProcessorBus_Slave_b slaveb;
public :
    SRAM(const char *);
};
