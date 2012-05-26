/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_ring_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_parse.h"
#include "hfp_indicators_handler.h"
#include "hfp_ring_handler.h"

#include <panic.h>


/****************************************************************************
NAME	
	hfpHandleRingInd

DESCRIPTION
	Send ring indication to the app and update hfp profile state

RETURNS
	void
*/
static void sendRingIndToApp(HFP *hfp)
{
	/* Send a message to the application */
	MAKE_HFP_MESSAGE(HFP_RING_IND);
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_RING_IND, message);

	/* This is necessary when dealing with 0.96 AGs or HSP */
	if (!hfp->indicator_status.indexes.call_setup && supportedProfileIsHfp(hfp->hfpSupportedProfile))
		hfpSendIndicatorCallSetupToApp(hfp, hfp_incoming_call_setup);

    /* In hsp mode we just want to update the call state. */
    if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
	{
		/* B-8153 Workaround for AG's that open audio connection and then send RING indications
 		   This resulted in an active call in the hfpIncomingCallEstablish state */
		if(hfp->state != hfpActiveCall)
		{
        	hfpSetState(hfp, hfpIncomingCallEstablish);
		}
	}
}


/****************************************************************************
NAME	
	hfpSendInBandRingIndToApp

DESCRIPTION
	Tell the app about the AG's current in-band ring tone setting.

RETURNS
	void
*/
void hfpSendInBandRingIndToApp(HFP *hfp, bool ring)
{
	MAKE_HFP_MESSAGE(HFP_IN_BAND_RING_IND);
	message->ring_enabled = ring;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_IN_BAND_RING_IND, message);
}


/****************************************************************************
NAME	
	hfpHandleRing

DESCRIPTION
	Received a RING indication.

AT INDICATION
	RING

RETURNS
	void
*/
void hfpHandleRing(Task profileTask)
{
	HFP *hfp = (HFP *) profileTask;

	switch(hfp->state)
	{
	case hfpSlcConnecting:
		/* Just ignore the ring. Makes us more forgiving to AGs that send RINGs early! */
        break;

	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		sendRingIndToApp(hfp);
		break;
			
	case hfpInitialising:
	case hfpReady:
	default:
		/* Panic in debug and ignore in release lib variants */
		HFP_DEBUG(("Received AT RING under invalid state: 0x%x \n", hfp->state));
		break;
	}
}


/****************************************************************************
NAME	
	hfpHandleInBandRingTone

DESCRIPTION
	Received a BSIR indication.

AT INDICATION
	+BSIR

RETURNS
	void
*/
void hfpHandleInBandRingTone(Task profileTask, const struct hfpHandleInBandRingTone *ind)
{
	HFP *hfp = (HFP *) profileTask;

	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Pass the indiaction on to the app */
	hfpSendInBandRingIndToApp(hfp, ind->enable);
}


/****************************************************************************
NAME	
	hfpHandleInBandRingToneDisable

DESCRIPTION
	Received a BSIR disable indication.

AT INDICATION
	+BSIR: 0

RETURNS
	void
*/
void hfpHandleInBandRingToneDisable(Task profileTask)
{
	struct hfpHandleInBandRingTone ind;
	ind.enable = 0;
	hfpHandleInBandRingTone(profileTask, &ind);
}


/****************************************************************************
NAME	
	hfpHandleInBandRingToneEnable

DESCRIPTION
	Received a BSIR enable indication.

AT INDICATION
	+BSIR: 1

RETURNS
	void
*/
void hfpHandleInBandRingToneEnable(Task profileTask)
{
	struct hfpHandleInBandRingTone ind;
	ind.enable = 1;
	hfpHandleInBandRingTone(profileTask, &ind);
}
