/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_indicators_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"

#include <ctype.h>
#include <panic.h>
#include <string.h>
#include <util.h>


static __inline__ ptr matchChar(ptr p, ptr e, char ch)
{
	return p && p != e && ((int) *p) == ((int) ch) ? p+1 : 0;
}


/* create the internal message and send it */
static void sendHfpIndicatorListInd(Task profileTask, hfp_indicators *ind)
{
	HFP *hfp = (HFP *) profileTask;

	MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_INDICATOR_LIST_IND);
	message->indexes = *ind;
	MessageSend(&hfp->task, HFP_INTERNAL_AT_INDICATOR_LIST_IND, message);
}


static void sendIndicatorServiceToApp(HFP *hfp, uint16 service)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Send a message to the app */
	{
	MAKE_HFP_MESSAGE(HFP_SERVICE_IND);
	message->service = service;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SERVICE_IND, message);
	}
}


static void sendIndicatorCallToApp(HFP *hfp, uint16 call)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Update the local value */
	hfp->indicator_status.call = call;

	/* Update the profile state depending on the value of the indicator */
	if (hfp->indicator_status.call)
	{
		/* If indicator is set then we have to ahve an active call. */
		hfpSetState(hfp, hfpActiveCall);
	}
	else
	{
		/* Only reset the call state if we already have an active call. */
		if (hfp->state == hfpActiveCall)
		{
			hfpSetState(hfp, hfpSlcConnected);
		}
		else
		{
			/* This is necessary when dealing with 0.96 AGs */
			if (!hfp->indicator_status.indexes.call_setup && 
				((hfp->state == hfpIncomingCallEstablish) || (hfp->state == hfpOutgoingCallEstablish)) &&
				supportedProfileIsHfp(hfp->hfpSupportedProfile))
			{
				hfpSendIndicatorCallSetupToApp(hfp, hfp_no_call_setup);
			}
		}
	}

	/* Send a message to the app */
	{
		MAKE_HFP_MESSAGE(HFP_CALL_IND);
		message->call = hfp->indicator_status.call;
		message->hfp = hfp;
		MessageSend(hfp->clientTask, HFP_CALL_IND, message);
	}
}


static void sendIndicatorSignalToApp(HFP *hfp, uint16 signal)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Send a message to the app */
	{
	MAKE_HFP_MESSAGE(HFP_SIGNAL_IND);
	message->signal = signal;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SIGNAL_IND, message);
	}
}


static void sendIndicatorRoamToApp(HFP *hfp, uint16 roam)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Send a message to the app */
	{
	MAKE_HFP_MESSAGE(HFP_ROAM_IND);
	message->roam = roam;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_ROAM_IND, message);
	}
}


static void sendIndicatorBattChgToApp(HFP *hfp, uint16 batt_chg)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Send a message to the app */
	{
	MAKE_HFP_MESSAGE(HFP_BATTCHG_IND);
	message->battchg = batt_chg;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_BATTCHG_IND, message);
	}
}


static void sendIndicatorCallHeldToApp(HFP *hfp, uint16 call_held)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Send a message to the app */
	{
	MAKE_HFP_MESSAGE(HFP_CALLHELD_IND);
	message->callheld = call_held;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_CALLHELD_IND, message);
	}
}

/* Pull out indicator init state from struct passed inf rom parser */
static uint16 getIndicatorValue(const struct hfpHandleIndicatorStatus *s, uint16 index)
{
    if(1 <= index && index <= s->d.count)
    {
        struct value_hfpHandleIndicatorStatus_d t = get_hfpHandleIndicatorStatus_d(&s->d, index-1);
        return t.n;
    }    
    return 0;
}


/* Send the indicator list to the app so it can parse it for any non-HFP indicators it is interested in */
static void hfpSendUnhandledIndicatorListToApp(HFP *hfp, uint16 register_index, uint16 cind_index, uint16 min_range, uint16 max_range)
{
	MAKE_HFP_MESSAGE(HFP_EXTRA_INDICATOR_INDEX_IND);	
	message->hfp = hfp;
	message->indicator_register_index = register_index;
	message->indicator_index = cind_index;
	message->min_range = min_range;
	message->max_range = max_range;
	MessageSend(hfp->clientTask, HFP_EXTRA_INDICATOR_INDEX_IND, message);
}


/* Handle in here the AT cmds we want to ignore */
void hfpHandleNull(Task task, const struct hfpHandleNull *ignored)
{
	task = task;
	ignored = ignored;
}


/****************************************************************************
NAME	
	hfpSendIndicatorCallSetupToApp

DESCRIPTION
	Update hfp profile state and send call setup indication to the App

RETURNS
	void
*/
void hfpSendIndicatorCallSetupToApp(HFP *hfp, hfp_call_setup call_setup)
{
	checkHfpProfile(hfp->hfpSupportedProfile);
	
	/* Check the indicator value so we can decide what state we're in */
	switch (call_setup)
	{
	case hfp_no_call_setup:
		/* If we're not in an active call then we must be idle */
		if (!hfp->indicator_status.call)
		{
			if (hfp->state == hfpIncomingCallEstablish ||
				hfp->state == hfpOutgoingCallEstablish ||
				hfp->state == hfpOutgoingCallAlerting)
				hfpSetState(hfp, hfpSlcConnected);
		}
		else
		{
			/* If we are in an active call then we have just rejected an second call */
			hfpSetState(hfp, hfpActiveCall);
		}

        hfp->caller_id_name_sent = 0;
		break;

	case hfp_incoming_call_setup:
        if (!hfp->indicator_status.call)
		    /* Incoming call being established. */
		    hfpSetState(hfp, hfpIncomingCallEstablish);
		break;

	case hfp_outgoing_call_setup:
		/* Outgoing call is being established */
		hfpSetState(hfp, hfpOutgoingCallEstablish);
		break;

	case hfp_outgoing_call_alerting_setup:
		/* An outgoing call is being established and remote end is being alerted. */
		hfpSetState(hfp, hfpOutgoingCallAlerting);
		break;

	default:
		/* Should never get here */
		HFP_DEBUG(("Unknown call_setup value received 0x%x\n", call_setup));
	}

	/* Send a message to the app */
	{
		MAKE_HFP_MESSAGE(HFP_CALL_SETUP_IND);
		message->call_setup = call_setup;
		message->hfp = hfp;
		MessageSend(hfp->clientTask, HFP_CALL_SETUP_IND, message);
	}
}


/****************************************************************************
NAME	
	hfpHandleIndicatorListNokia

DESCRIPTION
	Nokia phone specific indicator list.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorListNokia(Task profileTask)
{
	/* 
		Result of dictionary look-up so values are hard coded.
		We don't have any unhandled indicators to send to the app.
	*/
	hfp_indicators ind_indexes = { 2, 1, 3, 0, 0, 0, 0 };
	sendHfpIndicatorListInd(profileTask, &ind_indexes);
}


/****************************************************************************
NAME	
	hfpHandleIndicatorListNokia60

DESCRIPTION
	Nokia phone (60 series) specific indicator list.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorListNokia60(Task profileTask)
{
	/* Result of dictionary look-up so values are hard coded */
	hfp_indicators ind_indexes = { 2, 1, 4, 0, 0, 0, 0 };
	sendHfpIndicatorListInd(profileTask, &ind_indexes);
}


/****************************************************************************
NAME	
	hfpHandleIndicatorListEricsson

DESCRIPTION
	Ericsson phone specific indicator list.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorListEricsson(Task profileTask)
{
	HFP *hfp = (HFP *) profileTask;

	/* Result of dictionary look-up so values are hard coded - call setup not supported */
	{
	    hfp_indicators ind_indexes = { 5, 8, 0, 0, 0, 0, 0 };
	    sendHfpIndicatorListInd(profileTask, &ind_indexes);
    }

	if (hfp->indicator_status.extra_indicators)
	{
		uint16 index = 0;
		uint16 i = 0;
		uint8 *end = (uint8 *) (hfp->indicator_status.extra_indicators + strlen((char*)hfp->indicator_status.extra_indicators));

		/* Look through the list of indicators passed in and send up a message for each match */
		while ((uint8 *) hfp->indicator_status.extra_indicators+i < end)
		{
			uint16 cind_index = 0;
			uint16 max = 1;

			/* Find if the indicator matches anything in the string */						
			if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "battchg", 7))
			{
				cind_index = 1;				
				max = 5;
			}
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "signal", 6))
			{
				cind_index = 2;
				max = 5;
			}
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "batterywarning", 14))
				cind_index = 3;
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "chargerconnected", 16))
				cind_index = 4;
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "sounder", 7))
				cind_index = 6;
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "message", 7))
				cind_index = 7;
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "roam", 4))
				cind_index = 9;
			else if (!UtilCompare((const uint16 *) hfp->indicator_status.extra_indicators+i, (const uint16 *) "smsfull", 7))
				cind_index = 10;

			/* Send the update to the app - if any */
			if (cind_index)
				hfpSendUnhandledIndicatorListToApp(hfp, index, cind_index, 0, max);

			/* Increment i to the end of the current string, look for the \r separator */
			while ((uint8 *) hfp->indicator_status.extra_indicators+i != end && *(hfp->indicator_status.extra_indicators+i) != '\r')
				i++;

			/* Skip past the \r */
			i++;

			/* Increment the index into the configuration array */
			index++;
		}

		/* Don't need to store this so free it */
		free((uint8 *) hfp->indicator_status.extra_indicators);
		hfp->indicator_status.extra_indicators = 0;
	}
}

/****************************************************************************
NAME	
	hfpHandleIndicatorListEricsson

DESCRIPTION
	Ericsson phone specific indicator list.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorListMotorola(Task profileTask)
{
	/* Result of dictionary look-up so values are hard coded */
	hfp_indicators ind_indexes = { 2, 3, 6, 0, 0, 0, 0 };
	sendHfpIndicatorListInd(profileTask, &ind_indexes);
}



/****************************************************************************
NAME	
	hfpHandleIndicatorList

DESCRIPTION
	Generic indicator list handler, if we don't get a dictionary look up
	match we'll end up in here so we need to parse the string manually as it
	is too complicated for the parser.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorList(Task profileTask, const struct hfpHandleIndicatorList *ind)
{
    hfp_indicators ind_indexes = { 0, 0, 0, 0, 0, 0, 0 };
    uint16 index      = 0;
    ptr	   p	      = ind->str.data;
	uint16 length     = ind->str.length;
	HFP *hfp		  = (HFP *) profileTask;
    
    ptr e = p + length;
    ptr n;

    p = hfpSkipSpace(p, e);

    while((n = matchChar(p, e, '(')) != 0)
    {
        index++;
        p = matchChar(hfpSkipSpace(n, e), e, '"');

        /* 
            Make sure the string matches but also make sure that 
            only that string matches e.g. for "call" we don't 
            want "call_tmp" to match.
        */
		if (!UtilCompare((const uint16 *) p, (const uint16 *) "service", 7))
        {
            ind_indexes.service = index;
        }
        else if (!UtilCompare((const uint16 *) p, (const uint16 *) "callsetup", 9))
        {
            ind_indexes.call_setup = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "call_setup", 10))
        {
            ind_indexes.call_setup = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "call\"", 5))
        {
            ind_indexes.call = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "signal", 6) && 
		         supportedProfileIsHfp15(hfp->hfpSupportedProfile))
        {   /* Only interpreted by a HFP v1.5 device */
            ind_indexes.signal_strength = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "roam", 4) && 
		         supportedProfileIsHfp15(hfp->hfpSupportedProfile))
        {   /* Only interpreted by a HFP v1.5 device */
            ind_indexes.roaming_status = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "battchg", 7) && 
		         supportedProfileIsHfp15(hfp->hfpSupportedProfile))
        {   /* Only interpreted by a HFP v1.5 device */
            ind_indexes.battery_charge = index;
        }
		else if (!UtilCompare((const uint16 *) p, (const uint16 *) "callheld", 8) && 
		         (hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_CONTROL) && 
		         supportedProfileIsHfp15(hfp->hfpSupportedProfile))
        {   /* Only interpreted by a HFP v1.5 device that supports the enhanced call control feature */
            ind_indexes.call_hold_status = index;
        }
  
		/* This is to work out the extra indicators we must tell the app about */
		if (hfp->indicator_status.extra_indicators)
		{
			uint16 indicator_length = 0;
			uint16 i = 0;
			uint16 register_index = 0;
			ptr temp;

			/* Get the end ptr for the string the app passed in. */
			uint8 *end = (uint8 *) (hfp->indicator_status.extra_indicators + strlen((char*)hfp->indicator_status.extra_indicators));

			/* Find the length of the indicator in the CIND string */
			indicator_length = (uint16)UtilFind(0xFFFF, '"', (const uint16*)p, 0, 1, length);
			indicator_length -= (uint16)p;
	/*		while (*(p+indicator_length) != '"')
				indicator_length++;
		*/				
			/* 
				For each indicator string in the config string the app passed in see 
				if the current indicaot matches that string 
			*/
			while ((uint8 *) hfp->indicator_status.extra_indicators+i < end)
			{
				uint16 cind_index = 0;
				uint16 min = 0;
				uint16 max = 0;

				/* Match the CIND string against the app config string */
				if (!UtilCompare((const uint16 *) p, (const uint16 *) hfp->indicator_status.extra_indicators+i, indicator_length))
				{
					/* We have a match, so set the cind_index. */
					cind_index = index;

					/* Get the min and max range */
					temp = hfpSkipPastChar(p, e, '(');
					temp = hfpSkipSpace(temp, e);
					while (temp != e && isdigit((int) *temp)) min = min * 10 + (*temp++ - '0');
					temp = hfpSkipSpace(temp, e);
					while (temp != e && (*temp==',' || *temp=='-')) temp++;
					temp = hfpSkipSpace(temp, e);
					while (temp != e && isdigit((int) *temp)) max = max * 10 + (*temp++ - '0');
				}

				/* Send the update to the app - if any */
				if (cind_index)
					hfpSendUnhandledIndicatorListToApp(hfp, register_index, cind_index, min, max);

				/* Increment i to the end of the current string, look for the \r separator */
				while ((uint8 *) hfp->indicator_status.extra_indicators+i != end && *(hfp->indicator_status.extra_indicators+i) != '\r')
					i++;

				/* Skip past the \r */
				i++;

				/* Increment the index into the configuration array */
				register_index++;
			}
		}

        /* skip , ( ... ) ) */
        p = hfpSkipPastChar(p, e, ')');
		p = hfpSkipPastChar(p, e, ')');

        /* Skip separating comma if present. */
        n = hfpSkipSpace(matchChar(p, e, ','), e);
        if(n) p = n;
    }
    
    /* To work with old AGs don't check callsetup indicator. */
    if (ind_indexes.service && ind_indexes.call)
    {
        sendHfpIndicatorListInd(profileTask, &ind_indexes );
    }

	/* Don't need to store this so free it */
	if (hfp->indicator_status.extra_indicators)
	{
		free((uint8 *) hfp->indicator_status.extra_indicators);
		hfp->indicator_status.extra_indicators = 0;
	}
}


/****************************************************************************
NAME	
	hfpHandleIndicatorListInd

DESCRIPTION
	Message containing the indicator index list. 

RETURNS
	void
*/
void hfpHandleIndicatorListInd(HFP *hfp, const HFP_INTERNAL_AT_INDICATOR_LIST_IND_T *ind)
{
	/* Update local indicator index values */
	hfp->indicator_status.indexes = ind->indexes;

	/* Proceed to the next step in the SLC establishment */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CIND_READ_REQ, 0);
}


/****************************************************************************
NAME	
	hfpHandleIndicatorStatus

DESCRIPTION
	Generic handler for initial indicator status update. Called when we don't 
	have a dictionary match.

AT INDICATION
	+CIND

RETURNS
	void
*/
void hfpHandleIndicatorStatus(Task profileTask, const struct hfpHandleIndicatorStatus *status)
{
	HFP *hfp = (HFP *) profileTask;	
    hfp_indicators* hfpIndIndexes = &((HFP *)profileTask)->indicator_status.indexes;
    
	/* Manually pull out each of the indicator states from the struct passed in */
	MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_INDICATOR_STATUS_IND);
	message->values.service = getIndicatorValue(status, hfpIndIndexes->service);
	message->values.call = getIndicatorValue(status, hfpIndIndexes->call);
	message->values.call_setup = getIndicatorValue(status, hfpIndIndexes->call_setup);
    message->values.signal_strength = getIndicatorValue(status, hfpIndIndexes->signal_strength);
	message->values.roaming_status = getIndicatorValue(status, hfpIndIndexes->roaming_status);
	message->values.battery_charge = getIndicatorValue(status, hfpIndIndexes->battery_charge);
	message->values.call_hold_status = getIndicatorValue(status, hfpIndIndexes->call_hold_status);
    
    MessageSend(&hfp->task, HFP_INTERNAL_AT_INDICATOR_STATUS_IND, message);
}


/****************************************************************************
NAME	
	hfpHandleIndicatorInitialStatusInd

DESCRIPTION
	Message containing initial indicator values.

RETURNS
	void
*/
void hfpHandleIndicatorInitialStatusInd(HFP *hfp, const HFP_INTERNAL_AT_INDICATOR_STATUS_IND_T *ind)
{
	/* Proceed to the next step in the SLC establishment */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CMER_REQ, 0);

	/* Send internal messages so we can take appropriate action for each indicator */
	sendIndicatorServiceToApp(hfp, ind->values.service);
	sendIndicatorCallToApp(hfp, ind->values.call);

	/* Send message only if AG supports this indicator i.e. index non zero */	
	if (hfp->indicator_status.indexes.call_setup)
	{
		hfpSendIndicatorCallSetupToApp(hfp, (hfp_call_setup) ind->values.call_setup);
	}

	/* Send message only if AG supports this indicator i.e. index non zero */	
	if (hfp->indicator_status.indexes.signal_strength)
	{
		sendIndicatorSignalToApp(hfp, (hfp_call_setup) ind->values.signal_strength);
	}
	
	/* Send message only if AG supports this indicator i.e. index non zero */	
	if (hfp->indicator_status.indexes.roaming_status)
	{
		sendIndicatorRoamToApp(hfp, (hfp_call_setup) ind->values.roaming_status);
	}
	
	/* Send message only if AG supports this indicator i.e. index non zero */	
	if (hfp->indicator_status.indexes.battery_charge)
	{
		sendIndicatorBattChgToApp(hfp, (hfp_call_setup) ind->values.battery_charge);
	}
	
	/* Send message only if AG supports this indicator i.e. index non zero */	
	if (hfp->indicator_status.indexes.call_hold_status)
	{
		sendIndicatorCallHeldToApp(hfp, (hfp_call_setup) ind->values.call_hold_status);
	}
}


/****************************************************************************
NAME	
	hfpHandleIndicatorStatusUpdate

DESCRIPTION
	Indicator status change message received from the AG.

AT INDICATION
	+CIEV

RETURNS
	void
*/
void hfpHandleIndicatorStatusUpdate(Task profileTask, const struct hfpHandleIndicatorStatusUpdate *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* Check which indicator we have received and send an internal message for it */
	if (ind->index == hfp->indicator_status.indexes.service)
	{
		sendIndicatorServiceToApp(hfp, ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.call)
	{
		sendIndicatorCallToApp(hfp, ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.call_setup)
	{
		hfpSendIndicatorCallSetupToApp(hfp, (hfp_call_setup) ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.signal_strength)
	{
		sendIndicatorSignalToApp(hfp, ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.roaming_status )
	{
		sendIndicatorRoamToApp(hfp, ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.battery_charge)
	{
		sendIndicatorBattChgToApp(hfp, ind->value);
	}
	else if (ind->index == hfp->indicator_status.indexes.call_hold_status)
	{
		sendIndicatorCallHeldToApp(hfp, ind->value);
	}
	else
	{
		if (hfp->indicator_status.extra_inds_enabled)
		{
			/*
				We have received an indicator not defined by the HFP spec. Pass it 
				up to the application as it might want to make use of these extra
				indicators that some AGs send out. Don't bother sending this through
				the internal state machine since we don't know what this indicator
				is and therefore cannot check if we have received it while in the
				corrcet state.
			*/
			MAKE_HFP_MESSAGE(HFP_EXTRA_INDICATOR_UPDATE_IND);
			message->index = ind->index;
			message->value = ind->value;
			message->hfp = hfp;
			MessageSend(hfp->clientTask, HFP_EXTRA_INDICATOR_UPDATE_IND, message);
		}
	}	
}
