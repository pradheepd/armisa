

#define B15 0b1000000000000000
#define B14 0b0100000000000000
#define B13 0b0010000000000000
#define B12 0b0001000000000000
#define B11 0b0000100000000000
#define B10 0b0000010000000000
#define B09 0b0000001000000000
#define B08 0b0000000100000000
#define B07 0b0000000010000000

#define TH_FMT_02(a) ((((B15 | B14 | B13 | B12 | B11 | B10) & a) == 0x1c00)?true:false)

#define TH_FMT_01(a) ((((B15 | B14 | B13) & a) == 0x0000)?true:false)

#define TH_FMT_03(a) ((((B15 | B14 | B13) & a) == 0x2000)?true:false)

#define TH_FMT_04(a) ((((B15 | B14 | B13 | B12 | B11 | B10) & a) == 0x4000)?true:false)

#define TH_FMT_05(a) ((((B15 | B14 | B13 | B12 | B11 | B10) & a) == 0x4400)?true:false)

#define TH_FMT_06(a) ((((B15 | B14 | B13 | B12 | B11) & a) == 0x4800)?true:false)

#define TH_FMT_07(a) ((((B15 | B14 | B13 | B12 | B09) & a) == 0x5000)?true:false)

#define TH_FMT_08(a) ((((B15 | B14 | B13 | B12 | B09) & a) == 0x5200)?true:false)

#define TH_FMT_09(a) ((((B15 | B14 | B13 ) & a) == 0x6000)?true:false)

#define TH_FMT_10(a) ((((B15 | B14 | B13 | B12 ) & a) == 0x8000)?true:false)

#define TH_FMT_11(a) ((((B15 | B14 | B13 | B12 ) & a) == 0x9000)?true:false)

#define TH_FMT_12(a) ((((B15 | B14 | B13 | B12 ) & a) == 0xa000)?true:false)

#define TH_FMT_13(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08) & a) == 0xb000)?true:false)

#define TH_FMT_14(a) ((((B15 | B14 | B13 | B12) & a) == 0xb000)?true:false)

#define TH_FMT_15(a) ((((B15 | B14 | B13 | B12) & a) == 0xc000)?true:false)

#define TH_FMT_16(a) ((((B15 | B14 | B13 | B12) & a) == 0xd000)?true:false)

#define TH_FMT_17(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08) & a) == 0xdf00)?true:false)

#define TH_FMT_18(a) ((((B15 | B14 | B13 | B12 | B11) & a) == 0xe000)?true:false)

#define TH_FMT_19(a) ((((B15 | B14 | B13 | B12 | B11) & a) == 0xf000)?true:false) // 32 bit thumb instruction

#define TH_FMT_20(a) ((((B15 | B14 | B13 | B12 | B11 | B10) & a) == 0x1800)?true:false)

#define TH_FMT_21(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09) & a) == 0xea00)?true:false)

#define TH_FMT_22(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08 | B07) & a) == 0xfa00)?true:false)

#define TH_FMT_23(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08 | B07) & a) == 0xfa10)?true:false)

#define TH_FMT_24(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08 | B07) & a) == 0xfb00)?true:false)

#define TH_FMT_25(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 | B08 | B07) & a) == 0xfb10)?true:false)

#define TH_FMT_26(a) ((((B15 | B14 | B13 | B12 | B11 | B10 | B09 ) & a) == 0xf800)?true:false)