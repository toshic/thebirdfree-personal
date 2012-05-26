/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_send_data.h
    
DESCRIPTION
	
*/

#ifndef HFP_SEND_DATA_H_
#define HFP_SEND_DATA_H_


/****************************************************************************
NAME	
	hfpSendAtCmd

DESCRIPTION
	Send an AT command by putting it into the RFCOMM buffer.

RETURNS
	void
*/
void hfpSendAtCmd(Task profileTask, uint16 length, const char *at_cmd);


/****************************************************************************
NAME	
	hfpHandleWaitAtTimeout

DESCRIPTION
	Waiting for OK/ ERROR response to AT cmd timeout has expired. This means 
	that we have not received a response for the last AT cmd. So we don't
	get completely stuck, send out the next cmd anyway.

RETURNS
	void
*/
void hfpHandleWaitAtTimeout(HFP *hfp);


/****************************************************************************
NAME	
	hfpSendNextAtCmd

DESCRIPTION
	Attempt to send the next AT cmd (if any) pending in the RFCOMM buffer.

RETURNS
	void
*/
void hfpSendNextAtCmd(HFP *hfp, uint16 offset, const uint8 *data_out);



#endif /* HFP_SEND_DATA_H_ */
