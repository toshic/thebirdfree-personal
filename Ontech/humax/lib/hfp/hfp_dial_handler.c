/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_dial_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_dial_handler.h"
#include "hfp_indicators_handler.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


/* 
	The code for creating the dial cmd for dialing a number and a memory location 
	is almost the same so pull out common code into this function.
*/
static void createDialCmd(HFP *hfp, const char *atd_prefix, const char *atd_suffix, uint16 length, const uint8 *number)
{
	uint16 len_prefix = strlen(atd_prefix);
	uint16 len_suffix = strlen(atd_suffix);

	/* Create the dial request message */
	char *atd_cmd = (char *) PanicUnlessMalloc(len_prefix + length + len_suffix);
	memmove(atd_cmd, atd_prefix, len_prefix);
	memmove(atd_cmd+len_prefix, number, length);
	memmove(atd_cmd+len_prefix+length, atd_suffix, len_suffix);

	/* Send the message over the air */
	hfpSendAtCmd(&hfp->task, len_prefix + length + len_suffix, atd_cmd);

	/* Free up the allocated memory since the cmd has been copied into the RFC buffer */
	free(atd_cmd);
}


/****************************************************************************
NAME	
	hfpHandleLastNumberRedial

DESCRIPTION
	Issue a last number redial request.

RETURNS
	void
*/
void hfpHandleLastNumberRedial(HFP *hfp)
{
	static const char bldn[] = "AT+BLDN\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(bldn), bldn);
}


/****************************************************************************
NAME	
	hfpHandleLastNumberRedialError

DESCRIPTION
	The last number edial request received from the app has arrive while the
	profile instance is not in the correct state so send back an error
	response to the app.

RETURNS
	void
*/
void hfpHandleLastNumberRedialError(HFP *hfp)
{
	/* Send an error message to the app. */
	hfpSendCommonCfmMessageToApp(HFP_LAST_NUMBER_REDIAL_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBldnAtAck

DESCRIPTION
	Ack received for AT+BLDN AT cmd.

RETURNS
	void
*/
void hfpHandleBldnAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_LAST_NUMBER_REDIAL_CFM, hfp, status);

	/* This is necessary when dealing with 0.96 AGs */
	if (!hfp->indicator_status.indexes.call_setup && (status == hfp_success) &&
		supportedProfileIsHfp(hfp->hfpSupportedProfile))
    {
		hfpSendIndicatorCallSetupToApp(hfp, hfp_outgoing_call_setup);
    }
}


/****************************************************************************
NAME	
	hfpHandleDialNumberRequest

DESCRIPTION
	Issue a request to dial the supplied number.

RETURNS
	void
*/
void hfpHandleDialNumberRequest(HFP *hfp, const HFP_INTERNAL_AT_ATD_NUMBER_REQ_T *req)
{
	/* Create and send the dial cmd */
	createDialCmd(hfp, "ATD", ";\r", req->length, req->number);
	
	/* Free the number we allocated when the message was created */
	free(req->number);	
}


/****************************************************************************
NAME	
	hfpHandleDialNumberRequestError

DESCRIPTION
	received a dial number request while the profile instance was in the
	wrong state so send back an error message indicating this.

RETURNS
	void
*/
void hfpHandleDialNumberRequestError(HFP *hfp)
{
	/* Send back a cfm with the status set to error */
	hfpSendCommonCfmMessageToApp(HFP_DIAL_NUMBER_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleAtdNumberAtAck

DESCRIPTION
	Received an ack for the ATD cmd.

RETURNS
	void
*/
void hfpHandleAtdNumberAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_DIAL_NUMBER_CFM, hfp, status);

	/* This is necessary when dealing with 0.96 AGs */
	if (!hfp->indicator_status.indexes.call_setup && (status == hfp_success) &&
		supportedProfileIsHfp(hfp->hfpSupportedProfile))
    {
		hfpSendIndicatorCallSetupToApp(hfp, hfp_outgoing_call_setup);
    }
}


/****************************************************************************
NAME	
	hfpHandleDialMemoryRequest

DESCRIPTION
	Send a dial memory location request to the AG.

RETURNS
	void
*/
void hfpHandleDialMemoryRequest(HFP *hfp, const HFP_INTERNAL_AT_ATD_MEMORY_REQ_T *req)
{
	/* Create and send the dial request */
	createDialCmd(hfp, "ATD>", ";\r", req->length, req->memory);

	/* Free the memory allocated when the message was created */
	free(req->memory);
}


/****************************************************************************
NAME	
	hfpHandleDialMemoryRequestError

DESCRIPTION
	Received a dial memory request while the profile instance was in the
	wrong state so send back an error message indicating this.

RETURNS
	void
*/
void hfpHandleDialMemoryRequestError(HFP *hfp)
{
	/* Send an error message */
	hfpSendCommonCfmMessageToApp(HFP_DIAL_MEMORY_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleAtdMemoryAtAck

DESCRIPTION
	Received an ack for the ATD> cmd.

RETURNS
	void
*/
void hfpHandleAtdMemoryAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_DIAL_MEMORY_CFM, hfp, status);

	/* This is necessary when dealing with 0.96 AGs */
	if (!hfp->indicator_status.indexes.call_setup && (status == hfp_success) &&
		supportedProfileIsHfp(hfp->hfpSupportedProfile))
    {
		hfpSendIndicatorCallSetupToApp(hfp, hfp_outgoing_call_setup);
    }
}
