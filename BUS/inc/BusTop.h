#include "SimpleInitTarg.h"
#include <map>

using namespace std;

typedef enum {INTC_B = 0, INTC_NB}InterconnectType;

typedef struct _ITC{
    Simple_Initiator        *i;
    Simple_Target           *t;
    sc_dt::uint64           start;
    sc_dt::uint64           end;
    InterconnectType        type;
    ProcessorBus_Slave_base *b_sl;
    _ITC(): i(NULL), t(NULL), type(INTC_B), b_sl(NULL), start(0), end(0){}
}InterConnect;

class BusTop
{
    map<int,InterConnect*> ICMap;
    int MapCount;
    int getInterConnectID(sc_dt::uint64 start, sc_dt::uint64 end);
    BusTop();
    static BusTop *Instance;
    
    public:

    static BusTop* GetInstance();
    int CreateInterConnect(sc_dt::uint64 start, sc_dt::uint64 end, InterconnectType e, ProcessorBus_Slave_base *sl);
    bool write(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen);
    bool read(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen);
    bool nb_write(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen);
    bool nb_read(sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen);
    void register_read_cb_bw(Callback_Read_bw);
    void register_write_cb_bw(Callback_Write_bw);
    void register_read_cb_fw(int id, Callback_Read_fw);
    void register_write_cb_fw(int id, Callback_Write_fw);
    bool bus_write(int id, sc_dt::uint64 addr, unsigned char *data, sc_dt::uint64 datalen, BUSCMD e, bool err );
    ~BusTop();
};