/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Deals with connecting and disconnecting A2DP, and provides helper functions
    for determining active connections.
*/

#ifndef AUDIOADAPTOR_A2DP_SLC_H
#define AUDIOADAPTOR_A2DP_SLC_H


bool a2dpSlcConnect(devInstanceTaskData *inst);

bool a2dpSlcReOpen(devInstanceTaskData *inst);

void a2dpSlcDisconnectAll(void);

uint16 a2dpSlcIsMediaOpen(void);

bool a2dpSlcIsDifferentMediaOpen(devInstanceTaskData *inst);

bool a2dpSlcIsMediaStreaming(void);

bool a2dpSlcIsDifferentMediaStreaming(devInstanceTaskData *inst);

bool a2dpSlcIsMediaStarting(void);


#endif /* AUDIOADAPTOR_A2DP_SLC_H */

