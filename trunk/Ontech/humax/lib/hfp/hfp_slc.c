/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_slc.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_slc_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME	
	HfpSlcConnect

DESCRIPTION
	This function is used to initate the creation of a Service Level
	Connection (SLC). The profile instance, hfp, which will be used to create
	the connection is passed into the function. 
	
	The bd_addr specifies the address of the remote device (the Audio Gateway 
	or AG) to which the connection will be created.
	
	The HFP defines three indicators, (service, call and call_setup) that the 
	AG must send to the HFP device. However, the GSM specification defines a 
	number of other indicators the AG may support. The application can use 
	extra_indicators to specify that it wants to be notified whether the AG 
	supports a particular indicator and of its index in the CIND response. 
	The extra_indicators should be specified as a string with carriage returns 
	(\r) separating each indicator. For example, the extra_indicators string 
	might looks as follows, "battchg\rroam\rsounder". It is important that 
	spaces are not inserted into the string. The application will be notified 
	of the indicator's index (if it is supported by the AG) using a 
	HFP_EXTRA_INDICATOR_INDEX_IND message. If extra indicator reporting is 
	enabled then all indicator updates will be passed to the application 
	(using a HFP_EXTRA_INDICATOR_UPDATE_IND message) and not just the 
	indicators the application has registered an interest in. It is left up 
	to the application to determine whether the indicator update is of 
	interest. Most applications will not want to enable this functionality, 
	in which case extra_indicators should be set to null.

MESSAGE RETURNED
	HFP_SLC_CONNECT_CFM

RETURNS
	void
*/
void HfpSlcConnect(HFP *hfp, const bdaddr *bd_addr, const uint8 *extra_indicators)
{
#ifdef HFP_DEBUG_LIB
	if (!bd_addr)
		HFP_DEBUG(("Null address ptr passed in.\n"));

	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

		if (hfp->state == hfpReady)
		{
			HFP_INTERNAL_SLC_CONNECT_REQ_T message ;
			
			message.addr = *bd_addr;

		    /* Copy the config string of extra indicators, if supplied */
		  if (extra_indicators)
		  {
			    uint16 length = strlen((char *) extra_indicators)+1;
			    message.extra_indicators = (uint8 *) PanicUnlessMalloc(length);
			    memmove(message.extra_indicators, extra_indicators, length);
		  }
		  else
			{
			    message.extra_indicators = 0;
		  }
		    hfpHandleSlcConnectRequest(hfp, &message) ;	
		}

}


/****************************************************************************
NAME	
	HfpSlcConnectResponse

DESCRIPTION
	If the AG initiates an SLC to the HFP device the application will be
	notified about the incoming connection using a HFP_SLC_CONNECT_IND
	message. The application must respond to the message using this function.
	The profile instance to which the connection will be created is specified 
	by hfp. This must be the same as the profile instance in the
	HFP_SLC_CONNECT_IND message as the AG will be attempting to connect to a
	particular service on the local device. 
	
	The response is a boolean telling the HFP library whether to accept or 
	reject the incoming connection attempt. 
	
	The Bluetooth address of the device that initiated to the SLC is passed 
	in using bd_addr. This should be the same as the address in the 
	HFP_SLC_CONNECT_IND message. 
	
	The extra_indicators field is the same as described above.

MESSAGE RETURNED
	HFP_SLC_CONNECT_CFM

RETURNS
	void
*/
void HfpSlcConnectResponse(HFP *hfp, bool response, const bdaddr *bd_addr, const uint8 *extra_indicators)
{
#ifdef HFP_DEBUG_LIB
	if (!bd_addr)
		HFP_DEBUG(("Null address ptr passed in.\n"));

	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	{
		/* Send an internal message to kick off SLC creation */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_SLC_CONNECT_RES);
		message->addr = *bd_addr;
		message->response = response;
		
		/* Copy the config string of extra indicators, if supplied */
		if (extra_indicators)
		{
			uint16 length = strlen((char *) extra_indicators)+1;

			message->extra_indicators = (uint8 *) PanicUnlessMalloc(length);

			memmove(message->extra_indicators, extra_indicators, length);
		}
		else
			message->extra_indicators = 0;

		MessageSend(&hfp->task, HFP_INTERNAL_SLC_CONNECT_RES, message);
	}
}


/****************************************************************************
NAME	
	HfpSlcDisconnect

DESCRIPTION
	This function initiates the disconnection of an SLC for a particular 
	profile instance (hfp).

MESSAGE RETURNED
	HFP_SLC_DISCONNECT_IND

RETURNS
	void
*/
void HfpSlcDisconnect(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message to kick off the disconnect */
	MessageSend(&hfp->task, HFP_INTERNAL_SLC_DISCONNECT_REQ, 0);
}


/****************************************************************************
NAME	
	HfpGetSink

DESCRIPTION
	Some function calls to the connection library require a Sink to be 
	suplpied to identify the connection instance. Using this function the
	application can obtain the Sink associated with the SLC for a particular
	profile instance (hfp). If the profile instance supplied currently does
	not have an SLC established the message returned will contain an error
	status and an invalid Sink.

MESSAGE RETURNED
	HFP_SINK_CFM

RETURNS
	void
*/
void HfpGetSink(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message to kick off the request */
	MessageSend(&hfp->task, HFP_INTERNAL_GET_SINK_REQ, 0);
}
