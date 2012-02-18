/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_aac.c

DESCRIPTION
	This file contains 

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_aac.h"


/**************************************************************************/
void selectOptimalAacCapsSink(uint8 *caps)
{
    /* Object Type */
    if (caps[4] & (1 << 6))
        /* choose MPEG-4 AAC LC */
        caps[4] = (1 << 6);
    else
        /* choose MPEG-2 AAC LC */
        caps[4] = (1 << 7);
    
    /* Select Channel Mode */
    if (caps[6] & (1 << 2))
        /* choose stereo */
        caps[6] &= (1 << 2) | 0xf0;
    else
        /* choose mono */
        caps[6] &= (1 << 3) | 0xf0;
    
    /* Sample rate */
    if (caps[5] & (1 << 0))
        /* choose 44k1 */
        caps[5] = (1 << 0);
    else if (caps[6] & (1 << 7))
        /* choose 48k */
        caps[6] &= (1 << 7) | 0x0f;
    else if (caps[5] & (1 << 1))
        /* choose 32k */
        caps[5] = (1 << 1);
    else if (caps[5] & (1 << 2))
        /* choose 24k */
        caps[5] = (1 << 2);
    else if (caps[5] & (1 << 3))
        /* choose 22.05k */
        caps[5] = (1 << 3);
    else 
        /* choose 16k */
        caps[5] = (1 << 4);
    
    /* Use VBR if available, hence leave bit alone */
}


/**************************************************************************/
void getAacConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode)
{
	/* Set sample rate for both channels based on codec configuration */
	if (service_caps[6] & (1 << 7))
		*rate = 48000;
	else if (service_caps[5] & (1 << 0))
		*rate = 44100;
	else if (service_caps[5] & (1 << 1))
		*rate = 32000;
	else if (service_caps[5] & (1 << 2))
		*rate = 24000;
	else if (service_caps[5] & (1 << 3))
		*rate = 22050;
	else
		*rate = 16000;

	if (service_caps[6] & (1 << 3))
		/* Mono */
		*channel_mode = a2dp_mono;
	else if (service_caps[6] & ( 1 << 2))
		/* Stereo */
		*channel_mode = a2dp_stereo;
}
