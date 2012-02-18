/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    init.h
    
DESCRIPTION
	
*/

#ifndef SPP_INIT_H_
#define SPP_INIT_H_


#include "spp_private.h"



/****************************************************************************
NAME	
	sppCreateTaskInstance

DESCRIPTION
	Creates a task instance by allocating the memory for the task and 
    initialising it.
*/
SPP *sppCreateTaskInstance(uint16 priority, Sink sink, Task app, sppState state, uint8 chan, uint16 length, const uint8 *rec, const bool no_rec, uint16 lazy);


/****************************************************************************
NAME	
	sppInitTaskData

DESCRIPTION
	Initialise the task data fileds with the supplied values.
*/
void sppInitTaskData(SPP *spp, uint16 priority, Sink sink, Task app, sppState state, uint8 chan, uint16 length, const uint8 *rec, const bool no_rec, uint16 lazy);


/****************************************************************************
NAME	
	sppHandleInternalInitReq

DESCRIPTION
	Internal request to complete SPP library initialisation.
*/
void sppHandleInternalInitReq(SPP *spp, const SPP_INTERNAL_INIT_REQ_T *req);


/****************************************************************************
NAME	
	sppHandleRfcommRegisterCfm

DESCRIPTION
	Confirmation that Rfcomm channel has been allocated (or not).
*/
void sppHandleRfcommRegisterCfm(SPP *spp, const CL_RFCOMM_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	sppSendInitCfmToApp

DESCRIPTION
	Send an SPP_INIT_CFM message to the client task to show that spp lib 
    initialisation has completed.
*/
void sppSendInitCfmToApp(SPP *spp, spp_init_status status);


#endif /* SPP_INIT_H_ */
