/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles system messages received from Bluestack.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_sys_handler.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_statemanager.h"
#include "at_cmd.h"
#include "SppServer.h"

#include <pio.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>


#define VISTA_MSG_FILTER_DELAY 125    /* Used to filter DSP messages caused by Skype on Vista "blipping" mic stream */
#define DSP_MSG_FILTER_DELAY   500    /* Used to filter DSP message spam, caused by f/w delaying delivery of USB packets when an eSCO/SCO link closed */


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    sysHandleButtonsMessage

DESCRIPTION
    Handle messages received from button code.
    
*/
bool sysHandleButtonsMessage(MessageId id, Message message)
{
    switch(id)
    {

        
        default:
        {
            /* Unrecognised messages */
            return FALSE;
        }
    }
    
    return TRUE;
}


/****************************************************************************
NAME
    sysHandleSystemMessage

DESCRIPTION
    Handle messages received from firmware/DSP.
    
*/
void sysHandleSystemMessage(MessageId id, Message message)
{
    switch(id)
    {                
        case MESSAGE_FROM_KALIMBA:
        {
            switch (*((uint16*)message))
            {
                case KALIMBA_AUDIO_USB_OUT_STATUS:
                {
                    DEBUG_KALIMBA(("KALIMBA_AUDIO_USB_OUT_STATUS:%u\n",*(((uint16*)message)+1)));
                    if ( *(((uint16*)message)+1) )
                    {
                        if ( !MessageCancelFirst(&the_app->task, APP_AUDIO_STREAMING_INACTIVE) )
                        {
                            MessageSend(&the_app->task, APP_AUDIO_STREAMING_ACTIVE, NULL);
                        }
                    }
                    else
                    {
                        MessageSendLater(&the_app->task, APP_AUDIO_STREAMING_INACTIVE, NULL, DSP_MSG_FILTER_DELAY);
                    }
                    break;
                }
                case KALIMBA_AUDIO_USB_IN_STATUS:
                {
                    DEBUG_KALIMBA(("KALIMBA_AUDIO_USB_IN_STATUS:%u\n",*(((uint16*)message)+1)));
        #if !defined USE_HID_TELEPHONY
                    /* only do VOIP calls if HFP connected */
                    if (aghfpSlcGetConnectedHF() != NULL)
                    {
                        if ( *(((uint16*)message)+1) )
                        {
                            if ( !MessageCancelFirst(&the_app->task, APP_VOIP_CALL_INACTIVE) )
                            {
                                MessageSendLater(&the_app->task, APP_VOIP_CALL_ACTIVE, NULL, VISTA_MSG_FILTER_DELAY);
                            }
                        }
                        else
                        {
                            if ( !MessageCancelFirst(&the_app->task, APP_VOIP_CALL_ACTIVE) )
                            {
                                MessageSendLater(&the_app->task, APP_VOIP_CALL_INACTIVE, NULL, DSP_MSG_FILTER_DELAY);
                            }
                        }
                    }
        #endif            
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case MESSAGE_MORE_SPACE:
            break;
        case MESSAGE_MORE_DATA:
        {
            Source source = ((MessageMoreData *)message)->source;
            /* If this is on the USB source handle it as such */     

            if(source == StreamUartSource())
                parseUart(source,&the_app->task);
            else
                SppParse(source);
            break;
        }
        default:
        {
            DEBUG(("Unhandled System message 0x%X\n", (uint16)id));
            break;
        }
    }
}

