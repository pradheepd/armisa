#include "ARM_CORE.h"
#include "ThumbDefs.h"
#include "PeripheralDefs.h"

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

            unsigned int m_op = 0x0f00 ;

            m_op = m_op >> 8 ;

            switch(m_op) {
                case 0b0010:
                {
                    unsigned int m_rd = 0x7 ;

                    unsigned int m_rm = 0x38 ;

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
                    unsigned int m_op = inst & 0x0f ;

                    if(m_op == 0){
                        //NOP instruction
                    } else if(m_op == 0x10){
                        //Yield instruction
                    } else if(m_op == 0x20){
                        //Wait for Event. Switch to lowest power state and waits for event to wake up
                    } else if(m_op == 0x30){
                        //Wait for Interrupt. Switch to lowest power state and waits for interrupt to wake up
                    } else if(m_op == 0x40){
                        //Send a event in multi-processor system
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
            
            SPSR_svc = CPSR;

            //R[14] = R [15];

            Intruppted = true ;

            Intr_Mode = SVC_MODE ;

            LReg[SVC_MODE] = R[15];

            R[15] = VT_SOFTWARE_INTR ;

            //processor_busy = false ;
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_16(inst) || TH_FMT_18(inst)){

            //unconditional branch
            if(!Itblock()) {
                unsigned int m_offset = 0;

                if(TH_FMT_18(inst))
                    m_offset = inst & 0x7ff ;
                else
                    m_offset = inst & 0xff ;

                R[15] = R[15] + m_offset ;
            }
            
            //processor_busy = false ;
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

        } else if (TH_FMT_19(inst)){

        } else {
            //processor_busy = false;
            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();
        }

    }
