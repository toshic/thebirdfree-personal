/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_current_sep.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/

#include "a2dp_current_sep_handler.h"
#include "a2dp_private.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"

#include <print.h>
#include <stdlib.h>
#include <string.h>


/*****************************************************************************/
void A2dpGetCurrentSepCapabilities(A2DP *a2dp)
{

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpGetCurrentSepCapabilities NULL instance\n"));
#endif

	if ((a2dp->signal_conn.signalling_state == avdtp_state_open) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_streaming) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_starting) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_suspending) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfig_reading_caps) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfiguring)
		)
	{
		if (a2dpSendGetCapabilities(a2dp, a2dp->sep.remote_seid))
		{
			/* Start watchdog */
			PRINT(("A2dpGetCurrentSepCapabilities seid=%d\n",a2dp->sep.remote_seid));
			MessageSendLater(&a2dp->task, A2DP_INTERNAL_GET_CAPS_TIMEOUT_IND, 0, WATCHDOG_TGAVDP100);
			return;
		}
	}

	sendGetCurrentSepCapabilitiesCfm(a2dp, a2dp_wrong_state, 0, 0);
}


/*****************************************************************************/
uint8 *A2dpGetCurrentSepConfiguration(A2DP *a2dp, uint16 *size_caps)
{
	if ((a2dp->signal_conn.signalling_state == avdtp_state_configured) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_opening) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_remote_opening) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_open) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_streaming) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_starting) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_suspending)
		)
	{
		if (a2dp->sep.configured_service_caps)
		{
            uint8 *caps = (uint8 *)malloc(a2dp->sep.configured_service_caps_size);
            if (caps != NULL)
            {
				memmove(caps, a2dp->sep.configured_service_caps, a2dp->sep.configured_service_caps_size);   
				*size_caps = a2dp->sep.configured_service_caps_size;
				return caps;
			}
		}
	}

    /* Control only reaches here if we haven't got valid caps to return */
    *size_caps = 0;
    return 0;
}
