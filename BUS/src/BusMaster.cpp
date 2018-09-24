#include "BusTop.h"

ProcessorBus_base::ProcessorBus_base(const char *nm){
    int strl = strlen(nm)+1;
    name = (char *)malloc(strl);
    strcpy(name,nm);
    name[strl-1] = '\0';
}

const char * ProcessorBus_base::Name() {
    return name;
}

bool ProcessorBus_Master_b::write(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    return BusTop::GetInstance()->write(addr, data, datalen);
}

ProcessorBus_Master_b::ProcessorBus_Master_b(const char *name) : ProcessorBus_base(name){}

bool ProcessorBus_Master_b::read(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    return BusTop::GetInstance()->read(addr, data, datalen);
}

ProcessorBus_Master_nb::ProcessorBus_Master_nb(const char *name) : ProcessorBus_base(name){}

bool ProcessorBus_Master_nb::write(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    return BusTop::GetInstance()->nb_write(addr, data, datalen);
}

bool ProcessorBus_Master_nb::read(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    return BusTop::GetInstance()->nb_read(addr, data, datalen);
}

void ProcessorBus_Master_nb::register_read_cb(Callback_Read_bw cb)
{
    BusTop::GetInstance()->register_read_cb_bw(cb);
}
    
void ProcessorBus_Master_nb::register_write_cb(Callback_Write_bw cb)
{
    BusTop::GetInstance()->register_write_cb_bw(cb);
}
