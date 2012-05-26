/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    sdp_handler.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "sdp_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>


/*lint -e725 -e830 */

typedef enum 
{
	sdp_start_search,
	sdp_lock_set,
	sdp_search_lock_set,
	sdp_session_open_other_dev
} sdp_search_req;


/* Request the service handle(s) of the HSP service at the AG */
static const uint8 SdpPingServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x00, 0x00 /* 4 byte UUID - will be filled in by app */
};

/****************************************************************************
NAME	
	sendSdpCloseSearchCfm

DESCRIPTION
	Issue a confirm message to the task that requested the search be closed

RETURNS
	void	
*/
static void sendSdpCloseSearchCfm(Task appTask, sdp_close_status cfm_result)
{
    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_SDP_CLOSE_SEARCH_CFM);
        message->status = cfm_result;
        MessageSend(appTask, CL_SDP_CLOSE_SEARCH_CFM, message);
    }
}


/****************************************************************************
NAME	
	sendSearchRequest

DESCRIPTION
	Decide whather a search request primitive can be sent to BlueStack or not

RETURNS
	sdp_search_req - see typedef above
*/
static sdp_search_req sendSearchRequest(Task theAppTask, const connectionSdpState *state, const bdaddr *bd_addr)
{
	/* Check the first lock */
	if (state->sdpLock)
	{
		/* SDP search session open or non search primitive sent to BlueStack */
		if (BdaddrIsZero(&state->sdpServerAddr))
		{
			/* 
				SDP search session not open but another prim is currently 
				being handled (sdpLock set) so we can't start the search yet
			*/
			return sdp_lock_set;
		}
		else
		{
			/* Addr set - SDP search session open */
			if (state->sdpSearchLock)
			{
				/* Search currently active so can't start another one */
				return sdp_search_lock_set;
			}
			else
			{
				/* SDP session open but no active search */
				if (BdaddrIsSame(&state->sdpServerAddr, bd_addr))
				{
					/* 
						SDP search session open to the device we want to search 
						so we can just start our search
					*/
					return sdp_start_search;
				}
				else
				{
					/* 
						SDP search session open to a different device so we can't
						do our search until this session is closed.
					*/
					return sdp_session_open_other_dev;
				}
			}
		}
	}
	else
	{
		/* Check if we currently have a search active, block internal requests if sdpLock not set */
		if (state->sdpSearchLock || theAppTask == connectionGetCmTask())
		{
			/* Search currently active */
			return sdp_search_lock_set;
		}
		else
		{
			/* No search currently active */
			return sdp_start_search;
		}
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpRegisterRequest

DESCRIPTION
	Sends a register request to the SDP server

RETURNS
	void	
*/
void connectionHandleSdpRegisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T *req)
{
	MAKE_PRIM_T(SDS_REGISTER_REQ);
	prim->phandle = 0;
    prim->num_rec_bytes = req->record_length;
	prim->service_rec = VmGetHandleFromPointer(req->record);
	VmSendSdpPrim(prim);

	/* Set the lock */
	state->sdpLock = req->theAppTask;
}


/****************************************************************************
NAME	
	connectionHandleSdpRegisterCfm

DESCRIPTION
	Handle the register response received from BlueStack

RETURNS
	void	
*/
void connectionHandleSdpRegisterCfm(connectionSdpState *state, const SDS_REGISTER_CFM_T *cfm)
{
    if (state->sdpLock)
    {
        /* Send response to the task that originated the request */
        MAKE_CL_MESSAGE(CL_SDP_REGISTER_CFM);
        message->status = (connection_lib_status) cfm->result;
        message->service_handle = cfm->svc_rec_hndl;
        MessageSend(state->sdpLock, CL_SDP_REGISTER_CFM, message);
        
        /* Reset the lock */
        state->sdpLock = 0;
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpUnregisterRequest

DESCRIPTION
	Handle the request to unregister a particular service record

RETURNS
	void	
*/
void connectionHandleSdpUnregisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T *req)
{
	MAKE_PRIM_T(SDS_UNREGISTER_REQ);
	prim->phandle = 0;
	prim->svc_rec_hndl = req->service_handle;
	VmSendSdpPrim(prim);
	
	/* Set lock */
	state->sdpLock = req->theAppTask;
}


/****************************************************************************
NAME	
	connectionHandleSdpUnregisterCfm

DESCRIPTION
	Handle the response from BlueStack to an SDP unregister request

RETURNS
	void	
*/
void connectionHandleSdpUnregisterCfm(connectionSdpState *state, const SDS_UNREGISTER_CFM_T *cfm)
{
    if (state->sdpLock)
    {
        /* Send response to the client */
        MAKE_CL_MESSAGE(CL_SDP_UNREGISTER_CFM);
        message->status = (connection_lib_status) cfm->result;
        message->service_handle = cfm->svc_rec_hndl;
        MessageSend(state->sdpLock, CL_SDP_UNREGISTER_CFM, message);
        
        /* Reset the resource lock */
        state->sdpLock = 0;
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpServerConfigMtu

DESCRIPTION
	Send a request to BlueStack to configure the SDP server MTU size

RETURNS
	void	
*/
void connectionHandleSdpServerConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T *req)
{
    /* We don't get a response from this so send it without setting the lock. */
	MAKE_PRIM_T(SDS_CONFIG_REQ);
	prim->mtu = req->mtu;
	VmSendSdpPrim(prim);
}


/****************************************************************************
NAME	
	connectionHandleSdpClientConfigMtu

DESCRIPTION
	Send a request to BlueStack to configure the SDP client MTU size

RETURNS
	void	
*/
void connectionHandleSdpClientConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T *req)
{
	/* 
		We don't get a response from this so send it without setting the lock. */
	MAKE_PRIM_T(SDC_CONFIG_REQ);
	prim->mtu = req->mtu;
	VmSendSdpPrim(prim);
}


/****************************************************************************
NAME	
	connectionHandleSdpOpenSearchRequest

DESCRIPTION
	Send a request to BlueSTack to open an SDP search session

RETURNS
	void	
*/
void connectionHandleSdpOpenSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T *req)
{
	/* Resource free, set the lock */
	state->sdpLock = req->theAppTask;

	/* Store the address of the device we're opening the search to */
	state->sdpServerAddr = req->bd_addr;

	/* Send the request to BlueStack */
	{
		MAKE_PRIM_T(SDC_OPEN_SEARCH_REQ);
		connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
		VmSendSdpPrim(prim);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpOpenSearchCfm

DESCRIPTION
	Handle the response to the open SDP session request

RETURNS
	void	
*/
void connectionHandleSdpOpenSearchCfm(connectionSdpState *state, const SDC_OPEN_SEARCH_CFM_T *cfm)
{
	if(state->sdpLock == connectionGetCmTask())
	{
		/* Request was internal, start SDP Ping */
		uint8* sdp_ptr = (uint8 *) SdpPingServiceRequest;
		if (cfm->result == SDC_OPEN_SEARCH_OK)
			ConnectionSdpServiceSearchRequest(connectionGetCmTask(), &state->sdpServerAddr, 1, sizeof(SdpPingServiceRequest), sdp_ptr);
	}
	else if(state->sdpLock)
	{
		/* Send a response message up to the client */
        MAKE_CL_MESSAGE(CL_SDP_OPEN_SEARCH_CFM);
        message->status = connectionConvertSdpOpenStatus(cfm->result);
        MessageSend(state->sdpLock, CL_SDP_OPEN_SEARCH_CFM, message);
	}

	/* Check if the open search succeeded */
	if (cfm->result != SDC_OPEN_SEARCH_OK)
	{
		/* Reset the resource lock */
		state->sdpLock = 0;
		
		/* Reset the stored address */
		BdaddrSetZero(&state->sdpServerAddr);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpCloseSearchRequest

DESCRIPTION
	Issue a close search request to BlueStack

RETURNS
	void	
*/
void connectionHandleSdpCloseSearchRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T *req)
{
	/* Check the state of the resource lock */
	if (state->sdpLock)
	{
		/* Check if we have a search session currently open */
		if (!BdaddrIsZero(&state->sdpServerAddr))
		{
			/* We do have a search session open */
			if (state->sdpLock == req->theAppTask)
			{
				/* Only allow the task that opened the search to close it */
				MAKE_PRIM_T(SDC_CLOSE_SEARCH_REQ);
				VmSendSdpPrim(prim);
			}
			else
			{
				/* Task requesting the search be closed did not open it - send error */
				sendSdpCloseSearchCfm(req->theAppTask, sdp_task_did_not_open_search);
			}
		}
		else
		{
			/* We don't have a search session open, reply immediately with error */
			sendSdpCloseSearchCfm(req->theAppTask, sdp_search_not_open);
		}
	}
	else
	{
		/* There can't be a search open, reply immediately with an error */	
		sendSdpCloseSearchCfm(req->theAppTask, sdp_search_not_open);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpCloseSearchInd

DESCRIPTION
	Indication from BlueStack informing us that an SDP search session has 
	been closed.

RETURNS
	void	
*/
void connectionHandleSdpCloseSearchInd(connectionSdpState *state, const SDC_CLOSE_SEARCH_IND_T *ind)
{
	/* Inform the client that the SDP search session has been closed */
	sendSdpCloseSearchCfm(state->sdpLock, (sdp_close_status) ind->result);

	/* Reset the resource lock */
	state->sdpLock = 0;
		
	/* Reset the stored address */
	BdaddrSetZero(&state->sdpServerAddr);
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchRequest

DESCRIPTION
	Handle a request to initiate a service search to a remote device.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T *req)
{
	sdp_search_req srch_result = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	if (srch_result == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_SEARCH_REQ);
		prim->phandle = 0;
        connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
        prim->size_srch_pttrn = req->length;

		if (req->length)
			prim->srch_pttrn = VmGetHandleFromPointer(req->search_pattern);
		else
			prim->srch_pttrn = 0;

        prim->max_num_recs = req->max_responses;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;
	}
	else if(req->theAppTask != connectionGetCmTask())
	{
		Task *c = 0;

		/* Queue up the search request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ);
		COPY_CL_MESSAGE(req, message);

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (srch_result == sdp_lock_set)
			c = &state->sdpLock;
		else if (srch_result == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (srch_result == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchCfm

DESCRIPTION
	SDP service search results received so handle them here.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchCfm(connectionSdpState *state, const SDC_EX_SERVICE_SEARCH_CFM_T *cfm)
{
	if (state->sdpSearchLock == connectionGetCmTask())
	{
		/* Internal CFM means we're SDP Pinging */
		uint8* sdp_ptr = (uint8 *) SdpPingServiceRequest;
		
		/* Create next request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ);
	
		message->theAppTask = connectionGetCmTask();
		message->bd_addr = state->sdpServerAddr;
		message->max_responses = 1;
		message->length = sizeof(SdpPingServiceRequest);				 
		message->search_pattern = (uint8*) PanicUnlessMalloc(sizeof(SdpPingServiceRequest));
		memmove(message->search_pattern, sdp_ptr, sizeof(SdpPingServiceRequest));
		
		/* Send after 2 seconds to avoid thrashing the radio */
		MessageSendLater(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message, D_SEC(2));
		
		/* Tidy up memory in cfm */
		if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);
            free(record_list);
        }
        
		/* Free the lock so the next request can be handled */
		state->sdpSearchLock = 0;
	}
    else if (state->sdpSearchLock)
    {
        /* Send a cfm to the client task */
        MAKE_CL_MESSAGE_WITH_LEN(CL_SDP_SERVICE_SEARCH_CFM, cfm->size_rec_list);
        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->num_records = cfm->num_recs_ret;
        message->error_code = cfm->err_code;
        connectionConvertBdaddr(&message->bd_addr, &cfm->bd_addr);
        message->size_records = cfm->size_rec_list;
        
        if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);
            memmove(message->records, record_list, cfm->size_rec_list);
            free(record_list);
        }
        else
            message->records[0] = 0;
        
        MessageSend(state->sdpSearchLock, CL_SDP_SERVICE_SEARCH_CFM, message);
        
        /* Reset the resource lock */
        state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_rec_list)
        {
            uint8 *record_list = VmGetPointerFromHandle(cfm->rec_list);
            free(record_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpAttributeSearchRequest

DESCRIPTION
	Request to issue an SDP attribute search request to BlueStack.

RETURNS
	void	
*/
void connectionHandleSdpAttributeSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T *req)
{
	sdp_search_req res = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	
	if (res == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_ATTRIBUTE_REQ);
		prim->phandle = 0;
        connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
		prim->svc_rec_hndl = req->service_handle;
		prim->size_attr_list = req->size_attribute_list;

		if (req->size_attribute_list)
			prim->attr_list = VmGetHandleFromPointer(req->attribute_list);
		else
			prim->attr_list = 0;

		prim->max_num_attr = req->max_num_attr;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;
	}
	else
	{
		Task *c = 0;

		/* Queue up the search request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ);
		COPY_CL_MESSAGE(req, message);

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (res == sdp_lock_set)
			c = &state->sdpLock;
		else if (res == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (res == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpAttributeSearchCfm

DESCRIPTION
	A confirm to the attribute search has been received, handle it here.

RETURNS
	void	
*/
void connectionHandleSdpAttributeSearchCfm(connectionSdpState *state, const SDC_EX_SERVICE_ATTRIBUTE_CFM_T *cfm)
{
    if (state->sdpSearchLock)
    {
        /* Send a cfm to the client task */	
        MAKE_CL_MESSAGE_WITH_LEN(CL_SDP_ATTRIBUTE_SEARCH_CFM, cfm->size_attr_list);
        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->error_code = cfm->err_code;
        connectionConvertBdaddr(&message->bd_addr, &cfm->bd_addr);
        message->size_attributes = cfm->size_attr_list;	
        
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            memmove(message->attributes, attribute_list, cfm->size_attr_list);
            free(attribute_list);
        }
        else
            message->attributes[0] = 0;
        
        MessageSend(state->sdpSearchLock, CL_SDP_ATTRIBUTE_SEARCH_CFM, message);
        
        /* Reset the resource lock */
        state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            free(attribute_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchAttrRequest

DESCRIPTION
	Handle a request to perfor a service/ attribute search. If allowed, send
	request to BlueStack otherwise queue it up until it can be sent.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchAttrRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T *req)
{
	sdp_search_req res = sendSearchRequest(req->theAppTask, state, &req->bd_addr);
	
	if (res == sdp_start_search)
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_T(SDC_SERVICE_SEARCH_ATTRIBUTE_REQ);
		prim->phandle = 0;
        connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
		prim->size_srch_pttrn = req->size_search_pattern;

		if (req->size_search_pattern)
			prim->srch_pttrn = VmGetHandleFromPointer(req->search_pattern);
		else
			prim->srch_pttrn = 0;

        prim->size_attr_list = req->size_attribute_list;
        
		if (req->size_attribute_list)
			prim->attr_list = VmGetHandleFromPointer(req->attribute_list);
		else
			prim->attr_list = 0;

        prim->max_num_attr = req->max_num_attributes;
		VmSendSdpPrim(prim);

		/* Lock the resource */
		state->sdpSearchLock = req->theAppTask;
	}
	else
	{
		Task *c = 0;

		/* Queue up the search request */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ);
		COPY_CL_MESSAGE(req, message);

		/* Use the return value from searchRequest function to decide which resource to wait on */
		if (res == sdp_lock_set)
			c = &state->sdpLock;
		else if (res == sdp_search_lock_set)
			c = &state->sdpSearchLock;
		else if (res == sdp_session_open_other_dev)
			c = &state->sdpLock;
		else
			/* Should not get here but if we do panic */
			Panic();

		/* Queus up message with condition determined above */
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ, message, c);
	}
}


/****************************************************************************
NAME	
	connectionHandleSdpServiceSearchAttributeCfm

DESCRIPTION
	HAndle the search cfm received from BlueStack.

RETURNS
	void	
*/
void connectionHandleSdpServiceSearchAttributeCfm(connectionSdpState *state, const SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{	
    if (state->sdpSearchLock)
    {
        /* Send a cfm to the client task */
        MAKE_CL_MESSAGE_WITH_LEN(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM, cfm->size_attr_list);
        message->status = connectionConvertSdpSearchStatus(cfm->response);
        message->more_to_come = cfm->more_to_come;
        message->error_code = cfm->err_code;
        connectionConvertBdaddr(&message->bd_addr, &cfm->bd_addr);
        message->size_attributes = cfm->size_attr_list;
        
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            memmove(message->attributes, attribute_list, cfm->size_attr_list);
            free(attribute_list);
        }
        else
            message->attributes[0] = 0;
        
        MessageSend(state->sdpSearchLock, CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM, message);

        /* Reset the resource lock */
	    if (!cfm->more_to_come) 
            state->sdpSearchLock = 0;
    }
    else
    {
        if (cfm->size_attr_list)
        {
            uint8 *attribute_list = VmGetPointerFromHandle(cfm->attr_list);
            free(attribute_list);
        }
    }
}


/****************************************************************************
NAME	
	connectionHandleSdpTerminatePrimitiveRequest

DESCRIPTION
	Send a request to BVlueStack to terminate the current SDP search.

RETURNS
	void	
*/
void connectionHandleSdpTerminatePrimitiveRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T *req)
{
	if (state->sdpSearchLock && (req->theAppTask == state->sdpSearchLock))
	{
		/* 
			There must be an active search ongoing and the task trying to cancel it 
			must be the same task that started the search
		*/		
		MAKE_PRIM_T(SDC_TERMINATE_PRIMITIVE_REQ);
		prim->phandle = 0;
		VmSendSdpPrim(prim);
	}
	/* 
		For now we just silently ignore the terminate primitive if it is issued when we 
		don't have an active search or when the task issuing it is not the same as the
		task that started the search.
	*/
}

/*lint +e725 +e830 */
