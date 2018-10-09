#include "ARM_CORE.h"
#include "ThumbDefs.h"
#include "PeripheralDefs.h"

    bool ARM_CORE::CheckCondCodes(unsigned int code){
        bool m_ret = false;

        switch ( code ){
                case COND_EQ:
                    if(IS_ZSET())
                        m_ret = true;
                    break;
                case COND_NE:
                    if(IS_ZCLR())
                        m_ret = true;
                    break;
                case COND_CS:
                    if(IS_CSET())
                        m_ret = true;
                    break;
                case COND_CC:
                    if(IS_CCLR())
                        m_ret = true;
                    break;
                case COND_MI:
                    if(IS_NSET())
                        m_ret = true;
                    break;
                case COND_PL:
                    if(IS_NCLR())
                        m_ret = true;
                    break;
                case COND_VS:
                    if(IS_VSET())
                        m_ret = true;
                    break;
                case COND_VC:
                    if(IS_VCLR())
                        m_ret = true;
                    break;
                case COND_HI:
                    if(IS_CSET() && IS_ZCLR())
                        m_ret = true;
                    break;
                case COND_LS:
                    if(IS_CCLR() && IS_ZSET())
                        m_ret = true;
                    break;
                case COND_GE:
                    if((IS_NSET() && IS_VSET()) || (IS_NCLR() && IS_VCLR()))
                        m_ret = true;
                    break;
                case COND_LT:
                    if((IS_NSET() && IS_VCLR()) || (IS_NCLR() && IS_VSET()))
                        m_ret = true;
                    break;
                case COND_GT:
                    if(IS_ZCLR() && ((IS_NSET() && IS_VSET()) || (IS_NCLR() && IS_VCLR())))
                        m_ret = true;
                    break;
                case COND_LE:
                    if(IS_ZSET() && ((IS_NSET() && IS_VCLR()) || (IS_NCLR() && IS_VSET())))
                        m_ret = true;
                    break;
                case COND_AL:
                default:
                    m_ret = true;
                    break;
        }

        return m_ret;
    }

    bool ARM_CORE::Itblock(){

        if(ITMask == 0 || ITMask == 0x8)
        {
            IT_cond_base = 0;
            ITMask = 0;
            return false;
        }

        ITMask = ITMask << 1 ;

        CPSR = CPSR | ((ITMask | 0x03) << 25) ;
        CPSR = CPSR | ((ITMask >> 2) << 10) ;

        if(CheckCondCodes(IT_cond_base) == ((ITMask & 0x8) != 0x0?true:false)){
            return true;
        } else {
            return false;
        }
    }

    void ARM_CORE::execute_thumb_inst(){
        
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
                
                if(Intruppted == true && rd == 15 && rs == 14){ // return from exception
                    R[15] = LReg[Intr_Mode];
                    Intruppted = false;

                    CPSR = SPSR_exc[Intr_Mode] ;
                }
                
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
                    R[m_rd] = logic_shift_left((m_rd & 0x1f), R[m_rs]);
                break;
                case 0b0011: //lsr
                    R[m_rd] = logic_shift_right((m_rd & 0x1f), R[m_rs]);
                break;
                case 0b0100: //asr
                    R[m_rd] = arith_shift_right((m_rd & 0x1f), R[m_rs]);
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
                    if(Intruppted == true && m_rd == 15 && m_rs == 14){ // return from exception
                        R[15] = LReg[Intr_Mode];
                        Intruppted = false;

                        CPSR = SPSR_exc[Intr_Mode] ;
                    }

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

        } else if (TH_FMT_06(inst)){ // PC relative load

            unsigned int m_off8 = inst & 0xff ;

            unsigned int m_rt = inst & 0x700 ;

            m_rt = m_rt >> 8 ;

            bus.read(R[15]+m_off8, (unsigned char *)&R[m_rt],4);

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_07(inst)){

            unsigned int m_rd = inst & 0x7 ;

            unsigned int m_rn = inst & 0x38 ;
            
            m_rn = m_rn >> 3 ;

            unsigned int m_rm = inst & 0x1c ;

            m_rm = m_rm >> 6 ;

            unsigned int m_op =  inst & 0x0c00 ;

            m_op = m_op >> 10 ;

            unsigned int m_addr = 0;

            switch(m_op) {
                case 0b00: // store word to memory
                m_addr = R[m_rm] + R[m_rn] ;
                bus.write(m_addr, (unsigned char*)&R[m_rd],4);
                break;
                case 0b01: // store byte to memory
                m_addr = R[m_rm] + R[m_rn] ;
                bus.write(m_addr, (unsigned char*)&R[m_rd],1);
                break;
                case 0b10: // load word from memory
                m_addr = R[m_rm] + R[m_rn] ;
                bus.read(m_addr, (unsigned char*)&R[m_rd],4);
                break;
                case 0b11: // load unsigned byte
                m_addr = R[m_rm] + R[m_rn] ;
                bus.read(m_addr, (unsigned char*)&R[m_rd],1);                
                R[m_rd] = R[m_rd] & 0x000000ff ;
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_08(inst)){

            unsigned int m_rd = inst & 0x7 ;

            unsigned int m_rn = inst & 0x38 ;
            
            m_rn = m_rn >> 3 ;

            unsigned int m_rm = inst & 0x1c ;

            m_rm = m_rm >> 6 ;

            unsigned int m_op =  inst & 0x0c00 ;

            m_op = m_op >> 10 ;

            unsigned int m_addr = 0;

            switch(m_op) {
                case 0b00: // store halfword to memory
                m_addr = R[m_rm] + R[m_rn] ;
                bus.write(m_addr, (unsigned char*)&R[m_rd],2);
                break;
                case 0b01: // load signed byte
                m_addr = R[m_rm] + R[m_rn] ;
                bus.read(m_addr, (unsigned char*)&R[m_rd],1);
                if(R[m_rd] & 0x80) // sign extend
                    R[m_rd] = R[m_rd] | 0xffffff00 ;
                else
                    R[m_rd] = R[m_rd] & 0x000000ff ;
                break;
                case 0b10: // load halfword from memory unsigned
                m_addr = R[m_rm] + R[m_rn] ;
                bus.read(m_addr, (unsigned char*)&R[m_rd],2);
                R[m_rd] = R[m_rd] & 0x0000ffff;
                break;
                case 0b11: // load signed halfword
                m_addr = R[m_rm] + R[m_rn] ;
                bus.read(m_addr, (unsigned char*)&R[m_rd],2);
                if(R[m_rd] & 0x80) // sign extend
                    R[m_rd] = R[m_rd] | 0xffff0000 ;
                else
                    R[m_rd] = R[m_rd] & 0x0000ffff ;
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_09(inst)){

            unsigned int m_rt = inst & 0x7 ;

            unsigned int m_rn = inst & 0x38 ;
            
            m_rn = m_rn >> 3 ;

            unsigned int m_off5 = inst & 0x07c0 ;

            m_off5 = m_off5 >> 6 ;

            unsigned int m_op = inst & 0x1800 ;

            m_op = m_op >> 11 ;

            unsigned int m_addr = R[m_rn] + m_off5 ;

            switch (m_op) {
                
                case 0b00: // store word immidiate
                bus.write(m_addr, (unsigned char*)&R[m_rt], 4);
                break;
                case 0b01: // load word immidiate
                bus.read(m_addr, (unsigned char*)&R[m_rt], 4);
                break;
                case 0b10: // store byte immidiate
                bus.write(m_addr, (unsigned char*)&R[m_rt], 1);
                break;
                case 0b11: // load byte immidiate
                bus.read(m_addr, (unsigned char*)&R[m_rt], 1);
                R[m_rt] = R[m_rt] & 0x000000ff ;
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_10(inst)){

            unsigned int m_rt = inst & 0x7 ;

            unsigned int m_rn = inst & 0x38 ;
            
            m_rn = m_rn >> 3 ;

            unsigned int m_off5 = inst & 0x07c0 ;

            m_off5 = m_off5 >> 6 ;

            unsigned int m_op = inst & 0x0800 ;

            m_op = m_op >> 11 ;

            unsigned int m_addr = R[m_rn] + m_off5 ;

            switch (m_op) {
                
                case 0b0: // store halfword immidiate
                bus.write(m_addr, (unsigned char*)&R[m_rt], 2);
                break;
                case 0b1: // load halfword immidiate
                bus.read(m_addr, (unsigned char*)&R[m_rt], 2);
                R[m_rt] = R[m_rt] & 0x0000ffff ;
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_11(inst)){

            unsigned int m_off8 = inst & 0x0ff ;

            unsigned int m_rt = inst & 0x700 ;

            m_rt = m_rt >> 8 ;

            unsigned int m_op = inst & 0x0800 ;

            m_op = m_op >> 11 ;

            unsigned int m_addr = R[13] + m_off8 ;

            switch (m_op) {
                
                case 0b0: // store word SP relative
                bus.write(m_addr, (unsigned char*)&R[m_rt], 4);
                break;
                case 0b1: // load word SP relative
                bus.read(m_addr, (unsigned char*)&R[m_rt], 4);
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_12(inst)){

            unsigned int m_off8 = inst & 0x0ff ;

            unsigned int m_rt = inst & 0x700 ;

            m_rt = m_rt >> 8 ;

            unsigned int m_op = inst & 0x0800 ;

            m_op = m_op >> 11 ;

            switch (m_op) {
                
                case 0b0: // add to pc and store in dst register
                R[m_rt] = R[15] + m_off8 ;
                break;
                case 0b1: // add to sp and store in dst register
                R[m_rt] = R[13] + m_off8 ;
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_13(inst)){

            unsigned int m_immi = inst & 0x7f ;

            if ( inst & 0x80){ // decrement SP with immidiate

                R[13] = R[13] - m_immi ;

            }else { // increment SP with immidiate

                R[13] = R[13] + m_immi ;

            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_14(inst)){

            unsigned int m_op = inst & 0x0f00 ;

            m_op = m_op >> 8 ;

            switch(m_op) {
                case 0b0010:
                {
                    unsigned int m_rd = inst & 0x7 ;

                    unsigned int m_rm = inst & 0x38 ;

                    m_rm = m_rm >> 3 ;

                    m_op = inst & 0xc0 ;

                    m_op = m_op >> 6 ;

                    if(m_op == 0){ //sign extend halfword
                        R[m_rd] = R[m_rm] & 0x0000ffff ;

                        if(R[m_rd] & 0x8000) {
                            R[m_rd] = R[m_rd] | 0xffff0000 ;
                        }
                    } else if (m_op == 0b01){ // sign extend byte
                        R[m_rd] = R[m_rm] & 0x000000ff ;

                        if(R[m_rd] & 0x80) {
                            R[m_rd] = R[m_rd] | 0xffffff00 ;
                        }
                    } else if (m_op == 0b10){ // unsign extend halfword

                        R[m_rd] = R[m_rm] & 0x0000ffff ;

                    } else { // unsign extend byte

                        R[m_rd] = R[m_rm] & 0x000000ff ;

                    }
                }
                break;
                case 0b0001:
                case 0b0011: // branch if zero
                {
                    unsigned int m_rn = inst & 0x7;

                    if(!Itblock() && m_rn == 0) {

                        unsigned int m_immi6 = inst & 0xf8 ;

                        if(inst & 0x200)
                            m_immi6 = m_immi6 | 0x20 ;

                        R[15] = R[15] + (m_immi6 * 2) ;

                        //b_thumb = false;  //commented as multiplied by 2
                        //CPSR = CPSR & 0xffffffdf ;

                    }
                }
                break;
                case 0b1001:
                case 0b1011: // branch if not zero
                {
                    unsigned int m_rn = inst & 0x7;

                    if(!Itblock() && m_rn != 0) {

                        unsigned int m_immi6 = inst & 0xf8 ;

                        if(inst & 0x200)
                            m_immi6 = m_immi6 | 0x20 ;

                        R[15] = R[15] + (m_immi6 * 2) ;
                    
                        //b_thumb = false; //commented as multiplied by 2
                        //CPSR = CPSR & 0xffffffdf ;
                    }
                }
                break;
                case 0b0100:
                case 0b0101: // copy from memory to registers based on stack value
                {
                    unsigned int m_reglist9 = inst & 0x1f;

                    for(int i=0;i<9;i++){
                        if(m_reglist9 & 0x1){
                            
                            bus.read(R[13],(unsigned char *)&R[i],4);

                            R[13] = R[13] - 4 ;
                        }
                        m_reglist9 = m_reglist9 >> 1;
                    }
                }
                break;
                case 0b1100:
                case 0b1101: // copy registers to memory based on stack value
                {
                    unsigned int m_reglist9 = inst & 0x1f;

                    for(int i=0;i<9;i++){
                        if(m_reglist9 & 0x1){
                            
                            bus.write(R[13],(unsigned char *)&R[i],4);

                            R[13] = R[13] + 4 ;
                        }
                        m_reglist9 = m_reglist9 >> 1;
                    }
                }
                break;
                case 0b1010:
                {
                    unsigned int m_rd = inst & 0x7 ;

                    unsigned int m_rm = inst & 0x38 ;

                    m_rm = m_rm >> 3 ;

                    unsigned int m_op = inst & 0xc0 ;

                    m_op = m_op >> 6 ;

                    if(m_op == 0) { // Reverse a register value
                        ((unsigned char *)&R[m_rd])[3] = ((unsigned char *)&R[m_rm])[0] ;
                        ((unsigned char *)&R[m_rd])[2] = ((unsigned char *)&R[m_rm])[1] ;
                        ((unsigned char *)&R[m_rd])[1] = ((unsigned char *)&R[m_rm])[2] ;
                        ((unsigned char *)&R[m_rd])[0] = ((unsigned char *)&R[m_rm])[3] ;
                    } else if (m_op == 1) { // Reverse a halfword value
                        ((unsigned char *)&R[m_rd])[3] = ((unsigned char *)&R[m_rm])[2] ;
                        ((unsigned char *)&R[m_rd])[2] = ((unsigned char *)&R[m_rm])[3] ;
                        ((unsigned char *)&R[m_rd])[1] = ((unsigned char *)&R[m_rm])[0] ;
                        ((unsigned char *)&R[m_rd])[0] = ((unsigned char *)&R[m_rm])[1] ;
                    } else if (m_op == 3) { // Reverse signed halfword value
                        ((unsigned char *)&R[m_rd])[1] = ((unsigned char *)&R[m_rm])[0] ;
                        ((unsigned char *)&R[m_rd])[0] = ((unsigned char *)&R[m_rm])[1] ;

                        if(R[m_rd] & 0x80) {
                            R[m_rd] = R[m_rd] | 0xffff0000 ;
                        }
                    }
                }
                break;
                case 0b1111:
                {
                    unsigned int m_mask = inst & 0x0f;
                    
                    unsigned int m_op = inst & 0xf0 ;
                    
                    m_op = m_op >> 4;
                    
                    if(m_mask == 0) {

                        if(m_op == 0){
                            //NOP instruction
                        } else if(m_op == 0x1){
                            //Yield instruction
                        } else if(m_op == 0x2){
                            //Wait for Event. Switch to lowest power state and waits for event to wake up
                        } else if(m_op == 0x3){
                            //Wait for Interrupt. Switch to lowest power state and waits for interrupt to wake up
                        } else if(m_op == 0x4){
                            //Send a event in multi-processor system
                        }
                    } else { // IT instructions
                        IT_cond_base = m_op ;
                        ITMask = m_mask ;//(m_mask << 1) | 0x01 ;

                        CPSR = CPSR | ((ITMask | 0x03) << 25) ;
                        CPSR = CPSR | ((ITMask >> 2) << 10) ;
                        CPSR = CPSR | (IT_cond_base << 12) ;
                    }
                }
                break;
                case 0b0110:
                {
                    unsigned int m_snd = inst & 0x0f ;

                    unsigned int m_fst = inst & 0xf0 ;

                    m_fst = m_fst >> 4 ;

                    if(m_fst == 0b0101 && m_snd == 0b1000 && !Itblock()){
                        //set endianess bit
                        CPSR = CPSR | 0x200 ;
                    } else if(m_fst == 0b110 && !Itblock()){ // enable exceptions
                        if(m_snd & 0x1)
                            CPSR = CPSR & 0xffffffbf ; //clear the bit

                        if(m_snd & 0x2)
                            CPSR = CPSR & 0xffffff7f ;

                        if(m_snd & 0x4)
                            CPSR = CPSR & 0xfffffeff ;

                    } else if(m_fst == 0b111 && !Itblock()){ // disable exceptions
                        if(m_snd & 0x1)
                            CPSR = CPSR | 0x40 ; // set the bit

                        if(m_snd & 0x2)
                            CPSR = CPSR | 0x80 ;

                        if(m_snd & 0x4)
                            CPSR = CPSR | 0x100 ;
                    }

                }
                break;
                case 0b1110:
                {
                    //software breakpoint
                }
                break;
            }

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_15(inst)){

            unsigned int m_reglist8 = inst & 0x0ff ;

            unsigned int m_rn = inst & 0x700 ;

            m_rn = m_rn >> 8 ;

            unsigned int m_op = inst & 0x0800 ;

            m_op = m_op >> 11 ;

            unsigned int m_addr = R[m_rn] ;

            switch (m_op) {
                
                case 0b0: // store multiple registers to address
                    for(int i =0; i < 8; i++){
                        if(m_reglist8 & 0x01){
                            bus.write(m_addr, (unsigned char *)&R[i], 4);
                            m_addr+=4;
                        }
                        m_reglist8 = m_reglist8 >> 1 ;
                    }
                break;
                case 0b1: // load multiple registers from address
                    for(int i =0; i < 8; i++){
                        if(m_reglist8 & 0x01){
                            bus.read(m_addr, (unsigned char *)&R[i], 4);
                            m_addr+=4;
                        }
                        m_reglist8 = m_reglist8 >> 1 ;
                    }
                break;
            }

            R[m_rn] = m_addr ;
            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_17(inst)){
            
            SPSR_exc[SVC_MODE] = CPSR;

            //R[14] = R [15];

            Intruppted = true ;

            Intr_Mode = SVC_MODE ;

            LReg[SVC_MODE] = R[15];

            R[15] = VT_SOFTWARE_INTR ;

            ITMask = 0;
            IT_cond_base = 0;

            //processor_busy = false ;
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_16(inst) || TH_FMT_18(inst)){

            //unconditional branch
            if(!Itblock()) {
                unsigned int m_offset = 0;

                bool m_execinst = false ;

                if(TH_FMT_18(inst)){ // conditional branch
                    m_offset = inst & 0x7ff ;
                    m_execinst = true ;
                } else {
                    unsigned int m_cond = inst & 0x0f00 ;
                    m_cond = m_cond >> 8 ;

                    if(CheckCondCodes(m_cond)){
                        m_execinst = true ;
                        m_offset = inst & 0xff ;
                    }
                    
                }

                if(m_execinst) {
                    // check if to change to arm mode
                    if((m_offset & 0x01) == 0){
                        b_thumb = false ;
                        CPSR = CPSR & 0xffffffdf ;
                    }

                    R[15] = R[15] + m_offset ;
                }
            }
            
            //processor_busy = false ;
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_19(inst)){
            //32 bit thumb instructions
            inst = inst << 16 ;

            bus.read(R[15],(unsigned char *)&inst,2);

            R[15] = R[15] + 2 ;

            if( (inst & 0x2000000) == 0 ) { //Data Processing instructions

                unsigned int m_op = inst &  0x01e00000 ;

                m_op = m_op >> 21 ;

                bool m_sbit = ((inst & 0x00100000) == 0)?false:true ;

                unsigned int imm12 = inst & 0xff ;

                imm12 = imm12 | ( (inst & 0x7000) >> 4 ) ;

                imm12 = imm12 | ( (inst & 0x04000000) >> 14 ) ;

                unsigned int m_rn = inst & 0x000f0000 ;

                m_rn = m_rn >> 16 ;

                unsigned int m_rd = inst & 0xf00 ;

                m_rd = m_rd >> 8 ;

                switch (m_op) {
                    case 0b1010: // Add with carry
                    {
                        R[m_rd] = R[m_rn] + imm12 ;

                        if(IS_CSET())
                            R[m_rd]++;

                        if(m_sbit && !Itblock()) {
                            VCLR();
                            
                            if(R[m_rd] < R[m_rn])
                                CSET();
                            else
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
                    case 0b1000: //Add or compare negative
                    {
                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            
                            R[m_rd] = R[m_rd] + imm12 ;
                            
                            /*if(IS_CSET())
                                 R[m_rd]++ ;*/
                        } else
                            R[m_rd] = R[m_rn] + imm12 ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
                            VCLR();
                            
                            if(R[m_rd] < R[m_rn])
                                CSET();
                            else
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0000: //logical AND or tst
                    {
                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] & imm12 ;
                        } else
                            R[m_rd] = R[m_rn] & imm12 ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0001: //Bit Clear
                    {
                        R[m_rd] = R[m_rn] & (~imm12) ;

                        if(m_sbit && !Itblock()) {
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
                    case 0b0100: // Exclusive OR or test eq
                    {
                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] ^ imm12 ;
                        } else
                            R[m_rd] = R[m_rn] ^ imm12 ;

                        if(m_sbit && !Itblock()) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0011: // move negative or ORN
                    {
                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = ~imm12 ;
                        else
                            R[m_rd] = R[m_rn] | (~imm12) ;

                        if(m_sbit && !Itblock()) {
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
                    case 0b0010: // move or logical OR
                    {
                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = imm12 ;
                        else
                            R[m_rd] = R[m_rn] | imm12 ;

                        if(m_sbit && !Itblock()) {
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
                    case 0b1110: // Reverse Subtract
                    {
                        R[m_rd] = imm12 - R[m_rn] ;

                        if(m_sbit && !Itblock()) {
                            VCLR();

                            if(R[m_rd] < imm12)
                                CSET();
                            else
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
                    case 0b1011: //Subtract with carry
                    {
                        R[m_rd] = R[m_rn] - imm12 ;

                        if(IS_CSET())
                            R[m_rd]--;

                        if(m_sbit && !Itblock()) {
                            VCLR();

                            if(R[m_rd] < imm12)
                                CSET();
                            else
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
                    case 0b1101: //cmp or sub
                    {
                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] - imm12 ;
                        } else
                            R[m_rd] = R[m_rn] - imm12 ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                }

            } else if ( (inst & 0x3000000) == 0x2000000 ){ //Add/Sub plain 12 bit immi
                
                unsigned int m_op = inst &  0x00f00000 ;

                m_op = m_op >> 20 ;

                unsigned int imm12 = inst & 0xff ;

                imm12 = imm12 | ( (inst & 0x7000) >> 4 ) ;

                imm12 = imm12 | ( (inst & 0x04000000) >> 14 ) ;

                unsigned int m_rn = inst & 0x000f0000 ;

                m_rn = m_rn >> 16 ;

                unsigned int m_rd = inst & 0xf00 ;

                m_rd = m_rd >> 8 ;

                switch(m_op){
                    case 0b0000: //Add wide
                        R[m_rd] = R[m_rn] + imm12 ;
                    break;
                    case 0b1010: //Sub Wide
                        R[m_rd] = R[m_rn] - imm12 ;
                    break;
                    case 0b0010: //Addr before current Inst
                        R[m_rd] = R[15] - imm12 - 4 ;
                    break;
                    case 0b1000: //Addr after current Inst
                        R[m_rd] = R[15] - imm12 ;
                    break;
                    case 0b1100: //Move Top
                        imm12 = ((m_rn << 12) | imm12) << 16 ;
                        R[m_rd] = imm12 ;
                    break;
                    case 0b0100: //Move Wide
                        imm12 = ((m_rn << 12) | imm12) ;
                        R[m_rd] = imm12 ;
                    break;
                }
            } else if ( (inst & 0x3000000) == 0x3000000 ){
                
                unsigned int m_op = inst &  0x00e00000 ;

                m_op = m_op >> 21 ;

                unsigned int imm5_1 = inst & 0x1f ;

                unsigned int imm5_2 = inst & 0xc0 ;

                imm5_2 = imm5_2 >> 6 ;

                imm5_2 = imm5_2 | ( (inst & 0x7000) >> 10 ) ;

                unsigned int m_rn = inst & 0x000f0000 ;

                m_rn = m_rn >> 16 ;

                unsigned int m_rd = inst & 0xf00 ;

                m_rd = m_rd >> 8 ;

                switch(m_op) {
                    case 0b011: //bit field clear or bit field insert
                    if(m_rn == 0b1111){
                        for(int m_i=imm5_1;m_i<=imm5_2;m_i--){

                            R[m_rd] = R[m_rd] & ~( 0x1 << m_i ) ;

                        }
                    } else {
                        for(int m_i=imm5_1;m_i<=imm5_2;m_i--){

                            R[m_rd] = R[m_rd] | (( 0x1 << m_i ) & R[m_rn]) ;

                        }
                    }
                    break;
                    case 0b010: //signed bit field extract
                    {
                        unsigned int m_ext = 0;

                        for(int m_i=imm5_1;m_i<=imm5_2;m_i--){ // extract

                            m_ext = m_ext | (( 0x1 << m_i ) & R[m_rn]) ;

                        }

                        m_ext = m_ext >> imm5_2 ; // shift to 0

                        if(m_ext & (0x1 << imm5_1)){ // sign extend

                            for(int m_i=31;m_i<imm5_1;m_i--){

                                m_ext = m_ext | (0x1 << m_i) ;

                            }
                        }

                        R[m_rd] = m_ext ;
                    }
                    break;
                    case 0b000: //signed saturate LSL
                    {
                        unsigned int m_ext = R[m_rn] << imm5_2 ; // LSL

                        int m_old = (int) (m_ext) ;

                        if(m_ext & (0x1 << imm5_1)) { // saturate and sign extend

                            for(int m_i=31;m_i>imm5_1;m_i--){

                                m_ext = m_ext | ( 0x1 << m_i ) ;

                            }
                        }

                        if(m_old < ((int)m_ext)) // set Q flag if saturation occurred
                            QSET();

                        R[m_rd] = m_ext ;
                    }
                    break;
                    case 0b001: //signed saturate ASR
                    if(imm5_2 != 0)
                    {
                        bool m_HSbit = false;

                        if(R[m_rn] & 0x80000000)
                            m_HSbit = true ;

                        unsigned int m_ext = R[m_rn] >> imm5_2 ; // ASR

                        for(int m_i=31;m_i>=(31-imm5_2);m_i--){ // sign extend
                            m_ext = m_ext | (0x1 << m_i) ;
                        }

                        int m_old = (int) (m_ext) ;

                        if(m_ext & (0x1 << imm5_1)) { // saturate and sign extend

                            for(int m_i=31;m_i>imm5_1;m_i--){

                                m_ext = m_ext | ( 0x1 << m_i ) ;

                            }
                        }

                        if(m_old < ((int)m_ext)) // set Q flag if saturation occurred
                            QSET();

                        R[m_rd] = m_ext ;
                    } else { // signed saturate 16-bit

                        unsigned int m_ext1 = R[m_rn] & 0x0000ffff ;
                        unsigned int m_ext2 = R[m_rn] & 0xffff0000 ;

                        if(m_ext1 & 0x8000)
                            m_ext1 = m_ext1 | 0xffff0000 ;

                        unsigned int m_old1 = m_ext1 ;

                        unsigned int m_old2 = m_ext2 ;

                        if(m_ext1 & (0x1 << imm5_1)) { // saturate and sign extend

                            for(int m_i=15;m_i>imm5_1;m_i--){

                                m_ext1 = m_ext1 | ( 0x1 << m_i ) ;

                            }
                        }

                        if(m_ext2 & (0x1 << (imm5_1+16))) { // saturate and sign extend

                            for(int m_i=31;m_i>(imm5_1+16);m_i--){

                                m_ext2 = m_ext2 | ( 0x1 << m_i ) ;

                            }
                        }

                        if((m_old1 < m_ext1) || (m_old2 < m_ext2))
                            QSET();

                        m_ext1 = m_ext1 & 0x0000ffff ;

                        R[m_rd] = m_ext1 | m_ext2 ;
                    }
                    break;
                    case 0b110: //Unsigned Bit Field extract
                    {
                        unsigned int m_ext = R[m_rn] >> imm5_2 ;

                        unsigned int m_old = m_ext ;

                        for(int m_i = imm5_1; m_i < 32; m_i++){
                            m_ext = m_ext & ~(0x1 << m_i) ; // set to zero
                        }

                        if(m_old > m_ext)
                            QSET();

                        R[m_rd] = m_ext ;
                    }
                    break;
                    case 0b100: //Unsigned saturate, LSL                    
                    {
                        unsigned int m_ext = R[m_rn] << imm5_2 ; //LSL

                        unsigned int m_old = m_ext ;

                        for(int m_i = (imm5_1+1); m_i < 32; m_i++){
                            m_ext = m_ext & ~(0x1 << m_i) ; // set to zero
                        }

                        if(m_old > m_ext)
                            QSET();

                        R[m_rd] = m_ext ;
                    }
                    break;
                    case 0b101: //Unsigned saturate, ASR
                    if(imm5_2 != 0)
                    {
                        bool m_HSbit = false;

                        if(R[m_rn] & 0x80000000)
                            m_HSbit = true ;

                        unsigned int m_ext = R[m_rn] >> imm5_2 ; //ASR

                        for(int m_i=31;m_i>=(31-imm5_2);m_i--){ // sign extend
                            m_ext = m_ext | (0x1 << m_i) ;
                        }
                        
                        unsigned int m_old = m_ext ;

                        for(int m_i = (imm5_1+1); m_i < 32; m_i++){
                            m_ext = m_ext & ~(0x1 << m_i) ; // set to zero
                        }

                        if(m_old > m_ext)
                            QSET();

                        R[m_rd] = m_ext ;

                    } else { //Unsigned saturate 16-bit
                        
                        unsigned int m_ext1 = R[m_rn] & 0x0000ffff ;
                        unsigned int m_ext2 = R[m_rn] & 0xffff0000 ;

                        unsigned int m_old1 = m_ext1 ;
                        unsigned int m_old2 = m_ext2 ;

                        for(int m_i = (imm5_1+1); m_i < 16; m_i++){
                            m_ext1 = m_ext1 & ~(0x1 << m_i) ; // set to zero
                        }

                        for(int m_i = (imm5_1+1+16); m_i < 32; m_i++){
                            m_ext1 = m_ext1 & ~(0x1 << m_i) ; // set to zero
                        }

                        if((m_old1 > m_ext1) || (m_old2 > m_ext2))
                            QSET();
                    }
                    break;
                }
            }
        } else if(TH_FMT_21(inst)) {
                inst = inst << 16 ;

                bus.read(R[15],(unsigned char *)&inst,2);

                R[15] = R[15] + 2 ;

                unsigned int m_op = inst &  0x01e00000 ;

                m_op = m_op >> 21 ;

                bool m_sbit = ((inst & 0x00100000) == 0)?false:true ;

                unsigned int imm5 = inst & 0xc0 ;

                imm5 = imm5 >> 6 ;

                imm5 = imm5 | ( (inst & 0x7000) >> 10 ) ;

                unsigned int m_rn = inst & 0x000f0000 ;

                m_rn = m_rn >> 16 ;

                unsigned int m_rd = inst & 0xf00 ;

                m_rd = m_rd >> 8 ;

                unsigned int m_rm = inst & 0xf ;

                unsigned int m_typ = 0x30 ;

                m_typ = m_typ >> 4 ;

                switch (m_op) {
                    case 0b1010: // Add with carry
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ,imm5,m_rm,m_sbit);

                        R[m_rd] = R[m_rn] + m_shres ;

                        if(IS_CSET())
                            R[m_rd]++;

                        if(m_sbit && !Itblock()) {
                            VCLR();
                            
                            if(R[m_rd] < R[m_rn])
                                CSET();
                            else
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
                    case 0b1000: //Add or compare negative
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            
                            R[m_rd] = R[m_rd] + m_shres ;
                        } else
                            R[m_rd] = R[m_rn] + m_shres ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
                            VCLR();
                            
                            if(R[m_rd] < R[m_rn])
                                CSET();
                            else
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0000: //logical AND or tst
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit) {
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] & m_shres ;
                        } else
                            R[m_rd] = R[m_rn] & m_shres ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0001: //Bit Clear
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        R[m_rd] = R[m_rn] & (~m_shres) ;

                        if(m_sbit && !Itblock()) {
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
                    case 0b0100: // Exclusive OR or test eq
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] ^ m_shres ;
                        } else
                            R[m_rd] = R[m_rn] ^ m_shres ;

                        if(m_sbit && !Itblock()) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0011: // move negative or ORN
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = ~m_shres ;
                        else
                            R[m_rd] = R[m_rn] | (~m_shres) ;

                        if(m_sbit && !Itblock()) {
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
                    case 0b0010: // move and immediate shift
                    {
                        if((m_typ == 0b11) && (imm5 == 0))
                            imm5 = 1 ;

                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_shres ;
                        /*else
                            R[m_rd] = R[m_rn] | m_shres ;*/

                        if(m_sbit && !Itblock()) {
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
                    case 0b1110: // Reverse Subtract
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        R[m_rd] = m_shres - R[m_rn] ;

                        if(m_sbit && !Itblock()) {
                            VCLR();

                            if(R[m_rd] < m_shres)
                                CSET();
                            else
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
                    case 0b1011: //Subtract with carry
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);

                        R[m_rd] = R[m_rn] - m_shres ;

                        if(IS_CSET())
                            R[m_rd]--;

                        if(m_sbit && !Itblock()) {
                            VCLR();

                            if(R[m_rd] < m_shres)
                                CSET();
                            else
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
                    case 0b1101: //cmp or sub
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);
                        
                        unsigned int m_old = 0;

                        if(m_rn == 0b1111 && m_sbit){
                            m_old = R[m_rd] ;
                            R[m_rd] = R[m_rd] - m_shres ;
                        } else
                            R[m_rd] = R[m_rn] - m_shres ;

                        if(m_sbit && (!Itblock() || m_rn == 0b1111)) {
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

                        if(m_rn == 0b1111 && m_sbit)
                            R[m_rd] = m_old ;
                    }
                    break;
                    case 0b0110: //Pack halfword
                    {
                        unsigned int m_shres = DecodeImmShift(m_typ, imm5, m_rm, m_sbit);
                        
                        if(m_typ & 0x10) {

                            R[m_rd] = R[m_rn] ;

                            R[m_rd] = R[m_rd] & 0xffff0000 ;

                            R[m_rd] = R[m_rd] | (m_shres & 0x0000ffff) ;

                        } else {

                            R[m_rd] = m_shres ;

                            R[m_rd] = R[m_rd] & 0xffff0000 ;

                            R[m_rd] = R[m_rd] | (R[m_rn] & 0x0000ffff) ;
                        }

                        if(m_sbit && !Itblock()) {
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
                }
            
        } else if(TH_FMT_22(inst)) {
            
            inst = inst << 16 ;

            bus.read(R[15], (unsigned char *)&inst,2) ;
            R[15] = R[15] + 2 ;

            bool m_isextinst = inst & 0x80 ;

            unsigned int m_op = 0 ;

            bool m_sbit = false ;

            if(!m_isextinst) {
                m_op = inst & 0x600000 ;

                m_op = m_op >> 21 ;

                m_sbit = ((inst & 0x100000) !=0)?true:false;
            } else {
                m_op = inst & 0x700000 ;

                m_op = m_op >> 20 ;
            } 

            unsigned int m_rn = inst & 0xf0000 ;

            m_rn = m_rn >> 16 ;

            unsigned int m_rd = inst & 0xf00 ;

            m_rd = m_rd >> 8 ;

            unsigned int m_rm = inst & 0xf ;

            unsigned int m_op2 = 0;
            
            if(!m_isextinst)
                m_op2 = inst &0x70 ;
            else
                m_op2 = inst &0x30 ;

            m_op2 = m_op2 >> 4 ;

            if(!m_isextinst) {
                unsigned int m_shres = DecodeImmShift(m_op, (m_rm & 0x1f), m_rm, m_sbit);
                
                R[m_rd] = m_shres ;
            } else {
                switch (m_op) {
                    case 0b100 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);

                        if(m_shres & 0x8)
                            m_shres = m_shres | 0xffffff00 ;
                        else
                            m_shres = m_shres & 0x000000ff ;

                        if(m_rn == 0b1111)                   
                            R[m_rd] = m_shres ;
                        else
                            R[m_rd] = R[m_rn] + m_shres ;
                    }
                    break;
                    case 0b010 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);

                        m_shres = m_shres & 0x00ff00ff ;

                        if(m_shres & 0x8)
                            m_shres = m_shres | 0x0000ff00 ;
                        else
                            m_shres = m_shres & 0xffff00ff ;

                        if(m_shres & 0x800)
                            m_shres = m_shres | 0xff000000 ;
                        else
                            m_shres = m_shres & 0x00ffffff ;

                        unsigned int m_res1 = ((R[m_rn] | 0x0000ffff) + (m_shres | 0x0000ffff)) | 0xffff ;

                        unsigned int m_res2 = ((R[m_rn] | 0xffff0000) + (m_shres | 0xffff0000)) | 0xffff0000 ;
                        
                        if(m_rn == 0b1111)
                            R[m_rd] = m_res1 | m_res2 ;
                        else
                            R[m_rd] = R[m_rn] + (m_res1 | m_res2) ;
                    }
                    break;
                    case 0b000 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);

                        if(m_shres & 0x80)
                            m_shres = m_shres | 0xffff0000 ;
                        else
                            m_shres = m_shres & 0x0000ffff ;
                        
                        if(m_rn == 0b1111)
                            R[m_rd] = m_shres ;
                        else                            
                            R[m_rd] = R[m_rn] + m_shres ;
                    }
                    break;
                    case 0b101 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);

                        m_shres = m_shres & 0x000000ff ;

                        if(m_rn == 0b1111)                   
                            R[m_rd] = m_shres ;
                        else
                            R[m_rd] = R[m_rn] + m_shres ;
                    }
                    break;
                    case 0b011 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);

                        m_shres = m_shres & 0x00ff00ff ;

                        unsigned int m_res1 = ((R[m_rn] | 0x0000ffff) + (m_shres | 0x0000ffff)) | 0xffff ;

                        unsigned int m_res2 = ((R[m_rn] | 0xffff0000) + (m_shres | 0xffff0000)) | 0xffff0000 ;
                        
                        if(m_rn == 0b1111)
                            R[m_rd] = m_res1 | m_res2 ;
                        else
                            R[m_rd] = R[m_rn] + (m_res1 | m_res2) ;
                    }
                    break;
                    case 0b001 :
                    {
                        unsigned int m_shres = DecodeImmShift(0b11, m_op2*8, m_rm, m_sbit);
                        
                        m_shres = m_shres & 0x0000ffff ;
                        
                        if(m_rn == 0b1111)
                            R[m_rd] = m_shres ;
                        else                            
                            R[m_rd] = R[m_rn] + m_shres ;
                    }
                    break;
                }
            }
            
        } else if(TH_FMT_23(inst)) {
            inst = inst << 16 ;

            bus.read(R[15], (unsigned char *)&inst,2) ;
            R[15] = R[15] + 2 ;

            bool m_isodpinst = inst & 0x80 ;

            unsigned int m_op = inst & 0x700000 ;

            m_op = m_op >> 20 ;             

            unsigned int m_rn = inst & 0xf0000 ;

            m_rn = m_rn >> 16 ;

            unsigned int m_rd = inst & 0xf00 ;

            m_rd = m_rd >> 8 ;

            unsigned int m_rm = inst & 0xf ;

            unsigned int m_op2 = inst &0x70 ;

            m_op2 = m_op2 >> 4 ;

            unsigned int m_opPpre = (m_op << 3) | m_op2 ;

            if(!m_isodpinst) {
                
                switch(m_opPpre){
                    
                    case 0b001001 : //QADD16
                    {
                        unsigned int sum1 = (R[m_rn] & 0xffff) + (R[m_rm] & 0xffff) ;

                        unsigned int sum2 = (R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ;

                        R[m_rd] = ( sum1 & 0xffff ) | ( sum2 & 0xffff0000 ) ;
                    }
                    break;
                    case 0b001100 : //UADD16
                    {
                        unsigned int sum1 = (R[m_rn] & 0xffff) + (R[m_rm] & 0xffff) ;

                        unsigned int sum2 = (R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ;

                        R[m_rd] = ( sum1 & 0xffff ) | ( sum2 & 0xffff0000 ) ;
                    }
                    break;
                    case 0b000001: //QADD8
                    {
                        unsigned int sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = sum1 | sum2 | sum3 | sum4 ;
                    }
                    break;
                    case 0b000100: //UADD8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case 0b010001: //QASX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rm] & 0x0000ffff) + ((R[m_rn] & 0xffff0000) >> 16) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case 0b010100: //UASX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rm] & 0x0000ffff) + ((R[m_rn] & 0xffff0000) >> 16) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case 0b101001: //QSUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b001110: //UHADD16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case 0b100001: //QSUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case 0b000110: //UHADD8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b110001: //QASX
                    {   unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b010110: //UHASX
                    {   
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case 0b001000 : //SADD16
                    {
                        unsigned int sum1 = (R[m_rn] & 0xffff) + (R[m_rm] & 0xffff) ;

                        unsigned int sum2 = (R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ;

                        R[m_rd] = ( sum1 & 0xffff ) | ( sum2 & 0xffff0000 ) ;
                    }
                    break;
                    case 0b101110: //UHSUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case 0b000000: //SADD8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b100110: //UHSUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b010000: //SASX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break ;
                    case  0b110110: //UHSAX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break ;
                    case  0b001010: //SHADD16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b001101: //UQADD16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) + (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b000010: //SHADD8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b000101: //UQADD8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) + (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) + (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) + (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) + (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b010010: //SHASX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b010101: //UQASX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) + (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b101010: //SHSUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b101101: //UQSUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b100010: //SHSUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b100101: //UQSUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b110010: //SHSAX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b110101: //UQSAX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b101000: //SSUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b101100: //USUB16
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;
                        unsigned int m_res2 = ((R[m_rn] & 0xffff0000) - (R[m_rm] & 0xffff0000) ) & 0xffff0000 ;

                        R[m_rd] = m_res1 | m_res2 ;
                    }
                    break;
                    case  0b100000: //SSUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b100100: //USUB8
                    {
                        unsigned int m_sum1 = ((R[m_rn] & 0x000000ff) - (R[m_rm] & 0x000000ff)) & 0x000000ff ;
                        unsigned int m_sum2 = ((R[m_rn] & 0x0000ff00) - (R[m_rm] & 0x0000ff00)) & 0x0000ff00 ;
                        unsigned int m_sum3 = ((R[m_rn] & 0x00ff0000) - (R[m_rm] & 0x00ff0000)) & 0x00ff0000 ;
                        unsigned int m_sum4 = ((R[m_rn] & 0xff000000) - (R[m_rm] & 0xff000000)) & 0xff000000 ;

                        R[m_rd] = m_sum1 | m_sum2 | m_sum3 | m_sum4 ;
                    }
                    break;
                    case  0b110000: //SSAX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                    case  0b110100: //USAX
                    {
                        unsigned int m_res1 = ((R[m_rn] & 0x0000ffff) + ((R[m_rm] & 0xffff0000) >> 16) ) & 0x0000ffff ;
                        unsigned int m_res2 = (((R[m_rn] & 0xffff0000) >> 16) - (R[m_rm] & 0x0000ffff) ) & 0x0000ffff ;

                        R[m_rd] = m_res1 | ( m_res2 << 16 ) ;
                    }
                    break;
                }
            } else {
                
                switch(m_opPpre){
                    case  0b011000: //CLZ count leading zeros
                    {
                        unsigned int m_msbit = 0x80000000 ;

                        R[m_rd] = 0;

                        while(!(m_msbit & R[m_rm])){
                            R[m_rd]++ ;
                            m_msbit = m_msbit >> 1 ;
                        }
                    }
                    break;
                    case  0b000000: //QADD
                    {
                        R[m_rd] = R[m_rn] + R[m_rm] ;
                    }
                    break;
                    case  0b000001: //QDADD
                    {
                        R[m_rd] = R[m_rm] + (2*R[m_rn]) ;
                    }
                    break;
                    case  0b000011: //QDSUB
                    {
                        R[m_rd] = R[m_rm] - (2*R[m_rn]) ;
                    }
                    break;
                    case  0b000010: //QSUB
                    {
                        R[m_rd] = R[m_rm] - R[m_rn] ;
                    }
                    break;
                    case  0b001010: //RBIT reverse bits
                    {
                        unsigned int m_bit = 0x1 ;
                        unsigned int m_dbit = 0x80000000 ;

                        R[m_rd] = 0 ;

                        for(int m_i=0; m_i<32; m_i++){
                            if(R[m_rm] & m_bit)
                                R[m_rd] = R[m_rd] | m_dbit ;
                            else
                                R[m_rd] = R[m_rd] & (~m_dbit) ;

                            m_bit = m_bit << 1;
                            m_dbit = m_dbit >> 1;
                        }
                    }
                    break;
                    case  0b001000: //REV byte reverse word
                    {
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff) << 24) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff00) << 8) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff0000) >> 8) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff000000) >> 24) ;
                    }
                    break;
                    case  0b001001: //REV16 reverse packed halfword
                    {
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff) << 8) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff00) >> 8) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff0000) << 8) ;
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff000000) >> 8) ;
                    }
                    break;
                    case  0b001011: //REVSH Byte-Reverse Signed Halfword
                    {
                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff) << 8) ;

                        if(R[m_rd] & 0x8000)
                            R[m_rd] = R[m_rd] | 0xffff0000 ;

                        R[m_rd] = R[m_rd] | ((R[m_rm] & 0xff00) >> 8) ;
                    }
                    break;
                    case  0b010000: //Select Bytes
                    {
                        if(CPSR & 0x10000)
                            R[m_rd] = R[m_rd] | (R[m_rn] & 0xff) ;
                        else
                            R[m_rd] = R[m_rd] | (R[m_rm] & 0xff) ;

                        if(CPSR & 0x20000)
                            R[m_rd] = R[m_rd] | (R[m_rn] & 0xff00) ;
                        else
                            R[m_rd] = R[m_rd] | (R[m_rm] & 0xff00) ;

                        if(CPSR & 0x40000)
                            R[m_rd] = R[m_rd] | (R[m_rn] & 0xff0000) ;
                        else
                            R[m_rd] = R[m_rd] | (R[m_rm] & 0xff0000) ;

                        if(CPSR & 0x80000)
                            R[m_rd] = R[m_rd] | (R[m_rn] & 0xff000000) ;
                        else
                            R[m_rd] = R[m_rd] | (R[m_rm] & 0xff000000) ;
                    }
                    break;
                }
            }

        } else if(TH_FMT_24(inst)) {
            inst = inst << 16 ;

            bus.read(R[15], (unsigned char *)&inst,2) ;
            R[15] = R[15] + 2 ;

            unsigned int m_op = inst & 0x700000 ;

            m_op = m_op >> 20 ;             

            unsigned int m_rn = inst & 0xf0000 ;

            m_rn = m_rn >> 16 ;

            unsigned int m_racc = inst & 0xf000 ;

            m_racc = m_racc >> 12 ;

            unsigned int m_rd = inst & 0xf00 ;

            m_rd = m_rd >> 8 ;

            unsigned int m_rm = inst & 0xf ;

            unsigned int m_op2 = inst & 0xf0 ;

            m_op2 = m_op2 >> 4 ;

            unsigned int m_opPpre = (m_op << 4) | m_op2 ;

            switch(m_opPpre) {
                case 0b0000000:  //MLA multiply accumilate
                {
                    int result = (((int)R[m_rn]) * ((int)R[m_rm])) ;

                    if(m_racc != 0b1111) // only multiply if acc == 0b1111
                        result = result + (int)R[m_racc] ;
                    
                    R[m_rd] = (unsigned int)result ;
                }
                break;
                case 0b0000001:  //MLS multiply and subtract
                {
                    if(m_racc != 0b1111) {
                        int result = (int)R[m_racc] - (((int)R[m_rn]) * ((int)R[m_rm])) ;

                        R[m_rd] = (unsigned int)result ;
                    }
                }
                break;
                case 0b0010000: //SMLABB, SMLABT, SMLATB, SMLATT
                case 0b0010001:
                case 0b0010010:
                case 0b0010011:
                {
                    int m_operand1 = 0;
                    int m_operand2 = 0;

                    bool m_mbit = (m_op2 & 0b01)?true:false;

                    bool m_nbit = (m_op2 & 0b10)?true:false;

                    if(m_mbit) {
                        unsigned int m_tmp = ((R[m_rm] & 0xffff0000)>> 16 ) ;

                    if(m_tmp & 0x8000)
                        m_tmp = m_tmp | 0xffff0000 ;

                        m_operand2 = (int) m_tmp ;
                    } else {
                        unsigned int m_tmp = (R[m_rm] & 0xffff) ;

                        if(m_tmp & 0x8000)
                            m_tmp = m_tmp | 0xffff0000 ;

                        m_operand2 = (int) m_tmp ;
                    }

                    if(m_nbit) {
                        unsigned int m_tmp = ((R[m_rn] & 0xffff0000)>> 16 ) ;

                        if(m_tmp & 0x8000)
                            m_tmp = m_tmp | 0xffff0000 ;

                        m_operand1 = (int) m_tmp ;
                    } else {
                        unsigned int m_tmp = (R[m_rn] & 0xffff) ;

                        if(m_tmp & 0x8000)
                            m_tmp = m_tmp | 0xffff0000 ;

                        m_operand1 = (int) m_tmp ;
                    }

                    int m_result = m_operand1 * m_operand2 ;

                    if(m_racc != 0b1111)
                        R[m_rd] = (unsigned int)(m_result + (int)R[m_racc]) ;
                    else
                        R[m_rd] = (unsigned int)(m_result) ;
                }
                break;
                case  0b0100000: // Signed dual multiply accumilate add
                case  0b0100001:
                {
                    bool m_mswp= (m_op2 & 0b01)?true:false;

                    int m_operand1 = (int)(R[m_rn]) ;
                    int m_operand2 = 0 ;

                    if(m_mswp){
                        m_operand2 = (int)DecodeImmShift(0b11,16,m_rm,false) ;
                    } else
                        m_operand2 = (int)(R[m_rm]) ;

                    int m_prod1 = ((m_operand1 << 16) >> 16) * ((m_operand2 << 16) >> 16) ;
                    int m_prod2 = ((m_operand1 & 0xffff0000) >> 16) * ((m_operand2 & 0xffff0000) >> 16) ;

                    if(m_racc != 0b1111)
                        R[m_rd] = (unsigned int)( m_prod1 + m_prod2 + (int)R[m_racc]) ;
                    else
                        R[m_rd] = (unsigned int)( m_prod1 + m_prod2 ) ;

                }
                break;
                case 0b0110000:
                case 0b0110001: //Signed 32 + 16 x 32-bit, most significant word
                {
                    bool m_mset = (m_op2 & 0b01)?true:false ;

                    long int m_res = 0 ;

                    int m_operand1 = 0 ;

                    int m_operand2 = 0 ;

                    if(m_mset)
                        m_operand2 = (((int)R[m_rm]) >> 16) ;
                    else
                        m_operand2 = (((int)R[m_rm]) << 16) >> 16 ;

                    if(m_racc != 0b1111)
                        m_res = ((int)R[m_rn] * m_operand2) + (((int)R[m_racc]) << 16 ) ;
                    else
                        m_res = ((int)R[m_rn] * m_operand2) ;

                    R[m_rd] = (unsigned int)(m_res >> 16) ;
                }
                break;
                case 0b1000000: // Signed Dual Multiply Subtract and Accumulate
                case 0b1000001:
                {
                    bool m_mset = (m_op2 & 0b01)?true:false ;

                    int m_res = 0 ;

                    int m_operand2 = 0 ;

                    if(m_mset)
                        m_operand2 = (int)DecodeImmShift(0b11,16,m_rm,false) ;
                    else
                        m_operand2 = (int)R[m_rm] ;

                    if(m_racc != 0b1111) {
                        m_res = (((((int)R[m_rn]) << 16) >> 16) * ((m_operand2 << 16) >> 16))  \
                              - ((((int)R[m_rn]) >> 16) * (m_operand2 >> 16)) \
                              + ((int)R[m_racc]) ;
                    } else {
                        m_res = ((int)R[m_rn] * m_operand2) ;
                    }

                    R[m_rd] = (unsigned int)(m_res) ;
                }
                break;
                case 0b1010000: // Signed 32 + 32 x 32-bit, most significant word
                case 0b1010001:
                {
                    bool m_rset = (m_op2 & 0b01)?true:false ;

                    long int m_res = (int)R[m_rn] * (int)R[m_rm] ;

                    if(m_rset)
                        m_res = m_res + 0x80000000 ;

                    if(m_racc != 0b1111)
                        m_res = (((int)R[m_racc]) << 16 ) + m_res ;

                    R[m_rd] = (unsigned int) (m_res >> 32) ;
                }
                break;
                case 0b1100000: // Signed 32 – 32 x 32-bit, most significant word
                case 0b1100001:
                {
                    bool m_rset = (m_op2 & 0b01)?true:false ;

                    long int m_res = (int)R[m_rn] * (int)R[m_rm] ;

                    if(m_rset)
                        m_res = m_res + 0x80000000 ;

                    if(m_racc != 0b1111)
                        m_res = (((int)R[m_racc]) << 16 ) - m_res ;

                    R[m_rd] = (unsigned int) (m_res >> 32) ;
                }
                break;
                case 0b1110000: // USAD8 or USADA8
                {
                    R[m_rd] = R[m_rd] | (unsigned int)((int)(R[m_rn] & 0xff) - (int)(R[m_rm] & 0xff)) ;
                    R[m_rd] = R[m_rd] | (unsigned int)((((int)(R[m_rn] & 0xff00) - (int)(R[m_rm] & 0xff00)) >> 8) << 8) ;
                    R[m_rd] = R[m_rd] | (unsigned int)((((int)(R[m_rn] & 0xff0000) - (int)(R[m_rm] & 0xff0000)) >> 16) << 16) ;
                    R[m_rd] = R[m_rd] | (unsigned int)((((int)(R[m_rn] & 0xff000000) - (int)(R[m_rm] & 0xff000000)) >> 24) << 24) ;
                    
                    if(m_racc != 0b1111)
                        R[m_rd] = R[m_rd] + R[m_racc] ;
                }
                break;
            }

        } else if(TH_FMT_25(inst)) {

            inst = inst << 16 ;

            bus.read(R[15], (unsigned char *)&inst,2) ;
            R[15] = R[15] + 2 ;

            unsigned int m_op = inst & 0x700000 ;

            m_op = m_op >> 20 ;             

            unsigned int m_rn = inst & 0xf0000 ;

            m_rn = m_rn >> 16 ;

            unsigned int m_rdLo = inst & 0xf000 ;

            m_rdLo = m_rdLo >> 12 ;

            unsigned int m_rdHi = inst & 0xf00 ;

            m_rdHi = m_rdHi >> 8 ;

            unsigned int m_rm = inst & 0xf ;

            unsigned int m_op2 = inst & 0xf0 ;

            m_op2 = m_op2 >> 4 ;

            unsigned int m_opPpre = (m_op << 4) | m_op2 ;

            switch(m_opPpre) {
                case 0b0000000: // signed 32 x 32
                {
                    long int m_res = ((int)R[m_rn]) * ((int)R[m_rm]) ;

                    R[m_rdHi] = (unsigned int)(m_res >> 32) ;

                    R[m_rdLo] = (unsigned int)(m_res) ;
                }
                break;
                case 0b0011111: // Signed divide
                {
                    if((int)R[m_rm]== 0) {
                        R[m_rdLo] = 0;
                    } else {
                        R[m_rdLo] = (unsigned int)(((int)R[m_rn]) / ((int)R[m_rm])) ;
                    }
                }
                break;
                case 0b0100000: // Unsigned 32 x 32
                {
                    unsigned long int m_res = R[m_rn] * R[m_rm] ;

                    R[m_rdHi] = (unsigned int)(m_res >> 32) ;

                    R[m_rdLo] = (unsigned int)(m_res) ;
                }
                break;
                case 0b0111111: // Unsigned divide
                {
                    if(R[m_rm]== 0) {
                        R[m_rdLo] = 0;
                    } else {
                        R[m_rdLo] = R[m_rn] / R[m_rm] ;
                    }
                }
                break;
                case 0b1000000: // Signed 64 + 32 x 32
                {
                    unsigned long m_operand = R[m_rdHi] ;
                    
                    m_operand = m_operand << 32 ;

                    long int m_add = (int)(m_operand | R[m_rdLo]) ;

                    long int m_res = ((int)R[m_rn]) * ((int)R[m_rm]) ;

                    m_res = m_res + m_add ;

                    R[m_rdHi] = (unsigned int)(m_res >> 32) ;

                    R[m_rdLo] = (unsigned int)(m_res) ;
                }
                break;
                case 0b1001000: // Signed 64 + 16 x 16
                case 0b1001001:
                case 0b1001010:
                case 0b1001011:
                {

                }
                break;
            }

        } else {
            //processor_busy = false;
            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();
        }

    }


unsigned int ARM_CORE::DecodeImmShift(unsigned int type, unsigned int shift, unsigned int Reg, bool sbit){
    
    unsigned int Result = 0;

    switch(type){
        case 0b00: //LSL
        {
            bool m_HSbit = ((R[Reg] & (0x1 << (32-shift)))!= 0)?true:false;

            Result = R[Reg] << shift ;

            if(sbit && m_HSbit && !Itblock())
                CSET();
        }
        break;
        case 0b01: //LSR
        {
            if(shift == 0)
                shift = 32 ;
            
            bool m_LSbit = ((R[Reg] & (0x1 << (shift-2)))!= 0)?true:false;

            Result = R[Reg] >> (shift-1) ;

            if(sbit && m_LSbit && !Itblock())
                CSET();
        }
        break;
        case 0b10: //ASR
        {
            if(shift == 0)
                shift = 32 ;

            bool m_LSbit = ((R[Reg] & (0x1 << (shift-2)))!= 0)?true:false;

            bool m_HSbit = ((R[Reg] & 0x80000000)!= 0)?true:false;

            Result = R[Reg] >> (shift-1) ;

            for(int m_i=31;m_i>(31-(shift-1));m_i--){
                Result = Result | (0x1 << m_i) ;
            }

            if(sbit && m_LSbit && !Itblock())
                CSET();
        }
        break;
        case 0b11: //ROR
        {
            Result = R[Reg] ;

            for (int m_i=0; m_i<shift; m_i++) {

                bool LSBset = Result & 0x00000001;
                
                Result = Result >> 1;
                
                if(LSBset) {
                    Result = Result | 0x80000000;
                    
                    if(sbit && !Itblock())
                        CSET();
                } else {
                    if(sbit && !Itblock())
                        CCLR();
                }
            }
        }
        break;
    }

    return Result ;
}