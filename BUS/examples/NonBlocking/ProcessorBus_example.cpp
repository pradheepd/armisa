#include "ProcessorBus.h"
#include "iostream"

using namespace std;

char MData[20];

void MasterReadCB(bool result, unsigned char* data)
{
    if(result){
        cout << "Read Callback recieved : success" << endl;

        if(strcmp("Hello World",(char *)data)== 0){
            cout << "Data verified .." << endl;
        } else
            cout << "Data verification failed .." << endl;
    } else
        cout << "Read Callback recieved : fail" << endl;
    sc_stop();
}

void MasterWriteCB(bool result)
{
    if(result){
        cout << "Write Callback recieved : success" << endl;
    } else
        cout << "Write Callback recieved : fail" << endl;
}

SC_MODULE(MASTER) {

ProcessorBus_Master_nb masterb;

void thread() {

    strcpy(MData,"Hello World") ;

    int datalen = strlen(MData);

    if(masterb.write(0,(unsigned char *)MData,datalen+1))
        cout << "Write succeeded .." << endl;
    else
        cout << "Write Failed .." << endl;

    if(masterb.read(0,(unsigned char *)MData,datalen+1))
        cout << "Read succeeded .." << endl;
    else
        cout << "Read Failed .." << endl;
    
}

friend void MasterReadCB(bool result, unsigned char* data);
friend void MasterWriteCB(bool result);

SC_CTOR(MASTER): masterb("Bus_Master") {
    SC_THREAD(thread);

    masterb.register_read_cb(MasterReadCB);
    masterb.register_write_cb(MasterWriteCB);
}
};

char Data[1024];

typedef struct _TS{
    unsigned int    addr;
    unsigned char   *data;
    unsigned int    datalen;
    BUSCMD          e;
    _TS(unsigned int a, unsigned char *d, unsigned int l, BUSCMD c): addr(a), data(d), datalen(l), e(c){}
}TransStruct;

sc_fifo<TransStruct*> reqs;

bool ReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    //memcpy(data,&Data[addr],datalen);

    TransStruct *t = new TransStruct(addr,data,datalen,CMD_READ);

    if(reqs.nb_write(t))
        return true;
    else
        return false;
}

bool WriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    //memcpy(&Data[addr],data, datalen);

    TransStruct *t = new TransStruct(addr,data,datalen,CMD_WRITE);

    if(reqs.nb_write(t))
        return true;
    else
        return false;
}

SC_MODULE(SLAVE){

ProcessorBus_Slave_nb   slaveb;

sc_in<bool>             clk;

friend bool ReadCB(unsigned int addr, unsigned char *data, unsigned int datalen);

friend bool WriteCB(unsigned int addr, unsigned char *data, unsigned int datalen);

void ProcessReqs()
{
    TransStruct *tr = NULL;
    if(reqs.nb_read(tr)){
        if(tr->e == CMD_READ){
            memcpy(tr->data,&Data[tr->addr],tr->datalen);
        } else if(tr->e == CMD_WRITE){
            memcpy(&Data[tr->addr],tr->data, tr->datalen);
        } else
            slaveb.bus_write(tr->addr,tr->data,tr->datalen,tr->e,true);    

        slaveb.bus_write(tr->addr,tr->data,tr->datalen,tr->e,false);
        
    } else {
        cout << "slave : no requests on bus" << endl;
    }

    delete tr;
}

SC_CTOR(SLAVE): slaveb("Bus_Slave"){

    slaveb.register_Slave_Addr(0,1024);
    slaveb.register_read_cb(ReadCB);
    slaveb.register_write_cb(WriteCB);

    memset(Data,0,1024);

    SC_METHOD(ProcessReqs);
    dont_initialize();
    sensitive << clk.pos();
}

};

int sc_main(int argc, char* argv[]) {

    sc_clock clock ("mem_clock",1,0.5);

    SLAVE s("Peripheral");
    MASTER m("Processor");

    s.clk(clock);

    sc_start();

    return 0;
}