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

#if 1
//#pragma import(__use_no_semihosting_swi)
#define LCD_PRINTF

#ifdef LCD_PRINTF
extern int lcd_terminal_char(int ch); /* in LCD.c */
#else
extern int  sendchar(int ch);  /* in Serial.c */
#endif
extern int  getkey(void);      /* in Serial.c */
extern long timeval;           /* in Time.c   */



struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


int fputc(int ch, FILE *f) {
#ifdef LCD_PRINTF
  return (lcd_terminal_char(ch));
#else  
  return (sendchar(ch));
#endif
}

int fgetc(FILE *f) {
  return (getkey());
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
#ifdef LCD_PRINTF
  lcd_terminal_char(ch);
#else  
  sendchar (ch);
#endif  
}


void _sys_exit(int return_code) {
  while (1);    /* endless loop */
}
#endif
