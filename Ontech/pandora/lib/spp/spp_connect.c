/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of BlueLab 4.1.2-Release 

FILE NAME
	spp_connect.c        

DESCRIPTION
    API functions for establishing an SPP connection.

*/

#include "spp_private.h"
#include "spp.h"
#include "init.h"
#include "spp_connect_handler.h"

/*****************************************************************************
	Defining frame size limits and defaults.  NB. Request limits greater than
	response limits as requested frame size is decremented in connection lib
*****************************************************************************/

#define RFCOMM_MAX_REQ_FRAME_SIZE 32767
#define RFCOMM_MAX_RES_FRAME_SIZE 32767
#define RFCOMM_MIN_REQ_FRAME_SIZE 24
#define RFCOMM_MIN_RES_FRAME_SIZE 23
#define RFCOMM_DEFAULT_FRAME_SIZE 650

/************************************************************/
static void sendInternalSppConnectMsg(SPP *spp, const bdaddr *bd_addr, const spp_connect_params *config)
{	
	if (config)
	{
		/* If frame size is invalid */
		if((config->max_frame_size < RFCOMM_MIN_REQ_FRAME_SIZE || config->max_frame_size > RFCOMM_MAX_REQ_FRAME_SIZE) && config->max_frame_size != 0)
		{
			/* Message app about connect failure */
			sppSendConnectCfmToApp(spp_connect_invalid_frame_size,spp);
		}
		else
		{
			/* Send connect request using specified params */
			MAKE_SPP_MESSAGE(SPP_INTERNAL_CONNECT_REQ);
		    message->addr = *bd_addr;
			message->rfcomm_channel_number = config->rfcomm_channel_number;
			if(config->max_frame_size != 0)
			{
				message->max_frame_size = config->max_frame_size;
			}
			else
			{
				message->max_frame_size = RFCOMM_DEFAULT_FRAME_SIZE; 
			}
			
			if (config->size_search_pattern)
       	 	{
           		message->length_service_search_pattern = config->size_search_pattern;
            	message->service_search_pattern = config->search_pattern;
        	}
        	else
        	{
            	message->length_service_search_pattern = 0;
            	message->service_search_pattern = 0;
        	}   
			
			MessageSend(&spp->task, SPP_INTERNAL_CONNECT_REQ, message);	
		} 
	}
	else
	{
		/* Send connect request using default params */
		MAKE_SPP_MESSAGE(SPP_INTERNAL_CONNECT_REQ);
	    message->addr = *bd_addr;
		message->rfcomm_channel_number = 0;
		message->length_service_search_pattern = 0;
        message->service_search_pattern = 0;
		message->max_frame_size = RFCOMM_DEFAULT_FRAME_SIZE;
		MessageSend(&spp->task, SPP_INTERNAL_CONNECT_REQ, message);	
	}
}


/************************************************************/
static void sendInternalSppConnectResponse(SPP *spp, const bdaddr *bd_addr, bool resp, uint16 max_frame_size)
{
	MAKE_SPP_MESSAGE(SPP_INTERNAL_CONNECT_RES);
	message->addr = *bd_addr;
	
	/* If max frame size is invalid */
	if((max_frame_size < RFCOMM_MIN_RES_FRAME_SIZE || max_frame_size > RFCOMM_MAX_RES_FRAME_SIZE) && max_frame_size != 0)
	{	
		/* Reject Request */		
		message->max_frame_size = 0;
		message->response = 0;	
		MessageSend(&spp->task, SPP_INTERNAL_CONNECT_RES, message);		

		/* Message app about failure */
		{
		MAKE_SPP_MESSAGE(SPP_INTERNAL_SEND_CFM_TO_APP);
		message->status = spp_connect_invalid_frame_size;
		message->spp = spp;
		MessageSend(&spp->task, SPP_INTERNAL_SEND_CFM_TO_APP, message);
		}
	}
	else
	{	
		/* Respond to request */
    	message->response = resp;
		if(max_frame_size != 0)
		{
			message->max_frame_size = max_frame_size;
		}
		else
		{
			message->max_frame_size = RFCOMM_DEFAULT_FRAME_SIZE;
		}	
		
		MessageSend(&spp->task, SPP_INTERNAL_CONNECT_RES, message);			
	}				
}


/************************************************************/
void SppConnect(SPP *spp, const bdaddr *bd_addr)
{
#ifdef SPP_DEBUG_LIB
	if (!bd_addr)
		SPP_DEBUG(("Null address ptr passed in.\n"));
#endif

    /* Send an internal connect message */
    sendInternalSppConnectMsg(spp, bd_addr, 0);
}


/************************************************************/
void SppConnectEx(SPP *spp, const bdaddr *bd_addr, const spp_connect_params *config)
{
    /* Send an internal connect message */
    sendInternalSppConnectMsg(spp, bd_addr, config);
}


/************************************************************/
void SppConnectLazy(const bdaddr *bd_addr, uint16 priority, Task appTask, const spp_connect_params *config)
{
#ifdef SPP_DEBUG_LIB
	if (!bd_addr)
		SPP_DEBUG(("Null address ptr passed in.\n"));
#endif

    {
        /* Create the SPP task here */
        SPP *spp = sppCreateTaskInstance(priority, 0, appTask, sppReady, 0, 0, 0, 0, 1);

        /* Send an internal connect message */
        sendInternalSppConnectMsg(spp, bd_addr, config);
    }
}


/************************************************************/
void SppConnectResponse(SPP *spp, bool response, const bdaddr *bd_addr)
{
#ifdef SPP_DEBUG_LIB
	if (!bd_addr)
		SPP_DEBUG(("Null address ptr passed in.\n"));
#endif

    /* Send an internal response message */
    sendInternalSppConnectResponse(spp, bd_addr, response, 0);
}


/************************************************************/
void SppConnectResponseEx(SPP *spp, bool response, const bdaddr *bd_addr, uint16 max_frame_size)
{
#ifdef SPP_DEBUG_LIB
	if (!bd_addr)
		SPP_DEBUG(("Null address ptr passed in.\n"));
#endif

    /* Send an internal response message */
    sendInternalSppConnectResponse(spp, bd_addr, response, max_frame_size);
}


/************************************************************/
void SppConnectResponseLazy(SPP *spp, bool response, const bdaddr *bd_addr, uint16 priority, uint16 max_frame_size)
{
#ifdef SPP_DEBUG_LIB
	if (!bd_addr)
		SPP_DEBUG(("Null address ptr passed in.\n"));
#endif
 
    /* Complete spp task init since app needs to fill in the priority */
    sppInitTaskData(spp, priority, 0, spp->clientTask, spp->state, spp->local_server_channel, 0, 0, 0, 1);

    /* Send an internal connect response message */
    sendInternalSppConnectResponse(spp, bd_addr, response, max_frame_size);
}


/************************************************************/
void SppDisconnect(SPP *spp)
{
	/* Send an internal message to kick off the disconnect */
	MessageSend(&spp->task, SPP_INTERNAL_DISCONNECT_REQ, 0);
}
