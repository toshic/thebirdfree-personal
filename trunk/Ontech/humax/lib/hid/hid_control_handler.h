/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of BlueLab 4.1.2-Release */
#ifndef __hid_control_handler_h
#define __hid_control_handler_h

void hidHandleControlPacket(HID *hid, Source source);
void hidHandleInternalControlRequest(HID *hid, HID_INTERNAL_CONTROL_REQ_T *req);

#ifdef HID_DEVICE
void hidHandleInternalGetIdleResponse(HID *hid, HID_INTERNAL_GET_IDLE_RES_T *res);
void hidHandleInternalSetIdleResponse(HID *hid, HID_INTERNAL_SET_IDLE_RES_T *res);
void hidHandleInternalGetProtocolResponse(HID *hid, HID_INTERNAL_GET_PROTOCOL_RES_T *res);
void hidHandleInternalSetProtocolResponse(HID *hid, HID_INTERNAL_SET_PROTOCOL_RES_T *res);
void hidHandleInternalGetReportResponse(HID *hid, HID_INTERNAL_GET_REPORT_RES_T *res);
void hidHandleInternalSetReportResponse(HID *hid, HID_INTERNAL_SET_REPORT_RES_T *res);
#endif

#ifdef HID_HOST
void hidHandleInternalGetIdleRequest(HID *hid, HID_INTERNAL_GET_IDLE_REQ_T *req);
void hidHandleInternalSetIdleRequest(HID *hid, HID_INTERNAL_SET_IDLE_REQ_T *req);
void hidHandleInternalGetProtocolRequest(HID *hid, HID_INTERNAL_GET_PROTOCOL_REQ_T *req);
void hidHandleInternalSetProtocolRequest(HID *hid, HID_INTERNAL_SET_PROTOCOL_REQ_T *req);
void hidHandleInternalGetReportRequest(HID *hid, HID_INTERNAL_GET_REPORT_REQ_T *req);
void hidHandleInternalSetReportRequest(HID *hid, HID_INTERNAL_SET_REPORT_REQ_T *req);
#endif

void hidHandleInternalRequestTimeout(HID *hid);
void hidRequestFailed(HID *hid, int request, hid_status status);

#endif
