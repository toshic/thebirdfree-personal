/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_csr_features_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_CSR_FEATURES_HANDLER_H_
#define HFP_CSR_FEATURES_HANDLER_H_

#include "hfp_private.h"


/*
	Send upstream HFP_CSR_MODIFY_INDICATORS_CFM message 
*/
void hfpSendCsrModifyIndicatorsCfm(HFP *hfp, hfp_lib_status status);

/*
	Send upstream HFP_CSR_SMS_CFM message 
*/
void hfpSendCsrSmsCfm(HFP *hfp, hfp_lib_status status, uint16 length, const uint8 *sms);

/*
	Handle CSR Supported Features request.
*/
void hfpHandleCsrSupportedFeaturesReq(HFP *hfp, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ_T *msg);

/*
	Have received response to AT+CSRSF
*/
void hfpHandleCsrSfAtAck(HFP *hfp, hfp_lib_status status);

/*
	Handle CSR Supported Features ack.
*/
void hfpHandleCsrSupportedFeaturesAck(HFP *hfp, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK_T *msg);

/*
	Handle CSR Power Level Request
*/
void hfpHandleCsrPowerLevelReq(HFP *hfp, HFP_INTERNAL_CSR_POWER_LEVEL_REQ_T *msg);

/*
	Handle CSR Power Source Request
*/
void hfpHandleCsrPowerSourceReq(HFP *hfp, HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T *msg);

/*
	Handle CSR Pwr Source Ack.
*/
void hfpHandleCsrPwrAtAck(HFP *hfp, hfp_lib_status status);

/*
	Handle CSR Battery Level Ack.
*/
void hfpHandleCsrBatAtAck(HFP *hfp, hfp_lib_status status);

/*
	Handle CSR Modify Indicators Request
*/
void hfpHandleCsrModIndsReq(HFP *hfp, HFP_INTERNAL_CSR_MOD_INDS_REQ_T *msg);

/*
	Handle CSR Modify Indicators Ack.
*/
void hfpHandleCsrModIndsReqAck(HFP *hfp, hfp_lib_status status);

/*
	Handle internal Disable Indicators
*/
void hfpCsrMofifyIndicatorsDisableReq(HFP *hfp);

/*
	Handle Get SMS ack.
*/
void hfpHandleCsrGetSmsAck(HFP *hfp, hfp_lib_status status);

/*
	Handle internal Get SMS request.
*/
void hfpHandleCsrGetSmsReq(HFP *hfp, HFP_INTERNAL_CSR_GET_SMS_REQ_T *msg);

/*
	Handle internal message for AG Requesting a battery report
*/
void hfpHandleInternalResponseCSRBatRequest(HFP *hfp);


/*todo*/
void hfpHandleFeatureNegotiationRes ( HFP *hfp , HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES_T * msg ) ;


#endif /* HFP_CSR_FEATURES_HANDLER_H_ */

