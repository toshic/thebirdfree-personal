/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_ok.h
    
DESCRIPTION
	
*/

#ifndef HFP_OK_H_
#define HFP_OK_H_


/****************************************************************************
NAME	
	hfpHandleAtOk

DESCRIPTION
	An OK response received from the AG for the AT cmd just sent.

RETURNS
	void
*/
void hfpHandleAtOk(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAtError

DESCRIPTION
	An ERROR response received from the AG for the AT cmd just sent.

RETURNS
	void
*/
void hfpHandleAtError(HFP *hfp);



#endif /* HFP_OK_H_ */
