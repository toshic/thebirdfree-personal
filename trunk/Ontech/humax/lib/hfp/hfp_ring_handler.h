/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_ring_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_RING_HANDLER_H_
#define HFP_RING_HANDLER_H_


/****************************************************************************
NAME	
	hfpSendInBandRingIndToApp

DESCRIPTION
	Tell the app about the AG's current in-band ring tone setting.

RETURNS
	void
*/
void hfpSendInBandRingIndToApp(HFP *hfp, bool ring);


#endif /* HFP_RING_HANDLER_H_ */
