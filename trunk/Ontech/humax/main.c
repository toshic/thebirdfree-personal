/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    main.c
@brief    Main entry point for the application.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_msg_handler.h"
#include "headset_a2dp_stream_control.h"
#include "headset_avrcp_event_handler.h"
#include "headset_avrcp_msg_handler.h"
#include "headset_cl_msg_handler.h"
#include "headset_codec_msg_handler.h"
#include "headset_debug.h"
#include "headset_event_handler.h"
#include "headset_events.h"
#include "headset_hfp_call.h"
#include "headset_hfp_msg_handler.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_inquiry.h"
#include "headset_led_manager.h"
#include "headset_private.h"
#include "headset_statemanager.h"
#include "at_cmd.h"

#include <a2dp.h>
#include <audio.h>
#include <avrcp.h>
#include <boot.h>
#include <charger.h>
#include <codec.h>
#include <connection.h>
#include <hfp.h>
#include <kalimba_standard_messages.h>
#include <memory.h>
#include <panic.h>
#include <pio.h>
#include <stdlib.h>
#include <test.h>

#ifdef SD_SUPPORT
#include "headset_sd.h"
#endif

#ifdef TEST_HARNESS
#include "test_bc5_stereo.h"
#include "vm2host_connection.h"
#include "vm2host_hfp.h"
#include "vm2host_avrcp.h"
#include "vm2host_a2dp.h"
#endif

#ifdef DEBUG_MAIN
#define MAIN_DEBUG(x) DEBUG(x)
#else
#define MAIN_DEBUG(x) 
#endif


#define TX_START_TEST_MODE_LO_FREQ  (2441)
#define TX_START_TEST_MODE_LEVEL    (63)
#define TX_START_TEST_MODE_MOD_FREQ (0)


/* Single instance of the Headset state */
hsTaskData theHeadset;


/****************************************************************************
  FUNCTIONS
*/


/* Handle any application messages */
static void handleAppMessage( Task task, MessageId id, Message message )
{
	switch (id)
    {        
    case APP_RESUME_A2DP:
        MAIN_DEBUG(("APP_RESUME_A2DP\n"));		
        streamControlBeginA2dpStreaming();
        break;
	case APP_AVRCP_CONTROLS:
		MAIN_DEBUG(("APP_AVRCP_CONTROLS\n"));
		avrcpEventHandleControls((APP_AVRCP_CONTROLS_T*)message);
		break;
	case APP_AVRCP_CONNECT_REQ:
		MAIN_DEBUG(("APP_AVRCP_CONNECT_REQ\n"));
		handleAVRCPConnectReq((APP_AVRCP_CONNECT_REQ_T*)message);
		break;
	case APP_SEND_PLAY:
		MAIN_DEBUG(("APP_SEND_PLAY\n"));
		if (theHeadset.features.autoSendAvrcp)
			avrcpSendPlay();
		break;
	case APP_ENABLE_POWER_OFF:
		MAIN_DEBUG(("APP_ENABLE_POWER_OFF\n"));
		theHeadset.PowerOffIsEnabled = TRUE ;
		break;
	case APP_LIMBO_TIMEOUT:
        MAIN_DEBUG(("APP_LIMBO_TIMEOUT\n"));
        stateManagerUpdateLimboState();
        break;
	case APP_CHECK_FOR_AUDIO_TRANSFER:
		MAIN_DEBUG(("APP_CHECK_FOR_AUDIO_TRANSFER\n"));
		headsetCheckForAudioTransfer() ;
		break;
	case APP_CONNECT_HFP_LINK_LOSS:
		MAIN_DEBUG(("APP_CONNECT_HFP_LINK_LOSS\n"));
		break;
	case APP_CONNECT_A2DP_LINK_LOSS:
		MAIN_DEBUG(("APP_CONNECT_A2DP_LINK_LOSS\n"));
		break;
	case APP_EVENT_REFRESH_ENCRYPTION:
		MAIN_DEBUG(("APP_EVENT_REFRESH_ENCRYPTION\n"));
		/* Refresh encryption on HFP link if audio not active */
		if (HfpGetSlcSink(theHeadset.hfp_hsp))
		{
			if (!HfpGetAudioSink(theHeadset.hfp_hsp))
			{
				MAIN_DEBUG(("HFP Key Refreshed\n"));
				/* Refresh the encryption key */
				ConnectionSmEncryptionKeyRefreshSink(HfpGetSlcSink(theHeadset.hfp_hsp));
			}
		}
		/* Refresh encryption on A2DP link if audio not active */
		if (A2dpGetSignallingSink(theHeadset.a2dp))
		{
			if (!A2dpGetMediaSink(theHeadset.a2dp))
			{
				MAIN_DEBUG(("A2DP Key Refreshed\n"));
				/* Refresh the encryption key */
				ConnectionSmEncryptionKeyRefreshSink(A2dpGetSignallingSink(theHeadset.a2dp));
			}
		}
		MessageSendLater(&theHeadset.task, APP_EVENT_REFRESH_ENCRYPTION, 0, D_MIN(theHeadset.Timeouts.EncryptionRefreshTimeout_m));
		break;
	case APP_CANCEL_HSP_INCOMING_CALL:
		MAIN_DEBUG(("APP_CANCEL_HSP_INCOMING_CALL\n"));
		/* Clear the incoming call flag */
		theHeadset.HSPIncomingCallInd = FALSE;
		/* if the ring indication has timed out and the state is still incoming 
			call establish, return to connected state else the headset will 
			remain in incoming call state if the call is answered or rejected
 			on the AG */
		if ( stateManagerGetHfpState() == headsetIncomingCallEstablish)
		{
			MAIN_DEBUG(("HSP ring with no connect, return to connected\n")) ; 
			stateManagerEnterHfpConnectedState() ;    
		}
		break;
	case APP_CONNECT_A2DP:
		MAIN_DEBUG(("APP_CONNECT_A2DP\n"));
		if (!A2dpGetSignallingSink(theHeadset.a2dp))
		{
		    bdaddr bd_addr;
			a2dpConnectBdaddrRequest(&bd_addr,FALSE);
		}
		break;
	case APP_TX_TEST_MODE:
		MAIN_DEBUG(("APP_TX_TEST_MODE\n"));
		TestTxStart (TX_START_TEST_MODE_LO_FREQ, 
                 TX_START_TEST_MODE_LEVEL, 
                 TX_START_TEST_MODE_MOD_FREQ) ;
		break;
	case APP_INQUIRY_CONTINUE:
		MAIN_DEBUG(("APP_INQUIRY_CONTINUE\n"));
		/* continue with inquiry */
		inquiryContinue();
		break;
		
#ifdef SD_SUPPORT
        case APP_SD_EVENT:
                /* send event to SD player */
                sd_button_handler(((APP_SD_EVENT_T*)message)->event);
                break;
#endif
	default:
		MAIN_DEBUG(("APP UNHANDLED MSG: 0x%x\n",id));
		break;
	}
}


/* Handle any audio plugin messages */
static void handleAudioPluginMessage( Task task, MessageId id, Message message )
{
	switch (id)
    {        
		case AUDIO_PLUGIN_DSP_MSG:
			/* Make sure this is the clock mismatch rate, sent from the DSP via the a2dp decoder common plugin */
			if (((AUDIO_PLUGIN_DSP_MSG_T*)message)->id == KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE)
			{
				if (stateManagerIsA2dpConnected())
				{
					theHeadset.clock_mismatch_rate = ((AUDIO_PLUGIN_DSP_MSG_T*)message)->value;
				}
			}
			break;
	}	
}

/*************************************************************************
NAME    
    app_handler
    
DESCRIPTION
    This is the main message handler for the Headset Application.  All
    messages pass through this handler to the subsequent handlers.

RETURNS

*/
static void app_handler(Task task, MessageId id, Message message)
{
    /* Determine the message type based on base and offset */
    if ( ( id >= EVENTS_EVENT_BASE ) && ( id <= EVENTS_LAST_EVENT ) )
    {
        /* Message is a User Generated Event */
        handleUEMessage(task, id,  message);
    }
    else if ( (id >= CL_MESSAGE_BASE) && (id <= CL_MESSAGE_TOP) )
    {
        handleCLMessage(task, id,  message);
    #ifdef TEST_HARNESS
        vm2host_connection(task, id, message);
    #endif
    }
    else if ( (id >= CODEC_MESSAGE_BASE ) && (id <= CODEC_MESSAGE_TOP) )
    {     
        handleCodecMessage(task, id,  message);
    }
    else if ( (id >= HFP_MESSAGE_BASE ) && (id <= HFP_MESSAGE_TOP) )
    {     
        handleHFPMessage(task, id,  message);
    #ifdef TEST_HARNESS
        vm2host_hfp(task, id, message);
    #endif
    }      
    else if ( (id >= A2DP_MESSAGE_BASE ) && (id <= A2DP_MESSAGE_TOP) )
    {     
        handleA2DPMessage(task, id,  message);
	#ifdef TEST_HARNESS
        vm2host_a2dp(task, id, message);
    #endif
    }      
    else if ( (id >= AVRCP_MESSAGE_BASE ) && (id <= AVRCP_MESSAGE_TOP) )
    {     
        handleAVRCPMessage(task, id,  message);
    #ifdef TEST_HARNESS
        vm2host_avrcp(task, id, message);
    #endif
    }      
    else if( id == MESSAGE_MORE_DATA )
    {
        Source source = ((MessageMoreData *)message)->source;
        if(source == StreamUartSource())
            parseUart(source,task);
    }
	else if ( (id >= HEADSET_MSG_BASE ) && (id <= HEADSET_MSG_TOP) )
    {     
        handleAppMessage(task, id,  message);
    }  
	else if ( (id >= AUDIO_PLUGIN_MESSAGE_BASE) && (id <= AUDIO_PLUGIN_MESSAGE_TOP) )
	{
		handleAudioPluginMessage(task, id,  message);
	}
    else /* This message is not one of the above */
    {
        /* Pass this message to default handler */
        MAIN_DEBUG(("MSGTYPE ? [%x]\n", id)) ;
    }
}


int main(void)
{
#ifdef SD_SUPPORT
    uint16 bootmode = 0;
    uint16 pios = 0;
#endif
    
    MAIN_DEBUG(("Main entered\n")); 

    PROFILE_MEMORY(("InitStart"))
    PROFILE_TIME(("InitStart"))
    
    /* initialise our headset state */
    memset(&theHeadset, 0, sizeof(hsTaskData));

    /* Set up the Application task handler */
    theHeadset.task.handler = app_handler;
    
    /* Initialise the data contained in the hsTaskData structure */
    InitHeadsetData();

    /* Initialise the Codec Library */
    InitCodec();

#ifdef TEST_HARNESS
    test_init();
#endif
    MessageSinkTask(StreamUartSink(),&theHeadset.task);

    /* Start the message scheduler loop */
    MessageLoop();
    
    /* Never get here...*/
    return 0;
}
