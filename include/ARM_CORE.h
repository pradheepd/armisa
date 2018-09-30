#include "systemc.h"
#include "ProcessorBus.h"

// cpu destination mask
/*#define DST_MASK_R0         0b0000
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
#define DST_MASK_INST       0b10000*/
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

enum SHFT_TYP {
    LL = 0,
    LR,
    AR,
    RR
};

/*typedef struct _BT_opts{
    bool load ;
    bool pre ;
    bool wrtbck ;
    bool isprevilageuser ;
    bool is_add ;
    
    unsigned int addr;

    _BT_opts():load(false), pre(false), wrtbck(false), isprevilageuser(false),
    is_add(false),addr(0){}
}BT_opts;*/

typedef enum {
    SVC_MODE = 0,
    IRQ,
    FIQ,
    ABRT,
    Undefined,
    System
}Interrupt_Mode;

class ARM_CORE : public sc_module
{
public:
    //system clock
    sc_in<bool>          sclk;
private:
    //s bit
    bool                 s_bit;
    
    //immi bit
    bool                 immi_bit;

    //thumb mode
    bool                 b_thumb;

    //Bus Master Mode
    ProcessorBus_Master_b bus;

    //halfword transfer bit
    bool halfword_transfer;
    
    //shift register
    unsigned int shift;
    
    //shift type
    SHFT_TYP     shft_typ;
    
    //shift amount
    unsigned int shft_amt;
    
    //dest register
    unsigned int rd;
    
    //n register
    unsigned int rn;
    
    //m register
    unsigned int rm;

    //destination mask
    //unsigned int Dst_Mask;

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
   // bool bulktransfer;

    //bulk transfer opts
    //BT_opts Bt_opts;

    //sc_fifo<unsigned int> BT_RegList;

    //Returns if IT block is active in thumb state
    bool Itblock();

    //Logic shift left and update carry flags in ARM and Thumb mode
    unsigned int logic_shift_left(unsigned int reg, unsigned int s);

    //Logic shift right and update carry flags in ARM and Thumb mode
    unsigned int logic_shift_right(unsigned int reg, unsigned int s);

    //Arith shift right and update carry flags in ARM and Thumb mode
    unsigned int arith_shift_right(unsigned int reg, unsigned int s);

    //Rotate over right and update carry flags in ARM and Thumb mode
    unsigned int rotate_right(unsigned int reg, unsigned int s);

    //Execute the ARM instruction
    void execute_inst();

    //Execute the Thumb instruction
    void execute_thumb_inst();

    //Process initial condition ARM state
    void process_condition();

    //Main Process thread
    void process();

public:
    //Constructor
    ARM_CORE(const char* name);

};
