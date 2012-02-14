/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#include "aghfp_common.h"

#include <panic.h>


/*****************************************************************************/
void aghfpSetState(AGHFP *aghfp, aghfp_state state)
{
	aghfp->state = state;
}


/*****************************************************************************
	Create a common cfm message (many AGHFP defined messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.
*/
void aghfpSendCommonCfmMessageToApp(uint16 message_id, AGHFP *aghfp, aghfp_lib_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_COMMON_CFM_MESSAGE);
	message->status = status;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, message_id, message);
}


/*****************************************************************************/
bool supportedProfileIsHfp(aghfp_profile profile)
{
    if ((profile == aghfp_handsfree_profile) || (profile == aghfp_handsfree_15_profile))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*****************************************************************************/
bool supportedProfileIsHfp15(aghfp_profile profile)
{
    if (profile == aghfp_handsfree_15_profile)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*****************************************************************************/
bool supportedProfileIsHsp(aghfp_profile profile)
{
    if (profile == aghfp_headset_profile)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************/
bool aghfpCodecBitmapToAghfpEnum(unsigned int aghfp_codec_bitmap, uint8 *aghfp_codec_enum)
{
	bool rtn_value = TRUE;

	/* Translate from internal codec bitmap to AGHFP enum */
	switch(aghfp_codec_bitmap)
	{
		case(wbs_codec_mask_cvsd):
			*aghfp_codec_enum = aghfp_wbs_codec_cvsd;
			break;
		case(wbs_codec_mask_sbc):
			*aghfp_codec_enum = aghfp_wbs_codec_sbc;
			break;
		case(wbs_codec_mask_amr_wb):
			*aghfp_codec_enum = aghfp_wbs_codec_amr;
			break;
		case(wbs_codec_mask_evrc_wb):
			*aghfp_codec_enum = aghfp_wbs_codec_evrc;
			break;
		default:
			rtn_value = FALSE;
			break;
	}

	return(rtn_value);
}

