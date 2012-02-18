/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_auth.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_DM_SECURITY_AUTH_H_
#define	CONNECTION_DM_SECURITY_AUTH_H_


/****************************************************************************
NAME
	connectionAuthInit

FUNCTION
	This function is called initailise the managed list of trusted devices

RETURNS
	The number of devices registered with the Bluestack Security Manager
*/
uint16 connectionAuthInit(cl_dm_bt_version version);


/****************************************************************************
NAME
	connectionAuthAddDevice

FUNCTION
	This function is called to add a trusted device to the persistent trusted 
	device list

RETURNS
	TRUE or FALSE to indicate if the device was successfully added to the 
	Trusted device List
*/
uint16 connectionAuthAddDevice(cl_dm_bt_version version, const bdaddr* peer_bd_addr, cl_sm_link_key_type link_key_type, const uint8* link_key, uint16 trusted, uint16 bonded);

/****************************************************************************
NAME
	connectionAuthGetDevice

FUNCTION
	This function is called to add a trusted device to the persistent trusted 
	device list.  A flag indicating if the device was found is returned.
*/
uint16 connectionAuthGetDevice(const bdaddr *peer_bd_addr, cl_sm_link_key_type *link_key_type, uint8 *link_key, uint16 *trusted);

/****************************************************************************
NAME
	connectionAuthDeleteDevice

FUNCTION
	This function is called to remove a trusted device from the persistent 
    trusted device list.  A flag indicating if the device was successfully removed 
    is returned.
*/
uint16 connectionAuthDeleteDevice(const bdaddr* peer_bd_addr);


/****************************************************************************
NAME
	connectionAuthDeleteAllDevices

FUNCTION
	This function is called to remove all trusted devices from the persistent 
    trusted device list.  A flag indicating if all the devices were successfully 
	removed is returned.
*/
uint16 connectionAuthDeleteAllDevice(uint16 ps_base);


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
uint16 connectionAuthSetTrustLevel(cl_dm_bt_version version, const bdaddr* peer_bd_addr, uint16 trusted);


/****************************************************************************
NAME
	connectionAuthSendLinkKey

FUNCTION
	This function is called to send the link key of the specified device to the
	Bluestack Security Manager

RETURNS
	void
*/
void connectionAuthSendLinkKey(const bdaddr* peer_bd_addr);


/****************************************************************************
NAME
	connectionAuthSendSspLinkKey

FUNCTION
	This function is called to send the link key of the specified device to the
	Bluestack Security Manager in mode 4. In addition to the link key this also
	sends the link key type

*/
bool connectionAuthSendSspLinkKey(const bdaddr* peer_bd_addr, bool authenticated);


/****************************************************************************
NAME
	connectionAuthUpdateMru

FUNCTION
	This function is called to keep a track of the most recently used device.
	The TDI index is updated provided that the device specified is currently
	stored in the TDL.

RETURNS
	TRUE if device specified is in the TDL, otherwise FALSE
*/
uint16 connectionAuthUpdateMru(const bdaddr* peer_bd_addr);


/****************************************************************************
NAME
	connectionAuthGetMruBdAddr

FUNCTION
	This function is called get the Bluetooth Device Address of the Most
	Recently Used device

RETURNS
	Pointer to Bluetooth device address
*/
/*bdaddr connectionAuthGetMruBdAddr(void);*/


/****************************************************************************
NAME
	connectionAuthPutAttribute

FUNCTION
	This function is called to store the specified data in the specified 
	persistent  store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthPutAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata, const uint8* psdata);


/****************************************************************************
NAME
	connectionAuthGetAttribute

FUNCTION
	This function is called to read the specified data from the specified 
	persistent store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetAttribute(Task appTask, uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata);


/****************************************************************************
NAME
	connectionAuthGetIndexedAttribute

FUNCTION
	This function is called to read the specified data from the specified 
	persistent store key.  The persistent store key is calulated from
	the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetIndexedAttribute(Task appTask, uint16 ps_base, uint16 mru_index, uint16 size_psdata);

#endif	/* CONNECTION_DM_SECURITY_AUTH_H_ */
