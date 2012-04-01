/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include "chardevice.h"
#include "telnet.h"

#pragma import(__use_no_semihosting_swi)
extern int lcd_terminal_char(int ch); /* in LCD.c */
extern int  sendchar(int ch);  /* in Serial.c */
extern int  getkey(void);      /* in Serial.c */
extern long timeval;           /* in Time.c   */



struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stderr;
FILE __stdin;

FILE __uartout;
FILE __uartin;

FILE __zigbeeout;
FILE __zigbeein;

int fputc(int ch, FILE *f) {
    if(f == &__stdout){
//        return (sendchar(ch));
        telnet_putchar_all(ch);
        return (console_putchar(ch));
    }else if( f == &__stderr){
        telnet_putchar_all(ch);
        return (lcd_terminal_char(ch));
    }else if( f == &__uartout){
        return (console_putchar(ch));
    }else
        return telnet_putchar(f,ch);
}

int fgetc(FILE *f) {
//  return (getkey());
    return (console_getchar());
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
//  sendchar (ch);
    telnet_putchar_all(ch);
    console_putchar(ch);
}


void _sys_exit(int return_code) {
  while (1);    /* endless loop */
}

