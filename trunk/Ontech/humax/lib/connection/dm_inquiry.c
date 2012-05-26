/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    dm_inquiry.c        

DESCRIPTION
    This file contains the implementation of the inquiry management
    entity. This is responsible for ensuring that only one inquiry
    request at a time is issued to Bluestack. It also manages the 
    inquiry process by filtering on class of device and initiating
    remote name requests (if requested by the client).

NOTES
	An inquiry can only be initiated by one task at a time. If an inquiry
	request is received while the connection lib is already performing an 
	inquiry a CL_DM_INQUIRE_RESULT message is returned with status set to
	busy.

	Setting the Class of Device field to zero will turn off class of device
	filtering of inquiry results and all devices found will be returned.
*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <vm.h>
#include <string.h>


/*****************************************************************************/
void ConnectionReadLocalName(Task theAppTask)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_LOCAL_NAME_REQ);
   message->theAppTask = theAppTask;
   MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_LOCAL_NAME_REQ, message, &theCm.inqState.nameLock);
}


/*****************************************************************************/
void ConnectionWriteInquiryMode(Task theAppTask, inquiry_mode mode)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ);
	message->theAppTask = theAppTask;
	message->mode = mode;
	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ, message, &theCm.inqState.inquiryLock);
}


/*****************************************************************************/
void ConnectionWriteEirData(uint8 fec_required, uint8 size_eir_data, const uint8 *eir_data)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_EIR_DATA_REQ);
	message->fec_required = fec_required;
	message->size_eir_data = (size_eir_data <= HCI_EIR_DATA_LENGTH)? size_eir_data : HCI_EIR_DATA_LENGTH;

	if (message->size_eir_data)
	{
		message->eir_data = (uint8 *)PanicUnlessMalloc(message->size_eir_data);
		memmove(message->eir_data, eir_data, message->size_eir_data);
	}
	else
	{
		message->eir_data = 0;
	}
	MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_EIR_DATA_REQ, message);
}

