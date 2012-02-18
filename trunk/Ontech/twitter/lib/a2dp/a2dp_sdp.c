/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_sdp.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_private.h"
#include "a2dp_sdp.h"

#include <connection.h>


/*****************************************************************************/
void a2dpHandleSdpRegisterCfm(A2DP *a2dp, const CL_SDP_REGISTER_CFM_T *cfm)
{
	a2dp->sep.configured_service_caps_size--;
    if (cfm->status==success)
	{
        /* Register the l2cap psm if all service records have been registered */
		if (!a2dp->sep.configured_service_caps_size)
			a2dpRegisterL2cap(a2dp);
	}
    else
	{
        /* Send indication that the initialisation failed */
        a2dpSendInitCfmToClient(a2dp, a2dp_sdp_fail, 0);
	}
}
