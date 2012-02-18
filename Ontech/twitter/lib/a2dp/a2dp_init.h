/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_init.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_INIT_H_
#define A2DP_INIT_H_

#include "a2dp.h"


/***************************************************************************
NAME
	a2dpSendInitCfmToClient

DESCRIPTION
    Send an A2DP_INIT_CFM message to the client task idicating the outcome
    of the library initialisation request.
*/
void a2dpSendInitCfmToClient(A2DP *a2dp, a2dp_status_code status, device_sep_list *seps);


/***************************************************************************
NAME
	a2dpInitTask

DESCRIPTION
    Initialise the a2dp task data structure with the supplied parameters.
*/
void a2dpInitTask(A2DP *a2dp, Task clientTask);


#endif /* A2DP_INIT_H_ */
