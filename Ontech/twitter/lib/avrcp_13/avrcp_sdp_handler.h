/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_sdp_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_SDP_HANDLER_H_
#define AVRCP_SDP_HANDLER_H_

#include "avrcp.h"

#include <connection.h>


/****************************************************************************
NAME	
	avrcpSendGetSupportedFeaturesCfm

DESCRIPTION
    Send GetSupportedFeatures confirmation message to the application task.
*/
void avrcpSendGetSupportedFeaturesCfm(AVRCP *avrcp, avrcp_status_code status, uint16 features);


/****************************************************************************
NAME	
	avrcpSendGetExtensionsCfm

DESCRIPTION
    Send GetExtensions confirmation message to the application task.
*/
void avrcpSendGetExtensionsCfm(AVRCP *avrcp, avrcp_status_code status, uint16 extensions);


/****************************************************************************
NAME	
	avrcpRegisterServiceRecord

DESCRIPTION
    Attempt to register a service record for the RCP service.
*/
void avrcpRegisterServiceRecord(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleSdpRegisterCfm

DESCRIPTION
	This function is called on receipt of an CL_SDP_REGISTER_CFM
	indicating the outcome of registering a service record.
*/
void avrcpHandleSdpRegisterCfm(AVRCP *avrcp, const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	avrcpHandleServiceSearchAttributeCfm

DESCRIPTION
	Handle result of service/attribute search.
*/
void avrcpHandleServiceSearchAttributeCfm(AVRCP *avrcp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);


/* Send SDP search to find profile version at remote end. */
void avrcpGetProfileVersion(AVRCP *avrcp, bool app_request);


/* Send SDP search to find supported features at remote end. */
void avrcpGetSupportedFeatures(AVRCP *avrcp, bool app_request);


#endif /* AVRCP_SDP_HANDLER_H_ */
