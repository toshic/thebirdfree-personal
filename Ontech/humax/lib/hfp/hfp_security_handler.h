/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_security_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_SECURITY_HANDLER_H_
#define HFP_SECURITY_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleEncryptionChangeInd

DESCRIPTION	
	Handle an unsolicited CL_SM_ENCRYPTION_CHANGE_IND message from the
	connection lib informing us of a change in the encryption status of a 
	connection (identified by it's sink).
*/
void hfpHandleEncryptionChangeInd(HFP *hfp, const CL_SM_ENCRYPTION_CHANGE_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleEncryptionChangeInd

DESCRIPTION	
	Handle an unsolicited CL_SM_ENCRYPTION_KEY_REFRESH_IND message from the
	connection lib informing us the encryption key of a connection (identified 
	by it's sink) has been refreshed.
*/
void hfpHandleEncryptionKeyRefreshInd(HFP *hfp, const CL_SM_ENCRYPTION_KEY_REFRESH_IND_T *ind);


#endif /* HFP_SECURITY_HANDLER_H_ */
