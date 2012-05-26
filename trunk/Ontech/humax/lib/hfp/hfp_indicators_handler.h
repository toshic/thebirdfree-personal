/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_indicators_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_INDICATORS_HANDLER_H_
#define HFP_INDICATORS_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleIndicatorListInd

DESCRIPTION
	Message containing the indicator index list. 

RETURNS
	void
*/
void hfpHandleIndicatorListInd(HFP *hfp, const HFP_INTERNAL_AT_INDICATOR_LIST_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleIndicatorInitialStatusInd

DESCRIPTION
	Message containing initial indicator values.

RETURNS
	void
*/
void hfpHandleIndicatorInitialStatusInd(HFP *hfp, const HFP_INTERNAL_AT_INDICATOR_STATUS_IND_T *ind);


/****************************************************************************
NAME	
	hfpSendIndicatorCallSetupToApp

DESCRIPTION
	Update hfp profile state and send call setup indication to the App

RETURNS
	void
*/
void hfpSendIndicatorCallSetupToApp(HFP *hfp, hfp_call_setup call_setup);



#endif /* HFP_INDICATORS_HANDLER_H_ */
