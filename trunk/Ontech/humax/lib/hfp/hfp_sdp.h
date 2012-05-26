/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_sdp.h
    
DESCRIPTION
	
*/

#ifndef HFP_SDP_H_
#define HFP_SDP_H_


/****************************************************************************
NAME	
	hfpRegisterServiceRecord

DESCRIPTION
	Register the service record corresponding to the specified profile 

RETURNS
	void
*/
void hfpRegisterServiceRecord(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSdpRegisterCfm

DESCRIPTION
	Outcome of SDP service register request.

RETURNS
	void
*/
void hfpHandleSdpRegisterCfm(HFP *hfp, const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpHandleSdpInternalRegisterInit

DESCRIPTION
	Handle the outcome of the SDP register request if we're initialising the
	HFP profile lib.

RETURNS
	void
*/
void hfpHandleSdpInternalRegisterInit(HFP *hfp, const HFP_INTERNAL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpHandleSdpInternalRegisterCfm

DESCRIPTION
	Handle the outcome of the SDP register request if we have initialised
	the profile lib.

RETURNS
	void
*/
void hfpHandleSdpInternalRegisterCfm(HFP *hfp, const HFP_INTERNAL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	handleSdpUnregisterCfm

DESCRIPTION
	Outcome of SDP service unregister request.

RETURNS
	void
*/
void handleSdpUnregisterCfm(HFP *hfp, const CL_SDP_UNREGISTER_CFM_T *cfm);


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
void hfpGetProfileServerChannel(HFP *hfp, const bdaddr *addr);


/****************************************************************************
NAME	
	hfpHandleServiceSearchAttributeCfm

DESCRIPTION
	Service search has completed, check it has succeeded and get the required
	attrubutes from the returned list.

RETURNS
	void
*/
void hfpHandleServiceSearchAttributeCfm(HFP *hfp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpGetAgSupportedFeatures

DESCRIPTION
	AG does not support BRSF command so we need to perform an SDP search
	to get its supported features.

RETURNS
	void
*/
void hfpGetAgSupportedFeatures(HFP *hfp);


/****************************************************************************
NAME	
	hfpGetAgProfileVersion

DESCRIPTION
	Requests HFP profile version supported by the AG.
	
RETURNS
	void
*/
void hfpGetAgProfileVersion(HFP *hfp);



#endif /* HFP_SDP_H_ */

