/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */

/* CSR WARNING: DON'T DO ANYTHING TOO CRAZY TO THIS FILE */
/* it has to be parsed by devHost/kalimba/Makefile as well as the C compiler */

/*!
   @file kalimba_standard_messages.h

   @brief
      The messages passed between the kalimba libraries and the VM
      application.

   @description
      Messages passed between MCU and kalimba consist of a 16-bit ID and up to
      four 16-bit arguments.

      If the top-bit is set (0x8000 - 0xFFFF) the message was sent by,
      or is routed to, the BlueCore firmware. Those messages are
      defined in kalimba_messsages.h, which is supplied as a kalimba
      include file but not as a VM include file (since no such
      messages will reach the VM code.)

      kalimba_standard_messages.h lists the standard messages which
      pass between kalimba and the VM application. Those are typically
      sent by the CSR supplied DSP libraries or sample VM
      applications. All messages used here are in the range 0x7000 -
      0x7FFF.  Any customer specific messages must use numbers less
      than this.

      In the descriptions of each message, the four arguments are
      referred to by number (0..3) after their name, for example
      period (1) means that the period parameter is the second of the
      four parameters,
*/

#ifndef KALIMBA_STANDARD_MESSAGES_INCLUDED
#define KALIMBA_STANDARD_MESSAGES_INCLUDED


/* ****************************************************************************
// ****************************************************************************
//                      KALIMBA <--> VM APPLICATION MESSAGES
// ****************************************************************************
// **************************************************************************** */


/*!
  MESSAGE:  Go     (VM APPLICATION --> KALIMBA)

  @brief
   Indicates that the VM application has done all of its streamconnects
   and hence the dsp can start reading/writing data to its ports.
*/
#define   KALIMBA_MSG_GO                         0x7000

/*!
  MESSAGE:  SBC decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_SBCDEC_CONVERT_TO_MONO     0x7010

/*!
  MESSAGE:  SBC encoder set bitpool     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc encoder what bitpool value to encode using and hence
   the dsp can start reading/writing data to its ports.

  @param bitpool (0), the bitpool to use (valid range is 2..250.)
*/
#define   KALIMBA_MSG_SBCENC_SET_BITPOOL         0x7020

/*!
  MESSAGE:  SBC encoder set parameters     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc encoder what encoder parameters to use.

  @param param (0)
       the format is as follows (see SBC spec)
                  bit 6-7:  sampling frequency,
                  bit 4-5:  blocks,
                  bit 2-3:  channel_mode,
                  bit 1:    allocation method,
                  bit 0:    subbands
*/
#define   KALIMBA_MSG_SBCENC_SET_PARAMS          0x7021

/*!
  MESSAGE:  MP3 decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the mp3 decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_MP3DEC_CONVERT_TO_MONO     0x7030

/*!
  MESSAGE:  AAC decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the aac decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_AACDEC_CONVERT_TO_MONO     0x7040


/*!
  MESSAGE:  Clock mismatch rate on Source device     (KALIMBA --> VM APPLICATION)

  @brief
   Tells the VM the clock mismatch rate for a Source device so an application can store the rate per device.

  @param(0), clock mismatch rate
*/
#define KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE      0x7051


/*!
  MESSAGE:  Pulse LED control    (VM APPLICATION --> KALIMBA)

  @brief
   Tells the pulse_led libary which PIO to pulse and at what rate

  @param mask (0), the PIO mask to use (that is, which PIO lines to modify)
  @param period (1), the period of the pulsing (in units of 0.35 secs)
*/
#define   KALIMBA_MSG_PULSE_LED                  0x7100


/*!
  MESSAGE:  Play new dtmf tone                    (VM APPLICATION --> KALIMBA)

  @brief
    Tells the dtmf library to play a new dtmf tone.

  @param(0), tone to play
  @param(1), duration of tone in samples
  @param(2), duration of silence following the tone in ms
*/
#define    KALIMBA_MSG_DTMF_NEW_TONE_MESSAGE_ID   0x7150


/*!
  MESSAGE:  Silence and clip detect initialise channel
                                                 (VM APPLICATION --> KALIMBA)

  @brief
    Tells the silence and clip detect operator to initalise an instance
    (structure) with silence and clipping limits.

  @param(0), instance number
  @param(1), duration of silence in seconds  - period of silence before sending message
  @param(2), clip limit                      - signal level required to indicate clipping
*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_INITIALISE_ID        0x7200



/*!
  MESSAGE:  Silence and clip detect clipping detected
                                                 (KALIMBA --> VM APPLICATION)

  @brief
    Message to the VM to indicate clipping has been detected

*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_CLIP_DETECTED_ID     0x7201


/*!
  MESSAGE:  Silence and clip detect silence detected
                                                 (KALIMBA --> VM APPLICATION)

  @brief
    Message to the VM to indicate silence has been detected

*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_SILENCE_DETECTED_ID  0x7202


/*!
  MESSAGE:  Graphix driver control
                                                 (VM APPLICATION --> KALIMBA)

  @brief
    Tells the DSP graphic driver library to do something.

*/
#define   KALIMBA_MSG_GRAPHIX_DRIVER_FROM_VM_MSG_ID                  0x7400


/*!
  MESSAGE: Report status of SD card music player.
                                                 (KALIMBA --> VM APPLICATION)

  @brief Reports the sd card library version, card status and FAT type.
    
  @param(0), version number     - 16-bit LSW 0xD1-D2-D3-D4: version D1.D2.D3D4
  @param(1), card status        - 0 for OK, otherwise error
  @param(2), FAT type           - 0 for FAT32, 1 for FAT16, otherwise unknown

*/
#define KALIMBA_MSG_SD_CARD_STATUS                                  0x7500

/*!
  MESSAGE: Request an SD card music player operation.
                                                 (VM APPLICATION --> KALIMBA)

  @brief Request the SD music player perform a file operation: find/play/stop/pause/seek

  Takes a series of sub-messages with varying parameters depending upon type.
  The sub-message type is always stored in param(0)

  SUBMESSAGE FIND_FILE_REQUEST
    Request the SD music player finds file <track number>
  @param(0), FIND_FILE_REQUEST  - 0
  @param(1), track number       - 0..65535
    
  SUBMESSAGE PLAY_REQUEST
    Request the SD music player plays file <track number>
  @param(0), PLAY_REQUEST       - 1 
  @param(1), track number       - 0..65535 

  SUBMESSAGE STOP_REQUEST
    Request the SD music player stops playing file <track number>
  @param(0), STOP_REQUEST       - 2 
  @param(1), track number       - 0..65535 

  SUBMESSAGE PAUSE_REQUEST
    Request the SD music player pauses playback of file <track number>
  @param(0), PAUSE_REQUEST      - 3
  @param(1), track number       - 0..65535 

  SUBMESSAGE SCAN_REQUEST
    Request the SD music player FFW or REW file <track number>
  @param(0), SCAN_REQUEST       - 4
  @param(1), track number       - 0..65535 
  @param(2), nav step size      - -16..16 +ve:FFW -ve:REW

*/
#define KALIMBA_MSG_SD_PLAY_REQUEST                                 0x7501

/*!
  MESSAGE: Response to VM request for SD music player operation.
                                                 (KALIMBA --> VM APPLICATION)

  @brief Response from Kalimba to the requested file operation.
    
  This is a LONG message, with payload length as the first parameter and the
  message payload as the second parameter. The first word of the 2nd parameter
  contains a submessage type.

  @param(0), payload length
  @param(1), message payload

  The payload is as follows:-

  SUBMESSAGE FILE_NOT_FOUND
    Failure response to FIND_FILE_REQUEST
  @param(1), FILE_NOT_FOUND     - 0
  @param(2), track number       - 0..65535 number of track not found
  @param(3), reserved

  SUBMESSAGE FILE_FOUND
    Success response to FIND_FILE_REQUEST
  @param(1), FILE_FOUND         - 1
  @param(2), track number       - 0..65535
  @param(3), reserved
  Remaining payload (payload length-3) is the track filename

  SUBMESSAGE PLAYING_FILE
    Success response to PLAY_REQUEST
  @param(1), PLAYING_FILE       - 2
  @param(2), track number       - 0..65535
  @param(3), playback time      - current playback time 

  SUBMESSAGE END_OF_FILE
    SD music player has finished playing the current track
  @param(1), END_OF_FILE        - 3
  @param(2), track number       - 0..65535
  @param(3), playback time      - time at EOF

  SUBMESSAGE PLAYING_STOPPED
    Success response to STOP_REQUEST
  @param(1), PLAYING_STOPPED    - 4
  @param(2), track number       - 0..65535
  @param(3), playback time      - time at stop point

  SUBMESSAGE PLAYING_PAUSED
    Success response to PAUSE_REQUEST
  @param(1), PLAYING_PAUSED     - 5
  @param(2), track number       - 0..65535
  @param(3), playback time      - time at paused point

  SUBMESSAGE SCANNING
    Success response to SCAN_REQUEST
  @param(1), SCANNING           - 6
  @param(2), track number       - 0..65535
  @param(3), playback time      - time just scanned to 

*/
#define KALIMBA_MSG_SD_PLAY_RESPONSE                                0x7502

/*!
  MESSAGE: Load a codec overlay into Kalimba.
                                                 (KALIMBA --> VM APPLICATION)

  @brief Tell SD music player to load a codec as an overlay.

  Placeholder - this message not yet implemented.
    
*/
#define KALIMBA_MSG_SD_LOAD_CODEC_OVERLAY                           0x7503

#endif
