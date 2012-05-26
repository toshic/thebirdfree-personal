/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_inquiry.h
@brief   Handles proximity pairing inquiries
*/
#ifndef HEADSET_INQUIRY_H
#define HEADSET_INQUIRY_H


#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************
NAME    
    inquiryContinue
    
DESCRIPTION
    Continue with inquiry 
*/
void inquiryContinue(void);


/****************************************************************************
NAME    
    inquiryClearDataclearInquiryData
    
DESCRIPTION
    Reset inquiry data to initial values
*/
void inquiryClearData(void);


/****************************************************************************
NAME    
    inquiryStart
    
DESCRIPTION
    Kick off Inquiry
*/
void inquiryStart( void );


/****************************************************************************
NAME    
    inquiryStop
    
DESCRIPTION
    If inquiry in progress throw away the results and cancel further CL messages
*/
void inquiryStop( void );


/****************************************************************************
NAME    
    inquiryHandleResult
    
DESCRIPTION
    Inquiry result received
*/
void inquiryHandleResult( CL_DM_INQUIRE_RESULT_T* result );


/****************************************************************************
NAME    
    inquiryReminder
    
DESCRIPTION
    Send inquiry reminder
*/
void inquiryReminder(void);


#endif
