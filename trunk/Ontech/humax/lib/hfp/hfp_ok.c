/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_ok.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_call_handler.h"
#include "hfp_caller_id_handler.h"
#include "hfp_dial_handler.h"
#include "hfp_dtmf_handler.h"
#include "hfp_hs_handler.h"
#include "hfp_multiple_calls_handler.h"
#include "hfp_nrec_handler.h"
#include "hfp_ok.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "hfp_sound_handler.h"
#include "hfp_voice_handler.h"
#include "hfp_voice_tag_handler.h"
#include "hfp_subscriber_num_handler.h"
#include "hfp_response_hold_handler.h"
#include "hfp_network_operator_handler.h"
#include "hfp_extended_error_handler.h"
#include "hfp_current_calls_handler.h"

#include "hfp_csr_features_handler.h"

#include <panic.h>
#include <string.h>


#define HFP_CME_MIN_ERROR_CODE 0
#define HFP_CME_MAX_ERROR_CODE 32

static const hfp_lib_status extendedStatusCode[ HFP_CME_MAX_ERROR_CODE - HFP_CME_MIN_ERROR_CODE + 1 ] =
{
    hfp_ag_failure,                     /* CME ERROR: 0  */
    hfp_no_connection_to_phone,         /* CME ERROR: 1  */
    hfp_fail,          
    hfp_operation_not_allowed,          /* CME ERROR: 3  */
    hfp_operation_not_supported,        /* CME ERROR: 4  */
    hfp_ph_sim_pin_required,            /* CME ERROR: 5  */
    hfp_fail,          
    hfp_fail,          
    hfp_fail,          
    hfp_fail,          
    hfp_sim_not_inserted,               /* CME ERROR: 10 */
    hfp_sim_pin_required,               /* CME ERROR: 11 */
    hfp_sim_puk_required,               /* CME ERROR: 12 */
    hfp_sim_failure,                    /* CME ERROR: 13 */
    hfp_sim_busy,                       /* CME ERROR: 14 */
    hfp_fail,          
    hfp_incorrect_password,             /* CME ERROR: 16 */
    hfp_sim_pin2_required,              /* CME ERROR: 17 */
    hfp_sim_puk2_required,              /* CME ERROR: 18 */
    hfp_fail,          
    hfp_memory_full,                    /* CME ERROR: 20 */
    hfp_invalid_index,                  /* CME ERROR: 21 */
    hfp_memory_failure,                 /* CME ERROR: 22 */
    hfp_text_string_too_long,           /* CME ERROR: 23 */
    hfp_invalid_chars_in_text_string,   /* CME ERROR: 24 */
    hfp_dial_string_too_long,           /* CME ERROR: 25 */
    hfp_invalid_chars_in_dial_string,   /* CME ERROR: 26 */
    hfp_fail,          
    hfp_fail,          
    hfp_fail,          
    hfp_no_network_service,             /* CME ERROR: 30 */
    hfp_fail,          
    hfp_network_not_allowed             /* CME ERROR: 32 */
};

/* Convert supplied CME ERROR code to internal HFP library status code. */
/* Unrecongnised codes are mapped to hfp_fail.                          */
static hfp_lib_status convertErrorCode ( uint16 CmeErrorCode )
{
    if ( (CmeErrorCode >= HFP_CME_MIN_ERROR_CODE) && (CmeErrorCode <= HFP_CME_MAX_ERROR_CODE) )
    {
        return extendedStatusCode[ CmeErrorCode ];
    }
    
    return hfp_fail;
}


/* Check which AT cmd was waiting for this response */
static void atCmdAckReceived(HFP *hfp, hfp_lib_status status)
{
	/* Cancel the AT response timeout message */
	(void) MessageCancelAll(&hfp->task, HFP_INTERNAL_WAIT_AT_TIMEOUT_IND);

	/* Which message was waiting for this? */
	switch (hfp->at_cmd_resp_pending)
	{
	case hfpCmdPending:
		/* This is a cmd which we don't particularly care about, as long as we get the ack */
		break;

	case hfpBrsfCmdPending:
		/* Have received response to AT+BRSF */
		hfpHandleBrsfAtAck(hfp, status);
		break;

	case hfpCmerCmdPending:
		/* Have received response to AT+CMER */
		hfpHandleCmerAtAck(hfp, status);
		break;

	case hfpCkpdCmdPending:
		/* Have received a response to AT+CKPD */
		hfpHandleHsCkpdAtAck(hfp, status);
		break;

	case hfpAtaCmdPending:
		/* Have received response to ATA */
		hfpHandleAtaAtAck(hfp, status);
		break;

	case hfpChupCmdPending:
		/* Have received response to AT+CHUP */
		hfpHandleChupAtAck(hfp, status);
		break;

	case hfpBldnCmdPending:
		/* Have received response to AT+BLDN */
		hfpHandleBldnAtAck(hfp, status);
		break;

	case hfpBvraCmdPending:
		/* Have received response to AT+BVRA */
		hfpHandleBvraAtAck(hfp, status);
		break;

	case hfpVgsCmdPending:
		/* Have received response to AT+VGS */
		hfpHandleVgsAtAck(hfp, status);
		break;

	case hfpVgmCmdPending:
		/* Have received response to AT+VGM */
		hfpHandleVgmAtAck(hfp, status);
		break;

	case hfpClipCmdPending:
		/* Have received response to AT+CLIP */
		hfpHandleClipAtAck(hfp, status);
		break;

	case hfpAtdNumberCmdPending:
		/* Have received response to ATD */
		hfpHandleAtdNumberAtAck(hfp, status);
		break;

	case hfpAtdMemoryCmdPending:
		/* Have received response to ATD> */
		hfpHandleAtdMemoryAtAck(hfp, status);
		break;

	case hfpBinpCmdPending:
		/* Have received response to AT+BINP */
		hfpHandleBinpAtAck(hfp, status);
		break;

	case hfpNrecCmdPending:
		/* Have received response to AT+NREC */
		hfpHandleNrecAtAck(hfp, status);
		break;

	case hfpVtsCmdPending:
		/* Have received response to AT+VTS */
		hfpHandleVtsAtAck(hfp, status);
		break;

	case hfpCcwaCmdPending:
		/* Have received response to AT+CCWA */
		hfpHandleCcwaAtAck(hfp, status);
		break;

	case hfpChldZeroCmdPending:
		/* Have received a response to AT+CHLD=0 */
		hfpHandleChldZeroAtAck(hfp, status);
		break;

	case hfpChldOneCmdPending:
		/* Have received a response to AT+CHLD=1 */
		hfpHandleChldOneAtAck(hfp, status);
		break;

	case hfpChldOneIdxCmdPending:
		/* Have received a response to AT+CHLD=1<idx> */
		hfpHandleChldOneAtAck(hfp, status);
	    break;
	    
	case hfpChldTwoCmdPending:
		/* Have received a response to AT+CHLD=2 */
		hfpHandleChldTwoAtAck(hfp, status);
		break;

	case hfpChldTwoIdxCmdPending:
		/* Have received a response to AT+CHLD=2<idx> */
		hfpHandleChldTwoAtAck(hfp, status);
	    break;
	    
	case hfpChldThreeCmdPending:
		/* Have received a response to AT+CHLD=3 */
		hfpHandleChldThreeAtAck(hfp, status);
		break;

	case hfpChldFourCmdPending:
		/* Have received a response to AT+CHLD=4 */
		hfpHandleChldFourAtAck(hfp, status);
		break;

	case hfpCnumCmdPending:
		/* Have received response to AT+CNUM */
		hfpHandleCnumAtAck(hfp, status);
	    break;
	    
	case hfpBtrhReqCmdPending:
		/* Have received response to AT+BTRH? */
		hfpHandleBtrhStatusAtAck(hfp, status);
	    break;
	    
	case hfpBtrhZeroCmdPending:
		/* Have received response to AT+BTRH=0 */
		hfpHandleBtrhHoldAtAck(hfp, status);
	    break;
	    
	case hfpBtrhOneCmdPending:
		/* Have received response to AT+BTRH=1 */
		hfpHandleBtrhAcceptAtAck(hfp, status);
	    break;
	    
	case hfpBtrhTwoCmdPending:
		/* Have received response to AT+BTRH=2 */
		hfpHandleBtrhRejectAtAck(hfp, status);
	    break;
	    
	case hfpCopsFormatCmdPending:
		/* Have received response to AT+COPS=3,0 */
		hfpHandleCopsFormatAtAck(hfp, status);
	    break;
	    
	case hfpCopsReqCmdPending:
		/* Have received response to AT+COPS? */
		hfpHandleCopsReqAtAck(hfp, status);
	    break;
	    
	case hfpCmeeCmdPending:
		/* Have received response to AT+CMEE */
		hfpHandleCmeeAtAck(hfp, status);
	    break;
	    
	case hfpClccCmdPending:
		/* Have received response to AT+CLCC */
		hfpHandleClccAtAck(hfp, status);
	    break;
		
	case hfpCsrSfPending:
		/* Have received response to AT+CSRSF */
		hfpHandleCsrSfAtAck(hfp, status);
		break;
	case hfpCsrPwrPending:
		/* Have received response to AT+CSRPWR */
		hfpHandleCsrPwrAtAck(hfp, status);
		break;
	case hfpCsrBatPending:
		/* Have received response to AT+CSRBATT */
		hfpHandleCsrBatAtAck(hfp, status);
		break;
	case hfpCsrModIndPending:
		/* Have received response to AT+CSRBATT */
		hfpHandleCsrModIndsReqAck(hfp, status);
		break;
	case hfpCsrGetSmsPending:
		/* Have received response to AT+CSRGETSMS */
		hfpHandleCsrGetSmsAck(hfp, status);
		break;
		
	case hfpNoCmdPending:
	default:
		/* This should not happen, if we get an ack we should be waiting for it! */
		break;
	}

	/* Reset the flag as we've just received a response */ 
	hfp->at_cmd_resp_pending = hfpNoCmdPending;

	/* Try to send the next AT cmd pending */
	hfpSendNextAtCmd(hfp, SinkClaim(hfp->sink, 0), SinkMap(hfp->sink));
}


/****************************************************************************
NAME	
	hfpHandleOk

DESCRIPTION
	Handle the AT OK indication sent by the AG.

AT INDICATION
	OK

RETURNS
	void
*/
void hfpHandleOk(Task profileTask)
{
	HFP *hfp = (HFP *) profileTask;    

	hfpHandleAtOk(hfp);
}


/****************************************************************************
NAME	
	hfpHandleError

DESCRIPTION
	Handle the AT ERROR indication sent by the AG.

AT INDICATION
	ERROR

RETURNS
	void
*/
void hfpHandleError(Task profileTask)
{
    HFP *hfp = (HFP *) profileTask;    

	hfpHandleAtError(hfp);
}


/****************************************************************************
NAME	
	hfpHandleExtendedError

DESCRIPTION
	Extended error result code indication sent by the AG

AT INDICATION
	+CME ERROR

RETURNS
	void
*/
void hfpHandleExtendedError(Task profileTask, const struct hfpHandleExtendedError *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* Silently ignore extended error message if we are not HFP v.15 */
	if (supportedProfileIsHfp15(hfp->hfpSupportedProfile))
	{
		/* Handle this response in the cotext of the AT cmd it relates to */
	    atCmdAckReceived(hfp, convertErrorCode(ind->err));
	}
}


/****************************************************************************
NAME	
	hfpHandleNoCarrierInd

DESCRIPTION
	Handle the AT NO CARRIER indication sent by the AG.

AT INDICATION
	NO CARRIER

RETURNS
	void
*/
void hfpHandleNoCarrierInd(Task profileTask)
{
	/* For the moment just ignore this */
	profileTask = profileTask;
}


/****************************************************************************
NAME	
	handleUnrecognised

DESCRIPTION
	Called when we receive data that cannot be recognised by the AT cmd 
	parser. To stop us from panicking if the AG is spamming us with 
	unrecognised data e.g. some out of spec AG is sending us the wrong
	commands, we only send the unrecognised data to the app if we have the
	resources, otherwise we silently ignore it!

RETURNS
	void
*/
void handleUnrecognised(const uint8 *data, uint16 length, Task task)
{
	HFP *hfp = (HFP *) task;

	if (!length)
		return;

	/* 
		Create a message and send it directly to the app. 
		No point going through the state machine because we don't 
		know what this data is anyway.
	*/
	{
		HFP_UNRECOGNISED_AT_CMD_IND_T *message = (HFP_UNRECOGNISED_AT_CMD_IND_T *) malloc(sizeof(HFP_UNRECOGNISED_AT_CMD_IND_T) + length);
		
		if (message)
		{
			message->hfp = hfp;
			message->size_data = length;
			memmove(message->data, data, length);
			MessageSend(hfp->clientTask, HFP_UNRECOGNISED_AT_CMD_IND, message);	
		}
		/* If we didn't alloc the mesage don't panic, just ignore this we only send these up if we have spare resources */
	}
}


/****************************************************************************
NAME	
	hfpHandleAtOk

DESCRIPTION
	An OK response received from the AG for the AT cmd just sent.

RETURNS
	void
*/
void hfpHandleAtOk(HFP *hfp)
{
	/* Handle this response in the cotext of the AT cmd it relates to */
	atCmdAckReceived(hfp, hfp_success);
}


/****************************************************************************
NAME	
	hfpHandleAtError

DESCRIPTION
	An ERROR response received from the AG for the AT cmd just sent.

RETURNS
	void
*/
void hfpHandleAtError(HFP *hfp)
{
	/* Handle this response in the cotext of the AT cmd it relates to */
	atCmdAckReceived(hfp, hfp_fail);
}
