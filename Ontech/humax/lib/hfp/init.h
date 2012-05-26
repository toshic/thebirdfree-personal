/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    init.h
    
DESCRIPTION
	
*/

#ifndef HFP_INIT_H_
#define HFP_INIT_H_




/****************************************************************************
NAME	
	hfpHandleInternalInitReq

DESCRIPTION
	Send internal init req messages until we have completed the profile
	lib initialisation.

RETURNS
	void
*/
void hfpHandleInternalInitReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpSendInternalInitCfm

DESCRIPTION
	Send an internal init cfm message.

RETURNS
	void
*/

void hfpSendInternalInitCfm(Task task, hfp_init_status	s, uint8 c);


/****************************************************************************
NAME	
	hfpHandleInternalInitCfm

DESCRIPTION
	This message is sent once various parts of the library initialisation 
	process have completed.

RETURNS
	void
*/
void hfpHandleInternalInitCfm(HFP *hfp, const HFP_INTERNAL_INIT_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpResetConnectionRelatedState

DESCRIPTION
	Reset the connection related state in the profile instance as we don't
	currently have a connection.

RETURNS
	Task
*/
void hfpResetConnectionRelatedState(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSyncRegisterCfm

DESCRIPTION
	Handles confirmation registering the HFP to receive Synchronous connection
	notifications from the Connection library.  This completes the HFP initialisation
	process - send message to app.

RETURNS
	void
*/
void hfpHandleSyncRegisterCfm(HFP *hfp);


#endif /* HFP_INIT_H_ */
