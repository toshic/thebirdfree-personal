/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    profile_handler.h
    
DESCRIPTION
	Header file for the AVRCP profile library.
*/

#ifndef AVRCP_PROFILE_HANDLER_H_
#define AVRCP_PROFILE_HANDLER_H_


/****************************************************************************
NAME	
	avrcpProfileHandler

DESCRIPTION
	All messages for the profile lib instance are handled by this function.
*/
void avrcpProfileHandler(Task task, MessageId id, Message message);


#endif /* AVRCP_PROFILE_HANDLER_H */
