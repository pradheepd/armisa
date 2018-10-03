#include "systemc.h"
#include "fstream"
#include "ARM_CORE.h"
#include "PeripheralDefs.h"

#if 0
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
#endif
    
    unsigned int ARM_CORE::logic_shift_left(unsigned int reg, unsigned int s){
        
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
    
    unsigned int ARM_CORE::logic_shift_right(unsigned int reg, unsigned int s){
        
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
    
    unsigned int ARM_CORE::arith_shift_right(unsigned int reg, unsigned int s){

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
    
    unsigned int ARM_CORE::rotate_right(unsigned int reg, unsigned int s){
        
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

    void ARM_CORE::execute_inst () {
        //clear flags set by previous inst
        //VCLR();
        //CCLR();
        //ZCLR();
        //NCLR();
#if 0
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
                //processor_busy = false;

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
            //processor_busy = false;

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

            //processor_busy = false;
            cycles = 0;

            //clear flags set by previous inst
            VCLR();
            CCLR();
            ZCLR();
            NCLR();

            return;
        }
#endif
        //check if bx instruction
        bool mult_flag = false;
        unsigned int mult_op = 0;
        unsigned int check_inst = inst & BX_MASK ;
        check_inst = check_inst >> 4;

        if(check_inst == IN_BX)
        {
            // get the rm register
            rm = inst & RMREG_MASK;

            // check if to change to thumb mode
            if(R[rm] & 0x01){
                b_thumb = true;
                CPSR = CPSR | 0x20 ;
            }

            //Move the value of register to PC
            R[15] = R[rm];

            //processor_busy = false;
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

            // check if to change to thumb mode
            /*if(offset & 0x01){ //commented as its shifted left
                b_thumb = true;
                CPSR = CPSR | 0x20 ;
            }*/

            if(IN_BL == check_inst)
                R[14] = R[15];

            R[15] = R[15] + (int)offset ;

            //processor_busy = false;
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

        if( check_inst == IN_SDS_B || check_inst == IN_SDS_W ) {
            
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

            SPSR_exc[SVC_MODE] = CPSR;

            //R[14] = R [15];

            Intruppted = true ;

            Intr_Mode = SVC_MODE ;

            //CCPSR = &

            LReg[SVC_MODE] = R[15];

            R[15] = VT_SOFTWARE_INTR ;

            //processor_busy = false ;
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

            //bulktransfer = true;
            bool load = false;
            bool pre = false;
            bool is_add = false;
            bool wrtbck = false;
            bool isprevilageuser = false;

            unsigned int opts = inst & 0x01f00000 ;

            opts = opts >> 20 ;

            //check if load or store op
            if(opts & 0x01)
                load = true;
            
            //check if pre or post
            if(opts & 0x100)
                pre = true;

            //check if up / down
            if(opts & 0x80)
                is_add = true;

            //check if write back is true
            if(opts & 0x02)
                wrtbck = true;

            //check if PSR or force user
            if(opts & 0x04)
                isprevilageuser = true;

            
            rn = inst & 0x000f0000 ;

            rn = rn >> 16 ;

            unsigned int regs = inst & 0x0000ffff ;
            unsigned int i = 0;
            unsigned int addr = 0;

            while(regs != 0){

                if(pre) {
                    if(is_add)
                        addr = R[rn] + 4;
                    else
                        addr = R[rn] - 4;
                }
                
                if(regs & 0x01){
                    //BT_RegList.write(i);
                    if(load) {
                        bus.read(addr, (unsigned char *)&R[i], 4);
                    } else {
                        bus.write(addr, (unsigned char *)&R[i], 4);
                    }
                }
                i++;
                
                if(!pre) {
                    if(is_add)
                        addr = R[rn] + 4;
                    else
                        addr = R[rn] - 4;
                } else if (wrtbck) {
                    R[rn] = addr;
                } else {

                }

                regs = regs >> 1;
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
                R[rd] = CPSR ;
            } else {
                R[rd] = SPSR ;
            }

            //processor_busy = false;
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

            //processor_busy = false;
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
            
            if(!load) {
                if(isbyte){
                    unsigned char val = 0;
                    bus.read(addr, &val,1);
                    R[rd] = val;
                } else {
                    unsigned int val = 0;
                    bus.read(addr, (unsigned char *)&val,4);
                    R[rd] = val;                    
                }
            }else {
                if(isbyte){
                    unsigned char val = R[rd] & 0xffff;
                    bus.write(addr, &val,1);
                } else {
                    unsigned int val = R[rd];
                    bus.write(addr, (unsigned char *)&val,4);
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
            unsigned char val[2] = {0,0};

            if(!load) {
                if(SH == 1){
                    bus.read(addr, &val[0],1);
                    R[rd] = val[0];
                } else if(SH == 3){
                    bus.read(addr, &val[0],2);
                    R[rd] = val[1];
                    R[rd] = R[rd] << 8;
                    R[rd] = R[rd] | val[0];
                }
            }else {
                if(SH == 1){
                    val[0] = R[rd] & 0xffff;
                    bus.write(addr, &val[0],1);
                } else if(SH == 3){
                    val[0] = R[rd] & 0xffff;
                    val[1] = (R[rd] & 0xffff0000) >> 8 ;
                    bus.write(addr, &val[0],2);
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
                
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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

                    if(R[rd] < R[rn])
                        CSET();

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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
                break;
            }
            case IN_MOV:
            {
                R[rd] = operand2;

                if(s_bit && Intruppted && operand2 == 14 && rd == 15) {
                    R[15] = LReg[Intr_Mode] ;
                    Intruppted = false;

                    CPSR = SPSR_exc[Intr_Mode] ;
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
                //processor_busy = false;
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
                //processor_busy = false;
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
                //processor_busy = false;
                break;
            }   
            default:
                //processor_busy = false;
                break;
        };
    }

    void ARM_CORE::process_condition() {
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
        } else {
            if(ITMask == 0 || ITMask == 0x8)
                execute_thumb_inst();
            else{
                if(Itblock()){
                    execute_thumb_inst();
                }
            }
        }
        //if ( cycles != 0 && b_thumb == false)
        //    execute_inst();
    }
#if 0
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
#endif
    void ARM_CORE::process() {

        /*if(sclk.read() && !//processor_busy)
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
            //processor_busy = true;
        } else {
            flash_rsignal = false;
            flash_wsignal = false;
            sram_rsignal = false;
            sram_wsignal = false;
        }*/
        if(!b_thumb){
            bus.read(R[15],(unsigned char *)&inst,4);
            // increment program counter
            R[15] = R[15] + 4;
        }else{
            bus.read(R[15],(unsigned char *)&inst,2);

            inst = inst & 0x0000ffff ;
            // increment program counter
            R[15] = R[15] + 2;
        }

        process_condition();
    }

    ARM_CORE::ARM_CORE(const char *name) : sc_module(name), immi(0), rot(0), b_thumb(false),
     s_bit(false), immi_bit(false), shift(0), rm(0), rn(0), rd(0), CPSR(0),SPSR(0), CCPSR(&CPSR),
     Intruppted(false), Intr_Mode(SVC_MODE), bus("armcore_master"), halfword_transfer(false),
     IT_cond_base(0), ITMask(0)
     /*,flash_rsignal("FL_RS"),
     sram_rsignal("SR_RS"),
     flash_wsignal("FL_WS"),
     sram_wsignal("SR_WS") */
     {
        SC_HAS_PROCESS(ARM_CORE);

        //sc_fifo<unsigned int> BT_RegList(16);
        memset (R, 0, sizeof(unsigned int) * 15);
        
        bus.read(VT_IN_SP_VAL_ADDR,(unsigned char *)&R[13],4);
        
        bus.read(VT_RESET,(unsigned char *)&R[15],4);

        /*flash.intr(flash_rsignal);
        sram.intr(sram_rsignal);

        flash.intrw(flash_wsignal);
        sram.intrw(sram_wsignal);*/

        //Set processor state to take instruction
        //Dst_Mask = DST_MASK_INST;

        SC_METHOD(process);
        dont_initialize();
            sensitive << sclk.pos();

        /*SC_METHOD(process2f);
         dont_initialize();
        sensitive << busintrf.pos();

        SC_METHOD(process2s);
         dont_initialize();
        sensitive << busintrs.pos();*/
    }


int sc_main (int argc, char* argv[]) {

    sc_clock clock ("system_clock",1,0.5);
    
    FLASH flash("flash_mem");

    /*if(argc < 2) {
        cout << "Program bin file not provided" << endl;
        exit (1);
    } else {
        if(!flash.Load_Program(argv[1]))
            exit(1);
    }*/
    
    INTRNAL internal("Internal_mem");
    SRAM sram("sram_mem");
    ARM_CORE processor("PROCESSOR");
    processor.sclk(clock);
    /*processor.busintrf(flash.intrp);
    processor.busintrs(sram.intrp);

    flash.sclk(clock);
    sram.sclk(clock);*/

    sc_start();

    return 0;
}
