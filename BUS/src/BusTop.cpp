#include "BusTop.h"
#include "string.h"

BusTop *BusTop::Instance = NULL;

BusTop* BusTop::GetInstance(){
    if(Instance == NULL)
        Instance = new BusTop();

    return Instance;
}

BusTop::BusTop(): MapCount(0){}

int BusTop::CreateInterConnect(sc_dt::uint64 start, sc_dt::uint64 end, InterconnectType e
    , ProcessorBus_Slave_base *sl) {

    for(map<int,InterConnect*>::iterator it = ICMap.begin(); it != ICMap.end(); ++it) {
        if((start >= it->second->start) 
        || (start < it->second->end) 
        || (end >= it->second->start) 
        || (end < it->second->end) ){
            cout << "Addr range inside another peripheral : " << it->second->b_sl->Name() << endl;

            return -1;
        }
    }

    string itr_nm = sl->Name() + string("_itr") ;
    string trg_nm = sl->Name() + string("_trg") ;

    Simple_Initiator *itr = new Simple_Initiator(itr_nm.c_str());

    Simple_Target *trg = new Simple_Target(trg_nm.c_str());

    itr->init_socket(trg->targ_socket);

    InterConnect *ic = new InterConnect();

    ic->i = itr;
    ic->t = trg;
    ic->type = e;
    ic->start = start;
    ic->end = end;
    ic->b_sl = sl;

    ICMap.insert(pair<int,InterConnect*>(MapCount,ic));
    MapCount++;

    return (MapCount - 1);
}

int BusTop::getInterConnectID(sc_dt::uint64 start, sc_dt::uint64 end)
{
    for(map<int,InterConnect*>::iterator it = ICMap.begin(); it != ICMap.end(); ++it) {
        if(start >= it->second->start && end <= it->second->end)
            return it->first;
    }
    return -1;
}

bool BusTop::write(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen)
{
    int id = getInterConnectID(addr, addr + datalen);

    map<int,InterConnect*>::iterator itc = ICMap.find(id);

    if(itc == ICMap.end())
        return false;

    sc_time delay(0,SC_NS);

    if(itc->second->type == INTC_B) {

        tlm_generic_payload gp;
        
        gp.set_command(TLM_WRITE_COMMAND);
        gp.set_data_length(datalen);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_byte_enable_ptr(0);

        itc->second->i->init_socket->b_transport(gp, delay);
        if(gp.get_response_status() == TLM_OK_RESPONSE)
            return true;
        else
            return false;
    } else {
        cout << "The peripheral only supports non blocking Calls : " << itc->second->b_sl->Name() << endl;
        return false;
    }
}

bool BusTop::read(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen)
{
    int id = getInterConnectID(addr, addr + datalen);

    map<int,InterConnect*>::iterator itc = ICMap.find(id);

    if(itc == ICMap.end())
        return false;

    sc_time delay(0,SC_NS);

    if(itc->second->type == INTC_B) {

        tlm_generic_payload gp;
        
        gp.set_command(TLM_READ_COMMAND);
        gp.set_data_length(datalen);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_byte_enable_ptr(0);

        itc->second->i->init_socket->b_transport(gp, delay);
        if(gp.get_response_status() == TLM_OK_RESPONSE)
            return true;
        else
            return false;
    } else {
        cout << "The peripheral only supports non blocking Calls : " << itc->second->b_sl->Name() << endl;
        return false;
    }
}

bool BusTop::nb_write(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen)
{
    int id = getInterConnectID(addr, addr + datalen);

    map<int,InterConnect*>::iterator itc = ICMap.find(id);

    sc_time delay(0,SC_NS);

    if(itc == ICMap.end())
        return false;

    if(itc->second->type == INTC_NB) {

        tlm_generic_payload *trans = new tlm_generic_payload();

        tlm_generic_payload &gp = *trans;
        
        gp.set_command(TLM_WRITE_COMMAND);
        gp.set_data_length(datalen);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_byte_enable_ptr(0);

        tlm_phase ph = BEGIN_REQ;

        itc->second->i->init_socket->nb_transport_fw(gp, ph, delay);
        if(gp.get_response_status() == TLM_OK_RESPONSE)
            return true;
        else
            return false;
    } else {
        cout << "The peripheral only supports non blocking Calls : " << itc->second->b_sl->Name() << endl;
        return false;
    }
}

bool BusTop::nb_read(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen)
{
    int id = getInterConnectID(addr, addr + datalen);

    map<int,InterConnect*>::iterator itc = ICMap.find(id);

    sc_time delay(0,SC_NS);

    if(itc == ICMap.end())
        return false;

    if(itc->second->type == INTC_NB) {

        tlm_generic_payload *trans = new tlm_generic_payload();

        tlm_generic_payload &gp = *trans;
        
        gp.set_command(TLM_READ_COMMAND);
        gp.set_data_length(datalen);
        gp.set_address(addr);
        gp.set_data_ptr(data);
        gp.set_byte_enable_ptr(0);

        tlm_phase ph = BEGIN_REQ;

        itc->second->i->init_socket->nb_transport_fw(gp, ph, delay);
        if(gp.get_response_status() == TLM_OK_RESPONSE)
            return true;
        else
            return false;
    } else {
        cout << "The peripheral only supports non blocking Calls : " << itc->second->b_sl->Name() << endl;
        return false;
    }
}

void BusTop::register_read_cb_bw(Callback_Read_bw cb)
{
    for(map<int,InterConnect*>::iterator it = ICMap.begin(); it != ICMap.end(); ++it) {
        it->second->i->register_init_read_cb(cb);
    }
}

void BusTop::register_write_cb_bw(Callback_Write_bw cb)
{
    for(map<int,InterConnect*>::iterator it = ICMap.begin(); it != ICMap.end(); ++it) {
        it->second->i->register_init_write_cb(cb);
    }
}

void BusTop::register_read_cb_fw(int id, Callback_Read_fw cb)
{
    map<int,InterConnect*>::iterator it = ICMap.find(id);

    if(it != ICMap.end())
        it->second->t->register_targ_read_cb(cb);
}

void BusTop::register_write_cb_fw(int id, Callback_Write_fw cb)
{
    map<int,InterConnect*>::iterator it = ICMap.find(id);

    if(it != ICMap.end())
        it->second->t->register_targ_write_cb(cb);
}

bool BusTop::bus_write(int id, sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen, BUSCMD e, bool err )
{
    map<int,InterConnect*>::iterator it = ICMap.find(id);

    if(it != ICMap.end())
        return it->second->t->bus_write(addr, data, datalen, e, err);
    
    return false;
}

BusTop::~BusTop()
{
    for(map<int,InterConnect*>::iterator it = ICMap.begin(); it != ICMap.end(); ++it) {
        delete it->second->i ;
        delete it->second->t ;

        delete it->second;
    }
}
