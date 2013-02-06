/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_interface.c
    
DESCRIPTION
	Entry Point for the Generic Object Exchange Profile (GOEP) library
	Receives function calls from clients

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <message.h>

#include <connection.h>
#include <vm.h>
#include <string.h>
#include <stream.h>

#include "goep.h"
#include "goep_private.h"
#include "goep_init.h"
#include "goep_packet.h"

static uint16 convGoepResponseCode(uint16 result, bool lastPacket);

/* Service request array used to find the channel number */
static const uint8 goepServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...4 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0xaa, 0xbb /* Class ID required - replace 0xaabb with correct value */
};

/* Attribute request array */
static const uint8 protocolAttributeRequest [] =
{
    0x35, /* type = DataElSeq */
    0x03, /* size ...3 bytes in DataElSeq */
    0x09, 0x00, 0x04/* 2 byte UINT attrID ProtocolDescriptorList */    
};

static uint16 convGoepResponseCode(uint16 result, bool lastPacket)
{
	uint16 respCode;
	
	switch (result)
	{
	case goep_svr_resp_OK:
		if (lastPacket)
			respCode = goep_Rsp_Success;
		else
			respCode = goep_Rsp_Continue;
		break;
	case goep_svr_resp_NotFound:
		respCode = goep_Rsp_NotFound;
		break;
	case goep_svr_resp_Forbidden:
		respCode = goep_Rsp_Forbidden;
		break;
	case goep_svr_resp_Unauthorized:
		respCode = goep_Rsp_Unauthorised;
		break;
	case goep_svr_resp_PreConFail:
		respCode = goep_Rsp_PreConFail;
		break;
	case goep_svr_resp_ServUnavailable:
		respCode = goep_Rsp_ServUnavail;
		break;
	case goep_svr_resp_BadRequest:
		default:
		respCode = goep_Rsp_BadRequest;
		break;
	};
	return respCode;
}


/*!
	@brief Initialise the GOEP Library.
	@param theAppTask The current application task.
	@param role Client or Server
	@param servClass Type of GOEP required (e.g. FTP or OPP)

	This should be the first function called.
*/
void GoepInit(Task theAppTask, goep_serv_role role, goep_serv_class servClass)
{
	Task goepTask;
	goepState *state;
	MAKE_GOEP_MESSAGE(GOEP_INIT_CFM);
	
	GOEP_DEBUG(("Goep Init\n"));
	
	goepTask = goepGetTheTask(); /* Can't fail since goepGetTheTask will Panic on no memory */
	state = (goepState*)goepTask;
	
	state->theApp=theAppTask;
	state->sink=0;
    state->role=role;
    state->servClass=servClass;
	state->srcUsed = 0;
	state->useHeaders = FALSE;
		
	message->goep = state;
	message->status = goep_success;
	MessageSend(theAppTask, GOEP_INIT_CFM, message);
	
	state->state= goep_initialised;
}

/*!
	@brief Get an RFCOMM channel to use with this GOEP Instance.
	@param session GOEP Session Instance.
	
	This function should only be called for server type applications.
*/
void GoepGetChannel(GOEP *goep)
{
	GOEP_DEBUG(("Goep GetChannel\n"));
	
	ConnectionRfcommAllocateChannel(&goep->task);
}

/*
	@brief Open an GOEP Connection with a server.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param bd_addr The Bluetooth address of the device being replied to.
	@param rfc_channel RFCOMM Server channel to use for the connection
	@param maxPacketSize Maximum packet size supported by this client.
	@param size_target Length of the target parameter.
	@param target GOEP target header.
*/
void GoepConnect(GOEP *goep, const bdaddr *bdAddr, uint8 rfc_channel, uint16 maxPacketSize, 
				 	uint16 size_target, const uint8 *target)
{
    uint8 sp_ptr[sizeof(goepServiceRequest)];
	
	if (goep->role != goep_Client)
	{
		GOEP_DEBUG(("Command only valid if a client\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	if (goep->state != goep_initialised)
	{
		GOEP_DEBUG(("Command only valid in initialised state\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	goep->rfcChan = rfc_channel;
	
	/* Copy the const GOEP service request array  into a temp buffer */
	memmove(sp_ptr, goepServiceRequest, sizeof(goepServiceRequest));

    /* Modify the Class id as per Server Class */
    sp_ptr[5] = (goep->servClass >> 8) & 0xff;
	sp_ptr[6] = goep->servClass & 0xff;
	
	memmove(&(goep->bdAddr), bdAddr, sizeof(bdaddr));
	goep->state = goep_connecting;
	goep->pktLen = maxPacketSize;
	goep->conLen = size_target;
	goep->conInfo = (uint8 *)PanicUnlessMalloc(size_target);
    memmove(goep->conInfo, target, size_target);
    
	GOEP_DEBUG(("ConnectionSdpServiceSearchAttributeRequest call\n"));
	ConnectionSdpServiceSearchAttributeRequest(&goep->task, bdAddr, 0x32, sizeof(goepServiceRequest), sp_ptr, sizeof(protocolAttributeRequest), protocolAttributeRequest);
}

/*
	@brief Start an authentication challenge during connect.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param nonce MD5 Digest Nonce to use
	@param options Optional options to use.
	@param size_realm Length of the optional Realm string.
	@param realm Option realm string.
	
	This function can be called by a GOEP Server on receipt of a GOEP_CONNECT_IND message to start
	and authentication challenge.
	A GOEP_AUTH_RESULT_IND message will be received to indicate the result of the authentication challenge.
*/
void GoepConnectAuthChallenge(GOEP *goep, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm)
{
	if (goep->role != goep_Server)
	{
		GOEP_DEBUG(("Command only valid if a server\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	if (goep->state != goep_connecting)
	{
		GOEP_DEBUG(("Command only valid in connecting state\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	if (goepSendConnectResponse(goep, goep_Rsp_Unauthorised, nonce, options, size_realm, realm)!=goep_success)
	{ /* Failed to send packet */
		goepMsgSendConnectConfirm(goep, goep_failure, 0);
		goep->state = goep_initialised;
	}
	else
		goep->state = goep_connect_auth;
}

/*
	@brief Respond to an authentication challenge during connect.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param digest MD5 Digest response. Must be a 16byte string.
	@param size_userid Length of the optional User Id string.
	@param userid Option User Id string.
	@param nonce Optional MD5 Digest Nonce as sent in the Challenge. Must be a 16byte string or NULL.
	
	This function can be called by a GOEP Client on receipt of a GOEP_AUTH_REQUEST_IND message to reply
	to an authentication challenge.
	A GOEP_CONNECT_CFM message will be received to indicate that the connection process
	has completed.
*/
void GoepConnectAuthResponse(GOEP *goep, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce)
{
	if (goep->role != goep_Client)
	{
		GOEP_DEBUG(("Command only valid if a client\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	if (goep->state != goep_connect_auth)
	{
		GOEP_DEBUG(("Command only valid in connecting with authentication state\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	if (goepSendFullConnect(goep, digest, size_userid, userid, nonce) != goep_success)
	{ /* Failed to send packet */
		goepMsgSendConnectConfirm(goep, goep_failure, 0);
	}
}

/*
	@brief Respond to a GOEP connection attempt from a client.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result TRUE to accept the connetion and FALSE to reject it.
	@param maxPacketSize Maximum packet size supported by this client.
*/
void GoepConnectResponse(GOEP *goep, goep_svr_resp_codes result, uint16 maxPacketSize)
{
	uint16 respCode;
	
	if (goep->role != goep_Server)
	{
		GOEP_DEBUG(("Command only valid if a server\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	if ((goep->state != goep_connecting) && (goep->state != goep_connect_auth))
	{
		GOEP_DEBUG(("Command only valid in connecting state\n"));
		goepMsgSendConnectConfirm(goep, goep_invalid_command, 0);
		return;
	}
	
	if (maxPacketSize < goep->pktLen)
		goep->pktLen = maxPacketSize;
	
	switch (result)
	{
	case goep_svr_resp_OK:
		respCode = goep_Rsp_Success;
		break;
	case goep_svr_resp_Forbidden:
		respCode = goep_Rsp_Forbidden;
		break;
	case goep_svr_resp_Unauthorized:
		respCode = goep_Rsp_Unauthorised;
		break;
	case goep_svr_resp_BadRequest:
		default:
		respCode = goep_Rsp_BadRequest;
		break;
	};
	
	if (goep->conInfo)
	{ /* conInfo present, we need to generate a connectionID */
		goep->conID = VmGetClock();
		goep->useConID = TRUE;
	}
	
	/* Send relevant response packet */
	if (goepSendConnectResponse(goep, respCode, NULL, 0, 0, NULL)!=goep_success)
	{ /* Failed to send packet */
		goepMsgSendConnectConfirm(goep, goep_failure, 0);
		goep->state = goep_initialised;
	}
	else
	{ /* Send success message */
		goepMsgSendConnectConfirm(goep, goep_success, goep->pktLen);
		if (respCode == goep_Rsp_Success)
			goep->state = goep_connected;
		else
			goep->state = goep_connect_refused;
	}
	
	/* Clean up temp. storage */
	if (goep->conInfo)
	{
		free(goep->conInfo);
		goep->conInfo=NULL;
		goep->conLen=0;
	}
}

/****************************************************************************
NAME	
	GoepDisconnect

DESCRIPTION
	Close an GOEP Connection with a server.  All transfer operations
	associated with this session MUST either be completed or aborted before this
	function is called.
	After GOEP_DISCONNECT_* has been received with a 'success' state, the
	only valid function to call is GoepDeRegister or GoepConnect.
	
RETURNS
    void
*/
void GoepDisconnect(GOEP *goep)
{
	if (goep->role != goep_Client)
	{
		GOEP_DEBUG(("Command only valid if a client\n"));
		goepMsgSendDisconnectConfirm(goep, goep_invalid_command);
		return;
	}
	
    if ((goep->state != goep_connected) && (goep->state != goep_connect_abort) 
			&& (goep->state != goep_connecting) && (goep->state != goep_connect_auth))
    {
		goepMsgSendDisconnectConfirm(goep, goep_invalid_command);
        return ;
    }
	
	GOEP_DEBUG(("Sending Disconnect Packet\n"));
	if (goepSendDisconnect(goep)!=goep_success)
	{ /* Failed to send packet */
		goepMsgSendDisconnectConfirm(goep, goep_failure);
	}
    
	if ((goep->state == goep_connecting) || (goep->state == goep_connect_auth))
		goep->state = goep_connect_cancel;
	else if (goep->state != goep_connect_abort)
    	goep->state = goep_disconnecting;
}

/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

*/
void GoepLocalPutFirst(GOEP *goep,
				  		uint16 size_name, const uint8* name, 
						uint16 size_packet, const uint8 *packet, 
						uint32 totalLen, bool onlyPacket)
{
	Source src;
	
    if (goep->state!=goep_connected)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be a name. Check it */
    if ((size_name == 0) || (!name))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
	
	src = StreamRegionSource(packet, size_packet);
    
    GOEP_DEBUG( ("Sending Put First Packet\n") );
    if (goepSendPutFirst(goep, size_name, name, size_packet, src, totalLen, onlyPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param src Source which contains the data.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
*/
void GoepLocalPutFirstSource(GOEP *goep,
				  		uint16 size_name, const uint8* name, 
						uint16 size_packet, Source src, 
						uint32 totalLen, bool onlyPacket)
{
    if (goep->state!=goep_connected)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be a name. Check it */
    if ((size_name == 0) || (!name))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    GOEP_DEBUG( ("Sending Put First Packet - Source\n") );
    if (goepSendPutFirst(goep, size_name, name, size_packet, src, totalLen, onlyPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_type Length of the object type.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

*/
void GoepLocalPutFirstType(GOEP *goep,
				  		uint16 size_name, const uint8* name, 
				  		uint16 size_type, const uint8* type, 
						uint16 size_packet, const uint8 *packet, 
						uint32 totalLen, bool onlyPacket)
{
	Source src;
    if (goep->state!=goep_connected)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be a name. Check it */
    if ((size_name == 0) || (!name))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
	
	src = StreamRegionSource(packet, size_packet);
    
    GOEP_DEBUG( ("Sending Put First Packet\n") );
    if (goepSendPutFirstType(goep, size_name, name, size_type, type, size_packet, src, totalLen, onlyPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_type Length of the object type.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param src Source which contains the data.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

*/
void GoepLocalPutFirstTypeSource(GOEP *goep,
				  		uint16 size_name, const uint8* name, 
				  		uint16 size_type, const uint8* type, 
						uint16 size_packet, Source src, 
						uint32 totalLen, bool onlyPacket)
{
    if (goep->state!=goep_connected)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be a name. Check it */
    if ((size_name == 0) || (!name))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    GOEP_DEBUG( ("Sending Put First Packet - Source\n") );
    if (goepSendPutFirstType(goep, size_name, name, size_type, type, size_packet, src, totalLen, onlyPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Put the next packet of a multi-packet object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_packet Length of packet in this request.
	@param packet Packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepLocalPutNextPacket(GOEP *goep,
				 uint16 size_packet, const uint8 *packet, bool lastPacket)
{
	Source src;
    if (goep->state!=goep_pushing)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be data in the body. Check it */
    if ((size_packet == 0) || (!packet))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
	
	src = StreamRegionSource(packet, size_packet);
    
    GOEP_DEBUG( ("Sending Put Next Packet\n") );
    if ( goepSendPutNext(goep, size_packet, src, lastPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Put the next packet of a multi-packet object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_packet Length of packet in this request.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepLocalPutNextPacketSource(GOEP *goep,
				 uint16 size_packet, Source src, bool lastPacket)
{
    if (goep->state!=goep_pushing)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    if (size_packet > goep->pktLen)
    { /* Packet is larger than we are allowed to send */
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    /* According to the Bluetooth GOEP Spec. Version 1.1, there must be data in the body. Check it */
    if ((size_packet == 0))
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_parameters);
        return ;
    }
    
    GOEP_DEBUG( ("Sending Put Next Packet - Source\n") );
    if ( goepSendPutNext(goep, size_packet, src, lastPacket)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalPutCompleteInd(goep, goep_failure);
    }
}

/*
	@brief Respond to a Remote Put start packet from a client.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result Response code for the connection.
*/
void GoepRemotePutResponse(GOEP *goep, goep_svr_resp_codes result)
{
	uint16 respCode;
	
	switch (result)
	{
	case goep_svr_resp_OK:
		respCode = goep_Rsp_Success;
		goep->state = goep_connected;
		break;
	case goep_svr_resp_Continue:
		respCode = goep_Rsp_Continue;
		break;
		default:
		respCode = goep_Rsp_BadRequest;
		break;
	};
	
	/* Send relevant response packet */
	goepSendResponse(goep, respCode);
	if (respCode != goep_Rsp_Continue)
	{
		goepMsgSendRemotePutCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}

/*!
	@brief Delete an Object.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name The object name.
*/
void GoepDelete(GOEP *goep,
				  		uint16 size_name, const uint8* name)
{
    if (goep->state != goep_connected)
    {
		goepMsgSendLocalPutCompleteInd(goep, goep_invalid_command);
        return ;
    }
	
    if (goepSendPutFirst(goep, size_name, name, 0, NULL, 0, TRUE)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendDeleteConfirm(goep, goep_failure);
    }
	else
		goep->state= goep_deleting;
}

/*!
	@brief Respond to a Delete request from a client.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result Response code for the connection.
*/
void GoepRemoteDeleteResponse(GOEP *goep, goep_svr_resp_codes result)
{
	uint16 respCode;
	
	switch (result)
	{
	case goep_svr_resp_OK:
		respCode = goep_Rsp_Success;
		break;
	case goep_svr_resp_Forbidden:
		respCode = goep_Rsp_Forbidden;
		break;
		default:
		respCode = goep_Rsp_BadRequest;
		break;
	};
	
	/* Send relevant response packet */
	goepSendResponse(goep, respCode);
	goep->state = goep_connected;
}

/*
	@brief Acknowledge the receiption of a Remote PUT packet and request the next one.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	
	This function should only be called if the last packet received from a Remote PUT operation had
	the moreData flag set to TRUE.
*/
void GoepRemotePutResponseAck(GOEP *goep)
{
	/* Send relevant response packet */
	goepSendResponse(goep, goep_Rsp_Continue);
}

/****************************************************************************
NAME	
	GoepAbort

DESCRIPTION
	
RETURNS
    void
*/
void GoepAbort(GOEP *goep)
{
	goep_states cmd = goep->state;
    GOEP_DEBUG( ("Sending Abort Packet\n") );
    if ( goepSendAbort(goep)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendAbortConfirm(goep, cmd, goep_failure);
    }
}

/****************************************************************************
NAME	
	GoepSetPath

DESCRIPTION
	
RETURNS
    void
*/
void GoepSetPath(GOEP *goep, uint8 flags, uint16 length, const uint8* folder)
{
    if (goep->state!=goep_connected)
    { /* Can only sensibly change dir when we aren't doing anything else */
		goepMsgSendSetPathConfirm(goep, goep_invalid_command);
        return ;
    }
    
    GOEP_DEBUG( ("Sending Setpath Packet\n") );
    if ( goepSendSetPath(goep, flags, length, folder)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendSetPathConfirm(goep, goep_failure);
    }
}

/*
	@brief Respond to a Remote Set Path packet from a client.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result Response code for the connection.
*/
void GoepRemoteSetPathResponse(GOEP *goep, goep_svr_resp_codes result)
{
	uint16 respCode;
	
	switch (result)
	{
	case goep_svr_resp_OK:
		respCode = goep_Rsp_Success;
		break;
	case goep_svr_resp_Forbidden:
		respCode = goep_Rsp_Forbidden;
		break;
	case goep_svr_resp_NotFound:
		respCode = goep_Rsp_NotFound;
		break;
	case goep_svr_resp_Unauthorized:
		respCode = goep_Rsp_UnauthComp;
		break;
	case goep_svr_resp_ServUnavailable:
		respCode = goep_Rsp_ServUnavail;
		break;
	default:
		respCode = goep_Rsp_BadRequest;
		break;
	};
	
	/* Send relevant response packet */
	goepSendResponse(goep, respCode);
	goep->state = goep_connected;
}

/*!
	@brief Start a GET transaction and wait for the first packet.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the type.
	@param type Type of object to get.
*/
void GoepLocalGetFirstPacket(GOEP *goep,
				  uint16 size_name, const uint8* name,
				  uint16 size_type, const uint8* type)
{
    if (goep->state!=goep_connected)
    { /* Can't start a transfer unless we are idle */
		goepMsgSendLocalGetCompleteInd(goep, goep_invalid_command);
	    goep->useHeaders = FALSE;
        return ;
    }
    
    if ( (size_name==0) && (size_type==0) )
    { /* There must be either a name or a type */
		goepMsgSendLocalGetCompleteInd(goep, goep_invalid_parameters);
		goep->useHeaders = FALSE;
        return ;
    }
    
    GOEP_DEBUG( ("Sending GET Start Packet\n") );
    if ( goepSendGetStart(goep, size_name, name, size_type, type)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalGetCompleteInd(goep, goep_failure);
		goep->useHeaders = FALSE;
    }
}

/*!
	@brief Acknowledge the reception if this packet and get the next one.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	
	This function should only be called if the last packet received from a local GET operation had
	the moreData flag set to TRUE.
*/
void GoepLocalGetAck(GOEP *goep)
{
    if ((goep->state!=goep_pulling_first) && (goep->state!=goep_pulling))
    { /* Can't continue a transfer if one isn't running */
		goepMsgSendLocalGetCompleteInd(goep, goep_invalid_command);
        return ;
    }
    
    GOEP_DEBUG( ("Sending GET Next Packet\n") );
    if ( goepSendGetNext(goep)!=goep_success)
    { /* Failed to send packet */
		goepMsgSendLocalGetCompleteInd(goep, goep_failure);
    }
}

/*!
	@brief Send the first response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	@param size_data Length of the data packet.
	@param data Data to send for the get.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepRemoteGetResponse(GOEP *goep, goep_svr_resp_codes result, uint32 totLen,
				  uint16 size_name, const uint8* name,
				  uint16 size_type, const uint8* type,
				  uint16 size_data, const uint8* data, bool lastPacket)
{
	uint16 respCode = convGoepResponseCode(result, lastPacket);
	Source src = StreamRegionSource(data, size_data);
	
	/* Send relevant response packet */
	goepSendGetResponse(goep, respCode, totLen, name, size_name, type, size_type, src, size_data);
	
	if (respCode != goep_Rsp_Continue)
	{ /* If the last packet is being sent or the request has been rejected */
		goepMsgSendRemoteGetCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}

void GoepRemoteGetResponseHdr(GOEP *goep, goep_svr_resp_codes result, uint32 totLen,
				  uint16 size_name, const uint8* name,
				  uint16 size_type, const uint8* type,
				  uint16 size_hdr, uint8* hdr,
				  uint16 size_data, const uint8* data, bool lastPacket)
{
	uint16 respCode = convGoepResponseCode(result, lastPacket);
	Source src = StreamRegionSource(data, size_data);
	
	/* Send relevant response packet */
	goepSendGetResponseHdr(goep, respCode, totLen, name, size_name, type, size_type, src, size_data, hdr, size_hdr);
	
	if (respCode != goep_Rsp_Continue)
	{ /* If the last packet is being sent or the request has been rejected */
		goepMsgSendRemoteGetCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}


/*!
	@brief Send the first response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	@param size_data Length of the data packet.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepRemoteGetResponseSource(GOEP *goep, goep_svr_resp_codes result, uint32 totLen,
				  uint16 size_name, const uint8* name,
				  uint16 size_type, const uint8* type,
				  uint16 size_data,  Source src, bool lastPacket)
{
	uint16 respCode = convGoepResponseCode(result, lastPacket);
	
	/* Send relevant response packet */
	goepSendGetResponse(goep, respCode, totLen, name, size_name, type, size_type, src, size_data);
	
	if (respCode != goep_Rsp_Continue)
	{ /* If the last packet is being sent or the request has been rejected */
		goepMsgSendRemoteGetCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}

/*!
	@brief Send a subsequent response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_data Length of the data packet.
	@param data Data to send for the get.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepRemoteGetResponsePkt(GOEP *goep,
				  uint16 size_data, const uint8* data, bool lastPacket)
{
	uint16 respCode;
	Source src = StreamRegionSource(data, size_data);
	
	if (lastPacket)
		respCode = goep_Rsp_Success;
	else
		respCode = goep_Rsp_Continue;
		
	/* Send relevant response packet */
	goepSendGetResponse(goep, respCode, 0, NULL, 0, NULL, 0, src, size_data);
	
	if (lastPacket)
	{
		goepMsgSendRemoteGetCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}

/*!
	@brief Send a subsequent response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	@param size_data Length of the data packet.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void GoepRemoteGetResponsePktSource(GOEP *goep,  uint16 size_data, Source src, bool lastPacket)
{
	uint16 respCode;
	
	if (lastPacket)
		respCode = goep_Rsp_Success;
	else
		respCode = goep_Rsp_Continue;
		
	/* Send relevant response packet */
	goepSendGetResponse(goep, respCode, 0, NULL, 0, NULL, 0, src, size_data);
	
	if (lastPacket)
	{
		goepMsgSendRemoteGetCompleteInd(goep, goep_success);
		goep->state = goep_connected;
	}
}

/*!
	@brief The packet received has been processed and is no longer needed.
	@param goep GOEP Session Handle (as returned in GOEP_REGISTER_CFM).
	
	Every packet send to the client that contains a source must be declared complete before the next
	function is called.
	e.g. When a GOEP_REMOTE_PUT_START_IND has been received, GoepPacketComplete must be called before calling
	GoepRemotePutResponse.
*/
void GoepPacketComplete(GOEP *goep)
{
	if (goep->srcUsed > 0)
	{
		SourceDrop(StreamSourceFromSink(goep->sink), goep->srcUsed);
		goep->srcUsed = 0;
	}
}

/*!
	@brief Send a packet containing Application Specific Headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param length The total length of the paramters added.
	
	This function should be called on receipt of a GOEP_GET_APP_HEADERS_IND message
	when the parameters have been added to the packet.
*/
void GoepSendAppSpecificPacket(GOEP *goep, uint16 len_used)
{
    if ((goep->state != goep_pulling_first) && (goep->state != goep_remote_get))
    { /* Can't start a transfer unless we are idle */
		GOEP_DEBUG(("   IN THE WRONG STATE AND OUR SINK COULD BE IN A BIT OF A MESS\n"));
		Panic();
    }
	
	goepSendAppSpecific(goep, len_used);
}

/*!
	@brief Start a GET transaction using application specific headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the type.
	@param type Type of object to get.
	
	A GOEP_LOCAL_GET_START_IND message will be received for the first packet.
	A GOEP_LOCAL_GET_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalGetFirstPacketHeaders(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type)
{
	goep->useHeaders = TRUE;
	GoepLocalGetFirstPacket(goep, size_name, name, size_type, type);
}

/*!
	@brief Send the first response to a Get, including application specific headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	
	A GOEP_GET_APP_HEADERS_IND message will be received to indicate that the application
	should add it's custom parameters.  When complete, GoepSendAppSpecificPacket should
	be called to actually send the packet.
*/
void GoepRemoteGetResponseHeaders(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type)
{
	goep->useHeaders = TRUE;
	
	GoepRemoteGetResponse(goep, result, totLen,
				  size_name, name, size_type, type, 0, NULL, FALSE);
}

void GoepRemoteGetResponseAppHeaders(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_header, uint8* header, uint16 size_type, const uint8* type)
{
	GoepRemoteGetResponseHdr(goep, result, totLen,
				  0, NULL, size_type, type, size_header, header, 0, NULL, TRUE);
}


