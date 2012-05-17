/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    USB audio support code,
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_usb_audio.h"

#include <boot.h>
#include <panic.h>
#include <sink.h>
#include <source.h>
#include <string.h>
#include <stdlib.h>
#include <usb.h>


#define SAMPLE_RATE                     ((uint32) 48000)

/* USB Audio class defines */
#define SET_CUR                         (0x01)
#define GET_CUR                         (0x81)
#define SAMPLING_FREQ_CONTROL           (0x01)


static UsbInterface s_ac, s_as_microphone, s_as_speaker;
static UsbCodes s_codes_ac = {0x01, 0x01, 0x00};  /* AUDIO, AUDIOCONTROL, PR_PROTOCOL_UNDEFINED */
static UsbCodes s_codes_as = {0x01, 0x02, 0x00};  /* AUDIO, AUDIOSTREAMING, PR_PROTOCOL_UNDEFINED */

static void audioControlHandler(Task task, MessageId id, Message message);
static void audioStreamingHandler(Task task, MessageId id, Message message);
static void handleAudioControlClassRequest(Source req);
static void handleAudioStreamingClassRequest(Source req);


/*
    USB Audio Descriptors

    Our descriptors declare the dongle as a microphone and speaker
    that support 16 bit mono PCM at 8khz.

    Terminal Architecture
    =====================

    Microphone         -> USB Output Terminal
    TerminalID 1          TerminalID 2

    USB Input Terminal -> Speaker
    TerminalID 3          TerminalID 4

    USB Streaming:
    Isochronous, mono, 16 bit, PCM input and output.

    Format of PCM stream:
    | Mono LSB | Mono MSB |
*/
static const uint8 s_audio_control_interface_descriptor[] =
{
    /* Class Specific Header */
    0x0A,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01, /* bcdADC = Audio Device Class v1.00 */
    0x0A + 0x0c + 0x09 + 0x0c + 0x09, /* wTotalLength LSB */
    0x00,         /* wTotalLength MSB */
    0x02,         /* bInCollection = 2 AudioStreaming interfaces */
    0x01,         /* baInterfaceNr(1) - AS#1 id */
    0x02,         /* baInterfaceNr(2) - AS#2 id */

    /* Microphone IT */
    0x0c,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = INPUT_TERMINAL */
    0x01,         /* bTerminalID */
    0x03, 0x02, /* wTerminalType = Personal Microphone */
    0x00,         /* bAssocTerminal = none */
    0x01,         /* bNrChannels = 1 */
    0x00, 0x00, /* wChannelConfig = mono */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */

    /* Microphone OT */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x03,         /* bDescriptorSubType = OUTPUT_TERMINAL */
    0x02,         /* bTerminalID */
    0x01, 0x01, /* wTerminalType = USB streaming */
    0x00,         /* bAssocTerminal = none */
    0x01,         /* bSourceID - Microphone IT */
    0x00,         /* iTerminal = same as USB product string */

    /* Speaker IT */
    0x0c,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = INPUT_TERMINAL */
    0x03,         /* bTerminalID */
    0x01, 0x01, /* wTerminalType = USB streaming */
    0x00,         /* bAssocTerminal = none */
    0x02,         /* bNrChannels = 2 */
    0x03, 0x00, /* wChannelConfig = left front and right front */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */

    /* Speaker OT */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x03,         /* bDescriptorSubType = OUTPUT_TERMINAL */
    0x04,         /* bTerminalID */
    0x02, 0x03, /* wTerminalType = Headphones */
    0x00,         /* bAssocTerminal = none */
    0x03,         /* bSourceID - Speaker IT*/
    0x00,         /* iTerminal = same as USB product string */
};


static const uint8 s_audio_streaming_interface_descriptor_speaker[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x03,         /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00, /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x02,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    SAMPLE_RATE & 0xff, /* tSampleFreq */
    (SAMPLE_RATE >> 8) & 0xff,
    (SAMPLE_RATE >> 16) & 0xff,

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x81,         /* bmAttributes = MaxPacketsOnly and SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};


static const uint8 s_audio_streaming_interface_descriptor_microphone[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x02,         /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00, /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,        /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    SAMPLE_RATE & 0xff, /* tSampleFreq */
    (SAMPLE_RATE >> 8) & 0xff,
    (SAMPLE_RATE >> 16) & 0xff,

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};


static const uint8 s_audio_endpoint_user_data[] =
{
    0, /* bRefresh */
    0  /* bSyncAddress */
};


/*  Streaming Isochronous Endpoint. Maximum packet size 192 (stereo at 48khz) */
static const EndPointInfo s_epinfo_as_speaker[] =
{
    {
        s_audio_endpoint_user_data,
        sizeof(s_audio_endpoint_user_data),
        end_point_iso_in,
        end_point_attr_iso,
        192,
        1,
        1
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size 96 (mono at 48khz) */
static const EndPointInfo s_epinfo_as_microphone[] =
{
    {
        s_audio_endpoint_user_data,
        sizeof(s_audio_endpoint_user_data),
        end_point_iso_out,
        end_point_attr_iso,
        96,
        1,
        1
    }
};


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    audioControlHandler

DESCRIPTION
    Handle message more data messages for the audio control interface.
    
*/
static void audioControlHandler(Task task, MessageId id, Message message)
{
    if (id == MESSAGE_MORE_DATA)
    {
        handleAudioControlClassRequest(StreamUsbClassSource(s_ac));
    }
}


/****************************************************************************
NAME
    audioStreamingHandler

DESCRIPTION
    Handle message more data messages for the audio streaming interface.
    
*/
static void audioStreamingHandler(Task task, MessageId id, Message message)
{
    if (id == MESSAGE_MORE_DATA)
    {
        handleAudioStreamingClassRequest(StreamUsbClassSource(s_as_speaker));
        handleAudioStreamingClassRequest(StreamUsbClassSource(s_as_microphone));
    }
}


/****************************************************************************
NAME
    handleAudioControlClassRequest

DESCRIPTION
    Process class requests to the Audio Control Interface.
    
*/
static void handleAudioControlClassRequest(Source req)
{
    uint16 packet_size;
    bool activity = FALSE;
    Sink resp = StreamSinkFromSource(req);

    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        /*
            Build the response. It must contain the original request,
            so copy from the source header.
        */
        UsbResponse usbresp;
        memcpy(&usbresp.original_request, SourceMapHeader(req), sizeof(UsbRequest));
        usbresp.data_length = 0;

        /* We advertise any commands on this endpoint, so reject */
        usbresp.success = FALSE;

        /* Sink packets can never be zero-length, so flush a dummy byte */
        (void)SinkClaim(resp, 1);
        (void)SinkFlushHeader(resp, 1, (void *)&usbresp, sizeof(UsbResponse));

        /* Discard the original request */
        SourceDrop(req, packet_size);

        activity = TRUE;
    }
}


/****************************************************************************
NAME
    handleAudioStreamingClassRequest

DESCRIPTION
    Process class requests to the Audio Streaming Interface or Endpoint.
    
*/
static void handleAudioStreamingClassRequest(Source req)
{
    uint16 packet_size;
    bool activity = FALSE;
    Sink resp = StreamSinkFromSource(req);

    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        /*
            Build the response. It must contain the original request,
            so copy from the source header.
        */
        UsbResponse usbresp;
        memcpy(&usbresp.original_request, SourceMapHeader(req), sizeof(UsbRequest));

        /* Set the response fields to default values to make the code below simpler */
        usbresp.success = TRUE;
        usbresp.data_length = 0;

        /* Endpoint only allows SET_/GET_ of sampling frequency */
        if (((usbresp.original_request.wValue >> 8) == SAMPLING_FREQ_CONTROL) &&
            (usbresp.original_request.wLength == 3))
        {
            if (usbresp.original_request.bRequest == SET_CUR)
            {
                uint32 new_rate = (uint32)SourceMap(req)[0] |
                                ((uint32)SourceMap(req)[1] << 8) |
                                ((uint32)SourceMap(req)[2] << 16);

                /* Reject bad value */
                if (new_rate != SAMPLE_RATE) usbresp.success = FALSE;
            }
            else if (usbresp.original_request.bRequest == GET_CUR)
            {
                /* Return current value */
                uint8 *ptr = SinkMap(resp) + SinkClaim(resp, 3);
                ptr[0] = (uint16)(SAMPLE_RATE & 0xff);
                ptr[1] = (uint16)(SAMPLE_RATE >> 8);
                ptr[2] = (uint16)(SAMPLE_RATE >> 16);    /*lint !e572 !e778 */
                usbresp.data_length = 3;
            }
            else usbresp.success = FALSE;
        }
        else
        {
            usbresp.success = FALSE;
        }

        /* Send response */
        if (usbresp.data_length)
        {
            (void)SinkFlushHeader(resp, usbresp.data_length, (void *)&usbresp, sizeof(UsbResponse));
        }
        else
        {
            /* Sink packets can never be zero-length, so flush a dummy byte */
            (void)SinkClaim(resp, 1);
            (void)SinkFlushHeader(resp, 1, (void *)&usbresp, sizeof(UsbResponse));
        }

        /* Discard the original request */
        SourceDrop(req, packet_size);

        activity = TRUE;
    }
}


/****************************************************************************
  MAIN FUNCTIONS
*/
      
/****************************************************************************
NAME
    usbAudioTimeCritical

DESCRIPTION
    Time critical USB initialisation.
    
*/
void usbAudioTimeCritical(void)
{
    /* Add an Audio Control Interface */
    s_ac = UsbAddInterface(&s_codes_ac, 0x24, s_audio_control_interface_descriptor,
                         sizeof(s_audio_control_interface_descriptor));
    if (s_ac == usb_interface_error) Panic();

    /* Add the speaker Audio Streaming Interface */
    s_as_speaker = UsbAddInterface(&s_codes_as, 0x24, s_audio_streaming_interface_descriptor_speaker,
                          sizeof(s_audio_streaming_interface_descriptor_speaker));
    
    if (s_as_speaker == usb_interface_error) Panic();

    /* Add the speaker endpoint */
    (void) PanicFalse(UsbAddEndPoints(s_as_speaker, 1, s_epinfo_as_speaker));

    /* Add the microphone Audio Streaming Interface */
    s_as_microphone = UsbAddInterface(&s_codes_as, 0x24, s_audio_streaming_interface_descriptor_microphone,
                          sizeof(s_audio_streaming_interface_descriptor_microphone));
    
    if (s_as_microphone == usb_interface_error) Panic();

    /* Add the microphone endpoint */
    (void) PanicFalse(UsbAddEndPoints(s_as_microphone, 1, s_epinfo_as_microphone));
}


/****************************************************************************
NAME
    usbAudioInit

DESCRIPTION
    Initialise USB support in the app.
    
*/
void usbAudioInit(void)
{
    /* Store the handler functions for the audio control and audio streaming */
    the_app->audio_control.handler = audioControlHandler;
    the_app->audio_streaming.handler = audioStreamingHandler;

    /*  Register our task for the sinks */
    (void) MessageSinkTask(StreamUsbClassSink(s_as_speaker), &the_app->audio_streaming);
    (void) MessageSinkTask(StreamUsbClassSink(s_as_microphone), &the_app->audio_streaming);
    (void) MessageSinkTask(StreamUsbClassSink(s_ac), &the_app->audio_control);
}

