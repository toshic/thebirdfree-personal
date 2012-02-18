/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_mp3.c

DESCRIPTION
	This file contains 

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_caps_parse.h"
#include "a2dp_private.h"
#include "a2dp_mp3.h"


static void selectMp3Caps(uint8 *caps, const uint8 *local_codec)
{
	/* layer III */
    caps[4] &= 0x3f;
    
    /* Use CRC protection if available, hence leave bit alone */
    
    /* Select Channel Mode */
    if ((caps[4] & 0x01) && (local_codec[4] & 0x01))
        /* choose joint stereo */
        caps[4] &= 0xf1;
    else if ((caps[4] & 0x02) && (local_codec[4] & 0x02))
        /* choose stereo */
        caps[4] &= 0xf2;
    else if ((caps[4] & 0x04) && (local_codec[4] & 0x04))
        /* choose dual channel */
        caps[4] &= 0xf4;
    else
        /* choose mono */
        caps[4] &= 0xf8;
    
    /* MPF-1 and clear RFA */
    caps[5] &= 0x3f;
    
    /* Sample rate */
    if ((caps[5] & 0x02) && (local_codec[5] & 0x02))
        /* choose 44k1 */
        caps[5] &= 0xc2;
    else if ((caps[5] & 0x01) && (local_codec[5] & 0x01))
        /* choose 48k */
        caps[5] &= 0xc1;
    else if ((caps[5] & 0x04) && (local_codec[5] & 0x04))
        /* choose 32k */
        caps[5] &= 0xc4;
    else if ((caps[5] & 0x08) && (local_codec[5] & 0x08))
        /* choose 24k */
        caps[5] &= 0xc8;
    else if ((caps[5] & 0x10) && (local_codec[5] & 0x10))
        /* choose 22.05k */
        caps[5] &= 0xd0;
    else 
        /* choose 16k */
        caps[5] &= 0xe0;
    
    /* Use VBR if available, hence leave bit alone */
    
    /* Bit Rate.
	   We try and use the rate nearest to 128kbps.
       In reality the source won't encode MP3 in realtime
       so it will probably only have a single bit set
       representing it's pre-compressed file.
    */
    if ((caps[6] & 0x02) && (local_codec[6] & 0x02))
    {
        /* choose 128k */
        caps[6] &= 0x82; 
        caps[7] = 0;
    }
    else if ((caps[6] & 0x01) && (local_codec[6] & 0x01))
    {
        /* choose 112k */
        caps[6] &= 0x81; 
        caps[7] = 0;
    }
    else if ((caps[6] & 0x04) && (local_codec[6] & 0x04))
    {
        /* choose 160k */
        caps[6] &= 0x84; 
        caps[7] = 0;
    }
    else if ((caps[7] & 0x80) && (local_codec[7] & 0x80))
    {
        /* choose 96k */
        caps[6] &= 0x80; 
        caps[7] &= 0x80;
    }
    else if ((caps[6] & 0x08) && (local_codec[6] & 0x08))
    {
        /* choose 192k */
        caps[6] &= 0x88;
        caps[7] = 0;
    }
    else if ((caps[7] & 0x40) && (local_codec[7] & 0x40))
    {
        /* choose 80k */
        caps[6] &= 0x80; 
        caps[7] &= 0x40;
    }
    else if ((caps[6] & 0x10) && (local_codec[6] & 0x10))
    {
        /* choose 224k */
        caps[6] &= 0x90;
        caps[7] = 0;
    }
    else if ((caps[7] & 0x20) && (local_codec[7] & 0x20))
    {
        /* choose 64k */
        caps[6] &= 0x80; 
        caps[7] &= 0x20;
    }
    else if ((caps[6] & 0x20) && (local_codec[6] & 0x20))
    {
        /* choose 256k */
        caps[6] &= 0xa0;
        caps[7] = 0;
    }
    else if ((caps[7] & 0x10) && (local_codec[7] & 0x10))
    {
        /* choose 56k */
        caps[6] &= 0x80; 
        caps[7] &= 0x10;
    }
    else if ((caps[6] & 0x40) && (local_codec[6] & 0x40))
    {
        /* choose 320k */
        caps[6] &= 0xc0;
        caps[7] = 0;
    }
    else if ((caps[7] & 0x08) && (local_codec[7] & 0x08))
    {
        /* choose 48k */
        caps[6] &= 0x80; 
        caps[7] &= 0x08;
    }
    else if ((caps[7] & 0x04) && (local_codec[7] & 0x04))
    {
        /* choose 40k */
        caps[6] &= 0x80; 
        caps[7] &= 0x04;
    }
    else if ((caps[7] & 0x02) && (local_codec[7] & 0x02))
    {
        /* choose 32k */
        caps[6] &= 0x80; 
        caps[7] &= 0x02;
    }
}


/**************************************************************************/
void selectOptimalMp3CapsSink(const uint8 *local_caps, uint8 *caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;
	
    if (!caps)
        return;
    
    selectMp3Caps(caps, local_codec);
}


/**************************************************************************/
void selectOptimalMp3CapsSource(const uint8 *local_caps, uint8 *caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;

    selectMp3Caps(caps, local_codec);
}


/**************************************************************************/
void getMp3ConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode)
{
    if (!service_caps)
    {
        *rate = 0;
        *channel_mode = a2dp_mono;
        return;
    }

    /* Set sample rate for both channels based on codec configuration */
    if (service_caps[5] & 0x01)
		*rate = 48000;
	else if (service_caps[5] & 0x02)
		*rate = 44100;
	else if (service_caps[5] & 0x04)
		*rate = 32000;
	else if (service_caps[5] & 0x08)
		*rate = 24000;
	else if (service_caps[5] & 0x10)
		*rate = 22050;
	else
		*rate = 16000;

	if (service_caps[4] & 0x08)
		/* Mono */
		*channel_mode = a2dp_mono;
	else if (service_caps[4] & 0x04)
		/* Stereo - dual channel */
		*channel_mode = a2dp_dual_channel;
	else if (service_caps[4] & 0x02)
		/* Stereo */
		*channel_mode = a2dp_stereo;
	else
		/* Stereo - joint */
		*channel_mode = a2dp_joint_stereo;
}
