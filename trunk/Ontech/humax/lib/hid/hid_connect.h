/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    hid_connect.h
DESCRIPTION
	Header file for the internal connection message handler functions.
*/

#ifndef HID_CONNECT_H_
#define HID_CONNECT_H_

void hidHandleInternalConnectReq(HID_LIB *hid_lib, const HID_INTERNAL_CONNECT_REQ_T *req);
void hidHandleInternalConnectRes(HID *hid, const HID_INTERNAL_CONNECT_RES_T *res);
void hidHandleInternalConnectTimeoutInd(HID *hid);
void hidHandleInternalDisconnectReq(HID *hid);
void hidHandleInternalDisconnectTimeoutInd(HID *hid);
void hidHandleInternalConnectControlInd(HID *hid, HID_INTERNAL_CONNECT_IND_T *ind);
void hidHandleInternalConnectInterruptInd(HID *hid, HID_INTERNAL_CONNECT_IND_T *ind);
void hidRejectInternalConnectInd(HID *hid, HID_INTERNAL_CONNECT_IND_T *ind);
    
#endif /* HID_CONNECT_H_ */
