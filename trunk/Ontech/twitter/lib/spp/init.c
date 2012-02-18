/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	init.c        

DESCRIPTION
	This file contains the initialisation code for the Spp profile library.

*/

#include "spp.h"
#include "init.h"
#include "spp_sdp.h"
#include "spp_private.h"
#include "spp_common.h"

/* TODO : PJH - need to change UUID */
/* Default Spp service record */
static const unsigned char spp_service_record [] =
{
    0x09, 0x00, 0x01,   /* ServiceClassIDList(0x0001) */
#if 0    
    0x35, 0x03,         /* DataElSeq 3 bytes */
    0x19, 0x11, 0x01,   /* UUID SerialPort(0x1101) */
#else
    0x35, 0x11,          /* DataElSeq 17 bytes */
    0x1c, 0x46, 0x34, 0x81, 0x10, 0x23, 0x3a, 0x46, 0xdf, 0x99, 0x86, 0x33, 0x5c, 0xe0, 0xd1, 0xe2, 0x67,/* HKMC UUID */
#endif    
    0x09, 0x00, 0x04,   /* ProtocolDescriptorList(0x0004) */
    0x35, 0x0c,         /* DataElSeq 12 bytes */
    0x35, 0x03,         /* DataElSeq 3 bytes */
    0x19, 0x01, 0x00,   /* UUID L2CAP(0x0100) */
    0x35, 0x05,         /* DataElSeq 5 bytes */
    0x19, 0x00, 0x03,   /* UUID RFCOMM(0x0003) */
    0x08, 0x00,         /* uint8 0x00 */
    0x09, 0x00, 0x06,   /* LanguageBaseAttributeIDList(0x0006) */
    0x35, 0x09,         /* DataElSeq 9 bytes */
    0x09, 0x65, 0x6e,   /* uint16 0x656e */
    0x09, 0x00, 0x6a,   /* uint16 0x006a */
    0x09, 0x01, 0x00,   /* uint16 0x0100 */
    0x09, 0x01, 0x00,   /* ServiceName(0x0100) = "OnTech" */
    0x25, 0x06,         /* String length 6 */
    'O','n','T','e','c', 'h'
};

/*****************************************************************************/
const profile_task_recipe spp_recipe = {
    sizeof(SPP),
    SPP_INTERNAL_TASK_INIT_REQ,
    {sppProfileHandler},
    1,
    (const profile_task_recipe *) 0
};


/*****************************************************************************/
static void sendCfmMsg(Task task, spp_init_status status, SPP *spp)
{
    MAKE_SPP_MESSAGE(SPP_INIT_CFM);
    message->spp = spp;
	message->status = status;
	MessageSend(task, SPP_INIT_CFM, message);
}


/*****************************************************************************/
static void sppSendInitReq(SPP *spp, Task connectTask, const profile_task_recipe *task_recipe)
{
    MAKE_SPP_MESSAGE(SPP_INTERNAL_INIT_REQ);
    message->connectionTask = connectTask;
    message->recipe = task_recipe;
    MessageSend(&spp->task, SPP_INTERNAL_INIT_REQ, message);
}


/*****************************************************************************/
SPP *sppCreateTaskInstance(uint16 priority, Sink sink, Task app, sppState state, uint8 chan, uint16 length, const uint8 *rec, bool no_rec, uint16 lazy)
{
    SPP *spp = PanicUnlessNew(SPP);
    sppInitTaskData(spp, priority, sink, app, state, chan, length, rec, no_rec, lazy);
    return spp;
}


/*****************************************************************************/
void sppSendInitCfmToApp(SPP *spp, spp_init_status status)
{
    if (spp->lazy)
    {
        /* Pass NULL SPP ptr to client as it's about to be deleted. */
        sendCfmMsg(spp->clientTask, status, 0);
        free(spp);
    }
    else
    {
        sendCfmMsg(spp->clientTask, status, spp);
        sppSetState(spp, sppReady);

       	/* If the initialisation failed, free the allocated task. */
    	if (status != spp_init_success)
	    	free(spp);
    }
}


/*****************************************************************************/
void sppInitTaskData(SPP *spp, uint16 priority, Sink sink, Task app, sppState state, uint8 chan, uint16 length, const uint8 *rec, const bool no_rec, uint16 lazy)
{
    spp->priority = priority;
	spp->sink = sink;
    sppSetState(spp, state);

	/* Store the task where we send responses. */
	spp->clientTask = app;

    /* Init the local server channel - this field is only used for incoming connections */
    spp->local_server_channel = chan;

    spp->sdp_record_handle = 0;
    spp->task.handler = sppProfileHandler;
    spp->lazy = lazy;
	
	/* If client supplied a service record use it, otherwise use the default */
    if (length)
    {
		spp->length_sr = length;
  	    spp->sr = rec;
    }
	/* User wish to register a default service record */
    else if(!no_rec)
    {
		spp->length_sr = sizeof(spp_service_record);
        spp->sr = (uint8 *) spp_service_record;
    }
	else
    {
    	spp->length_sr = 0;
    	spp->sr = 0;
    }
}


/*****************************************************************************/
void SppInit(Task theAppTask, spp_device_type type_of_device, uint16 priority)
{
    SPP *spp = sppCreateTaskInstance(priority, 0, theAppTask, sppInitialising, 0, 0, 0, 0, 0);
    sppSendInitReq(spp, 0, 0);
    
    /* Keep the compiler happy but this has now been deprecated */
    type_of_device = type_of_device;
}


/*****************************************************************************/
void SppInitEx(Task theAppTask, uint16 priority, const spp_init_params *config)
{
    if (!config)
    {
        /* The client needs to supply config params so fail the init immediately */
        sendCfmMsg(theAppTask, spp_init_fail, 0);
    }
	/* either size_service_record and no_service_record shall be zero */
	else if(config->size_service_record && config->no_service_record)
	{
		sendCfmMsg(theAppTask, spp_init_fail, 0);
	}
    else
	{
        SPP *spp = sppCreateTaskInstance(priority, 0, theAppTask, sppInitialising, 0, config->size_service_record, config->service_record, config->no_service_record, 0);    
        sppSendInitReq(spp, 0, 0);		
    }
}


/*****************************************************************************/
void SppInitLazy(Task clientTask, Task connectionTask, const spp_init_params *config)
{
    if (!config)
    {
        /* The client needs to supply config params so fail the init immediately */
        sendCfmMsg(clientTask, spp_init_fail, 0);
    }
	/* either size_service_record and no_service_record shall be zero */
	else if(config->size_service_record && config->no_service_record)
	{
		sendCfmMsg(clientTask, spp_init_fail, 0);
	}
    else
    {
		/* Create a temporary task to handle the library initialisation */
	    SPP *spp = sppCreateTaskInstance(0, 0, clientTask, sppInitialising, 0, config->size_service_record, config->service_record, config->no_service_record, 1);        

	    /* Get an rfcomm channel */
        if (!config->client_recipe)
            sppSendInitReq(spp, connectionTask, &spp_recipe);
        else
            sppSendInitReq(spp, connectionTask, config->client_recipe);
		
    }
}


/*****************************************************************************/
void sppHandleInternalInitReq(SPP *spp, const SPP_INTERNAL_INIT_REQ_T *req)
{
    if (spp->lazy)
        ConnectionRfcommAllocateChannelLazy(&spp->task, req->connectionTask, req->recipe);
    else
        ConnectionRfcommAllocateChannel(&spp->task);
}


/*****************************************************************************/
void sppHandleRfcommRegisterCfm(SPP *spp, const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
    if (cfm->status == success)
    {
        spp->local_server_channel = cfm->server_channel;
        
		if(!spp->length_sr)
		{
			/* Skip register service record, send init cfm to app */
			sppSendInitCfmToApp(spp, spp_init_success);
		}
		else
		{
        	/* Rfcomm channel allocated, register a service record for the profile */
        	sppRegisterServiceRecord(spp, cfm->server_channel);
		}
    }
    else
        /* Send an init cfm with error code indicating channel alloc failed */
        sppSendInitCfmToApp(spp, spp_init_rfc_chan_fail);
}

