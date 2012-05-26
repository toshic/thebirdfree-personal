/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_event_handler.c
@brief    Handle events arriving at the app.
*/

#include "headset_a2dp_connection.h"
#include "headset_avrcp_event_handler.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_event_handler.h"
#include "headset_events.h"
#include "headset_hfp_call.h"
#include "headset_hfp_slc.h"
#include "headset_inquiry.h"
#include "headset_led_manager.h"
#include "headset_tones.h"
#include "headset_statemanager.h"
#include "headset_volume.h"
#include "headset_auth.h"
#ifdef SD_SUPPORT
#include "headset_sd.h"
#endif

#include <audio.h>
#include <boot.h>
#include <stddef.h>


#ifdef DEBUG_EVENTS
#define EVENTS_DEBUG(x) DEBUG(x)
#else
#define EVENTS_DEBUG(x) 
#endif


/****************************************************************************/
static bool eventAllowedWhileProfilesInitialising(MessageId id)
{
	switch (id)
	{
		case EventPowerOn:
		case EventPowerOff:
		case EventOkBattery:
		case EventChargerConnected:
        case EventChargerDisconnected:        
        case EventTrickleCharge:
		case EventFastCharge:      
		case EventChargeError: 
		case EventLowBattery:
		case EventLEDEventComplete:
		case EventCancelLedIndication:
		case EventTone1:
		case EventTone2:
			return TRUE;
		default:
			return FALSE;
	}
}

#ifdef SD_SUPPORT
static void eventHandleEventInSDMode(MessageId id)
{
    APP_SD_EVENT_T *message;
    sd_event_id sd_event = BUTTON_INVALID;

    /* set event type */
    switch (id)
    {
        case EventPlay:
        case EventEstablishA2dp:
            sd_event = BUTTON_PLAY_PAUSE;
            break;
        case EventPause:
            sd_event = BUTTON_PLAY_PAUSE;
            break;
        case EventStop:
            sd_event = BUTTON_STOP;
            break;
        case EventSkipForward:
            sd_event = BUTTON_NEXT;
            break;
        case EventSkipBackward:
            sd_event = BUTTON_PREV;
            break;
        case EventFFWDPress:
            sd_event = BUTTON_NEXT_HELD;
            break;
        case EventFFWDRelease:
            sd_event = BUTTON_NEXT_RELEASED;
            break;
        case EventRWDPress:
            sd_event = BUTTON_PREV_HELD;
            break;
        case EventRWDRelease:
            sd_event = BUTTON_PREV_RELEASED;
            break;
        default:
            /* not a valid event for SD player, just return */
            return;
    }

    /* create message to post event to SD player */
    message = PanicUnlessNew(APP_SD_EVENT_T);
    message->event = sd_event;
    /* conditionally send message to SD player when it isn't busy */
    MessageSendConditionally(&theHeadset.task, APP_SD_EVENT, message, &theHeadset.sd_player->sd_busy);
}
#endif


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************/
void handleUEMessage( Task task, MessageId id, Message message )
{ 
    headsetHfpState lState = stateManagerGetHfpState() ;
    
    /* If we do not want the event received to be indicated then set this to FALSE. */
    bool lIndicateEvent = TRUE ;

#ifdef SD_SUPPORT
    /* in SD mode bypass normal event handling */
    if (theHeadset.sd_player != NULL && theHeadset.sd_player->sd_mode == 1)
    {
        eventHandleEventInSDMode(id);
        return;
    }
#endif

	
	if (theHeadset.ProfileLibrariesInitialising)
	{
		if (!eventAllowedWhileProfilesInitialising(id))
		{
			EVENTS_DEBUG(("Event 0x%x while profiles initialising - queue\n",id));
			MessageSendConditionally(&theHeadset.task, id, 0, &theHeadset.ProfileLibrariesInitialising);
			return;
		}
	}    
    
    /* Deal with user generated Event specific actions*/
    switch ( id )
    {   
            /*these are the events that are not user generated and can occur at any time*/
        case EventOkBattery:
        case EventChargerDisconnected:
        case EventLEDEventComplete:
        case EventTrickleCharge:
		case EventFastCharge:
        case EventLowBattery:
        case EventPowerOff:
        case EventLinkLoss:
        case EventSLCConnected:
		case EventA2dpConnected:
        case EventError:
        case EventChargeError:
        case EventCancelLedIndication:
        case EventAutoSwitchOff:
		case EventHfpReconnectFailed:
		case EventA2dpReconnectFailed:
            /*do nothing for these events*/
        break ;
        default:
                    /* If we have had an event then reset the timer - if it was the event then we will switch off anyway*/
            if (theHeadset.Timeouts.AutoSwitchOffTime_s !=0)
            {
                /*EVENTS_DEBUG(("HS: AUTOSent Ev[%x] Time[%d]\n",id , theHeadset.Timeouts.AutoSwitchOffTime_s )) ;*/
                MessageCancelAll( &theHeadset.task , EventAutoSwitchOff ) ;
                MessageSendLater( &theHeadset.task , EventAutoSwitchOff , 0 , D_SEC(theHeadset.Timeouts.AutoSwitchOffTime_s) ) ;                
            }
			
#ifdef ROM_LEDS			
                /*handles the LED event timeouts - restarts state indications if we have had a user generated event only*/
            if (theHeadset.theLEDTask.gLEDSStateTimeout)
            {   
                EVENTS_DEBUG(("HS: Restart St Inds[%d]\n", lState )) ;                    
                LEDManagerIndicateState (lState , stateManagerGetA2dpState()) ;     
                theHeadset.theLEDTask.gLEDSStateTimeout = FALSE ;
            }
            else
            {
                    /*reset the current number of repeats complete - i.e restart the timer so that the leds will disable after
                    the correct time*/
                LEDManagerResetStateIndNumRepeatsComplete() ;
            }
#endif			
   
        break;
    }
    
    
    switch (id)
    {
    case EventPowerOn:
        EVENTS_DEBUG(("EventPowerOn\n"));
		
		if (!theHeadset.headsetPoweredOn)
		{
			theHeadset.headsetPoweredOn = TRUE;
			theHeadset.a2dp_list_index = 0xf;
			theHeadset.hfp_list_index = 0xf;
			theHeadset.switch_a2dp_source = FALSE;
		
        	stateManagerPowerOn();
			
			if(theHeadset.Timeouts.EncryptionRefreshTimeout_m != 0)
				MessageSendLater(&theHeadset.task, APP_EVENT_REFRESH_ENCRYPTION, 0, D_MIN(theHeadset.Timeouts.EncryptionRefreshTimeout_m));
			
			if ( theHeadset.Timeouts.DisablePowerOffAfterPowerOnTime_s )
        	{
            	theHeadset.PowerOffIsEnabled = FALSE ;
            	MessageSendLater ( &theHeadset.task , APP_ENABLE_POWER_OFF , 0 , D_SEC ( theHeadset.Timeouts.DisablePowerOffAfterPowerOnTime_s ) ) ;
        	}
        	else
        	{
            	theHeadset.PowerOffIsEnabled = TRUE ;
        	}
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
        break;
    case EventPowerOff:
        EVENTS_DEBUG(("EventPowerOff\n"));
		if (theHeadset.buttons_locked || !theHeadset.PowerOffIsEnabled)
		{
			EVENTS_DEBUG(("Buttons Locked / Power off disabled - ignore event\n"));
			lIndicateEvent = FALSE ;
			break;
		}
		
		theHeadset.headsetPoweredOn = FALSE;
		theHeadset.a2dp_list_index = 0xf;
		theHeadset.hfp_list_index = 0xf;
		theHeadset.switch_a2dp_source = FALSE;
		
        stateManagerEnterPoweringOffState();
        AuthResetConfirmationFlags();
        if (theHeadset.gMuted)
            VolumeMuteOff() ;
        hfpCallClearQueuedEvent() ;
		
		if(theHeadset.Timeouts.EncryptionRefreshTimeout_m != 0)
    		MessageCancelAll ( &theHeadset.task, APP_EVENT_REFRESH_ENCRYPTION) ;
		
        MessageCancelAll ( &theHeadset.task , EventPairingFail) ;
		MessageCancelAll ( &theHeadset.task, APP_CHECK_FOR_LOW_BATT) ;
        break;
    case EventCancelLedIndication:
        EVENTS_DEBUG(("EventCancelLedIndication\n"));
#ifdef ROM_LEDS		
        LedManagerResetLEDIndications() ;            
#endif		
		break ;  
    case EventOkBattery:
        /*EVENTS_DEBUG(("EventOkBattery\n"));*/
        break;
    case EventLowBattery:
        EVENTS_DEBUG(("EventLowBattery\n"));
        break;
    case EventEnterPairing:    
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}       
		EVENTS_DEBUG(("EventEnterPairing\n"));
        stateManagerEnterConnDiscoverableState( ) ;       
        break ;
    case EventPairingFail:  
        EVENTS_DEBUG(("EventPairingFail\n"));
        if (lState != headsetTestMode)
        {      
			switch (theHeadset.features.exitPairingModeAction)
			{
			case PairingExit_PowerOff:
				MessageSend ( &theHeadset.task , EventPowerOff , 0) ;
				break;
			case PairingExit_PowerOffPdlEmpty:
				if (theHeadset.PDLEntries == 0)
					MessageSend ( &theHeadset.task , EventPowerOff , 0) ;
				else
					stateManagerEnterHfpConnectableState( TRUE) ; 
				break;
			case PairingExit_Connectable:
				stateManagerEnterHfpConnectableState( TRUE) ; 
				break;
			default:
				break;
			}
        }
        break ;    
    case EventPairingSuccessful:
        EVENTS_DEBUG(("EventPairingSuccessful\n"));
        if (lState == headsetConnDiscoverable)
        {
            stateManagerEnterHfpConnectableState( FALSE) ;
        }
        break ;
    case EventConfirmationAccept:
        EVENTS_DEBUG(("EventConfirmationAccept\n"));
        headsetPairingAcceptRes();
        break;
    case EventConfirmationReject:
        EVENTS_DEBUG(("EventConfirmationReject\n"));
        headsetPairingRejectRes();
        break;
    case EventToggleDebugKeys:
        EVENTS_DEBUG(("EventToggleDebugKeys\n"));
        /* if debug keys functionality enabled toggle current state */
        if (theHeadset.features.debugKeysEnabled)
        {
            ConnectionSmSecModeConfig(&theHeadset.task,
                                      cl_sm_wae_acl_owner_none,
                                      !theHeadset.debugKeysInUse,
                                      TRUE);
        }
        break;
    case EventSLCConnected:
        EVENTS_DEBUG(("EventSLCConnected\n"));
        hfpCallRecallQueuedEvent() ;
        break;
    case EventLinkLoss:
        EVENTS_DEBUG(("EventLinkLoss\n"));        
        break;
    case EventSLCDisconnected:
        EVENTS_DEBUG(("EventSLCDisconnected\n"));
		if (lState == headsetPoweringOn )
			lIndicateEvent = FALSE ;
		
        theHeadset.voice_recognition_enabled = FALSE;
        break;
    case EventTone1:
        EVENTS_DEBUG(("EventTone1\n"));  
		/* Disable event tone 1 if button locking active */
		if (theHeadset.buttons_locked)
			lIndicateEvent = FALSE ;
        break;
    case EventTone2:
        EVENTS_DEBUG(("EventTone2\n"));    
		/* Event tone 2 active regardless of button locking */
        break;
    case EventLEDEventComplete:
        EVENTS_DEBUG(("EventLEDEventComplete\n"));
		
#ifdef ROM_LEDS			
        if ( (( LMEndMessage_t *)message)->Event  == EventResetPairedDeviceList )
        {
            /* The reset has been completed */
            MessageSend(&theHeadset.task , EventResetComplete , 0 ) ;
        }
       	
		if (theHeadset.features.QueueLEDEvents )
        {
        	if (theHeadset.theLEDTask.Queue.Event1)
        	{
            	EVENTS_DEBUG(("HS : Play Q'd Ev [%x]\n", (theHeadset.theLEDTask.Queue.Event1)  ));
            	/*EVENTS_DEBUG(("HS : Queue [%x][%x][%x][%x]\n", theHeadset.theLEDTask.Queue.Event1,
                                                              theHeadset.theLEDTask.Queue.Event2,
                                                              theHeadset.theLEDTask.Queue.Event3,
                                                              theHeadset.theLEDTask.Queue.Event4
                                                                    
                                                                ));*/
                
             	LEDManagerIndicateEvent ( (theHeadset.theLEDTask.Queue.Event1) ) ;
    
             	/*shuffle the queue*/
             	theHeadset.theLEDTask.Queue.Event1 = theHeadset.theLEDTask.Queue.Event2 ;
             	theHeadset.theLEDTask.Queue.Event2 = theHeadset.theLEDTask.Queue.Event3 ;
             	theHeadset.theLEDTask.Queue.Event3 = theHeadset.theLEDTask.Queue.Event4 ;
             	theHeadset.theLEDTask.Queue.Event4 = 0x00 ;
        	}
		}
#endif
		
        break;
    case EventEstablishSLC:   
		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}		
		EVENTS_DEBUG(("EventEstablishSLC\n"));
		
		/* Cancel inquiry and throw away results if one is in progress */
        inquiryStop();
		
		if (theHeadset.slcConnectFromPowerOn)
		{
			/* This is from power on so carry out the power on connect sequence */
			lIndicateEvent = FALSE ;			
					
			/* Determine what the headset should connect to now it is powered on */ 
			switch (theHeadset.features.ConnectActionOnPowerOn)
			{
			case AR_ConnectToAG:
				theHeadset.slcConnectFromPowerOn = FALSE;
				if (theHeadset.features.PowerOnConnectToDevices == Connect_List)
				{
					/* Connect to devices in list order */	
					hfpSlcListConnection();
				}
				else
				{
					/* Try a connection attempt to Last AG. */  
					hfpSlcLastConnectRequest( hfp_handsfree_profile ) ;	
				}
				break;
			case AR_ConnectToAGandA2DP:
				if (theHeadset.features.PowerOnConnectToDevices == Connect_List)
				{
					/* Connect to devices in list order */	
					if (!hfpSlcListConnection())
						theHeadset.slcConnectFromPowerOn = FALSE;
				}
				else
				{
					/* Try a connection attempt to Last AG. */  
					if (!hfpSlcLastConnectRequest( hfp_handsfree_profile ))
						theHeadset.slcConnectFromPowerOn = FALSE;
				}
				break;
			case AR_None:
			default:
				theHeadset.slcConnectFromPowerOn = FALSE;
				break;
			}	
		}
		else
		{
			/* This is a manual connect */
			if (theHeadset.features.ManualConnectToDevices == Connect_List)
			{
				/* Connect to devices in list order */	
				if (!hfpSlcListConnection())
					lIndicateEvent = FALSE ;
			}
			else
			{
				/* Try a connection attempt to Last AG. */       
        		if (!hfpSlcLastConnectRequest( hfp_handsfree_profile ) )
					lIndicateEvent = FALSE ;
			}
		}
        
        break;
    case EventHfpReconnectFailed:
        EVENTS_DEBUG(("EventHfpReconnectFailed\n"));
        break;
	case EventA2dpReconnectFailed:
        EVENTS_DEBUG(("EventA2dpReconnectFailed\n"));
        break;
    case EventInitateVoiceDial:                   
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventInitateVoiceDial [%d]\n", theHeadset.voice_recognition_enabled )) ; 
        /* Toggle the voice dial behaviour depending on whether we are currently active */
        if (theHeadset.voice_recognition_enabled)
        {
            hfpCallCancelVoiceDial() ;
            lIndicateEvent = FALSE ;
        }
        else
        {     
            if (!hfpCallInitiateVoiceDial())
				lIndicateEvent = FALSE ;
        }            
        break ;
    case EventLastNumberRedial:          
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}		
		
		if (theHeadset.features.LNRCancelsVoiceDialIfActive)
		{
			if (theHeadset.voice_recognition_enabled)
			{
				MessageSend(&theHeadset.task, EventInitateVoiceDial, 0);
				lIndicateEvent = FALSE ;
				break;
			}
		}
		
		EVENTS_DEBUG(("EventLastNumberRedial\n" )) ; 
		
        if (!hfpCallInitiateLNR())
			lIndicateEvent = FALSE ;
        break ;
    case EventAnswer:
        EVENTS_DEBUG(("EventAnswer\n" )) ;		
        /* Call the HFP lib function, this will determine the AT cmd to send
           depending on whether the profile instance is HSP or HFP compliant. */ 
        hfpCallAnswer();
        break ; 
    case EventReject:
        EVENTS_DEBUG(("EventReject\n" )) ;
        /* Reject incoming call - only valid for instances of HFP. */ 
        hfpCallReject();
        break ;
    case EventCancelEnd:
		if (theHeadset.features.EndCallWithNoSCOtransfersAudio && !HfpGetAudioSink(theHeadset.hfp_hsp))
		{
			lIndicateEvent = FALSE;
			MessageSend(&theHeadset.task, EventTransferToggle, 0);
		}
		else
		{
        	EVENTS_DEBUG(("EventCancelEnd\n" )) ;
        	/* Terminate the current ongoing call process */
        	hfpCallHangUp();
		}
        break ;
    case EventTransferToggle :
	    EVENTS_DEBUG(("EventTransferToggle\n")) ;    
        hfpCallTransferToggle() ;
	    break ;
    case EventSCOLinkOpen :        
        EVENTS_DEBUG(("EventScoLinkOpen\n")) ;
        break ;
    case EventSCOLinkClose:        
        EVENTS_DEBUG(("EventScoLinkClose\n")) ;
        break ;        
    case EventResetPairedDeviceList:          
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventResetPairedDeviceList\n")) ;  
        if ( stateManagerIsHfpConnected () )
        {
            /* Then we have an SLC active */
            hfpSlcDisconnect();
        }             
		if ( stateManagerIsA2dpConnected() )
    	{      
       		a2dpDisconnectRequest();
    	}
        configManagerReset() ;
        break ;
    case EventToggleMute:
        EVENTS_DEBUG(("EventToggleMute\n")) ;
        VolumeToggleMute() ;
        break ;    
    case EventMuteOn :
        EVENTS_DEBUG(("EventMuteOn\n")) ;
        VolumeMuteOn() ;
        break ;
    case EventMuteOff:
        EVENTS_DEBUG(("EventMuteOff\n")) ;
        VolumeMuteOff() ;
        break ;
    case EventMuteReminder :        
        EVENTS_DEBUG(("EventMuteReminder\n")) ;
        MessageSendLater( &theHeadset.task , EventMuteReminder , 0 ,D_SEC(theHeadset.Timeouts.MuteRemindTime_s ) )  ;            
        break;
    case EventEnterDutState :
        EVENTS_DEBUG(("EventEnterDutState\n")) ;            
		stateManagerEnterTestModeState() ;
        break;  
	case EventEnterDutMode :
        EVENTS_DEBUG(("EventEnterDutMode\n")) ; 
		if (lState != headsetTestMode)
        {
        	MessageSend( task , EventEnterDutState, 0 ) ;
        }
		ConnectionEnterDutMode();
        break;
	case EventEnterTXContTestMode:
		EVENTS_DEBUG(("EventEnterTXContTestMode\n"));
		if (lState != headsetTestMode)
        {
        	MessageSend( task , EventEnterDutState, 0 ) ;
        }
		MessageSendLater( task , APP_TX_TEST_MODE, 0, 1000 ) ;
		break;
    case (EventAutoSwitchOff):
        EVENTS_DEBUG(("EventAutoSwitchOff [%d] sec elapsed\n" , theHeadset.Timeouts.AutoSwitchOffTime_s )) ;
        switch ( lState )
            {                   
                case headsetHfpConnectable:
                case headsetConnDiscoverable:
					if (!stateManagerIsA2dpConnected() && !stateManagerIsAvrcpConnected())
					{
						theHeadset.buttons_locked = FALSE;
	                    MessageSend ( &theHeadset.task , EventPowerOff , 0) ;
					}
                    break;

				case headsetPoweringOn:
                case headsetHfpConnected:
                case headsetOutgoingCallEstablish:   
                case headsetIncomingCallEstablish:   
                case headsetActiveCall:            
                case headsetTestMode:
                    break ;
                default:
                    break ;
            }
        break;
    case EventResetComplete:
        EVENTS_DEBUG(("EventResetComplete\n"));
        break;
    case EventError:        
        EVENTS_DEBUG(("EventError\n")) ;
        break;
    case EventEnableLEDS:     
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventEnableLEDS\n")) ;
		
#ifdef ROM_LEDS		
        LedManagerEnableLEDS() ;        
#endif
		
        break ;
    case EventDisableLEDS:            
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventDisableLEDS\n")) ;
		
#ifdef ROM_LEDS			
        LedManagerDisableLEDS() ;            
#endif
		
        break ;
    case EventChargeError:
        EVENTS_DEBUG(("EventChargeError\n")) ;  
        break;
    case EventEndOfCall :        
        EVENTS_DEBUG(("EventEndOfCall\n")) ;
        break;    
	case EventEstablishA2dp:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventEstablishA2dp\n")) ;
		
		/* This is a manual connect */
		if (theHeadset.features.ManualConnectToDevices == Connect_List)
		{
			/* Connect to devices in list order */				
			if (!a2dpListConnection())
				lIndicateEvent = FALSE ;
		}
		else
		{
			/* Try connection to last A2DP device */
			if (!a2dpEstablishConnection(FALSE, TRUE))
        	{
            	/*do not perform the action*/
            	lIndicateEvent = FALSE ;
        	}	 
		}
		
        break;
	case EventA2dpConnected:
        EVENTS_DEBUG(("EventA2dpConnected\n")) ;		
        break;    
	case EventA2dpDisconnected:
        EVENTS_DEBUG(("EventA2dpDisconnected\n")) ;
		if (lState == headsetPoweringOn )
			lIndicateEvent = FALSE ;
        break; 
	case EventVolumeMax:
		EVENTS_DEBUG(("EventVolumeMax\n"));
		break;
	case EventVolumeMin:
		EVENTS_DEBUG(("EventVolumeMin\n"));
		break;
	case EventPlay:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventPlay\n"));
		/* Always indicate play event as will try to connect A2DP if not already connected */
		avrcpEventPlay();		
		break;
	case EventPause:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventPause\n"));
		avrcpEventPause();		
		break;
	case EventStop:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		EVENTS_DEBUG(("EventStop\n"));
		avrcpEventStop();		
		break;
	case EventSkipForward:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}		
		/* Only indicate event if AVRCP connected */
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventSkipForward\n"));
			avrcpEventSkipForward();
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;
	case EventSkipBackward:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		/* Only indicate event if AVRCP connected */
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventSkipBackward\n"));
			avrcpEventSkipBackward();	
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;
	case EventFFWDPress:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventFFWDPress\n"));
			avrcpEventFastForwardPress();
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;
	case EventFFWDRelease:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventFFWDRelease\n"));
			avrcpEventFastForwardRelease();
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;
	case EventRWDPress:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventRWDPress\n"));
			avrcpEventFastRewindPress();
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;
	case EventRWDRelease:		
		if (theHeadset.buttons_locked)
		{
			lIndicateEvent = FALSE ;
			break;
		}
		if ( stateManagerIsAvrcpConnected() )
		{
			EVENTS_DEBUG(("EventRWDRelease\n"));
			avrcpEventFastRewindRelease();
		}
		else
		{
			lIndicateEvent = FALSE ;
		}
		break;		
	case EventEnterDFUMode:
		EVENTS_DEBUG(("EventEnterDFUMode\n"));
		BootSetMode(0);
		break;		
	case EventToggleButtonLocking:
		EVENTS_DEBUG(("EventToggleButtonLocking\n"));
		if (theHeadset.buttons_locked)
		{
			theHeadset.buttons_locked = FALSE;
			MessageSend( &theHeadset.task , EventButtonLockingOff , 0 ) ;
		}
		else
		{
			theHeadset.buttons_locked = TRUE;	
			MessageSend( &theHeadset.task , EventButtonLockingOn , 0 ) ;
		}
		break;
	case EventButtonLockingOn:
		EVENTS_DEBUG(("EventButtonLockingOn\n"));
		break;
	case EventButtonLockingOff:
		EVENTS_DEBUG(("EventButtonLockingOff\n"));
		break;    
		
	case EventSwitchA2dpSource:
		EVENTS_DEBUG(("EventSwitchA2dpSource\n"));
		a2dpSwitchSource();
		break;
	case EventSwitchAudioMode:
		EVENTS_DEBUG(("EventSwitchAudioMode\n"));
		if (theHeadset.eqMode >= eq_mode_passthrough)
		{
			theHeadset.eqMode = eq_mode_level1;
		}
		else
		{
			theHeadset.eqMode++;	
		}
		if ((theHeadset.dsp_process == dsp_process_a2dp) && (theHeadset.seid != FASTSTREAM_SEID))
		{
			/* Set audio mode for A2DP only (not used for Faststream) */
			AudioSetMode(theHeadset.eqMode, 0);
		}
		break;
	case EventToggleLEDS:
		EVENTS_DEBUG(("EventToggleLEDS\n"));
		
#ifdef ROM_LEDS			
		LedManagerToggleLEDS();
#endif
		
		break;
		
	/* threeway calling events */
	case EventThreeWayReleaseAllHeld:
		EVENTS_DEBUG(("EventThreeWayReleaseAllHeld\n"));
        HfpMultipleCallsReleaseHeldOrRejectWaiting(theHeadset.hfp);
        break;
 	case EventThreeWayAcceptWaitingReleaseActive:
        EVENTS_DEBUG(("EventThreeWayAcceptWaitingReleaseActive\n"));
        HfpMultipleCallsReleaseActiveAcceptOther(theHeadset.hfp);
        break;
	case EventThreeWayAcceptWaitingHoldActive:
		EVENTS_DEBUG(("EventThreeWayAcceptWaitingHoldActive\n"));
		HfpMultipleCallsHoldActiveAcceptOther(theHeadset.hfp);
		break;
	case EventThreeWayAddHeldTo3Way:
 		EVENTS_DEBUG(("EventThreeWayAddHeldTo3Way\n"));
		HfpMultipleCallsAddHeldCall(theHeadset.hfp);
 		break;
	case EventThreeWayConnect2Disconnect:
 		EVENTS_DEBUG(("EventThreeWayConnect2Disconnect\n"));
		HfpMultipleCallsExplicitCallTransfer(theHeadset.hfp);
		break;
        /* end threeway calling events */
		
	case EventRssiPair:
        EVENTS_DEBUG(("EventRssiPair\n"));
		/* start inquiry on this event */
        inquiryStart();
        break;
    case EventRssiPairReminder:
        EVENTS_DEBUG(("EventRssiPairReminder\n"));
		inquiryReminder();
        break;
    case EventRssiPairTimeout:
        EVENTS_DEBUG(("EventRssiPairTimeout\n"));
		/* Stop any pending inquiry now as this pairing mode has timed out */
        inquiryStop();
        break;

    default:
        EVENTS_DEBUG(("UNHANDLED EVENT: 0x%x\n",id));
        lIndicateEvent = FALSE ;
        break;
    }    
    
    if ( lIndicateEvent )
    {
        LEDManagerIndicateEvent ( id ) ;
                 
        TonesPlayEvent ( id ) ;		
    }   
}
