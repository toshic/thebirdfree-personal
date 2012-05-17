/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Acts on stream open and closure. 
*/

#ifndef AUDIOADAPTOR_STREAMMANAGER_H
#define AUDIOADAPTOR_STREAMMANAGER_H


void streamManagerOpenNotify (void);

void streamManagerOpenComplete (bool status);

void streamManagerCloseComplete (void);


#endif /* AUDIOADAPTOR_STREAMMANAGER_H */



