/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_authorise.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_security_auth.h"

#include    <message.h>
#include	<string.h>
#include    <vm.h>


/*****************************************************************************/
void ConnectionSmAuthoriseResponse(const bdaddr* bd_addr, dm_protocol_id protocol_id, uint32 channel, bool incoming, bool authorised)
{
#ifdef CONNECTION_DEBUG_LIB
	if ((protocol_id != protocol_l2cap) && (protocol_id != protocol_rfcomm))
	{
		CL_DEBUG(("ConnectionSmAuthoriseResponse - Out of range protocol id 0x%x\n", protocol_id));
	}

    if((protocol_id == protocol_rfcomm) && ((channel < RFCOMM_SERVER_CHANNEL_MIN) || (channel > RFCOMM_SERVER_CHANNEL_MAX)))
    {
        CL_DEBUG(("cd ..Out of range RFCOMM server channel 0x%lx\n", channel));
    }

    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHORISE_RES);
        message->bd_addr = *bd_addr;
        message->protocol_id = protocol_id;
        message->channel = channel;
        message->incoming = incoming;
        message->authorised = authorised;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_AUTHORISE_RES, message);
    }
}


/*****************************************************************************/
void ConnectionSmPinCodeResponse(const bdaddr* bd_addr, uint16 length, const uint8* pin_code)
{   
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_PIN_REQUEST_RES);
        message->bd_addr = *bd_addr;
        message->pin_length = length;
        if (length > 0)
            memcpy(message->pin, pin_code, length);
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_PIN_REQUEST_RES, message);
    }
}


/*****************************************************************************/
void ConnectionSmIoCapabilityResponse(const bdaddr* bd_addr, cl_sm_io_capability io_capability, bool force_mitm, bool bonding, bool oob_data_present, uint8* oob_hash_c, uint8* oob_rand_r)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES);
		
        message->bd_addr = *bd_addr;
        message->io_capability = io_capability;
        message->bonding = bonding;
		message->mitm = force_mitm;
		
        if(oob_data_present)
        {
            message->oob_data_present = 1;
			message->oob_hash_c = PanicUnlessMalloc(CL_SIZE_OOB_DATA);
			message->oob_rand_r = PanicUnlessMalloc(CL_SIZE_OOB_DATA);
            memcpy(message->oob_hash_c, oob_hash_c, CL_SIZE_OOB_DATA);
            memcpy(message->oob_rand_r, oob_rand_r, CL_SIZE_OOB_DATA);
        }
        else
        {
            message->oob_data_present = 0;
            message->oob_hash_c = NULL;
            message->oob_rand_r = NULL;
        }
		
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES, message);
    }
}

/*****************************************************************************/
void ConnectionSmUserConfirmationResponse(const bdaddr* bd_addr, bool confirm)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES);
	message->bd_addr = *bd_addr;
	message->confirm = confirm;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES, message);
}


/*****************************************************************************/
void ConnectionSmUserPasskeyResponse(const bdaddr* bd_addr, bool cancelled, uint32 passkey)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES);
	message->bd_addr = *bd_addr;
	message->cancelled = cancelled;
	message->numeric_value = passkey;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES, message);
}


/*****************************************************************************/
void ConnectionSmDeleteAuthDevice(const bdaddr* bd_addr)
{
    /* Remove the device from the trusted device list */
    connectionAuthDeleteDevice(bd_addr);
}



/*****************************************************************************/
void ConnectionSmDeleteAllAuthDevices(uint16 ps_base)
{
    /* Remove all devices from the trusted device list */
    connectionAuthDeleteAllDevice(ps_base);
}




