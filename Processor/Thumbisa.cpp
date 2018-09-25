#include "ARM_CORE.h"
#include "ThumbDefs.h"

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
            //processor_busy = false;
            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();
        }

    }