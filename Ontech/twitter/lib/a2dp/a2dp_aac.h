/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_aac.h
    
DESCRIPTION
	
*/

#ifndef A2DP_AAC_H_
#define A2DP_AAC_H_


/*************************************************************************
NAME    
     selectOptimalAacCapsSink
    
DESCRIPTION
    Selects the optimal configuration for AAC playback by setting a single 
	bit in each field of the passed caps structure.

	Note that the priority of selecting features is a
	design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalAacCapsSink(uint8 *caps);


/*************************************************************************
NAME    
     getAacConfigSettings
    
DESCRIPTION    
	Return the codec configuration settings (rate and channel mode) for the physical codec based
	on the A2DP codec negotiated settings.
*/
void getAacConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode);


#endif /* A2DP_AAC_H_ */
