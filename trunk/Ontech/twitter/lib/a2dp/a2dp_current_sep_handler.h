/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_current_sep_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_CURRENT_SEP_HANDLER_H_
#define A2DP_CURRENT_SEP_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	sendGetCurrentSepCapabilitiesCfm

DESCRIPTION
    Send cfm message to app as a result of calling the A2dpGetCurrentSepCapabilities API.
*/
void sendGetCurrentSepCapabilitiesCfm(A2DP *a2dp, a2dp_status_code status, const uint8 *caps, uint16 size_caps);


#endif /* A2DP_CURRENT_SEP_HANDLER_H_ */
