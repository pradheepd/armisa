#include "ProcessorBus.h"
#include "iostream"

using namespace std;

SC_MODULE(MASTER) {

ProcessorBus_Master_b masterb;

void thread() {

    char Data[20];
    strcpy(Data,"Hello World") ;

    int datalen = strlen(Data);

    if(masterb.write(0,(unsigned char *)Data,datalen+1))
        cout << "Write succeeded .." << endl;
    else
        cout << "Write Failed .." << endl;

    if(masterb.read(0,(unsigned char *)Data,datalen+1))
        cout << "Read succeeded .." << endl;
    else
        cout << "Read Failed .." << endl;

    if(strcmp("Hello World",Data)== 0){
        cout << "Data verified .." << endl;
    } else
        cout << "Data verification failed .." << endl;
    
}

SC_CTOR(MASTER): masterb("Bus_Master") {
    SC_THREAD(thread);
}
};

char Data[1024];

bool ReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(data,&Data[addr],datalen);

    return true;
}

bool WriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(&Data[addr],data, datalen);

    return true;
}

SC_MODULE(SLAVE){

ProcessorBus_Slave_b slaveb;

friend bool ReadCB(unsigned int addr, unsigned char *data, unsigned int datalen);

friend bool WriteCB(unsigned int addr, unsigned char *data, unsigned int datalen);

SC_CTOR(SLAVE): slaveb("Bus_Slave"){

    slaveb.register_Slave_Addr(0,1024);
    slaveb.register_read_cb(ReadCB);
    slaveb.register_write_cb(WriteCB);

    memset(Data,0,1024);
}

};

int sc_main(int argc, char* argv[]) {
    
    SLAVE s("Peripheral");
    MASTER m("Processor");

    sc_start();

    return 0;
}