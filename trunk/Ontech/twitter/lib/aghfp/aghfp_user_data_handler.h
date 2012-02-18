/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/

#ifndef AGHFP_USER_DATA_HANDLER_H_
#define AGHFP_USER_DATA_HANDLER_H_


/****************************************************************************
    Attempt to send the user supplied data to the remote device.
*/
void aghfpHandleUserDataReq(AGHFP *aghfp, const AGHFP_INTERNAL_USER_DATA_REQ_T *req);


/****************************************************************************
    The aghfp library is in the wrong state to attempt sending the supplied
    data so send back an error to the client.
*/
void aghfpHandleUserDataReqFail(AGHFP *aghfp, const AGHFP_INTERNAL_USER_DATA_REQ_T *req);


#endif /* AGHFP_USER_DATA_HANDLER_H_ */
