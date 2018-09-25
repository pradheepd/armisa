#include "ProcessorBus.h"
// flash size in kbs
#define FLASH_SIZE  64 * 1024

// sram size in kbs
#define SRAM_SIZE   20 * 1024

//flash start address
#define FLASH_START_ADDR    0x00000000

//sram start address
#define SRAM_START_ADDR     0x20000000

// Declare all peripherals here

//flash
class FLASH : public sc_module{
    ProcessorBus_Slave_b slaveb;
public :
    FLASH(const char *);
};

//sram
class SRAM : public sc_module{
    ProcessorBus_Slave_b slaveb;
public :
    SRAM(const char *);
};