/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_info_handler.c        

DESCRIPTION
    This is the informational primitive management entity and is used to 
	route responses back to the task that initiated the request.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_info_handler.h"

#include <vm.h>
#include <string.h>


/* Send a link quality cfm message to the client */
static void sendLinkQualityCfm(Task task, hci_status status, uint8 link_quality, Sink sink)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_LINK_QUALITY_CFM);
        message->status = status;
        message->link_quality = link_quality;
        message->sink = sink;
        MessageSend(task, CL_DM_LINK_QUALITY_CFM, message);
    }
}


/* Send an RSSI cfm message to the client */
static void sendRssiCfm(Task task, hci_status status, uint8 rssi, Sink sink)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_RSSI_CFM);
        message->status = status;
        message->rssi = rssi;
        message->sink = sink;
        MessageSend(task, CL_DM_RSSI_CFM, message);
    }
}


/* Send an clock offset cfm message to the client */
static void sendClockOffsetCfm(Task task, hci_status status, uint16 offset, Sink sink)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_CLOCK_OFFSET_CFM);	
        message->status = status;
        message->sink = sink;
        message->clock_offset = offset;
        MessageSend(task, CL_DM_CLOCK_OFFSET_CFM, message);
    }
}


/* Send a remote supported features cfm message to the client */
static void sendRemoteSupportedFeaturesCfm(Task task, hci_status status, const uint16 *features, Sink sink)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_REMOTE_FEATURES_CFM);
        message->status = status;	
        message->sink = sink;
        
        if (features)
            memmove(message->features, features, 4);
        else
            *message->features = 0;
        
        MessageSend(task, CL_DM_REMOTE_FEATURES_CFM, message);
    }
}


/****************************************************************************
NAME	
    connectionHandleReadAddrRequest

DESCRIPTION
    Handle an internal request to read the local bluetooth address

RETURNS
    void
*/
void connectionHandleReadAddrRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_BD_ADDR_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_READ_BD_ADDR);
	VmSendDmPrim(prim);

	/* Set the lock */
	state->stateInfoLock = req->theAppTask;
    state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadBdAddrComplete

DESCRIPTION
    Handle a read bd addr complete event

RETURNS
    void
*/
void connectionHandleReadBdAddrComplete(connectionReadInfoState *state, const DM_HCI_READ_BD_ADDR_COMPLETE_T *cfm)
{
    if (state->stateInfoLock)
    {
        /* Send the result to the client */
        MAKE_CL_MESSAGE(CL_DM_LOCAL_BD_ADDR_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        connectionConvertBdaddr(&message->bd_addr, &cfm->bd_addr);
        MessageSend(state->stateInfoLock, CL_DM_LOCAL_BD_ADDR_CFM, message);
        
        /* Reset the lock */
        state->stateInfoLock = 0;
    }
}


/****************************************************************************
NAME	
    connectionHandleReadLinkQualityRequest

DESCRIPTION
    Issue a request to read the link quality on a particular connection.

RETURNS
    void
*/
void connectionHandleReadLinkQualityRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T *req)
{
	bdaddr addr;

	/* Check we got a valid addr */
	if (!SinkGetBdAddr(req->sink, &addr))
	{
		/* Send an error to the app as it didn't pass in a valid sink */
		sendLinkQualityCfm(req->theAppTask, hci_error_no_connection, 0, req->sink);
	}
	else
	{
		/* Response not outstanding so issue request */
		MAKE_PRIM_C(DM_HCI_GET_LINK_QUALITY);
		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		VmSendDmPrim(prim);

		/* Set the lock */
		state->stateInfoLock = req->theAppTask;
		state->sink = req->sink;
	}
}


/****************************************************************************
NAME	
    connectionHandleReadLinkQualityComplete

DESCRIPTION
    Confirm containing the link quality if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadLinkQualityComplete(connectionReadInfoState *state, const DM_HCI_GET_LINK_QUALITY_COMPLETE_T *cfm)
{
	/* Let the app know the outcome of the read request */
	sendLinkQualityCfm(state->stateInfoLock, connectionConvertHciStatus(cfm->status), cfm->link_quality, state->sink);

	/* Reset the lock */
	state->stateInfoLock = 0;
	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadRssiRequest

DESCRIPTION
    Request to read the RSSI on a particular connection.

RETURNS
    void
*/
void connectionHandleReadRssiRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_RSSI_REQ_T *req)
{
	bdaddr addr;

	/* Check we got a valid addr */
	if (!SinkGetBdAddr(req->sink, &addr))
	{
		/* Send an error to the app as it didn't pass in a valid sink */
		sendRssiCfm(req->theAppTask, hci_error_no_connection, 0, req->sink);
	}
	else
	{
		/* Response not outstanding so issue request */
		MAKE_PRIM_C(DM_HCI_READ_RSSI);
		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		VmSendDmPrim(prim);

		/* Set the lock */
		state->stateInfoLock = req->theAppTask;
		state->sink = req->sink;
	}
}


/****************************************************************************
NAME	
    connectionHandleReadRssiComplete

DESCRIPTION
    Confirm containing the RSSI value if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadRssiComplete(connectionReadInfoState *state, const DM_HCI_READ_RSSI_COMPLETE_T *cfm)
{
	/* Pass the result to the app */
	sendRssiCfm(state->stateInfoLock, connectionConvertHciStatus(cfm->status), cfm->rssi, state->sink);

	/* Reset the lock */
	state->stateInfoLock = 0;
	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadclkOffsetRequest

DESCRIPTION
    Request to read the clock offset of a remote device.

RETURNS
    void
*/
void connectionHandleReadclkOffsetRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T *req)
{
	bdaddr addr;

	/* Check we got a valid addr */
	if (!SinkGetBdAddr(req->sink, &addr))
	{
		/* Send an error to the app as it didn't pass in a valid sink */
		sendClockOffsetCfm(req->theAppTask, hci_error_no_connection, 0, req->sink);		
	}
	else
	{
		/* Response not outstanding so issue request */
		MAKE_PRIM_C(DM_HCI_READ_CLOCK_OFFSET);
		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		VmSendDmPrim(prim);

		/* Set the lock */
		state->stateInfoLock = req->theAppTask;
		state->sink = req->sink;
	}		
}


/****************************************************************************
NAME	
    connectionHandleReadClkOffsetComplete

DESCRIPTION
    Confirm containing the remote clock offset if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadClkOffsetComplete(connectionReadInfoState *state, const DM_HCI_READ_CLOCK_OFFSET_COMPLETE_T *cfm)
{
	/* Tell the application about the outcome of the read clock offset request */
	sendClockOffsetCfm(state->stateInfoLock, connectionConvertHciStatus(cfm->status), cfm->clock_offset, state->sink);

	/* Reset the lock */
	state->stateInfoLock = 0;
	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadRemoteSupportedFeaturesRequest

DESCRIPTION
    Request to read the supported features of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T *req)
{
	bdaddr addr;

	/* Check we got a valid addr */
	if (!SinkGetBdAddr(req->sink, &addr))
	{
		/* Send an error to the app as it didn't pass in a valid sink */
		sendRemoteSupportedFeaturesCfm(req->theAppTask, hci_error_no_connection, 0, req->sink);		
	}
	else
	{
		/* Response not outstanding so issue request */
		MAKE_PRIM_C(DM_HCI_READ_REMOTE_FEATURES);
		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		VmSendDmPrim(prim);

		/* Set the lock */
		state->stateInfoLock = req->theAppTask;
		state->sink = req->sink;
	}		
}


/****************************************************************************
NAME	
    connectionHandleReadRemoteSupportedFeaturesCfm

DESCRIPTION
    Confirm containing the remote supported features, if the read request
	succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesCfm(connectionReadInfoState *state, const DM_HCI_READ_REMOTE_FEATURES_COMPLETE_T *cfm)
{
	/* Send message up to the client only if the lock is set */
	if (state->stateInfoLock)
	{
		/* 
			Need to check the lock since we get an unsolicited remote features cfm 
			message every time an ACL is opened because BlueStack obtains the remote
			features automatically.
		*/
		sendRemoteSupportedFeaturesCfm(state->stateInfoLock, connectionConvertHciStatus(cfm->status), (const uint16 *) &(cfm->features), state->sink);
	}

	/* Reset the lock */
	state->stateInfoLock = 0;
	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadLocalVersionRequest

DESCRIPTION
    Request to read the version information of the local device.
  
RETURNS
    void
*/
void connectionHandleReadLocalVersionRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *req)
{
	/* Response not outstanding so issue request */
	MAKE_PRIM_C(DM_HCI_READ_LOCAL_VERSION);
	VmSendDmPrim(prim);
 
	/* Set the lock */
 	state->stateInfoLock = req->theAppTask;
}


/****************************************************************************
NAME	
    connectionHandleReadLocalVersionCfm

DESCRIPTION
    Confirm containing the local version information, if the read request
	succeeded.

RETURNS
    void
*/
void connectionHandleReadLocalVersionCfm(connectionReadInfoState *state, const DM_HCI_READ_LOCAL_VERSION_COMPLETE_T *cfm)
{
	if(state->stateInfoLock == connectionGetCmTask())
	{
		/* We got here as part of intialisation, continue version setup */
		if(cfm->hci_version >= HCI_VER_2_1)
		{
			/* Firmware supports 2.1 so can setup the BT version */
			MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_BT_VERSION_REQ);
			message->theAppTask = connectionGetCmTask();
			message->version = BT_VERSION_2p1;
			MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_SET_BT_VERSION_REQ, message);
		}
		else
		{
			/* Nothing more to be done for version init, continue init process */
			state->version = bluetooth_unknown;
			connectionSendInternalInitCfm(connectionInitVer);
		}
	}
	else
	{
    	/* Create and send the message */
    	MAKE_CL_MESSAGE(CL_DM_LOCAL_VERSION_CFM);
		message->hciVersion = connectionConvertHciVersion(cfm->hci_version);
		message->hciRevision = cfm->hci_revision;
    	message->status = connectionConvertHciStatus(cfm->status);	
    	message->lmpVersion       = cfm->lmp_version;
		message->manufacturerName = cfm->manuf_name;
		message->lmpSubVersion    = cfm->lmp_subversion;
         
		MessageSend(state->stateInfoLock, CL_DM_LOCAL_VERSION_CFM, message);
	}
	/* Reset the lock */
 	state->stateInfoLock = 0;
 	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadRemoteVersionRequest

DESCRIPTION
    Request to read the version information of a remote device.
  
RETURNS
    void
*/
void connectionHandleReadRemoteVersionRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T *req)
{
	bdaddr addr;
		
	/* Check we got a valid addr */
	if (!SinkGetBdAddr(req->sink, &addr))
	{
		/* Create and send the message */
		MAKE_CL_MESSAGE(CL_DM_REMOTE_VERSION_CFM);
		message->status = hci_error_no_connection;	
		message->lmpVersion       = 0;
		message->manufacturerName = 0;
		message->lmpSubVersion    = 0;
		MessageSend(req->theAppTask, CL_DM_REMOTE_VERSION_CFM, message);	
	}
	else
	{
		/* Response not outstanding so issue request */
		MAKE_PRIM_C(DM_HCI_READ_REMOTE_VERSION);
 		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
 		VmSendDmPrim(prim);
 
 		/* Set the lock */
 		state->stateInfoLock = req->theAppTask;
	}
}


/****************************************************************************
NAME	
    connectionHandleReadRemoteVersionCfm

DESCRIPTION
    Confirm containing the remote version information, if the read request
	succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteVersionCfm(connectionReadInfoState *state, const DM_HCI_READ_REMOTE_VERSION_COMPLETE_T *cfm)
{
     /* Create and send the message */
     MAKE_CL_MESSAGE(CL_DM_REMOTE_VERSION_CFM);
     message->status = connectionConvertHciStatus(cfm->status);	
     message->lmpVersion       = cfm->LMP_version;
     message->manufacturerName = cfm->manufacturer_name;
     message->lmpSubVersion    = cfm->LMP_subversion;
         
     MessageSend(state->stateInfoLock, CL_DM_REMOTE_VERSION_CFM, message);
 
 	/* Reset the lock */
 	state->stateInfoLock = 0;
 	state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleSetBtVersionReq

DESCRIPTION
    Handle setting BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionReq(connectionReadInfoState *state, const CL_INTERNAL_DM_SET_BT_VERSION_REQ_T *req)
{
	MAKE_PRIM_T(DM_SET_BT_VERSION_REQ);
	prim->version = req->version;
	VmSendDmPrim(prim);

	/* Set the lock */
	state->stateInfoLock = req->theAppTask;
}


/****************************************************************************
NAME	
    connectionHandleSetBtVersionCfm

DESCRIPTION
    Confirm containing the request status and current BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionCfm(connectionReadInfoState *state, const DM_SET_BT_VERSION_CFM_T *cfm)
{
	if(state->stateInfoLock)
	{	
		if(state->stateInfoLock == connectionGetCmTask())
		{
			/* We finished initialising bt version, continue init process */
			connectionSendInternalInitCfm(connectionInitVer);
		}
		else
		{
			MAKE_CL_MESSAGE(CL_DM_READ_BT_VERSION_CFM);
			message->status = connectionConvertHciStatus(cfm->status);
			message->version = connectionConvertBtVersion(cfm->version);
			MessageSend(state->stateInfoLock,CL_DM_READ_BT_VERSION_CFM, message);	
		}
	}
	
	state->stateInfoLock = 0;
	state->version = cfm->version;
}


/****************************************************************************
NAME	
    connectionHandleDmHciModeChangeEvent

DESCRIPTION
    An indication from BlueStack that an ACL has been opened. Some clients
	may need this information so pass the indication up to the task 
	registered as the "app task" (we don't know who else to pass this to!).

RETURNS
    void
*/
void connectionHandleDmHciModeChangeEvent(Task task, const DM_HCI_MODE_CHANGE_EVENT_T *ev)
{
    const msg_filter *msgFilter = connectionGetMsgFilter();
	if (task && (msgFilter[0] & msg_group_mode_change)) 
    {
		/* Check mode change was successful */
		if ((ev->status == HCI_SUCCESS) && ((ev->mode == HCI_BT_MODE_ACTIVE) || (ev->mode == HCI_BT_MODE_SNIFF)))
		{
			MAKE_CL_MESSAGE(CL_DM_MODE_CHANGE_EVENT);
			connectionConvertBdaddr(&message->bd_addr, &ev->bd_addr);

			/* Convert HCI mode into connection library power mode */
			switch (ev->mode)
			{
				case HCI_BT_MODE_ACTIVE:
					message->mode = lp_active;
					break;
				case HCI_BT_MODE_SNIFF:
					message->mode = lp_sniff;
					break;
				default:
					Panic();
					break;
			}
			message->interval = ev->length;
			MessageSend(task, CL_DM_MODE_CHANGE_EVENT, message);
		}
    }
}
