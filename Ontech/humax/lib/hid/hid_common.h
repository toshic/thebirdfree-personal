/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    hid_common.h    
DESCRIPTION
	Header file for the common HID functions.
*/

#ifndef HID_COMMON_H_
#define HID_COMMON_H_

#include "hid.h"
#include "hid_private.h"

HID *hidCreate(HID_LIB *hid_lib, const bdaddr *addr);
HID *hidFindFromBddr(HID_LIB *hid_lib, const bdaddr *addr);
HID *hidFindFromSink(HID_LIB *hid_lib, Sink sink);
HID *hidFindFromConnectionId(HID_LIB *hid_lib, uint16 con_id);
void hidDestroy(HID *hid);

void hidSetState(HID *hid, hidState state);
hidState hidGetState(HID *hid);

void hidL2capConfigure(HID *hid, l2cap_config_params *l2cap_config, int psm);

bool hidConnIsConnected(HID *hid, int psm);
bool hidConnIsDisconnected(HID *hid, int psm);
void hidConnConnecting(HID *hid, int psm, uint16 con_id);
void hidConnConnected(HID *hid, int psm, Sink sink, uint16 mtu);
void hidConnDisconnect(HID *hid, int psm);
void hidConnDisconnected(HID *hid, Sink sink);
void hidConnDisconnectedPsm(HID *hid, int psm);

#endif /* HID_COMMON_H_ */

