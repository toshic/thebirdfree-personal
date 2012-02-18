/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_link_policy_handler.c        

DESCRIPTION
    This file contains the implementation of the link policy management entity.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_link_policy_handler.h"

#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>


static const uint16_t rs_table_normal[] =
{
    /* if we have at least 1 ACL, we don't want to role switch on outoing or incoming connections. */
    DM_LP_ROLESWITCH_TABLE_ENTRY(0, 0, 0, 1, 0, DM_LP_ROLESWITCH_ACTION_OUTGOING_NO_RS),
    0 /* End of this table. */
};


/*****************************************************************************/
static void sendRoleCfmToClient(Task client, hci_status status, hci_role role, Sink sink)
{
    if (client)
    {
    	MAKE_CL_MESSAGE(CL_DM_ROLE_CFM);
	    message->status = status;
	    message->role = role;
	    message->sink = sink;
	    MessageSend(client, CL_DM_ROLE_CFM, message);
    }
}


/*****************************************************************************/
static bool sendSetRoleRequest(Sink sink, hci_role role)
{
	bdaddr	bd_addr;
	
	if(SinkGetBdAddr(sink, &bd_addr))
	{
		MAKE_PRIM_C(DM_HCI_SWITCH_ROLE);
		connectionConvertBdaddr_t(&prim->bd_addr, &bd_addr);
		prim->role = connectionConvertHciRole_t(role);
		VmSendDmPrim(prim);

		return TRUE;
	}
	else
		return FALSE;
}


/*****************************************************************************/
static bool sendGetRoleRequest(Sink sink)
{
	bdaddr	bd_addr;
	
	if(SinkGetBdAddr(sink, &bd_addr))
	{
		MAKE_PRIM_C(DM_HCI_ROLE_DISCOVERY);
		connectionConvertBdaddr_t(&prim->bd_addr, &bd_addr);
		VmSendDmPrim(prim);

		return TRUE;
	}
	else
		return FALSE;
}


/****************************************************************************
Set the link supervision timeout on the ACL link identified by the specified 
Bluetooth Device Address to the specified timeout value.	
*/
static void setLinkSupervisionTimeout(const bdaddr* addr, uint16 timeout)
{
    MAKE_PRIM_C(DM_HCI_WRITE_LINK_SUPERV_TIMEOUT);
    prim->handle = 0; 
    connectionConvertBdaddr_t(&prim->bd_addr, addr);   
    prim->timeout = timeout;
    VmSendDmPrim(prim);
}


/****************************************************************************
The Power Table specifies the link policy power settings for the link 
identified by the Bluetooth Device Address.  
*/
static void processPowerTable(const bdaddr *addr, uint16 num_entries, lp_power_table const *power_table)
{
	/*
		This is a bit naughty since we're relying on the fact that the two tables are
		identical, but they have been designed to be so for the sake of efficiency go for it!
	*/
	LP_POWERSTATE_T *my_power_table = (LP_POWERSTATE_T *)PanicUnlessMalloc(num_entries*sizeof(LP_POWERSTATE_T));
	memcpy(my_power_table, power_table, num_entries*sizeof(LP_POWERSTATE_T));

	{
		MAKE_PRIM_T(DM_LP_WRITE_POWERSTATES_REQ);
		connectionConvertBdaddr_t(&prim->bd_addr, addr);
		prim->num_states = num_entries;
		prim->states = VmGetHandleFromPointer(my_power_table);
		VmSendDmPrim(prim);
	}
}


/*****************************************************************************/
void connectionHandleLinkPolicySetRoleReq(connectionLinkPolicyState* linkPolicyState, const CL_INTERNAL_DM_SET_ROLE_REQ_T* req)
{
	if(!linkPolicyState->roleLock)
	{
		/* Set the requested role by sending a request to Bluestack*/
		if (!sendSetRoleRequest(req->sink, req->role))
			/* If the client passed us an invalid sink tell it */
			sendRoleCfmToClient(connectionGetAppTask(), hci_error_no_connection, req->role, req->sink);
		else
			/* Otherwise set the resource lock */
			linkPolicyState->roleLock = req->sink;
	}
	else
	{
		/* Role request in progress, post a conditional internal 
           message to queue request for processing later */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_ROLE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_SET_ROLE_REQ, message, (Task*) &linkPolicyState->roleLock);
	}
}


/*****************************************************************************/
void connectionHandleLinkPolicyGetRoleReq(connectionLinkPolicyState* linkPolicyState, const CL_INTERNAL_DM_GET_ROLE_REQ_T* req)
{
	if(!linkPolicyState->roleLock)
	{
		/* Get the requested role by sending a request to Bluestack*/
		if (!sendGetRoleRequest(req->sink))
			/* If the client passed us an invalid sink tell it */
			sendRoleCfmToClient(connectionGetAppTask(), hci_error_no_connection, hci_role_dont_care, req->sink);
		else
			/* Otherwise set the resource lock */
			linkPolicyState->roleLock = req->sink;	
	}
	else
	{
		/* Role request in progress, post a conditional internal 
           message to queue request for processing later */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_GET_ROLE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_GET_ROLE_REQ, message, (Task*) &linkPolicyState->roleLock);
	}
}


/*****************************************************************************/
void connectionHandleDmSwitchRoleComplete(Task app_task, connectionLinkPolicyState *linkPolicyState, const DM_HCI_SWITCH_ROLE_COMPLETE_T* ind)
{
	if(linkPolicyState->roleLock)
	{
		sendRoleCfmToClient(connectionGetAppTask(), connectionConvertHciStatus(ind->status), connectionConvertHciRole(ind->role), linkPolicyState->roleLock);
	
		/* Reset the lock */
		linkPolicyState->roleLock = 0;
	}
    else
    {
        hci_status status; 
        hci_role role;
        bdaddr addr;

		MAKE_CL_MESSAGE(CL_DM_ROLE_IND);

		status = connectionConvertHciStatus(ind->status); 
        role = connectionConvertHciRole(ind->role);
        connectionConvertBdaddr(&addr, &ind->bd_addr);
		
	    message->status = status;
	    message->role = role;
	    message->bd_addr = addr;
	    MessageSend(connectionGetAppTask(), CL_DM_ROLE_IND, message);
    }
}


/*****************************************************************************/
void connectionHandleRoleDiscoveryComplete(connectionLinkPolicyState* linkPolicyState, const DM_HCI_ROLE_DISCOVERY_COMPLETE_T* ind)
{
	if(linkPolicyState->roleLock)
	{
		sendRoleCfmToClient(connectionGetAppTask(), connectionConvertHciStatus(ind->status), connectionConvertHciRole(ind->role), linkPolicyState->roleLock);
	
		/* Reset the lock */
		linkPolicyState->roleLock = 0;
	}
}


/*****************************************************************************/
void connectionHandleSetLinkSupervisionTimeoutReq(const CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T* req)
{
	bdaddr bd_addr;

	if(SinkGetBdAddr(req->sink, &bd_addr))
	{
		/* Set link supervision timeout */
		setLinkSupervisionTimeout(&bd_addr, req->timeout);
	}
}


/*****************************************************************************/
void connectionHandleLinkSupervisionTimeoutInd(const DM_HCI_LINK_SUPERV_TIMEOUT_IND_T* ind)
{
	MAKE_CL_MESSAGE(CL_DM_LINK_SUPERVISION_TIMEOUT_IND);
	connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	message->timeout = ind->timeout;
	MessageSend(connectionGetAppTask(), CL_DM_LINK_SUPERVISION_TIMEOUT_IND, message);
}


/*****************************************************************************/
void connectionHandleSetLinkPolicyReq(const CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T* req)
{
	bdaddr bd_addr;

	if(SinkGetBdAddr(req->sink, &bd_addr))
	{
		/* Process the power table */
		processPowerTable(&bd_addr, req->size_power_table, req->power_table);
	}
}


/****************************************************************************/
void connectionLinkPolicyHandleWritePowerStatesCfm(const DM_LP_WRITE_POWERSTATES_CFM_T *cfm)
{
	/* The attempt to set a power table may have failed */
	if((cfm->result != LP_POWERSTATES_SUCCESS) && (cfm->result != LP_POWERSTATES_UNKNOWN_DEVICE))
	{
		/* A client is passing in duff power tables - panic as low power support will not work */
		Panic();
	}
}


/*****************************************************************************/
void connectionHandleSetSniffSubRatePolicyReq(connectionReadInfoState* infoState, const CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T* req)
{
	bdaddr bd_addr;

	/* If we have a valid address and dev supports SSR */
	if(SinkGetBdAddr(req->sink, &bd_addr) && infoState->version == bluetooth2_1)
	{
		/* Set sniff subrate policy with device associated with the sink */
		MAKE_PRIM_C(DM_HCI_SNIFF_SUB_RATE);
		connectionConvertBdaddr_t(&prim->bd_addr, &bd_addr);
		prim->max_remote_latency = req->max_remote_latency;
		prim->min_remote_timeout = req->min_remote_timeout;
		prim->min_local_timeout = req->min_local_timeout;
		VmSendDmPrim(prim);
	}
}


/****************************************************************************/
void connectionHandleSniffSubRatingInd(const DM_HCI_SNIFF_SUB_RATING_IND_T *ind)
{
	/* tell the app we are sniff subrating */
	MAKE_CL_MESSAGE(CL_DM_SNIFF_SUB_RATING_IND);
	message->status = connectionConvertHciStatus(ind->status);
    connectionConvertBdaddr(&message->bd_addr, &ind->bd_addr);
	message->transmit_latency = ind-> transmit_latency;
    message->receive_latency = ind->receive_latency;
    message->remote_timeout = ind->remote_timeout;
    message->local_timeout = ind->local_timeout;
	MessageSend(connectionGetAppTask(), CL_DM_SNIFF_SUB_RATING_IND, message);
}


/****************************************************************************/
void connectionHandleSetRoleSwitchParamsReq(const CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ_T* req)
{
    MAKE_PRIM_T(DM_LP_WRITE_ROLESWITCH_POLICY_REQ);
	prim->version = 0;
    if (req->rs_table)
    {
	    prim->length = sizeof(req->rs_table);
	    prim->rs_table = VmGetHandleFromPointer((uint16*)req->rs_table);
    }
    else
    {
        prim->length = sizeof(rs_table_normal);
	    prim->rs_table = VmGetHandleFromPointer((uint16*)rs_table_normal);
    }
	VmSendDmPrim(prim);
}
