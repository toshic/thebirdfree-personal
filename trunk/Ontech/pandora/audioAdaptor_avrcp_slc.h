/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Defines data structures used internally throughout the application.
*/

#ifndef AUDIOADAPTOR_AVRCP_SLC_H
#define AUDIOADAPTOR_AVRCP_SLC_H


#include <avrcp.h>


bool avrcpSlcConnect(devInstanceTaskData *inst);

void avrcpSlcDisconnectAll(void);
    

#endif /* AUDIOADAPTOR_AVRCP_SLC_H */

