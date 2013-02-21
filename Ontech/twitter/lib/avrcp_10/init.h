/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    init.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_INIT_H_
#define AVRCP_INIT_H_


/****************************************************************************
NAME	
	avrcpInitTaskData

DESCRIPTION
	Initialise all members of the AVRCP task data structure.
*/
void avrcpInitTaskData(AVRCP *avrcp, Task client, avrcpState state, avrcp_device_type dev, uint16 lazy);


/****************************************************************************
NAME	
	avrcpHandleInternalInitReq

DESCRIPTION
	Init request messages are sent internally withing the profile instance
	until its initialisation has completed. These messages are handled in
	this function.
*/
void avrcpHandleInternalInitReq(AVRCP *avrcp, const AVRCP_INTERNAL_INIT_REQ_T *req);


/****************************************************************************
NAME	
	avrcpSendInitCfmToClient

DESCRIPTION
	Send an AVRCP_INIT_CFM message to the client task.
*/
void avrcpSendInitCfmToClient(Task clientTask, AVRCP *avrcp, avrcp_status_code status);


#endif /* AVRCP_INIT_H_ */
