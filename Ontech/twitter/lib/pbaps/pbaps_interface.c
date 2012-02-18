/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_interface.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - public interface.

*/

#include <vm.h>
#include <print.h>
#include <memory.h>
#include <pbap_common.h>

#include "pbaps.h"
#include "pbaps_private.h"


/*!
	@brief Initialise the PBAP Server Library.
	@param theAppTask The current application task.
	@param priority The profile instance low power mode priority. For a set of
	profile instances with connections on a given ACL the instance with the
	highest priority value wins and the low power mode on the ACL is set
	according to it's power table.
	@param repsositories Repositories supported by the server.  Mask values
	defined in pbap_common.h.
	
	PBAPS_INIT_CFM message will be received by the application. 
*/
void PbapsInit(Task theAppTask, uint16 priority, uint8 repositories)
{
	pbapsState *state = PanicUnlessNew(pbapsState);
	
	PRINT(("PBAPS Init\n"));
		
	memset(state, 0, sizeof(pbapsState));
	
	state->task.handler = pbapsIntHandler;
	state->theAppTask = theAppTask;
	state->repos = repositories;
		
	GoepInit(&state->task, goep_Server, goep_PBAP_PSE);
}

/*!
 @brief Respond to a connection request.  
 @param pbaps PBAPS Session Handle.
 @param accept TRUE to accept the connection, or FALSE to refuse it.
 @param pktSize Maximum packet size that can be accepted on this connection.
*/
void PbapsConnectResponse(PBAPS *pbaps, bool accept, uint16 pktSize)
{
    Task state;
    
    state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_CONNECTION_RESP);
        
        message->accept = accept;
        message->pktSize = pktSize;
        
		MessageSend(state, PBAPS_INT_CONNECTION_RESP, message);
	}
}

/*
	@brief Start an authentication challenge during connect.
    @param pbaps PBAPS Session Handle.
	@param nonce MD5 Digest Nonce to use. Must be a 16byte string.
	@param options Optional options to use.
	@param size_realm Length of the optional Realm string.
	@param realm Option realm string.
	
	This function can be called by a GOEP Server on receipt of a PBAPS_CONNECT_IND message to start
	and authentication challenge.
	A PBAPS_AUTH_RESULT_IND message will be received to indicate the result of the authentication challenge.
*/
void PbapsConnectAuthChallenge(PBAPS *pbaps, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm)
{
    Task state;
    
    state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_AUTH_CHALLENGE);
        
        message->nonce = nonce;
        message->options = options;
        message->size_realm = size_realm;
        message->realm = realm;
        
		MessageSend(state, PBAPS_INT_AUTH_CHALLENGE, message);
	}
}

/*!
    @brief Respond to a Set Phonebook Root request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_ROOT_IND message.
*/
void PbapsSetPhonebookRootResponse(PBAPS *pbaps, pbaps_set_phonebook_result result)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_spb_ok) && (result < pbaps_spb_eol)), ("PBAC - Invalid Set Phonebook Response (Root)\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_SET_PB_ROOT_RESP);
        
        message->result = result;
        
		MessageSend(state, PBAPS_INT_SET_PB_ROOT_RESP, message);
	}
}

/*!
    @brief Respond to a Set Phonebook Repository request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_REPOSITORY_IND message.
*/
void PbapsSetPhonebookRepositoryResponse(PBAPS *pbaps, pbaps_set_phonebook_result result)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_spb_ok) && (result < pbaps_spb_eol)), ("PBAC - Invalid Set Phonebook Response (Repository)\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_SET_PB_REPOSITORY_RESP);
        
        message->result = result;
        
		MessageSend(state, PBAPS_INT_SET_PB_REPOSITORY_RESP, message);
	}
}

/*!
    @brief Respond to a Set Phonebook sub-folder request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_BOOK_IND message.
*/
void PbapsSetPhonebookBookResponse(PBAPS *pbaps, pbaps_set_phonebook_result result)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_spb_ok) && (result < pbaps_spb_eol)), ("PBAC - Invalid Set Phonebook Response (Book)\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_SET_PB_BOOK_RESP);
        
        message->result = result;
        
		MessageSend(state, PBAPS_INT_SET_PB_BOOK_RESP, message);
	}
}

/*!
    @brief Respond to a Set Phonebook parent request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_PARENT_IND message.
*/
void PbapsSetPhonebookParentResponse(PBAPS *pbaps, pbaps_set_phonebook_result result)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_spb_ok) && (result < pbaps_spb_eol)), ("PBAC - Invalid Set Phonebook Response (Book)\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_INT_SET_PB_PARENT_RESP);
        
        message->result = result;
        
		MessageSend(state, PBAPS_INT_SET_PB_PARENT_RESP, message);
	}
}

/*!
    @brief Respond to a Get vCard List request sending application parameters.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param pbook_size The size of the phonebook if MaxListCount was zero.
	@param new_missed The number of new missed calls when the phonebook object us mch.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
*/
void PbapsGetvCardListFirstPacketParams(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint16 pbook_size, uint8 new_missed)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_LIST_FIRST);
        
        message->result = result;
		message->totalLen = totalLen;
		message->pbook_size = pbook_size;
		message->new_missed = new_missed;
		message->packet = NULL;
		message->size_packet = 0;
		message->onlyPacket = FALSE;
        
		MessageSend(state, PBAPS_GET_VCARD_LIST_FIRST, message);
	}
}

/*!
    @brief Respond to a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
*/
void PbapsGetvCardListFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_LIST_FIRST);
        
        message->result = result;
		message->totalLen = totalLen;
		message->pbook_size = 0;
		message->new_missed = 0;
		message->packet = packet;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_VCARD_LIST_FIRST, message);
	}
}

/*!
    @brief Respond to a Get vCard List request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
*/
void PbapsGetvCardListFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_FIRST_SRC);
        
		message->command = pbaps_com_PullvCardList;
        message->result = result;
		message->totalLen = totalLen;
		message->src = src;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_SEND_FIRST_SRC, message);
	}
}

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_LIST_NEXT_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT_SRC);
        
		message->command = pbaps_com_PullvCardList;
		message->src = src;
		message->size_packet = size_packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT_SRC, message);
	}
}

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardListFirstPacketSource should be used instead of
	PbapsGetvCardListFirstPacket.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_LIST_NEXT_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT);
        
		message->command = pbaps_com_PullvCardList;
		message->size_packet = size_packet;
		message->packet = packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT, message);
	}
}

/*!
    @brief Respond to a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Entry
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardEntryFirstPacketSource should be used instead of
	PbapsGetvCardEntryFirstPacket.
	
	This function should be called on recept of a PBAPS_GET_VCARD_ENTRY_START_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_ENTRY_FIRST);
        
        message->result = result;
		message->totalLen = totalLen;
		message->packet = packet;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_VCARD_ENTRY_FIRST, message);
	}
}

/*!
    @brief Respond to a Get vCard Entry request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Entry
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_VCARD_ENTRY_START_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_FIRST_SRC);
        
		message->command = pbaps_com_PullvCard;
        message->result = result;
		message->totalLen = totalLen;
		message->src = src;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_SEND_FIRST_SRC, message);
	}
}

/*!
    @brief Send the next packet of a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardEntryNextPacketSource should be used instead of
	PbapsGetvCardEntryNextPacket.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_ENTRY_NEXT_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT);
        
		message->command = pbaps_com_PullvCard;
		message->size_packet = size_packet;
		message->packet = packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT, message);
	}
}

/*!
    @brief Send the next packet of a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_ENTRY_NEXT_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT_SRC);
        
		message->command = pbaps_com_PullvCard;
		message->src = src;
		message->size_packet = size_packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT_SRC, message);
	}
}

/*!
    @brief Respond to a Get Phonebook request sending application parameters.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param pbook_size The size of the phonebook if MaxListCount was zero.
	@param new_missed The number of new missed calls when the phonebook object us mch.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
*/
void PbapsGetPhonebookFirstPacketParams(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint16 pbook_size, uint8 new_missed)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_PHONEBOOK_FIRST);
        
        message->result = result;
		message->totalLen = totalLen;
		message->pbook_size = pbook_size;
		message->new_missed = new_missed;
		message->packet = NULL;
		message->size_packet = 0;
		message->onlyPacket = FALSE;
        
		MessageSend(state, PBAPS_GET_PHONEBOOK_FIRST, message);
	}
}

/*!
    @brief Respond to a Get Phonebook request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetPhonebookFirstPacketSource should be used instead of
	PbapsGetPhonebookFirstPacket.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
	A PBAPS_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_PHONEBOOK_FIRST);
        
        message->result = result;
		message->totalLen = totalLen;
		message->pbook_size = 0;
		message->new_missed = 0;
		message->packet = packet;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_PHONEBOOK_FIRST, message);
	}
}

/*!
    @brief Respond to a Get vCard List request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
	A PBAPS_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket)
{
    Task state;
    
	PBAPS_ASSERT(((result >= pbaps_get_ok) && (result < pbaps_get_eol)), ("PBAC - Invalid Get Response\n")); 
	
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_FIRST_SRC);
        
		message->command = pbaps_com_PullPhonebook;
        message->result = result;
		message->totalLen = totalLen;
		message->src = src;
		message->size_packet = size_packet;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, PBAPS_GET_SEND_FIRST_SRC, message);
	}
}

/*!
    @brief Send the next packet of a Get Phonebook request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetPhonebookNextPacketSource should be used instead of
	PbapsGetPhonebookNextPacket.
	
	This function should be called on receipt of a PBAPS_GET_PHONEBOOK_NEXT_IND message.
	A PBAPS_GET_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT);
        
		message->command = pbaps_com_PullPhonebook;
		message->size_packet = size_packet;
		message->packet = packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT, message);
	}
}

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_PHONEBOOK_NEXT_IND message.
	A PBAPS_GET_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket)
{
    Task state;
    
	state = (Task)(&pbaps->task);
	
	/* Send an internal message */
	{
		MAKE_PBAPS_MESSAGE(PBAPS_GET_SEND_NEXT_SRC);
        
		message->command = pbaps_com_PullPhonebook;
		message->src = src;
		message->size_packet = size_packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, PBAPS_GET_SEND_NEXT_SRC, message);
	}
}
