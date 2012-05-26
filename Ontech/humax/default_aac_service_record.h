/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    default_aac_service_record.h
@brief Default aac SDP service record.
*/
#ifndef A2DP_DEFAULT_AAC_SINK_H_
#define A2DP_DEFAULT_AAC_SINK_H_



/*
  SDP Service Record used when AAC enabled
  Required to work round issue with Toshiba W44T phone
*/
static const uint8 a2dp__aac_sink_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSink(0x110b) */
    0x11,
    0x0b,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
    
  /* 48 bytes */
 

  0x09, /* Attribute: Provider name */
    0x01,
    0x02, 
  0x25,
  0x12, /* "AVExtension_Device"   - required to work with Toshiba W44T using AAC */
    0x41, 0x56, 0x45, 0x78, 0x74, 0x65, 0x6e, 0x73, 0x69, 
    0x6f, 0x6e, 0x5f, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65
}; /* 71 bytes */


#endif /* A2DP_DEFAULT_AAC_SINK_H_ */
