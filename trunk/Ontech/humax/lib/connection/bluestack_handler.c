/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

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

#undef DEBUG_PRINT_ENABLED

#include <print.h>
#include <vm.h>

/*lint -e655 -e525 -e830 */

/* Connection state management */
#define SET_CM_STATE(s) theCm->state = (s);

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
    This macro is called as a result of a message arriving when the
    Connection Library was not expecting it.

RETURNS
    void	
*/
#define handleUnexpected(code, state, type) \
    CL_DEBUG(("handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type))

/****************************************************************************
NAME	
    connectionBluestackHandlerDm

DESCRIPTION
    Message handler for the DM primitives.

RETURNS
    void	
*/
static void connectionBluestackHandlerDm(connectionState *theCm, DM_UPRIM_T *message)
{
	const uint8 state = theCm->state;	
	if (state == connectionReady)
	{
		switch (message->type)
		{
			case DM_SYNC_REGISTER_CFM:
			    PRINT(("DM_SYNC_REGISTER_CFM\n"));
			    connectionHandleSyncRegisterCfm( (DM_SYNC_REGISTER_CFM_T *)message);
			    return;
		    
			case DM_SYNC_UNREGISTER_CFM:
			    PRINT(("DM_SYNC_UNREGISTER_CFM\n"));
			    connectionHandleSyncRegisterCfm( (DM_SYNC_REGISTER_CFM_T *)message );
				return;
		    
			case DM_EX_SYNC_CONNECT_CFM:
			    PRINT(("DM_EX_SYNC_CONNECT_CFM\n"));
				connectionHandleSyncConnectCfm(theCm->theAppTask, (DM_EX_SYNC_CONNECT_CFM_T *) message);
				return;
		    
			case DM_EX_SYNC_CONNECT_COMPLETE_IND:
			    PRINT(("DM_EX_SYNC_CONNECT_COMPLETE_IND\n"));
				connectionHandleSyncConnectCompleteInd(theCm->theAppTask, (DM_EX_SYNC_CONNECT_COMPLETE_IND_T *) message);
				return;
		    
			case DM_SYNC_CONNECT_IND:
			    PRINT(("DM_SYNC_CONNECT_IND\n"));
				connectionHandleSyncConnectInd((DM_SYNC_CONNECT_IND_T *) message);
				return;
		    
			case DM_EX_SYNC_DISCONNECT_IND:
			    PRINT(("DM_EX_SYNC_DISCONNECT_IND\n"));
				connectionHandleSyncDisconnectInd((DM_EX_SYNC_DISCONNECT_IND_T *) message);
				return;
		    
			case DM_EX_SYNC_DISCONNECT_CFM:
			    PRINT(("DM_EX_SYNC_DISCONNECT_CFM\n"));
				connectionHandleSyncDisconnectCfm((DM_EX_SYNC_DISCONNECT_CFM_T *) message);
				return;
		    
			case DM_EX_SYNC_RENEGOTIATE_IND:
			    PRINT(("DM_EX_SYNC_RENEGOTIATE_IND\n"));
				connectionHandleSyncRenegotiateInd((DM_EX_SYNC_RENEGOTIATE_IND_T *) message);
				return;
			
			case DM_EX_SYNC_RENEGOTIATE_CFM: /* 31783 */
				PRINT(("DM_EX_SYNC_RENEGOTIATE_CFM\n"));
				connectionHandleSyncRenegotiateCfm((DM_EX_SYNC_RENEGOTIATE_CFM_T *) message);
				return;
		}

		switch (message->type)
		{
			case DM_SM_ADD_DEVICE_CFM: /* 11284 */
			case DM_SM_SSP_ADD_DEVICE_CFM: /* 11296 */
				PRINT(("DM_SM_SSP_ADD_DEVICE_CFM\n"));
				connectionHandleSmAddDeviceCfmReady(&theCm->smState, (DM_SM_ADD_DEVICE_CFM_T*)message);
				return;

			case DM_SM_LINK_KEY_REQUEST_IND: /* 11286 */
				PRINT(("DM_SM_LINK_KEY_REQUEST_IND\n"));
				connectionHandleSmLinkKeyReqInd((DM_SM_LINK_KEY_REQUEST_IND_T*)message);
				return;

			case DM_SM_PIN_REQUEST_IND:	/* 11287 */
				PRINT(("DM_SM_PIN_REQUEST_IND\n"));
				connectionHandleSmPinReqInd(theCm->theAppTask, (DM_SM_PIN_REQUEST_IND_T*)message);
				return;

			case DM_SM_LINK_KEY_IND: /* 11288 */
				PRINT(("DM_SM_LINK_KEY_IND\n"));
				connectionHandleSmLinkKeyInd(theCm->theAppTask, &theCm->infoState, &theCm->smState, (DM_SM_LINK_KEY_IND_T*)message);
				return;
					
			case DM_SM_AUTHORISE_IND: /* 11289 */
				PRINT(("DM_SM_AUTHORISE_IND\n"));
				connectionHandleSmAuthoriseInd(theCm->theAppTask, (DM_SM_AUTHORISE_IND_T*)message);
				return;
					
			case DM_SM_AUTHENTICATE_CFM: /* 11290 */
				PRINT(("DM_SM_AUTHENTICATE_CFM\n"));
				return;
												
			case DM_SM_ENCRYPT_CFM: /* 11291 */
				PRINT(("DM_SM_ENCRYPT_CFM\n"));
				connectionHandleEncryptCfm(&theCm->smState, (DM_SM_ENCRYPT_CFM_T *) message);
				return;

			case DM_SM_ENCRYPTION_CHANGE: /* 11292 */
				PRINT(("DM_SM_ENCRYPTION_CHANGE\n"));
				connectionHandleEncryptionChange((DM_SM_ENCRYPTION_CHANGE_T *)message);
				return;
						
			case DM_SM_READ_LOCAL_OOB_DATA_CFM: /* 11295 */
				PRINT(("DM_SM_READ_LOCAL_OOB_DATA_CFM\n"));
				connectionHandleReadLocalOobDataCfm(&theCm->smState, (DM_SM_READ_LOCAL_OOB_DATA_CFM_T*)message);
				return;

			case DM_SM_IO_CAPABILITY_RESPONSE_IND: /* 11297 */
				PRINT(("DM_SM_IO_CAPABILITY_RESPONSE_IND \n"));
				connectionHandleSmIoCapResInd(theCm->theAppTask, &theCm->smState, (DM_SM_IO_CAPABILITY_RESPONSE_IND_T*)message);
				return;

			case DM_SM_IO_CAPABILITY_REQUEST_IND: /* 11298 */
				PRINT(("DM_SM_IO_CAPABILITY_REQUEST_IND \n"));
				connectionHandleSmIoCapReqInd(theCm->theAppTask, (DM_SM_IO_CAPABILITY_REQUEST_IND_T*)message);
				return;			
				
			case DM_SM_SIMPLE_PAIRING_COMPLETE_IND: /* 11299 */
				PRINT(("DM_SM_SIMPLE_PAIRING_COMPLETE_IND\n"));
				connectionHandleSmSimplePairingCompleteInd(theCm->theAppTask, &theCm->smState, (DM_SM_SIMPLE_PAIRING_COMPLETE_IND_T*)message);
				return;

			case DM_SM_USER_CONFIRMATION_REQUEST_IND: /* 11300 */
				PRINT(("DM_SM_USER_CONFIRMATION_REQUEST_IND \n"));
				connectionHandleSmUserConfirmationReqInd(theCm->theAppTask, (DM_SM_USER_CONFIRMATION_REQUEST_IND_T*)message);
				return;
			
			case DM_SM_USER_PASSKEY_REQUEST_IND: /* 11301 */
				PRINT(("DM_SM_USER_PASSKEY_REQUEST_IND \n"));
				connectionHandleSmUserPasskeyReqInd(theCm->theAppTask, (DM_SM_USER_PASSKEY_REQUEST_IND_T*)message);
				return;
				
			case DM_SM_SSP_LINK_KEY_REQUEST_IND: /* 11302 */
				PRINT(("DM_SM_SSP_LINK_KEY_REQUEST_IND\n"));
				connectionHandleSmSspLinkKeyReqInd(theCm->theAppTask, &theCm->smState, (DM_SM_SSP_LINK_KEY_REQUEST_IND_T*)message);
				return;

			case DM_SM_USER_PASSKEY_NOTIFICATION_IND: /* 11303 */
				PRINT(("DM_SM_USER_PASSKEY_NOTIFICATION_IND \n"));
				connectionHandleSmUserPasskeyNotificationInd(theCm->theAppTask, &theCm->smState, (DM_SM_USER_PASSKEY_NOTIFICATION_IND_T*)message);
				return;

			case DM_SM_KEYPRESS_NOTIFICATION_IND: /* 11304 */
				PRINT(("DM_SM_KEYPRESS_NOTIFICATION_IND \n"));
				connectionHandleSmKeypressNotificationInd(theCm->theAppTask, (DM_SM_KEYPRESS_NOTIFICATION_IND_T*)message);
				return;			
		}
		
		switch (message->type)
		{
			case DM_HCI_INQUIRY_CANCEL_COMPLETE: /* 1057 */
				PRINT(("DM_HCI_INQUIRY_CANCEL_COMPLETE\n"));
				connectionHandleInquiryComplete(&theCm->inqState);
				return;

			case DM_HCI_INQUIRY_RESULT:	/* 1060 */
				PRINT(("DM_HCI_INQUIRY_RESULT\n"));
				connectionHandleInquiryResult(&theCm->inqState, (DM_HCI_INQUIRY_RESULT_T *)message);
				return;

			case DM_HCI_INQUIRY_COMPLETE: /* 1061 */
				PRINT(("DM_HCI_INQUIRY_COMPLETE\n"));
				connectionHandleInquiryComplete(&theCm->inqState);
				return;

			case DM_HCI_REMOTE_NAME_COMPLETE: /* 1065 */
				PRINT(("DM_HCI_REMOTE_NAME_COMPLETE\n"));
				connectionHandleRemoteNameComplete(&theCm->inqState, (DM_HCI_REMOTE_NAME_COMPLETE_T *) message);
				return;

			case DM_HCI_READ_REMOTE_FEATURES_COMPLETE: /* 1066 */
				PRINT(("DM_HCI_READ_REMOTE_FEATURES_COMPLETE\n"));
				connectionHandleReadRemoteSupportedFeaturesCfm(&theCm->infoState, (DM_HCI_READ_REMOTE_FEATURES_COMPLETE_T *) message);
				return;
					
			case DM_HCI_READ_REMOTE_VERSION_COMPLETE: /* 1067 */
				PRINT(("DM_HCI_READ_REMOTE_VERSION_COMPLETE_T\n"));
				connectionHandleReadRemoteVersionCfm(&theCm->infoState, (DM_HCI_READ_REMOTE_VERSION_COMPLETE_T *) message);
				return;
				
			case DM_HCI_READ_CLOCK_OFFSET_COMPLETE: /* 1068 */
				PRINT(("DM_HCI_READ_CLOCK_OFFSET_COMPLETE\n"));
				connectionHandleReadClkOffsetComplete(&theCm->infoState, (DM_HCI_READ_CLOCK_OFFSET_COMPLETE_T *) message);
				return;
		}
		
		switch (message->type)
		{														
			case DM_HCI_READ_INQUIRY_MODE_COMPLETE: /* 30730 */
				PRINT(("DM_HCI_READ_INQUIRY_MODE_COMPLETE\n"));
				connectionHandleReadInquiryModeComplete(&theCm->inqState, (DM_HCI_READ_INQUIRY_MODE_COMPLETE_T *)message);
				return;

			case DM_HCI_WRITE_INQUIRY_MODE_COMPLETE: /* 30731 */
				PRINT(("DM_HCI_WRITE_INQUIRY_MODE_COMPLETE\n"));
				connectionHandleWriteInquiryModeComplete(&theCm->inqState, (DM_HCI_WRITE_INQUIRY_MODE_COMPLETE_T *)message);
				return;

			case DM_HCI_INQUIRY_RESULT_WITH_RSSI: /* 30742 */
				PRINT(("DM_HCI_INQUIRY_RESULT_WITH_RSSI\n"));
				connectionHandleInquiryResultWithRssi(&theCm->inqState, (DM_HCI_INQUIRY_RESULT_WITH_RSSI_T *)message);
				return;
					
			case DM_HCI_SNIFF_SUB_RATING_IND: /* 30784 */
				PRINT(("DM_HCI_SNIFF_SUB_RATING_IND\n"));
				connectionHandleSniffSubRatingInd((DM_HCI_SNIFF_SUB_RATING_IND_T *)message);
				return;

			case DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE: /* 30786 */
				PRINT(("DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE\n"));
				connectionHandleReadEirDataComplete(&theCm->inqState, (DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE_T *)message);
				return;

			case DM_HCI_EXTENDED_INQUIRY_RESULT_IND: /* 30788 */
				PRINT(("DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T\n"));
				connectionHandleExtendedInquiryResult(&theCm->inqState, (DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T *)message);
				return;

			case DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE: /* 30790 */
				PRINT(("DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE\n"));
				connectionHandleReadInquiryTxComplete(&theCm->inqState, (DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE_T *) message);
				return;

			case DM_HCI_LINK_SUPERV_TIMEOUT_IND: /* 30792 */
				PRINT(("DM_HCI_LINK_SUPERV_TIMEOUT_IND\n"));
				connectionHandleLinkSupervisionTimeoutInd((DM_HCI_LINK_SUPERV_TIMEOUT_IND_T *)message);
				return;

			case DM_HCI_ENCRYPTION_KEY_REFRESH_IND: /* 30794 */
				PRINT(("DM_HCI_ENCRYPTION_KEY_REFRESH_IND\n"));
				connectionHandleEncryptionKeyRefreshInd((DM_HCI_ENCRYPTION_KEY_REFRESH_IND_T *) message);
				return;
		}
		
		switch (message->type)
		{														
			case DM_HCI_QOS_SETUP_CFM: /* 2064 */
				PRINT(("DM_HCI_QOS_SETUP_CFM\n"));
				connectionHandleQosSetupCfm((DM_HCI_QOS_SETUP_CFM_T *) message);
				return;

			case DM_HCI_ROLE_DISCOVERY_COMPLETE: /* 2066 */
				PRINT(("DM_HCI_ROLE_DISCOVERY_COMPLETE\n"));
				connectionHandleRoleDiscoveryComplete(&theCm->linkPolicyState, (DM_HCI_ROLE_DISCOVERY_COMPLETE_T *) message);
				return;
		
			case DM_HCI_SWITCH_ROLE_COMPLETE: /* 2067 */
				PRINT(("DM_HCI_SWITCH_ROLE_COMPLETE\n"));
				connectionHandleDmSwitchRoleComplete(theCm->theAppTask, &theCm->linkPolicyState, (DM_HCI_SWITCH_ROLE_COMPLETE_T *) message);
				return;

			case DM_HCI_READ_LOCAL_NAME_COMPLETE: /* 3147 */
				PRINT(("DM_HCI_READ_LOCAL_NAME_COMPLETE\n"));
				connectionHandleLocalNameComplete(&theCm->inqState, (DM_HCI_READ_LOCAL_NAME_COMPLETE_T *) message);
				return;

			case DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE: /* 3160 */
				PRINT(("DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE\n"));
				connectionHandleReadClassOfDeviceComplete(&theCm->infoState, (DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE_T *)message);
				return;

			case DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE: /* 3177 */
				PRINT(("DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE\n"));
				connectionHandleWriteIacLapComplete(&theCm->inqState, (DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_T *)message);
				return;

			case DM_HCI_READ_BD_ADDR_COMPLETE: /* 4110 */
				PRINT(("DM_HCI_READ_BD_ADDR_COMPLETE\n"));
				connectionHandleReadBdAddrComplete(&theCm->infoState, (DM_HCI_READ_BD_ADDR_COMPLETE_T *)message);
				return;

			case DM_HCI_GET_LINK_QUALITY_COMPLETE: /* 5129 */
				PRINT(("DM_HCI_GET_LINK_QUALITY_COMPLETE\n"));
				connectionHandleReadLinkQualityComplete(&theCm->infoState, (DM_HCI_GET_LINK_QUALITY_COMPLETE_T *) message);
				return;

			case DM_HCI_READ_RSSI_COMPLETE: /* 5130 */
				PRINT(("DM_HCI_READ_RSSI_COMPLETE\n"));
				connectionHandleReadRssiComplete(&theCm->infoState, (DM_HCI_READ_RSSI_COMPLETE_T *) message);
				return;

			case DM_LP_WRITE_POWERSTATES_CFM: /* 30977 */
				PRINT(("DM_LP_WRITE_POWERSTATES_CFM\n"));
				connectionLinkPolicyHandleWritePowerStatesCfm((DM_LP_WRITE_POWERSTATES_CFM_T *)message);
				return;
		}
	}
	
	if (state != connectionUninitialised)
	{
		if (state != connectionInitialising)
		{
			switch (message->type)
			{
				case DM_HCI_MODE_CHANGE_EVENT: /* 2063 */
					PRINT(("DM_HCI_MODE_CHANGE_EVENT\n"));
					connectionHandleDmHciModeChangeEvent(theCm->theAppTask, (DM_HCI_MODE_CHANGE_EVENT_T *)message);
					return;

				case DM_ACL_OPEN_CFM: /* 10252 */
					PRINT(("DM_ACL_OPEN_CFM\n"));
					connectionHandleDmAclOpenCfm(&theCm->smState, (DM_ACL_OPEN_CFM_T*)message);
					return;
											
				case DM_ACL_OPENED_IND: /* 10253 */
					PRINT(("DM_ACL_OPENED_IND\n"));
					connectionHandleDmAclOpenInd(theCm->theAppTask, (DM_ACL_OPENED_IND_T *) message);
					return;
				
				case DM_EN_ACL_OPENED_IND: /* 12292 */
					PRINT(("DM_EN_ACL_OPENED_IND\n"));
					connectionHandleDmEnAclOpenInd(theCm->theAppTask, &theCm->smState, (DM_EN_ACL_OPENED_IND_T *) message);
					return;

				case DM_EN_ACL_CLOSED_IND: /* 12293 */
					PRINT(("DM_EN_ACL_CLOSED_IND\n"));
					connectionHandleDmEnAclClosedInd(theCm->theAppTask, &theCm->smState, (DM_EN_ACL_CLOSED_IND_T *)message);
					return;
						
				case DM_ACL_CLOSED_IND: /* 28697 */
					PRINT(("DM_ACL_CLOSED_IND\n"));
					connectionHandleDmAclClosedInd(theCm->theAppTask, &theCm->smState, (DM_ACL_CLOSED_IND_T*)message);
					return;
			}
		}
		
		switch (message->type)
		{
			case DM_SET_BT_VERSION_CFM:
				PRINT(("DM_SET_BT_VERSION_CFM\n"));
				connectionHandleSetBtVersionCfm(&theCm->infoState, (DM_SET_BT_VERSION_CFM_T*)message);	
				return;

			case DM_HCI_READ_LOCAL_VERSION_COMPLETE:
				PRINT(("DM_HCI_READ_LOCAL_VERSION_COMPLETE_T\n"));
				connectionHandleReadLocalVersionCfm(&theCm->infoState, (DM_HCI_READ_LOCAL_VERSION_COMPLETE_T *) message);
				return;
						
			case DM_AM_REGISTER_CFM:
				PRINT(("DM_AM_REGISTER_CFM\n"));
				if (state == connectionInitialising)
				{
					connectionSendInternalInitCfm(connectionInitDm);
					return;
				}
				break;

			case DM_SM_ADD_DEVICE_CFM:
			case DM_SM_SSP_ADD_DEVICE_CFM:
				PRINT(("DM_SM_SSP_ADD_DEVICE_CFM\n"));
				if (state == connectionInitialising)
				{
					connectionHandleSmAddDeviceCfm(&theCm->smState, (DM_SM_ADD_DEVICE_CFM_T*)message);
					return;
				}
				break;
				
			case DM_SM_ACCESS_IND:
				PRINT(("DM_SM_ACCESS_IND\n"));
				if (state != connectionTestMode)
				{
					connectionHandleSmAccessInd(&theCm->sdpState, (DM_SM_ACCESS_IND_T*)message);
					return;
				}
				break;
						
			case DM_SM_SEC_MODE_CONFIG_CFM:	
				PRINT(("DM_SM_SEC_MODE_CONFIG_CFM\n"));
				if (state != connectionTestMode)
				{
					connectionHandleConfigureSecurityCfm(&theCm->smState, (DM_SM_SEC_MODE_CONFIG_CFM_T *)message);
					return;
				}
				break;

			case DM_SM_SET_SEC_MODE_CFM:
				PRINT(("DM_SM_SET_SEC_MODE_CFM\n"));
				if (state == connectionTestMode)
				{
					connectionHandleDutSecurityDisabled(theCm->theAppTask, (DM_SM_SET_SEC_MODE_CFM_T *)message);
					return;
				}
				else
				{
					connectionHandleSetSecurityModeCfm(&theCm->smState, (DM_SM_SET_SEC_MODE_CFM_T *)message);
					return;
				}
				break;
															
			case DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE:
				PRINT(("DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE\n"));
				if (state == connectionTestMode)
				{
					connectionHandleDutCfm(theCm->theAppTask, (DM_HCI_ENABLE_DEVICE_UT_MODE_COMPLETE_T *) message);
					return;
				}
				break;

			/* In the debug version of the lib check the status otherwise ignore. These prims all return 
			   a message which is essentially a DM_HCI_STANDARD_COMMAND_COMPLETE so we handle them all 
			   in the same way.	*/
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
				checkStatus(message);
				return;

			/* Primitives we ignore. These have a specific return message which we cannot generalise 
			   with the macro we use for the primitives above so for the moment ignore these. */
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
			case DM_HCI_READ_REMOTE_EXT_FEATURES_COMPLETE:
			case DM_HCI_SNIFF_SUB_RATE_COMPLETE:
				return;
		}
	}
	
	/* Prims we are not handling - for now panic the app */
	handleUnexpected(connectionUnexpectedDmPrim, state, message->type);
}

/****************************************************************************
NAME	
    connectionBluestackHandlerReadyL2cap

DESCRIPTION
    L2CAP message handler for the ready state.

RETURNS
    void	
*/
static void connectionBluestackHandlerL2cap(connectionState *theCm, L2CA_UPRIM_T *message)
{	
	if (theCm->state == connectionReady)
	{
		switch (message->type)
		{
			case L2CA_REGISTER_CFM:
				PRINT(("L2CA_REGISTER_CFM\n"));
				connectionHandleL2capRegisterCfm(&theCm->l2capState, (L2CA_REGISTER_CFM_T *)message);
				return;

			case L2CA_CONNECT_IND:
				PRINT(("L2CA_CONNECT_IND\n"));
				connectionHandleL2capConnectInd((L2CA_CONNECT_IND_T *) message);
				return;

			case L2CA_CONNECT_CFM:
				PRINT(("L2CA_CONNECT_CFM\n"));
				connectionHandleL2capConnectCfm((L2CA_CONNECT_CFM_T *)message);
				return;

			case L2CA_CONFIG_IND:
				PRINT(("L2CA_CONFIG_IND\n"));
				connectionHandleL2capConfigInd((L2CA_CONFIG_IND_T *) message);
				return;

			case L2CA_CONFIG_CFM:
				PRINT(("L2CA_CONFIG_CFM\n"));
				connectionHandleL2capConfigCfm((L2CA_CONFIG_CFM_T *)message);
				return;

			case L2CA_DISCONNECT_IND:
				PRINT(("L2CA_DISCONNECT_IND\n"));
				connectionHandleL2capDisconnectInd((L2CA_DISCONNECT_IND_T *) message);
				return;
			
			case L2CA_DISCONNECT_CFM:
				PRINT(("L2CA_DISCONNECT_CFM\n"));
				connectionHandleL2capDisconnectCfm((L2CA_DISCONNECT_CFM_T *) message);
				return;

			/* For the moment ignore this primitive. */
			case L2CA_TIMEOUT_IND:
				return;
		}
	}
	
	/* Prims we are not handling - for now panic the app */
	handleUnexpected(connectionUnexpectedL2capPrim, theCm->state, message->type);
}

/****************************************************************************
NAME	
    connectionBluestackHandlerReadyRfcomm

DESCRIPTION
    Rfcomm message handler for the ready state.

RETURNS
    void	
*/
static void connectionBluestackHandlerRfcomm(connectionState *theCm, RFCOMM_UPRIM_T *message)
{	
	if (theCm->state == connectionReady)
	{
		switch (message->type)
		{
			case RFC_REGISTER_CFM:
				PRINT(("RFC_REGISTER_CFM\n"));
				connectionHandleRfcommRegisterCfm(&theCm->rfcommState, (RFC_REGISTER_CFM_T*)message);
				return;

			case RFC_START_CFM:
				PRINT(("RFC_START_CFM\n"));
				connectionHandleRfcommStartCfm(&theCm->rfcommState, (RFC_START_CFM_T*)message);
				return;

			case RFC_START_IND:
				PRINT(("RFC_START_IND\n"));
				connectionHandleRfcommStartInd((RFC_START_IND_T*)message);
				return;

			case RFC_STARTCMP_IND:
				PRINT(("RFC_STARTCMP_IND\n"));
				connectionHandleRfcommStartCmpInd((RFC_STARTCMP_IND_T*)message);
				return;

			case RFC_EX_PARNEG_IND:
				PRINT(("RFC_EX_PARNEG_IND\n"));
				connectionHandleRfcommParnegInd((RFC_EX_PARNEG_IND_T*)message);
				return;

			case RFC_PARNEG_CFM:
				PRINT(("RFC_PARNEG_CFM\n"));
				connectionHandleRfcommParnegCfm((RFC_PARNEG_CFM_T*)message);
				return;

			case RFC_EX_ESTABLISH_IND:
				PRINT(("RFC_EX_ESTABLISH_IND\n"));
				connectionHandleRfcommEstablishInd((RFC_EX_ESTABLISH_IND_T*)message);
				return;

			case RFC_ESTABLISH_CFM:
				PRINT(("RFC_ESTABLISH_CFM\n"));
				connectionHandleRfcommEstablishCfm((RFC_ESTABLISH_CFM_T*)message);
				return;

			case RFC_CONTROL_IND:
				PRINT(("RFC_CONTROL_IND\n"));
				connectionHandleRfcommControlInd((RFC_CONTROL_IND_T*)message);
				return;

			case RFC_EX_RELEASE_IND:
				PRINT(("RFC_EX_RELEASE_IND\n"));
				connectionHandleRfcommReleaseInd((RFC_EX_RELEASE_IND_T*)message);
				return;

			case RFC_PORTNEG_IND:
				PRINT(("RFC_PORTNEG_IND\n"));				
				connectionHandleRfcommPortNegInd((RFC_PORTNEG_IND_T*)message);
				return;

			/* Ignore this message, handler function did nothing */
			case RFC_CLOSE_IND:
				return;
		}
	}
	else if (theCm->state == connectionInitialising)
	{			
		if (message->type == RFC_INIT_CFM)
		{
			PRINT(("RFC_INIT_CFM\n"));
			connectionSendInternalInitCfm(connectionInitRfc);
			return;
		}
	}
	
	/* Prims we are not handling - for now panic the app */
	handleUnexpected(connectionUnexpectedRfcPrim, theCm->state, message->type);
}

/****************************************************************************
NAME	
    connectionBluestackHandlerReadySdp

DESCRIPTION
    SDP message handler for the ready state.

RETURNS
    void	
*/
static void connectionBluestackHandlerSdp(connectionState *theCm, SDS_UPRIM_T *message)
{
	if (theCm->state == connectionReady)
	{
		switch (message->type)
		{
			case SDS_REGISTER_CFM:
				PRINT(("SDS_REGISTER_CFM\n"));
				connectionHandleSdpRegisterCfm(&theCm->sdpState, (SDS_REGISTER_CFM_T *)message);
				return;
							
			case SDS_UNREGISTER_CFM:
				PRINT(("SDS_UNREGISTER_CFM\n"));
				connectionHandleSdpUnregisterCfm(&theCm->sdpState, (SDS_UNREGISTER_CFM_T *)message);
				return;
								
			case SDC_OPEN_SEARCH_CFM:
				PRINT(("SDC_OPEN_SEARCH_CFM\n"));
				connectionHandleSdpOpenSearchCfm(&theCm->sdpState, (SDC_OPEN_SEARCH_CFM_T *)message);
				return;
							
			case SDC_CLOSE_SEARCH_IND:
				PRINT(("SDC_CLOSE_SEARCH_IND\n"));
				connectionHandleSdpCloseSearchInd(&theCm->sdpState, (SDC_CLOSE_SEARCH_IND_T *)message);
				return;

			case SDC_EX_SERVICE_SEARCH_CFM:
				PRINT(("SDC_EX_SERVICE_SEARCH_CFM\n"));
				connectionHandleSdpServiceSearchCfm(&theCm->sdpState, (SDC_EX_SERVICE_SEARCH_CFM_T *)message);
				return;
				
			case SDC_EX_SERVICE_ATTRIBUTE_CFM:
				PRINT(("SDC_EX_SERVICE_ATTRIBUTE_CFM\n"));
				connectionHandleSdpAttributeSearchCfm(&theCm->sdpState, (SDC_EX_SERVICE_ATTRIBUTE_CFM_T *)message);
				return;

			case SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM:
				PRINT(("SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
				connectionHandleSdpServiceSearchAttributeCfm(&theCm->sdpState, (SDC_EX_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
				return;
		}
	}
	
	/* Prims we are not handling - for now panic the app */
	handleUnexpected(connectionUnexpectedSdpPrim, theCm->state, message->type);
}

/****************************************************************************
NAME	
    connectionBluestackHandlerUninitialised

DESCRIPTION
    Message handler for the uninitialised state.

RETURNS
    void	
*/
static void connectionBluestackHandlerUninitialised(connectionState *theCm, MessageId id, Message message)
{
	/* Depending upon the message id...*/
	if (id == CL_INTERNAL_INIT_REQ)
	{
		PRINT(("CL_INTERNAL_INIT_REQ\n"));
		connectionHandleInternalInit(connectionInit);
	}
	else
	{
		/* Prims we are not handling - for now panic the app */
        handleUnexpected(connectionUnexpectedCmPrim, theCm->state, id);
	}
}

/****************************************************************************
NAME	
    connectionBluestackHandlerInitialising

DESCRIPTION
    Message handler for the initialising state.

RETURNS
    void	
*/
static void connectionBluestackHandlerInitialising(connectionState *theCm, MessageId id, Message message)
{
	/* Depending upon the message id...*/
	switch (id)
	{
        case CL_INTERNAL_INIT_CFM:
            PRINT(("CL_INTERNAL_INIT_CFM\n"));
		    connectionHandleInternalInit(((CL_INTERNAL_INIT_CFM_T*)message)->mask);
		    break;

		case CL_INTERNAL_INIT_TIMEOUT_IND:
            PRINT(("CL_INTERNAL_INIT_TIMEOUT_IND\n"));
		    (void)MessageCancelFirst(&theCm->task, CL_INTERNAL_INIT_CFM);
			SET_CM_STATE(connectionUninitialised);
			connectionSendInitCfm(theCm->theAppTask, fail, bluetooth_unknown);
			break;

		case CL_INTERNAL_SM_SET_SC_MODE_REQ:
			PRINT(("CL_INTERNAL_SM_SET_SC_MODE_REQ\n"));
            handleSetSecurityModeReq(&theCm->smState, (CL_INTERNAL_SM_SET_SC_MODE_REQ_T *)message);
		    break;

		case CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ:
		    PRINT(("CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ\n"));
			handleSetSspSecurityLevelReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T *)message);
		    break;

		case CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ:
		    PRINT(("CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ\n"));
            handleSecModeConfigReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T *)message);
		    break;

		case CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ:
 			PRINT(("CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ\n"));
 			connectionHandleReadLocalVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *)message);
 			break; 

		case CL_INTERNAL_DM_SET_BT_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_SET_BT_VERSION_REQ\n"));
			connectionHandleSetBtVersionReq(&theCm->infoState, (CL_INTERNAL_DM_SET_BT_VERSION_REQ_T *)message);
			break;

		default:  
			/* Prims we are not handling - for now panic the app */
			handleUnexpected(connectionUnhandledMessage, theCm->state, id);
			break;
	}
}

/****************************************************************************
NAME	
    connectionBluestackHandlerTestMode

DESCRIPTION
    Message handler for the test-mode state.

RETURNS
    void	
*/
static void connectionBluestackHandlerTestMode(connectionState *theCm, MessageId id, Message message)
{
    /* Depending upon the message id...*/
	switch (id)
	{
        case CL_INTERNAL_SM_SET_SC_MODE_REQ:
		    PRINT(("CL_INTERNAL_SM_SET_SC_MODE_REQ\n"));
            handleSetSecurityModeReq(&theCm->smState, (CL_INTERNAL_SM_SET_SC_MODE_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ:
		    PRINT(("CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ\n"));
			handleSetSspSecurityLevelReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ:
		    PRINT(("CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ\n"));
            handleSecModeConfigReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T*)message);
		    break;

		case CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ\n"));
			connectionHandleWriteScanEnableRequest((CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ\n"));
			connectionHandleReadLocalVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_SET_BT_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_SET_BT_VERSION_REQ\n"));
			connectionHandleSetBtVersionReq(&theCm->infoState, (CL_INTERNAL_DM_SET_BT_VERSION_REQ_T*)message);
			break;

		default:    
			/* Prims we are not handling - for now panic the app */
			handleUnexpected(connectionUnhandledMessage, theCm->state, id);
			break;
	}
}

/****************************************************************************
NAME	
    connectionBluestackHandlerReady	

DESCRIPTION
    Message handler for the ready state.

RETURNS
    void	
*/
static void connectionBluestackHandlerReady(connectionState *theCm, MessageId id, Message message)
{
    /* Depending upon the message id...*/
	switch(id)
	{
		case CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ:
		    PRINT(("CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ\n"));
            handleSecModeConfigReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ:
		    PRINT(("CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ\n"));
			handleSetSspSecurityLevelReq(&theCm->smState, &theCm->infoState, (CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ:
			PRINT(("CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ\n"));
			connectionHandleReadLocalOobDataReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T*)message);
			break;

		case CL_INTERNAL_SM_AUTHENTICATION_REQ:
			PRINT(("CL_INTERNAL_SM_AUTHENTICATION_REQ\n"));
			connectionHandleAuthenticationReq(&theCm->smState, (CL_INTERNAL_SM_AUTHENTICATION_REQ_T*)message);
			break;
					
		case CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ:
			PRINT(("CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ\n"));
			connectionHandleCancelAuthenticationReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T*)message);
			break;

		case CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND\n"));
			connectionHandleAuthenticationTimeout(&theCm->smState);
		    break;

        case CL_INTERNAL_SM_REGISTER_REQ:
		    PRINT(("CL_INTERNAL_SM_REGISTER_REQ\n"));
            handleRegisterReq((CL_INTERNAL_SM_REGISTER_REQ_T*)message);
		    break;

        case CL_INTERNAL_SM_UNREGISTER_REQ:
		    PRINT(("CL_INTERNAL_SM_UNREGISTER_REQ\n"));
            handleUnRegisterReq((CL_INTERNAL_SM_UNREGISTER_REQ_T*)message);
		    break;

        case CL_INTERNAL_SM_REGISTER_OUTGOING_REQ:
		    PRINT(("CL_INTERNAL_SM_REGISTER_OUTGOING_REQ\n"));
            handleRegisterOutgoingReq((CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ:
            PRINT(("CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ\n"));
            handleUnRegisterOutgoingReq((CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T*)message);
		    break;

        case CL_INTERNAL_SM_ENCRYPT_REQ:
		    PRINT(("CL_INTERNAL_SM_ENCRYPT_REQ\n"));
            handleEncryptReq(&theCm->smState, (CL_INTERNAL_SM_ENCRYPT_REQ_T *)message);
		    break;

		case CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ:
		    PRINT(("CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ\n"));
            handleEncryptionKeyRefreshReq(&theCm->infoState, (CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T *)message);
		    break;

        case CL_INTERNAL_SM_PIN_REQUEST_RES:
		    PRINT(("CL_INTERNAL_SM_PIN_REQUEST_RES\n"));
            handlePinRequestRes((CL_INTERNAL_SM_PIN_REQUEST_RES_T*)message);
		    break;

		case CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES\n"));
            handleIoCapabilityRequestRes(&theCm->smState, (CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T*)message);
		    break;
	
		case CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES\n"));
            handleUserConfirmationRequestRes((CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T*)message);
		    break;
	
		case CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES:
			PRINT(("CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES\n"));
            handleUserPasskeyRequestRes((CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T*)message);
		    break;
	
		case CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ:
			PRINT(("CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ\n"));
            handleSendKeypressNotificationReq((CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T*)message);
		    break;

		case CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ:
			PRINT(("CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ\n"));
            handleSetTrustLevelReq(&theCm->infoState, (CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T*)message);
		    break;
	
	    case CL_INTERNAL_SM_AUTHORISE_RES:
			PRINT(("CL_INTERNAL_SM_AUTHORISE_RES\n"));
            handleAuthoriseRes((CL_INTERNAL_SM_AUTHORISE_RES_T*)message);
		    break; 

		case CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ:
		    PRINT(("CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ\n"));
            handleAddAuthDeviceReq(&theCm->infoState, &theCm->smState, (CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *)message);
		    break; 

		case CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ:
		    PRINT(("CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ\n"));
            handleGetAuthDeviceReq(&theCm->smState, (CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T *)message);
		    break; 

        case CL_INTERNAL_SM_SET_SC_MODE_REQ:
		    PRINT(("CL_INTERNAL_SM_SET_SC_MODE_REQ\n"));
            handleSetSecurityModeReq(&theCm->smState, (CL_INTERNAL_SM_SET_SC_MODE_REQ_T*)message);
		    break;

		case CL_INTERNAL_DM_DUT_REQ:
			PRINT(("CL_INTERNAL_DM_DUT_REQ\n"));
			SET_CM_STATE(connectionTestMode);
			connectionHandleEnterDutModeReq(&theCm->infoState);
			break; 

		case CL_INTERNAL_DM_SET_BT_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_SET_BT_VERSION_REQ\n"));
			connectionHandleSetBtVersionReq(&theCm->infoState, (CL_INTERNAL_DM_SET_BT_VERSION_REQ_T*)message);
			break;

		case CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ\n"));
			connectionHandleReadLocalVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ\n"));
			connectionHandleWriteScanEnableRequest((CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_INQUIRY_REQ:
			PRINT(("CL_INTERNAL_DM_INQUIRY_REQ\n"));
			connectionHandleInquiryStart(&theCm->inqState, (CL_INTERNAL_DM_INQUIRY_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_INQUIRY_CANCEL_REQ:
			PRINT(("CL_INTERNAL_DM_INQUIRY_CANCEL_REQ\n"));
			connectionHandleInquiryCancel(&theCm->inqState, (CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_READ_REMOTE_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_READ_REMOTE_NAME_REQ\n"));
			connectionHandleReadRemoteName(&theCm->inqState, (CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_READ_LOCAL_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LOCAL_NAME_REQ\n"));
			connectionHandleReadLocalName(&theCm->inqState, (CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ:
			PRINT(("CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ\n"));
			connectionHandleChangeLocalName((CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T *)message);
			break;
	
		case CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ\n"));
			connectionHandleWriteInquiryTx(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_READ_INQUIRY_TX_REQ:
			PRINT(("CL_INTERNAL_DM_READ_INQUIRY_TX_REQ\n"));
			connectionHandleReadInquiryTx(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_WRITE_EIR_DATA_REQ:
		    PRINT(("CL_INTERNAL_DM_WRITE_EIR_DATA_REQ\n"));
			connectionHandleWriteEirDataRequest(&theCm->infoState, (CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_READ_EIR_DATA_REQ:
		    PRINT(("CL_INTERNAL_DM_READ_EIR_DATA_REQ\n"));
			connectionHandleReadEirDataRequest(&theCm->infoState, &theCm->inqState, (CL_INTERNAL_DM_READ_EIR_DATA_REQ_T *)message);
			break;

		case CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ:
			PRINT(("CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ\n"));
			connectionHandleReadClassOfDeviceRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ\n"));
			connectionHandleWritePageScanActivityRequset((CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ\n"));
			connectionHandleWriteInquiryScanActivityRequest((CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ\n"));
			connectionHandleWriteCodRequset((CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ\n"));
			connectionHandleWriteCachedPageModeRequest((CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ\n"));
			connectionHandleWriteCachedClkOffsetRequest((CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ:
			PRINT(("CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ\n"));
			connectionHandleClearParamCacheRequest((CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ\n"));
			connectionHandleWriteFlushTimeoutRequest((CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_IAC_LAP_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_IAC_LAP_REQ\n"));
			connectionHandleWriteIacLapRequest(&theCm->inqState, (CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ\n"));
			connectionHandleWriteInquiryModeRequest(&theCm->inqState, (CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T *)message);
			break; 
			
		case CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ:
			PRINT(("CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ\n"));
			connectionHandleReadInquiryModeRequest(&theCm->inqState, (CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T *)message);
			break; 
			
		case CL_INTERNAL_DM_READ_BD_ADDR_REQ:
			PRINT(("CL_INTERNAL_DM_READ_BD_ADDR_REQ\n"));
			connectionHandleReadAddrRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_BD_ADDR_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_READ_LINK_QUALITY_REQ:
			PRINT(("CL_INTERNAL_DM_READ_LINK_QUALITY_REQ\n"));
			connectionHandleReadLinkQualityRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T *)message);
			break; 

		case CL_INTERNAL_DM_READ_RSSI_REQ:
			PRINT(("CL_INTERNAL_DM_READ_RSSI_REQ\n"));
			connectionHandleReadRssiRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_RSSI_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_READ_CLK_OFFSET_REQ:
			PRINT(("CL_INTERNAL_DM_READ_CLK_OFFSET_REQ\n"));
			connectionHandleReadclkOffsetRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ:
			PRINT(("CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ\n"));
			connectionHandleReadRemoteSupportedFeaturesRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_EN_ACL_DETACH_REQ:
			PRINT(("CL_INTERNAL_DM_EN_ACL_DETACH_REQ\n"));
			connectionHandleDmEnAclDetachReq((CL_INTERNAL_DM_EN_ACL_DETACH_REQ_T*)message);
			break;

 		case CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ:
 			PRINT(("CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ\n"));
 			connectionHandleReadRemoteVersionRequest(&theCm->infoState, (CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T *) message);
 			break; 
			
		case CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ:
 			PRINT(("CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ\n"));
			connectionHandleSmChangeLinkKeyReq((CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T*) message);
			break; 

		case CL_INTERNAL_SDP_REGISTER_RECORD_REQ:
			PRINT(("CL_INTERNAL_SDP_REGISTER_RECORD_REQ\n"));
			connectionHandleSdpRegisterRequest(&theCm->sdpState, (CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ:
			PRINT(("CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ\n"));
			connectionHandleSdpUnregisterRequest(&theCm->sdpState, (CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ:
			PRINT(("CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ\n"));
			connectionHandleSdpServerConfigMtu(&theCm->sdpState, (CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T *)message);
			break; 
			
		case CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ:
			PRINT(("CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ\n"));
			connectionHandleSdpClientConfigMtu(&theCm->sdpState, (CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_OPEN_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_OPEN_SEARCH_REQ\n"));
			connectionHandleSdpOpenSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_CLOSE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_CLOSE_SEARCH_REQ\n"));
			connectionHandleSdpCloseSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_SERVICE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_SERVICE_SEARCH_REQ\n"));
			connectionHandleSdpServiceSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ:
			PRINT(("CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ\n"));
			connectionHandleSdpAttributeSearchRequest(&theCm->sdpState, (CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ\n"));
			connectionHandleSdpServiceSearchAttrRequest(&theCm->sdpState, (CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T *)message);
			break; 

		case CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ:
			PRINT(("CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ\n"));
			connectionHandleSdpTerminatePrimitiveRequest(&theCm->sdpState, (CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T *)message);
			break; 

		case CL_INTERNAL_L2CAP_REGISTER_REQ:
			PRINT(("CL_INTERNAL_L2CAP_REGISTER_REQ\n"));
			connectionHandleL2capRegisterReq(&theCm->l2capState, (CL_INTERNAL_L2CAP_REGISTER_REQ_T *)message);
			break; 

		case CL_INTERNAL_L2CAP_UNREGISTER_REQ:
			PRINT(("CL_INTERNAL_L2CAP_UNREGISTER_REQ\n"));
			connectionHandleL2capUnregisterReq((CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *)message);
			break; 

		case CL_INTERNAL_L2CAP_CONNECT_REQ:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_REQ\n"));
			connectionHandleL2capConnectReq((CL_INTERNAL_L2CAP_CONNECT_REQ_T *)message);
			break; 

		case CL_INTERNAL_L2CAP_CONNECT_RES:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_RES\n"));
			connectionHandleL2capConnectRes((CL_INTERNAL_L2CAP_CONNECT_RES_T *) message);
			break; 

		case CL_INTERNAL_L2CAP_DISCONNECT_REQ:
			PRINT(("CL_INTERNAL_L2CAP_DISCONNECT_REQ\n"));
			connectionHandleL2capDisconnectReq((CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *) message);
			break; 

		case CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND\n"));
			connectionHandleL2capConnectTimeout((CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND_T *)message);
			break; 

        case CL_INTERNAL_RFCOMM_REGISTER_REQ:
		    PRINT(("CL_INTERNAL_RFCOMM_REGISTER_REQ\n"));
			connectionHandleRfcommRegisterReq(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_REGISTER_REQ_T *)message);
			break; 

        case CL_INTERNAL_RFCOMM_CONNECT_REQ:
		    PRINT(("CL_INTERNAL_RFCOMM_CONNECT_REQ\n"));
			connectionHandleRfcommConnectReq(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_CONNECT_REQ_T *)message);
			break; 

        case CL_INTERNAL_RFCOMM_CONNECT_RES:
		    PRINT(("CL_INTERNAL_RFCOMM_CONNECT_RES\n"));
			connectionHandleRfcommConnectRes((CL_INTERNAL_RFCOMM_CONNECT_RES_T *)message);
			break; 
    
        case CL_INTERNAL_RFCOMM_DISCONNECT_REQ:
		    PRINT(("CL_INTERNAL_RFCOMM_DISCONNECT_REQ\n"));
			connectionHandleRfcommDisconnectReq((CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T *)message);
			break; 

		case CL_INTERNAL_RFCOMM_CONTROL_REQ:
			PRINT(("CL_INTERNAL_RFCOMM_CONTROL_REQ\n"));
			connectionHandleRfcommControlReq((CL_INTERNAL_RFCOMM_CONTROL_REQ_T *)message);
			break; 

        case CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND\n"));
			connectionHandleRfcommRegisterTimeout((CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND_T *)message);
			break; 

        case CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND\n"));
			connectionHandleRfcommConnectTimeout(&theCm->rfcommState, (CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND_T *)message);
			break; 

		case CL_INTERNAL_SYNC_REGISTER_REQ:
			PRINT(("CL_INTERNAL_SYNC_REGISTER_REQ\n"));
			connectionHandleSyncRegisterReq((CL_INTERNAL_SYNC_REGISTER_REQ_T *) message);
			break; 

		case CL_INTERNAL_SYNC_UNREGISTER_REQ:
			PRINT(("CL_INTERNAL_SYNC_UNREGISTER_REQ\n"));
			connectionHandleSyncUnregisterReq((CL_INTERNAL_SYNC_UNREGISTER_REQ_T *) message);
			break; 

		case CL_INTERNAL_SYNC_CONNECT_REQ:
			PRINT(("CL_INTERNAL_SYNC_CONNECT_REQ\n"));
			connectionHandleSyncConnectReq((CL_INTERNAL_SYNC_CONNECT_REQ_T *) message);
			break; 

		case CL_INTERNAL_SYNC_CONNECT_RES:
			PRINT(("CL_INTERNAL_SYNC_CONNECT_RES\n"));
			connectionHandleSyncConnectRes((CL_INTERNAL_SYNC_CONNECT_RES_T *) message);
			break; 

		case CL_INTERNAL_SYNC_DISCONNECT_REQ:
			PRINT(("CL_INTERNAL_SYNC_DISCONNECT_REQ\n"));
			connectionHandleSyncDisconnectReq((CL_INTERNAL_SYNC_DISCONNECT_REQ_T *) message);
			break; 
    
		case CL_INTERNAL_SYNC_RENEGOTIATE_REQ:
			PRINT(("CL_INTERNAL_SYNC_RENEGOTIATE_REQ\n"));
			connectionHandleSyncRenegotiateReq((CL_INTERNAL_SYNC_RENEGOTIATE_REQ_T *) message);
			break; 
    
		case CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND\n"));
			connectionHandleSyncRegisterTimeoutInd((CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND_T *) message);
			break; 

		case CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND:
			PRINT(("CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND\n"));
			connectionHandleSyncUnregisterTimeoutInd((CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND_T *) message);
			break; 
				
		case CL_INTERNAL_DM_SET_ROLE_REQ:
			PRINT(("CL_INTERNAL_DM_SET_ROLE_REQ\n"));
			connectionHandleLinkPolicySetRoleReq(&theCm->linkPolicyState, (CL_INTERNAL_DM_SET_ROLE_REQ_T *) message);
			break; 
	
		case CL_INTERNAL_DM_GET_ROLE_REQ:
			PRINT(("CL_INTERNAL_DM_GET_ROLE_REQ\n"));
			connectionHandleLinkPolicyGetRoleReq(&theCm->linkPolicyState, (CL_INTERNAL_DM_GET_ROLE_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ:
			PRINT(("CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ\n"));
			connectionHandleSetLinkSupervisionTimeoutReq((CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T *) message);
			break; 

		case CL_INTERNAL_DM_SET_LINK_POLICY_REQ:
			PRINT(("CL_INTERNAL_DM_SET_LINK_POLICY_REQ\n"));
			connectionHandleSetLinkPolicyReq((CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T *) message);
			break; 
	
		case CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ:
			PRINT(("CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ\n"));
			connectionHandleSetSniffSubRatePolicyReq(&theCm->infoState, (CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T *) message);
			break; 

		case CL_INTERNAL_SM_GET_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SM_GET_ATTRIBUTE_REQ"));
			connectionSmHandleGetAttributeReq(theCm->theAppTask, (CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T *) message);
			break; 

		case CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ:
			PRINT(("CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ"));
			connectionSmHandleGetIndexedAttributeReq(theCm->theAppTask, (CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T *) message);
			break; 

        case CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP:
			PRINT(("CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP"));
			connectionHandleL2capInterlockDisconnectRsp((CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP_T*)message);
			break; 

		default:   
			/* Prims we are not handling - for now panic the app */
			handleUnexpected(connectionUnhandledMessage, theCm->state, id);
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
    connectionState *theCm = (connectionState *)task;
	connectionStates state = theCm->state;
    
	PRINT(("connectionBluestackHandler - Id = 0x%x\n",id));

	/* Handle Bluestack primitives seperately */
	switch (id)
	{
		case MESSAGE_BLUESTACK_DM_PRIM:
			connectionBluestackHandlerDm(theCm, (DM_UPRIM_T *)message);
			break;
			
		case MESSAGE_BLUESTACK_RFCOMM_PRIM:
			connectionBluestackHandlerRfcomm(theCm, (RFCOMM_UPRIM_T *)message);
			break;
	
		case MESSAGE_BLUESTACK_L2CAP_PRIM:           
			connectionBluestackHandlerL2cap(theCm, (L2CA_UPRIM_T *)message);
			break;
	
		case MESSAGE_BLUESTACK_SDP_PRIM:
			connectionBluestackHandlerSdp(theCm, (SDS_UPRIM_T *)message);
			break;
		
		case MESSAGE_BLUESTACK_UDP_PRIM:
		case MESSAGE_BLUESTACK_TCP_PRIM:          
			handleUnexpected(connectionUnhandledMessage, theCm->state, id);
			break;

		/* CL_SDP_CLOSE_SEARCH_CFM Primitive arrived as a result of an internal 
		   call to close SDP search, can't avoid so ignore
		   Handled as a special case to allow the compiler to generate better
		   code for the previous switch statements. */
		case CL_SDP_CLOSE_SEARCH_CFM:
			break;
		
		/* Everything else must be internal connection library primitives */	
		default:
		{
			switch (state)
			{
				case connectionReady:
					connectionBluestackHandlerReady(theCm, id, message);
					break;

				case connectionUninitialised:
					connectionBluestackHandlerUninitialised(theCm, id, message);
					break;
		
				case connectionInitialising:
					connectionBluestackHandlerInitialising(theCm, id, message);
					break;
		
				case connectionTestMode:
					connectionBluestackHandlerTestMode(theCm, id, message);
					break;
			}
		}
	}
}

/*lint +e655 +e525 +e830 */
