/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_common.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_COMMON_H_
#define AVRCP_COMMON_H_


/****************************************************************************
NAME	
	avrcpSetState

DESCRIPTION
	Update the avrcp state.
*/
void avrcpSetState(AVRCP *avrcp, avrcpState state);


/****************************************************************************
NAME	
	avrcpSendCommonCfmMessageToApp

DESCRIPTION
	Create a common cfm message (many messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.

RETURNS
	void
*/
void avrcpSendCommonCfmMessageToApp(uint16 message_id, avrcp_status_code status, Sink sink, AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleDeleteTask

DESCRIPTION
	Detele a dynamically allocated AVRCP task instance. Before deleting make 
    sure all messages for that task are flushed from the message queue.
*/
void avrcpHandleDeleteTask(AVRCP *avrcp);


#endif /* AVRCP_COMMON_H_ */
