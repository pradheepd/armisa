#include "SimpleInitTarg.h"

Simple_Initiator::Simple_Initiator(const char *name): sc_module(sc_module_name (name)), PEQ("Init_PEQ",this,&Simple_Initiator::peq_cb_w_phase)
{
    init_socket.register_nb_transport_bw(this, &Simple_Initiator::nb_transport_bw_cb);
}

tlm_sync_enum Simple_Initiator::nb_transport_bw_cb(tlm_generic_payload &gp,
    tlm_phase &phase, sc_time &delay) {
        
    if(phase == BEGIN_REQ || phase == END_RESP){
        cout << "Invalid TLM call recieved PHASE :"  << phase << endl; 
        sc_stop();
        return TLM_COMPLETED;
    }else if(phase == END_REQ) {
        //cout << this->name() << " : End request recieved" << endl;
        gp.set_response_status(TLM_OK_RESPONSE);
        return TLM_ACCEPTED;
    } else if(phase == BEGIN_RESP) {
        PEQ.notify(gp,phase, delay);
        gp.set_response_status(TLM_OK_RESPONSE);
        return TLM_ACCEPTED;
    } else{
        gp.set_response_status(TLM_OK_RESPONSE);
        return TLM_ACCEPTED;
    }
}

void Simple_Initiator::peq_cb_w_phase ( tlm_generic_payload &gp, const tlm_phase &ph){

    if(ph != BEGIN_RESP)
        cout << "Error : Phase is not begin resp" << endl;

    if(gp.get_command() == TLM_READ_COMMAND){

        if(gp.get_response_status() == TLM_OK_RESPONSE)
            Init_Read_CB(true, gp.get_data_ptr());
        else
            Init_Read_CB(false, NULL);

        delete (tlm_generic_payload *)&gp;

    } else if(gp.get_command() == TLM_WRITE_COMMAND){

        if(gp.get_response_status() == TLM_OK_RESPONSE)
            Init_Write_CB(true);
        else
            Init_Write_CB(false);

        delete (tlm_generic_payload *)&gp;

    }
}

 void Simple_Initiator::register_init_write_cb(Callback_Write_bw cb){
    Init_Write_CB = cb;
 }
    
void Simple_Initiator::register_init_read_cb(Callback_Read_bw cb){
    Init_Read_CB = cb;
}

Simple_Target::Simple_Target(const char *name): sc_module(sc_module_name (name)), PEQ("Targ_PEQ")
{
    SC_HAS_PROCESS(Simple_Target);
    SC_THREAD(Targ_Thread);
    sensitive << PEQ.get_event();

    targ_socket.register_b_transport(this, &Simple_Target::b_transport_cb);
    targ_socket.register_nb_transport_fw(this, &Simple_Target::nb_transport_fw_cb);
}

void Simple_Target::Targ_Thread() {
    while(1) {
    wait();

    //cout << "Event recieved : " << sc_time_stamp() << endl;

    tlm_generic_payload *transp = PEQ.get_next_transaction();

    while(transp != NULL) {
                                  
        tlm_generic_payload &trans = *transp;

        //cout << "Target thread recieved the transaction" << endl;

        tlm_phase ph = BEGIN_RESP;

        sc_time delay(0,SC_NS);

        if(targ_socket->nb_transport_bw(trans,ph,delay) == TLM_ACCEPTED)
        {
            sc_dt::uint64 addr = trans.get_address();
            unsigned char *dataptr = trans.get_data_ptr();
            unsigned int datalen = trans.get_data_length();

            switch(trans.get_command()) {
                case TLM_WRITE_COMMAND:
                    if(Targ_Write_CB(addr, dataptr, datalen))
                        trans.set_response_status(TLM_OK_RESPONSE);
                    else
                        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
                break;
                case TLM_READ_COMMAND:
                    if(Targ_Read_CB(addr, dataptr, datalen))
                        trans.set_response_status(TLM_OK_RESPONSE);
                    else
                        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
                break;
                default:
                break;
            }
        }
        transp = PEQ.get_next_transaction();
    }
    }
}

bool Simple_Target::bus_write(unsigned int addr, unsigned char *data, unsigned int datalen,BUSCMD e, bool error){

    tlm_generic_payload *transp = new tlm_generic_payload();

    tlm_generic_payload &trans = *transp;

    tlm_phase ph = BEGIN_RESP ;

    sc_time delay(0,SC_NS);

    if(e == CMD_READ)
        trans.set_command(TLM_READ_COMMAND);
    else
        trans.set_command(TLM_WRITE_COMMAND);

    trans.set_data_length(datalen);
    trans.set_address(addr);
    trans.set_data_ptr(data);
    
    trans.set_byte_enable_ptr(0);

    if(!error)
        trans.set_response_status(TLM_OK_RESPONSE);
    else
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);

    if(targ_socket->nb_transport_bw(trans,ph,delay) != TLM_ACCEPTED){
        cout << "end response failed ..." << endl;
        return false;
    } else
        return true;
}

void Simple_Target::b_transport_cb(tlm_generic_payload &trans, sc_time &delay)
{
    sc_dt::uint64 addr = trans.get_address();
    unsigned char *dataptr = trans.get_data_ptr();
    unsigned int datalen = trans.get_data_length();

    switch(trans.get_command()) {
        case TLM_WRITE_COMMAND:
            if(Targ_Write_CB(addr, dataptr, datalen))
                trans.set_response_status(TLM_OK_RESPONSE);
            else
                trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        break;
        case TLM_READ_COMMAND:
            if(Targ_Read_CB(addr, dataptr, datalen))
                trans.set_response_status(TLM_OK_RESPONSE);
            else
                trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        break;
        default:
        break;
    }
}

tlm_sync_enum Simple_Target::nb_transport_fw_cb(tlm_generic_payload &trans, tlm_phase &ph, sc_time &delay)
{
    if(ph == END_REQ || ph == BEGIN_RESP){
            cout << "Invalid TLM call recieved PHASE : "  << ph << endl; 
            sc_stop();
            return TLM_COMPLETED;
        }else if(ph == BEGIN_REQ){
            if(trans.get_address() > 1024) {
                cout << "Address out of Range ..." << endl;
                return TLM_COMPLETED;
            }
            //cout << "BEGIN_REQ :" << sc_time_stamp() << endl;
            trans.set_response_status(TLM_OK_RESPONSE);
            PEQ.notify(trans,sc_time(0,SC_NS));
            return TLM_ACCEPTED;
        } else if (ph == END_RESP){
            trans.set_response_status(TLM_OK_RESPONSE);
            return TLM_COMPLETED;
        } 
        return TLM_ACCEPTED ;
}

void Simple_Target::register_targ_write_cb(Callback_Write_fw cb)
{
    Targ_Write_CB = cb;
}

void Simple_Target::register_targ_read_cb(Callback_Read_fw cb)
{
    Targ_Read_CB = cb;
}