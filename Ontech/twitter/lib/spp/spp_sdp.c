/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	spp_sdp.c        

DESCRIPTION
    Functions handling SDP related tasks	

NOTES

*/

#include "spp_sdp.h"
#include "init.h"
#include "spp_connect_handler.h"
#include "spp_common.h"

#include <panic.h>
#include <string.h>
#include <service.h>
#include <sdp_parse.h>

#include <stdio.h>
#include <ctype.h>

/*****************************************************************************/
static void storeSdpHandle(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    if (cfm->status == success)
        spp->sdp_record_handle = cfm->service_handle;
    else
        spp->sdp_record_handle = 0;
}


/*****************************************************************************/
void sppRegisterServiceRecord(SPP *spp, uint8 chan)
{
    uint8 *service_record = 0;

   
    /* Create a copy of the service record that we can modify */
    service_record = (uint8 *)PanicUnlessMalloc(spp->length_sr);
    memcpy(service_record, spp->sr, spp->length_sr);

    if (!SdpParseInsertRfcommServerChannel(spp->length_sr, service_record, chan))
    {
        /* If we fail to insert the rfcomm channel return an error to the app */
        free(service_record);
        sppSendInitCfmToApp(spp, spp_init_sdp_reg_fail);
    }
    else
    {
        /* 
            Send the service record to the connection lib to be 
            registered with BlueStack. Note we no longer own the memory
            allocated above so don't free it.
        */
        ConnectionRegisterServiceRecord(&spp->task, spp->length_sr, service_record);
    }
}


/*****************************************************************************/
void sppHandleSdpRegisterCfm(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    storeSdpHandle(spp, cfm);

    if (cfm->status == success)
        sppSendInitCfmToApp(spp, spp_init_success);	
    else
        sppSendInitCfmToApp(spp, spp_init_sdp_reg_fail);
}


/*****************************************************************************/
void sppHandleSdpRegisterCfmReady(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    storeSdpHandle(spp, cfm);
}


/*****************************************************************************/
void sppHandleSdpUnregisterCfm(SPP *spp, const CL_SDP_UNREGISTER_CFM_T *cfm)
{
	if (cfm->status == success)
	{
		/* Unregister succeeded reset the service record handle */
		spp->sdp_record_handle = 0;
	}
}


/*****************************************************************************/
void sppHandleSdpServiceSearchAttributeCfm(SPP *spp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    if (cfm->status == sdp_response_success)
    {		
        uint8* rfcomm_channels;	
		uint8 size_rfcomm_channels = 1, channels_found = 0;

		rfcomm_channels = PanicUnlessMalloc(size_rfcomm_channels * sizeof(uint8));
		
        /* See if the received data contains an rfcomm channel. */
        if (SdpParseGetMultipleRfcommServerChannels(cfm->size_attributes, (uint8*)cfm->attributes, size_rfcomm_channels, &rfcomm_channels, &channels_found))
        {
            /* We have an rfcomm channel we can proceed with the connection establishment */
            MAKE_SPP_MESSAGE(SPP_INTERNAL_RFCOMM_CONNECT_REQ);
            message->addr = cfm->bd_addr;
            message->rfc_channel = rfcomm_channels[0];
			free(rfcomm_channels);
            MessageSend(&spp->task, SPP_INTERNAL_RFCOMM_CONNECT_REQ, message);
            
            sppSetState(spp, sppConnecting);
        }
        else
        {
            /* Can't cope with this for now so Panic */
            Panic();
        }
    }
    else
    {
        /* 
            SDP search failed, usually because rremote device does not support SPP 
            service or is not there. 
        */
        sppSendConnectCfmToApp(spp_connect_sdp_fail, spp);
    }
}
