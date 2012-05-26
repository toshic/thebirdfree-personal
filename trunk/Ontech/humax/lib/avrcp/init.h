/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    init.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_INIT_H_
#define AVRCP_INIT_H_


/****************************************************************************
NAME	
	avrcpHandleInternalInitReq

DESCRIPTION
	Init request messages are sent internally withing the profile instance
	until its initialisation has completed. These messages are handled in
	this function.
*/
void avrcpHandleInternalInitReq(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpSendInternalInitCfm

DESCRIPTION
	Send an internal init cfm message.
*/

void avrcpSendInternalInitCfm(Task task, avrcp_status_code status);


/****************************************************************************
NAME	
	avrcpHandleInternalInitCfm

DESCRIPTION
	This message is sent once various parts of the library initialisation 
	process have completed.
*/
void avrcpHandleInternalInitCfm(AVRCP *avrcp, const AVRCP_INTERNAL_INIT_CFM_T *cfm);


#endif /* AVRCP_INIT_H_ */
