/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_sdp_handler.c
    
DESCRIPTION
*/

#include "avrcp_sdp_handler.h"
#include "avrcp_init.h"

#include <sdp_parse.h>
#include <stdlib.h>
#include <string.h>


/* TODO - check version numbers */

/* SDP record for the target device role */
static const uint8 serviceRecordTarget[] =
{
  0x09, /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35, /* DataElSeq 3 bytes */
  0x03,
    0x19, /* uuid - TARGET */
    0x11,
    0x0C,
  0x09, /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35, /* DataElSeq 16 bytes */
  0x10,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0017 */
        0x00,
        0x17,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVCTP(0x0017) */
      0x00,
      0x17,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09, /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35, /* DataElSeq 8 bytes */
  0x08,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid RemoteControl(0x110e) */
      0x11,
      0x0e,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* SupportedFeatures(0x0311) = "0x0002" */
    0x03,
    0x11,
  0x09, /* uint16 0x0002 - category 2 */
    0x00,
    0x02
}; /* 48 bytes */


/* SDP record for the controller device role */
static const uint8 serviceRecordController[] =
{
  0x09, /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35, /* DataElSeq 3 bytes */
  0x03,
    0x19, /* uuid - CONTROLLER */
    0x11,
    0x0E,
  0x09, /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35, /* DataElSeq 16 bytes */
  0x10,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0017 */
        0x00,
        0x17,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVCTP(0x0017) */
      0x00,
      0x17,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09, /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35, /* DataElSeq 8 bytes */
  0x08,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid RemoteControl(0x110e) */
      0x11,
      0x0e,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09, /* uint16 0x0001 - category 1 */
    0x00,
    0x01
}; /* 48 bytes */
/* Request the profile descriptor list  */
static const uint8 profileDescriptorRequest [] =
{
    0x35,               /* 0b00110 101 type=DataElSeq */
    0x03,               /* size = 3 bytes in DataElSeq */        
    0x09, 0x00, 0x09    /* 2 byte UINT attrID - BluetoothProfileDescriptorList */
};

/* Request the service handle(s) of the AVRCP service at the TG */
static const uint8 AvrcpServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x11, 0x0C /* AVRCP target service class UUID */
};

/* Request the supported features */
static const uint8 supportedFeaturesAttributeRequest [] =
{
    0x35,               /* 0b00110 101 type=DataElSeq */
    0x03,               /* size = 6 bytes in DataElSeq */        
    0x09, 0x03, 0x11    /* 2 byte UINT attrID - SupportedFeatures */
};


static void avrcpHandleProfileResult(AVRCP *avrcp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm, bool tell_app)
{
	uint16 sdp_data = 0;
	avrcp_status_code status = avrcp_fail;
	uint16 remote_extensions = 0;

	/* Check the outcome of the service search */
	if (cfm->status == sdp_response_success)
	{		
		if (SdpParseGetProfileVersion(cfm->size_attributes, (uint8*)cfm->attributes, 0x110e, &sdp_data))
		{	
			status = avrcp_success;
			/* See if this is the metadata extension version of the profile. */
			if (sdp_data == 0x0103)
				remote_extensions = AVRCP_EXTENSION_METADATA;		
		}
	}
	if (tell_app)
	{
		/* The app requested this info so send it up. */
		avrcpSendGetExtensionsCfm(avrcp, status, remote_extensions);	
	}
}


static void avrcpHandleFeaturesResult(AVRCP *avrcp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm, bool tell_app)
{
	uint16 sdp_data = 0;
	avrcp_status_code status = avrcp_fail;

	/* Check the outcome of the service search */
	if (cfm->status == sdp_response_success)
	{		
		if (SdpParseGetSupportedFeatures(cfm->size_attributes, (uint8*)cfm->attributes, &sdp_data))
		{	
			status = avrcp_success;
		}
	}
	if (tell_app)
	{
		/* The app requested this info so send it up. */
		avrcpSendGetSupportedFeaturesCfm(avrcp, status, sdp_data);
	}
}


/*****************************************************************************/
void avrcpSendGetSupportedFeaturesCfm(AVRCP *avrcp, avrcp_status_code status, uint16 features)
{
	MAKE_AVRCP_MESSAGE(AVRCP_GET_SUPPORTED_FEATURES_CFM);
	message->status = status;
	message->features = features;
	MessageSend(avrcp->clientTask, AVRCP_GET_SUPPORTED_FEATURES_CFM, message);
}


/*****************************************************************************/
void avrcpSendGetExtensionsCfm(AVRCP *avrcp, avrcp_status_code status, uint16 extensions)
{
	MAKE_AVRCP_MESSAGE(AVRCP_GET_EXTENSIONS_CFM);
	message->status = status;
	message->extensions = extensions;
	MessageSend(avrcp->clientTask, AVRCP_GET_EXTENSIONS_CFM, message);
}


/*****************************************************************************/
void avrcpRegisterServiceRecord(AVRCP *avrcp)
{
    uint16 size_record = 0;
    const uint8 *service_record = 0;
	uint8 *service_record_copy = 0;
	uint16 profile = 0;
	uint8 features = 0;

    if (avrcp->device_type == avrcp_target)
    {
        size_record = sizeof(serviceRecordTarget);
        service_record = serviceRecordTarget;
    }
    else if (avrcp->device_type == avrcp_controller)
    {
        size_record = sizeof(serviceRecordController);
        service_record = serviceRecordController;
    }
    else if (avrcp->device_type == avrcp_target_and_controller)
    {
        /* 
            If we need to register both records try and keep track of which 
            one we need to register next. Register target first. 
        */
        size_record = sizeof(serviceRecordTarget);
        service_record = serviceRecordTarget;
        avrcp->device_type = avrcp_device_none;
    }
    else if (avrcp->device_type == avrcp_device_none)
    {
        /* Time to register the controller service record */
        size_record = sizeof(serviceRecordController);
        service_record = serviceRecordController;
        avrcp->device_type = avrcp_target_and_controller;
    }
    else
    {
        /* Some unknown value has been passed in so fail the init. */
        avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, avrcp_fail);
        return;
    }

	/* Create a copy of the service record that we can modify */
	service_record_copy = (uint8 *)PanicUnlessMalloc(size_record);
	memcpy(service_record_copy, service_record, size_record);

	if (avrcp->local_controller_features && ((avrcp->device_type == avrcp_controller) || (avrcp->device_type == avrcp_target_and_controller)))
		features = avrcp->local_controller_features;
	if (avrcp->local_target_features && ((avrcp->device_type == avrcp_target) || (avrcp->device_type == avrcp_device_none)))
		features = avrcp->local_target_features;

	if (features)
	{
		/* Insert supported metadata features */
		if (!SdpParseInsertSupportedFeatures(size_record, service_record_copy, features))
		{
			/* If we fail to insert the supported features return an error to the app */		
			avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, avrcp_fail);
			
			/* Free the allocated memory */
			free(service_record_copy);	
			return;
		}	
	}
	
	/* Set correct profile version depending on if the metadata extensions are enabled. */
	if (avrcp->local_extensions & AVRCP_EXTENSION_METADATA)
		profile = 0x0103;
	else
		profile = 0x0100;
	/* Insert supported metadata features */
	if (!SdpParseInsertProfileVersion(size_record, service_record_copy, 0x110e, profile))
	{
		/* If we fail to insert the profile version return an error to the app */		
		avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, avrcp_fail);
		
		/* Free the allocated memory */
		free(service_record_copy);	
		return;
	}

	/* Register the service record */
	ConnectionRegisterServiceRecord(&avrcp->task, size_record, service_record_copy);
}


/*****************************************************************************/
void avrcpHandleSdpRegisterCfm(AVRCP *avrcp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    if (avrcp->device_type == avrcp_device_none)
    {
        /* One more service record to register */
        avrcpRegisterServiceRecord(avrcp);
    }
    else
    {
	    /* Send an initialisation confirmation with the result of the SDP registration */
        avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, cfm->status==success ? avrcp_success : avrcp_fail);
    }
}


/*****************************************************************************/
void avrcpHandleServiceSearchAttributeCfm(AVRCP *avrcp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
	switch (avrcp->sdp_search_mode)
	{
	case avrcp_sdp_search_profile_version:
		avrcpHandleProfileResult(avrcp, cfm, FALSE);
		break;
	case avrcp_sdp_search_app_profile_version:
		avrcpHandleProfileResult(avrcp, cfm, TRUE);
		break;
	case avrcp_sdp_search_features:
		avrcpHandleFeaturesResult(avrcp, cfm, FALSE);
		break;
	case avrcp_sdp_search_app_features:
		avrcpHandleFeaturesResult(avrcp, cfm, TRUE);
		break;
	default:
		break;
	}

	avrcp->sdp_search_mode = avrcp_sdp_search_none;
}


/*****************************************************************************/
void avrcpGetProfileVersion(AVRCP *avrcp, bool app_request)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(avrcp->sink, &my_addr))
	{
		if (app_request)
			avrcp->sdp_search_mode = avrcp_sdp_search_app_profile_version;
		else
			avrcp->sdp_search_mode = avrcp_sdp_search_profile_version;
		ConnectionSdpServiceSearchAttributeRequest(&avrcp->task, &my_addr, 0x32, sizeof(AvrcpServiceRequest), (uint8 *) AvrcpServiceRequest, sizeof(profileDescriptorRequest), profileDescriptorRequest);
	}
}


/*****************************************************************************/
void avrcpGetSupportedFeatures(AVRCP *avrcp, bool app_request)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(avrcp->sink, &my_addr))
	{
		if (app_request)
			avrcp->sdp_search_mode = avrcp_sdp_search_app_features;
		else
			avrcp->sdp_search_mode = avrcp_sdp_search_features;
		ConnectionSdpServiceSearchAttributeRequest(&avrcp->task, &my_addr, 0x32, sizeof(AvrcpServiceRequest), (uint8 *) AvrcpServiceRequest, sizeof(supportedFeaturesAttributeRequest), supportedFeaturesAttributeRequest);
	}
}
