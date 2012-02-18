/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    bluestack_handler.c        

DESCRIPTION
    Handles primitives received by BlueStack and routes them to the 
    appropriate handler function depending on their type.

NOTES

*/



/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "init.h"
#include "bluestack_handler.h"
#include "dm_baseband_handler.h"
#include "dm_dut_handler.h"
#include "dm_info_handler.h"
#include "dm_inquiry_handler.h"
#include "dm_sync_handler.h"
#include "dm_security_handler.h"
#include "l2cap_handler.h"
#include "rfc_handler.h"
#include "sdp_handler.h"
#include "dm_link_policy_handler.h"
#include "dm_acl_handler.h"


#include <app/bluestack/types.h>
#include <app/bluestack/dm_prim.h>
#include <app/bluestack/l2cap_prim.h>
#include <app/bluestack/rfcomm_prim.h>
#include <app/bluestack/sdc_prim.h>
#include <app/bluestack/sds_prim.h>

#include <print.h>
#include <vm.h>



/*lint -e655 -e525 -e830 */

/* Connection state management */
#define    SET_CM_STATE(s)  theCm->state=s;


/* List of reason code for default handling of unexpected messages */
typedef enum
{
    connectionUnexpectedCmPrim,
    connectionUnexpectedDmPrim,
    connectionUnexpectedRfcPrim,
	connectionUnexpectedSdpPrim,
	connectionUnexpectedL2capPrim,
    connectionUnhandledMessage
}connectionUnexpectedReasonCode;

/****************************************************************************
NAME	
    handleUnexpected	

DESCRIPTION
    This function is called as a result of a message arriving when the
    Connection Library was not expecting it.

RETURNS
    void	
*/
static void handleUnexpected(connectionUnexpectedReasonCode code, connectionStates state, uint16 type)
{
	UNUSED(state);
	UNUSED(type);

    switch(code)
    {
        case connectionUnexpectedCmPrim:
        case connectionUnexpectedDmPrim:
        case connectionUnexpectedRfcPrim:
		case connectionUnexpectedSdpPrim:
		case connectionUnexpectedL2capPrim:
		case connectionUnhandledMessage:
        default:
            CL_DEBUG(("handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
    }
}


/****************************************************************************
NAME	
    connectionBluestackHandler	

DESCRIPTION
    This is the main task handler for all messages sent to the Connection
    Library task.

RETURNS
    void	
*/
void connectionBluestackHandler(Task task, MessageId id, Message message)
{
    /* Get access to the Connection Library instance state */
    connectionState *theCm = (connectionState*)task;
	connectionStates libState = theCm->state;
    
	PRINT(("connectionBluestackHandler - Id = 0x%x\n",id));
	
    /* Depending upon the message id...*/
    switch(id)
    {
	    case CL_INTERNAL_INIT_REQ:
		    PRINT(("CL_INTERNAL_INIT_REQ\n"));

            /* Request to initialise the Connection Library */
		    switch(libState)
		    {
				case connectionInitialising:
		        case connectionReady:
		        case connectionTestMode:
                default:
			        goto cm_prim_error;
			        break;

		        case connectionUninitialised:
			        /* Move to the Initialising state */
			        SET_CM_STATE(connectionInitialising);
			        /* Start Initialisation process */
			        connectionHandleInternalInit(connectionInit);
			        /* Start a Timer to notify the Client if the initialisation fails */
			        MessageSendLater(&theCm->task, CL_INTERNAL_INIT_TIMEOUT_IND, NO_PAYLOAD, (uint32) INIT_TIMEOUT);
			        break;
		    }
		    break;

        case CL_INTERNAL_INIT_CFM:
            PRINT(("CL_INTERNAL_INIT_CFM\n"));

            /* A Connection Library entity has completed it's initialisation process */
            switch(libState)
            {
                case connectionUninitialised:
			    case connectionReady:
                case connectionTestMode:
                default:
				    goto cm_prim_error;
				    break;

			    case connectionInitialising:
				    /* Check to see if all objects have been initialised yet */
				    connectionHandleInternalInit(((CL_INTERNAL_INIT_CFM_T*)message)->mask);
				    /* If we're ready to run, change state */
				    if(theCm->initMask == connectionInitComplete)  
                    {
					    SET_CM_STATE(connectionReady);
                    }
				    break;
            }
            break;
			
		case CL_INTERNAL_INIT_TIMEOUT_IND:
            PRINT(("CL_INTERNAL_INIT_TIMEOUT_IND\n"));

            /* The initialisation of Bluestack has timed out.  Let
			the Client Application know */
		    (void) MessageCancelFirst(&theCm->task, CL_INTERNAL_INIT_CFM);
			SET_CM_STATE(connectionUninitialised);
			connectionSendInitCfm(theCm->theAppTask, fail, bluetooth_unknown);
			break;

		case CL_INTERNAL_DM_INQUIRY_REQ:
            PRINT(("CL_INTERNAL_DM_INQUIRY_REQ\n"));

            /* Request to start an Inquiry */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Kick off an inquiry */
					connectionHandleInquiryStart(&theCm->inqState, (CL_INTERNAL_DM_INQUIRY_REQ_T *)message);
					break;
			}
			break;

		case CL_INTERNAL_DM_INQUIRY_CANCEL_REQ:
			PRINT(("CL_INTERNAL_DM_INQUIRY_CANCEL_REQ\n"));

            /* Request to cancel Inquiry */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Cancel the inquiry attempt */
					connectionHandleInquiryCancel(&theCm->inqState, (CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T *)message);
					break;
			}
			break;
			
		case CL_INTERNAL_DM_READ_REMOTE_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_READ_REMOTE_NAME_REQ\n"));

            /* Request to read the remote name */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Read the remote name */
					connectionHandleReadRemoteName(&theCm->inqState, (CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T *)message);
					break;
			}
			break;
			
		case CL_INTERNAL_DM_READ_LOCAL_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LOCAL_NAME_REQ\n"));

            /* Request to read the local name */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Read the remote name */
					connectionHandleReadLocalName(&theCm->inqState, (CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T *)message);
					break;
			}
			break;
			
		case CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ\n"));

			/* Request to change the local name */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Attempt to change the local name */
					connectionHandleChangeLocalName((CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T *)message);
					break;
			}
			break;
			
		case CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ\n"));

            /* Write inquiry Tx */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Read the remote name */
					connectionHandleWriteInquiryTx(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T *)message);
					break;
			}
			break;
			
		case CL_INTERNAL_DM_READ_INQUIRY_TX_REQ:
			PRINT(("CL_INTERNAL_DM_READ_INQUIRY_TX_REQ\n"));

            /* Read inquiry Tx */
			switch(libState)
			{				
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Read the remote name */
					connectionHandleReadInquiryTx(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T *)message);
					break;
			}
			break;

		case CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ:
			PRINT(("CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ\n"));

			/* Request to read the local oob data */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadLocalOobDataReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T*)message);
					break;
			}
			break;  

		case CL_INTERNAL_SM_AUTHENTICATION_REQ:
			PRINT(("CL_INTERNAL_SM_AUTHENTICATION_REQ\n"));

			/* Request to initiate Authentication to a remote device */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleAuthenticationReq(&theCm->smState, (CL_INTERNAL_SM_AUTHENTICATION_REQ_T*)message);
					break;
			}
			break;     
			
		case CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ:
			PRINT(("CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ\n"));

			/* Request to cancel Authentication to a remote device */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleCancelAuthenticationReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T*)message);
					break;
			}
			break;  

		case CL_INTERNAL_DM_WRITE_EIR_DATA_REQ:
            PRINT(("CL_INTERNAL_DM_WRITE_EIR_DATA_REQ\n"));
		/* Write the Extended Inquiry Response data */
		switch(libState)
		{
			case connectionUninitialised:
			case connectionInitialising:
			case connectionTestMode:
			default:
				goto cm_prim_error;
				break;

			case connectionReady:
				connectionHandleWriteEirDataRequest(&theCm->infoState, (CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T *)message);
				break;
		}
		break;

		case CL_INTERNAL_DM_READ_EIR_DATA_REQ:
            PRINT(("CL_INTERNAL_DM_READ_EIR_DATA_REQ\n"));
		/* Read the Extended Inquiry Response data */
		switch(libState)
		{
			case connectionUninitialised:
			case connectionInitialising:
			case connectionTestMode:
			default:
				goto cm_prim_error;
				break;

			case connectionReady:
				connectionHandleReadEirDataRequest(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_READ_EIR_DATA_REQ_T *)message);
				break;
		}
		break;

		case CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND\n"));

			/* Indicates that Authentication has timed out */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					/* Notify application that Authentication has timeout out */
					connectionHandleAuthenticationTimeout(&theCm->smState);
				    break;
			}
			break;

        case CL_INTERNAL_SM_SET_SC_MODE_REQ:
            PRINT(("CL_INTERNAL_SM_SET_SC_MODE_REQ\n"));

            /* Request to set the current security mode */
            switch(libState)
			{
                case connectionUninitialised:
                default:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                case connectionInitialising:
				case connectionTestMode:
                    handleSetSecurityModeReq(&theCm->smState, (CL_INTERNAL_SM_SET_SC_MODE_REQ_T*)message);
				    break;
			    
			}
			break;

		case CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ:
            PRINT(("CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ\n"));

            /* Request to set the current security mode */
            switch(libState)
			{
                case connectionUninitialised:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                case connectionInitialising:
				case connectionTestMode:
					handleSetSspSecurityLevelReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T*)message);
				    break;
			}
			break;

		case CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ:
            PRINT(("CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ\n"));

            /* Request to set the current security mode */
            switch(libState)
			{
                case connectionUninitialised:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                case connectionInitialising:
				case connectionTestMode:
                    handleSecModeConfigReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T*)message);
				    break;
			}
			break;
			
        case CL_INTERNAL_SM_REGISTER_REQ:
            PRINT(("CL_INTERNAL_SM_REGISTER_REQ\n"));

            /* Request to register an incoming service with Bluestack */
            switch(libState)
			{
                case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                    handleRegisterReq((CL_INTERNAL_SM_REGISTER_REQ_T*)message);
				    break;
			}
			break;

        case CL_INTERNAL_SM_UNREGISTER_REQ:
            PRINT(("CL_INTERNAL_SM_UNREGISTER_REQ\n"));

            /* Request to unregister an incoming service with Bluestack */
            switch(libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                    handleUnRegisterReq((CL_INTERNAL_SM_UNREGISTER_REQ_T*)message);
				    break;
			}
			break;

        case CL_INTERNAL_SM_REGISTER_OUTGOING_REQ:
            PRINT(("CL_INTERNAL_SM_REGISTER_OUTGOING_REQ\n"));

            /* Request to register an outgoing service with Bluestack */
            switch(libState)
			{
                case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                    handleRegisterOutgoingReq((CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T*)message);
				    break;
			}
			break;

        case CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ:
            PRINT(("CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ\n"));

            /* Request to unregister an outgoing service with Bluestack */
            switch(libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break; 

                case connectionReady:
                    handleUnRegisterOutgoingReq((CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T*)message);
				    break;
			}
			break;

        case CL_INTERNAL_SM_ENCRYPT_REQ:
            PRINT(("CL_INTERNAL_SM_ENCRYPT_REQ\n"));

            /* Request to encrypt a link */
            switch(libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                    handleEncryptReq(&theCm->smState, (CL_INTERNAL_SM_ENCRYPT_REQ_T *)message);
				    break;

			}
			break;
			
		case CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ:
            PRINT(("CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ\n"));

            /* Request to encrypt a link */
            switch(libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                    goto cm_prim_error;
                    break;

                case connectionReady:
                    handleEncryptionKeyRefreshReq(&theCm->infoState, (CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T *)message);
				    break;

			}
			break;

        case CL_INTERNAL_SM_PIN_REQUEST_RES:
            PRINT(("CL_INTERNAL_SM_PIN_REQUEST_RES\n"));

            /* Response to a request for a pin code from the application */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handlePinRequestRes((CL_INTERNAL_SM_PIN_REQUEST_RES_T*)message);
				    break;
			    }
			break;

		case CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES\n"));

            /* Response to a request for IO capability from the application */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleIoCapabilityRequestRes(&theCm->smState, (CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T*)message);
				    break;
			    }
			break;
			
		case CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES\n"));
			/* Response to a request for user ssp confirmation from the application */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleUserConfirmationRequestRes((CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T*)message);
				    break;
			    }
			break;
			
		case CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES\n"));
			/* Response to a request for user ssp passkey entry from the application */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleUserPasskeyRequestRes((CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T*)message);
				    break;
			    }
			break;
			
		case CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ:
			PRINT(("CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ\n"));
			/* Request to bluestack to notify remote dev of keypress during passkey entry */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleSendKeypressNotificationReq((CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T*)message);
				    break;
			    }
			break;

		case CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ:
			PRINT(("CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ\n"));
			/* Request to bluestack to update a devices trust level */
            switch(libState)
			{ 
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleSetTrustLevelReq(&theCm->infoState, (CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T*)message);
				    break;
			    }
			break;
			
        case CL_INTERNAL_SM_AUTHORISE_RES:
            PRINT(("CL_INTERNAL_SM_AUTHORISE_RES\n"));

            /* Response to a request for authorisation for an incoming connection */
            switch(libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

			    case connectionReady:
                    handleAuthoriseRes((CL_INTERNAL_SM_AUTHORISE_RES_T*)message);
				    break; 
			}
			break;

		case CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ:
            PRINT(("CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ\n"));

			/* Request to add authorised device */
			switch (libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

				case connectionReady:
                    handleAddAuthDeviceReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *)message);
				    break; 
			}
			break;

		case CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ:
            PRINT(("CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ\n"));

			/* Request to add authorised device */
			switch (libState)
			{
			    case connectionUninitialised:
			    case connectionInitialising:
                case connectionTestMode:
                default:
                    goto cm_prim_error;
                    break;

				case connectionReady:
                    handleGetAuthDeviceReq(&theCm->smState, (CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T *)message);
				    break; 
			}
			break;

		case CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ:
			PRINT(("CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ\n"));
			
			/* Request to read class of device */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadClassOfDeviceRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ\n"));
			
			/* Requests a write page scan activity */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWritePageScanActivityRequset((CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ\n"));

			/* Requests a write inquiry scan activity */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteInquiryScanActivityRequest((CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ\n"));

			/* Requests a write scan enable */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
				case connectionTestMode:
					connectionHandleWriteScanEnableRequest((CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ\n"));

			/* Requests a write scan enable */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteCodRequset((CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ\n"));

            /* Requests a write cached page mode */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteCachedPageModeRequest((CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ\n"));

            /* Requests a write cached clock offset */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteCachedClkOffsetRequest((CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ:
			PRINT(("CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ\n"));

            /* Request to clear cached parameter cache */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleClearParamCacheRequest((CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ\n"));

            /* Request to set the flush timeout for an ACL */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteFlushTimeoutRequest((CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_IAC_LAP_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_IAC_LAP_REQ\n"));

			/* Request to set the inquiry scan access codes */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteIacLapRequest(&theCm->inqState, (CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ\n"));

			/* Request to set the inquiry mode */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleWriteInquiryModeRequest(&theCm->inqState, (CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T *)message);
					break; 
			}
			break;
			
		case CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ\n"));

			/* Request to read the inquiry mode */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadInquiryModeRequest(&theCm->inqState, (CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T *)message);
					break; 
			}
			break;
			
		case CL_INTERNAL_DM_READ_BD_ADDR_REQ:
			PRINT(("CL_INTERNAL_DM_READ_BD_ADDR_REQ\n"));

            /* Request to read the local Bluetooth device address */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadAddrRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_BD_ADDR_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_READ_LINK_QUALITY_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LINK_QUALITY_REQ\n"));

			/* Request the link quality on the link */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadLinkQualityRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_READ_RSSI_REQ:
			PRINT(("CL_INTERNAL_DM_READ_RSSI_REQ\n"));

			/* Request the rssi on the link */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadRssiRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_RSSI_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_READ_CLK_OFFSET_REQ:
			PRINT(("CL_INTERNAL_DM_READ_CLK_OFFSET_REQ\n"));

			/* Read the clock offset of the device */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadclkOffsetRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ:
			PRINT(("CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ\n"));

			/* Read the remote supported features of a device */
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleReadRemoteSupportedFeaturesRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T *) message);
					break; 
			}
			break;
			
		case CL_INTERNAL_DM_EN_ACL_DETACH_REQ:
			switch(libState)
			{
				case connectionInitialising:
				case connectionUninitialised:
                default:
					goto dm_prim_error;
					break;

                case connectionTestMode:
				case connectionReady:
					connectionHandleDmEnAclDetachReq((CL_INTERNAL_DM_EN_ACL_DETACH_REQ_T*)message);
					break;
			}
			break;
			

		case CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ:
 			PRINT(("CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ\n"));
 
 			/* Read the remote supported features of a device */
 			switch(libState)
 			{
				case connectionUninitialised:
				default:
					goto cm_prim_error;
 					break;
 				
 				case connectionInitialising:
				case connectionTestMode:
 				case connectionReady:
 					connectionHandleReadLocalVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *) message);
 					break; 
 			}
 			break;

		case CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ:
 			PRINT(("CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ\n"));
 
 			/* Read the remote supported features of a device */
 			switch(libState)
 			{
 				case connectionUninitialised:
 				case connectionInitialising:
 				case connectionTestMode:
				default:
 					goto dm_prim_error;
 					break;
 
 				case connectionReady:
 					connectionHandleReadRemoteVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T *) message);
 					break; 
 			}
 			break;

			
		case CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ:
			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSmChangeLinkKeyReq((CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T*) message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_REGISTER_RECORD_REQ:
			PRINT(("CL_INTERNAL_SDP_REGISTER_RECORD_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpRegisterRequest(&theCm->sdpState, (CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ:
			PRINT(("CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpUnregisterRequest(&theCm->sdpState, (CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ:
			PRINT(("CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpServerConfigMtu(&theCm->sdpState, (CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ:
			PRINT(("CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpClientConfigMtu(&theCm->sdpState, (CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_OPEN_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_OPEN_SEARCH_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpOpenSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_CLOSE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_CLOSE_SEARCH_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpCloseSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_SERVICE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_SERVICE_SEARCH_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpServiceSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpAttributeSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpServiceSearchAttrRequest(&theCm->sdpState, (CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ:
			PRINT(("CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSdpTerminatePrimitiveRequest(&theCm->sdpState, (CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_REGISTER_REQ:
			PRINT(("CL_INTERNAL_L2CAP_REGISTER_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capRegisterReq(&theCm->l2capState, (CL_INTERNAL_L2CAP_REGISTER_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_UNREGISTER_REQ:
			PRINT(("CL_INTERNAL_L2CAP_UNREGISTER_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capUnregisterReq((CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_CONNECT_REQ:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capConnectReq((CL_INTERNAL_L2CAP_CONNECT_REQ_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_CONNECT_RES:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_RES\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capConnectRes((CL_INTERNAL_L2CAP_CONNECT_RES_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_DISCONNECT_REQ:
			PRINT(("CL_INTERNAL_L2CAP_DISCONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capDisconnectReq((CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capConnectTimeout((CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND_T *)message);
					break; 
			}
			break;

        case CL_INTERNAL_RFCOMM_REGISTER_REQ:
            PRINT(("CL_INTERNAL_RFCOMM_REGISTER_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommRegisterReq(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_REGISTER_REQ_T *)message);
					break; 
			}
			break;


        case CL_INTERNAL_RFCOMM_CONNECT_REQ:
            PRINT(("CL_INTERNAL_RFCOMM_CONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommConnectReq(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_CONNECT_REQ_T *)message);
					break; 
			}
			break;


        case CL_INTERNAL_RFCOMM_CONNECT_RES:
            PRINT(("CL_INTERNAL_RFCOMM_CONNECT_RES\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommConnectRes((CL_INTERNAL_RFCOMM_CONNECT_RES_T *)message);
					break; 
			}
			break;

            
        case CL_INTERNAL_RFCOMM_DISCONNECT_REQ:
            PRINT(("CL_INTERNAL_RFCOMM_DISCONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommDisconnectReq((CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T *)message);
					break; 
			}
			break;


        case CL_INTERNAL_RFCOMM_CONTROL_REQ:
            PRINT(("CL_INTERNAL_RFCOMM_CONTROL_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommControlReq((CL_INTERNAL_RFCOMM_CONTROL_REQ_T *)message);
					break; 
			}
			break;


        case CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommRegisterTimeout((CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND_T *)message);
					break; 
			}
			break;

        case CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleRfcommConnectTimeout(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND_T *)message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_REGISTER_REQ:
			PRINT(("CL_INTERNAL_SYNC_REGISTER_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncRegisterReq((CL_INTERNAL_SYNC_REGISTER_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_UNREGISTER_REQ:
			PRINT(("CL_INTERNAL_SYNC_UNREGISTER_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncUnregisterReq((CL_INTERNAL_SYNC_UNREGISTER_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_CONNECT_REQ:
			PRINT(("CL_INTERNAL_SYNC_CONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncConnectReq((CL_INTERNAL_SYNC_CONNECT_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_CONNECT_RES:
			PRINT(("CL_INTERNAL_SYNC_CONNECT_RES\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncConnectRes((CL_INTERNAL_SYNC_CONNECT_RES_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_DISCONNECT_REQ:
			PRINT(("CL_INTERNAL_SYNC_DISCONNECT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncDisconnectReq((CL_INTERNAL_SYNC_DISCONNECT_REQ_T *) message);
					break; 
			}
			break;
            
		case CL_INTERNAL_SYNC_RENEGOTIATE_REQ:
			PRINT(("CL_INTERNAL_SYNC_RENEGOTIATE_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncRenegotiateReq((CL_INTERNAL_SYNC_RENEGOTIATE_REQ_T *) message);
					break; 
			}
			break;
            
		case CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncRegisterTimeoutInd((CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSyncUnregisterTimeoutInd((CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND_T *) message);
					break; 
			}
			break;
						
		case CL_INTERNAL_DM_SET_ROLE_REQ:
			PRINT(("CL_INTERNAL_DM_SET_ROLE_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleLinkPolicySetRoleReq(&theCm->linkPolicyState, (CL_INTERNAL_DM_SET_ROLE_REQ_T *) message);
					break; 
			}
            break;
			
		case CL_INTERNAL_DM_GET_ROLE_REQ:
			PRINT(("CL_INTERNAL_DM_GET_ROLE_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleLinkPolicyGetRoleReq(&theCm->linkPolicyState, (CL_INTERNAL_DM_GET_ROLE_REQ_T *) message);
					break; 
			}
            break;

		case CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ:
			PRINT(("CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSetLinkSupervisionTimeoutReq((CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T *) message);
					break; 
			}
            break;

		case CL_INTERNAL_DM_SET_LINK_POLICY_REQ:
			PRINT(("CL_INTERNAL_DM_SET_LINK_POLICY_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSetLinkPolicyReq((CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T *) message);
					break; 
			}
            break;
			
		case CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ:
			PRINT(("CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSetSniffSubRatePolicyReq(&theCm->infoState, (CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T *) message);
					break; 
			}
            break;
            
        case CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ:
            PRINT(("CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ\n"));
            
            switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleSetRoleSwitchParamsReq((CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ_T *) message);
					break; 
			}
            break;

		case CL_INTERNAL_DM_DUT_REQ:
			PRINT(("CL_INTERNAL_DM_DUT_REQ\n"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
                    /* Not allowed to enter DUT mode from this state */
					connectionSendDutCfmToClient(theCm->theAppTask, fail);
					break;

				case connectionReady:
					/* Update the local state */
					SET_CM_STATE(connectionTestMode);

					/* Enter test mode */
					connectionHandleEnterDutModeReq(&theCm->infoState);
					break; 
			}
			break;	
							
		case CL_INTERNAL_DM_SET_BT_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_SET_BT_VERSION_REQ\n"));
			switch(libState)
			{
				case connectionUninitialised:
					goto cm_prim_error;
					break;

                case connectionInitialising:
				case connectionTestMode:
				case connectionReady:
					connectionHandleSetBtVersionReq(&theCm->infoState, (CL_INTERNAL_DM_SET_BT_VERSION_REQ_T*)message);
					break;
			}
			break;
			
		case CL_INTERNAL_SM_GET_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SM_GET_ATTRIBUTE_REQ"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionSmHandleGetAttributeReq(theCm->theAppTask, (CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T *) message);
					break; 
			}
			break;

		case CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionSmHandleGetIndexedAttributeReq(theCm->theAppTask, (CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T *) message);
					break; 
			}
			break;

        case CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP:
			PRINT(("CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP"));

			switch(libState)
			{
				case connectionUninitialised:
				case connectionInitialising:
				case connectionTestMode:
                default:
					goto cm_prim_error;
					break;

				case connectionReady:
					connectionHandleL2capInterlockDisconnectRsp((CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP_T*)message);
					break; 
			}
			break;

	default:    
    switch(id)
    {    
		case MESSAGE_BLUESTACK_DM_PRIM:
			/* Bluestack Device Manager primitives */
			switch(((DM_UPRIM_T*)message)->type)
			{
				case DM_AM_REGISTER_CFM:
					PRINT(("DM_AM_REGISTER_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionReady:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionInitialising:
							connectionSendInternalInitCfm(connectionInitDm);
							break;
					}
					break;

				case DM_ACL_OPENED_IND:
					PRINT(("DM_ACL_OPENED_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							/* Inform the client an ACL has been opened. */
							connectionHandleDmAclOpenInd(theCm->theAppTask, (DM_ACL_OPENED_IND_T *) message);
							break;
					}
					break;

                case DM_EN_ACL_OPENED_IND:
                    PRINT(("DM_EN_ACL_OPENED_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							/* Inform the client an ACL has been opened. */
							connectionHandleDmEnAclOpenInd(theCm->theAppTask, &theCm->smState, (DM_EN_ACL_OPENED_IND_T *) message);
							break;
					}
                    break;

                case DM_ACL_OPEN_CFM:
					PRINT(("DM_ACL_OPEN_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							connectionHandleDmAclOpenCfm(&theCm->smState, (DM_ACL_OPEN_CFM_T*)message);
							break;
					}
					break;
										
				case DM_ACL_CLOSED_IND:
					PRINT(("DM_ACL_CLOSED_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							connectionHandleDmAclClosedInd(theCm->theAppTask, &theCm->smState, (DM_ACL_CLOSED_IND_T*)message);
							break;
					}
					break;

                case DM_EN_ACL_CLOSED_IND:
                    PRINT(("DM_EN_ACL_CLOSED_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							connectionHandleDmEnAclClosedInd(theCm->theAppTask, &theCm->smState, (DM_EN_ACL_CLOSED_IND_T *)message);
							break;
					}
					break;

				case DM_HCI_READ_REMOTE_FEATURES_COMPLETE:
					PRINT(("DM_HCI_READ_REMOTE_FEATURES_COMPLETE\n"));

					switch(libState)
					{
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleReadRemoteSupportedFeaturesCfm(&theCm->infoState, (DM_HCI_READ_REMOTE_FEATURES_COMPLETE_T *) message);
							break;
					}
					break;
					
				case DM_HCI_READ_LOCAL_VERSION_COMPLETE:
					PRINT(("DM_HCI_READ_LOCAL_VERSION_COMPLETE_T\n"));

					switch(libState)
					{
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

						case connectionInitialising:
						case connectionTestMode:
						case connectionReady:
							connectionHandleReadLocalVersionCfm(&theCm->infoState, (DM_HCI_READ_LOCAL_VERSION_COMPLETE_T *) message);
							break;
					}
					break;
					
				case DM_HCI_READ_REMOTE_VERSION_COMPLETE:
					PRINT(("DM_HCI_READ_REMOTE_VERSION_COMPLETE_T\n"));

					switch(libState)
					{
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleReadRemoteVersionCfm(&theCm->infoState, (DM_HCI_READ_REMOTE_VERSION_COMPLETE_T *) message);
							break;
					}
					break;
					
				case DM_SM_ACCESS_IND:
					PRINT(("DM_SM_ACCESS_IND\n"));
					switch(libState)
					{
						case connectionUninitialised:
						case connectionTestMode:
						default:
							goto dm_prim_error;
							break;

						case connectionInitialising:							
						case connectionReady:
							connectionHandleSmAccessInd(&theCm->sdpState, (DM_SM_ACCESS_IND_T*)message);
							break;
					}
					break;
					
				case DM_SM_ADD_DEVICE_CFM:
				case DM_SM_SSP_ADD_DEVICE_CFM:
					PRINT(("DM_SM_SSP_ADD_DEVICE_CFM\n"));


					switch(libState)
					{
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionInitialising:
							connectionHandleSmAddDeviceCfm(&theCm->smState, (DM_SM_ADD_DEVICE_CFM_T*)message);
							break;

						case connectionReady:
							/* CFM here is a result of changing trust level or when adding a trusted device */
							connectionHandleSmAddDeviceCfmReady(&theCm->smState, (DM_SM_ADD_DEVICE_CFM_T*)message);
							break;
					}
					break;

				case DM_SM_LINK_KEY_REQUEST_IND:
					PRINT(("DM_SM_LINK_KEY_REQUEST_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmLinkKeyReqInd((DM_SM_LINK_KEY_REQUEST_IND_T*)message);
							break;
					}
					break;

				case DM_SM_SSP_LINK_KEY_REQUEST_IND:
					PRINT(("DM_SM_SSP_LINK_KEY_REQUEST_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmSspLinkKeyReqInd(theCm->theAppTask, &theCm->smState, (DM_SM_SSP_LINK_KEY_REQUEST_IND_T*)message);
							break;
					}
					break;

				case DM_SM_LINK_KEY_IND:
					PRINT(("DM_SM_LINK_KEY_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmLinkKeyInd(theCm->theAppTask, &theCm->infoState, &theCm->smState, (DM_SM_LINK_KEY_IND_T*)message);
							break;
					}
					break;
												
				case DM_SM_PIN_REQUEST_IND:
					PRINT(("DM_SM_PIN_REQUEST_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmPinReqInd(theCm->theAppTask, (DM_SM_PIN_REQUEST_IND_T*)message);
							break;
					}
					break;

				case DM_SM_IO_CAPABILITY_REQUEST_IND:
					PRINT(("DM_SM_IO_CAPABILITY_REQUEST_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmIoCapReqInd(theCm->theAppTask, (DM_SM_IO_CAPABILITY_REQUEST_IND_T*)message);
							break;
					}
					break;
					
				
				case DM_SM_IO_CAPABILITY_RESPONSE_IND:
					PRINT(("DM_SM_IO_CAPABILITY_RESPONSE_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmIoCapResInd(theCm->theAppTask, &theCm->smState, (DM_SM_IO_CAPABILITY_RESPONSE_IND_T*)message);
							break;
					}
					break;
					
				
				case DM_SM_USER_CONFIRMATION_REQUEST_IND:
					PRINT(("DM_SM_USER_CONFIRMATION_REQUEST_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmUserConfirmationReqInd(theCm->theAppTask, (DM_SM_USER_CONFIRMATION_REQUEST_IND_T*)message);
							break;
					}
					break;
				
				
				case DM_SM_USER_PASSKEY_REQUEST_IND:
					PRINT(("DM_SM_USER_PASSKEY_REQUEST_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmUserPasskeyReqInd(theCm->theAppTask, (DM_SM_USER_PASSKEY_REQUEST_IND_T*)message);
							break;
					}
					break;
					
				
				case DM_SM_USER_PASSKEY_NOTIFICATION_IND:
					PRINT(("DM_SM_USER_PASSKEY_NOTIFICATION_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmUserPasskeyNotificationInd(theCm->theAppTask, &theCm->smState, (DM_SM_USER_PASSKEY_NOTIFICATION_IND_T*)message);
							break;
					}
					break;
					

				case DM_SM_KEYPRESS_NOTIFICATION_IND:
					PRINT(("DM_SM_KEYPRESS_NOTIFICATION_IND \n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmKeypressNotificationInd(theCm->theAppTask, (DM_SM_KEYPRESS_NOTIFICATION_IND_T*)message);
							break;
					}
					break;

				case DM_SM_AUTHORISE_IND:
					PRINT(("DM_SM_AUTHORISE_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmAuthoriseInd(theCm->theAppTask, (DM_SM_AUTHORISE_IND_T*)message);
							break;
					}
					break;

				case DM_SM_AUTHENTICATE_CFM:
					PRINT(("DM_SM_AUTHENTICATE_CFM\n"));
					break;

				case DM_SM_SIMPLE_PAIRING_COMPLETE_IND:
					PRINT(("DM_SM_SIMPLE_PAIRING_COMPLETE_IND\n"));
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSmSimplePairingCompleteInd(theCm->theAppTask, &theCm->smState, (DM_SM_SIMPLE_PAIRING_COMPLETE_IND_T*)message);
							break;
					}
					break;

				case DM_SM_SET_SEC_MODE_CFM:
					PRINT(("DM_SM_SET_SEC_MODE_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;
        
						case connectionTestMode:
							connectionHandleDutSecurityDisabled(theCm->theAppTask, (DM_SM_SET_SEC_MODE_CFM_T *)message);
							break;

                        case connectionInitialising:
						case connectionReady:
							connectionHandleSetSecurityModeCfm(&theCm->smState, (DM_SM_SET_SEC_MODE_CFM_T *)message);
							break;
					}
					break;

				case DM_SM_SEC_MODE_CONFIG_CFM:	
					PRINT(("DM_SM_SEC_MODE_CONFIG_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

                        case connectionInitialising:
						case connectionReady:
							connectionHandleConfigureSecurityCfm(&theCm->smState, (DM_SM_SEC_MODE_CONFIG_CFM_T *)message);
							break;
					}
					break;
					
				case DM_SM_ENCRYPTION_CHANGE:
					PRINT(("DM_SM_ENCRYPTION_CHANGE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;	

						case connectionReady:
							connectionHandleEncryptionChange((DM_SM_ENCRYPTION_CHANGE_T *)message);
							break;				
					}
					break;

				case DM_SM_READ_LOCAL_OOB_DATA_CFM:
					PRINT(("DM_SM_READ_LOCAL_OOB_DATA_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleReadLocalOobDataCfm(&theCm->smState, (DM_SM_READ_LOCAL_OOB_DATA_CFM_T*)message);
							break;
					}
					break;

				case DM_HCI_INQUIRY_RESULT:
					PRINT(("DM_HCI_INQUIRY_RESULT\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;	

						case connectionReady:
							connectionHandleInquiryResult(&theCm->inqState, (DM_HCI_INQUIRY_RESULT_T *)message);
							break;				
					}
					break;
																
				case DM_HCI_INQUIRY_RESULT_WITH_RSSI:
					PRINT(("DM_HCI_INQUIRY_RESULT_WITH_RSSI\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;	

						case connectionReady:
							connectionHandleInquiryResultWithRssi(&theCm->inqState, (DM_HCI_INQUIRY_RESULT_WITH_RSSI_T *)message);
							break;				
					}
					break;
							
				case DM_HCI_EXTENDED_INQUIRY_RESULT_IND:
					PRINT(("DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T\n"));
					
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;	

						case connectionReady:
							connectionHandleExtendedInquiryResult(&theCm->inqState, (DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T *)message);
							break;				
					}
					break;
					
				case DM_HCI_INQUIRY_COMPLETE:
					PRINT(("DM_HCI_INQUIRY_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the inquiry complete event */
							connectionHandleInquiryComplete(&theCm->inqState);
							break;
					}
					break;

				case DM_HCI_INQUIRY_CANCEL_COMPLETE:
					PRINT(("DM_HCI_INQUIRY_CANCEL_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the inquiry cancel complete event */
							connectionHandleInquiryComplete(&theCm->inqState);
							break;
					}
					break;

				case DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE:
					PRINT(("DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the write iac lap complete event */
							connectionHandleWriteIacLapComplete(&theCm->inqState, (DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_T *)message);
							break;
					}
					break;

				case DM_HCI_WRITE_INQUIRY_MODE_COMPLETE:
					PRINT(("DM_HCI_WRITE_INQUIRY_MODE_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the write inquiry mode complete event */
							connectionHandleWriteInquiryModeComplete(&theCm->inqState, (DM_HCI_WRITE_INQUIRY_MODE_COMPLETE_T *)message);
							break;
					}					
					break;
					
				case DM_HCI_READ_INQUIRY_MODE_COMPLETE:
					PRINT(("DM_HCI_READ_INQUIRY_MODE_COMPLETE\n"));
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read inquiry mode complete event */
							connectionHandleReadInquiryModeComplete(&theCm->inqState, (DM_HCI_READ_INQUIRY_MODE_COMPLETE_T *)message);
							break;
					}					
					break;
					
				case DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE:
					PRINT(("DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE\n"));
					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read inquiry mode complete event */
							connectionHandleReadEirDataComplete(&theCm->inqState, (DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE_T *)message);
							break;
					}					
					break;
					
				case DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE:
					PRINT(("DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read dev class complete event */
							connectionHandleReadClassOfDeviceComplete(&theCm->infoState, (DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE_T *)message);
							break;
					}
					break;

				case DM_HCI_READ_BD_ADDR_COMPLETE:
					PRINT(("DM_HCI_READ_BD_ADDR_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read bd addr complete event */
							connectionHandleReadBdAddrComplete(&theCm->infoState, (DM_HCI_READ_BD_ADDR_COMPLETE_T *)message);
							break;
					}
					break;

				case DM_HCI_GET_LINK_QUALITY_COMPLETE:
					PRINT(("DM_HCI_GET_LINK_QUALITY_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read link quality complete event */
							connectionHandleReadLinkQualityComplete(&theCm->infoState, (DM_HCI_GET_LINK_QUALITY_COMPLETE_T *) message);
							break;
					}
					break;

				case DM_HCI_READ_RSSI_COMPLETE:
					PRINT(("DM_HCI_READ_RSSI_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read RSSI complete event */
							connectionHandleReadRssiComplete(&theCm->infoState, (DM_HCI_READ_RSSI_COMPLETE_T *) message);
							break;
					}
					break;

				case DM_HCI_READ_CLOCK_OFFSET_COMPLETE:
					PRINT(("DM_HCI_READ_CLOCK_OFFSET_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							/* Handle the read clock offset complete event */
							connectionHandleReadClkOffsetComplete(&theCm->infoState, (DM_HCI_READ_CLOCK_OFFSET_COMPLETE_T *) message);
							break;
					}
					break;	
					
				case DM_SET_BT_VERSION_CFM:
					PRINT(("DM_SET_BT_VERSION_CFM\n"));
					switch(libState)
					{
						case connectionUninitialised:
							goto dm_prim_error;
							break;

                        case connectionInitialising:
						case connectionTestMode:
						case connectionReady:
							connectionHandleSetBtVersionCfm(&theCm->infoState, (DM_SET_BT_VERSION_CFM_T*)message);	
							break;
					}
					break;		

				case DM_LP_WRITE_POWERSTATES_CFM:
					PRINT(("DM_LP_WRITE_POWERSTATES_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionLinkPolicyHandleWritePowerStatesCfm((DM_LP_WRITE_POWERSTATES_CFM_T *)message);
							break;
					}
					break;
					
				case DM_HCI_SNIFF_SUB_RATING_IND:
					PRINT(("DM_HCI_SNIFF_SUB_RATING_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleSniffSubRatingInd((DM_HCI_SNIFF_SUB_RATING_IND_T *)message);
							break;
					}
					break;
					
				case DM_HCI_LINK_SUPERV_TIMEOUT_IND:
					PRINT(("DM_HCI_LINK_SUPERV_TIMEOUT_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleLinkSupervisionTimeoutInd((DM_HCI_LINK_SUPERV_TIMEOUT_IND_T *)message);
							break;
					}
					break;
					
				case DM_SYNC_REGISTER_CFM:
				    PRINT(("DM_SYNC_REGISTER_CFM\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
        				    connectionHandleSyncRegisterCfm( (DM_SYNC_REGISTER_CFM_T *)message );
							break;
				    }
				    break;
				    
				case DM_SYNC_UNREGISTER_CFM:
				    PRINT(("DM_SYNC_UNREGISTER_CFM\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
        				    connectionHandleSyncRegisterCfm( (DM_SYNC_REGISTER_CFM_T *)message );
							break;
				    }
				    break;
				    
				case DM_EX_SYNC_CONNECT_CFM:
				    PRINT(("DM_EX_SYNC_CONNECT_CFM\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncConnectCfm(theCm->theAppTask, (DM_EX_SYNC_CONNECT_CFM_T *) message);
							break;
				    }
				    break;
				    
				case DM_EX_SYNC_CONNECT_COMPLETE_IND:
				    PRINT(("DM_EX_SYNC_CONNECT_COMPLETE_IND\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncConnectCompleteInd(theCm->theAppTask, (DM_EX_SYNC_CONNECT_COMPLETE_IND_T *) message);
							break;
				    }
				    break;
				    
				case DM_SYNC_CONNECT_IND:
				    PRINT(("DM_SYNC_CONNECT_IND\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncConnectInd((DM_SYNC_CONNECT_IND_T *) message);
							break;
				    }
				    break;
				    
				case DM_EX_SYNC_DISCONNECT_IND:
				    PRINT(("DM_EX_SYNC_DISCONNECT_IND\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncDisconnectInd((DM_EX_SYNC_DISCONNECT_IND_T *) message);
							break;
				    }
				    break;
				    
				case DM_EX_SYNC_DISCONNECT_CFM:
				    PRINT(("DM_EX_SYNC_DISCONNECT_CFM\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncDisconnectCfm((DM_EX_SYNC_DISCONNECT_CFM_T *) message);
							break;
				    }
				    break;
				    
				case DM_SYNC_RENEGOTIATE_IND:
				    PRINT(("DM_SYNC_RENEGOTIATE_IND\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncRenegotiateInd((DM_EX_SYNC_RENEGOTIATE_IND_T *) message);
							break;
				    }
				    break;
				
				case DM_SYNC_RENEGOTIATE_CFM:
				    PRINT(("DM_SYNC_RENEGOTIATE_CFM\n"));
				    
				    switch(libState)
				    {
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;
						case connectionReady:
							connectionHandleSyncRenegotiateCfm((DM_EX_SYNC_RENEGOTIATE_CFM_T *) message);
							break;
				    }
				    break;
				
				case DM_HCI_SWITCH_ROLE_COMPLETE:
					PRINT(("DM_HCI_SWITCH_ROLE_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleDmSwitchRoleComplete(theCm->theAppTask, &theCm->linkPolicyState, (DM_HCI_SWITCH_ROLE_COMPLETE_T *) message);
							break;
					}
					break;
					
				case DM_HCI_ROLE_DISCOVERY_COMPLETE:
					PRINT(("DM_HCI_ROLE_DISCOVERY_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleRoleDiscoveryComplete(&theCm->linkPolicyState, (DM_HCI_ROLE_DISCOVERY_COMPLETE_T *) message);
							break;
					}
					break;
				
				case DM_HCI_QOS_SETUP_CFM:
					PRINT(("DM_HCI_QOS_SETUP_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleQosSetupCfm((DM_HCI_QOS_SETUP_CFM_T *) message);
							break;
					}
					break;
					
				case DM_HCI_REMOTE_NAME_COMPLETE:
					PRINT(("DM_HCI_REMOTE_NAME_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleRemoteNameComplete(&theCm->inqState, (DM_HCI_REMOTE_NAME_COMPLETE_T *) message);
							break;
					}
					break;

				case DM_HCI_READ_LOCAL_NAME_COMPLETE:
					PRINT(("DM_HCI_READ_LOCAL_NAME_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleLocalNameComplete(&theCm->inqState, (DM_HCI_READ_LOCAL_NAME_COMPLETE_T *) message);
							break;
					}
					break;

				case DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE:
					PRINT(("DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleReadInquiryTxComplete(&theCm->inqState, (DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE_T *) message);
							break;
					}
					break;
					
				case DM_SM_ENCRYPT_CFM:
					PRINT(("DM_SM_ENCRYPT_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleEncryptCfm(&theCm->smState, (DM_SM_ENCRYPT_CFM_T *) message);
							break;
					}
					break;

				case DM_HCI_ENCRYPTION_KEY_REFRESH_IND:
					PRINT(("DM_HCI_ENCRYPTION_KEY_REFRESH_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
							goto dm_prim_error;
							break;

						case connectionReady:
							connectionHandleEncryptionKeyRefreshInd((DM_HCI_ENCRYPTION_KEY_REFRESH_IND_T *) message);
							break;
					}
					break;
					
				case DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE:
					PRINT(("DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE\n"));

					switch(libState)
					{
						case connectionTestMode:
							connectionHandleDutCfm(theCm->theAppTask, (DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE_T *) message);
							break;

						case connectionInitialising:
						case connectionUninitialised:
						case connectionReady:
                        default:
							goto dm_prim_error;
							break;
					}

					break;

				case DM_HCI_MODE_CHANGE_EVENT:
					PRINT(("DM_HCI_MODE_CHANGE_EVENT\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
                        default:
							goto dm_prim_error;
							break;

                        case connectionTestMode:
						case connectionReady:
							connectionHandleDmHciModeChangeEvent(theCm->theAppTask, (DM_HCI_MODE_CHANGE_EVENT_T *)message);
							break;
					}
					break;

					/* 
						In the debug version of the lib check the status otherwise ignore. These prims all return 
						a message which is essentially a DM_HCI_STANDARD_COMMAND_COMPLETE so we handle them all 
						in the same way. 
					*/
				case DM_HCI_WRITE_CLASS_OF_DEVICE_COMPLETE:
				case DM_HCI_WRITE_PAGESCAN_ACTIVITY_COMPLETE:
				case DM_HCI_WRITE_INQUIRYSCAN_ACTIVITY_COMPLETE:
				case DM_HCI_WRITE_SCAN_ENABLE_COMPLETE:
				case DM_HCI_SET_EVENT_FILTER_COMPLETE:
				case DM_WRITE_CACHED_PAGE_MODE_CFM:
				case DM_WRITE_CACHED_CLOCK_OFFSET_CFM:
				case DM_CLEAR_PARAM_CACHE_CFM:
				case DM_HCI_CHANGE_LOCAL_NAME_COMPLETE:
				case DM_HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_COMPLETE:
                case DM_HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE:		
				case DM_HCI_SNIFF_SUB_RATE_COMPLETE:
					checkStatus(message);
					break;

					/* 
						Primitives we ignore. These have a specific return message which we cannot generalise 
						with the macro we use for the primitives above so for the moment ignore these. 
					*/
				case DM_SM_BONDING_CFM:
				case DM_SM_REMOVE_DEVICE_CFM:
                case DM_HCI_READ_LOCAL_FEATURES_COMPLETE:
                case DM_HCI_DELETE_STORED_LINK_KEY_COMPLETE:
				case DM_HCI_WRITE_LINK_SUPERV_TIMEOUT_COMPLETE:
				case DM_HCI_WRITE_LP_SETTINGS_COMPLETE:
				case DM_HCI_WRITE_AUTO_FLUSH_TIMEOUT_COMPLETE:
                case DM_EN_ENABLE_ENHANCEMENTS_CFM:
				case DM_HCI_LINK_KEY_CHANGE_COMPLETE:
				case DM_HCI_CREATE_CONNECTION_CANCEL_COMPLETE:
					break;

				default:
					/* Prims we are not handling - for now panic the app */
					goto dm_prim_error;
			}
			break;

        case MESSAGE_BLUESTACK_RFCOMM_PRIM:
            /* Bluestack RFCOMM primitive */
            switch(((RFCOMM_UPRIM_T*)message)->type)
            {
			    case RFC_INIT_CFM:
                    PRINT(("RFC_INIT_CFM\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
						case connectionReady:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionInitialising:
							connectionSendInternalInitCfm(connectionInitRfc);
							break;
				    }
				    break;

                case RFC_REGISTER_CFM:
                    PRINT(("RFC_REGISTER_CFM\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
							connectionHandleRfcommRegisterCfm(&theCm->rfcommState, (RFC_REGISTER_CFM_T*)message);
							break;
				    }
				    break;

                case RFC_START_CFM:
                    PRINT(("RFC_START_CFM\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
							connectionHandleRfcommStartCfm(&theCm->rfcommState, (RFC_START_CFM_T*)message);
							break;
				    }
				    break;

                case RFC_START_IND:
                    PRINT(("RFC_START_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommStartInd((RFC_START_IND_T*)message);
							break;
				    }
				    break;

                case RFC_STARTCMP_IND:
                    PRINT(("RFC_STARTCMP_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommStartCmpInd((RFC_STARTCMP_IND_T*)message);
							break;
				    }
				    break;

                case RFC_EX_PARNEG_IND:
                    PRINT(("RFC_EX_PARNEG_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommParnegInd((RFC_EX_PARNEG_IND_T*)message);
							break;
				    }
				    break;

                case RFC_PARNEG_CFM:
                    PRINT(("RFC_PARNEG_CFM\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommParnegCfm((RFC_PARNEG_CFM_T*)message);
							break;
				    }
				    break;

                case RFC_EX_ESTABLISH_IND:
                    PRINT(("RFC_EX_ESTABLISH_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommEstablishInd((RFC_EX_ESTABLISH_IND_T*)message);
							break;
				    }
				    break;

                case RFC_ESTABLISH_CFM:
                    PRINT(("RFC_ESTABLISH_CFM\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommEstablishCfm((RFC_ESTABLISH_CFM_T*)message);
							break;
				    }
				    break;

                case RFC_CONTROL_IND:
                    PRINT(("RFC_CONTROL_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommControlInd((RFC_CONTROL_IND_T*)message);
							break;
				    }
				    break;

                case RFC_EX_RELEASE_IND:
                    PRINT(("RFC_EX_RELEASE_IND\n"));

				    switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommReleaseInd((RFC_EX_RELEASE_IND_T*)message);
							break;
				    }
				    break;

				case RFC_PORTNEG_IND:
					PRINT(("RFC_PORTNEG_IND\n"));
					
					switch(libState)
				    {
						case connectionUninitialised:
                        case connectionInitialising:
						case connectionTestMode:
                        default:
							goto rfc_prim_error;
							break;

						case connectionReady:
                            connectionHandleRfcommPortNegInd((RFC_PORTNEG_IND_T*)message);
							break;
					}
					break;

                /* Ignore this message, handler function did nothing */
                case RFC_CLOSE_IND:
				/* Should never receive this but ignore it specifically just in case. */
				case RFC_DATA_IND:
					break;

				default:
					/* Prims we are not handling - for now panic the app */
					goto rfc_prim_error;
            }
            break;
			
		case MESSAGE_BLUESTACK_L2CAP_PRIM:           
			/* Bluestack L2CAP primitive */
			switch(((L2CA_UPRIM_T*)message)->type)
			{
				case L2CA_REGISTER_CFM:
					PRINT(("L2CA_REGISTER_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							/* Handle the register cfm event */
							connectionHandleL2capRegisterCfm(&theCm->l2capState, (L2CA_REGISTER_CFM_T *)message);
							break;
					}
					break;

				case L2CA_CONNECT_IND:
					PRINT(("L2CA_CONNECT_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capConnectInd((L2CA_CONNECT_IND_T *) message);
							break;
					}
					break;

				case L2CA_CONNECT_CFM:
					PRINT(("L2CA_CONNECT_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capConnectCfm((L2CA_CONNECT_CFM_T *)message);
							break;
					}
					break;

				case L2CA_CONFIG_IND:
					PRINT(("L2CA_CONFIG_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capConfigInd((L2CA_CONFIG_IND_T *) message);
							break;
					}
					break;

				case L2CA_CONFIG_CFM:
					PRINT(("L2CA_CONFIG_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capConfigCfm((L2CA_CONFIG_CFM_T *)message);
							break;
					}
					break;

				case L2CA_DISCONNECT_IND:
					PRINT(("L2CA_DISCONNECT_IND\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capDisconnectInd((L2CA_DISCONNECT_IND_T *) message);
							break;
					}
					break;

				case L2CA_DISCONNECT_CFM:
					PRINT(("L2CA_DISCONNECT_CFM\n"));

					switch(libState)
					{
						case connectionInitialising:
						case connectionUninitialised:
						case connectionTestMode:
                        default:
							goto l2cap_prim_error;
							break;

						case connectionReady:
							connectionHandleL2capDisconnectCfm((L2CA_DISCONNECT_CFM_T *) message);
							break;
					}
					break;

					/* For the moment ignore this primitive. */
				case L2CA_TIMEOUT_IND:
					break;

				default:
					/* Prims we are not handling - for now panic the app */
					goto l2cap_prim_error;
			}
			break;
			
		case MESSAGE_BLUESTACK_SDP_PRIM:
			/* Bluestack SDP primitive */

			switch(((SDC_UPRIM_T*)message)->type)
			{
				case SDS_REGISTER_CFM:
					PRINT(("SDS_REGISTER_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpRegisterCfm(&theCm->sdpState, (SDS_REGISTER_CFM_T *)message);
							break;
					}
					break;

				case SDS_UNREGISTER_CFM:
					PRINT(("SDS_UNREGISTER_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpUnregisterCfm(&theCm->sdpState, (SDS_UNREGISTER_CFM_T *)message);
							break;
					}
					break;

				case SDC_OPEN_SEARCH_CFM:
					PRINT(("SDC_OPEN_SEARCH_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpOpenSearchCfm(&theCm->sdpState, (SDC_OPEN_SEARCH_CFM_T *)message);
							break;
					}
					break;

				case SDC_CLOSE_SEARCH_IND:
					PRINT(("SDC_CLOSE_SEARCH_IND\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpCloseSearchInd(&theCm->sdpState, (SDC_CLOSE_SEARCH_IND_T *)message);
							break;
					}
					break;

				case SDC_EX_SERVICE_SEARCH_CFM:
					PRINT(("SDC_EX_SERVICE_SEARCH_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpServiceSearchCfm(&theCm->sdpState, (SDC_EX_SERVICE_SEARCH_CFM_T *)message);
							break;
					}
					break;

				case SDC_EX_SERVICE_ATTRIBUTE_CFM:
					PRINT(("SDC_EX_SERVICE_ATTRIBUTE_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpAttributeSearchCfm(&theCm->sdpState, (SDC_EX_SERVICE_ATTRIBUTE_CFM_T *)message);
							break;
					}
					break;

				case SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM:
					PRINT(("SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));

					switch(libState)
					{
						case connectionUninitialised:
						case connectionInitialising:				
						case connectionTestMode:
                        default:
							goto sdp_prim_error;
							break;

						case connectionReady:
							connectionHandleSdpServiceSearchAttributeCfm(&theCm->sdpState, (SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
							break;
					}
					break;

				default:
					/* Prims we are not handling - for now panic the app */
					goto sdp_prim_error;
					break;
			}
			break;
			
		case MESSAGE_BLUESTACK_UDP_PRIM:
			/* Bluestack UDP primitive */
			break;
			
		case MESSAGE_BLUESTACK_TCP_PRIM:          
			/* Bluestack TCP Primitive */
			break;
			
		default:
			/* Unhandled message ID received */
			
			/*
				CL_SDP_CLOSE_SEARCH_CFM Primitive arrived as a result of an internal 
				call to close SDP search, can't avoid so ignore
				
				Handled as a special case to allow the compiler to generate better
				code for the previous switch statements.
				
			*/
			if (id != CL_SDP_CLOSE_SEARCH_CFM)
			{
				handleUnexpected(connectionUnhandledMessage, libState, id);
			}	
			
			break;
    }
	}
    return;
    
    cm_prim_error:
    handleUnexpected(connectionUnexpectedCmPrim, libState, id);
    return;
    
    dm_prim_error:
    handleUnexpected(connectionUnexpectedDmPrim, libState, ((DM_UPRIM_T*)message)->type);
    return;
    
    rfc_prim_error:
    handleUnexpected(connectionUnexpectedRfcPrim, libState, ((RFCOMM_UPRIM_T*)message)->type);
    return;
    
    l2cap_prim_error:
    handleUnexpected(connectionUnexpectedL2capPrim, libState, ((L2CA_UPRIM_T*)message)->type);
    return;
    
    sdp_prim_error:
    handleUnexpected(connectionUnexpectedSdpPrim, libState, ((SDC_UPRIM_T *)message)->type);
    return;
    
    
 
}

/*lint +e655 +e525 +e830 */
