/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_handler.c        

DESCRIPTION
    This files contains the implementation of the Security Entity API for
    the Connection Library.  The Application Task and/or Profile Libraries
    make calls to these functions to manage all Bluetooth security related
    functionality.  In order to correctly manage the Connection Library 
    state machine calls to these API's result in a private message being 
    posted to the main Connection Library task handler.  Depending upon the
    current state of the Connection Library these messages are handled
    appropriately.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "init.h"
#include "dm_security_auth.h"
#include "dm_security_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>


#define AUTH_BITS_SET(AUTH_REQ, BITS) bool auth_bits_set = (AUTH_REQ & BITS) ? TRUE:FALSE;


/*****************************************************************************/
static void aclOpen(const bdaddr* bd_addr)
{
    MAKE_PRIM_T(DM_ACL_OPEN_REQ); 
	connectionConvertBdaddr_t(&prim->bd_addr, bd_addr);
    VmSendDmPrim(prim);
}


/*****************************************************************************/
static void aclClose(const bdaddr* bd_addr)
{
    MAKE_PRIM_T(DM_ACL_CLOSE_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, bd_addr);
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    startBonding	

DESCRIPTION
    This function is called to send a DM_SM_BONDING_REQ to bluestack

RETURNS
	
*/
static void startBonding(const bdaddr* bd_addr)
{
    MAKE_PRIM_T(DM_SM_BONDING_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, bd_addr);
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    cancelBonding	

DESCRIPTION
    This function is called to send a DM_SM_BONDING_CANCEL_REQ to bluestack

RETURNS
	
*/
static void cancelBonding(const bdaddr* bd_addr, bool force)
{
	MAKE_PRIM_T(DM_SM_BONDING_CANCEL_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, bd_addr);
	prim->force = force;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionSendAuthenticateCfm	

DESCRIPTION
    This function is called to send a CL_SM_AUTHENTICATE_CFM message to the
    specified task.  This indicates that a pairing operation has completed.  
    The message confirms the Bluetooth Device Address of the device just 
    paired with and a result code

RETURNS
	
*/
static void connectionSendAuthenticateCfm(Task task, const bdaddr* bd_addr, authentication_status status, cl_sm_link_key_type key_type, bool bonded)
{
    /*
        This message indicates that a pairing operation has completed.  
        The message confirms the Bluetooth device address of the device just 
        paired with and a result code indicatiing the outcome of the pairing attempt.
    */
    if (task)
    {
        MAKE_CL_MESSAGE(CL_SM_AUTHENTICATE_CFM);
        message->bd_addr = *bd_addr;
        message->status = status;
		message->key_type = key_type;
		message->bonded = bonded;
        MessageSend(task, CL_SM_AUTHENTICATE_CFM, message);
    }
}


/****************************************************************************
    This function is called when a CL_INTERNAL_SM_SET_SC_MODE_CFM arrives at
    the CL with a destination task of the CL.  This will occur as a result of
    DM_SM_SET_SEC_MODE_REQ being sent from within the CL.  This message
    is sent during the Authentication process.	
*/
static void handleInternalSetSecurityModeCfm(connectionSmState* smState, const DM_SM_SET_SEC_MODE_CFM_T* cfm)
{
    /* Check to determine if Authentication is active */
    if(smState->authReqLock)
    {
            /* If the Peer Bluetooth Device Address is non zero then establish
               an ACL connection to start the Authentication Process */
        if(cfm->success)
        {
            /* If the Peer Bluetooth Device Address is non zero then...*/ 
            if(!BdaddrIsZero(&smState->authRemoteAddr))
                	aclOpen(&smState->authRemoteAddr);
            else
                /* Reset the Authentication request lock */
                smState->authReqLock = 0;
        }
    }
}


/*****************************************************************************/
static void endAuthentication(connectionSmState* smState, authentication_status status)
{	
	/* If Authentication has failed then let the application know about it */
	if(status != auth_status_success)
	{		
		connectionSendAuthenticateCfm(smState->authReqLock, &smState->authRemoteAddr, status, cl_sm_link_key_none, FALSE);
	}
	/* If Authentication timed out, ensure ACL connection is closed */
	if(status == auth_status_timeout)
	{		
		if(smState->security_mode != sec_mode4_ssp)
			aclClose(&smState->authRemoteAddr);
		else
			cancelBonding(&smState->authRemoteAddr, FALSE);
	}
	else
	{
		/* Cancel any pending authentication timeout */
		(void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND);
	}
	
    /* Zero storage of peer device */
    BdaddrSetZero(&smState->authRemoteAddr);

    /* Restore the security settings, the fact that the peer Bluetooth Device
       Address is zero is used to decode the fact that Authentication has ended.
       The resulting CFM handler will reset the Authentication Lock */
	if(smState->security_mode != sec_mode4_ssp)
	    ConnectionSmSetSecurityMode(connectionGetCmTask(), smState->security_mode, smState->enc_mode);
	else
		smState->authReqLock = 0;

}


/****************************************************************************/
static void sendEncryptionChangeInd(Task clientTask, Sink sink, bool encrypted)
{
    if (clientTask)
    {
        MAKE_CL_MESSAGE(CL_SM_ENCRYPTION_CHANGE_IND);
        message->sink = sink;
        message->encrypted = encrypted;
        MessageSend(clientTask, CL_SM_ENCRYPTION_CHANGE_IND, message);
    }
}


/****************************************************************************/
static void sendAclOpenedIndToClient(Task task, BD_ADDR_T *addr, bool incoming, uint24_t dev_class, hci_status status)
{
    const msg_filter *msgFilter = connectionGetMsgFilter();

    if (task && (msgFilter[0] & msg_group_acl))
    {
        MAKE_CL_MESSAGE(CL_DM_ACL_OPENED_IND);
        connectionConvertBdaddr(&message->bd_addr, addr);
        message->incoming = incoming;
        message->dev_class = dev_class;
        message->status = status;
        MessageSend(task, CL_DM_ACL_OPENED_IND, message);
    }
}


/****************************************************************************/
static void handleGenericAclClosedInd(Task task, connectionSmState *smState, BD_ADDR_T *dev_addr, hci_status status)
{
    bdaddr	addr;    
    const msg_filter *msgFilter = connectionGetMsgFilter();

	connectionConvertBdaddr(&addr, dev_addr);

    /* Check to determine if Authentication is active */
    if(smState->authReqLock)
    {
        /* Authentication is active, an ACL connection to the peer device has just been
           dropped, therefore Authentication has ended */
        if(BdaddrIsSame(&smState->authRemoteAddr, &addr))
			/* In mode 4 we end auth on the bonding cfm so dont handle here */
			if(smState->security_mode != sec_mode4_ssp)
				/* End Authentication */
				endAuthentication(smState, auth_status_success);
    }
    
    if (task && (msgFilter[0] & msg_group_acl))
    {
        MAKE_CL_MESSAGE(CL_DM_ACL_CLOSED_IND);
        message->bd_addr = addr;
        message->status = status;
        MessageSend(task, CL_DM_ACL_CLOSED_IND, message);
    }
}


/****************************************************************************
NAME	
    connectionHandleSmAddDeviceCfm	

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library is
	initialising.

RETURNS
	
*/
void connectionHandleSmAddDeviceCfm(connectionSmState* smState, const DM_SM_ADD_DEVICE_CFM_T* cfm)
{
    /* 
       During initialisation we expect to see a CFM for every device added to
       the database from the entries in the trusted device list
    */
    if(cfm->success)
    {
        if(++smState->deviceCount == smState->noDevices)
        {
            /* Confirm Sm entity has been initialised */
            connectionSendInternalInitCfm(connectionInitSm);
        }
    }
}

/****************************************************************************
NAME	
    connectionHandleSmAddDeviceCfmReady	

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library has been
	initialised.

RETURNS
	
*/
void connectionHandleSmAddDeviceCfmReady(connectionSmState* smState, const DM_SM_ADD_DEVICE_CFM_T* cfm)
{
	if (smState->deviceReqLock)
	{
		bdaddr	addr;
	
		/* Convert from CSR to CCL format */
		connectionConvertBdaddr(&addr, &cfm->bd_addr);

		/* Send message to application */
		{
			MAKE_CL_MESSAGE(CL_SM_ADD_AUTH_DEVICE_CFM);
			message->bd_addr = addr;
			message->status = (cfm->success) ? success : fail;
			MessageSend(smState->deviceReqLock, CL_SM_ADD_AUTH_DEVICE_CFM, message);
		}
	}

	/* Reset lock */
	smState->deviceReqLock = 0;
}


/****************************************************************************
NAME	
    connectionHandleSmAccessInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_ACCESS_IND from Bluestack

RETURNS
	
*/
void connectionHandleSmAccessInd(connectionSdpState* sdpState, const DM_SM_ACCESS_IND_T* ind)
{
	bdaddr bd_addr;
	connectionConvertBdaddr(&bd_addr, &ind->bd_addr);
	
	/* If this is not an indication for SDP and the address matches one we're searching... */
	if(!(ind->protocol_id == SEC_PROTOCOL_L2CAP && ind->channel == 1) && BdaddrIsSame(&bd_addr, &sdpState->sdpServerAddr))
	{
		/* If the connection lib has a search open we're SDP Pinging */
		if(sdpState->sdpLock == connectionGetCmTask())
		{
			/* If we are waiting for a search result... */
			if(sdpState->sdpSearchLock == connectionGetCmTask())
			{
				/* Free the search lock so the result is binned and we dont send a new request */
				sdpState->sdpSearchLock = 0;
			}
			else
			{
				/* If we're not waiting for a result there must be a pending message */
				MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ);
				sdpState->sdpSearchLock = 0;
			}
		
		/* Close the search */
		ConnectionSdpCloseSearchRequest(connectionGetCmTask());
		}
	}
}


/****************************************************************************
NAME	
    connectionHandleSmLinkKeyReqInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_LINK_KEY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmLinkKeyReqInd(const DM_SM_LINK_KEY_REQUEST_IND_T* ind)
{
	bdaddr	addr;
	
	/* Convert from CSR to CCL format */
	connectionConvertBdaddr(&addr, &ind->bd_addr);
    connectionAuthSendLinkKey(&addr);
}


/****************************************************************************
NAME	
    connectionHandleSmSspLinkKeyReqInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_SSP_LINK_KEY_REQUEST_IND
    from the Bluestack Security Manager. 

RETURNS

*/
void connectionHandleSmSspLinkKeyReqInd(Task theAppTask, connectionSmState* smState, const DM_SM_SSP_LINK_KEY_REQUEST_IND_T* ind)
{
    bdaddr	addr;
	
    /* Convert from CSR to CCL format */
    connectionConvertBdaddr(&addr, &ind->bd_addr);
	
    /* If we responded negative */
    if(!connectionAuthSendSspLinkKey(&addr, ind->authenticated))
    {			
        /* Start SDP Ping for legacy devs */
        if(!ind->rhsf && ind->initiated_outgoing)
            ConnectionSdpOpenSearchRequest(connectionGetCmTask(), &addr);
    }
}


/****************************************************************************
NAME	
    connectionHandleSmLinkKeyInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_LINK_KEY_IND
    from the Bluestack Security Manager.  This indicates that a pairing
    operation has been completed.  This function will store the link key
    for the device specified in the Trusted Device List and send a message
    to the Application task

RETURNS

*/
void connectionHandleSmLinkKeyInd(Task theAppTask, connectionReadInfoState* infoState, connectionSmState* smState, const DM_SM_LINK_KEY_IND_T* ind)
{
	bool bonded = FALSE;
	cl_sm_link_key_type key_type = connectionConvertLinkKeyType(ind->key_type);	
	bdaddr	addr;
	
	/* Check if we're bonding */
	AUTH_BITS_SET(smState->authentication_requirements, (HCI_MITM_NOT_REQUIRED_GENERAL_BONDING | HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING | AUTH_REQ_UNKNOWN));
	
	connectionConvertBdaddr(&addr, &ind->bd_addr);
			
	/* If legacy pairing or bonding (and not debug key) store key */
	if((key_type == cl_sm_link_key_legacy) || (auth_bits_set && (key_type != cl_sm_link_key_debug)))
	{
		bonded = connectionAuthAddDevice(infoState->version, &addr, key_type, ind->key, FALSE, TRUE);
	}
	/* If non bonding (and not debug key) register key */
	else if(!auth_bits_set && key_type !=cl_sm_link_key_debug)
	{
		bonded = FALSE;
		(void) connectionAuthAddDevice(infoState->version, &addr, key_type, ind->key, FALSE, FALSE);
	}
	
	/* For legacy devs this is where authentication ends */
	if(smState->security_mode == sec_mode4_ssp && key_type == cl_sm_link_key_legacy)
		endAuthentication(smState, auth_status_success);

	/* Reset authentication requirements + send cfm to app */
	smState->authentication_requirements = AUTH_REQ_UNKNOWN;
	connectionSendAuthenticateCfm(theAppTask, &addr, auth_status_success, key_type, bonded);
}


/****************************************************************************
NAME	
    connectionHandleSmPinReqInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_PIN_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmPinReqInd(Task theAppTask, const DM_SM_PIN_REQUEST_IND_T* ind)
{
    /* Send a message to the Application requesting a PIN code */
    MAKE_CL_MESSAGE(CL_SM_PIN_CODE_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
    MessageSend(theAppTask, CL_SM_PIN_CODE_IND, message);
}


/****************************************************************************
NAME	
    connectionHandleSmIoCapReqInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_IO_CAPABILITY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmIoCapReqInd(Task theAppTask, DM_SM_IO_CAPABILITY_REQUEST_IND_T* ind)
{
	/* Send a message to the Application requesting IO capability */
	MAKE_CL_MESSAGE(CL_SM_IO_CAPABILITY_REQ_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
    MessageSend(theAppTask, CL_SM_IO_CAPABILITY_REQ_IND, message);
}

/****************************************************************************
NAME	
    connectionHandleSmIoCapResInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_IO_CAPABILITY_RESPONSE_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmIoCapResInd(Task theAppTask, connectionSmState* smState, DM_SM_IO_CAPABILITY_RESPONSE_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_SM_REMOTE_IO_CAPABILITY_IND);
	
    connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
    message->authentication_requirements = connectionConvertAuthenticationRequirements(ind->authentication_requirements);
	{	
	
		/* Check we dont already have an internal auth requirement */
		AUTH_BITS_SET(smState->authentication_requirements, AUTH_REQ_UNKNOWN);
	
		/* If remote dev initiated */	
		if(auth_bits_set)
		{
			/* Remember it's requirements (for the dedicated bonding case) */
			smState->authentication_requirements = ind->authentication_requirements;
		}
	}
    message->io_capability = connectionConvertIoCapability(ind->io_capability);
    message->oob_data_present = ind->oob_data_present;
	
    MessageSend(theAppTask, CL_SM_REMOTE_IO_CAPABILITY_IND, message);
}


/****************************************************************************
NAME	
    connectionHandleSmUserConfirmationReqInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_USER_CONFIRMATION_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmUserConfirmationReqInd(Task theAppTask, DM_SM_USER_CONFIRMATION_REQUEST_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_SM_USER_CONFIRMATION_REQ_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	message->numeric_value = ind->numeric_value;
	MessageSend(theAppTask, CL_SM_USER_CONFIRMATION_REQ_IND,message);
}


/****************************************************************************
NAME	
    connectionHandleSmUserPasskeyReqInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_USER_PASSKEY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmUserPasskeyReqInd(Task theAppTask, DM_SM_USER_PASSKEY_REQUEST_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_SM_USER_PASSKEY_REQ_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	MessageSend(theAppTask, CL_SM_USER_PASSKEY_REQ_IND,message);
}


/****************************************************************************
NAME	
    connectionHandleSmUserPasskeyNotificationInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_USER_PASSKEY_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmUserPasskeyNotificationInd(Task theAppTask, connectionSmState* smState, DM_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_SM_USER_PASSKEY_NOTIFICATION_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	message->passkey = ind->passkey;
	MessageSend(theAppTask, CL_SM_USER_PASSKEY_NOTIFICATION_IND,message);
}


/****************************************************************************
NAME	
    connectionHandleSmKeypressNotificationInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_KEYPRESS_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmKeypressNotificationInd(Task theAppTask, DM_SM_KEYPRESS_NOTIFICATION_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_SM_KEYPRESS_NOTIFICATION_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	message->type = connectionConvertKeypressType(ind->notification_type);
	MessageSend(theAppTask, CL_SM_KEYPRESS_NOTIFICATION_IND,message);
}

/****************************************************************************
NAME	
    connectionHandleSmChangeLinkKeyReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ message
    from the connection lib.

RETURNS
	
*/
void connectionHandleSmChangeLinkKeyReq(const CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T* req)
{
	bdaddr bd_addr;
	if(SinkGetBdAddr(req->sink, &bd_addr))
	{
		/* Send DM_HCI_CHANGE_LINK_KEY message to bluestack */
		MAKE_PRIM_C(DM_HCI_CHANGE_LINK_KEY);
		connectionConvertBdaddr_t(&prim->bd_addr, &bd_addr); 
		VmSendDmPrim(prim);
	}
}

/****************************************************************************
NAME	
    connectionHandleSmAuthoriseInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_AUTHORISE_IND from the 
    Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmAuthoriseInd(Task theAppTask, const DM_SM_AUTHORISE_IND_T* ind)
{
    /* Send a message to the Application requesting authorisation for the 
       device to connect */
    MAKE_CL_MESSAGE(CL_SM_AUTHORISE_IND);
    connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
    message->protocol_id = connectionConvertProtocolId_t(ind->protocol_id);
    message->channel = ind->channel;
    message->incoming = ind->incoming;
    MessageSend(theAppTask, CL_SM_AUTHORISE_IND, message);
}


/****************************************************************************
NAME	
    connectionHandleSmSimplePairingCompleteInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_SIMPLE_PAIRING_COMPLETE_IND 
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmSimplePairingCompleteInd(Task theAppTask, connectionSmState* smState, const DM_SM_SIMPLE_PAIRING_COMPLETE_IND_T* ind)
{
	bdaddr bd_addr;
	connectionConvertBdaddr(&bd_addr, &ind->bd_addr);
		
	if(ind->status != HCI_SUCCESS)
	{
		if(!smState->authReqLock)
			connectionSendAuthenticateCfm(theAppTask, &bd_addr, connectionConvertAuthStatus(ind->status), cl_sm_link_key_none, FALSE);	
		
		/* Authentication finished, forget the requirements */
		smState->authentication_requirements = AUTH_REQ_UNKNOWN;	
	}
	
	if(smState->authReqLock)
	{
		endAuthentication(smState, connectionConvertAuthStatus(ind->status));
	}
}


/****************************************************************************
NAME	
    connectionHandleDmAclOpenCfm	

DESCRIPTION
    This function is called when confirmation that a previously requested
    open ACL connection has completed
RETURNS
	
*/
void connectionHandleDmAclOpenCfm(connectionSmState* smState, const DM_ACL_OPEN_CFM_T* cfm)
{
    /* Check to determine if Authentication is active */
    if(smState->authReqLock)
    {
        /* 
           We are currently in the process of performing Authentication with a
           remote device.  At this point if the ACL connection was successful
           then we have successfully paired
        */
        if(cfm->success)
        {
			bdaddr	addr;
			connectionConvertBdaddr(&addr, &cfm->bd_addr);
			
            /* Provided this is the correct device */
            if(BdaddrIsSame(&smState->authRemoteAddr, &addr))
                /* Authentication successful, close the ACL link */
                aclClose(&smState->authRemoteAddr);
        }
        else
        {
			/* End Authentication and notify application task */
			endAuthentication(smState, auth_status_fail);
        }
    }
}


/****************************************************************************
NAME	
    connectionHandleDmAclOpenInd

DESCRIPTION
    An indication from BlueStack that an ACL has been opened. Some clients
	may need this information so pass the indication up to the task 
	registered as the "app task" (we don't know who else to pass this to!).

RETURNS
    void
*/
void connectionHandleDmAclOpenInd(Task task, const DM_ACL_OPENED_IND_T *ind)
{
    sendAclOpenedIndToClient(task, (BD_ADDR_T *)&ind->bd_addr, ind->incoming, ind->dev_class, hci_error_unrecognised);
}


/*****************************************************************************/
void connectionHandleDmEnAclOpenInd(Task task, connectionSmState* smState, const DM_EN_ACL_OPENED_IND_T *ind)
{
    sendAclOpenedIndToClient(task, (BD_ADDR_T *)&ind->bd_addr, ind->incoming, ind->dev_class, connectionConvertHciStatus(ind->status));
	if(smState->authReqLock && smState->security_mode == sec_mode4_ssp)
    {
		authentication_status status = connectionConvertAuthStatus(ind->status);
        /* 
           We are currently in the process of performing Authentication with a
           remote device.  At this point if the ACL open failed authentication
		   failed.
        */
        if(status == auth_status_timeout)
        {
			/* Cancel any pending authentication timeout */
			(void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND);
			/* End Authentication and notify application task */
			endAuthentication(smState, status);
		}
	}
}


/****************************************************************************
NAME	
    connectionHandleDmAclClosedInd	

DESCRIPTION
    This function is called when an ACL connection is closed.  If Authentication 
    is active and we have just paired successfully with the specified peer device
RETURNS
	
*/
void connectionHandleDmAclClosedInd(Task task, connectionSmState *smState, const DM_ACL_CLOSED_IND_T *ind)
{	
	/* Inform the app task that the ACL has been closed. */	
    handleGenericAclClosedInd(task, smState, (BD_ADDR_T *)&ind->bd_addr, hci_error_unrecognised);
}


/*****************************************************************************/
void connectionHandleDmEnAclClosedInd(Task task, connectionSmState *smState, const DM_EN_ACL_CLOSED_IND_T *ind)
{
    handleGenericAclClosedInd(task, smState, (BD_ADDR_T *)&ind->bd_addr, connectionConvertHciStatus(ind->reason));
}


/****************************************************************************
NAME	
    connectionHandleReadLocalOobDataReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ 
	message

RETURNS
	
*/
void connectionHandleReadLocalOobDataReq(connectionReadInfoState* infoState, connectionSmState* smState, const CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T* req)
{
	/* Check we can handle reading OOB data */
	if(infoState->version == bluetooth2_1)
	{
		if(!smState->authReqLock)
    	{
        	/* Use the authReqLock to avoid getting new oob data in the middle of authentication */
        	smState->authReqLock = req->task;
			/* Send request to bluestack */
			{
			MAKE_PRIM_T(DM_SM_READ_LOCAL_OOB_DATA_REQ);  
			prim->unused = 0;
			VmSendDmPrim(prim);
			}
    	}
    	else
    	{
    		/* Authentication procedure in progress, post a conditional internal 
    	 	message to queue request for processing later */
    		MAKE_CL_MESSAGE(CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ);
    		COPY_CL_MESSAGE(req, message);
     		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ, message, &smState->authReqLock);
    	}
	}
	else
	{
		/* Send fail cfm to app - unsupported feature */
		MAKE_CL_MESSAGE(CL_SM_READ_LOCAL_OOB_DATA_CFM);
    	message->status = hci_error_unsupported_feature;
		message->oob_hash_c[0] = 0;
		message->oob_rand_r[0] = 0;
     	MessageSend(req->task, CL_SM_READ_LOCAL_OOB_DATA_CFM, message);
	}
}


/****************************************************************************
NAME	
    connectionHandleReadLocalOobDataCfm	

DESCRIPTION
    This function is called on receipt of a DM_SM_READ_LOCAL_OOB_DATA_CFM message

RETURNS
	
*/
void connectionHandleReadLocalOobDataCfm(connectionSmState* smState, DM_SM_READ_LOCAL_OOB_DATA_CFM_T* cfm)
{
	if(smState->authReqLock)
	{
		MAKE_CL_MESSAGE(CL_SM_READ_LOCAL_OOB_DATA_CFM);
		
		message->status = connectionConvertHciStatus(cfm->status);
		
		if(cfm->status == HCI_SUCCESS)
		{
			/* Get the data from the primitive */
			uint8* oob_data = VmGetPointerFromHandle(cfm->oob_hash_c);
			memcpy(message->oob_hash_c, oob_data, CL_SIZE_OOB_DATA); 
			free(oob_data);
			
			oob_data = VmGetPointerFromHandle(cfm->oob_rand_r); 
			memcpy(message->oob_rand_r, oob_data, CL_SIZE_OOB_DATA);
			free(oob_data);
		}
		else
		{
			memset(message->oob_hash_c, 0, CL_SIZE_OOB_DATA);
			memset(message->oob_rand_r, 0, CL_SIZE_OOB_DATA);
		}
		
		MessageSend(smState->authReqLock, CL_SM_READ_LOCAL_OOB_DATA_CFM, message);
	}
	
	smState->authReqLock = 0;
}


/****************************************************************************
NAME	
    connectionHandleAuthenticationReq	

DESCRIPTION
    This function is called on receipt of a CM_AUTHENTICATE_REQ message

RETURNS
	
*/
void connectionHandleAuthenticationReq(connectionSmState* smState, const CL_INTERNAL_SM_AUTHENTICATION_REQ_T* req)
{
    if(!smState->authReqLock)
    {
        /* Lock Authentication */
        smState->authReqLock = req->task;

        /* Remove device from the trusted device list */
        (void) ConnectionSmDeleteAuthDevice(&req->bd_addr);

        /* Send a timeout message result in a timeout if Authentication fails */
        {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND);
        message->theAppTask = req->task;
        MessageSendLater(connectionGetCmTask(), CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND, message, req->timeout);
        }

        /* Store the Bluetooth device address of the device we are trying to 
           Authenticate with */
        smState->authRemoteAddr = req->bd_addr;
		
        /* Set the security mode to SEC_MODE3_LINK (Wait for CFM) or SEC_MODE4_SSP if 2.1 dev */
        if(smState->security_mode != sec_mode4_ssp)
            ConnectionSmSetSecurityMode(connectionGetCmTask(), sec_mode3_link, smState->enc_mode);
        else
        {
            smState->authentication_requirements = HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING;
            startBonding(&smState->authRemoteAddr);
        }
    }
    else
    {
        /* Authentication procedure in progress, post a conditional internal 
           message to queue request for processing later */
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHENTICATION_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_AUTHENTICATION_REQ, message, &smState->authReqLock);
    }
}


/****************************************************************************
NAME	
    connectionHandleCancelAuthenticationReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ message

RETURNS
	
*/
void connectionHandleCancelAuthenticationReq(connectionReadInfoState* infoState, connectionSmState* smState, const CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T* req)
{
	/* Check the cancel request comes from the task that initiated bonding */
    if(smState->authReqLock == req->task)
   	{
		/* Check firmware supports the command before we go any further */
		if(infoState->version == bluetooth2_1)
		{
    	   	/* Cancel bonding, locks will be tidied up on pairing complete */
			cancelBonding(&smState->authRemoteAddr, req->force);
		}
		else
		{
			/* Cancel bonding by closing the ACL */
			if(req->force)
				aclClose(&smState->authRemoteAddr);
		}
    }
}


/****************************************************************************
NAME	
    connectionHandleAuthenticationTimeout	

DESCRIPTION
    This function is called on receipt of a 
    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND message.  This indicates that
    that a request to pair to a remote device has timed out.  Let the source
    task know

RETURNS
	
*/
void connectionHandleAuthenticationTimeout(connectionSmState* smState)
{
	/* End Authentication and notify application task */
	endAuthentication(smState, auth_status_timeout);
}


/****************************************************************************
NAME	
    handleSetSecurityModeReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SET_SC_MODE_REQ message.

RETURNS
	
*/
void handleSetSecurityModeReq(connectionSmState* smState, const CL_INTERNAL_SM_SET_SC_MODE_REQ_T* req)
{
    if(!smState->setSecurityModeLock)
    {
        /* Record the current mode */
        if(!smState->authReqLock)
        {
            smState->security_mode = req->mode;
            smState->enc_mode = req->mode3_enc;
        }

        /* In order to return the CFM back to the source task we need to
           lock further requests from being sent to Bluestack until the cfm
           for this instance is returned */
        smState->setSecurityModeLock = req->theAppTask;

		/* Send set sec mode primitive to bluestack */
		{
		MAKE_PRIM_T(DM_SM_SET_SEC_MODE_REQ);    
		prim->mode = connectionConvertSecurityMode_t(req->mode);
		prim->mode3_enc = (uint8) req->mode3_enc;
		VmSendDmPrim(prim);
		}
    }
    else
    {
        /* Message still outstanding, conditionally sent a private message
           to be consumed on the outstanding message request being completed */
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_SC_MODE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_SET_SC_MODE_REQ, message, &smState->setSecurityModeLock);
    }
}


/****************************************************************************
NAME	
    handleSetDefaultSecurityLevelReq	

DESCRIPTION
    This function is called on receipt of an 
	CL_INTERNAL_SM_SET_DEFAULT_SECURITY_LEVEL_REQ message.

RETURNS
	
*/
void handleSetSspSecurityLevelReq(connectionSmState* smState, connectionReadInfoState* infoState, const CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T* req)
{
	if(req->ssp_sec_level < ssp_secl_level_unknown)
	{
		if(req->protocol_id == protocol_unknown)
		{
			/* Set default security level */
    		MAKE_PRIM_T(DM_SM_SET_DEFAULT_SECURITY_REQ);
			prim->secl_default = connectionConvertSspSecurityLevel_t(req->ssp_sec_level, TRUE, req->authorised, req->disable_legacy);
			VmSendDmPrim(prim);
		}
		else
		{
			/* Set protocol security level */
			    MAKE_PRIM_T(DM_SM_REGISTER_REQ);
    			prim->protocol_id = connectionConvertProtocolId(req->protocol_id);
    			prim->channel = req->channel;
    			prim->outgoing_ok = req->outgoing_ok;   
    			prim->security_level = connectionConvertSspSecurityLevel_t(req->ssp_sec_level, req->outgoing_ok, req->authorised, req->disable_legacy);
    			prim->psm = req->psm;
    			VmSendDmPrim(prim);
		}
	}
}


/****************************************************************************
NAME	
    handleSecModeConfigReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ message.

RETURNS
	
*/
void handleSecModeConfigReq(connectionSmState* smState, connectionReadInfoState* infoState, const CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T* req)
{
	/* Make sure the firmware can support the sec mode config primitive */
	if(infoState->version != bluetooth_unknown)
	{
		if(!smState->setSecurityModeLock)
		{
			/* Configure SM */
			MAKE_PRIM_T(DM_SM_SEC_MODE_CONFIG_REQ);
		
			prim->write_auth_enable = connectionConvertWriteAuthEnable_t(req->write_auth_enable);
		
			prim->config = DM_SM_SEC_MODE_CONFIG_NONE;
		
			/* 2.1 devs need access inds to delete out of date link keys and stop sdp ping */
			if(infoState->version == bluetooth2_1)
				prim->config |= DM_SM_SEC_MODE_CONFIG_SEND_ACCESS_IND;		
		
			if(req->debug_keys)
				prim->config |= DM_SM_SEC_MODE_CONFIG_DEBUG_MODE;
			
			if(req->legacy_auto_pair_key_missing)
				prim->config |= DM_SM_SEC_MODE_CONFIG_LEGACY_AUTO_PAIR_MISSING_LINK_KEY;
		
			smState->setSecurityModeLock = req->theAppTask;
			VmSendDmPrim(prim);
		}
		else
		{
			/* Message still outstanding, conditionally sent a private message
           	   to be consumed on the outstanding message request being completed */
        	MAKE_CL_MESSAGE(CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ);
        	COPY_CL_MESSAGE(req, message);
        	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ, message, &smState->setSecurityModeLock);
		}
	}
	else
	{
		/* Send sec mode config fail message to app */
		MAKE_CL_MESSAGE(CL_SM_SEC_MODE_CONFIG_CFM);
		message->success = FALSE;
		message->wae = cl_sm_wae_never;
		message->indications = FALSE;
		message->debug_keys = FALSE;
		MessageSend(req->theAppTask,CL_SM_SEC_MODE_CONFIG_CFM,message);
	}
}


/****************************************************************************
NAME	
    connectionHandleSetSecurityModeCfm	

DESCRIPTION
    This function is called on receipt of an 
    DM_SM_SET_SC_MODE_CFM message.

RETURNS
	
*/
void connectionHandleSetSecurityModeCfm(connectionSmState* smState, const DM_SM_SET_SEC_MODE_CFM_T* cfm)
{
    if(smState->setSecurityModeLock == connectionGetCmTask())
    {
        /* Request originated within the CL, handle internally */
        handleInternalSetSecurityModeCfm(smState, cfm);
    }
    else
    {
        if (smState->setSecurityModeLock)
        {
            /* Send CFM to source task */
            MAKE_CL_MESSAGE(CL_SM_SECURITY_LEVEL_CFM);
            message->success = cfm->success;
            MessageSend(smState->setSecurityModeLock, CL_SM_SECURITY_LEVEL_CFM, message);
        }
    }

    /* Reset lock */
    smState->setSecurityModeLock = 0;
}


/****************************************************************************
NAME	
    connectionHandleConfigureSecurityCfm	

DESCRIPTION
    This function is called on receipt of an 
    DM_SM_SEC_MODE_CONFIG_CFM message.

RETURNS
	
*/
void connectionHandleConfigureSecurityCfm(connectionSmState* smState, const DM_SM_SEC_MODE_CONFIG_CFM_T* cfm)
{
	if(smState->setSecurityModeLock)
	{
		if(smState->setSecurityModeLock != connectionGetCmTask())
		{
			/* Send CFM to source task */
			MAKE_CL_MESSAGE(CL_SM_SEC_MODE_CONFIG_CFM);
			message->success = cfm->status ? FALSE:TRUE;
			message->wae = cfm->write_auth_enable;
			message->indications = (cfm->config & DM_SM_SEC_MODE_CONFIG_SEND_ACCESS_IND) ? TRUE:FALSE;
			message->debug_keys = (cfm->config & DM_SM_SEC_MODE_CONFIG_DEBUG_MODE) ? TRUE:FALSE;
        	MessageSend(smState->setSecurityModeLock, CL_SM_SEC_MODE_CONFIG_CFM, message);
		}
	}
	smState->setSecurityModeLock = 0;
}
		

/****************************************************************************
NAME	
    handleRegisterReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_REGISTER_REQ message.

RETURNS
	
*/
void handleRegisterReq(const CL_INTERNAL_SM_REGISTER_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_REGISTER_REQ);
    prim->protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->channel = req->channel;
    prim->outgoing_ok = req->outgoing_ok;   
    prim->security_level = connectionConvertSecurityLevel_t(req->security_level);
    prim->psm = req->psm;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleUnRegisterReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_UNREGISTER_REQ message.

RETURNS
	
*/
void handleUnRegisterReq(const CL_INTERNAL_SM_UNREGISTER_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_UNREGISTER_REQ);
    prim->protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->channel = req->channel;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleRegisterOutgoingReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_REGISTER_OUTGOING_REQ message.

RETURNS
	
*/
void handleRegisterOutgoingReq(const CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_REGISTER_OUTGOING_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
    prim->protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->remote_channel = req->remote_channel;
    prim->outgoing_security_level = connectionConvertSecurityLevel_t(req->outgoing_security_level);
    prim->psm = req->psm;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleUnRegisterOutgoingReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ message.

RETURNS
	
*/
void handleUnRegisterOutgoingReq(const CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_UNREGISTER_OUTGOING_REQ);
    connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
    prim->protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->remote_channel = req->channel;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleEncryptReq

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ENCRYPT_REQ message.

RETURNS
	
*/
void handleEncryptReq(connectionSmState *smState, const CL_INTERNAL_SM_ENCRYPT_REQ_T* req)
{
	if(!smState->encryptReqLock)
    {
		bdaddr addr;

		/* Store the task id so we know who to route the response to. */
        smState->encryptReqLock = req->theAppTask;
		smState->sink = req->sink;

		{
			/* Acivate/De-activate encyption as requested */
			MAKE_PRIM_T(DM_SM_ENCRYPT_REQ);

			/* Check we got a valid addr */
			if (SinkGetBdAddr(req->sink, &addr))
				connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		
			prim->encrypt = req->encrypt;
			VmSendDmPrim(prim);   
		}
    }
    else
    {
        /* 
			Message still outstanding, conditionally sent a private message
			to be consumed on the outstanding message request being completed.
		*/
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPT_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_ENCRYPT_REQ, message, &smState->encryptReqLock);
    }
}


/****************************************************************************
NAME	
    connectionHandleEncryptCfm

DESCRIPTION
    Handle the confirm message informing us of the outcome of the encrypt 
	request.

RETURNS
	
*/
void connectionHandleEncryptCfm(connectionSmState *smState, const DM_SM_ENCRYPT_CFM_T *cfm)
{
    if (smState->encryptReqLock)
    {
        MAKE_CL_MESSAGE(CL_SM_ENCRYPT_CFM);
        
        if (cfm->success)
            message->status = success;
        else
            message->status = fail;
        
        message->sink = smState->sink;
        message->encrypted = cfm->encrypted;
        MessageSend(smState->encryptReqLock, CL_SM_ENCRYPT_CFM, message);
    }

	/* Reset the resource lock */
	smState->encryptReqLock = 0;
	smState->sink = 0;
}


/****************************************************************************
NAME	
    handleEncryptionKeyRefreshReq

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ message.

RETURNS
	
*/
void handleEncryptionKeyRefreshReq(connectionReadInfoState* infoState, const CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T* req)
{
	if(infoState->version != bluetooth_unknown)
	{
		MAKE_PRIM_C(DM_HCI_REFRESH_ENCRYPTION_KEY);
		connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
		VmSendDmPrim(prim);
	}
}


/****************************************************************************
NAME	
    connectionHandleEncryptionKeyRefreshInd

DESCRIPTION
    Handle the encryption key refresh indication

RETURNS
	
*/
void connectionHandleEncryptionKeyRefreshInd(const DM_HCI_ENCRYPTION_KEY_REFRESH_IND_T *ind)
{
	bdaddr addr;
	uint16 i = 0;
	uint16 max_sinks = 6;		
	hci_status status = connectionConvertHciStatus(ind->status);
						
	/* Allocate a block to hold a list of sinks */
	Sink *sink_list = (Sink *)PanicNull(calloc(max_sinks, sizeof(Sink))); 

	/* Convert the address in the ind */
	connectionConvertBdaddr(&addr, &ind->bd_addr);

	/* Get a list of the tasks that have sinks on the ACL */
    if(StreamSinksFromBdAddr(&max_sinks, sink_list, &addr))
	{
		/* Send a message to each task informing it of the change in encryption status */
		for (i=0; i < max_sinks; i++)
		{
			/* Check we have a sink */
			if(sink_list[i])
			{
				/* Get the associated task */
				Task task = MessageSinkGetTask(sink_list[i]);

				if(task)
				{
					MAKE_CL_MESSAGE(CL_SM_ENCRYPTION_KEY_REFRESH_IND);
        			message->status = status;
        			message->sink = sink_list[i];
        			MessageSend(task, CL_SM_ENCRYPTION_KEY_REFRESH_IND, message);
				}
			}
		}
	}

	/* Free the sink list */
	free(sink_list);
}


/****************************************************************************
NAME	
    handlePinRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_PIN_REQUEST_RES message.

RETURNS
	
*/
void handlePinRequestRes(const CL_INTERNAL_SM_PIN_REQUEST_RES_T* res)
{
    MAKE_PRIM_T(DM_SM_PIN_REQUEST_RES);
    connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
	
	/* Reject pin if length is zero or it exceeds MAX_HCI_PIN_LENGTH */
	if((res->pin_length > 0) && (res->pin_length <= HCI_MAX_PIN_LENGTH))
	{
		 prim->pin_length = res->pin_length;
		 memcpy(prim->pin, res->pin, res->pin_length);
	}
	else
	{
		 prim->pin_length = 0;
	}
	
    VmSendDmPrim(prim);    
}


/****************************************************************************
NAME	
    handleIoCapabilityRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES message.

RETURNS
	
*/
void handleIoCapabilityRequestRes(connectionSmState* smState, const CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T* res)
{
	cl_sm_io_capability io_capability = res->io_capability;
	uint8 authentication_requirements;
	
	/* Check if either we or the remote dev are doing dedicated bonding */
	AUTH_BITS_SET(smState->authentication_requirements, HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING);
	
	if(res->bonding)
	{
		if(auth_bits_set)
		{
			authentication_requirements = res->mitm ? HCI_MITM_REQUIRED_DEDICATED_BONDING : HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING;
		}
		else
		{
			authentication_requirements = res->mitm ? HCI_MITM_REQUIRED_GENERAL_BONDING : HCI_MITM_NOT_REQUIRED_GENERAL_BONDING;
		}
	}
	else
	{
		if(auth_bits_set)
		{
			/* Reject response */
			io_capability = cl_sm_reject_request;
			authentication_requirements = AUTH_REQ_UNKNOWN;
			
		}
		else
		{
			authentication_requirements = res->mitm ? HCI_MITM_REQUIRED_NO_BONDING : HCI_MITM_NOT_REQUIRED_NO_BONDING;
		}
	}
	
	
	smState->authentication_requirements = authentication_requirements;
	
    if(io_capability != cl_sm_reject_request)
    {
		MAKE_PRIM_T(DM_SM_IO_CAPABILITY_REQUEST_RES);
        connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
        prim->io_capability = connectionConvertIoCapability_t(io_capability);
		
		prim->authentication_requirements = authentication_requirements;
		
        if(res->oob_data_present)
        {
            prim->oob_data_present = 1;
            prim->oob_hash_c = VmGetHandleFromPointer(res->oob_hash_c);
            prim->oob_rand_r = VmGetHandleFromPointer(res->oob_rand_r);
        }
        else
        {
            prim->oob_data_present = 0;
            prim->oob_hash_c = NULL;
            prim->oob_rand_r = NULL;
        }			
		
        VmSendDmPrim(prim); 
     }
     else
     {
        MAKE_PRIM_T(DM_SM_IO_CAPABILITY_REQUEST_NEG_RES);
        connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
        prim->reason = hci_error_pairing_not_allowed;
        VmSendDmPrim(prim);
     }
}


/****************************************************************************
NAME	
    handleUserConfirmationRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_USER_CONFIRMATION_REQUEST_RES message.

RETURNS
	
*/
void handleUserConfirmationRequestRes(const CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T* res)
{
	if(res->confirm)
	{
		MAKE_PRIM_T(DM_SM_USER_CONFIRMATION_REQUEST_RES);
		connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
		VmSendDmPrim(prim);
	}
	else
	{
		MAKE_PRIM_T(DM_SM_USER_CONFIRMATION_REQUEST_NEG_RES);
		connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
		VmSendDmPrim(prim); 
	}
}


/****************************************************************************
NAME	
    handleUserPasskeyRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_USER_PASSKEY_REQUEST_RES message.

RETURNS
	
*/
void handleUserPasskeyRequestRes(const CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T* res)
{
	if(!res->cancelled)
	{
		MAKE_PRIM_T(DM_SM_USER_PASSKEY_REQUEST_RES);
		connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
		prim->numeric_value = res->numeric_value;
		VmSendDmPrim(prim);
	}
	else
	{
		MAKE_PRIM_T(DM_SM_USER_PASSKEY_REQUEST_NEG_RES);
		connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
		VmSendDmPrim(prim); 
	}
}


/****************************************************************************
NAME	
    handleSendKeypressNotificationReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SEND_KEYPRESS_NOTIFICATION_REQ message.

RETURNS
	
*/
void handleSendKeypressNotificationReq(const CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T* req)
{
	MAKE_PRIM_T(DM_SM_SEND_KEYPRESS_NOTIFICATION_REQ)
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
	prim->notification_type = connectionConvertKeypressType_t(req->type);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleSetTrustLevelReq

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ
     message.

RETURNS
	
*/
void handleSetTrustLevelReq(connectionReadInfoState* infoState, const CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T* req)
{
	connectionAuthSetTrustLevel(infoState->version, &req->bd_addr, req->trusted);
}


/****************************************************************************
NAME	
    handleAuthoriseRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_AUTHORISE_RES message.

RETURNS
	
*/
void handleAuthoriseRes(const CL_INTERNAL_SM_AUTHORISE_RES_T* res)
{
    MAKE_PRIM_T(DM_SM_AUTHORISE_RES);
    connectionConvertBdaddr_t(&prim->bd_addr, &res->bd_addr);
    prim->protocol_id = connectionConvertProtocolId(res->protocol_id);
    prim->channel = res->channel;
    prim->incoming = res->incoming;
    prim->authorised = res->authorised;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    handleAddAuthDeviceReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ to add a device to the trusted device
	list.
	
RETURNS
	
*/
void handleAddAuthDeviceReq(connectionReadInfoState *infoState, connectionSmState *smState, const CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *req)
{
	if (smState->deviceReqLock)
	{
        /* 
			Message still outstanding, conditionally sent a private message
			to be consumed on the outstanding message request being completed.
		*/
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ, message, &smState->deviceReqLock);
	}
	else
	{
		/* Add device, check if failed locally */
		if (!connectionAuthAddDevice(infoState->version, &req->bd_addr, req->link_key_type, req->link_key, req->trusted, req->bonded))
		{
			MAKE_CL_MESSAGE(CL_SM_ADD_AUTH_DEVICE_CFM);
			message->bd_addr = req->bd_addr;
			message->status = fail;
			MessageSend(req->theAppTask, CL_SM_ADD_AUTH_DEVICE_CFM, message);
		}
		else
		{
		    /* Set lock */
			smState->deviceReqLock = req->theAppTask;
		}
	}
}


/****************************************************************************
NAME	
    handleGetAuthDeviceReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ to get a device from the trusted device
	list.
	
RETURNS
	
*/
void handleGetAuthDeviceReq(connectionSmState *smState, const CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T *req)
{
	if (smState->deviceReqLock)
	{
        /* 
			Message still outstanding, conditionally sent a private message
			to be consumed on the outstanding message request being completed.
		*/
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ, message, &smState->deviceReqLock);
	}
	else
	{
		uint8 linkkey[SIZE_LINK_KEY];
		cl_sm_link_key_type link_key_type;
		uint16 trusted;
		MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_AUTH_DEVICE_CFM, SIZE_LINK_KEY);
		
		message->bd_addr = req->bd_addr;
		
		if (connectionAuthGetDevice(&req->bd_addr, &link_key_type, &linkkey[0], &trusted))
		{
			message->status = success;
			message->trusted = trusted;
			message->link_key_type = link_key_type;
			message->size_link_key = SIZE_LINK_KEY;
			memcpy(&message->link_key[0], &linkkey[0], SIZE_LINK_KEY);
		}
		else
		{
			message->status = fail;
			message->link_key_type = cl_sm_link_key_none;
		}
		
		MessageSend(req->theAppTask, CL_SM_GET_AUTH_DEVICE_CFM, message);
	}
}


/****************************************************************************/
void connectionHandleEncryptionChange(DM_SM_ENCRYPTION_CHANGE_T *ind)
{
	bdaddr addr;
	uint16 i = 0;
	uint16 max_sinks = 6;		

	/* Allocate a block to hold a list of sinks */
	Sink *sink_list = (Sink *)PanicNull(calloc(max_sinks, sizeof(Sink))); 

	/* Convert the address in the ind */
	connectionConvertBdaddr(&addr, &ind->bd_addr);

	/* Get a list of the tasks that have sinks on the ACL */
    if(StreamSinksFromBdAddr(&max_sinks, sink_list, &addr))
	{
		/* Send a message to each task informing it of the change in encryption status */
		for (i=0; i < max_sinks; i++)
		{
			/* Check we have a sink */
			if(sink_list[i])
			{
				/* Get the associated task */
				Task task = MessageSinkGetTask(sink_list[i]);

				if(task)
				{
					/* Send encryption change message to that task */
					sendEncryptionChangeInd(task, sink_list[i], ind->encrypted);
				}
			}
		}
	}

	/* Free the sink list */
	free(sink_list);
}


/***************************************************************************
NAME	
    connectionSmHandleGetAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetAttributeReq(Task appTask, CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T * req)
{	
	connectionAuthGetAttribute(appTask, req->ps_base, &req->bd_addr, req->size_psdata);
}


/***************************************************************************
NAME	
    connectionSmHandleGetIndexedAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetIndexedAttributeReq(Task appTask, CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T * req)
{	
	connectionAuthGetIndexedAttribute(appTask, req->ps_base, req->index, req->size_psdata);
}
