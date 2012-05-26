/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_multiple_calls_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_MULTIPLE_CALLS_HANDLER_H_
#define HFP_MULTIPLE_CALLS_HANDLER_H_



/****************************************************************************
NAME	
	hfpHandleCallHoldSupportInd

DESCRIPTION
	Call hold parameters received and parsed. 

RETURNS
	void
*/
void hfpHandleCallHoldSupportInd(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationEnable

DESCRIPTION
	Enable call waiting notifications from the AG.

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationEnable(HFP *hfp, const HFP_INTERNAL_AT_CCWA_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationEnableError

DESCRIPTION
	Send an error immediately because the cmd could not be sent out.

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationEnableError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCcwaAtAck

DESCRIPTION
	AG has responded with OK or ERROR to out call waiting notification enable 
	request so pass this result directly to the app.

RETURNS
	void
*/
void hfpHandleCcwaAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldZero

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=0
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this either results in all held calls being released
	or it sets User Determined User Busy (UDUB) for a waiting call.

RETURNS
	void
*/
void hfpHandleChldZero(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldZeroError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldZeroError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldZeroAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=0. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldZeroAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldOne

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=1
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this results in all active calls being released 
	and the held or waiting call being accepted.

RETURNS
	void
*/
void hfpHandleChldOne(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldOneError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldOneError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldOneAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=1. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldOneAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldOneIdx

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=1,<idx>
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this results in all active calls being released 
	and the held or waiting call being accepted.

RETURNS
	void
*/
void hfpHandleChldOneIdx(HFP *hfp, const HFP_INTERNAL_AT_CHLD_1X_REQ_T *msg);


/****************************************************************************
NAME	
	hfpHandleChldOneIdxError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldOneIdxError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldOneIdxAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=1,<idx>. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldOneIdxAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldTwo

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=2
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this places all active calls on hold and accepts 
	the held or waiting call.

RETURNS
	void
*/
void hfpHandleChldTwo(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldTwoError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldTwoError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldTwoAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=2. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldTwoAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldTwoIdx

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=2,<idx>
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this places all active calls on hold and accepts 
	the held or waiting call.

RETURNS
	void
*/
void hfpHandleChldTwoIdx(HFP *hfp, const HFP_INTERNAL_AT_CHLD_2X_REQ_T *msg);


/****************************************************************************
NAME	
	hfpHandleChldTwoIdxError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldTwoIdxError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldTwoIdxAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=2,<idx>. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldTwoIdxAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldThree

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=3
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this adds the held call to the conversation.

RETURNS
	void
*/
void hfpHandleChldThree(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldThreeError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldThreeError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldThreeAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=3. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldThreeAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChldFour

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=4
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this connects the two external calls and disconnects 
	the subscriber from both calls (explicit call transfer). Support for this 
	functionality is optional for the HF device.

RETURNS
	void
*/
void hfpHandleChldFour(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldFourError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldFourError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleChldFourAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=4. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldFourAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_MULTIPLE_CALLS_HANDLER_H_ */
