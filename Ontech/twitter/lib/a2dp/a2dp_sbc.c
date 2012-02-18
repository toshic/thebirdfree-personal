/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_sbc.c        

DESCRIPTION
	This file contains 

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_caps_parse.h"
#include "a2dp_sbc.h"



/* SBC header defines */
#define SBC_HEADER_CHANNEL_SHIFT    (0x02)
#define SBC_HEADER_CHANNEL_MASK     (0x0c)
#define SBC_HEADER_BLOCKS_SHIFT     (0x04)
#define SBC_HEADER_BLOCKS_MASK      (0x30)
#define SBC_HEADER_FREQUENCY_SHIFT  (0x06)
#define SBC_HEADER_FREQUENCY_MASK   (0xc0)
#define SBC_HEADER_SUBBANDS_8       (0x01)
#define SBC_HEADER_ALLOCATION_SNR   (0x02)



typedef struct
{
    uint8                   sub_bands;
    uint8                   blocks;
    uint16                  frequency;
    a2dp_channel_mode		mode;
} sbc_parameters;



/*****************************************************************************/
static void extractSbcParameters(sbc_parameters *sbc, uint8 sbc_format)
{
	/* Takes an SBC header and decodes the parameters. */
    const uint16 freq_lut[] = {16000,32000,44100,48000};
    sbc->sub_bands = (sbc_format & SBC_HEADER_SUBBANDS_8?8:4);
    sbc->blocks = 4*(1+((sbc_format & SBC_HEADER_BLOCKS_MASK) >> SBC_HEADER_BLOCKS_SHIFT));
    sbc->frequency  = freq_lut[(sbc_format & SBC_HEADER_FREQUENCY_MASK) >> SBC_HEADER_FREQUENCY_SHIFT];
    sbc->mode = (a2dp_channel_mode) ((sbc_format & SBC_HEADER_CHANNEL_MASK) >> SBC_HEADER_CHANNEL_SHIFT);
}


/*****************************************************************************/
static uint16 frameLengthFromRate(const sbc_parameters *sbc, uint32 rate)
{
	/* Returns the frame length needed to produce data at the specified rate */
    return (uint16)((rate * (uint32)sbc->sub_bands * (uint32)sbc->blocks) / ((uint32)8 * (uint32)sbc->frequency));
}


/*****************************************************************************/
static uint8 bitpoolFromFrameLength(const sbc_parameters *sbc, uint16 frame_length)
{
	/* Calculates the bitpool required to produce the requested frame length */
    uint8 bitpool;

    /* Equations derived from A2DP Spec, Section 12.9. */
    switch(sbc->mode)
    {
        default:
        case a2dp_mono:
            bitpool = (8*(frame_length - 4) - 4*sbc->sub_bands) / sbc->blocks;
            break;

        case a2dp_dual_channel:
            bitpool = (8*(frame_length - 4) - 8*sbc->sub_bands) / (2*sbc->blocks);
            break;

        case a2dp_stereo:
            bitpool = (8*(frame_length - 4) - 8*sbc->sub_bands) / sbc->blocks;
            break;

        case a2dp_joint_stereo:
            bitpool = (8*(frame_length - 4) - 9*sbc->sub_bands) / sbc->blocks;
            break;
    }

    return bitpool;
}


static void getSbcCaps(uint8 *caps, const uint8 *local_codec)
{
	/* Select Channel Mode. Only mono is mandatory at the source. */
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
    
    /* Select Block Length. All lengths are mandatory at Source and Sink. */
    if (caps[5] & 0x10)
        /* choose 16 */
        caps[5] &= 0x1f;
    else if (caps[5] & 0x20)
        /* choose 12 */
        caps[5] &= 0x2f;
    else if (caps[5] & 0x40)
        /* choose 8 */
        caps[5] &= 0x4f;
    else
        /* choose 4 */
        caps[5] &= 0x8f;
    
    /* Select Subbands. 8 subbands is mandatory at both Source and Sink. */
    if (caps[5] & 0x04)
        /* choose 8 */
        caps[5] &= 0xf7;
    else
        /* choose 4 */
        caps[5] &= 0xfb;
    
    /* Select Allocation Method. Loudness is mandatory at both Source and Sink. */
    if (caps[5] & 0x01)
        /* choose Loudness */
        caps[5] &= 0xfd;
    else
        /* choose SNR */
        caps[5] &= 0xfe;
}


/*************************************************************************/
void getSbcConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode)
{
    if (!service_caps)
    {
        *rate = 0;
        *channel_mode = a2dp_mono;
        return;
    }
    
    /* Work out the sample rate based on the codec configuration. */
    if (service_caps[4] & 0x10)
        *rate = 48000;
    else if (service_caps[4] & 0x20)
        *rate = 44100;
    else if (service_caps[4] & 0x40)
        *rate = 32000;
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


/**************************************************************************/
void selectOptimalSbcCapsSink(const uint8 *local_caps, uint8 *caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;

	/* All codec caps are mandatory at the Sink end except the sample rates where only 
		48k and 44.1k are mandatory, but at least one of these must be supported at the Source end.
	*/
    /* Select sample frequency */
    if (caps[4] & 0x20)
        /* choose 44k1 */
        caps[4] &= 0x2f;
    else if (caps[4] & 0x10)
        /* choose 48k */
        caps[4] &= 0x1f;
    else if (caps[4] & 0x40)
        /* choose 32k */
        caps[4] &= 0x4f;
    else 
        /* choose 16k */
        caps[4] &= 0x8f;

    getSbcCaps(caps, local_codec);
}


/**************************************************************************/
void selectOptimalSbcCapsSource(const uint8 *local_caps, uint8 *caps)
{
	/*
		Select SBC parameters for optimal performance. We now own the 
		malloc block containing the configure so clear the unwanted 
		fields and pass it back. Note that the library validates that 
		our capabilities are compatible hence there is also at least 
		one overlapping bit per field.
	*/

	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;
			
	/* Select sample rates based on what is supported at boths ends.
		At least one of 48k and 44.1k must be supported at both ends.*/
	if ((caps[4] & 0x10) && (local_codec[4] & 0x10))
        /* choose 48kHz */
        caps[4] &= 0x1f;
	else if ((caps[4] & 0x20) && (local_codec[4] & 0x20))
        /* choose 44kHz */
        caps[4] &= 0x2f;
    else if ((caps[4] & 0x40) && (local_codec[4] & 0x40))
        /* choose 32kHz */
        caps[4] &= 0x4f;
    else
        /* choose 16kHz */
        caps[4] &= 0x8f;
    
    getSbcCaps(caps, local_codec);
}


/*****************************************************************************/
uint8 a2dpSbcFormatFromConfig(const uint8 *config)
{
    uint8 sbc_format;

    /* sample frequency */
    if (config[4] & 0x80)
        sbc_format = 0 << SBC_HEADER_FREQUENCY_SHIFT;   
    else if (config[4] & 0x40)
        sbc_format = 1 << SBC_HEADER_FREQUENCY_SHIFT;
    else if (config[4] & 0x20)
        sbc_format = 2 << SBC_HEADER_FREQUENCY_SHIFT;
    else
        sbc_format = 3 << SBC_HEADER_FREQUENCY_SHIFT;

    /* channel mode */
    if (config[4] & 0x08)
        sbc_format |= ((uint8) a2dp_mono) << SBC_HEADER_CHANNEL_SHIFT;
    else if (config[4] & 0x04)
        sbc_format |= ((uint8) a2dp_dual_channel) << SBC_HEADER_CHANNEL_SHIFT;
    else if (config[4] & 0x02)
        sbc_format |= ((uint8) a2dp_stereo) << SBC_HEADER_CHANNEL_SHIFT;
    else
        sbc_format |= ((uint8) a2dp_joint_stereo) << SBC_HEADER_CHANNEL_SHIFT;

    /* block length */
    if (config[5] & 0x80)
        sbc_format |= 0 << SBC_HEADER_BLOCKS_SHIFT;
    else if (config[5] & 0x40)
        sbc_format |= 1 << SBC_HEADER_BLOCKS_SHIFT;
    else if (config[5] & 0x20)
        sbc_format |= 2 << SBC_HEADER_BLOCKS_SHIFT;
    else
        sbc_format |= 3 << SBC_HEADER_BLOCKS_SHIFT;

    /* sub bands */
    if (config[5] & 0x04)
        sbc_format |= SBC_HEADER_SUBBANDS_8;

    /* allocation method */
    if (config[5] & 0x02)
        sbc_format |= SBC_HEADER_ALLOCATION_SNR;

    return sbc_format;
}


/*****************************************************************************/
uint8 a2dpSbcSelectBitpoolAndPacketSize(uint8 sbc_header, uint32 rate, uint16 l2cap_mtu, uint16 *packet_size)
{
    sbc_parameters sbc;
    uint16 frame_length;
    uint16 frames;
    uint16 pdu;
    uint8 bitpool;

    /* Extract sbc_header into more usual structure */
    extractSbcParameters(&sbc, sbc_header);

    /*
        As the radio environment changes, the radio will switch between DH and 
        DM packets. In order to stream optimally, we should make sure our 
        L2CAP packet uses baseband packets efficiently with both DH and DM packets.
    */
    if (l2cap_mtu >= 668)
        /* 2xDH5 or 3xDM5 */
        pdu = 668;
    else if (l2cap_mtu >= 335)
        /* DH5 or DM5+DM3 */
        pdu = 335;
    else
        /* Out of spec: minimum A2DP MTU is 335. Try to carry on anyway. */
        pdu = l2cap_mtu;

    /* Return the total packet size */
    *packet_size = pdu;

    /*
       In order to use the radio bandwidth most efficiently we want to send full 
       L2CAP packets.  To achieve this we round down the frame length so that 
       an integer multiple of frames fills the PDU.
    */
    frame_length = frameLengthFromRate(&sbc, rate);

    /* Deduct RTP and SBC payload headers */
    pdu -= 13;

    /* Calculate how many frames fit - rounding up */
    frames = (pdu + frame_length-1) / frame_length;

    /* Adjust the frame length so it fills the packet */
    frame_length = pdu / frames;

    /* Get bitpool to achieve this frame length */
    bitpool = bitpoolFromFrameLength(&sbc, frame_length);

    return bitpool;
}
