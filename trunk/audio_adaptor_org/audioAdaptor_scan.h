/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles discovery of remote devices.
*/


#ifndef AUDIOADAPTOR_SCAN_H
#define AUDIOADAPTOR_SCAN_H


#include "audioAdaptor_private.h"

#include <connection.h>


void scanWriteEirData (CL_DM_LOCAL_NAME_COMPLETE_T *message);

void scanMakeDiscoverable (void);

void scanMakeConnectable (void);

void scanMakeUnconnectable (void);

void scanKickInquiryScan (void);

void scanFlushInquireResults (void);

void scanStoreInquireResult (const CL_DM_INQUIRE_RESULT_T *prim);

bool scanLoadNextInquireResult (void);

bool scanHaveInquireResults (void);

bool scanRepeatSdpSearch (void);

void scanKickFirstSdpSearch (void);

bool scanKickNextSdpSearch (void);

void scanCancelSdpSearch (void);

bool scanHandleSdpSearchResult (CL_SDP_SERVICE_SEARCH_CFM_T *prim);

bool scanIsSuitableDevice (devInstanceTaskData *inst);

void scanProcessNextInquireResult (void);

void scanSearchComplete (devInstanceTaskData *inst);

void scanSdpSearchReq (void);

void scanInquiryScanReq (void);

void scanCancelInquiryScan (void);

void scanStartAppInquiryTimer (void);

void scanStopAppInquiryTimer (void);


#endif /* AUDIOADAPTOR_SCAN_H */

