/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    HID support code.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_configure.h"
#include "audioAdaptor_usb_hid.h"

#include <panic.h>
#include <sink.h>
#include <source.h>
#include <string.h>
#include <stdlib.h>
#include <usb.h>

#define ENABLE_KEYBD
#define ENABLE_CCD

#define HID_DESCRIPTOR_COUNT (1)
#define HID_DESCRIPTOR_LENGTH           (6+(HID_DESCRIPTOR_COUNT*3))
#define HID_DESCRIPTOR                  (0x21)

#define HID_KEYBD_REPORT_DESCRIPTOR           (0x22)
#define HID_KEYBD_REPORT_DESCRIPTOR_LENGTH    (44)

#define HID_CCD_REPORT_DESCRIPTOR           (0x22)
#define HID_CCD_REPORT_DESCRIPTOR_LENGTH    (47)

#define KEYBD_END_POINT_OUT end_point_int_out
#define CCD_END_POINT_OUT   end_point_int_out2

static UsbInterface                     s_hid_keybd, s_hid_ccd;
static UsbCodes                         s_codes_hid = {0x03, 0x00, 0x00};

/* USB HID class descriptor */
static const uint8 s_hid_keybd_descriptor[HID_DESCRIPTOR_LENGTH] =
{
    HID_DESCRIPTOR_LENGTH,              /* bLength */
    HID_DESCRIPTOR,                     /* bDescriptorType */
    0x11, 0x01,                         /* bcdHID */
    0,                                  /* bCountryCode */
    1,                                  /* bNumDescriptors */
    HID_KEYBD_REPORT_DESCRIPTOR,        /* bDescriptorType */
    HID_KEYBD_REPORT_DESCRIPTOR_LENGTH, /* wDescriptorLength */
    0                                   /* wDescriptorLength */
};


/* HID Report Descriptor - Keyboard with vendor specific extensions */
static const uint8 s_hid_keybd_report_descriptor[HID_KEYBD_REPORT_DESCRIPTOR_LENGTH] =
{
    0x05, 0x01,                    /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06,                    /* USAGE (Keyboard) */
    0xa1, 0x01,                    /* COLLECTION (Application) */
    0x05, 0x07,                    /*   USAGE_PAGE (Keyboard) */

    /* 8 bits - Modifiers (alt, shift etc) */
    0x19, 0xe0,                    /*   USAGE_MINIMUM (Keyboard LeftControl) */
    0x29, 0xe7,                    /*   USAGE_MAXIMUM (Keyboard Right GUI) */
    0x15, 0x00,                    /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,                    /*   LOGICAL_MAXIMUM (1) */
    0x75, 0x01,                    /*   REPORT_SIZE (1) */
    0x95, 0x08,                    /*   REPORT_COUNT (8) */
    0x81, 0x02,                    /*   INPUT (Data,Var,Abs) */

    /* 8 bits - Reserved byte */
    0x75, 0x08,                    /*   REPORT_SIZE (8) */
    0x95, 0x01,                    /*   REPORT_COUNT (1) */
    0x81, 0x01,                    /*   INPUT (Const) */

    /* 6 x 8 bits - Key states */
    0x19, 0x00,                    /*   USAGE_MINIMUM (Reserved (no event indicated)) */
    0x29, 0x91,                    /*   USAGE_MAXIMUM (Keyboard Application) */
    0x15, 0x00,                    /*   LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,              /*   LOGICAL_MAXIMUM (255) */
    0x75, 0x08,                    /*   REPORT_SIZE */
    0x95, 0x06,                    /*   REPORT_COUNT */
    0x81, 0x00,                    /*   INPUT (Data,Ary,Abs) */
    
    0xc0                           /* END_COLLECTION */
};


/* USB HID endpoint information */
static const EndPointInfo s_end_point_info_hid_keybd[] =
{
    {
        NULL,
        0,
        KEYBD_END_POINT_OUT,
        end_point_attr_int,
        16,
        1,
        1
    }
};


/* USB HID class descriptor - Consumer Control Device*/
static const uint8 s_hid_ccd_descriptor[HID_DESCRIPTOR_LENGTH] =
{
    HID_DESCRIPTOR_LENGTH,              /* bLength */
    HID_DESCRIPTOR,                     /* bDescriptorType */
    0x11, 0x01,                         /* bcdHID */
    0,                                  /* bCountryCode */
    1,                                  /* bNumDescriptors */
    HID_CCD_REPORT_DESCRIPTOR,          /* bDescriptorType */
    HID_CCD_REPORT_DESCRIPTOR_LENGTH,   /* wDescriptorLength */
    0                                   /* wDescriptorLength */
};


/* HID Report Descriptor - Consumer Control Device */
static const uint8 s_hid_ccd_report_descriptor[HID_CCD_REPORT_DESCRIPTOR_LENGTH] = 
{
    0x05, 0x0C,                  /* USAGE_PAGE (Consumer Devices) */
    0x09, 0x01,                  /* USAGE (Consumer Control) */
    0xa1, 0x01,                  /* COLLECTION (Application) */
    
    0x85, 0x01,                  /*   REPORT_ID (1) */
    
    0x15, 0x00,                  /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,                  /*   LOGICAL_MAXIMUM (1) */
    0x09, 0xcd,                  /*   USAGE (Play/Pause - OSC) */
    0x09, 0xb5,                  /*   USAGE (Next Track - OSC) */
    0x09, 0xb6,                  /*   USAGE (Previous Track - OSC) */
    0x09, 0xb7,                  /*   USAGE (Stop - OSC) */
    0x75, 0x01,                  /*   REPORT_SIZE (1) */
    0x95, 0x04,                  /*   REPORT_COUNT (4) */
    0x81, 0x02,                  /*   INPUT (Data,Var,Abs) */
    
    0x15, 0x00,                  /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,                  /*   LOGICAL_MAXIMUM (1) */
    0x09, 0xb0,                  /*   USAGE (Play - OOC) */
    0x09, 0xb1,                  /*   USAGE (Pause - OOC) */
    0x75, 0x01,                  /*   REPORT_SIZE (1) */
    0x95, 0x02,                  /*   REPORT_COUNT (2) */
    0x81, 0x22,                  /*   INPUT (Data,Var,Abs,NoPref) */
    
    0x75, 0x01,                  /*   REPORT_SIZE (1) */
    0x95, 0x02,                  /*   REPORT_COUNT (2) */
    0x81, 0x01,                  /*   INPUT (Const) */
    
    0xc0                        /* END_COLLECTION */
};


/* USB HID endpoint information */
static const EndPointInfo s_end_point_info_hid_ccd[] =
{
    {
        NULL,
        0,
        CCD_END_POINT_OUT,
        end_point_attr_int,
        16,
        1,
        1
    }
};


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    claimSink

DESCRIPTION
    Grab space in sink and clear.
    
*/
#if defined ENABLE_KEYBD || defined ENABLE_CCD
static uint8* claimSink (Sink sink, uint16 size)
{
    uint8 *ptr;
    
    if ( SinkSlack(sink)>=size )
    {
        (void)SinkClaim(sink, size);
        ptr = SinkMap(sink);
        memset(ptr, 0, size);
    
        return ptr;
    }
    
    return 0;
}
#endif


#define MM_KEYCODE           0x8000
#define MM_BUTTON_MASK       0x00FF
#define MM_BUTTON_SHIFT      0
#define MM_ON_OFF_CONTROL    0x1000
#define KEYBD_HOLD_MODIFIER  0x2000
#define KEYBD_MODIFIER_MASK  0x0F00
#define KEYBD_MODIFIER_SHIFT 8
#define KEYBD_BUTTON_MASK    0x00FF
#define KEYBD_BUTTON_SHIFT   0


/****************************************************************************
NAME
    sendHidKeycode

DESCRIPTION
    Presses and releases the specified HID button to simulate a key press.
    
*/
static bool sendHidKeycode(uint16 keycode, bool state)
{
    if ( keycode & MM_KEYCODE )
    {   /* HID Multimedia keycode */
#ifdef ENABLE_CCD    
        uint8 *ptr;
        uint8 key = (uint8)((keycode & MM_BUTTON_MASK) >> MM_BUTTON_SHIFT);
        
        /* Get the sink */
        Sink sink = StreamUsbEndPointSink(CCD_END_POINT_OUT);   
    
        DEBUG_HID((" mm:%x",keycode & 0x7FFF));
        
        if ( keycode & MM_ON_OFF_CONTROL )
        {    /* Send an On/Off Control key code */
            if ( state )
            {    /* Button down - copy data into sink and flush it */
                if ( (ptr = claimSink(sink, 2))!=0 )
                {
                    ptr[0] = 1;  /* Report Id 1 */
                    ptr[1] = key;
                    (void) SinkFlush(sink, 2);
                }
                
                return ptr!=0;
            }
            else
            {    /* Button up - - copy data into sink and flush it */
                if ( (ptr = claimSink(sink, 2))!=0 )
                {
                    ptr[0] = 1;  /* Report Id 1 */
                    ptr[1] = 0;
                    (void) SinkFlush(sink, 2);
                }
                
                return ptr!=0;
            }
        }
        else
        {    /* Send a One Shot Control key code */
            if ( state )
            {   
                if ( (ptr = claimSink(sink, 4))!=0 )
                {
                    /* Button down data */
                    ptr[0] = 1;  /* Report Id 1 */
                    ptr[1] = key;
                    
                    /* Button up data */
                    ptr[2] = 1;  /* Report Id 1 */
                    ptr[3] = 0;
                
                    /* Flush button down data */    
                    (void) SinkFlush(sink, 2);
                    
                    /* Flush button up data */    
                    (void) SinkFlush(sink, 2);
                }
                
                   return ptr!=0;
            }
            else
            {   /* Button up - copy data into sink and flush it */
                /* Do nothing for button up - OSC mechanism means it's already happened */
                return TRUE;
            }
        }
#else /* ENABLE_CCD not defined */
        keycode = keycode;
#endif /* ENABLE_CCD */   
    }
    else
    {   /* HID Keyboard keycode */
#ifdef ENABLE_KEYBD    
        static uint8 held_modifier;
        uint8 *ptr;
        uint8 modifier = (uint8)((keycode & KEYBD_MODIFIER_MASK) >> KEYBD_MODIFIER_SHIFT);
        uint8 key = (uint8)((keycode & KEYBD_BUTTON_MASK) >> KEYBD_BUTTON_SHIFT);
        
        /* Get the sink */
        Sink sink = StreamUsbEndPointSink(KEYBD_END_POINT_OUT);
    
        DEBUG_HID((" kbd:%x,%x\n",modifier,key));
        
        if ( keycode & KEYBD_HOLD_MODIFIER )
        {    /* Keycode indicates that modifier is to be held */
            held_modifier |= modifier;
        }
        else
        {    /* Release specified modifier */
            held_modifier &= ~modifier;
        }
        
        if ( (ptr = claimSink(sink, 16))!=0 )
        {
            /* Button down - copy data into sink and flush it */
            /* Report Id 0 does not need specifying */
            ptr[0] = modifier | held_modifier;
            ptr[2] = key;
            (void) SinkFlush(sink, 8);
        
            /* Button up - copy data into sink and flush it */
            /* Report Id 0 does not need specifying */
            ptr[0] = held_modifier;
            (void) SinkFlush(sink, 8);
        }
        
        return ptr!=0;
#else /* ENABLE_KEYBD not defined */
        keycode = keycode;
#endif /* ENABLE_KEYBD */
    }
    
    return FALSE;
}


#if defined USE_HID_TELEPHONY    
/****************************************************************************
NAME
    sendTelephonyHidReport

DESCRIPTION
    Issues a HID Telephony report over USB.
    
*/
static void sendTelephonyHidReport(mvdHidCommand hid_cmd)
{
    uint8 *ptr;
    
    /* Get the sink */
    Sink sink = StreamUsbEndPointSink(KEYBD_END_POINT_OUT);

    DEBUG_HID((" tel:%x\n",(uint16)hid_cmd));
    
    switch (hid_cmd)
    {
        case HidCmdAnswer:
        {
            ptr = claimSink(sink, 8); 
            /* TODO: Construct report to answer incoming call */
            (void)SinkFlush(sink, 8);  /* Send HID report */
            break;
        }
        case HidCmdHangUp:
        {
            ptr = claimSink(sink, 8); 
            /* TODO: Construct report to terminate existing call */ 
            (void)SinkFlush(sink, 8);  /* Send HID report */
            break;
        }
        case HidCmdReject:
        {
            ptr = claimSink(sink, 8); 
            /* TODO: Construct report to reject incoming call */ 
            (void)SinkFlush(sink, 8);  /* Send HID report */
            break;
        }
        case HidCmdCancel:
        {
            ptr = claimSink(sink, 8); 
            /* TODO: Construct report to cancel outgoing call */ 
            (void)SinkFlush(sink, 8);  /* Send HID report */
            break;
        }
        default:
        {
            /* Ignore all other commands */
            break;
        }
    }
}
#endif /* USE_HID_TELEPHONY */


#define MAX_ITERATIONS 4
#define MAX_REPEATS 4
static struct
{
    unsigned int idx:8;
    unsigned int count:8;
} iterations[MAX_ITERATIONS];

static struct
{
    unsigned int idx:8;
    unsigned int count:8;
    unsigned int rep:8;
    unsigned int inc:8;
} repeats[MAX_REPEATS];


/****************************************************************************
NAME
    flushIterations

DESCRIPTION
    Clear all iterations array entries.
    
*/
static void flushIterations (void)
{
    uint16 i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        iterations[i].idx = 0xff;
    }
}


/****************************************************************************
NAME
    flushRepeats

DESCRIPTION
    Clear all repeats array entries.
    
*/
static void flushRepeats (void)
{
    uint16 i;
    for (i = 0; i < MAX_REPEATS; i++)
    {
        repeats[i].idx = 0xff;
    }
}


/****************************************************************************
NAME
    pushIteration

DESCRIPTION
    Store the passed values into the first free position in the iterations array.
    
*/
static bool pushIteration (uint8 idx, uint8 count)
{
    uint8 i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        if (iterations[i].idx == 0xff)
        {
            DEBUG_HID((" push:i=%u \n",i));
            iterations[i].idx = idx;
            iterations[i].count = count;
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    popIteration

DESCRIPTION
    Remove the last valid entry from the iterations array.
    
*/
static bool popIteration (uint8* idx, uint8* count)
{
    uint8 i = MAX_ITERATIONS;
    while (i)
    {
        if (iterations[--i].idx != 0xff)
        {
            DEBUG_HID((" pop:i=%u \n",i));
            *idx = iterations[i].idx;
            *count = iterations[i].count;
            iterations[i].idx = 0xff;
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    addRepeat

DESCRIPTION
    Store the passed values into the first free position in the repeats array.
    
*/
static bool addRepeat (uint8 idx, uint8 count, uint8 rep, uint8 inc)
{
    uint8 i;
    for (i = 0; i < MAX_REPEATS; i++)
    {
        if (repeats[i].idx == 0xff)
        {
            DEBUG_HID((" add:i=%u \n",i));
            repeats[i].idx = idx;
            repeats[i].count = count;
            repeats[i].rep = rep;
            repeats[i].inc = inc;
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    getRepeat

DESCRIPTION
    Remove the last valid entry from the repeats array.
    
*/
static bool getRepeat (uint8 curr_idx, uint8* idx, uint8* count, uint8* rep, uint8* inc)
{
    uint8 i = MAX_REPEATS;
    while (i)
    {
        if (repeats[--i].idx <= curr_idx)
        {
            DEBUG_HID((" get:i=%u \n",i));
            *idx = repeats[i].idx;
            *count = repeats[i].count;
            *rep = repeats[i].rep;
            *inc = repeats[i].inc;
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    updateRepeat

DESCRIPTION
    Update the values of the specified entry in the repeats array.
    
*/
static bool updateRepeat (uint8 idx, uint8 count, uint8 rep, uint8 inc)
{
    uint8 i;
    for (i = 0; i < MAX_REPEATS; i++)
    {
        if (repeats[i].idx == idx)
        {
            DEBUG_HID((" updt:i=%u \n",i));
            repeats[i].count = count;
            repeats[i].rep = rep;
            repeats[i].inc = inc;
            return TRUE;
        }
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    findRepeat

DESCRIPTION
    Find the specified entry in the repeats array.
    
*/
static bool findRepeat (uint8 idx)
{
    uint8 i;
    for (i = 0; i < MAX_REPEATS; i++)
    {
        if (repeats[i].idx == idx)
        {
            DEBUG_HID((" find:i=%u \n",i));
            return TRUE;
        }
    }
    
    return FALSE;
}


/*
    Top bit defines HID command type:
    0x8000 implies HID MM(set)/HID Keybd(clear)
    
    For a HID Keybd command:
    0x4000 implies HID Sequence(set)/HID Keycode(clear)
    
    For a HID Sequence:
    0x2000 implies Repeat(set)/Iterate(clear) [if HID Sequence bit set]
    0x1000 implies End(set)/Start(clear) [if HID Sequence bit set]
    
    For a HID Keycode:
    0x2000 implies Press and Hold(set)/Press and Release(clear)
*/
#define ITERATE_START 0x4000    /* HID Sequence, Iterate Start */
#define ITERATE_END   0x5000    /* HID Sequence, Iterate End   */
#define REPEAT_START  0x6000    /* HID Sequence, Repeat Start  */
#define REPEAT_END    0x7000    /* HID Sequence, Repeat End    */

/****************************************************************************
NAME
    sendHidSequence

DESCRIPTION
    Send the HID sequence.
    
*/
static void sendHidSequence (void)
{
    static uint8 seq_idx;
    uint16 keycode;
    uint8 count;
    uint8 idx;
    uint8 rep;
    uint8 inc;
    bool exit = FALSE;
        
    if ( seq_idx == 0 )
    {
        flushIterations();
        flushRepeats();
    }
    
    while ( !exit && configureGetHidSequenceKeycode(seq_idx++, &keycode) )
    {
        switch (keycode & 0xf000)
        {
            case ITERATE_START:
            {
                DEBUG_HID(("  [%u]ITERATE_START\n", seq_idx));
                count = keycode & 0xff;
                pushIteration(seq_idx, count);
                DEBUG_HID((" idx:%u cnt:%u\n", seq_idx, count));
                break;
            }
            case ITERATE_END:
            {
                DEBUG_HID(("  [%u]ITERATE_END\n", seq_idx));
                popIteration(&idx, &count);
                if (count>0)
                {
                    seq_idx = idx;
                    pushIteration(idx, --count);
                    DEBUG_HID((" idx:%u cnt:%u\n", seq_idx, count));
                    MessageSend(&the_app->task, APP_HID_SEQUENCE, 0);
                    exit = TRUE;
                }
                break;
            }
            case REPEAT_START:
            {
                DEBUG_HID(("  [%u]REPEAT_START\n", seq_idx));
                if ( !findRepeat(seq_idx) )
                {
                    count = rep = keycode & 0xff;
                    inc = (keycode>>8) & 0xf;
                    addRepeat(seq_idx, count, rep, inc);
                    DEBUG_HID((" idx:%u cnt:%u rep:%u inc:%u\n", seq_idx, count, rep, inc));
                }
                break;
            }
            case REPEAT_END:
            {
                DEBUG_HID(("  [%u]REPEAT_END\n", seq_idx));
                if ( getRepeat(seq_idx, &idx, &count, &rep, &inc) )
                {
                    if (count>0)
                    {
                        seq_idx = idx;
                        updateRepeat(seq_idx, --count, rep, inc);
                        DEBUG_HID((" idx:%u cnt:%u rep:%u inc:%u\n", idx, count, rep, inc));
                    }
                    else
                    {
                        rep += inc;
                        count = rep;
                        updateRepeat(idx, count, rep, inc);
                        DEBUG_HID((" idx:%u cnt:%u rep:%u inc:%u", idx, count, rep, inc));
                    }
                }
                break;
            }
            default:
            {
                DEBUG_HID(("  [%u]  keycode\n", seq_idx));

                if ( !sendHidKeycode(keycode, TRUE) )
                {   /* Failed to send HID command */
                    DEBUG_HID((" FAILED \n"));
                    seq_idx--;
                }
                
                MessageSend(&the_app->task, APP_HID_SEQUENCE, 0);
                exit = TRUE;
                break;            
            }
        }
    }
    
    if (!exit)
    {   /* Exiting while statement because end of sequence reached */
        seq_idx = 0;
    }
}


/****************************************************************************
NAME
    actionHidCommand

DESCRIPTION
    Act on the HID command specified.
    
*/
static void actionHidCommand (mvdHidCommand command)
{
    switch (command)
    {
        case HidCmdNop:
        {
            DEBUG_HID(("Nop"));
            break;
        }    
        case HidCmdAnswer:
        {
            DEBUG_HID(("Answer"));
    #if defined USE_HID_TELEPHONY
            sendTelephonyHidReport(HidCmdAnswer);
    #else        
            sendHidKeycode(the_app->hid_config.answer | KEYBD_HOLD_MODIFIER, TRUE);    /* Leave any modifier held down */
            sendHidKeycode(the_app->hid_config.answer & KEYBD_MODIFIER_MASK, TRUE);    /* Now release it */
    #endif        
            break;
        }
        case HidCmdHangUp:
        {
            DEBUG_HID(("HangUp"));
    #if defined USE_HID_TELEPHONY
            sendTelephonyHidReport(HidCmdHangup);
    #else        
            sendHidKeycode(the_app->hid_config.hangup | KEYBD_HOLD_MODIFIER, TRUE);    /* Leave any modifier held down */
            sendHidKeycode(the_app->hid_config.hangup & KEYBD_MODIFIER_MASK, TRUE);    /* Now release it */
    #endif        
            break;
        }
        case HidCmdPlay:
        {
            DEBUG_HID(("Play"));
            sendHidKeycode(the_app->hid_config.play, TRUE);
            break;
        }
        case HidCmdPause:
        {
            DEBUG_HID(("Pause"));
            sendHidKeycode(the_app->hid_config.pause, TRUE);
            break;
        }
        case HidCmdPlayPause:
        {
            DEBUG_HID(("PlayPause"));
            sendHidKeycode(the_app->hid_config.playpause, TRUE);
            break;
        }
        case HidCmdResume:
        {
            DEBUG_HID(("Resume"));
            sendHidKeycode(the_app->hid_config.play, TRUE);
            break;
        }
        case HidCmdStop:
        {
            DEBUG_HID(("Stop"));
            sendHidKeycode(the_app->hid_config.stop, TRUE);
            break;
        }
        case HidCmdNext:
        {
            DEBUG_HID(("Next"));
            sendHidKeycode(the_app->hid_config.next, TRUE);
            break;
        }
        case HidCmdPrev:
        {
            DEBUG_HID(("Prev"));
            sendHidKeycode(the_app->hid_config.prev, TRUE);
            break;
        }
        case HidCmdReject:
        {
            DEBUG_HID(("Reject"));
    #if defined USE_HID_TELEPHONY
            sendTelephonyHidReport(HidCmdReject);
    #endif        
            break;
        }
        case HidCmdCancel:
        {
            DEBUG_HID(("Cancel"));
    #if defined USE_HID_TELEPHONY
            sendTelephonyHidReport(HidCmdCancel);
    #endif        
            break;
        }
        case HidCmdUnused1:
        case HidCmdUnused2:
        case HidCmdUnused3:
        {
            DEBUG_HID(("Unused=Nop"));
            break;
        }
        case HidCmdSequence:
        {
            DEBUG_HID(("Sequence"));
            sendHidSequence();
            break;
        }
        default:
        {
            break;
        }
    }
    DEBUG_HID(("\n"));
}


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    usbHidInitTimeCritical

DESCRIPTION
    Time critical USB HID initialisation.
    
*/
void usbHidInitTimeCritical(void)
{
#ifdef ENABLE_KEYBD    
    /* Add an HID Class Interface */
    s_hid_keybd = UsbAddInterface(&s_codes_hid, HID_DESCRIPTOR, s_hid_keybd_descriptor, HID_DESCRIPTOR_LENGTH);
    if (s_hid_keybd == usb_interface_error) Panic();

    /* Register HID Keyboard report descriptor with the interface */
    (void) PanicFalse(UsbAddDescriptor(s_hid_keybd, HID_KEYBD_REPORT_DESCRIPTOR, s_hid_keybd_report_descriptor, HID_KEYBD_REPORT_DESCRIPTOR_LENGTH));
    /* Add required endpoints to the interface */
    (void) PanicFalse(UsbAddEndPoints(s_hid_keybd, 1, s_end_point_info_hid_keybd));
#else
    s_codes_hid = s_codes_hid;    
#endif
#ifdef ENABLE_CCD
    /* Add an HID Class Interface */
    s_hid_ccd = UsbAddInterface(&s_codes_hid, HID_DESCRIPTOR, s_hid_ccd_descriptor, HID_DESCRIPTOR_LENGTH);
    if (s_hid_ccd == usb_interface_error) Panic();

    /* Register HID Consumer Control Device report descriptor with the interface */
    (void) PanicFalse(UsbAddDescriptor(s_hid_ccd, HID_CCD_REPORT_DESCRIPTOR, s_hid_ccd_report_descriptor, HID_CCD_REPORT_DESCRIPTOR_LENGTH));
    /* Add required endpoints to the interface */
    (void) PanicFalse(UsbAddEndPoints(s_hid_ccd, 1, s_end_point_info_hid_ccd));
#else
    s_codes_hid = s_codes_hid;    
#endif    
}


/****************************************************************************
NAME
    usbHidInit

DESCRIPTION
    Initialise USB HID support in the app.
    
*/
void usbHidInit(void)
{
    /*  Register our task for the sinks */
    (void) MessageSinkTask(StreamUsbClassSink(s_hid_keybd), &the_app->task);
    (void) MessageSinkTask(StreamUsbClassSink(s_hid_ccd), &the_app->task);
}


/****************************************************************************
NAME
    usbHidHandleInterfaceEvent

DESCRIPTION
    Process a more data message for the USB source.
    
*/
void usbHidHandleInterfaceEvent(Source source)
{
    static uint8 idle_rate = 0;
    
    if (the_app->a2dp_source == SourceUsb)
    {    
        Sink sink = StreamSinkFromSource(source);
        uint16 packet_size;

        while ((packet_size = SourceBoundary(source)) != 0)
        {
            /* Build the respnse. It must contain the original request, so copy 
               from the source header. */
            UsbResponse resp;
            memcpy(&resp.original_request, SourceMapHeader(source), sizeof(UsbRequest));
        
            /* Set the response fields to default values to make the code below simpler */
            resp.success = FALSE;
            resp.data_length = 0;
        
            switch (resp.original_request.bRequest)
            {
                /* GET_REPORT */
                case 0x01:
                {
                    DEBUG_USB(("Get_Report src=0x%X wValue=0x%X wIndex=0x%X wLength=0x%X\n", (uint16)source, resp.original_request.wValue, resp.original_request.wIndex, resp.original_request.wLength));
                    break;
                }
            
                /* GET_IDLE */
                case 0x02:
                {
                    uint8 *out = SinkMap(sink) + SinkClaim(sink, 1);
                    DEBUG_USB(("Get_Idle src=0x%X wValue=0x%X wIndex=0x%X\n", (uint16)source, resp.original_request.wValue, resp.original_request.wIndex));
                    out[0] = idle_rate;
                    resp.success = TRUE;
                    resp.data_length = 1;                
                    break;
                }
            
                /* SET_REPORT */
                case 0x09:
                {
                    const uint8 *in = SourceMap(source);
                    DEBUG_USB(("Set_Report src=0x%X wValue=0x%X wIndex=0x%X wLength=0x%X -> \n", (uint16)source, resp.original_request.wValue, resp.original_request.wIndex, resp.original_request.wLength));
                    while (resp.original_request.wLength--)
                    {
                        DEBUG_USB(("0x%x \n",*in++));
                    }
                    in = in;
                    break;
                }
            
                /* SET_IDLE */
                case 0x0A:    
                {
                    DEBUG_USB(("Set_Idle src=0x%X wValue=0x%X wIndex=0x%X\n", (uint16)source, resp.original_request.wValue, resp.original_request.wIndex));
                    idle_rate = resp.original_request.wValue >> 8;
                    resp.success = TRUE;
                    break;
                }
            
                default:
                {
                    DEBUG_USB(("HID req=0x%X src=0x%X wValue=0x%X wIndex=0x%X wLength=0x%X\n", resp.original_request.bRequest, (uint16)source, resp.original_request.wValue, resp.original_request.wIndex, resp.original_request.wLength));
                    break;            
                }
            }
        
            /* Send response */
            if (resp.data_length)
            {
                (void)SinkFlushHeader(sink, resp.data_length, (uint16 *)&resp, sizeof(UsbResponse));
            }
            else
            {
                   /* Sink packets can never be zero-length, so flush a dummy byte */
                (void) SinkClaim(sink, 1);
                (void) SinkFlushHeader(sink, 1, (uint16 *) &resp, sizeof(UsbResponse));          
            }   

            /* Discard the original request */
              SourceDrop(source, packet_size);
        }
    }
}


/****************************************************************************
NAME
    usbHidSignalVolumeUp

DESCRIPTION
    Act on receiving volume up indication.
    
*/
void usbHidSignalVolumeUp (void)
{
    /* Not supported */
}


/****************************************************************************
NAME
    usbHidSignalVolumeDown

DESCRIPTION
    Act on receiving volume down indication.
    
*/
void usbHidSignalVolumeDown (void)
{
    /* Not supported */
}


/****************************************************************************
NAME
    usbHidSignalMuteState

DESCRIPTION
    Act on receiving mute indication.
    
*/
void usbHidSignalMuteState (bool state)
{
    /* Not supported */
}


/****************************************************************************
NAME
    usbHidIssueHidCommand

DESCRIPTION
    Issue a HID command based on the specified event and application state.
    
*/
void usbHidIssueHidCommand (mvdAppEvent event)
{
    if (the_app->a2dp_source == SourceUsb)
    {    
        mvdHidCommand command;
    
        DEBUG_HID(("HidCmd %u ",(uint16)event));

        switch (the_app->app_state )
        {
            case AppStateUninitialised:
            case AppStateInitialising:
            {
                break;
            }
            case AppStateIdle:
            case AppStateInquiring:
            case AppStateSearching:
            {
                DEBUG_HID(("D "));
                command = configureGetHidCommand(event, 8);
                actionHidCommand(command);
                break;
            }
            case AppStateStreaming:
            {
                DEBUG_HID(("S "));
                command = configureGetHidCommand(event, 4);
                actionHidCommand(command);
                break;
            }
            case AppStateInCall:
            {
                DEBUG_HID(("V "));
                command = configureGetHidCommand(event, 0);
                actionHidCommand(command);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    DEBUG_HID(("\n"));
}


/****************************************************************************
NAME
    usbHidIssueNextHidCommand

DESCRIPTION
    Issue next HID sequence.
    
*/
void usbHidIssueNextHidCommand (void)
{
    if (the_app->a2dp_source == SourceUsb)
    {
        DEBUG_HID(("NextHidSequence\n"));
        sendHidSequence();
    }
}
