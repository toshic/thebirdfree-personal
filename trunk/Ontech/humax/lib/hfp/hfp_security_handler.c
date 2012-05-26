/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_security_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_security_handler.h"
#include "hfp_common.h"

#include <panic.h>


/****************************************************************************/
void hfpHandleEncryptionChangeInd(HFP *hfp, const CL_SM_ENCRYPTION_CHANGE_IND_T *ind)
{
	MAKE_HFP_MESSAGE(HFP_ENCRYPTION_CHANGE_IND);
	message->encrypted = ind->encrypted;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_ENCRYPTION_CHANGE_IND, message);
}


/****************************************************************************/
void hfpHandleEncryptionKeyRefreshInd(HFP *hfp, const CL_SM_ENCRYPTION_KEY_REFRESH_IND_T *ind)
{
	/* Send indication to app (message is in form of common cfm) */
	hfpSendCommonCfmMessageToApp(HFP_ENCRYPTION_KEY_REFRESH_IND, hfp, (ind->status ? hfp_fail : hfp_success));
}
