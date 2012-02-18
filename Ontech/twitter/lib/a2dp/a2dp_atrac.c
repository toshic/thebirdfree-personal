/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_atrac.c        

DESCRIPTION
	This file contains 

NOTES

*/


#ifdef A2DP_ATRAC

/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_atrac.h"


/*************************************************************************/
void getAtracConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode)
{
    if (!service_caps)
    {
        *rate = 0;
        *channel_mode = a2dp_mono;
        return;
    }
    
    if (service_caps[4] & 0x10)
        /* Mono */
        *channel_mode = a2dp_mono;
    else if (service_caps[4] & 0x08)
        /* Stereo - dual channel */
        *channel_mode = a2dp_dual_channel;
    else
        /* Stereo - joint */
        *channel_mode = a2dp_joint_stereo;

	/* Work out the sample rate based on the codec configuration. */
    if (service_caps[5] & 0x20)
        *rate = 44100;
    else
        *rate = 48000;
}


/**************************************************************************/
void selectOptimalAtracCapsSink(const uint8 *local_caps, uint8 *caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;

	/* Assume ATRAC version has only one of the bits set, so don't process the version */

	/* Select Channel Mode */
    if ((caps[4] & 0x04) && (local_codec[4] & 0x04))
        /* choose joint stereo */
        caps[4] &= 0xe4;
    else if ((caps[4] & 0x08) && (local_codec[4] & 0x08))
        /* choose dual channel */
        caps[4] &= 0xe8;
    else
        /* choose single channel */
        caps[4] &= 0xf0;

    /* Select sample frequency */
    if ((caps[5] & 0x10) && (local_codec[5] & 0x10))
        /* choose 48k */
        caps[5] &= 0x1f;
    else 
        /* choose 44k1 */
        caps[5] &= 0x2f;

	/* Use VBR if available, hence leave bit alone */

	/* Choose a bit rate index. Probably need to have different preference order. */
	if ((caps[5] & 0x04) && (local_codec[5] & 0x04))
	{
		/* 0x0000 */
		caps[5] &= 0x3c;
		caps[6] = 0;
		caps[7] = 0;
	}
	else if ((caps[5] & 0x02) && (local_codec[5] & 0x02))
	{
		/* 0x0001 */
		caps[5] &= 0x3a;
		caps[6] = 0;
		caps[7] = 0;
	}
	else if ((caps[5] & 0x01) && (local_codec[5] & 0x01))
	{
		/* 0x0002 */
		caps[5] &= 0x39;
		caps[6] = 0;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x80) && (local_codec[6] & 0x80))
	{
		/* 0x0003 */
		caps[6] &= 0x80;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x40) && (local_codec[6] & 0x40))
	{
		/* 0x0004 */
		caps[6] &= 0x40;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x20) && (local_codec[6] & 0x20))
	{
		/* 0x0005 */
		caps[6] &= 0x20;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x10) && (local_codec[6] & 0x10))
	{
		/* 0x0006 */
		caps[6] &= 0x10;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x08) && (local_codec[6] & 0x08))
	{
		/* 0x0007 */
		caps[6] &= 0x08;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x04) && (local_codec[6] & 0x04))
	{
		/* 0x0008 */
		caps[6] &= 0x04;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x02) && (local_codec[6] & 0x02))
	{
		/* 0x0009 */
		caps[6] &= 0x02;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[6] & 0x01) && (local_codec[6] & 0x01))
	{
		/* 0x000a */
		caps[6] &= 0x01;
		caps[5] &= 38;
		caps[7] = 0;
	}
	else if ((caps[7] & 0x80) && (local_codec[7] & 0x80))
	{
		/* 0x000b */
		caps[7] &= 0x80;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x40) && (local_codec[7] & 0x40))
	{
		/* 0x000c */
		caps[7] &= 0x40;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x20) && (local_codec[7] & 0x20))
	{
		/* 0x000d */
		caps[7] &= 0x20;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x10) && (local_codec[7] & 0x10))
	{
		/* 0x000e */
		caps[7] &= 0x10;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x08) && (local_codec[7] & 0x08))
	{
		/* 0x000f */
		caps[7] &= 0x08;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x04) && (local_codec[7] & 0x04))
	{
		/* 0x0010 */
		caps[7] &= 0x04;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else if ((caps[7] & 0x02) && (local_codec[7] & 0x02))
	{
		/* 0x0011 */
		caps[7] &= 0x02;
		caps[5] &= 38;
		caps[6] = 0;
	}
	else
	{
		/* 0x0012 */
		caps[7] &= 0x01;
		caps[5] &= 38;
		caps[6] = 0;
	}
}

#else
	static const int dummy;
#endif /* A2DP_ATRAC */
