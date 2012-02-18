/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_auth.c        

DESCRIPTION
	This file contains the functionality to manage a list of trusted devices.
    A list of devices is maintained in the persistent store.  This list 
    contains the Bluetooth device address, link key and trust status of upto
    eight devices.  The number of devices to manage is configured dynamically
    as a parameter to the Initialisation function

NOTES

*/

/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_security_auth.h"

#include <bdaddr.h>
#include <message.h>
#include <panic.h>
#include <ps.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>


/*lint -e525 -e725 -e830 */

/* 
   The trusted device list is stored in the user area of Persistent store.  The
   following defines it's location
*/
#define	TRUSTED_DEVICE_LIST_BASE		(42)
#define	TRUSTED_DEVICE_LIST_LENGTH		(8)
#define TRUSTED_DEVICE_LIST_END			((TRUSTED_DEVICE_LIST_BASE + TRUSTED_DEVICE_LIST_LENGTH) - 1)
#define	MAX_TRUSTED_DEVICES				((TRUSTED_DEVICE_LIST_END - TRUSTED_DEVICE_LIST_BASE) + 1)

/*
   In order to maintain a record of the Most Recently Used (MRU) device, an
   index is used to store the device usage order 
*/
#define	TRUSTED_DEVICE_INDEX			(41)


/*
   This value is retuned if an entry found in the Trusted Device List is
   not indexed from the Trusted Device Index
*/
#define	INDEX_INVALID					(99)
    
/* 
   Trusted device record, each record contains the Bluetooth device address
   of the trusted device, the link key generated during pairing and it's type, 
 */
typedef struct
{
	bdaddr					bd_addr;
	uint8					link_key[SIZE_LINK_KEY];
	uint16					trusted;
	cl_sm_link_key_type		link_key_type;
}TrustedDeviceRecordType;

/*
	The Trusted Device index is used to maintain a list of the Most Recently 
	Used trusted device.  This index is reordered each time the bonding procedure
	is successful with a remote device.  Each item in this list refers to an entry
	in the Trusted Device List.  Adopting this approach results in Persistent Store
	write operations being kept to a minimum
*/
typedef struct
{
	uint16						order[TRUSTED_DEVICE_LIST_LENGTH];
}TrustedDeviceIndexType;


/* The number of devices to be manage */
#define NO_DEVICES_TO_MANAGE    (8)


/****************************************************************************

DESCRIPTION
	This function creates and populates a DM_SM_SSP_ADD_DEVICE_REQ primitive 
    and sends it to Bluestack to register a device with the Bluestack 
    Security Manager
*/
static void register_device(cl_dm_bt_version version, const TrustedDeviceRecordType* p_dev_record)
{	
	if(version != bluetooth_unknown)
	{
		/* If we have a known bluetooth version firmware must support latest DM prims */
		MAKE_PRIM_T(DM_SM_SSP_ADD_DEVICE_REQ);
		connectionConvertBdaddr_t(&prim->bd_addr, &p_dev_record->bd_addr);
    	prim->update_trust_level = TRUE;
    	prim->trusted = p_dev_record->trusted;
    	prim->update_link_key = TRUE;
    	memcpy(prim->link_key, p_dev_record->link_key, SIZE_LINK_KEY);
		prim->link_key_type = connectionConvertLinkKeyType_t(p_dev_record->link_key_type);
		VmSendDmPrim(prim);
	}
	else
	{
		/* Old firmware, use old prims */
		MAKE_PRIM_T(DM_SM_ADD_DEVICE_REQ);
		connectionConvertBdaddr_t(&prim->bd_addr, &p_dev_record->bd_addr);
    	prim->update_trust_level = TRUE;
    	prim->trusted = p_dev_record->trusted;
    	prim->update_link_key = TRUE;
    	memcpy(prim->link_key, p_dev_record->link_key, SIZE_LINK_KEY);
		VmSendDmPrim(prim);
	}
}

/****************************************************************************

DESCRIPTION
	This function creates and populates a DM_SM_REMOVE_DEVICE_REQ primitive 
    and sends it to Bluestack to unregister a device with the Bluestack 
    Security Manager
*/
static void unregister_device(const bdaddr* bd_addr)
{
	MAKE_PRIM_T(DM_SM_REMOVE_DEVICE_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, bd_addr);
    VmSendDmPrim(prim);
}


/****************************************************************************

DESCRIPTION
	This function searches for the specified peer device in the Trusted Device 
	List (TDL). The value returned as position defines the record number of the
	device in TDL.

	 --------------------------------------
	|R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 |
	 --------------------------------------

	If the value returned is zero then the device does not exist in
	the TDL
*/
static uint16 find_trusted_device(const bdaddr* p_peer_addr)
{
	uint16						rec;
    TrustedDeviceRecordType     record;

	/* Loop through list of trusted devices */
	for(rec = 0; rec < NO_DEVICES_TO_MANAGE; rec++)
	{
		if(PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType)))
		{
			/* If this device is already a trusted device? */
			if(BdaddrIsSame(&record.bd_addr, p_peer_addr))
				return (rec + 1);
		}
	}
	/* Device is not in the trusted device list */
	return 0;
}


/****************************************************************************

DESCRIPTION
	This function will find the next free position in the Trusted Device List.

*/
static uint16 find_free_position(void)
{
	uint16						rec,index;
	TrustedDeviceIndexType		tdi;
	uint16						pos;
    TrustedDeviceRecordType     record;

	/* Loop through list of trusted devices */
	for(rec = 0; rec < NO_DEVICES_TO_MANAGE; rec++)
	{
		if(!PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType)))
        {
			return (rec + 1);
        }
	}

	/* As the Trusted Device List is full then the position will be the
	   one occupied by the LRU device */
	(void)PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));
	
	/* The LRU device is either the last device in the index or if empty (0) we
	   must traverse through the list to find the device */
	index = NO_DEVICES_TO_MANAGE;
	do
	{
		if(tdi.order[--index])
			break;
	}
	while(index);

	/* Record position */
	pos = tdi.order[index];
	
	if (PsRetrieve(TRUSTED_DEVICE_LIST_BASE + pos - 1, &record, sizeof(TrustedDeviceRecordType)))
		/* Remove from the BlueStack security datatbase */
		unregister_device(&record.bd_addr);

	/* Remove LRU device from the TDI */
	tdi.order[index] = 0; 
	(void)PsStore(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));	

	return pos;
}

/****************************************************************************

DESCRIPTION
	This function searches the Trusted Device Index (TDI) for a device 
	referenced by it's position in Trusted Device List (TDL).

	The TDI keeps a track of the order in which devices were connected
	too.  Each entry in the TDI holds a reference to a record
	in the TDL.  The TDI entries are ordered from Most Recently
	Used (MRU) to Least Recently Used (LRU)
	
	 --------------------------------------
	|Ra | Rb | Rc | Rd | Rd | Re | Rf | Rg |
	 --------------------------------------
	 ^									^
	 |									|
	 MRU								LRU
	
*/
static uint16 search_trusted_device_index(const uint16 pos)
{
	TrustedDeviceIndexType		tdi;
	uint16						offset = INDEX_INVALID;
	
	/* Read the TDI from persistent store */
	if(PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
	{
		/* Loop through the TDI searching for an occurance of the device reference */
		for(offset = 0; offset < NO_DEVICES_TO_MANAGE; offset++)
		{
			/* If the required device is found in the TDI, return it's relative
			   position (offset) in the TDI */
			if(tdi.order[offset] == pos)
				break;
		}
	}

	return offset;
}


/****************************************************************************

DESCRIPTION
	This function updates the Trusted Device Index
*/
static void update_trusted_device_index(const uint16 position, const uint16 order)
{
	TrustedDeviceIndexType		tdi;
	uint16						index;
	
	/* Read the TDI from persistent store */
	if(PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
	{
		if(order != INDEX_INVALID)
		{
			/* Re-order TDI */
			for(index = 0; index < order; index++)
				tdi.order[order - index] = tdi.order[(order - index) - 1];
		}
		
		/* Update with the position of the new record in the TDL as MRU device */
		tdi.order[0] = position;
		
		/* Store persistently */
		(void)PsStore(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));
	}
}


/****************************************************************************

DESCRIPTION
	This function updates a record in the Trusted Device List
*/
static uint16 update_trusted_device_list ( cl_dm_bt_version version, 
										const uint16 position, 
										const bdaddr* peer_bd_addr, 
										uint8 link_key_type, 
										const uint8* link_key,
                                        uint16 trusted,
										uint16 bonded)
{
	uint16 rec = 0;
    uint16 ok = TRUE;
    TrustedDeviceRecordType record;

	/* Check position is valid */
	if(position)
		rec  = position - 1;

	/* Populate device record structure */
	record.bd_addr = *peer_bd_addr;
	record.link_key_type = link_key_type;
	memcpy(record.link_key, link_key, SIZE_LINK_KEY);
	record.trusted = trusted;
	
	if(bonded)
	{		
		/* Store trusted device persistently in the list */
		if(PsStore(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType)))
    	{
        	/* Update the device in the Bluestack Security Manager database */
			register_device(version, &record);
    	}
    	else
    	{
        	ok = FALSE;
    	}
	}
	else
	{
		/* Update the device in the Bluestack Security Manager database */
		register_device(version, &record);
	}
	
    return ok;     
}


/****************************************************************************

DESCRIPTION
	This function will delete an entry from the trusted device index
*/
static uint16 delete_from_trusted_device_index(uint16 order, uint16 noDevices)
{
    TrustedDeviceIndexType		tdi;
	uint16						index;
    uint16                      ok = FALSE;
	
	/* Read the TDI from persistent store */
	if(PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
	{
		if(order != INDEX_INVALID)
		{
			/* Delete index from TDI and reorder TDI */
			for(index = order; index < (noDevices - 1); index++)
				tdi.order[index] = tdi.order[index+1];

            /* LRU index is now invalid */
            tdi.order[noDevices - 1] = 0;
		}
		
		/* Store persistently */
		(void)PsStore(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));

        ok = TRUE;
	}

	return ok;
}


/****************************************************************************
NAME
	connectionAuthInit

FUNCTION
	This function is called to initialise the Trusted Device List.  All
	devices in the list are registered with the Bluestack Security Manager.

RETURNS
	The number of devices registered with Bluestack
*/
uint16 connectionAuthInit(cl_dm_bt_version version)
{
	uint16					registeredDevices = 0;
	TrustedDeviceIndexType	tdi;
	uint16					rec;
    TrustedDeviceRecordType record;
	
	/* Check if we have a valid Trusted Device Index */
	if(!PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
	{
		/* Reset TDI to zero */
		memset(&tdi, 0, sizeof(TrustedDeviceIndexType));
		(void)PsStore(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));

		/* Delete TDL */
		for(rec = TRUSTED_DEVICE_LIST_BASE; rec <= TRUSTED_DEVICE_LIST_END; rec++)
			(void)PsStore(rec, NULL, 0);
	}
	else
	{
		/* Search through the Trusted Device List */
		for(rec = 0; rec < NO_DEVICES_TO_MANAGE; rec++)
		{
			/* For all devices in the list... */
			if(PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType)))
			{
				
				/* Register the device with Bluestack Security Manager */
				register_device(version, &record);
				
				/* Keep a count of the number of active device records */
				registeredDevices++;
			}
		}
	}

	/* Return the number of devices registered with the Bluestack Security Manager */
	return registeredDevices;
}

/****************************************************************************
NAME
	connectionAuthAddDevice

FUNCTION
	This function is called to add a trusted device to the persistent trusted 
	device list.  A flag indicating if the device was successfully added is
	returned.
*/
uint16 connectionAuthAddDevice(cl_dm_bt_version version, const bdaddr* peer_bd_addr, cl_sm_link_key_type link_key_type, const uint8* link_key, uint16 trusted, uint16 bonded)
{
	/* Holds the position of a device in the trusted device list (TDL) */
	uint16		position = 0;
	/* Defines the Most Recently Used (MRU) order of a device  */
	uint16		order = 0;

	/* Search the trusted device list for the specified device */
	position = find_trusted_device(peer_bd_addr);
	
	/* If the device to be added is currently stored in the TDL */
	if(position)
	{
		/* Determine the MRU order of the device by searching for the 
		   device position in the Trusted Device Index (TDI) */
		order = search_trusted_device_index(position);
	}
	else
	{
		/* The device is not currently in the trusted device list, find
		   a free position in the TDL to store the new device.  The next
		   free position will either be an empty one or the LRU. Dont do
 		   this if we're not bonding as we dont need to store the device */
		if(bonded)
		{
			order = NO_DEVICES_TO_MANAGE - 1;
			position = find_free_position();

			/* Limit check position */
			if(position == 0)
				position = 1;
		}
	}

	if(bonded)
	{
		/* Keep a track of the the most recently used device by updating the TDI */
		update_trusted_device_index(position, order);
	}
	else
	{
		/* If we have a bonded key for this device */
		if(position)
		{
			/* Delete it and re-order TDI */
        	(void) delete_from_trusted_device_index(order, NO_DEVICES_TO_MANAGE);

        	/* Delete device from TDL */
        	(void) PsStore(TRUSTED_DEVICE_LIST_BASE + position - 1, NULL, 0);
		}
	}

	/* Store the new device in the TDL (or just register it if non bonded) */
	return update_trusted_device_list(version, position, peer_bd_addr, link_key_type, link_key, trusted, bonded);
}

/****************************************************************************
NAME
	connectionAuthGetDevice

FUNCTION
	This function is called to add a trusted device to the persistent trusted 
	device list.  A flag indicating if the device was found is returned.
*/
uint16 connectionAuthGetDevice(const bdaddr *peer_bd_addr, cl_sm_link_key_type *link_key_type, uint8 *link_key, uint16 *trusted)
{
	uint16						rec;
	uint16						res = FALSE;
    TrustedDeviceRecordType     record;
    

	/* Search for the device in the TDL */
	rec = find_trusted_device(peer_bd_addr);

    /* If the device is in the TDL */
	if(rec)
	{
		rec--;
		/* Get the link key */
		(void)PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType));
		res = TRUE;

		*trusted = record.trusted;
		*link_key_type = record.link_key_type;
		memcpy(link_key, record.link_key, SIZE_LINK_KEY);
	}
	
	return res;
}



/****************************************************************************
NAME
	connectionAuthDeleteDevice

FUNCTION
	This function is called to remove a trusted device from the persistent 
    trusted device list.  A flag indicating if the device was successfully removed 
    is returned.
*/
uint16 connectionAuthDeleteDevice(const bdaddr* peer_bd_addr)
{
    /* Holds the position of a device in the trusted device list (TDL) */
	uint16		position = 0;

    /* Defines the order or the device in the TDI */
	uint16		order = 0;

    /* Flag to indicate if the device was deleted */
    uint16      deleted = FALSE;

	
    /* Search the trusted device list for the specified device */
	position = find_trusted_device(peer_bd_addr);
	
	/* If the device is in the TDL */
	if(position)
	{
        /* Find this device in the TDI */
		order = search_trusted_device_index(position);

        /* Delete it and re-order TDI */
        (void) delete_from_trusted_device_index(order, NO_DEVICES_TO_MANAGE);

        /* Delete device from TDL */
        (void) PsStore(TRUSTED_DEVICE_LIST_BASE + position - 1, NULL, 0);

        /* Remove from the BlueStack security datatbase */
		unregister_device(peer_bd_addr);

		deleted = TRUE;	
	}
	return deleted;
}


/****************************************************************************
NAME
	connectionAuthDeleteAllDevices

FUNCTION
	This function is called to remove all trusted devices from the persistent 
    trusted device list.  A flag indicating if all the devices were successfully 
	removed is returned.
*/
uint16 connectionAuthDeleteAllDevice(uint16 ps_base)
{
	/* Flag to indicate if the devices were deleted */
    uint16      				deleted = FALSE;
	
	/* Trusted device list record index */
	uint16						rec = 0;
	
	/* Trusted device record */
	TrustedDeviceRecordType     record;
	
	/* trusted device index */
	TrustedDeviceIndexType		tdi;
	
	/* Loop through list of trusted devices */
	for(rec = 0; rec < NO_DEVICES_TO_MANAGE; rec++)
	{
		if(PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType)))
		{
			/* Unregister with Bluestack security manager */
			unregister_device(&record.bd_addr);
			
			/* Delete entry from TDL */
			(void)PsStore(TRUSTED_DEVICE_LIST_BASE + rec, NULL, 0);

			deleted = TRUE;
		}

		/* Delete any associated attribute data */
		if(ps_base)
		{
			(void)PsStore(ps_base + rec, NULL, 0);
		}
	}
	
	/* Delete TDI */
	if(deleted)
	{
		memset(&tdi, 0, sizeof(TrustedDeviceIndexType));
		(void)PsStore(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi));
	}
	
	return deleted;
}


/****************************************************************************
NAME
	connectionAuthSetTrustLevel

FUNCTION
	This function is called to set the trust level of a device stored in the
    trusted device list.  The Blustack Security Manager is updated with the
    change.

RETURNS
	TRUE is record updated, otherwise FALSE
*/
uint16 connectionAuthSetTrustLevel(cl_dm_bt_version version, const bdaddr* peer_bd_addr, uint16 trusted)
{
    /* Holds the position of a device in the trusted device list (TDL) */
	uint16  rec = 0;
    TrustedDeviceRecordType record;
    
    /* Search for the device in the TDL */
	rec = find_trusted_device(peer_bd_addr);

	
	/* If the device is in the TDL */
	if(rec)
	{
		rec--;

		/* Read the record */
		(void)PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType));

        /* Update the trust level */
        record.trusted = trusted;

        /* Store the record */
	    (void)PsStore(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType));

        /* Update Bluestack Security Manager Database */
        register_device(version, &record);
		
        /* Record updated */
        return TRUE;
    }

	
    /* Record for this device does not exist */
    return FALSE;
}


/****************************************************************************
NAME
	connectionAuthUpdateMru

FUNCTION
	This function is called to keep a track of the most reecently used device.
	The TDI index is updated provided that the device specified is currently
	stored in the TDL.
*/
uint16 connectionAuthUpdateMru(const bdaddr* peer_bd_addr)
{
	/* Holds the position of a device in the trusted device list (TDL) */
	uint16		position = 0;
	/* Defines the Most Recently Used (MRU) order of a device */
	uint16		order = 0;
	/* Assume device is NOT in the TDL */
	uint16		found = FALSE;

	/* Search the trusted device list for the specified device */
	position = find_trusted_device(peer_bd_addr);
	
	/* If the device to be added is currently stored in the TDL */
	if(position)
	{
		/* The device is currently stored in TDL */
		found = TRUE;

		/* Determine the MRU order of the device by searching for the 
		   device position in the Trusted Device Index (TDI) */
		order = search_trusted_device_index(position);
	
		/* Keep a track of the the most recently used device by updating the TDI */
		update_trusted_device_index(position, order);
	}
	
	return found;
}


/****************************************************************************
NAME
	connectionAuthGetMruBdAddr

FUNCTION
	This function is called get the Bluetooth Device Address of the Most
	Recently Used device
*/
/*bdaddr connectionAuthGetMruBdAddr(void)
{
	TrustedDeviceIndexType		tdi;
    bdaddr                   bd_addr;
    TrustedDeviceRecordType     record;

	
	memset(&record, 0, sizeof(TrustedDeviceRecordType));*/

	/* Read Trusted Device Index from Persistent store */
	/*if(PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
	{*/

		/* Read the device record from the Trusted Device List */
		/*(void)PsRetrieve((TRUSTED_DEVICE_LIST_BASE + tdi.order[0]) - 1, &record, sizeof(TrustedDeviceRecordType));
	}*/
	
	/* Return Bluetooth Device Address */
	/*bd_addr = record.bd_addr;

	
    return bd_addr;
}*/


/****************************************************************************
NAME
	connectionAuthSendLinkKey

FUNCTION
	This function is called to send the link key of the specified device to the
	Bluestack Security Manager

*/
void connectionAuthSendLinkKey(const bdaddr* peer_bd_addr)
{
	uint16						rec;
    TrustedDeviceRecordType     record;
	
    MAKE_PRIM_T(DM_SM_LINK_KEY_REQUEST_RES);
	connectionConvertBdaddr_t(&prim->bd_addr, peer_bd_addr);

	/* Search for the device in the TDL */
	rec = find_trusted_device(peer_bd_addr);
	
    /* If the device is in the TDL */
	if(rec)
	{
		rec--;
		/* Get the link key */
		(void)PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType));

		prim->valid = 1;
		memcpy(prim->key, record.link_key, SIZE_LINK_KEY);
	}
	else
	{
		/* Reject the request for a link key */
		prim->valid = 0;        
		memset(prim->key, 0, SIZE_LINK_KEY);
	}

	
	/* Send message to the Connection Manager */
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME
	connectionAuthSendSspLinkKey

FUNCTION
	This function is called to send the link key of the specified device to the
	Bluestack Security Manager in mode 4. In addition to the link key this also
	sends the link key type

*/
bool connectionAuthSendSspLinkKey(const bdaddr* peer_bd_addr, bool authenticated)
{
	uint16						rec;
    TrustedDeviceRecordType     record;
	bool 						success;
	uint8*						link_key;
	
    MAKE_PRIM_T(DM_SM_SSP_LINK_KEY_REQUEST_RES);
	connectionConvertBdaddr_t(&prim->bd_addr, peer_bd_addr);

	/* Search for the device in the TDL */
	rec = find_trusted_device(peer_bd_addr);
	
    /* If the device is in the TDL */
	if(rec)
	{
		rec--;
		/* Get the link key */
		(void)PsRetrieve(TRUSTED_DEVICE_LIST_BASE + rec, &record, sizeof(TrustedDeviceRecordType));

		/* Handle old records with no key type stored */
		if(record.link_key_type == 0)
			record.link_key_type = cl_sm_link_key_legacy;
		
		if(authenticated)
		{
			if(record.link_key_type == cl_sm_link_key_authenticated)
			{
				prim->key_type = connectionConvertLinkKeyType_t(record.link_key_type);
				link_key = (uint8*) PanicUnlessMalloc(SIZE_LINK_KEY);
				memcpy(link_key, record.link_key, SIZE_LINK_KEY);
				prim->key = VmGetHandleFromPointer(link_key);
				success = TRUE;
			}
			else
			{
				prim->key_type = connectionConvertLinkKeyType_t(cl_sm_link_key_none);
				prim->key = NULL;
				success = FALSE;
			}
		}
		else
		{
			prim->key_type = connectionConvertLinkKeyType_t(record.link_key_type);
			link_key = (uint8*) PanicUnlessMalloc(SIZE_LINK_KEY);
			memcpy(link_key, record.link_key, SIZE_LINK_KEY);
			prim->key = VmGetHandleFromPointer(link_key);
			success = TRUE;
		}
	}
	else
	{
		/* Reject the request for a link key */
		prim->key_type = connectionConvertLinkKeyType_t(cl_sm_link_key_none);
		prim->key = NULL;
		success = FALSE;
	}

	/* Send message to the Connection Manager */
	VmSendDmPrim(prim);
	
	return success;
}


/****************************************************************************
NAME
	connectionAuthPutAttribute

FUNCTION
	This function is called to store the specified data in the specified 
	persistent  store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthPutAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata, const uint8* psdata)
{
	uint16 index = find_trusted_device(bd_addr);
	
	if(index)
	{
		index--;
		(void)PsStore(ps_base + index, psdata, size_psdata);
	}
}


/****************************************************************************
NAME
	connectionAuthGetAttribute

FUNCTION
	This function is called to read the specified data from the specified 
	persistent store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetAttribute(Task appTask, uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata)
{
	/* Find device in the TDL */
	uint16 index = find_trusted_device(bd_addr);
	
    if (appTask)
	{
		/* Send a message back to the application task */
		MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_ATTRIBUTE_CFM, size_psdata);
		message->status = fail;
		message->size_psdata = size_psdata;
		message->psdata[0] = 0;
	
		/* If an entry exists in the TDL for the specified device */
		if(index)
		{
			index--;
			if(size_psdata)
			{
				/* Read attribute data */
				if(PsRetrieve(ps_base + index, message->psdata, size_psdata))
				{
					message->status = success;
				}
			}
		}	
		MessageSend(appTask, CL_SM_GET_ATTRIBUTE_CFM, message);
	}
}

/****************************************************************************
NAME
	connectionAuthGetAttribute

FUNCTION
	This function is called to read the specified data from the specified 
	persistent store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetIndexedAttribute(Task appTask, uint16 ps_base, uint16 mru_index, uint16 size_psdata)
{
	TrustedDeviceIndexType		tdi;
    TrustedDeviceRecordType     record;
	
	
	{
		/* Send a message back to the application task */
		MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_INDEXED_ATTRIBUTE_CFM, size_psdata);
		message->status = fail;
		message->size_psdata = size_psdata;
		message->psdata[0] = 0;
		
		/* Read Trusted Device Index from Persistent store */
		if ((mru_index < MAX_TRUSTED_DEVICES) && PsRetrieve(TRUSTED_DEVICE_INDEX, &tdi, sizeof(tdi)))
		{
			/* Read the device record from the Trusted Device List */
			if (tdi.order[mru_index] && PsRetrieve((TRUSTED_DEVICE_LIST_BASE + tdi.order[mru_index] - 1), &record, sizeof(TrustedDeviceRecordType)))
			{
                /* Get Bluetooth address of device */
                message->bd_addr = record.bd_addr;
                
                /* Check if application wants attribute data */
				if (size_psdata != 0)
				{
					/* Read attribute data */
					if(PsRetrieve(ps_base + tdi.order[mru_index]-1, message->psdata, size_psdata))
						message->status = success;
				}
				else
				{
                    /* No attribute data required, so just indicate success */
					message->status = success;
				}
			}
		}

        /* Send confirmation back to application */
		MessageSend(appTask, CL_SM_GET_INDEXED_ATTRIBUTE_CFM, message);
	}
}

/*lint +e525 +e725 +e830 */
