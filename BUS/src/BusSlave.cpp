#include "BusTop.h"

ProcessorBus_Slave_base::ProcessorBus_Slave_base(const char * nm){
    int strl = strlen(nm)+1;
    name = (char *)malloc(strl);
    strcpy(name,nm);
    name[strl-1] = '\0';
}

const char * ProcessorBus_Slave_base::Name() {
    return name;
}

ProcessorBus_Slave_b::ProcessorBus_Slave_b(const char * nm) : ProcessorBus_Slave_base(nm){}

bool ProcessorBus_Slave_b::register_Slave_Addr(sc_dt::uint64 start, sc_dt::uint64 end)
{
    connectid = BusTop::GetInstance()->CreateInterConnect(start, end, INTC_B, this);

    if(connectid < 0)
        return false;
    else
        return true;
}

bool ProcessorBus_Slave_b::bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err)
{
    cout << "Error, This is not a non blocking interface" << endl;
    return false;
}

void ProcessorBus_Slave_b::register_read_cb(Callback_Read_fw cb)
{
    BusTop::GetInstance()->register_read_cb_fw(connectid, cb);
}

void ProcessorBus_Slave_b::register_write_cb(Callback_Write_fw cb)
{
    BusTop::GetInstance()->register_write_cb_fw(connectid, cb);
}

ProcessorBus_Slave_nb::ProcessorBus_Slave_nb(const char * nm) : ProcessorBus_Slave_base(nm){}

bool ProcessorBus_Slave_nb::register_Slave_Addr(sc_dt::uint64 start, sc_dt::uint64 end)
{
    connectid = BusTop::GetInstance()->CreateInterConnect(start, end, INTC_NB, this);

    if(connectid < 0)
        return false;
    else
        return true;
}

bool ProcessorBus_Slave_nb::bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err)
{
    return BusTop::GetInstance()->bus_write(connectid, addr, data, datalen, e, err); 
}

void ProcessorBus_Slave_nb::register_read_cb(Callback_Read_fw cb)
{
    BusTop::GetInstance()->register_read_cb_fw(connectid, cb);
}

void ProcessorBus_Slave_nb::register_write_cb(Callback_Write_fw cb)
{
    BusTop::GetInstance()->register_write_cb_fw(connectid, cb);
}