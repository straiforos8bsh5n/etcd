/*
  main.cpp - Main loop for Arduino sketches
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

// make sure to define prototypes for all used interrupts
void USBInterrupt(void);

//unsigned char runSerialEvent;

void DeviceUSBInterrupt(void) __interrupt (INT_NO_USB)                       //USB interrupt service
{
    USBInterrupt();
}

// 0x08~0x0F belongs to register bank 1
__idata __at (0x08) volatile uint32_t timer0_overflow_count = 0;
__idata __at (0x0C) volatile uint8_t timer0_overflow_count_5th_byte = 0;

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

void Uart0_ISR(void) __interrupt (INT_NO_UART0)
{
    if (RI){
        uart0IntRxHandler();        
        RI =0;
    }
    if (TI){
        uart0IntTxHandler();
        TI =0;
    }
}

typedef void (*voidFuncPtr)(void);
extern __xdata voidFuncPtr intFunc[];
void INT0_ISR(void) __interrupt (INT_NO_INT0)
{
    intFunc[0]();
}
void INT1_ISR(void) __interrupt (INT_NO_INT1)
{
    intFunc[1]();
}

__xdata voidFuncPtr touchKeyHandler = NULL;
void TOUCHKEY_ISR(void) __interrupt (INT_NO_TKEY)
{
    if (touchKeyHandler!=NULL){
        touchKeyHandler();
    }
}


void main(void)
{
    init();
    
    //!!!initVariant();
    
    setup();
    
    for (;;) {
        loop();
        if (1) {
#ifndef USER_USB_RAM
            USBSerial_flush();
#endif
            //serialEvent();
        }
    }
    
    //    return 0;
}

unsigned char _sdcc_external_startup (void) __nonbanked
{
    return 0;
}


