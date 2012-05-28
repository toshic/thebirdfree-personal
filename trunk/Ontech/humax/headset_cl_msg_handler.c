/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_cl_msg_handler.c
@brief    Handle connection library messages arriving at the app.
*/


#include "headset_auth.h"
#include "headset_debug.h"
#include "headset_cl_msg_handler.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_inquiry.h"
#include "headset_private.h"
#include "headset_scan.h"
#include "headset_statemanager.h"
#include "uart.h"

#include <connection.h>
#include <panic.h>
#include <pio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_CL_MSG
#define CL_MSG_DEBUG(x) DEBUG(x)
#else
#define CL_MSG_DEBUG(x) 
#endif


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************/
void handleCLMessage( Task task, MessageId id, Message message )
{
    switch (id)
    {
    case CL_INIT_CFM:
        CL_MSG_DEBUG(("CL_INIT_CFM\n"));
        if(((CL_INIT_CFM_T *)message)->status == success)
        {
			/* Set the class of device to indicate this is a headset */
    		ConnectionWriteClassOfDevice(AUDIO_MAJOR_SERV_CLASS | AV_COD_RENDER | AV_MAJOR_DEVICE_CLASS | AV_MINOR_HEADSET);
    
    		/* Configure security */
    		ConnectionSmSetSecurityLevel(0, 1, ssp_secl4_l0, TRUE, FALSE, FALSE);
			/* Reset sec mode config - always turn off debug keys on power on */
    		ConnectionSmSecModeConfig(&theHeadset.task,
                             cl_sm_wae_acl_owner_none,
                             FALSE,
                             TRUE);
						
			/* Initialise Inquiry Data to NULL */
            theHeadset.inquiry_data = NULL;
			
			/* Start initialisation of config */
			InitUserFeatures();       
           
/*			InitHfp();*/
            InitA2dp();

        }
        else
            Panic();
        break;
    case CL_DM_WRITE_INQUIRY_MODE_CFM:
		/* Read the local name to put in our EIR data */
		ConnectionReadInquiryTx(&theHeadset.task);
		break;
	case CL_DM_READ_INQUIRY_TX_CFM:
		theHeadset.inquiry_tx = ((CL_DM_READ_INQUIRY_TX_CFM_T*)message)->tx_power;
		ConnectionReadLocalName(&theHeadset.task);
		break;
    case CL_DM_LOCAL_NAME_COMPLETE:
        CL_MSG_DEBUG(("CL_DM_LOCAL_NAME_COMPLETE\n"));
        /* write the EIR data */
        headsetWriteEirData((CL_DM_LOCAL_NAME_COMPLETE_T*)message);    
        break;
    case CL_SM_USER_CONFIRMATION_REQ_IND:
        CL_MSG_DEBUG(("CL_SM_USER_CONFIRMATION_REQ_IND\n"));
        headsetHandleUserConfirmationInd((CL_SM_USER_CONFIRMATION_REQ_IND_T*)message);
        break;
    case CL_SM_USER_PASSKEY_REQ_IND:
        CL_MSG_DEBUG(("CL_SM_USER_PASSKEY_REQ_IND\n"));
        headsetHandleUserPasskeyInd((CL_SM_USER_PASSKEY_REQ_IND_T*)message);
        break;
    case CL_SM_USER_PASSKEY_NOTIFICATION_IND:
        CL_MSG_DEBUG(("CL_SM_USER_PASSKEY_NOTIFICATION_IND\n"));
        headsetHandleUserPasskeyNotificationInd((CL_SM_USER_PASSKEY_NOTIFICATION_IND_T*)message);
        break;
    case CL_SM_KEYPRESS_NOTIFICATION_IND:
        CL_MSG_DEBUG(("CL_SM_KEYPRESS_NOTIFICATION_IND\n"));
        /* no action required? */
        break;
    case CL_SM_REMOTE_IO_CAPABILITY_IND: 
        CL_MSG_DEBUG(("CL_SM_REMOTE_IO_CAPABILITY_IND\n"));
        headsetHandleRemoteIoCapabilityInd((CL_SM_REMOTE_IO_CAPABILITY_IND_T*)message);
        break;
    case CL_SM_IO_CAPABILITY_REQ_IND:
        CL_MSG_DEBUG(("CL_SM_IO_CAPABILITY_REQ_IND\n"));
        headsetHandleIoCapabilityInd((CL_SM_IO_CAPABILITY_REQ_IND_T*)message);
        break;
    case CL_SM_SEC_MODE_CONFIG_CFM:
        CL_MSG_DEBUG(("CL_SM_SEC_MODE_CONFIG_CFM\n"));
        /* remember if debug keys are on or off */
        theHeadset.debugKeysInUse = ((CL_SM_SEC_MODE_CONFIG_CFM_T*)message)->debug_keys;
        break;
    case CL_SM_PIN_CODE_IND:  
        CL_MSG_DEBUG(("CL_SM_PIN_CODE_IND\n"));
        headsetHandlePinCodeInd((CL_SM_PIN_CODE_IND_T*) message);
        break;
    case CL_SM_AUTHORISE_IND:  
        CL_MSG_DEBUG(("CL_SM_AUTHORISE_IND\n"));
        headsetHandleAuthoriseInd((CL_SM_AUTHORISE_IND_T*) message);
        break;            
    case CL_SM_AUTHENTICATE_CFM:
        CL_MSG_DEBUG(("CL_SM_AUTHENTICATE_CFM\n"));
        headsetHandleAuthenticateCfm((CL_SM_AUTHENTICATE_CFM_T*) message);
        break;     
		
	case CL_DM_LOCAL_BD_ADDR_CFM:
        CL_MSG_DEBUG(("CL_DM_LOCAL_BD_ADDR_CFM\n"));
        break ;
		
	case CL_DM_INQUIRE_RESULT:
        CL_MSG_DEBUG(("CL_DM_INQUIRE_RESULT\n"));
        inquiryHandleResult((CL_DM_INQUIRE_RESULT_T*)message);
        break;
	case CL_DM_REMOTE_NAME_COMPLETE:
	{
		CL_DM_REMOTE_NAME_COMPLETE_T *name = (CL_DM_REMOTE_NAME_COMPLETE_T*)message;
		CL_MSG_DEBUG(("CL_DM_REMOTE_NAME_COMPLETE\n"));

		if(name->status == hci_success)
		{
			char *name_str = PanicUnlessMalloc(name->size_remote_name+1);
			memcpy(name_str,name->remote_name,name->size_remote_name);
			name_str[name->size_remote_name] = 0;
			UartPrintf("\r\n+RNM=%s\r\n",name_str);
			free(name_str);
		}
		else
			UartPrintf("\r\nERROR\r\n");
		
		break;
	}
	case CL_DM_RSSI_CFM:
	{
        CL_MSG_DEBUG(("CL_DM_RSSI_CFM_T %d (%d)\n",((CL_DM_RSSI_CFM_T*)message)->status,((CL_DM_RSSI_CFM_T*)message)->rssi));
		UartPrintf("\r\n+RSSI=%d\r\n",((CL_DM_RSSI_CFM_T*)message)->rssi);
        break;
	}
        
        
    /* Ignored messages */
    case CL_DM_ACL_OPENED_IND:
    case CL_DM_ACL_CLOSED_IND:
    case CL_SM_ENCRYPT_CFM:
    case CL_DM_DUT_CFM:
    case CL_DM_ROLE_CFM:
    case CL_DM_ROLE_IND:
    case CL_DM_SNIFF_SUB_RATING_IND:
    case CL_DM_LINK_SUPERVISION_TIMEOUT_IND:
        break;
        
    default:   
        CL_MSG_DEBUG(("CL UNHANDLED MSG: 0x%x\n",id));
        break;
    }    
}
