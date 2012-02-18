/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_csr_faststream.h
    
DESCRIPTION
	
*/

#ifndef A2DP_CSR_FASTSTREAM_H_
#define A2DP_CSR_FASTSTREAM_H_


#define FASTSTREAM_VENDOR_ID0		  0x0A
#define FASTSTREAM_VENDOR_ID1		  0x00
#define FASTSTREAM_VENDOR_ID2		  0x00
#define FASTSTREAM_VENDOR_ID3		  0x00
#define FASTSTREAM_CODEC_ID0		  0x01
#define FASTSTREAM_CODEC_ID1		  0x00
#define FASTSTREAM_MUSIC			  0x01
#define FASTSTREAM_VOICE			  0x02
#define FASTSTREAM_MUSIC_SAMP_48000   0x01
#define FASTSTREAM_MUSIC_SAMP_44100   0x02
#define FASTSTREAM_VOICE_SAMP_16000   0x20


/*************************************************************************
NAME    
    getCsrFastStreamConfigSettings
    
DESCRIPTION
    Get the sampling rates and channel mode from the codec config settings.
   
*/
void getCsrFastStreamConfigSettings(const uint8 *service_caps, a2dp_role_type role, uint32 *rate,
                                    a2dp_channel_mode *channel_mode,
                                    uint32 *voice_rate, uint8 *bitpool, uint8 *format);


/*************************************************************************
NAME    
    selectOptimalCsrFastStreamCapsSink
    
DESCRIPTION
    Selects the optimal configuration for FastStream playback by
    setting a single bit in each field of the pass caps structure.

*/
void selectOptimalCsrFastStreamCapsSink(const uint8 *local_caps, uint8 *remote_caps);


/*************************************************************************
NAME    
    selectOptimalCsrFastStreamCapsSource
    
DESCRIPTION
    Selects the optimal configuration for FastStream playback by
    setting a single bit in each field of the pass caps structure.

*/
void selectOptimalCsrFastStreamCapsSource(const uint8 *local_caps, uint8 *remote_caps);


#endif /* A2DP_CSR_FASTSTREAM_H_ */
