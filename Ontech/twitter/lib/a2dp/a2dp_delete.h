/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_delete.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_DELETE_H_
#define A2DP_DELETE_H_

#include "a2dp.h"


/****************************************************************************
NAME	
	a2dpDeleteTask

DESCRIPTION
	Send a message that will delete the task.
*/
void a2dpDeleteTask(A2DP *a2dp);


/****************************************************************************
NAME	
	a2dpHandleDeleteTask

DESCRIPTION
	Detele a dynamically allocated A2DP task instance. Before deleting make 
    sure all messages for that task are flushed from the message queue.
*/
void a2dpHandleDeleteTask(A2DP *a2dp);


#endif /* A2DP_DELETE_H_ */
