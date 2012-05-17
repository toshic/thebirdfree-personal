/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles system messages received from Bluestack.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_test.h"
#include "audioAdaptor_sys_handler.h"
#include "audioAdaptor_usb_hid.h"
#include "audioAdaptor_charger.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_statemanager.h"

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
        case BUTTON_DEVICE_CONNECT_REQ:
        {
            DEBUG(("BUTTON_DEVICE_CONNECT_REQ\n"));    
            if (the_app->dfu_requested)
            {
                the_app->dfu_requested = FALSE;
                testCancelDfuModeRequest();
            }
            else 
            {                                
                /* If it is clearing pdl, no connection attempts are allowed */
                if (!the_app->clearing_pdl && the_app->audioAdaptorPoweredOn)
                {
                    MAKE_APP_MESSAGE(APP_DEVICE_CONNECT_REQ);
                    message->disconnect_current = TRUE;
#ifdef DUAL_STREAM
                    MessageSendLater(&the_app->task, APP_DEVICE_CONNECT_REQ, message, 800);
#else /* DUAL_STREAM not defined */
                    MessageSend(&the_app->task, APP_DEVICE_CONNECT_REQ, message);
#endif /* DUAL_STREAM */                                        
                    the_app->PowerOffIsEnabled  = TRUE;
                }
            }
            break;
        }
        case BUTTON_DEVICE_DISCOVER_REQ:
        {
            DEBUG(("BUTTON_DEVICE_DISCOVER_REQ\n"));
            if (the_app->dfu_requested)
            {
                the_app->dfu_requested = FALSE;
                testCancelDfuModeRequest();
            }
            else
            {
                /* If it is clearing pdl, no discovering attempts are allowed */
                if (!the_app->clearing_pdl && the_app->audioAdaptorPoweredOn)
                {
                    MAKE_APP_MESSAGE(APP_DEVICE_DISCOVER_REQ);
                    message->disconnect_current = TRUE;
                    MessageSend(&the_app->task, APP_DEVICE_DISCOVER_REQ, message);
                    the_app->PowerOffIsEnabled  = TRUE;
                }
            }
            break;
        }
        case BUTTONS_CLEAR_PDL_REQ:
        {
            DEBUG(("BUTTONS_CLEAR_PDL_REQ\n"));    
            if (the_app->dfu_requested)
            {
                the_app->dfu_requested = FALSE;
                testCancelDfuModeRequest();
            }
            else
            {
                /* don't send clearing pdl request again, if it is still clearing it */
                if (!the_app->clearing_pdl)
                {
                    the_app->clearing_pdl = TRUE;
                
                    /* Disconnecting all connections */
                    kickCommAction(CommActionDisconnect);
  
                    /* Now just waiting for switch off */
                    /* Set a timeout so that we will turn off eventually anyway */
                    MessageSendLater ( &the_app->task , APP_RESET_PDL_REQ, 0 , D_SEC(3) ) ;
                }
            }            
            break;
        }
        case BUTTON_PWR_OFF_REQ:
        {
            DEBUG(("BUTTON_PWR_OFF_REQ\n"));
            if (the_app->a2dp_source == SourceAnalog)
            {
                if (the_app->audioAdaptorPoweredOn)
                {
                    MessageSend(&the_app->task, APP_POWEROFF_REQ, NULL);
                }
                else
                {
                    MessageSend(&the_app->task, APP_POWERON_REQ, NULL);
                }                
            }            
            break;
        }
        case BUTTON_PWR_RELEASE:
        {
            DEBUG(("BUTTON_PWR_RELEASE\n"));
            if (!the_app->PowerOffIsEnabled)
            {
                the_app->PowerOffIsEnabled = TRUE;
            }
            break;
        }
        case BUTTON_CONNECT_SECOND_DEVICE_REQ:
        {
#ifdef DUAL_STREAM
            DEBUG(("BUTTON_CONNECT_2ND_DEVICE_REQ\n"));    
            if (the_app->dfu_requested)
            {
                the_app->dfu_requested = FALSE;
                testCancelDfuModeRequest();
            }
            else 
            {            
                /* If it is clearing pdl, no connection attempts are allowed */
                if (!the_app->clearing_pdl && the_app->audioAdaptorPoweredOn)
                {
                    MessageCancelAll(&the_app->task, APP_DEVICE_CONNECT_REQ);
                    MessageSend(&the_app->task, APP_DEVICE_CONNECT_SECOND_REQ, NULL);
                }
            }
#endif /* DUAL_STREAM */
            break;
        }
#if defined DEV_PC_1645_USB || defined DEV_PC_1645_ANALOGUE || defined RED_PC_142
        case CHARGER_RAW:
        {
            DEBUG(("CHARGER_RAW 0x%x\n",((CHARGER_RAW_T*)message)->chg));    
            
            if (the_app->a2dp_source == SourceAnalog)
            {
                if (((CHARGER_RAW_T*)message)->chg)
                {
                     /* Send a message to indicate that charger is connected and
                        set the proper flash pattern */
                    MessageSend(&the_app->task, APP_CHARGER_CONNECTED, 0 );
                    the_app->chargerconnected = TRUE;
                }
                else
                {
                    /* send a message to indicate that charger is disconnected */
                    MessageSend(&the_app->task, APP_CHARGER_DISCONNECTED, 0 );
                    the_app->chargerconnected = FALSE;        
                }    
            }
            break;
        }
#endif
        
#if ((defined DEV_PC_1645_USB || defined DEV_PC_1645_ANALOGUE) && defined DEMO_MODE)
        case BUTTON_MODE_FASTSTREAM:
        {
            uint16 i;
            bool codec_chosen = FALSE;           
            DEBUG(("BUTTON_MODE_FASTSTREAM\n"));
            
            /* check the Faststream codec is enabled */
            if (!(the_app->a2dpCodecsEnabled & (1<<FASTSTREAM_CODEC_BIT)))
            {
                DEBUG(("    - codec not enabled\n"));
                break;
            }
                
            for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
            {
                if (the_app->dev_inst[i] != NULL)
                {
                    switch (the_app->dev_inst[i]->a2dp_state)
                    {
                        case A2dpStateConnected:
                        case A2dpStateOpen:
                        case A2dpStateStarting:
                        case A2dpStateStreaming:
                        case A2dpStateSuspending:
                        {                        
                            the_app->dev_inst[i]->a2dp_closing = TRUE;
                            setA2dpState(the_app->dev_inst[i], A2dpStateClosing);
                            A2dpClose(the_app->dev_inst[i]->a2dp);
                            the_app->dev_inst[i]->a2dp_reopen_tries = 0;
#ifdef DUAL_STREAM                
                            if (codec_chosen)
                                the_app->dev_inst[i]->a2dp_reopen_codec = FASTSTREAM_DS_SEID;
                            else
#endif                    
                                the_app->dev_inst[i]->a2dp_reopen_codec = FASTSTREAM_SEID;
                            codec_chosen = TRUE;
                            MessageSendConditionally(&the_app->dev_inst[i]->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, &the_app->dev_inst[i]->a2dp_closing);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
            break;
        }   
        case BUTTON_MODE_SBC:
        {
            uint16 i;
            bool codec_chosen = FALSE;
            DEBUG(("BUTTON_MODE_SBC\n"));
            
            for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
            {
                if (the_app->dev_inst[i] != NULL)
                {
                    switch (the_app->dev_inst[i]->a2dp_state)
                    {
                        case A2dpStateConnected:
                        case A2dpStateOpen:
                        case A2dpStateStarting:
                        case A2dpStateStreaming:
                        case A2dpStateSuspending:
                        {     
                            the_app->dev_inst[i]->a2dp_closing = TRUE;
                            setA2dpState(the_app->dev_inst[i], A2dpStateClosing);
                            A2dpClose(the_app->dev_inst[i]->a2dp);
                            the_app->dev_inst[i]->a2dp_reopen_tries = 0;
#ifdef DUAL_STREAM                
                            if (codec_chosen)
                                the_app->dev_inst[i]->a2dp_reopen_codec = SBC_DS_SEID;
                            else
#endif                    
                                the_app->dev_inst[i]->a2dp_reopen_codec = SBC_SEID;
                            codec_chosen = TRUE;
                            MessageSendConditionally(&the_app->dev_inst[i]->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, &the_app->dev_inst[i]->a2dp_closing);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
            break;
        }   
        case BUTTON_MODE_MP3:
        {
#ifdef INCLUDE_MP3_ENCODER_PLUGIN            
            uint16 i;
            bool codec_chosen = FALSE;
            DEBUG(("BUTTON_MODE_MP3\n"));
            
            /* check the MP3 codec is enabled */
            if (!(the_app->a2dpCodecsEnabled & (1<<MP3_CODEC_BIT)))
            {
                DEBUG(("    - codec not enabled\n"));
                break;
            }
            
            for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
            {
                if (the_app->dev_inst[i] != NULL)
                {
                    switch (the_app->dev_inst[i]->a2dp_state)
                    {
                        case A2dpStateConnected:
                        case A2dpStateOpen:
                        case A2dpStateStarting:
                        case A2dpStateStreaming:
                        case A2dpStateSuspending:
                        {     
                            if (!codec_chosen)
                            {
                                the_app->dev_inst[i]->a2dp_closing = TRUE;
                                setA2dpState(the_app->dev_inst[i], A2dpStateClosing);
                                A2dpClose(the_app->dev_inst[i]->a2dp);
                                the_app->dev_inst[i]->a2dp_reopen_tries = 0;    
                                the_app->dev_inst[i]->a2dp_reopen_codec = MP3_SEID;
                                codec_chosen = TRUE;
                                MessageSendConditionally(&the_app->dev_inst[i]->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, &the_app->dev_inst[i]->a2dp_closing);
                            }
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
#endif /* INCLUDE_MP3_ENCODER_PLUGIN */   
            break;
        }   
#endif /* (DEV_PC_1645_USB ||  DEV_PC_1645_ANALOGUE) && DEMO_MODE */
        
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
        case MESSAGE_MORE_DATA:
        {
            DEBUG(("MESSAGE_MORE_DATA\n"));
            /* If this is on the USB source handle it as such */                
            usbHidHandleInterfaceEvent(((MessageMoreData *)message)->source);
            break;
        }
        default:
        {
            DEBUG(("Unhandled System message 0x%X\n", (uint16)id));
            break;
        }
    }
}

