/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    A2DP Stream End Point capabilities.
*/

/*!
@file    audioAdaptor_a2dp.h
@brief    Definition of A2DP Stream End Point Capabilities.
*/
#ifndef AUDIOADAPTOR_A2DP_H
#define AUDIOADAPTOR_A2DP_H

/*******************************************************************************************/
/*! @name SBC configuration bit fields 
*/
/*@{ */
/*! [Octet 0] Support for 16kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_16000        128
/*! [Octet 0] Support for 32kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_32000         64
/*! [Octet 0] Support for 44.1kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_44100         32
/*! [Octet 0] Support for 48kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_48000         16
/*! [Octet 0] Support for Mono channel mode */
#define SBC_CHANNEL_MODE_MONO            8
/*! [Octet 0] Support for Dualchannel mode */
#define SBC_CHANNEL_MODE_DUAL_CHAN       4
/*! [Octet 0] Support for Stereo channel mode */
#define SBC_CHANNEL_MODE_STEREO          2
/*! [Octet 0] Support for Joint Stereo channel mode */
#define SBC_CHANNEL_MODE_JOINT_STEREO    1

/*! [Octet 1] Support for a block length of 4 */
#define SBC_BLOCK_LENGTH_4             128
/*! [Octet 1] Support for a block length of 8 */
#define SBC_BLOCK_LENGTH_8              64
/*! [Octet 1] Support for a block length of 12 */
#define SBC_BLOCK_LENGTH_12             32
/*! [Octet 1] Support for a block length of 16 */
#define SBC_BLOCK_LENGTH_16             16
/*! [Octet 1] Support for 4 subbands */
#define SBC_SUBBANDS_4                   8
/*! [Octet 1] Support for 8 subbands */
#define SBC_SUBBANDS_8                   4
/*! [Octet 1] Support for SNR allocation */
#define SBC_ALLOCATION_SNR               2
/*! [Octet 1] Support for Loudness allocation */
#define SBC_ALLOCATION_LOUDNESS          1

/*! [Octet 2] Minimum bitpool supported */
#define SBC_BITPOOL_MIN                  2
/*! [Octet 2] Maximum bitpool supported */
#define SBC_BITPOOL_MAX                250
/*! [Octet 2] Maximum bitpool for Low quality */
#define SBC_BITPOOL_LOW_QUALITY         20
/*! [Octet 2] Maximum bitpool for Medium quality */
#define SBC_BITPOOL_MEDIUM_QUALITY      32
/*! [Octet 2] Maximum bitpool for Good quality */
#define SBC_BITPOOL_GOOD_QUALITY        40
/*! [Octet 2] Maximum bitpool for High quality */
#define SBC_BITPOOL_HIGH_QUALITY        50

/*@} */

/*! @name SBC Capabilities for USB mode
    SBC capabilites for USB mode that an application can pass to the A2DP library during initialisation.
*/
static const uint8 sbc_caps_source_usb[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_48000     | SBC_SAMPLING_FREQ_44100    | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_16000     |   
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,
};

/*! @name SBC Capabilities for Analog mode
    SBC Capabilities for Analog mode that an application can pass to the A2DP library during initialisation.
*/
#ifdef ENABLE_EXTERNAL_ADC

static const uint8 sbc_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_48000     | SBC_SAMPLING_FREQ_44100    | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_16000     | 
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,
};

#else /* ENABLE_EXTERNAL_ADC not defined */

static const uint8 sbc_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_44100     | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_16000    | 
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,
};

#endif /* ENABLE_EXTERNAL_ADC */


/*! @name FASTSTREAM configuration bit fields 
*/
/*! [Octet 0] CSR Vendor ID Octet 0 */
#define FASTSTREAM_VENDOR_ID0          0x0A
/*! [Octet 1] CSR Vendor ID Octet 1 */
#define FASTSTREAM_VENDOR_ID1          0x00
/*! [Octet 2] CSR Vendor ID Octet 2 */
#define FASTSTREAM_VENDOR_ID2          0x00
/*! [Octet 3] CSR Vendor ID Octet 3 */
#define FASTSTREAM_VENDOR_ID3          0x00
/*! [Octet 4] Fastream Codec ID Octet 0 */
#define FASTSTREAM_CODEC_ID0          0x01
/*! [Octet 5] Fastream Codec ID Octet 1 */
#define FASTSTREAM_CODEC_ID1          0x00
/*! [Octet 6] Support for music */
#define FASTSTREAM_MUSIC              0x01
/*! [Octet 6] Support for voice */
#define FASTSTREAM_VOICE              0x02
/*! [Octet 7] Support for 48.0kHz sampling frequency */
#define FASTSTREAM_MUSIC_SAMP_48000   0x01
/*! [Octet 7] Support for 44.1kHz sampling frequency */
#define FASTSTREAM_MUSIC_SAMP_44100   0x02
/*! [Octet 7] Support for 16kHz sampling frequency */
#define FASTSTREAM_VOICE_SAMP_16000   0x20

/*@} */


/*! @name Fastream Capabilities for USB mode
    Faststream capabilites that an application can pass to the A2DP library during initialisation.
*/
static const uint8 faststream_caps_source_usb[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC,
    FASTSTREAM_MUSIC_SAMP_48000 | FASTSTREAM_MUSIC_SAMP_44100,
};

/*! @name Fastream Capabilities supporting bi-directional audio for USB mode
    Faststream capabilites that an application can pass to the A2DP library during initialisation.
*/
static const uint8 faststream_caps_bidirection_source_usb[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC | FASTSTREAM_VOICE,
    FASTSTREAM_MUSIC_SAMP_48000 | FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000,
};

/*! @name Fastream Capabilities for Analogue mode
    Faststream capabilites that an application can pass to the A2DP library during initialisation.
*/
#ifdef ENABLE_EXTERNAL_ADC

static const uint8 faststream_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC,
    FASTSTREAM_MUSIC_SAMP_48000 | FASTSTREAM_MUSIC_SAMP_44100,
};

static const uint8 faststream_caps_bidirection_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC | FASTSTREAM_VOICE,
    FASTSTREAM_MUSIC_SAMP_48000 | FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000,
};

#else /* ENABLE_EXTERNAL_ADC not defined */

static const uint8 faststream_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC,
    FASTSTREAM_MUSIC_SAMP_44100,
};

static const uint8 faststream_caps_bidirection_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    10,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    FASTSTREAM_VENDOR_ID0,
    FASTSTREAM_VENDOR_ID1,
    FASTSTREAM_VENDOR_ID2,
    FASTSTREAM_VENDOR_ID3,
    FASTSTREAM_CODEC_ID0,
    FASTSTREAM_CODEC_ID1,
    FASTSTREAM_MUSIC | FASTSTREAM_VOICE,
    FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000,
};

#endif /* ENABLE_EXTERNAL_ADC */


/*! @name MP3 configuration bit fields 
*/
/*@{ */
/*! [Octet 0] Support for Layer I (mp1) */
#define MP3_LAYER_I                    128
/*! [Octet 0] Support for Layer II (mp2) */
#define MP3_LAYER_II                    64
/*! [Octet 0] Support for Layer III (mp3) */
#define MP3_LAYER_III                   32
/*! [Octet 0] Support for CRC Protection */
#define MP3_CRC_PROTECTION              16
/*! [Octet 0] Support for Mono channel mode */
#define MP3_CHANNEL_MODE_MONO            8
/*! [Octet 0] Support for Dual channel mode */
#define MP3_CHANNEL_MODE_DUAL_CHAN       4
/*! [Octet 0] Support for Stereo channel mode */
#define MP3_CHANNEL_MODE_STEREO          2
/*! [Octet 0] Support for Joint stereo channel mode */
#define MP3_CHANNEL_MODE_JOINT_STEREO    1

/*! [Octet 1] Support for 16kHz sampling frequency */
#define MP3_SAMPLING_FREQ_16000         32
/*! [Octet 1] Support for 22050Hz sampling frequency */
#define MP3_SAMPLING_FREQ_22050         16
/*! [Octet 1] Support for 24kHz sampling frequency */
#define MP3_SAMPLING_FREQ_24000          8
/*! [Octet 1] Support for 32kHz sampling frequency */
#define MP3_SAMPLING_FREQ_32000          4
/*! [Octet 1] Support for 44.1kHz sampling frequency */
#define MP3_SAMPLING_FREQ_44100          2
/*! [Octet 1] Support for 48kHz sampling frequency */
#define MP3_SAMPLING_FREQ_48000          1

/*! [Octet 2] Support for Variable bit rate */
#define MP3_VBR                        128
/*! [Octet 2] Support for bit rate 1110 */
#define MP3_BITRATE_VALUE_1110          64
/*! [Octet 2] Support for bit rate 1101 */
#define MP3_BITRATE_VALUE_1101          32
/*! [Octet 2] Support for bit rate 1100 */
#define MP3_BITRATE_VALUE_1100          16
/*! [Octet 2] Support for bit rate 1011 */
#define MP3_BITRATE_VALUE_1011           8
/*! [Octet 2] Support for bit rate 1010 */
#define MP3_BITRATE_VALUE_1010           4
/*! [Octet 2] Support for bit rate 1001 */
#define MP3_BITRATE_VALUE_1001           2
/*! [Octet 2] Support for bit rate 1000 */
#define MP3_BITRATE_VALUE_1000           1

/*! [Octet 3] Support for bit rate 0111 */
#define MP3_BITRATE_VALUE_0111         128
/*! [Octet 3] Support for bit rate 0110 */
#define MP3_BITRATE_VALUE_0110          64
/*! [Octet 3] Support for bit rate 0101 */
#define MP3_BITRATE_VALUE_0101          32
/*! [Octet 3] Support for bit rate 0100 */
#define MP3_BITRATE_VALUE_0100          16
/*! [Octet 3] Support for bit rate 0011 */
#define MP3_BITRATE_VALUE_0011           8
/*! [Octet 3] Support for bit rate 0010 */
#define MP3_BITRATE_VALUE_0010           4
/*! [Octet 3] Support for bit rate 0001 */
#define MP3_BITRATE_VALUE_0001           2
/*! [Octet 3] Support for bit rate 0000 */
#define MP3_BITRATE_VALUE_FREE           1

/*@} */

/*! @name MP3 Capabilities for USB mode
    MP3 capabilites that an application can pass to the A2DP library during initialisation.
*/
static const uint8 mp3_caps_source_usb[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,
    
    MP3_LAYER_III              | MP3_CRC_PROTECTION          |
    MP3_CHANNEL_MODE_MONO      | MP3_CHANNEL_MODE_DUAL_CHAN  | MP3_CHANNEL_MODE_STEREO     | MP3_CHANNEL_MODE_JOINT_STEREO,
    
    MP3_SAMPLING_FREQ_16000    | MP3_SAMPLING_FREQ_22050     | MP3_SAMPLING_FREQ_24000     | MP3_SAMPLING_FREQ_32000   |
    MP3_SAMPLING_FREQ_44100    | MP3_SAMPLING_FREQ_48000,
    
    MP3_VBR                    | MP3_BITRATE_VALUE_1110      | MP3_BITRATE_VALUE_1101      | MP3_BITRATE_VALUE_1100    |
    MP3_BITRATE_VALUE_1011     | MP3_BITRATE_VALUE_1010      | MP3_BITRATE_VALUE_1001      | MP3_BITRATE_VALUE_1000,
    
    MP3_BITRATE_VALUE_0111     | MP3_BITRATE_VALUE_0110      | MP3_BITRATE_VALUE_0101      | MP3_BITRATE_VALUE_0100    |
    MP3_BITRATE_VALUE_0011     | MP3_BITRATE_VALUE_0010      | MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */
    
};

/*! @name MP3 Capabilities for Analog source
    MP3 capabilites that an application can pass to the A2DP library during initialisation.
*/
#ifdef ENABLE_EXTERNAL_ADC

static const uint8 mp3_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,
    
    MP3_LAYER_III              | MP3_CRC_PROTECTION          |
    MP3_CHANNEL_MODE_MONO      | MP3_CHANNEL_MODE_DUAL_CHAN  | MP3_CHANNEL_MODE_STEREO     | MP3_CHANNEL_MODE_JOINT_STEREO,
    
    MP3_SAMPLING_FREQ_16000    | MP3_SAMPLING_FREQ_22050     | MP3_SAMPLING_FREQ_24000     | MP3_SAMPLING_FREQ_32000   |
    MP3_SAMPLING_FREQ_44100    | MP3_SAMPLING_FREQ_48000,
    
    MP3_VBR                    | MP3_BITRATE_VALUE_1110      | MP3_BITRATE_VALUE_1101      | MP3_BITRATE_VALUE_1100    |
    MP3_BITRATE_VALUE_1011     | MP3_BITRATE_VALUE_1010      | MP3_BITRATE_VALUE_1001      | MP3_BITRATE_VALUE_1000,
    
    MP3_BITRATE_VALUE_0111     | MP3_BITRATE_VALUE_0110      | MP3_BITRATE_VALUE_0101      | MP3_BITRATE_VALUE_0100    |
    MP3_BITRATE_VALUE_0011     | MP3_BITRATE_VALUE_0010      | MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */
    
};

#else /* ENABLE_EXTERNAL_ADC not defined */

static const uint8 mp3_caps_source_analogue[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,
    
    MP3_LAYER_III              | MP3_CRC_PROTECTION          |
    MP3_CHANNEL_MODE_MONO      | MP3_CHANNEL_MODE_DUAL_CHAN  | MP3_CHANNEL_MODE_STEREO     | MP3_CHANNEL_MODE_JOINT_STEREO,
    
    MP3_SAMPLING_FREQ_16000    | MP3_SAMPLING_FREQ_22050     | MP3_SAMPLING_FREQ_24000     | MP3_SAMPLING_FREQ_32000   |
    MP3_SAMPLING_FREQ_44100,
    
    MP3_VBR                    | MP3_BITRATE_VALUE_1110      | MP3_BITRATE_VALUE_1101      | MP3_BITRATE_VALUE_1100    |
    MP3_BITRATE_VALUE_1011     | MP3_BITRATE_VALUE_1010      | MP3_BITRATE_VALUE_1001      | MP3_BITRATE_VALUE_1000,
    
    MP3_BITRATE_VALUE_0111     | MP3_BITRATE_VALUE_0110      | MP3_BITRATE_VALUE_0101      | MP3_BITRATE_VALUE_0100    |
    MP3_BITRATE_VALUE_0011     | MP3_BITRATE_VALUE_0010      | MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */
    
};

#endif /* ENABLE_EXTERNAL_ADC */


/* Maximum supported audio quality for the media stream */
typedef enum
{
    A2DP_AUDIO_QUALITY_LOW,
    A2DP_AUDIO_QUALITY_MEDIUM,
    A2DP_AUDIO_QUALITY_GOOD,
    A2DP_AUDIO_QUALITY_HIGH,
    A2DP_AUDIO_QUALITY_UNKNOWN
} a2dpAudioQuality;


#endif /* AUDIOADAPTOR_A2DP_H */
