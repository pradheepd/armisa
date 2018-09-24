#include "systemc.h"
#include "fstream"
#include "ThumbDefs.h"

// flash size in kbs
#define FLASH_SIZE  64 * 1024

// sram size in kbs
#define SRAM_SIZE   20 * 1024

//flash start address
#define FLASH_START_ADDR    0x00000000

//sram start address
#define SRAM_START_ADDR     0x20000000

// cpu destination mask
#define DST_MASK_R0         0b0000
#define DST_MASK_R1         0b0001
#define DST_MASK_R2         0b0010
#define DST_MASK_R3         0b0011
#define DST_MASK_R4         0b0100
#define DST_MASK_R5         0b0101
#define DST_MASK_R6         0b0110
#define DST_MASK_R7         0b0111
#define DST_MASK_R8         0b1000
#define DST_MASK_R9         0b1001
#define DST_MASK_R10        0b1010
#define DST_MASK_R11        0b1011
#define DST_MASK_R12        0b1100
#define DST_MASK_SP         0b1101
#define DST_MASK_LR         0b1110
#define DST_MASK_PC         0b1111
#define DST_MASK_INST       0b10000
#define DST_MASK_IMMI       0b10001

#define SHIFT_MASK          0x00000ff0

#define RMREG_MASK          0x0000000f

#define RNREG_MASK          0x000f0000

#define RDREG_MASK          0x0000f000

#define IMMI_MASK           0x000000ff

#define ROT_MASK            0x00000f00

#define BX_MASK             0x0ffffff0

#define BWL_MASK            0x00ffffff

#define MRS_MASK            0x0fff0000

#define MSR_MASK            0x0ffff000

// check macros for NZCV bits in APSR
#define IS_NSET() (((CPSR & 0x80000000 ) == 0x80000000)?true:false)
#define IS_NCLR() (((CPSR & 0x80000000 ) != 0x80000000)?true:false)
#define NSET() ( CPSR = CPSR | 0x80000000 )
#define NCLR() ( CPSR = CPSR & 0x8FFFFFFF )

#define IS_ZSET() (((CPSR & 0x40000000 ) == 0x40000000)?true:false)
#define IS_ZCLR() (((CPSR & 0x40000000 ) != 0x40000000)?true:false)
#define ZSET() ( CPSR = CPSR | 0x40000000 )
#define ZCLR() ( CPSR = CPSR & 0xBFFFFFFF )

#define IS_CSET() (((CPSR & 0x20000000 ) == 0x20000000)?true:false)
#define IS_CCLR() (((CPSR & 0x20000000 ) != 0x20000000)?true:false)
#define CSET() ( CPSR = CPSR | 0x20000000 )
#define CCLR() ( CPSR = CPSR & 0xDFFFFFFF )

#define IS_VSET() (((CPSR & 0x10000000 ) == 0x10000000)?true:false)
#define IS_VCLR() (((CPSR & 0x10000000 ) != 0x10000000)?true:false)
#define VSET() ( CPSR = CPSR | 0x10000000 )
#define VCLR() ( CPSR = CPSR & 0xEFFFFFFF )

//test for immidiate bit
#define IS_IMMI() ((inst & 0x01000000) == 0x01000000? true:false)

//test for s bit
#define IS_SBIT() ((inst & 0x00080000) == 0x01000000? true:false)

// condition bits in the arm instruction
#define COND_EQ 0b0000
#define COND_NE 0b1000
#define COND_CS 0b0100
#define COND_HS 0b0100
#define COND_CC 0b1100
#define COND_LO 0b1100
#define COND_MI 0b0010
#define COND_PL 0b1010
#define COND_VS 0b0110
#define COND_VC 0b1110
#define COND_HI 0b0001
#define COND_LS 0b1001
#define COND_GE 0b0101
#define COND_LT 0b1101
#define COND_GT 0b0011
#define COND_LE 0b1011
#define COND_AL 0b0111

//Data processing instructions
/*
 0000 = AND - Rd:= Op1 AND Op2
 0001 = EOR - Rd:= Op1 EOR Op2
 0010 = SUB - Rd:= Op1 - Op2
 0011 = RSB - Rd:= Op2 - Op1
 0100 = ADD - Rd:= Op1 + Op2
 0101 = ADC - Rd:= Op1 + Op2 + C
 0110 = SBC - Rd:= Op1 - Op2 + C - 1
 0111 = RSC - Rd:= Op2 - Op1 + C - 1
 1000 = TST - set condition codes on Op1 AND Op2
 1001 = TEQ - set condition codes on Op1 EOR Op2
 1010 = CMP - set condition codes on Op1 - Op2
 1011 = CMN - set condition codes on Op1 + Op2 1100 = ORR - Rd:= Op1 OR Op2
 1101 = MOV - Rd:= Op2
 1110 = BIC - Rd:= Op1 AND NOT Op2 1111 = MVN - Rd:= NOT Op2
*/
#define IN_BX  0b000100101111111111110001
#define IN_B   0b1010
#define IN_BL  0b1011
#define IN_SDS_B 0b00010000
#define IN_SDS_W 0b00010100
#define IN_MRS_C 0b000100001111
#define IN_MRS_S 0b000101001111
#define IN_MSR_IC 0b0011001010001111
#define IN_MSR_RC 0b0001001010001111
#define IN_MSR_IS 0b0011011010001111
#define IN_MSR_RS 0b0001011010001111

#define IN_AND 0b0000
#define IN_EOR 0b0001
#define IN_SUB 0b0010
#define IN_RSB 0b0011
#define IN_ADD 0b0100
#define IN_ADC 0b0101
#define IN_SBC 0b0110
#define IN_RSC 0b0111
#define IN_TST 0b1000
#define IN_TEQ 0b1001
#define IN_CMP 0b1010
#define IN_CMN 0b1011
#define IN_ORR 0b1100
#define IN_MOV 0b1101
#define IN_BIC 0b1110
#define IN_MVN 0b1111

typedef enum _DataMode{
    BYT = 0,
    HWORD,
    WORD
}DataMode;

// sram module
SC_MODULE(ARM_SRAM) {
    sc_in<bool>             sclk;

    sc_in<bool>             intr;
    sc_in<bool>             intrw;
    unsigned int            addr;
    unsigned char           data;
    unsigned char           data4bt[4];
    unsigned char           data2bt[2];
    DataMode                mode;
    sc_signal<bool>         intrp;

    bool dataready;
    // sram mem allocated in heap
    void *mem;

    // memory offset
    long offset;

    void memory_access() {

        if(mode == BYT) {

            memcpy((void *)&data, (void *)((unsigned long)mem +offset+ addr), 1 );

        } else if (mode == HWORD) {

            memcpy((void *)&data2bt, (void *)((unsigned long)mem +offset+ addr), 2 );

        } else {

            memcpy((void *)&data4bt, (void *)((unsigned long)mem +offset+ addr), 4 );
        }

        dataready = true;
    }

    void write_access() {

        unsigned char *valuep = (unsigned char *)((unsigned long)mem + offset+ addr);

        if(mode == BYT) {

            *valuep = data;

        } else if(mode == HWORD) {

            *valuep = data2bt[0] ;
            valuep++;
            *valuep = data2bt[1] ;

        } else {

            *valuep = data4bt[0] ;
            valuep++;
            *valuep = data4bt[1] ;
            valuep++;
            *valuep = data4bt[2] ;
            valuep++;
            *valuep = data4bt[3] ;

        }
    }

    void clearProcessorIntr() {
        if(!sclk.read()){
            if(dataready) {
                intrp = true;
                dataready = false;
            } else
                intrp = false;
        } else
            intrp = false;
    }

    SC_CTOR(ARM_SRAM): offset((long)mem - SRAM_START_ADDR), intrp("SR_INTP"), dataready(false){
        mem = malloc ( SRAM_SIZE );

        SC_METHOD(memory_access);
        dont_initialize();
        sensitive << intr.pos();

        SC_METHOD(write_access);
        dont_initialize();
        sensitive << intrw.pos();

        SC_METHOD(clearProcessorIntr);
        dont_initialize();
        sensitive << sclk;
    }
};

// flash module
SC_MODULE(ARM_FLASH) {
    sc_in<bool>             sclk;
    sc_in<bool>             intr;
    sc_in<bool>             intrw;
    unsigned int            addr;
    unsigned char           data;
    unsigned char           data4bt[4];
    unsigned char           data2bt[2];
    DataMode                mode;
    sc_signal<bool>         intrp;

    bool dataready;
    // flash mem allocated in heap
    void *mem;

    // memory offset
    long offset;

    void memory_access() {

        if(mode == BYT) {

            memcpy((void *)&data, (void *)((unsigned long)mem +offset+ addr), 1 );

        } else if (mode == HWORD) {

            memcpy((void *)&data2bt, (void *)((unsigned long)mem +offset+ addr), 2 );

        } else {

            memcpy((void *)&data4bt, (void *)((unsigned long)mem +offset+ addr), 4 );
        }

        dataready = true;
    }

    void write_access() {

        unsigned char *valuep = (unsigned char *)((unsigned long)mem + offset+ addr);

        if(mode == BYT) {

            *valuep = data;

        } else if(mode == HWORD) {

            *valuep = data2bt[0] ;
            valuep++;
            *valuep = data2bt[1] ;

        } else {

            *valuep = data4bt[0] ;
            valuep++;
            *valuep = data4bt[1] ;
            valuep++;
            *valuep = data4bt[2] ;
            valuep++;
            *valuep = data4bt[3] ;

        }
    }

    void clearProcessorIntr() {
        if(!sclk.read()){
            if(dataready) {
                intrp = true;
                dataready = false;
            } else
                intrp = false;
        } else
            intrp = false;
    }

    bool Load_Program(const char *Filename) {
        ifstream program;
        program.open(Filename, ifstream::binary );

        bool ret = false;

        if (program) {
            // get length of file:
            program.seekg (0, program.end);
            int length = program.tellg();
            program.seekg (0, program.beg);

            program.read((char *)mem,length);

            if (program) {
                cout << "whole program read successfully."<< endl;
                ret = true;
            } else {
                cout << "error: only " << program.gcount() << " could be read"<< endl;
            }
    
            program.close();
        } else {
            cout << "error: Opening program file" << endl;
        }
        return ret;
    }

    SC_CTOR(ARM_FLASH): offset((long)mem - FLASH_START_ADDR), intrp("FL_INTP"), dataready(false){
        mem = malloc ( FLASH_SIZE );

        SC_METHOD(memory_access);
        dont_initialize();
        sensitive << intr.pos();

        SC_METHOD(write_access);
        dont_initialize();
        sensitive << intrw.pos();

        SC_METHOD(clearProcessorIntr);
        dont_initialize();
        sensitive << sclk.neg();
    }
};

// sram object
ARM_SRAM sram("SRAM");

// flash object
ARM_FLASH flash("FLASH");

enum SHFT_TYP {
    LL = 0,
    LR,
    AR,
    RR
};

typedef struct _BT_opts{
    bool load ;
    bool pre ;
    bool wrtbck ;
    bool isprevilageuser ;
    bool is_add ;
    
    unsigned int addr;

    _BT_opts():load(false), pre(false), wrtbck(false), isprevilageuser(false),
    is_add(false),addr(0){}
}BT_opts;

typedef enum {
    SVC_MODE = 0,
    IRQ,
    FIQ,
    ABRT,
    Undefined,
    System
}Interrupt_Mode;

// processor module
SC_MODULE(ARM_CORE) {
    sc_in<bool>          sclk;
    //sc_in<sc_uint<28> > instn;
    //sc_in<sc_uint<4> >  cond;

    sc_in<bool >         busintrf;
    sc_in<bool >         busintrs;
    //sc_in<bool >         busintrp;
    //sc_in<sc_uint<32> >  datain;
    //sc_out<sc_uint<32> > dataout;

    sc_signal<bool>      flash_rsignal;
    sc_signal<bool>      sram_rsignal;

    sc_signal<bool>      flash_wsignal;
    sc_signal<bool>      sram_wsignal;

    //instruction cycle counter
    int cycles;
    
    //processor busy flag
    bool processor_busy;
    
    //s bit
    bool s_bit;
    
    //immi bit
    bool immi_bit;

    //thumb mode
    bool b_thumb;

    //halfword transfer bit
    bool halfword_transfer;
    
    //shift register
    unsigned int shift;
    
    //shift type
    SHFT_TYP shft_typ;
    
    //shift amount
    unsigned int shft_amt;
    
    //dest register
    unsigned int rd;
    
    //n register
    unsigned int rn;
    
    //m register
    unsigned int rm;

    //destination mask
    unsigned int Dst_Mask;

    //instruction
    unsigned int inst;
    
    //Rotate register
    unsigned int rot;
    
    //immidiate register
    unsigned int immi;

    //General purpose registers
    unsigned int R[16];

    //Processor Interrupted
    bool        Intruppted;

    //Interrupted mode
    Interrupt_Mode Intr_Mode;


    //Link register when in SVC, IRQ and FIQ
    unsigned int LReg[3];

    //Current Program Status Register
    unsigned int CPSR;

    //Saved Program Status Register
    unsigned int SPSR;

    unsigned int SPSR_svc;

    unsigned int SPSR_irq;

    unsigned int SPSR_fiq;

    //Interrupt mask register
    unsigned int IMR;

    //Control Register
    unsigned int CR;

    //bulktransfer initiated
    bool bulktransfer;

    //bulk transfer opts
    BT_opts Bt_opts;

    sc_fifo<unsigned int> BT_RegList;

    bool Itblock(){
        //implement IT block here
        return true;
    }
    
    unsigned int logic_shift_left(unsigned int reg, unsigned int s){
        
        unsigned int carry_mask = 0x80000000;
        carry_mask = carry_mask >> (s-1);
        
        if(!b_thumb) {
            if(( carry_mask & R[reg]) && s_bit){
                //set the carry flag
                CSET();
            } else
                CCLR();
        } else if(!Itblock()){
            if(carry_mask & R[reg]){
                //set the carry flag
                CSET();
            } else
                CCLR();
        }
        
        return R[reg] << s;
        
    }
    
    unsigned int logic_shift_right(unsigned int reg, unsigned int s){
        
        unsigned int carry_mask = 0x00000001;
        carry_mask = carry_mask << (s-1);
        
        if(!b_thumb) {
            if(( carry_mask & R[reg]) && s_bit){
                //set the carry flag
                CSET();
            } else
                CCLR();
        } else if(!Itblock()){
            if(carry_mask & R[reg]){
                //set the carry flag
                CSET();
            } else
                CCLR();
        }
        
        return R[reg] >> s;
        
    }
    
    unsigned int arith_shift_right(unsigned int reg, unsigned int s){

        unsigned int carry_mask = 0x00000001;
        carry_mask = carry_mask << (s-1);
        
        bool HSbit = ((R[reg] & 0x80000000) !=0)?true:false;
        
        unsigned int ret = logic_shift_right(reg, s);
        
        if(HSbit)
        {
            unsigned int shf_mask = 0x80000000;
            
            for (int i=0; i<s; i++) {
                shf_mask = shf_mask | (shf_mask >> i);
            }
            ret = ret | shf_mask ;
        }

        if(!b_thumb) {
            if(( carry_mask & R[reg]) && s_bit){
                //set the carry flag
                CSET();
            } else
                CCLR();
        } else if(!Itblock()){
            if(carry_mask & R[reg]){
                //set the carry flag
                CSET();
            } else
                CCLR();
        }
        
        return ret;
    }
    
    unsigned int rotate_right(unsigned int reg, unsigned int s){
        
        unsigned int registr = 0;
        unsigned int ret = 0;
        
        if(reg == DST_MASK_IMMI)
            registr = immi;
        else
            registr = R[reg];
        
        for (int i=0; i<s; i++) {
            bool LSBset = registr & 0x00000001;
            
            registr = registr >> 1;
            
            if(LSBset) {
                registr = registr | 0x80000000;
                CSET();
            } else {
                CCLR();
            }
        }
        
        // clear carry if s_bit not set
        if( !s_bit && !b_thumb )
            CCLR();

        if(b_thumb && Itblock())
            CCLR();
        
        return ret;
    }

    void execute_thumb_inst(){
        
        if(TH_FMT_02(inst)){
            //add or subtract immidiate
            unsigned int rd = inst & 0b111 ;
            unsigned int rs = inst & 0b111000 ;

            rs = rs >> 3 ;

            unsigned int rn = inst & 0b111000000 ;

            rn = rn >> 6 ;

            unsigned int result = 0;

            if(inst & 0x0200){
                //subtract
                result = R[rn] - rs ;
            } else {
                //add
                result = R[rn] + rs ;
            }
            
            if(!Itblock()){
                //update flags
                VCLR();
                CCLR();
                    
                if(R[rd]== 0)
                    ZSET();
                else
                    ZCLR();
                    
                if(R[rd] & 0x80000000)
                    NSET();
                else
                    NCLR();
            }

        } else if (TH_FMT_20(inst)) {
            //add or subtract register

            unsigned int rd = inst & 0b111 ;
            unsigned int rs = inst & 0b111000 ;

            rs = rs >> 3 ;

            unsigned int rn = inst & 0b111000000 ;

            rn = rn >> 6 ;

            if(inst & 0x0200){
                //subtract
                R[rd] = R[rn] - R[rs] ;
            } else {
                //add
                R[rd] = R[rn] + R[rs] ;
            }

            if(!Itblock()){
                //update flags
                VCLR();
                CCLR();
                    
                if(R[rd]== 0)
                    ZSET();
                else
                    ZCLR();
                    
                if(R[rd] & 0x80000000)
                    NSET();
                else
                    NCLR();
            }

        } else if (TH_FMT_01(inst)){

            unsigned int off5 = inst & 0b11111000000 ;
            off5 = off5 >> 6 ;

            unsigned int rd = inst & 0b111 ;

            unsigned int rs = inst & 0b111000 ;

            rs = rs >> 3 ;

            unsigned int op = inst & 0x1800 ;

            if( op == 0 && off5 == 0 && !Itblock()){
                //move register
                R[rd] = R[rs] ;

            } else if(op == 0 ) {
                //logic shift left
                R[rd] = logic_shift_left(rs, off5) ;

            } else if(op == 1 && Itblock()) {
                //logic shift right               
                R[rd] = logic_shift_right(rs, off5) ;

            } else if(op == 2 && Itblock()) {
                //arith shift right
                R[rd] = arith_shift_right(rs, off5) ;

            }

            if(!Itblock()){
                //update flags
                VCLR();
                    
                if(R[rd]== 0)
                    ZSET();
                else
                    ZCLR();
                    
                if(R[rd] & 0x80000000)
                    NSET();
                else
                    NCLR();
            }

        } else if (TH_FMT_03(inst)){
            
            unsigned int off8 = inst & 0x0f ;
            unsigned int rdn = inst & 0x70 ;

            rdn = rdn >> 8 ;

            unsigned int op = 0x1800 ;

            switch(op) {
                case 0b00: //move
                R[rdn] = off8 ;

                if(!Itblock()){
                    VCLR();
                    CCLR();
                        
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                        
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                break;
                case 0b01: //compare
                VCLR();
                CCLR();
                if(R[rdn] == off8)
                    ZSET();
                else
                    ZCLR();

                if(R[rdn] < off8)
                    NSET();
                else
                    NCLR();
                break;
                case 0b10: //add
                R[rdn] = R[rdn] + off8;
                if(!Itblock()){
                    VCLR();
                    CCLR();
                    ZCLR();
                    if(R[rdn] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                break;
                case 0b11: //sub
                R[rdn] = R[rdn] - off8;
                if(!Itblock()){
                    VCLR();
                    CCLR();
                    if(R[rdn]== 0)
                        ZSET();
                    else
                        ZCLR();
                        
                    if(R[rdn] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                break;
            }

        } else if (TH_FMT_04(inst)){

            unsigned int m_rd = inst & 0b111;
            unsigned int m_rs = inst & 0b111000;

            m_rs = m_rs >> 3;

            unsigned int m_op = inst & 0b1111000000 ;

            m_op = m_op >> 6;

            switch(m_op) {
                case 0b0000: //and
                    R[m_rd] = R[m_rd] & R[m_rs];
                    if(!Itblock()){
                        VCLR();
                        CCLR();
                        if(R[m_rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                            
                        if(R[m_rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b0001: //eor
                    R[m_rd] = R[m_rd] ^ R[m_rs];
                    if(!Itblock()){
                        VCLR();
                        CCLR();
                        if(R[m_rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                            
                        if(R[m_rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b0010: //lsl
                    R[m_rd] = logic_shift_left(m_rd, R[m_rs]);
                break;
                case 0b0011: //lsr
                    R[m_rd] = logic_shift_right(m_rd, R[m_rs]);
                break;
                case 0b0100: //asr
                    R[m_rd] = arith_shift_right(m_rd, R[m_rs]);
                break;
                case 0b0101: //adc
                    R[m_rd] = R[m_rd] + R[m_rs];

                    if(IS_CSET())
                        R[m_rd] = R[m_rd] + 1;

                    if(!Itblock()){
                        VCLR();
                        CCLR();
                        if(R[m_rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                            
                        if(R[m_rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b0110: //sbc
                    R[m_rd] = R[m_rd] - R[m_rs];
                    
                    if(IS_CSET())
                        R[m_rd] = R[m_rd] - 1;

                    if(!Itblock()){
                        VCLR();
                        CCLR();
                        if(R[m_rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                            
                        if(R[m_rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b0111: //ror
                    R[m_rd] = rotate_right(m_rd, R[m_rs]);
                break;
                case 0b1000: //tst
                {
                    unsigned int m_res = R[m_rd] & R[m_rs];
                    VCLR();
                    CCLR();
                    if(m_res== 0)
                        ZSET();
                    else
                        ZCLR();
                            
                    if(m_res & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }  
                break;
                case 0b1001: //rsb
                    R[m_rd] = R[m_rs] - R[m_rd];
                    if(!Itblock()){
                        VCLR();
                        CCLR();
                        if(R[m_rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                            
                        if(R[m_rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b1010: //cmp
                    VCLR();
                    CCLR();
                    if(R[m_rd] == R[m_rs])
                        ZSET();
                    else
                        ZCLR();

                    if(R[m_rd] <  R[m_rs])
                        NSET();
                    else
                        NCLR();
                break;
                case 0b1011: //cmn
                    VCLR();
                    CCLR();
                    if(R[m_rd] == R[m_rs])
                        ZSET();
                    else
                        ZCLR();

                    if(R[m_rd] >  R[m_rs])
                        NSET();
                    else
                        NCLR();
                break;
                case 0b1100: //orr
                    R[m_rd] = R[m_rd] | R[m_rs];
                
                    if(!Itblock()) {
                        VCLR();
                        CCLR();
                        if(R[rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                        
                        if(R[rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b1101: //mul
                {
                    unsigned long m_res = R[m_rd] * R[m_rs];
                    m_res = m_res & 0x00000000ffffffff;

                    R[m_rd] = m_res;

                    if(!Itblock()) {
                        VCLR();
                        CCLR();
                        if(R[rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                        
                        if(R[rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                }
                break;
                case 0b1110: //bic
                    R[m_rd] = R[m_rd] & !R[m_rs] ;
                    if(!Itblock()) {
                        VCLR();
                        CCLR();
                        if(R[rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                        
                        if(R[rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b1111: //mvn
                    R[m_rd] = ~R[m_rs] ;
                    if(!Itblock()) {
                        VCLR();
                        CCLR();
                        if(R[rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                        
                        if(R[rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
            }

        } else if (TH_FMT_05(inst)){

            unsigned int m_rd = inst & 0b111 ;
            unsigned int m_rs = inst & 0b1111000 ;

            m_rs = m_rs >> 3 ;

            if(inst & 0x80)
                m_rd = m_rd | 0x08 ;

            unsigned int m_op = inst & 0x30 ;

            switch(m_op){
                case 0b00: //add
                    R[m_rd] = R[m_rd] + R[m_rs] ;
                    if(!Itblock()) {
                        VCLR();
                        CCLR();
                        if(R[rd]== 0)
                            ZSET();
                        else
                            ZCLR();
                        
                        if(R[rd] & 0x80000000)
                            NSET();
                        else
                            NCLR();
                    }
                break;
                case 0b01: //cmp
                    VCLR();
                    CCLR();
                    if(R[m_rd] == R[m_rs])
                        ZSET();
                    else
                        ZCLR();

                    if(R[m_rd] <  R[m_rs])
                        NSET();
                    else
                        NCLR();
                break;
                case 0b10: //mov
                    R[m_rd] = R[m_rs] ;
                    VCLR();
                    CCLR();
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                        
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                break;
                case 0b11: // B & BL
                    if(Itblock()){
                        if(m_rs & 0x8)
                            R[14] = R[15];
                            
                        R[15] = R[m_rs];
                    }
                break;
            }

        } else if (TH_FMT_06(inst)){

        } else if (TH_FMT_07(inst)){

        } else if (TH_FMT_08(inst)){

        } else if (TH_FMT_09(inst)){

        } else if (TH_FMT_10(inst)){

        } else if (TH_FMT_11(inst)){

        } else if (TH_FMT_11(inst)){

        } else if (TH_FMT_12(inst)){

        } else if (TH_FMT_13(inst)){

        } else if (TH_FMT_14(inst)){

        } else if (TH_FMT_15(inst)){

        } else if (TH_FMT_16(inst)){

        } else if (TH_FMT_17(inst)){

        } else if (TH_FMT_18(inst)){

        } else if (TH_FMT_19(inst)){

        } else {
            processor_busy = false;
            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();
        }

    }

    void execute_inst () {
        //clear flags set by previous inst
        //VCLR();
        //CCLR();
        //ZCLR();
        //NCLR();

        //check for bulktransfer is true
        if(bulktransfer && cycles >= 1) {
            unsigned int val;

            if(BT_RegList.nb_read(val)){
                if(Bt_opts.pre){
                    if(Bt_opts.is_add)
                        Bt_opts.addr = Bt_opts.addr + 4 ;
                    else
                        Bt_opts.addr = Bt_opts.addr - 4 ;
                }

                if( Bt_opts.addr < SRAM_START_ADDR){
                    flash.addr = Bt_opts.addr;
                    if(Bt_opts.load) {
                        Dst_Mask = val ;
                        
                        flash.mode = WORD;

                        flash_rsignal = true;
                    } else {
                        flash.data = R[val] ;

                        flash.mode = WORD;

                        flash_wsignal = true;
                    }
                } else {
                    sram.addr = Bt_opts.addr;
                    if(Bt_opts.load) {
                        Dst_Mask = val ;
                        
                        sram.mode = WORD;

                        sram_rsignal = true;
                    } else {
                        sram.data = R[val] ;

                        sram.mode = WORD;

                        sram_wsignal = true;
                    }
                }

                if(!Bt_opts.pre) {
                    if(Bt_opts.is_add)
                        Bt_opts.addr = R[rn] + 4;
                    else
                        Bt_opts.addr = R[rn] - 4;
                } else if (Bt_opts.wrtbck) {
                    R[rn] = Bt_opts.addr;
                } else {

                }

            } else {
                cycles = 0;
                bulktransfer = false;
                processor_busy = false;

                //call constructor to set all values to zero
                BT_opts Bt_opts();

                //clear flags set by previous inst
                VCLR();
                CCLR();
                ZCLR();
                NCLR();
            }
            return ;
        }

        //check for halfword instrn cycle 1
        if(halfword_transfer){

            unsigned int SH = inst & 0x00000030 ;
            
            //sign extend byte
            if( SH == 2 && (R[Dst_Mask] & 0x80) ){
                R[Dst_Mask] = R[Dst_Mask] | 0xffffff00 ;
            }

            //sign extend hword
            if( SH == 3 && (R[Dst_Mask] & 0x8000) ){
                R[Dst_Mask] = R[Dst_Mask] | 0xffff0000 ;
            }

            halfword_transfer = false;
            processor_busy = false;

            cycles = 0;

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        //check for single data transfer instrn cycle 1
        unsigned int check_inst = inst & 0x0A000000 ;
        check_inst = check_inst >> 26 ;

        if (check_inst == 0b01 && cycles == 1) {

            processor_busy = false;
            cycles = 0;

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        //check if bx instruction
        bool mult_flag = false;
        unsigned int mult_op = 0;
        check_inst = inst & BX_MASK ;
        check_inst = check_inst >> 4;

        if(check_inst == IN_BX)
        {
            // get the rm register
            rm = inst & RMREG_MASK;

            // check if to change to thumb mode
            if(R[rm] & 0x01){
                b_thumb = true;
            }

            //Move the value of register to PC
            R[15] = R[rm];

            processor_busy = false;
            //Clear all the flags
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        //check for B/BL instruction
        check_inst = check_inst >>20 ;
        
        // B/BL instruction
        if(IN_B == check_inst || IN_BL == check_inst) {

            //get the offset
            unsigned int offset = inst & BWL_MASK ;

            //left shift 2 bits
            offset = offset << 2;

            //sign extend to 32 bits
            if( offset & 0x00200000) {
                offset = offset | 0xffa00000 ; 
            }

            if(IN_BL == check_inst)
                R[14] = R[15];

            R[15] = R[15] + (int)offset ;

            processor_busy = false;
            //Clear all the flags
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        // check for single data swap
        check_inst = inst & 0x0ff00000 ;

        check_inst = check_inst >> 20 ;

        if( check_inst = IN_SDS_B || check_inst == IN_SDS_W ) {
            
            unsigned int temp_reg = inst & 0x00000ff0 ;

            temp_reg = temp_reg >> 4 ;

            if(temp_reg == 0b00001001)
            {

                temp_reg = inst & 0x000ff00f ;

                rm = temp_reg & 0x0000000f ;

                rd = temp_reg >> 12 ;

                rd = temp_reg & 0x0000000f ;

                rn = temp_reg >> 4 ;

            }
        }

        // check for software interrupt
        check_inst = inst & 0x0f000000 ;

        check_inst = check_inst >> 24 ;

        if (check_inst == 0b1111) {

            SPSR_svc = CPSR;

            //R[14] = R [15];

            Intruppted = true ;

            Intr_Mode = SVC_MODE ;

            LReg[SVC_MODE] = R[15];

            R[15] = 0x08 ;

            processor_busy = false ;
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return ;
        }

        // check for Bulk transfer instrn
        check_inst = inst & 0x0e000000 ;

        check_inst = check_inst >> 25 ;

        if (check_inst == 0b100) {            

            bulktransfer = true;

            unsigned int opts = inst & 0x01f00000 ;

            opts = opts >> 20 ;

            //check if load or store op
            if(opts & 0x01)
                Bt_opts.load = true;
            
            //check if pre or post
            if(opts & 0x100)
                Bt_opts.pre = true;

            //check if up / down
            if(opts & 0x80)
                Bt_opts.is_add = true;

            //check if write back is true
            if(opts & 0x02)
                Bt_opts.wrtbck = true;

            //check if PSR or force user
            if(opts & 0x04)
                Bt_opts.isprevilageuser = true;

            
            rn = inst & 0x000f0000 ;

            rn = rn >> 16 ;

            unsigned int regs = inst & 0x0000ffff ;
            unsigned int i = 0;
            while(regs != 0){
                
                if(regs & 0x01)
                    BT_RegList.write(i);

                i++;

                regs = regs >> 1;
            }
            
            if(BT_RegList.num_available() == 0) {
                //clear flags set by previous inst
                VCLR();
                CCLR();
                ZCLR();
                NCLR();

                processor_busy = false ;
                bulktransfer = false;

                return;
            }

            if(Bt_opts.pre) {
                if(Bt_opts.is_add)
                    Bt_opts.addr = R[rn] + 4;
                else
                    Bt_opts.addr = R[rn] - 4;
            }
                
            if( Bt_opts.addr < SRAM_START_ADDR){
                flash.addr = Bt_opts.addr;
                if(Bt_opts.load) {
                    Dst_Mask = BT_RegList.read() ;
                    
                    flash.mode = WORD;

                    flash_rsignal = true;
                } else {
                    flash.data = R[BT_RegList.read()] ;

                    flash.mode = WORD;

                    flash_wsignal = true;
                }
            } else {
                sram.addr = Bt_opts.addr;
                if(Bt_opts.load) {
                    Dst_Mask = BT_RegList.read() ;
                    
                    sram.mode = WORD;

                    sram_rsignal = true;
                } else {
                    sram.data = R[BT_RegList.read()] ;

                    sram.mode = WORD;

                    sram_wsignal = true;
                }
            }

            if(!Bt_opts.pre) {
                if(Bt_opts.is_add)
                    Bt_opts.addr = R[rn] + 4;
                else
                    Bt_opts.addr = R[rn] - 4;
            } else if (Bt_opts.wrtbck) {
                R[rn] = Bt_opts.addr;
            } else {

            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        // check if MRS instruction
        check_inst = inst & MRS_MASK ;

        check_inst = check_inst >> 16 ;
        if ( check_inst == IN_MRS_C || check_inst == IN_MRS_S ) {

            // get the rd register
            rd = inst & RDREG_MASK;
            
            // right shift by 12 bits
            rd = rd >> 12;

            if(check_inst == IN_MRS_C) {
                R[rd] == CPSR ;
            } else {
                R[rd] == SPSR ;
            }

            processor_busy = false;
            //Clear all the flags
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        // check if MSR instruction
        check_inst = inst & MSR_MASK ;

        check_inst = check_inst >> 12 ;

        if(check_inst == IN_MSR_IC 
        || check_inst == IN_MSR_IS 
        || check_inst == IN_MSR_RC 
        || check_inst == IN_MSR_RS) {

            if(check_inst == IN_MSR_RC || check_inst == IN_MSR_RS) {
                // get the rm register
                rm = inst & RMREG_MASK;

                if (check_inst == IN_MSR_RC)
                    CPSR = R[rm] ;
                else
                    SPSR = R[rm] ;
            } else {
                unsigned int val;
                // get the rotate register
                rot = inst & ROT_MASK;
                
                // right shift by 8 bits;
                rot = rot >> 8;
                
                // get the immediate value
                immi = inst & IMMI_MASK;

                if(rot == 0){
                    val = immi;
                } else {
                    val = rotate_right(DST_MASK_IMMI, rot *2 );
                }

                if (check_inst == IN_MSR_IC)
                    CPSR = val ;
                else
                    SPSR = val ;
            }

            processor_busy = false;
            //Clear all the flags
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        // get the immi flag
        immi_bit = IS_IMMI();
        
        // get the s flag
        s_bit = IS_SBIT();
        
        if(!immi_bit) {
        
            // get the shift register
            shift = inst & SHIFT_MASK;
            
            // right shift by 4 bits
            shift = shift >> 4;
            
            // get the rm register
            rm = inst & RMREG_MASK;
            
            // get the shift type
            unsigned int typ = shift & 0x00000006;
            typ = typ >>1;
            
            // set the shift type
            shft_typ = (SHFT_TYP)typ;
            
            if((shift & 0x00000001) && ((shift & 0x00000008) == 0)){
                
                unsigned int shift_rg = shift & 0x000000f0;
                
                shift_rg = shift_rg >> 4;
                
                shft_amt = R[shift_rg] & 0x000000ff;
                
            } else if ((shift & 0x00000001) && (shift & 0x00000008)
              && ((shift & 0x00000002) == 0)
              && ((shift & 0x00000004) == 0)){
                mult_flag = true;
                shft_amt = 0;
                mult_op = shift & 0xff00 ;
                mult_op = mult_op >> 4 ;
                mult_op = R[mult_op] ;

            }  else if ((shift & 0x00000001) && (shift & 0x00000008)
              && ((shift & 0x00000002)
              || (shift & 0x00000004))){
                //Halfword data transfer
                halfword_transfer = true;
            } else {
                
                shft_amt = shift >> 3;
            }
            
        } else {
            
            // get the rotate register
            rot = inst & ROT_MASK;
            
            // right shift by 8 bits;
            rot = rot >> 8;
            
            // get the immediate value
            immi = inst & IMMI_MASK;
        }
        
        // get the rn register
        rn = inst & RNREG_MASK;
        
        // right shift by 16 bits
        rn = rn >> 16;
        
        // get the rd register
        rd = inst & RDREG_MASK;
        
        // right shift by 12 bits
        rd = rd >> 12;
        
        // input mask to get only the opcode , sbit and immi bit
        unsigned int op = inst & 0x00f00000;

        // right shift the value by 20 bits to get 4 bit opcode
        op = op >> 20 ;
        
        unsigned int operand2 = 0;
        //get operand 2
        if(!immi_bit) {
            if(shft_amt !=0) {
                switch (shft_typ){
                    case LL:
                        operand2 = logic_shift_left(rm, shft_amt);
                        break;
                    case LR:
                        operand2 = logic_shift_right(rm, shft_amt);
                        break;
                    case AR:
                        operand2 = arith_shift_right(rm, shft_amt);
                        break;
                    case RR:
                        operand2 = rotate_right(rm, shft_amt);
                        break;
                }
            }
        } else {
            if(rot == 0){
                operand2 = immi;
            } else {
                operand2 = rotate_right(DST_MASK_IMMI, rot *2 );
            }
        }

        //check for single data transfer instrn
        check_inst = inst & 0x0A000000 ;
        check_inst = check_inst >> 26 ;

        if (check_inst == 0b01) {
            bool load = false;
            bool pre = false;
            bool wrtbck = false;
            bool isbyte = false;
            bool is_add = false;

            //check if load or store op
            if(s_bit)
                load = true;
            
            //check if pre or post
            if(op & 0x80)
                pre = true;

            //check if up / down
            if(op & 0x40)
                is_add = true;

            //check if write back is true
            if(op & 0x01)
                wrtbck = true;

            //check if byte or word
            if(op & 0x02)
                isbyte = true;

            
            int addr = R[rn] ;
                
            if(pre) {
                if(is_add)
                    addr = R[rn] + operand2;
                else
                    addr = R[rn] - operand2;
            }
                
            if( addr < SRAM_START_ADDR){
                flash.addr = addr;
                if(load) {
                    Dst_Mask = rd ;
                    
                    if(isbyte)
                        flash.mode = BYT;
                    else
                        flash.mode = WORD;

                    flash_rsignal = true;
                } else {
                    flash.data = R[rd] ;

                    if(isbyte)
                        flash.mode = BYT;
                    else
                        flash.mode = WORD;

                    flash_wsignal = true;
                    processor_busy = false;
                }
            } else {
                sram.addr = addr;
                if(load) {
                    Dst_Mask = rd ;
                    
                    if(isbyte)
                        sram.mode = BYT;
                    else
                        sram.mode = WORD;

                    sram_rsignal = true;
                } else {
                    sram.data = R[rd] ;

                    if(isbyte)
                        sram.mode = BYT;
                    else
                        sram.mode = WORD;

                    sram_wsignal = true;
                    processor_busy = false;
                }
            }

            if(!pre) {
                if(is_add)
                    addr = R[rn] + operand2;
                else
                    addr = R[rn] - operand2;
            } else if (wrtbck) {
                R[rn] = addr;
            } else {

            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }
        // check if halfword instrn
        if(halfword_transfer) {
            bool load = false;
            bool pre = false;
            bool wrtbck = false;
            bool is_add = false;
            
            unsigned int SH = inst & 0x00000030;

            SH = SH >> 4 ;

            unsigned int offs = 0;

            //check if load or store op
            if(s_bit)
                load = true;
            
            //check if pre or post
            if(op & 0x80)
                pre = true;

            //check if up / down
            if(op & 0x40)
                is_add = true;

            //check if write back is true
            if(op & 0x01)
                wrtbck = true;

            //check if immidiate or register
            if(op & 0x02) {

                offs = shift & 0xf0 ;

                offs = shift | rm ;
            } else {
                offs = R[rm] ;
            }

            
            unsigned int addr = 0 ;
                
            if(pre) {
                if(is_add)
                    addr = R[rn] + offs;
                else
                    addr = R[rn] - offs;
            }
                
            if( addr < SRAM_START_ADDR){
                flash.addr = addr;
                if(load) {
                    Dst_Mask = rd ;
                    
                    if(SH == 1 || SH == 3){
                        flash.mode = HWORD ;
                    } else {
                        flash.mode = BYT ;
                    }

                    flash_rsignal = true;
                } else {
                    flash.data = R[rd] ;

                    if(SH == 1 || SH == 3){
                        flash.mode = HWORD ;
                    } else {
                        flash.mode = BYT ;
                    }

                    flash_wsignal = true;
                    processor_busy = false;
                    halfword_transfer = false;
                }
            } else {
                sram.addr = addr;
                if(load) {
                    Dst_Mask = rd ;
                    
                    if(SH == 1 || SH == 3){
                        sram.mode = HWORD ;
                    } else {
                        sram.mode = BYT ;
                    }

                    sram_rsignal = true;
                } else {
                    sram.data = R[rd] ;

                    if(SH == 1 || SH == 3){
                        sram.mode = HWORD ;
                    } else {
                        sram.mode = BYT ;
                    }

                    sram_wsignal = true;
                    processor_busy = false;
                    halfword_transfer = false;
                }
            }

            if(!pre) {
                if(is_add)
                    addr = R[rn] + operand2;
                else
                    addr = R[rn] - operand2;
            } else if (wrtbck) {
                R[rn] = addr;
            } else {

            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }

        unsigned int test = 0;
        
        switch (op) {
            case IN_AND:
            {
                if(!mult_flag)
                    R[rd] = R[rn] & operand2;
                else {
                    unsigned long result = R[rm] * mult_op ;
                    R[rn] = ((unsigned int *)result)[0];
                }
                
                if(s_bit) {
                    VCLR();
                    
                    if(mult_flag) {
                        CCLR();
                    }
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                
                processor_busy = false;
                break;
            }
            case IN_EOR:
            {
                if(!mult_flag)
                    R[rd] = R[rn] ^ operand2;
                else {
                    unsigned long result = (R[rm] * mult_op) + R[rd];
                    R[rn] = ((unsigned int *)result)[0] ;
                }
                if(s_bit) {
                    VCLR();

                    if(mult_flag) {
                        CCLR();
                    }

                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_SUB:
            {
                unsigned long result = 0;
                if(!mult_flag){
                    R[rd] = R[rn] - operand2;
                } else {
                    unsigned int A[2];
                    A[0] = R[rd];
                    A[1] = R[rn];
                    result = R[rm] * mult_op + *((unsigned long *)A);
                    R[rd] = ((unsigned int *)result)[0] ;
                    R[rn] = ((unsigned int *)result)[1] ;
                }
                if(s_bit) {
                    VCLR();

                    if(mult_flag)
                        CCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_RSB:
            {
                R[rd] = operand2 - R[rn];
                
                if(s_bit) {
                    VCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_ADD:
            {
                unsigned long result = 0;
                if(!mult_flag){
                    R[rd] = R[rn] + operand2;
                } else {
                    result = R[rm] * mult_op ;
                    R[rd] = ((unsigned int *)result)[0] ;
                    R[rn] = ((unsigned int *)result)[1] ;
                }

                if(s_bit) {
                    VCLR();

                    if(mult_flag)
                        CCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_ADC:
            {
                R[rd] = R[rn] + operand2;
                
                if(IS_CSET())
                    R[rd] = R[rd] +1 ;

                if(s_bit) {
                    VCLR();
                    CCLR();

                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_SBC:
            {                
                long result = 0;
                if(!mult_flag){
                    R[rd] = R[rn] - operand2;
                } else {
                    result = (int)R[rm] * (int)mult_op ;
                    R[rd] = ((unsigned int *)result)[0] ;
                    R[rn] = ((unsigned int *)result)[1] ;
                }
                if(IS_CSET())
                    R[rd] = R[rd] - 1;
                
                if(s_bit) {
                    VCLR();                    
                    CCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_RSC:
            {
                long result = 0;
                if(!mult_flag){
                    R[rd] = operand2 - R[rn];
                } else {
                    unsigned int A[2];
                    A[0] = R[rd];
                    A[1] = R[rn];
                    result = R[rm] * mult_op + *((long *)A);
                    result = (int)R[rm] * (int)mult_op ;
                    R[rd] = ((unsigned int *)result)[0] ;
                    R[rn] = ((unsigned int *)result)[1] ;
                }
                if(IS_CSET())
                    R[rd] = R[rd] - 1;
                
                if(s_bit) {
                    VCLR();

                    if(mult_flag)
                        CCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_TST:
            {
                test = R[rn] & operand2;
                
                if(s_bit) {
                    VCLR();
                    
                    if(test== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(test & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                
                processor_busy = false;
                break;
            }
            case IN_TEQ:
            {
                test = R[rn] ^ operand2;
                
                if(s_bit) {
                    VCLR();
                    
                    if(test== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(test & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_CMP:
            {
                test = R[rn] - operand2;
                
                if(s_bit) {
                    VCLR();
                    
                    if(test== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(test & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_CMN:
            {
                test = R[rn] + operand2;
                
                if(s_bit) {
                    VCLR();
                    
                    if(test == 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(test & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_ORR:
            {
                R[rd] = R[rn] | operand2;
                
                if(s_bit) {
                    VCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_MOV:
            {
                R[rd] = operand2;

                if(s_bit && Intruppted && rd == 15) {
                    R[15] = LReg[Intr_Mode] ;
                    Intruppted = false;
                }
                
                if(s_bit) {
                    VCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_MVN:
            {
                R[rd] = ~operand2;
                if(s_bit) {
                    VCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }
            case IN_BIC:
            {
                R[rd] = R[rn] & (!operand2);
                
                if(s_bit) {
                    VCLR();
                    
                    if(R[rd]== 0)
                        ZSET();
                    else
                        ZCLR();
                    
                    if(R[rd] & 0x80000000)
                        NSET();
                    else
                        NCLR();
                }
                processor_busy = false;
                break;
            }   
            default:
                processor_busy = false;
                break;
        };
    }

    void process_condition() {
        if(b_thumb == false)
        {
            // get the cond bits from 28 to 31
            unsigned int cond = inst & 0xf0000000;
            
            // get only M S 4 bits
            cond = cond >> 28;

            switch ( cond ){
                case COND_EQ:
                    if(IS_ZSET())
                        execute_inst();
                    break;
                case COND_NE:
                    if(IS_ZCLR())
                        execute_inst();
                    break;
                case COND_CS:
                    if(IS_CSET())
                        execute_inst();
                    break;
                case COND_CC:
                    if(IS_CCLR())
                        execute_inst();
                    break;
                case COND_MI:
                    if(IS_NSET())
                        execute_inst();
                    break;
                case COND_PL:
                    if(IS_NCLR())
                        execute_inst();
                    break;
                case COND_VS:
                    if(IS_VSET())
                        execute_inst();
                    break;
                case COND_VC:
                    if(IS_VCLR())
                        execute_inst();
                    break;
                case COND_HI:
                    if(IS_CSET() && IS_ZCLR())
                        execute_inst();
                    break;
                case COND_LS:
                    if(IS_CCLR() && IS_ZSET())
                        execute_inst();
                    break;
                case COND_GE:
                    if((IS_NSET() && IS_VSET()) || (IS_NCLR() && IS_VCLR()))
                        execute_inst();
                    break;
                case COND_LT:
                    if((IS_NSET() && IS_VCLR()) || (IS_NCLR() && IS_VSET()))
                        execute_inst();
                    break;
                case COND_GT:
                    if(IS_ZCLR() && ((IS_NSET() && IS_VSET()) || (IS_NCLR() && IS_VCLR())))
                        execute_inst();
                    break;
                case COND_LE:
                    if(IS_ZSET() && ((IS_NSET() && IS_VCLR()) || (IS_NCLR() && IS_VSET())))
                        execute_inst();
                    break;
                case COND_AL:
                default:
                    execute_inst();
                    break;
            };
        }
        if ( cycles != 0 && b_thumb == false)
            execute_inst();
    }

    void process2(unsigned int val) {
        if(DST_MASK_INST & Dst_Mask){
            inst = val;

            process_condition();
            // increment program counter
            R[15] = R[15] + 1;
        } else {
            R[Dst_Mask] = val;
            cycles++ ;
            process_condition();
        }
    }

    void process2f() {

        unsigned int val = 0;

        if(flash.mode == BYT) {

            val = flash.data ;
            val = val & 0x000000ff ;

        } else if (flash.mode = HWORD) {
        
            val = *((unsigned int *)flash.data2bt) ;
            val = val & 0x0000ffff ;

        } else {

            val = *((unsigned int *)flash.data4bt) ;
        }
        //flash_rsignal = false;
        process2(val);        
    }

    void process2s() {

        unsigned int val = 0;
        
        if(sram.mode == BYT) {

            val = sram.data ;
            val = val & 0x000000ff ;

        } else if (sram.mode = HWORD) {
        
            val = *((unsigned int *)sram.data2bt) ;
            val = val & 0x0000ffff ;

        } else {
            
            val = *((unsigned int *)sram.data4bt) ;
        }
        process2(val) ;        
    }

    void process() {

        if(sclk.read() && !processor_busy)
        {
            if(R[15] < SRAM_START_ADDR){
                flash.addr = R[15];
                flash.mode = WORD;
                flash_rsignal = true;
                Dst_Mask = DST_MASK_INST;
            } else {
                sram.addr = R[15];
                sram.mode = WORD;
                sram_rsignal = true;
                Dst_Mask = DST_MASK_INST;
            }
            processor_busy = true;
        } else {
            flash_rsignal = false;
            flash_wsignal = false;
            sram_rsignal = false;
            sram_wsignal = false;
        }
    }

    SC_CTOR(ARM_CORE):  immi(0), processor_busy(false), rot(0), b_thumb(false), cycles(0),
     s_bit(false), immi_bit(false), shift(0), rm(0), rn(0), rd(0), CPSR(0),SPSR(0),
     bulktransfer(false), Intruppted(false), Intr_Mode(SVC_MODE),
     flash_rsignal("FL_RS"),
     sram_rsignal("SR_RS"),
     flash_wsignal("FL_WS"),
     sram_wsignal("SR_WS") 
     {

        sc_fifo<unsigned int> BT_RegList(16);
        memset (R, 0, sizeof(unsigned int) * 15);
        R[15] = 0x04;

        flash.intr(flash_rsignal);
        sram.intr(sram_rsignal);

        flash.intrw(flash_wsignal);
        sram.intrw(sram_wsignal);

        //Set processor state to take instruction
        Dst_Mask = DST_MASK_INST;

        SC_METHOD(process);
         dont_initialize();
        sensitive << sclk;

        SC_METHOD(process2f);
         dont_initialize();
        sensitive << busintrf.pos();

        SC_METHOD(process2s);
         dont_initialize();
        sensitive << busintrs.pos();
    }
};


int sc_main (int argc, char* argv[]) {

    sc_clock clock ("system_clock",1,0.5);

    /*if(argc < 2) {
        cout << "Program bin file not provided" << endl;
        exit (1);
    } else {
        if(!flash.Load_Program(argv[1]))
            exit(1);
    }*/

    ARM_CORE processor("PROCESSOR");
    processor.sclk(clock);
    processor.busintrf(flash.intrp);
    processor.busintrs(sram.intrp);

    flash.sclk(clock);
    sram.sclk(clock);

    sc_start();

}
