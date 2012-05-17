/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Call functionality is handled in this file.
*/

#ifndef AUDIOADAPTOR_AGHFP_CALL_H
#define AUDIOADAPTOR_AGHFP_CALL_H

#include <aghfp.h>


void aghfpCallCreate (devInstanceTaskData *inst, aghfp_call_type call_type);

void aghfpCallAnswer (aghfp_call_type call_type);

void aghfpCallCancel (aghfp_call_type call_type);

void aghfpCallEnd (void);

void aghfpCallOpenComplete (bool status);

void aghfpCallCloseComplete (void);


#endif /* AUDIOADAPTOR_AGHFP_CALL_H */
