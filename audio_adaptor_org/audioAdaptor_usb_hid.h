/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles the USB HID connection to the host.
*/

#ifndef AUDIOADAPTOR_USB_HID_H
#define AUDIOADAPTOR_USB_HID_H

#include "audioAdaptor_private.h"


void usbHidInitTimeCritical(void);

void usbHidInit(void);

void usbHidHandleInterfaceEvent(Source source);

/****************************************************************************
 Functions that send messages to the PC to signal some event
*/
void usbHidSignalVolumeUp (void);

void usbHidSignalVolumeDown (void);

void usbHidSignalMuteState (bool state);

void usbHidIssueHidCommand(mvdAppEvent event);

void usbHidIssueNextHidCommand(void);


#endif

