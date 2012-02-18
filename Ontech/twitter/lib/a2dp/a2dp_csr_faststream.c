/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_csr_faststream.c        

DESCRIPTION
	This file contains 

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"
#include "a2dp_csr_faststream.h"
#include "a2dp_caps_parse.h"


/**************************************************************************/
void selectOptimalCsrFastStreamCapsSink(const uint8 *local_caps, uint8 *remote_caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;

	/* Choose what is supported at both sides */
    remote_caps[10] = (remote_caps[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE)) & (local_codec[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE));

    /* Currently only support 48.0kHz, 44.1kHz for Music and 16kHz for Voice */
   	remote_caps[11] = (remote_caps[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000)) &
					  (local_codec[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000));
}


/**************************************************************************/
void selectOptimalCsrFastStreamCapsSource(const uint8 *local_caps, uint8 *remote_caps)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return;
    
   	/* Choose what is supported at both sides */
    remote_caps[10] = (remote_caps[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE)) & (local_codec[10] & (FASTSTREAM_MUSIC | FASTSTREAM_VOICE));

	/* Currently only support 48.0kHz, 44.1kHz for Music and 16kHz for Voice */
	remote_caps[11] = ((remote_caps[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000)) &
					   (local_codec[11] & (FASTSTREAM_MUSIC_SAMP_48000 |  FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000)) );

}


/**************************************************************************/
void getCsrFastStreamConfigSettings(const uint8 *service_caps, a2dp_role_type role, uint32 *rate,
                                    a2dp_channel_mode *channel_mode,
                                    uint32 *voice_rate, uint8 *bitpool, uint8 *format)
{
    if (service_caps[10] & FASTSTREAM_MUSIC)
    {
		if (service_caps[11] & FASTSTREAM_MUSIC_SAMP_48000 )
        {
            *rate = 48000;

            switch (role)
            {
                case a2dp_source:
                    /* Configure the SBC format:
                       48.0kHz, Blocks 16, Sub-bands 8, Joint Stereo, Loudness, Bitpool = 29
                       (data rate = 212kbps, packet size = 72*3+4 = 220 = DM5).
                       
                       Note the DSP rounds the 71byte frames to 72bytes.
                    */
				    *format = 0xfd;
                    *bitpool = 29;
                    break;

                case a2dp_sink:
                    break;
            }
        }
        else if (service_caps[11] & FASTSTREAM_MUSIC_SAMP_44100)
        {
            *rate = 44100;

            switch (role)
            {
                case a2dp_source:
                    /* Configure the SBC format:
                       44.1kHz, Blocks 16, Sub-bands 8, Joint Stereo, Loudness, Bitpool = 29
                       (data rate = 195kbps, packet size = 72*3+4 = 220 = DM5).
                       
                       Note the DSP rounds the 71byte frames to 72bytes.
                    */
                    *format = 0xbd;
                    *bitpool = 29;
                    break;

                case a2dp_sink:
                    break;
            }
        }
    }
    else
        *rate = 0;
    
    if (service_caps[10] & FASTSTREAM_VOICE)
    {
        if (service_caps[11] & FASTSTREAM_VOICE_SAMP_16000)
        {
            *voice_rate = 16000;

            switch (role)
            {
                case a2dp_sink:
                    /*
                      Configure the SBC format for the microphone data
                      16kHz, Mono, Blocks 16, Sub-bands 8, Loudness, Bitpool = 32
                      (data rate = 72kbps, packet size = 3*72 + 4 = 220 <= DM5).
                    */
                    *format = 0x31;
                    *bitpool = 32;
                    break;

                case a2dp_source:
                    break;
            }
        }
    }
    else
        *voice_rate = 0;
    
    *channel_mode = a2dp_joint_stereo;
}

