#include "wiring_private.h"

#ifndef USER_USB_RAM
void USBDeviceCfg();
void USBDeviceIntCfg();
void USBDeviceEndPointCfg();
#endif

extern __idata volatile uint32_t timer0_overflow_count;
extern __idata volatile uint8_t timer0_overflow_count_5th_byte;

#if F_CPU == 56000000
#define T0_CYCLE 224
#else
#define T0_CYCLE 250
#endif

void Timer0Interrupt(void) __interrupt (INT_NO_TMR0) __using(1) //using register bank 1
{
    /*timer0_overflow_count++;
     */ //when putting timer0_millis and timer0_overflow_count in bank 1, C code is no longer correct
    __asm__ (";Increase timer0_overflow_count on R0~R4(5bytes)\n"
             "    inc r0                                   \n"
             "    cjne r0,#0,incTimer0_overflow_countOver$ \n"
             "    inc r1                                   \n"
             "    cjne r1,#0,incTimer0_overflow_countOver$ \n"
             "    inc r2                                   \n"
             "    cjne r2,#0,incTimer0_overflow_countOver$ \n"
             "    inc r3                                   \n"
             "    cjne r3,#0,incTimer0_overflow_countOver$ \n"
             "    inc r4                                   \n"
             "incTimer0_overflow_countOver$:               \n"
             );
}

uint32_t micros(){
    /*uint32_t m;
     uint8_t t;
     uint8_t interruptOn = EA;
     EA = 0;
     
     m = timer0_overflow_count;
     t = TL0;
     
     if ((TF0) && (t < 255)){
     m++;
     }
     
     if (interruptOn) EA = 1;*/
    
    __asm__ (";uint8_t interruptOn = EA; //to c            \n"
             ";clr and mov won't affect c                  \n"
             "    mov c,_EA                                \n"
             ";EA = 0;                                     \n"
             "    clr _EA                                  \n"
             ";Copy _timer0_overflow_count to local R0~R3,m\n"
             "    mov r0, (_timer0_overflow_count)         \n"
             "    mov r1, (_timer0_overflow_count)+1       \n"
             "    mov r2, (_timer0_overflow_count)+2       \n"
             "    mov r3, (_timer0_overflow_count)+3       \n"
             ";Copy TL0 to local R4, t                     \n"
             "    mov r4, _TL0                             \n"
             ";Copy TCON (TF0) to local R5                 \n"
             "    mov b, _TCON                             \n"
             ";if (interruptOn) EA = 1;                    \n"
             "    mov _EA,c                                \n"
             
             ";if ((TF0 in b) && (R4 != 255)){             \n"
             "    jnb b.5,incTimer0_overf_cntCopyOver$     \n"
             "    mov a,#1     \n"
             "    add a,r4     \n"
             "    jz incTimer0_overf_cntCopyOver$\n"
             
             ";m++                                         \n"
             "    inc r0                                   \n"
             "    cjne r0,#0,incTimer0_overf_cntCopyOver$  \n"
             "    inc r1                                   \n"
             "    cjne r1,#0,incTimer0_overf_cntCopyOver$  \n"
             "    inc r2                                   \n"
             "    cjne r2,#0,incTimer0_overf_cntCopyOver$  \n"
             "    inc r3                                   \n"
             "incTimer0_overf_cntCopyOver$:                \n"
            );
    
    //since TL0 (R4) always ranging from (256-T0_CYCLE) to 255, we can reduce R4 by (256-T0_CYCLE)
    
#if T0_CYCLE == 250
    __asm__ ("    clr c                                    \n"
             "    mov a, r4                                \n"
             "    subb a,#6                                \n"
             "    mov r4, a                                \n");
#elif T0_CYCLE == 224
    __asm__ ("    clr c                                    \n"
             "    mov a, r4                                \n"
             "    subb a,#32                               \n"
             "    mov r4, a                                \n");
#endif
    
#if F_CPU == 16000000
    //1m = 250t 1t=0.75us (m*250+t)*3/2/2
    //m in r0~r3
    //t in r4
    //t=((3*t)>>1);
    //m=(m*375)=m<<8+m*119;
    //return (m+t)>>1
    
    
    __asm__ (
             ";1m = 250t 1t=0.5us (m*250+t)*3/2/2  t is 0~249\n"
             ";we need to return (m<<8+m*119+((3*t)>>1)))>>1 \n"
             
             ";m=m*119;                                    \n"
             "    mov a, r0                                \n"
             "    mov r6, a                                \n"
             "    mov a, r1                                \n"
             "    mov r7, a                                \n"
             "    push ar3                                 \n"
             "    push ar2                                 \n"
             
             "    mov b, #119                              \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, a                                \n"
             "    mov r5, b                                \n"
             
             "    mov b, #119                              \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r1, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #119                              \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r2, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #119                              \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r3, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             ";m=m+m<<8;                                   \n"
             
             "    mov a, r6                                \n"
             "    add a, r1                                \n"
             "    mov r1, a                                \n"
             "    mov a, r7                                \n"
             "    addc a, r2                               \n"
             "    mov r2, a                                \n"
             "    pop a                                    \n"
             "    addc a, r3                               \n"
             "    mov r3, a                                \n"
             "    pop a                                    \n"
             "    addc a, r5                               \n"
             "    mov r5, a                                \n"
             
             ";t=((3*t)>>1));                              \n"
             
             "    mov b, #3                                \n"
             "    mov a, r4                                \n"
             "    mul ab                                   \n"
             "    mov r4, a                                \n"
             "    mov a, b                                 \n"
             "    clr c                                    \n"
             "    rrc a                                    \n"
             "    mov r6, a                                \n"
             "    mov a, r4                                \n"
             "    rrc a                                    \n"
             "    mov r4, a                                \n"
             
             ";get m+t                                     \n"
             "    mov r7, #0                               \n"
             "    mov a, r4                                \n"
             "    add a, r0                                \n"
             "    mov r0, a                                \n"
             "    mov a, r6                                \n"
             "    addc a, r1                               \n"
             "    mov r1, a                                \n"
             "    mov a, r7                                \n"
             "    addc a, r2                               \n"
             "    mov r2, a                                \n"
             "    mov a, r7                                \n"
             "    addc a, r3                               \n"
             "    mov r3, a                                \n"
             "    mov a, r7                                \n"
             "    addc a, r5                               \n"
             "    mov r5, a                                \n"
             
             ";return m+t                                  \n"
             "    clr c                                    \n"
             "    mov a, r5                                \n"
             "    rrc a                                    \n"
             "    mov r5, a                                \n"
             "    mov a, r3                                \n"
             "    rrc a                                    \n"
             "    mov r3, a                                \n"
             "    mov a, r2                                \n"
             "    rrc a                                    \n"
             "    mov b, a                                 \n"
             "    mov a, r1                                \n"
             "    rrc a                                    \n"
             "    mov dph, a                               \n"
             "    mov a, r0                                \n"
             "    rrc a                                    \n"
             "    mov dpl, a                               \n"
             "    mov a, r3                                \n"
             
             );
    //’dpl’ (LSB),’dph’,’b’ & ’acc’
#elif F_CPU == 24000000
    //24M CLK
    
     /*1m = 250t 1t=0.5us (m*250+t)/2
     
     t=(t>>1);
     m=m*125;
     
     return ( m+t );*/
    
    //assembly has better support for multiplication
    

__asm__ (
             ";1m = 250t 1t=0.5us (m*250+t)/2  t is 0~249  \n"
             ";we need to return m*125+t/2                 \n"
             ";t=(t>>1);                                   \n"
             "    mov a,r4                                 \n"
             "    clr c                                    \n"
             "    rrc a                                    \n"
             "    mov r4,a                                 \n"
             
             ";m=m*125;                                    \n"
             "    mov b, #125                              \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, a                                \n"
             "    mov r5, b                                \n"
             
             "    mov b, #125                              \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r1, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #125                              \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r2, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #125                              \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r3, a                                \n"
             
             ";return m+t                                  \n"
             "    mov r5, #0                               \n"
             "    mov a, r4                                \n"
             "    add a, r0                                \n"
             "    mov dpl, a                               \n"
             "    mov a, r1                                \n"
             "    addc a, r5                               \n"
             "    mov dph, a                               \n"
             "    mov a, r2                                \n"
             "    addc a, r5                               \n"
             "    mov b, a                                 \n"
             "    mov a, r3                                \n"
             "    addc a, r5                               \n"
             
             );
#elif F_CPU == 56000000
    //56M CLK
    
    /*1m = 224t 1t=0.21412us=3/14us (m*224+t)*3/14
     m*48+t*3/14
     //m in r0~r3
     //t in r4*/
    
    //assembly has better support for multiplication
    
    
    __asm__ (";t=t*3/14; t ranging 0~223                   \n"
             "    mov a,r4                                 \n"
             "    mov b,#14                                \n"
             "    div ab                                   \n"
             "    mov r5,b                                 \n"
             "    mov b,#3                                 \n"
             "    mul ab                                   \n"
             "    mov r4,a                                 \n"
             ";now r4=int(r4/14)*3                         \n"
             ";r4=r4+reminder*3/14                         \n"
             "    mov a,r5                                 \n"
             "    mov b,#3                                 \n"
             "    mul ab                                   \n"
             "    mov b,#14                                \n"
             "    div ab                                   \n"
             "    add a,r4                                 \n"
             "    mov r4,a                                 \n"
   
             ";m=m*48;                                     \n"
             "    mov b, #48                               \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, a                                \n"
             "    mov r5, b                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r1, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r2, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r3, a                                \n"
             //5th byte not needed
             //"    clr a                                    \n"
             //"    addc a, b                                \n"
             //"    mov r5, a                                \n"
             
             
             ";m=m+t                                       \n"
             "    mov r6, #0                               \n"
             "    mov a, r4                                \n"
             "    add a, r0                                \n"
             "    mov r0, a                                \n"
             "    mov a, r1                                \n"
             "    addc a, r6                               \n"
             "    mov r1, a                                \n"
             "    mov a, r2                                \n"
             "    addc a, r6                               \n"
             "    mov r2, a                                \n"
             "    mov a, r3                                \n"
             "    addc a, r6                               \n"
             "    mov r3, a                                \n"
             //5th byte not needed
             //"    mov a, r5                                \n"
             //"    addc a, r6                               \n"
             //"    mov r5, a                                \n"
             
             ";return                                      \n"

             "    mov dpl, r0                              \n"
             "    mov dph, r1                              \n"
             "    mov b, r2                                \n"
             "    mov a, r3                                \n"

             );

#else
#error "clock not supported yet"

    
#endif
    //return values: ’dpl’ 1B, ’dpl’ LSB & ’dph’ 2B,
    //’dpl’, ’dph’ and ’b’ 3B, ’dpl’,’dph’,’b’ & ’acc’ 4B
    
}

uint32_t millis()
{

    // disable interrupts while we read timer0_millis or we might get an
    // inconsistent value (e.g. in the middle of a write to timer0_millis)
    
    //assembly has better support for 5 byte data type
    //and efficient in shifting

    __asm__ (";uint8_t interruptOn = EA; //to c            \n"
             ";clr and mov won't affect c                  \n"
             "    mov c,_EA                                \n"
             ";EA = 0;                                     \n"
             "    clr _EA                                  \n"
             ";Copy _timer0_overflow_count to local R0~R4  \n"
             "    mov r0, (_timer0_overflow_count)         \n"
             "    mov r1, (_timer0_overflow_count)+1       \n"
             "    mov r2, (_timer0_overflow_count)+2       \n"
             "    mov r3, (_timer0_overflow_count)+3       \n"
             "    mov r4, (_timer0_overflow_count)+4       \n"
             ";if (interruptOn) EA = 1;                    \n"
             "    mov _EA,c                                \n"
             );

#if F_CPU == 16000000
    __asm__ (";return (timer0_overflow_count*48)>>8        \n"
             
             "    mov b, #48                               \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, b                                \n"
             ";lowest 8 bit not used (a), r0 free to use   \n"
             "    mov b, #48                               \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r0                                \n"
             "    mov r5, psw   ;keep c                    \n"
             "    mov dpl, a                               \n"
             "    mov r0, b                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    mov psw, r5   ;restore c                 \n"
             "    addc a, r0                               \n"
             "    mov r5, psw   ;keep c                    \n"
             "    mov dph, a                               \n"
             "    mov r0, b                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    mov psw, r5   ;restore c                 \n"
             "    addc a, r0                               \n"
             "    mov r5, psw   ;keep c                    \n"
             "    mov r1, a                                \n"
             "    mov r0, b                                \n"
             
             "    mov b, #48                               \n"
             "    mov a, r4                                \n"
             "    mul ab                                   \n"
             "    mov psw, r5   ;restore c                 \n"
             "    addc a, r0                               \n"
             
             ";calculation finished, a already in place    \n"
             "    mov b, r1                                \n"
             );
             
#elif F_CPU == 24000000
    __asm__ (";return timer0_overflow_count>>3             \n"
             ";Or: return (timer0_overflow_count<<5)>>8    \n"
             ";Or: return (timer0_overflow_count*32)>>8    \n"
             "    mov b, #32                               \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, b                                \n"
             ";lowest 8 bit not used (a), r0 free to use   \n"
             "    mov b, #32                               \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r0                                \n"
             ";carry won't be set, if I calculated right   \n"
             "    mov dpl, a                               \n"
             "    mov r0, b                                \n"
             
             "    mov b, #32                               \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    add a, r0                                \n"
             ";carry won't be set, if I calculated right   \n"
             "    mov dph, a                               \n"
             "    mov r0, b                                \n"
             
             "    mov b, #32                               \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    add a, r0                                \n"
             ";carry won't be set, if I calculated right   \n"
             "    mov r1, a                                \n"
             "    mov r0, b                                \n"
             
             "    mov b, #32                               \n"
             "    mov a, r4                                \n"
             "    mul ab                                   \n"
             "    add a, r0                                \n"
             ";carry won't be set, if I calculated right   \n"
             
             ";calculation finished, a already in place    \n"
             "    mov b, r1                                \n"
             );
#elif F_CPU == 56000000
    __asm__ (";return timer0_overflow_count*6/125          \n"
             //mutiply by 6
             "    mov b, #6                                \n"
             "    mov a, r0                                \n"
             "    mul ab                                   \n"
             "    mov r0, a                                \n"
             "    mov r5, b                                \n"
             
             "    mov b, #6                                \n"
             "    mov a, r1                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r1, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #6                                \n"
             "    mov a, r2                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r2, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #6                                \n"
             "    mov a, r3                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r3, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             "    mov b, #6                                \n"
             "    mov a, r4                                \n"
             "    mul ab                                   \n"
             "    add a, r5                                \n"
             "    mov r4, a                                \n"
             "    clr a                                    \n"
             "    addc a, b                                \n"
             "    mov r5, a                                \n"
             
             //ref:MCS51 三字节无符号除法程序（ASM）modify for 6 bytes
             "    mov r7,#125                              \n"
             "    mov a,r5                                 \n"
             "    mov b,r7                                 \n"
             "    div ab                                   \n"
             "    mov r5,b                                 \n"
             "    mov r6,a                                 \n"
             "    mov b,#40                                \n"
             ";after keeping high result, iterate all B bit with left shift \n"
             "1$: ;div125_iter_bits                        \n"
             "    clr c                                    \n"
             "    mov a,r0                                 \n"
             "    rlc a                                    \n"
             "    mov r0,a                                 \n"
             "    mov a,r1                                 \n"
             "    rlc a                                    \n"
             "    mov r1,a                                 \n"
             "    mov a,r2                                 \n"
             "    rlc a                                    \n"
             "    mov r2,a                                 \n"
             "    mov a,r3                                 \n"
             "    rlc a                                    \n"
             "    mov r3,a                                 \n"
             "    mov a,r4                                 \n"
             "    rlc a                                    \n"
             "    mov r4,a                                 \n"
             "    mov a,r5                                 \n"
             "    rlc a                                    \n"
             "    mov r5,a                                 \n"
             "    mov f0,c                                 \n"
             "    clr c                                    \n"
             "    subb a,r7                                \n"
             ";get 1 for result with overflow on shift or non-negitive subb result \n"
             "    jb f0,2$                                 \n"
             "    jc 3$    ;get 0 for this bit             \n"
             "2$: ;shift overflow                          \n"
             "    mov r5,a                                 \n"
             "    inc r0                                   \n"
             "3$: ;iternate next bit                       \n"
             "    djnz b,1$                                \n"
             "    mov a,r6                                 \n"
             "    clr ov                                   \n"
             "    jz 4$                                    \n"
             "    setb ov                                  \n"
             "4$: ;finish calculation, set high byte and reminder \n"
             "    xch a,r5                                 \n"
             "    mov r7,a                                 \n"
             
             ";return                                      \n"
             "    mov dpl, r0                              \n"
             "    mov dph, r1                              \n"
             "    mov b, r2                                \n"
             "    mov a, r3                                \n"

             );
#else
    #error "clock not supported yet"

#endif
}

void delay(uint32_t ms)
{
    uint32_t start = micros();
    
    while (ms > 0) {
        //yield();
        while ( ms > 0 && (micros() - start) >= 1000) {
            ms--;
            start += 1000;
        }
    }
}

void delayMicroseconds(uint16_t us){
#ifdef    F_CPU
#if        F_CPU <= 6000000
    us >>= 2;
#endif
#if        F_CPU <= 3000000
    us >>= 2;
#endif
#if        F_CPU <= 750000
    us >>= 4;
#endif
#endif
    
    // in arduino setup, the code above seems useless (clock generally much higher than 6M).
    
    while ( us ) {  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
//        ++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here

#ifdef    F_CPU
#if        F_CPU >= 14000000
//        ++ SAFE_MOD;
#endif
#if        F_CPU >= 16000000
//        ++ SAFE_MOD;  //seems SDCC is not very efficent, dirty tweak for a little more accuracy, but still not very accurate. accurate time need carefull planning and word alignment. Maybe for later.
#endif
#if        F_CPU >= 18000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 20000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 22000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 24000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 26000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 28000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 30000000
        ++ SAFE_MOD;
#endif
#if        F_CPU >= 32000000
        ++ SAFE_MOD;
#endif
#endif
        -- us;
    }
}

void init()
{
    //set internal clock 
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    
#if defined(CH551) || defined(CH552)
#if F_CPU == 32000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x07;  // 32MHz
#elif F_CPU == 24000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;  // 24MHz
#elif F_CPU == 16000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x05;  // 16MHz
#elif F_CPU == 12000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x04;  // 12MHz
#elif F_CPU == 6000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x03;  // 6MHz
#elif F_CPU == 3000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x02;  // 3MHz
#elif F_CPU == 750000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x01;  // 750KHz
#elif F_CPU == 187500
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x00;  // 187.5KHz
#else
#warning F_CPU invalid or not set
#endif

#elif defined(CH549)
#if F_CPU == 48000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x07;  // 48MHz
#elif F_CPU == 32000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;  // 32MHz
#elif F_CPU == 24000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x05;  // 24MHz
#elif F_CPU == 16000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x04;  // 16MHz
#elif F_CPU == 12000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x03;  // 12MHz
#elif F_CPU == 3000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x02;  // 3MHz
#elif F_CPU == 750000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x01;  // 750KHz
#elif F_CPU == 187500
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x00;  // 187.5KHz
#else
#warning F_CPU invalid or not set
#endif
    
#elif defined(CH559)
#if F_CPU == 24000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_DIV | 12;  // 24MHz, 12M*(24 default PLL)/12=24M
#elif F_CPU == 16000000
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_DIV | 18;  // 16MHz
#elif F_CPU == 56000000
    PLL_CFG = (7<<5) | (28);    //Fusb4x = 12M*(28 PLL)/7=48M
    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_DIV | 6;  // 56MHz, 12M*(28 PLL)/6=56M
#else
#warning F_CPU invalid or not set
#endif
    
#endif
    
    SAFE_MOD = 0x00;
    
    delayMicroseconds(5000); //needed to stablize internal RC
    
#ifndef USER_USB_RAM
    //init USB
    USBDeviceCfg();
    USBDeviceEndPointCfg();                                               //????
    USBDeviceIntCfg();                                                    //?????
    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;                                                       //????????????
    UEP2_T_LEN = 0;                                                       //????????????
#endif
    
    //init PWM
    PWM_CK_SE = 93;        //DIV by 94 for 1K freq on 24M clk
    PWM_CTRL = 0;
    
    //init T0 for millis
    TMOD = (TMOD & ~0x0F)|(bT0_M1);//mode 2 for autoreload
    T2MOD = T2MOD & ~bT0_CLK;    //bT0_CLK=0;clk Div by 12
    TH0 = 255-T0_CYCLE+1;
    TF0 = 0;
    ET0 = 1;
    TR0 = 1;
    
    EA = 1; //millis and delay needs interrupt
}

