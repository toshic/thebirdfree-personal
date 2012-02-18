/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    init.c        

DESCRIPTION
	Connection library initialisation		

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "bluestack_handler.h"
#include    "init.h"
#include    "dm_init.h"
#include    "dm_security_init.h"
#include    "rfc_init.h"
#include    "l2cap_init.h"
#include    "sdp_init.h"
#include    "tcp_init.h"
#include    "udp_init.h"
#include    "vm.h"

/*lint -e655 */

/****************************************************************************

	Local

*/

/* Enforce static linkage on the Connection Library state instance to ensure
   that the Connection Librry state can only be modified from within the task
   handler.

*/
static connectionState		theCm;

const msg_filter defaultMsgFilter = {msg_group_acl};

/****************************************************************************

DESCRIPTION
    Initialise the connection library locks
*/
static void initLocks(void)
{
	/* Init the locks */
	theCm.inqState.inquiryLock = 0;
	theCm.smState.setSecurityModeLock = 0;
	theCm.smState.authReqLock = 0;
	theCm.smState.encryptReqLock = 0;
	theCm.smState.deviceReqLock = 0;
	theCm.smState.sink = 0;
	theCm.infoState.stateInfoLock = 0;
    theCm.infoState.sink = 0;
	theCm.sdpState.sdpLock = 0;
	theCm.sdpState.sdpSearchLock = 0;
    theCm.rfcommState.registerLock = 0;
    theCm.rfcommState.lock = 0;
    theCm.l2capState.l2capRegisterLock = 0;
}


/****************************************************************************
NAME	
    connectionGetCmTask

DESCRIPTION
    This function returns the connection library task so that the connection
    library can post a message to itself.

RETURNS
    The connection library task.
*/
Task connectionGetCmTask(void)
{
    return &theCm.task;
}


/****************************************************************************
NAME	
    connectionGetAppTask

DESCRIPTION
    This function returns the application task.

RETURNS
    The application task.
*/
Task connectionGetAppTask(void)
{
    return theCm.theAppTask;
}


/****************************************************************************
NAME	
    connectionGetMsgFilter

DESCRIPTION
    This function returns the connection library message filter.

RETURNS
    The connection library message filter.
*/
const msg_filter *connectionGetMsgFilter(void)
{
	return theCm.msgFilter;
}


/*****************************************************************************/
void ConnectionInit(Task theAppTask)
{
	ConnectionInitEx(theAppTask, &defaultMsgFilter);
}

/*****************************************************************************/
void ConnectionInitEx(Task theAppTask, const msg_filter *msgFilter)
{
	theCm.msgFilter = msgFilter;

    /* Turn on extended bluestack prims */
    VmUseExtendedBluestackPrimitives();
    
	/* Initialise the Connection Library Task, all upstream messages sent by
       Bluestack will be handled by this task */
    theCm.task.handler = connectionBluestackHandler;
    
	/* If a task is already registered to receive BlueStack prims then we panic! */
	if (MessageBlueStackTask(connectionGetCmTask()))
	{
		CL_DEBUG(("ERROR - task already registered\n"));
	}

	/* Init the resource locks */
	initLocks();

    /* Store the application task */
    theCm.theAppTask = theAppTask;

    /* Start the initialisation process */
    MessageSend(connectionGetCmTask(), CL_INTERNAL_INIT_REQ, NO_PAYLOAD);
}


/****************************************************************************
NAME	
	connectionHandleInternalInit	

DESCRIPTION
	This function is called to control the initialsation process.  To avoid race
	conditions at initialisation, the process is serialised.

RETURNS

*/
void connectionHandleInternalInit(connectionInitMask mask)
{
    /* Update initilaisation mask */
    theCm.initMask |= mask;

    /* Check to see if all objects have been initialised */
    if(theCm.initMask == connectionInitComplete)
    {
		/* Initialise auth requirements to unknown */
		theCm.smState.authentication_requirements = AUTH_REQ_UNKNOWN;
			
		if(theCm.infoState.version == bluetooth2_1)
		{
			/* 2.1 Device Must be in Mode 4 */
			ConnectionSmSetSecurityMode(connectionGetCmTask(), sec_mode4_ssp, hci_enc_mode_pt_to_pt_and_bcast);
			/* Dev pairs during link setup with mode3 devs if no ACL is up */
			ConnectionSmSecModeConfig(connectionGetCmTask(), cl_sm_wae_acl_owner_none, FALSE, FALSE);
		}
		else
		{
			/* Set the security to Mode 2 */
			ConnectionSmSetSecurityMode(connectionGetCmTask(), sec_mode2_service, hci_enc_mode_pt_to_pt);
		}

        /* Some DM stuff can be initialised only after the DM register has happened so do it here */
        connectionDmInfoInit();

        /* Let the application we're ready to go */
        connectionSendInitCfm(theCm.theAppTask, success, theCm.infoState.version);
    }
    else
    {
        /* Depending upon the previous object initialised, initialise the next one */
        switch(mask)
        {
            case connectionInit:
                connectionDmInit(); 
                break;

            case connectionInitDm:
                connectionRfcInit();	
                break;

            case connectionInitRfc:
                connectionL2capInit();
                break;

            case connectionInitL2cap:
                connectionUdpInit();
                break;

            case connectionInitUdp:
                connectionTcpInit();
                break;

            case connectionInitTcp:
                connectionSdpInit(&theCm.sdpState);
                break;

            case connectionInitSdp: 
				connectionVersionInit();
				break;
				
			case connectionInitVer:
                theCm.smState.noDevices = connectionSmInit(&theCm.infoState);
                break;

            case connectionInitSm:
                /* We're ready! */                
            default:
                break;
        }
    }
}


/****************************************************************************
NAME	
	connectionSendInternalInitCfm	

DESCRIPTION
	This function is callled to send a CL_INTERNAL_INIT_CFM message to the 
	Connection Library task
*/
void connectionSendInternalInitCfm(connectionInitMask mask)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_INIT_CFM);
    message->mask = mask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_INIT_CFM, message);
}


/****************************************************************************
NAME	
	connectionSendInitCfm

DESCRIPTION
	This function is called from the main Connection Library task handler to 
	indicate to the Client application the result of the request to initialise
	the Connection Library
*/
void connectionSendInitCfm(Task task, connection_lib_status status, cl_dm_bt_version version)
{    
    MAKE_CL_MESSAGE(CL_INIT_CFM);
    message->status = status;
	message->version = version;
    MessageSend(task, CL_INIT_CFM, message);

    /* Cancel initialisation timeout */
    if(status == success)
        (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_INIT_TIMEOUT_IND);
}

/*lint +e655 */
