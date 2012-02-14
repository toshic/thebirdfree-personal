/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/

#ifndef AGHFP_INDICATORS_H_
#define AGHFP_INDICATORS_H_


#include "aghfp.h"


void aghfpHandleSendCindDetails(AGHFP *aghfp);
void aghfpHandleSendCindStatus(AGHFP *aghfp);
void aghfpHandleSendServiceIndicator(AGHFP *aghfp, aghfp_service_availability);
void aghfpSendCallIndicator(AGHFP *aghfp, aghfp_call_status status);
void aghfpHandleSendCallIndicator(AGHFP *aghfp, aghfp_call_status);
void aghfpSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status);
void aghfpHandleSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status);
void aghfpHandleSendCallHeldIndicator(AGHFP *aghfp, aghfp_call_held_status status);
void aghfpHandleSendSignalIndicator(AGHFP *aghfp, uint16 level);
void aghfpHandleSendRoamIndicator(AGHFP *aghfp, aghfp_roam_status status);
void aghfpHandleSendBattChgIndicator(AGHFP *aghfp, uint16 level);
void aghfpHandleCallerIdSetupReq(AGHFP *aghfp, bool enable);
void aghfpHandleCallWaitingSetupReq(AGHFP *aghfp, bool enable);
void aghfpHandleSendCallWaitingNotification(AGHFP *aghfp, uint8 type, uint16 size_number, uint8 *number, uint16 size_string, uint8 *string);
void aghfpHandleSetServiceState(AGHFP *aghfp, bool service_state);


#endif /* AGHFP_INDICATORS_H_ */
