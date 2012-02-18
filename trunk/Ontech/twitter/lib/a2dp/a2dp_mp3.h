/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_mp3.h
    
DESCRIPTION
	
*/

#ifndef A2DP_MP3_H_
#define A2DP_MP3_H_


/*************************************************************************
NAME    
     selectOptimalMp3CapsSink
    
DESCRIPTION
    Selects the optimal configuration for MP3 playback by setting a single 
	bit in each field of the passed caps structure.

	Note that the priority of selecting features is a
	design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalMp3CapsSink(const uint8 *local_caps, uint8 *caps);


/*************************************************************************
NAME    
     selectOptimalMp3CapsSource
    
DESCRIPTION
    Selects the optimal configuration for MP3 playback by setting a single 
	bit in each field of the passed caps structure.

	Note that the priority of selecting features is a
	design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalMp3CapsSource(const uint8 *local_caps, uint8 *caps);


/*************************************************************************
NAME    
     getMp3ConfigSettings
    
DESCRIPTION    
	Return the codec configuration settings (rate and channel mode) for the physical codec based
	on the A2DP codec negotiated settings.
*/
void getMp3ConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode);


#endif /* A2DP_MP3_H_ */
