/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_sdp.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_sdp.h"
#include "hsp_service_record.h"
#include "hfp_service_record.h"
#include "hfp_slc_handler.h"
#include "hfp_rfc.h"
#include "init.h"

#include <panic.h>
#include <service.h>
#include <string.h>


/* Request the service handle(s) of the HSP service at the AG */
static const uint8 HspServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x11, 0x12 /* 4 byte UUID - will be filled in by app */
};


/* Request the service handle(s) of the HFP service at the AG */
static const uint8 HfpServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x11, 0x1F /* 4 byte UUID - will be filled in by app */
};


/* Request the RFCOMM channel number of the AG's service */
static const uint8 protocolAttributeRequest [] =
{
    0x35, /* type = DataElSeq */
    0x03, /* size ...3 bytes in DataElSeq */
    0x09, 0x00, 0x04/* 2 byte UINT attrID ProtocolDescriptorList */    
};


/* Request the supported features of the HFP AG */
static const uint8 supportedFeaturesAttributeRequest [] =
{
    0x35,               /* 0b00110 101 type=DataElSeq */
    0x03,               /* size = 6 bytes in DataElSeq */        
    0x09, 0x03, 0x11    /* 2 byte UINT attrID - SupportedFeatures */
};


/* Request the profile descriptor list of the HFP AG */
static const uint8 profileDescriptorRequest [] =
{
    0x35,               /* 0b00110 101 type=DataElSeq */
    0x03,               /* size = 3 bytes in DataElSeq */        
    0x09, 0x00, 0x09    /* 2 byte UINT attrID - BluetoothProfileDescriptorList */
};


/* Find the rfcomm server channel in a service record */
static bool findRfcommServerChannel(const uint8 *ptr, const uint8 *end, Region *value)
{
    ServiceDataType type;
    Region record, protocols, protocol;
    record.begin = ptr;
    record.end   = end;
    while(ServiceFindAttribute(&record, saProtocolDescriptorList, &type, &protocols))
	if(type == sdtSequence)
	    while(ServiceGetValue(&protocols, &type, &protocol))
		if(type == sdtSequence
		&& ServiceGetValue(&protocol, &type, value) 
                && type == sdtUUID
		&& RegionMatchesUUID32(value, (uint32) UUID_RFCOMM)
		&& ServiceGetValue(&protocol, &type, value)
		&& type == sdtUnsignedInteger)
		{
		    /* Hah! found the server channel field */
		    return 1;
		}
    return 0; /* Failed */

}

/* Insert the rfcomm server channel into a service record */
static uint16 insertRfcommServerChannel(const uint8 *ptr, const uint8 *end, uint8 chan)
{
    Region value;
    if(findRfcommServerChannel(ptr, end, &value) && RegionSize(&value) == 1)
    {
		RegionWriteUnsigned(&value, (uint32) chan);
		return 1;
    }
    return 0;
}


/* Retrieve the rfcomm server channel */
static uint16 getRfcommChannelNumber(const uint8 *begin, const uint8 *end, uint16 *chan)
{
    Region value;
    if(findRfcommServerChannel(begin, end, &value))
    {
		*chan = (uint16) RegionReadUnsigned(&value);
		return 1;
    }
    return 0;
}


/* Find the supported features in a service record */
static bool findHfpSupportedFeatures(const uint8 *begin, const uint8 *end, Region *value)
{
    ServiceDataType type;
	Region record;
	record.begin = begin;
    record.end   = end;

	if (ServiceFindAttribute(&record, saSupportedFeatures, &type, value))
	{
		if(type == sdtUnsignedInteger)
			return 1;
	}
	return 0;
}


/* Insert the supported features into the HFP service record */
static uint16 insertHfpSupportedFeatures(const uint8 *begin, const uint8 *end, uint8 features)
{
	Region value;

	if (findHfpSupportedFeatures(begin, end, &value) && RegionSize(&value) == 2)
	{
		RegionWriteUnsigned(&value, (uint32) features);
		return 1;
	}
	return 0;
}


/* Find the profile version field so we can update it. */
/* Note1 - HFP spec states that the service class UUID should be Handsfree (0x111e).
           Some AG's return a UUID of HandsfreeAudioGateway(0x111f).  
           We need to allow for this.                                               */
static bool findProfileVersion(const uint8 *ptr, const uint8 *end, Region *value)
{
    ServiceDataType type;
    Region record, protocols, protocol;
    record.begin = ptr;
    record.end   = end;
    while(ServiceFindAttribute(&record, saBluetoothProfileDescriptorList, &type, &protocols))
	if(type == sdtSequence)
	    while(ServiceGetValue(&protocols, &type, &protocol))
    		if(type == sdtSequence
	        	&& ServiceGetValue(&protocol, &type, value) 
                && type == sdtUUID
		        && (RegionMatchesUUID32(value, (uint32) 0x111e) || RegionMatchesUUID32(value, (uint32) 0x111f))  /* See Note1 above */
		        && ServiceGetValue(&protocol, &type, value)
		        && type == sdtUnsignedInteger)
		    {
		        /* Have found the profile version */
		        return 1;
		    }

    /* Failed */
    return 0; 
}

/* Insert the profile version number depending on the type of instance initialised */
static uint16 insertHfpProfileVersion(const uint8 *begin, const uint8 *end, hfp_profile profile)
{
    Region value;
    if (findProfileVersion(begin, end, &value))
    {
        if (RegionSize(&value) == 2)
        {
            uint32 profile_version = 0;
            
            if (supportedProfileIsHfp15(profile))
                profile_version = 0x0105;
            else
                profile_version = 0x0101;
            
            /* Insert the data */
            RegionWriteUnsigned(&value, profile_version);
            return 1;
        }
    }

    return 0;
}


/* Obtain the profile version number */
static bool getHfpAgProfileVersion(const uint8 *begin, const uint8 *end, uint16 *profile)
{
    Region value;
    if (findProfileVersion(begin, end, &value))
    {
        if (RegionSize(&value) == 2)
        {   
            uint32 profile_version = RegionReadUnsigned(&value);
            
            if ( profile_version==0x0105 )
            {
	            *profile = (uint16)hfp_handsfree_15_profile;
            }
            else
            {	/* Not explicitly declared as v1.5, so assume v1.1 */
	            *profile = (uint16)hfp_handsfree_profile;
            }
            
            return TRUE;
        }
    }

    return FALSE;
}


/* Get the supported features from the returned attribute list */
static uint16 getHfpAgSupportedFeatures(const uint8 *begin, const uint8 *end, uint16 *features)
{
	Region value;
    if(findHfpSupportedFeatures(begin, end, &value))
    {
		*features = (uint16) RegionReadUnsigned(&value);
		return 1;
    }
    return 0;
}


/* Send an internal message with the outcome of the register request */
static void sendInternalSdpRegisterCfmMessage(HFP *hfp, hfp_lib_status status)
{
    MAKE_HFP_MESSAGE(HFP_INTERNAL_SDP_REGISTER_CFM);
	message->status = status;
	MessageSend(&hfp->task, HFP_INTERNAL_SDP_REGISTER_CFM, message);
}


/****************************************************************************
NAME	
	hfpRegisterServiceRecord

DESCRIPTION
	Register the service record corresponding to the specified profile 

RETURNS
	void
*/
void hfpRegisterServiceRecord(HFP *hfp)
{
	uint8 *service_record = 0;
    hfp_profile profile = hfp->hfpSupportedProfile;
    uint8 chan = hfp->local_rfc_server_channel;

	if (!hfp->service_record)
	{
		if (supportedProfileIsHsp(profile))
        {
            hfp->size_service_record = sizeof(hsp_service_record);
			hfp->service_record = (uint8 *) hsp_service_record;
        }
		else if (supportedProfileIsHfp(profile))
        {
            hfp->size_service_record = sizeof(hfp_service_record);
			hfp->service_record = (uint8 *) hfp_service_record;
        }
		else
		{
			/* Unknown profile, send an error */
            sendInternalSdpRegisterCfmMessage(hfp, hfp_fail);
			return;
		}
	}

	/* Create a copy of the service record that we can modify */
	service_record = (uint8 *)PanicUnlessMalloc(hfp->size_service_record);
	memmove(service_record, hfp->service_record, hfp->size_service_record);
	
	if (supportedProfileIsHfp(profile))
	{
		/* Insert the supported features into the service record. */
		if (!insertHfpSupportedFeatures(service_record, service_record + hfp->size_service_record, hfp->hfpSupportedFeatures))
		{
			/* Failed to insert the supported features into the service record */
            sendInternalSdpRegisterCfmMessage(hfp, hfp_fail);

			/* Free the allocated memory */
			free(service_record);
			return;
		}

        /* Insert the profile version number */
        if (!insertHfpProfileVersion(service_record, service_record + hfp->size_service_record, profile))
        {
            /* SDP register failed */
            sendInternalSdpRegisterCfmMessage(hfp, hfp_fail);

			/* Free the allocated memory */
			free(service_record);
			return;
        }
	}
	
	if (!insertRfcommServerChannel(service_record, service_record + hfp->size_service_record, chan))
	{
		/* If we fail to insert the rfcomm channel return an error to the app */		
        sendInternalSdpRegisterCfmMessage(hfp, hfp_fail);
		
		/* Free the allocated memory */
		free(service_record);
	}
	else
	{
		/* Send the service record to the connection lib to be registered with BlueStack */
		ConnectionRegisterServiceRecord(&hfp->task, hfp->size_service_record, service_record);
	}
}


/****************************************************************************
NAME	
	hfpHandleSdpRegisterCfm

DESCRIPTION
	Outcome of SDP service register request.

RETURNS
	void
*/
void hfpHandleSdpRegisterCfm(HFP *hfp, const CL_SDP_REGISTER_CFM_T *cfm)
{
	/* Check the outcome of the register request */
	if (cfm->status == success)
	{
		/* Set status to success */
        sendInternalSdpRegisterCfmMessage(hfp, hfp_success);
		
		/* Store the service handle */
		hfp->sdp_record_handle = cfm->service_handle;
	}
	else
	{
		/* Set status to fail */
		sendInternalSdpRegisterCfmMessage(hfp, hfp_fail);

		/* Reset the service handle */
		hfp->sdp_record_handle = 0;
	}
}


/****************************************************************************
NAME	
	hfpHandleSdpInternalRegisterInit

DESCRIPTION
	Handle the outcome of the SDP register request if we're initialising the
	HFP profile lib.

RETURNS
	void
*/
void hfpHandleSdpInternalRegisterInit(HFP *hfp, const HFP_INTERNAL_SDP_REGISTER_CFM_T *cfm)
{
	if (cfm->status == hfp_success)
	{		
		/* Service record register succeeded */
		hfpSendInternalInitCfm(&hfp->task, hfp_init_success, 0);
	}
	else
	{
		/* Service record register failed - we don't get more detail than this! */
		hfpSendInternalInitCfm(&hfp->task, hfp_init_sdp_reg_fail, 0);
	}
}


/****************************************************************************
NAME	
	hfpHandleSdpInternalRegisterCfm

DESCRIPTION
	Handle the outcome of the SDP register request if we have initialised
	the profile lib and are registering a service record during the 
	operation of the profile.

RETURNS
	void
*/
void hfpHandleSdpInternalRegisterCfm(HFP *hfp, const HFP_INTERNAL_SDP_REGISTER_CFM_T *cfm)
{
	/* If this failed then we should retry, SDS may have been busy with SDP search */
	if (cfm->status != hfp_success)
	{
		if (hfp->sdp_record_handle == 0)
		{
			/* Register service record */
			hfpRegisterServiceRecord(hfp);
		}

#ifdef HFP_DEBUG_LIB
		HFP_DEBUG(("SDP register request failed\n"));
#endif
	}
}


/****************************************************************************
NAME	
	handleSdpUnregisterCfm

DESCRIPTION
	Outcome of SDP service unregister request.

RETURNS
	void
*/
void handleSdpUnregisterCfm(HFP *hfp, const CL_SDP_UNREGISTER_CFM_T *cfm)
{
	if (cfm->status == success)
	{
		/* Unregister succeeded reset the service record handle */
		hfp->sdp_record_handle = 0;

		/* Check if we need to re-register the SDP record */
		if (hfp->state == hfpReady)
		{
			hfpRegisterServiceRecord(hfp);
		}
	}
}


/****************************************************************************
NAME	
	hfpGetProfileServerChannel

DESCRIPTION
	Initiate a service search to get the rfcomm server channel of the 
	required service on the remote device. We need this before we can 
	initiate a service level connection.

RETURNS
	void
*/
void hfpGetProfileServerChannel(HFP *hfp, const bdaddr *addr)
{
	uint16 sp_len;
	uint8 *sp_ptr;	

	sp_len = 0;
	sp_ptr = 0;

	/* Check which profile we support so we can device which search to use */
	if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
	{
		/* This task supports the HSP */
		sp_ptr = (uint8 *) HspServiceRequest;
		sp_len = sizeof(HspServiceRequest);
	}
	else if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
	{
		/* This task supports the HFP */
		sp_ptr = (uint8 *) HfpServiceRequest;
		sp_len = sizeof(HfpServiceRequest);
	}
	else
	{
		/* This should never happen */
		Panic();
	}

	/* 
			Issue the search request to the connection lib. The max number of attribute bytes
			is set to an arbitrary number, however the aim is to set it to a value so that
			if the remote end returns this many bytes we still have a block big enough to
			copy the data into. 
	*/
	hfp->sdp_search_mode = hfp_sdp_search_rfcomm_channel;
	ConnectionSdpServiceSearchAttributeRequest(&hfp->task, addr, 0x32, sp_len, sp_ptr, sizeof(protocolAttributeRequest), protocolAttributeRequest);
}


/****************************************************************************
NAME	
	hfpHandleServiceSearchAttributeCfm

DESCRIPTION
	Service search has completed, check it has succeeded and get the required
	attrubutes from the returned list.

RETURNS
	void
*/
void hfpHandleServiceSearchAttributeCfm(HFP *hfp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
	/* Check the outcome of the service search */
	if (cfm->status == sdp_response_success)
	{		
		uint16 sdp_data = 0;

		switch ( hfp->sdp_search_mode )
		{
		case hfp_sdp_search_rfcomm_channel:
			if (getRfcommChannelNumber(cfm->attributes, cfm->attributes+cfm->size_attributes, &sdp_data))
			{	/* We have an rfcomm channel we can proceed with the connection establishment */
			
				HFP_INTERNAL_RFCOMM_CONNECT_REQ_T message ;
				message.addr = cfm->bd_addr;
				message.rfc_channel = sdp_data;
					
				if (hfp->state == hfpSlcConnecting)
					hfpHandleRfcommConnectRequest(hfp, &message);
			
			}
			else
			{
				/* Received unexpected data.  Should never happen since we're issuing the search and should 
			       know what we're looking.  However, if it does, assume a failure has occurred as below.   */
			       
				/* Tell the app the connection attempt has failed. */
			    hfpSendSlcConnectCfmToApp(hfp_connect_sdp_fail, hfp);
			}
			break;
		case hfp_sdp_search_profile_version:
			
			if (getHfpAgProfileVersion(cfm->attributes, cfm->attributes+cfm->size_attributes, &sdp_data))
			{	/* Obtained version of HFP running on AG */
				hfp->agSupportedProfile = (hfp_profile)sdp_data;
				
				if( supportedProfileIsHfp15(hfp->agSupportedProfile) )
				{
					hfpSendRemoteAGProfileVerIndToApp(hfp->agSupportedProfile, hfp);
				}
			}
			else
			{
				/* Received unexpected data.  Should never happen since we're issuing the search and should 
			       know what we're looking.  However, if it does, assume a failure has occurred as below.   */

				/* Incorrect response from AG, so assume it's running HFP v1.0 */
				hfp->agSupportedProfile = hfp_handsfree_profile;
			}
			
			break;
		case hfp_sdp_search_supported_features:
			if (getHfpAgSupportedFeatures(cfm->attributes, cfm->attributes+cfm->size_attributes, &sdp_data))
			{	/* Send an internal message with the supported features. */
				hfpHandleSupportedFeaturesNotification(hfp, sdp_data);
			}
			else
			{
				/* Received unexpected data.  Should never happen since we're issuing the search and should 
			       know what we're looking.  However, if it does, just ignore since SLC setup is already 
			       being continued.                                                                         */
			}
			break;
		default:
			/* 
				else ... We have received data we don't know what to do with.
				This shouldn't happen since we're issuing the search and should 
				know what we're looking for so for the moment just ignore.
			*/
			break;
		}
	}
	else
	{
		switch ( hfp->sdp_search_mode )
		{
		case hfp_sdp_search_rfcomm_channel:
			/* A RFCOMM connection will not exist at this point */
            if (cfm->status == sdp_no_response_data)
			{	/* Tell the app the connection attempt has failed. */
			    hfpSendSlcConnectCfmToApp(hfp_connect_sdp_fail, hfp);
		    }
            else if (cfm->status == sdp_connection_error)
            {	/* If it was a page timeout the client would like to know */
                hfpSendSlcConnectCfmToApp(hfp_connect_timeout, hfp);
            }
            else
            {	/* All other sdp fails */
                hfpSendSlcConnectCfmToApp(hfp_connect_failed, hfp);
            }
			break;
		case hfp_sdp_search_profile_version:
			/* No response from AG, so assume it's running HFP v1.0 */
			hfp->agSupportedProfile = hfp_handsfree_profile;
			break;
		case hfp_sdp_search_supported_features:
			/* Just ignore.  SLC setup is already being continued */
			break;
		default:
			/* 
				else ... We have received data we don't know what to do with.
				This shouldn't happen since we're issuing the search and should 
				know what we're looking for so for the moment just ignore.
			*/
			break;
		}
	}
	
	
	/* Reset sdp search mode */
	hfp->sdp_search_mode = hfp_sdp_search_none;
}


/****************************************************************************
NAME	
	hfpGetAgSupportedFeatures

DESCRIPTION
	AG does not support BRSF command so we need to perform an SDP search
	to get its supported features.

RETURNS
	void
*/
void hfpGetAgSupportedFeatures(HFP *hfp)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(hfp->sink, &my_addr))
	{
		/* 
			Issue the search request to the connection lib. The max number of attribute bytes
			is set to an arbitrary number, however the aim is to set it to a value so that
			if the remote end returns this many bytes we still have a block big enough to
			copy the data into. 
		*/
		hfp->sdp_search_mode = hfp_sdp_search_supported_features;
		ConnectionSdpServiceSearchAttributeRequest(&hfp->task, &my_addr, 0x32, sizeof(HfpServiceRequest), (uint8 *) HfpServiceRequest, sizeof(supportedFeaturesAttributeRequest), supportedFeaturesAttributeRequest);
	}
	else
	{
		/* Something has gone wrong - panic */
		HFP_DEBUG(("SinkGetBdAddr failed\n"));
	}
}


/****************************************************************************
NAME	
	hfpGetAgProfileVersion

DESCRIPTION
	Requests HFP profile version supported by the AG.
	
RETURNS
	void
*/
void hfpGetAgProfileVersion(HFP *hfp)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(hfp->sink, &my_addr))
	{
		/* 
			Issue the search request to the connection lib. The max number of attribute bytes
			is set to an arbitrary number, however the aim is to set it to a value so that
			if the remote end returns this many bytes we still have a block big enough to
			copy the data into. 
		*/
		hfp->sdp_search_mode = hfp_sdp_search_profile_version;
		ConnectionSdpServiceSearchAttributeRequest(&hfp->task, &my_addr, 0x32, sizeof(HfpServiceRequest), (uint8 *) HfpServiceRequest, sizeof(profileDescriptorRequest), profileDescriptorRequest);
	}
	else
	{
		/* Something has gone wrong - panic */
		HFP_DEBUG(("SinkGetBdAddr failed\n"));
	}
}
