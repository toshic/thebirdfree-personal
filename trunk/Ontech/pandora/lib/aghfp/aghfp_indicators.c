/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#include "aghfp.h"
#include "aghfp_common.h"
#include "aghfp_indicators.h"
#include "aghfp_ok.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"

#include <bdaddr.h>
#include <panic.h>
#include <string.h> /* For memcpy */
#include <stdio.h>


/****************************************************************************
	Sends the mapping between each indicator supported by the AG and its
	corresponding range and index.
*/
void aghfpHandleSendCindDetails(AGHFP *aghfp)
{
    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
    {
	    aghfpSendAtCmd(aghfp, "+CIND:"
		    "(\"service\",(0,1)),"      /*Index 1*/
		    "(\"call\",(0,1)),"         /*Index 2*/
		    "(\"callsetup\",(0-3)),"    /*Index 3*/
		    "(\"callheld\",(0-2)),"     /*Index 4*/
		    "(\"signal\",(0-5)),"       /*Index 5*/
		    "(\"roam\",(0,1)),"         /*Index 6*/
		    "(\"battchg\",(0-5))");     /*Index 7*/
    }
    else
    {
	    aghfpSendAtCmd(aghfp, "+CIND:"
		    "(\"service\",(0,1)),"      /*Index 1*/
		    "(\"call\",(0,1)),"         /*Index 2*/
		    "(\"callsetup\",(0-3))");   /*Index 3*/
	}
}


/****************************************************************************
	Sends the current status of the AG indicators
*/
void aghfpHandleSendCindStatus(AGHFP *aghfp)
{
	char buf[4] = "1,0";

	switch(aghfp->state)
	{
		case aghfp_slc_connecting:
		case aghfp_slc_connected:
		case aghfp_incoming_call_establish:
		case aghfp_outgoing_call_establish:
		case aghfp_active_call:
			aghfpAtCmdBegin(aghfp);
			aghfpAtCmdString(aghfp, "+CIND:");

			if (!aghfp->service_state) 				buf[0] = '0';
			if (aghfp->state == aghfp_active_call)	buf[2] = '1';

			aghfpAtCmdString(aghfp, buf);

		    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
		    {
				aghfpAtCmdString(aghfp, ",0,0,5,0,5");
			}
			else
			{
				aghfpAtCmdString(aghfp, ",0");
			}

			aghfpAtCmdEnd(aghfp);
			break;

		default:
			AGHFP_DEBUG_PANIC(("Received caller ID status request without an SLC"));
			break;
    }
}


/****************************************************************************
 The app has asked to send a service indicator to the HF - create an internal
 message that the profile handler will deal with.
*/
void AghfpSendServiceIndicator(AGHFP *aghfp, aghfp_service_availability availability)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_SERVICE_INDICATOR);
	message->availability = availability;
	MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_SERVICE_INDICATOR, message);
}


/****************************************************************************
 The app has asked to send a service indicator to the HF - the profile handler
 has instructed us to act on that request.
*/
void aghfpHandleSendServiceIndicator(AGHFP *aghfp, aghfp_service_availability availability)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:1,"); /*service indicator is index 1 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", availability);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_SERVICE_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a call indicator to the HF - create an internal
 message that the profile handler will deal with.
*/
void AghfpSendCallIndicator(AGHFP *aghfp, aghfp_call_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_CALL_INDICATOR);
    message->status = status;
    MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CALL_INDICATOR, message);
}


/****************************************************************************
*/
void aghfpSendCallIndicator(AGHFP *aghfp, aghfp_call_status status)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:2,"); /*call indicator is index 2 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", status);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);
}


/****************************************************************************
 The app has asked to send a call indicator to the HF - the profile handler
 has instructed us to act on that request.
*/
void aghfpHandleSendCallIndicator(AGHFP *aghfp, aghfp_call_status status)
{
	aghfpSendCallIndicator(aghfp, status);
	
	switch(status)
	{
		case aghfp_call_none:
			aghfpSetState(aghfp, aghfp_slc_connected);
			break;
		case aghfp_call_active:
			aghfpSetState(aghfp, aghfp_active_call);
			break;
	}

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a call setup indicator to the HF - create an
 internal message that the profile handler will deal with.
*/
void AghfpSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status type)
{
    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR);
    message->type = type;
    MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR, message);
}


/****************************************************************************
*/
void aghfpSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status type)
{
    char buf[2];

    aghfpAtCmdBegin(aghfp);
    aghfpAtCmdString(aghfp, "+CIEV:3,"); /*call setup indicator is index 3 (see aghfpHandleSendCindDetails)*/
    sprintf(buf, "%d", type);
    aghfpAtCmdString(aghfp, buf);
    aghfpAtCmdEnd(aghfp);
}


/****************************************************************************
 The app has asked to send a call setup indicator to the HF - the profile
 handler has instructed us to act on that request.
*/
void aghfpHandleSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status type)
{
	aghfpSendCallSetupIndicator(aghfp, type);

    /* Store the call setup status for our own use later on - we use it to verify
       that commands the HF sends are valid in the current state. */
    aghfp->call_setup_status = type;

    if (aghfp->state != aghfp_active_call)
    {
        switch (type)
        {
            case aghfp_call_setup_none:
                aghfpSetState(aghfp, aghfp_slc_connected);
                break;
            case aghfp_call_setup_incoming:
                aghfpSetState(aghfp, aghfp_incoming_call_establish);
                break;
            case aghfp_call_setup_outgoing:
                aghfpSetState(aghfp, aghfp_outgoing_call_establish);
                break;
            default:
                break;
         }
    }

    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_SETUP_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a call held indicator to the HF - create an
 internal message that the profile handler will deal with.
*/
void AghfpSendCallHeldIndicator(AGHFP *aghfp, aghfp_call_held_status status)
{
    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR);
        message->status = status;
        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR, message);
    }
    else
    {
    	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_HELD_INDICATOR_CFM, aghfp, aghfp_fail);
    }
}


/****************************************************************************
 The app has asked to send a call held indicator to the HF - the profile
 handler has instructed us to act on that request.
*/
void aghfpHandleSendCallHeldIndicator(AGHFP *aghfp, aghfp_call_held_status status)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:4,"); /*call held indicator is index 4 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", status);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_HELD_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a signal level indicator to the HF - create an
 internal message that the profile handler will deal with.
*/
void AghfpSendSignalIndicator(AGHFP *aghfp, uint16 level)
{
    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR);
        message->level = MIN(level,5);
        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR, message);
    }
    else
    {
    	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_SIGNAL_INDICATOR_CFM, aghfp, aghfp_fail);
    }
}


/****************************************************************************
 The app has asked to send a signal level indicator to the HF - the profile
 handler has instructed us to act on that request.
*/
void aghfpHandleSendSignalIndicator(AGHFP *aghfp, uint16 level)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:5,"); /*signal indicator is index 5 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", level);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_SIGNAL_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a roaming status indicator to the HF - create an
 internal message that the profile handler will deal with.
*/
void AghfpSendRoamIndicator(AGHFP *aghfp, aghfp_roam_status status)
{
    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_ROAM_INDICATOR);
        message->status = status;
        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_ROAM_INDICATOR, message);
    }
    else
    {
    	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_ROAM_INDICATOR_CFM, aghfp, aghfp_fail);
    }
}


/****************************************************************************
 The app has asked to send a roaming status indicator to the HF - the profile
 handler has instructed us to act on that request.
*/
void aghfpHandleSendRoamIndicator(AGHFP *aghfp, aghfp_roam_status status)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:6,"); /*roam indicator is index 6 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", status);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_ROAM_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked to send a battery charge indicator to the HF - create an
 internal message that the profile handler will deal with.
*/
void AghfpSendBattChgIndicator(AGHFP *aghfp, uint16 level)
{
    if ( supportedProfileIsHfp15(aghfp->supported_profile) )
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR);
        message->level = MIN(level,5);
        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR, message);
    }
    else
    {
    	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_BATT_CHG_INDICATOR_CFM, aghfp, aghfp_fail);
    }
}


/****************************************************************************
 The app has asked to send a battery charge indicator to the HF - the profile
 handler has instructed us to act on that request.
*/
void aghfpHandleSendBattChgIndicator(AGHFP *aghfp, uint16 level)
{
	char buf[2];

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CIEV:7,"); /*service indicator is index 7 (see aghfpHandleSendCindDetails)*/
	sprintf(buf, "%d", level);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_BATT_CHG_INDICATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
*/
void aghfpHandleCallerIdSetupReq(AGHFP *aghfp, bool enable)
{
	MAKE_AGHFP_MESSAGE(AGHFP_CALLER_ID_SETUP_IND);
	message->aghfp = aghfp;
	message->enable = enable;
	MessageSend(aghfp->client_task, AGHFP_CALLER_ID_SETUP_IND, message);
	
    aghfpSendOk(aghfp);
}


/****************************************************************************
*/
void aghfpHandleCallWaitingSetupReq(AGHFP *aghfp, bool enable)
{
	/* Send indicator to app */
	MAKE_AGHFP_MESSAGE(AGHFP_CALL_WAITING_SETUP_IND);
	message->aghfp = aghfp;
	message->enable = enable;
	MessageSend(aghfp->client_task, AGHFP_CALL_WAITING_SETUP_IND, message);

	/* Send confirm to HF */
	aghfpSendOk(aghfp);

	/* Update our own state */
	if (enable)
	{
		aghfp->features_status |= aghfp_feature_call_waiting_notification;
	}
	else
	{
		aghfp->features_status &= ~aghfp_feature_call_waiting_notification;
	}
}


/****************************************************************************
 App wants to send a call waiting notification to the HF. Pass the request on
 to the profile handler
*/
void AghfpSendCallWaitingNotification(AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string)
{
	bool error = FALSE;

    /* Is call waiting notification currently enabled? */
    if (aghfp->features_status & aghfp_feature_call_waiting_notification)
    {
        MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION, size_number);
        if (!message) error = TRUE;

        if (!error)
        {
            message->type = type_number;
			message->size_string = size_string;
			message->string = NULL;

			if (string && size_string > 0)
			{
				/* Copy string */
				message->string = malloc(size_string);
				if (message->string)
				{
					memcpy(message->string, string, size_string);
				}
				else
				{
					error = TRUE;
				}
			}

			/* Copy number */
	        memcpy(message->number, number, size_number);
	        message->size_number = size_number;
		}

		if (!error)
		{
	        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION, message);
	    }
	    else
	    {
			free(message->string);
			free(message);
		}
    }
    else
    {
		error = TRUE;
	}

    if (error)
    {
        aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM, aghfp, aghfp_fail);
    }
}


/****************************************************************************
 App wants to send a call waiting notification to the HF. Profile handler
 has asked us to act on this request.
*/
void aghfpHandleSendCallWaitingNotification(AGHFP *aghfp, uint8 type, uint16 size_number, uint8 *number, uint16 size_string, uint8 *string)
{
	char buf[4];

    aghfpAtCmdBegin(aghfp);
    aghfpAtCmdString(aghfp, "+CCWA: ");
    aghfpAtCmdData(aghfp, number, size_number);
    aghfpAtCmdString(aghfp, ",");
	sprintf(buf, "%d", type);
	aghfpAtCmdString(aghfp, buf);
    aghfpAtCmdString(aghfp, ",1");

    if (size_string > 0 && string)
    {
		aghfpAtCmdString(aghfp, ",");
		aghfpAtCmdData(aghfp, string, size_string);
		free(string);
	}

    aghfpAtCmdEnd(aghfp);

    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM, aghfp, aghfp_success);
}


/****************************************************************************
*/
void AghfpSetServiceState(AGHFP *aghfp, bool service_state)
{
	/* Send message to profile handler */
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SET_SERVICE_STATE);
	message->service_state = service_state;
	MessageSend(&aghfp->task, AGHFP_INTERNAL_SET_SERVICE_STATE, message);
}


/****************************************************************************
*/
void aghfpHandleSetServiceState(AGHFP *aghfp, bool service_state)
{
	if (service_state > 1)
	{
		aghfpSendCommonCfmMessageToApp(AGHFP_SET_SERVICE_STATE_CFM, aghfp, aghfp_fail);
	}
	else
	{
		aghfp->service_state = service_state;
		aghfpSendCommonCfmMessageToApp(AGHFP_SET_SERVICE_STATE_CFM, aghfp, aghfp_success);
	}
}
