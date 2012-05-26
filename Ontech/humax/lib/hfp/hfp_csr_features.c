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
#include "hfp_csr_features_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************/
void HfpCsrPowerLevel(HFP *hfp, uint16 pwr_level)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

	if (hfp->use_csr2csr)
    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_POWER_LEVEL_REQ);
        message->pwr_level = pwr_level;
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_POWER_LEVEL_REQ, message);
    }
}


/****************************************************************************/
void HfpCsrPowerSource(HFP *hfp, hfp_csr_power_status_report pwr_status)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

	if (hfp->use_csr2csr)
    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_POWER_SOURCE_REQ);
        message->pwr_status = pwr_status;
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_POWER_SOURCE_REQ, message);
    }
}


/****************************************************************************/
void HfpCsrModifyIndicators(HFP *hfp, uint16 size_indicators, const hfp_csr_mod_indicator *indicators)
{
	if (hfp->use_csr2csr)
	{
		hfp_csr_mod_indicator *ind;
		
		ind = (hfp_csr_mod_indicator*)malloc(sizeof(hfp_csr_mod_indicator)*size_indicators);
		if (ind)
		{
		    MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_MOD_INDS_REQ);
			message->size_indicators = size_indicators;
			message->indicators = ind;
			memmove(ind, indicators, sizeof(hfp_csr_mod_indicator)*size_indicators);
		    MessageSend(&hfp->task, HFP_INTERNAL_CSR_MOD_INDS_REQ, message);
		}
		else
		{
			hfpSendCsrModifyIndicatorsCfm(hfp, hfp_csr_mod_ind_no_mem);
		}
	}
	else
	{
		hfpSendCsrModifyIndicatorsCfm(hfp, hfp_csr_not_inited);
	}
}


/****************************************************************************/
void HfpCsrMofifyIndicatorsDisable(HFP *hfp)
{
	if (hfp->use_csr2csr)
	{
	    MessageSend(hfp->clientTask, HFP_INTERNAL_CSR_MOD_INDS_DISABLE_REQ, NULL);
    }
	else
	{
		hfpSendCsrModifyIndicatorsCfm(hfp, hfp_csr_not_inited);
	}
}


/****************************************************************************/
void HfpCsrGetSms(HFP *hfp, uint16 index)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

	if (hfp->use_csr2csr)
    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_GET_SMS_REQ);
        message->index = index;
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_GET_SMS_REQ, message);
    }
    else
    {
	    hfpSendCsrSmsCfm(hfp, hfp_csr_not_inited, 0, NULL);
    }
}

