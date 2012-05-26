/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_csr_features.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


/****************************************************************************/
void HfpCsrSupportedFeaturesReq(HFP *hfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource , uint16 codecs )
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ);
		message->callerName = callerName;
		message->rawText = rawText;
		message->smsInd = smsInd;
		message->battLevel = battLevel;
		message->pwrSource = pwrSource;
		message->codecs = codecs ;
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ, message);
    }
}


/****************************************************************************/
void HfpFeatureNegotiationResponse (HFP * hfp , uint16 indicator, uint16 value )
{
    #ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif
    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES);
		message->indicator = indicator;
		message->value = value;
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES, message);
    }
}

