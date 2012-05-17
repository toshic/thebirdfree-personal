/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    audioAdaptor application main file.    
*/


/****************************************************************************
    Header files
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_usb_audio.h"
#include "audioAdaptor_configure.h"
#include "audioAdaptor_usb_hid.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_a2dp_stream_control.h"  
#include "audioAdaptor_aghfp_msg_handler.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_sys_handler.h"
#include "audioAdaptor_cl_msg_handler.h"
#include "audioAdaptor_codec_msg_handler.h"

#include <string.h>
#include <stdlib.h>
#include <panic.h>
#include <codec.h>
#include <pio.h>
#include <pcm.h>
#include <ps.h>
#include <file.h>
#include <kalimba.h>
#include <charger.h>
#include <kalimba_standard_messages.h>


/* Single instance of the Headset state */
mvdTaskData *the_app = NULL;


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    unhandledMessage

DESCRIPTION
    For debug purposes to catch unhandled state in the message handler.
    
*/
static void unhandledMessage(MessageId id)
{
    DEBUG(("Unhandled MAIN message 0x%X\n", (uint16)id));
}


/****************************************************************************
NAME
    messageHandler

DESCRIPTION
    Main application message handler.
    
*/
static void messageHandler(Task task, MessageId id, Message message)
{
    if (SYSTEM_MESSAGE(id))
    {
        sysHandleSystemMessage(id, message);
    }
    else if (CL_MESSAGE(id))
    {
        clMsgHandleLibMessage(id, message);
    }
    else if (CODEC_MESSAGE(id))
    {     
        codecMsgHandleLibMessage(id,  message);
    }
    else if (A2DP_MESSAGE(id))    
    {
        a2dpMsgHandleLibMessage(id, message);
    } 
    else if (AVRCP_MESSAGE(id))    
    {
        avcrpMsgHandleLibMessage(id, message);
    }
    else if (AGHFP_MESSAGE(id))    
    {
        aghfpMsgHandleLibMessage(id, message);
    }
    else if (APP_MESSAGE(id))    
    {
        eventHandleAppMessage(id, message);
    }
    else
    {    
        if(!sysHandleButtonsMessage(id, message))
        {
            /* If it's not a button message, it's unrecognised */
            unhandledMessage(id);
        }
    }
}


/****************************************************************************
  MAIN FUNCTIONS
*/
      
/****************************************************************************
NAME
    _init

DESCRIPTION
    Extra initialisation for time critical functionality .
    
*/
extern void _init(void);
void _init(void)
{
    uint8 source_type = SourceAnalog;
    
    /* Get the supported source input. Assume Analog if PS Key not available. */
    PsRetrieve(PSKEY_SOURCE_TYPE, &source_type, sizeof(uint8));
    
    if (source_type == SourceUsb)
    {
        usbAudioTimeCritical();
        usbHidInitTimeCritical();
    }
}


/****************************************************************************
NAME
    main

DESCRIPTION
    Main application entry point.
    
*/
int main(void)
{
#ifndef NO_CHARGER_TRAPS
    PioSetPsuRegulator ( TRUE ) ;
    /*disable the charging LED indications on BC5 - the app will handle these*/
    ChargerSupressLed0 ( TRUE )  ;
#endif    
    
    /*Power on */  
#ifdef SOFT_POWER
    PioSetDir(POWER_HOLD, POWER_HOLD);
    PioSet(POWER_HOLD, POWER_HOLD);
#endif
    
    /* Set up the application task handler */
    the_app = (mvdTaskData *)malloc(sizeof(mvdTaskData));
    PanicNull(the_app);
    memset(the_app, 0, sizeof(mvdTaskData));
        
    the_app->task.handler = messageHandler;
    MessageKalimbaTask(&the_app->task);
    
    /* Initialise the features */
    initUserFeatures ();
    
    /* Start the message scheduler loop and init application */
    MessageSend(&the_app->task, APP_INIT, 0);
    MessageLoop();

    /* Never get here! */
    return 0;
}
