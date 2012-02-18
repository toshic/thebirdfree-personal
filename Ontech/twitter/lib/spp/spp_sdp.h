/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    spp_sdp.h
    
DESCRIPTION
	
*/

#ifndef SPP_SDP_H_
#define SPP_SDP_H_


#include "spp.h"
#include "spp_private.h"


/****************************************************************************
NAME	
	sppRegisterServiceRecord

DESCRIPTION
	Register the service record with BlueStack.
*/
void sppRegisterServiceRecord(SPP *spp, uint8 chan);


/****************************************************************************
NAME	
	sppHandleSdpRegisterCfm

DESCRIPTION
	Handle the CL_SDP_REGISTER_CFM message from the connection lib, informing
    us of the outcome of the service register request.
*/
void sppHandleSdpRegisterCfm(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	sppHandleSdpRegisterCfmReady

DESCRIPTION
	As function sppHandleSdpRegisterCfm above but called when the library
    is in the ready not initialising state. This is used when registering
    the service record once the SPP connection has been disconnected.
*/
void sppHandleSdpRegisterCfmReady(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	sppHandleSdpUnregisterCfm

DESCRIPTION
	Handle the CL_SDP_UNREGISTER_CFM message from the connection lib, 
    informing us of the outcome of the service unregister request.
*/
void sppHandleSdpUnregisterCfm(SPP *spp, const CL_SDP_UNREGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	sppHandleSdpServiceSearchAttributeCfm

DESCRIPTION
	HAndle the message from the connection library informing us of the 
    outcome of the last SDP service-attribute search issued.
*/
void sppHandleSdpServiceSearchAttributeCfm(SPP *spp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *message);


#endif /* SPP_SDP_H_ */
