#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "tlm_utils/peq_with_get.h"

#include "ProcessorBus.h"

using namespace tlm;
using namespace tlm_utils;

class Simple_Initiator : sc_module
{
    peq_with_cb_and_phase<Simple_Initiator> PEQ;
    tlm_sync_enum nb_transport_bw_cb(tlm_generic_payload &trans, tlm_phase &ph, sc_time &delay);
    Callback_Write_bw Init_Write_CB;
    Callback_Read_bw Init_Read_CB;
    void peq_cb_w_phase ( tlm_generic_payload &gp, const tlm_phase &ph);
public:
    simple_initiator_socket<Simple_Initiator> init_socket;
    Simple_Initiator(const char *name);
    void register_init_write_cb(Callback_Write_bw cb);
    void register_init_read_cb(Callback_Read_bw cb);
};

class Simple_Target : sc_module
{
    peq_with_get<tlm_generic_payload> PEQ;
    void b_transport_cb(tlm_generic_payload &trans, sc_time &delay);
    tlm_sync_enum nb_transport_fw_cb(tlm_generic_payload &trans, tlm_phase &ph, sc_time &delay);    
    void Targ_Thread();
    Callback_Write_fw Targ_Write_CB;
    Callback_Read_fw Targ_Read_CB;
public:    
    simple_target_socket<Simple_Target> targ_socket;
    Simple_Target(const char *name);
    void register_targ_write_cb(Callback_Write_fw cb);
    void register_targ_read_cb(Callback_Read_fw cb);
    bool bus_write(unsigned int addr, unsigned char *data, unsigned int datalen, BUSCMD e, bool err);
};
