/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    hid_profile_handler.h
DESCRIPTION
	Header file for the HID profile library message handlers.
*/

#ifndef HID_PROFILE_HANDLER_H_
#define HID_PROFILE_HANDLER_H_

#include "hid.h"
#include "hid_private.h"

void hidProfileHandler(Task task, MessageId id, Message message);
void hidLibProfileHandler(Task task, MessageId id, Message message);

#endif /* HID_PROFILE_HANDLER_H_ */
