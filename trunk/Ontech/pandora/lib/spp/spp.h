/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    spp.h
    
DESCRIPTION
	Header file for the SPP library.
*/

/*!
@file	spp.h
@brief	Header file for the Serial Port Profile library.
		  
		This library implements the Serial Port Profile Specification. The
		Serial Port Profile Specification (hereafter referred to as SPP) is
		used to set up an emulated serial cable connection using RFCOMM between
		two peer devices. 

		As well as handling the RFCOMM link, the SPP library handles the
		registration of a service record with the SDP database, so that the
		service/applications can be reached via RFCOMM.
		
		The library exposes a functional downstream API and an upstream message
		based API.
*/

#ifndef SPP_H_
#define SPP_H_


#include <connection.h>
#include <message.h>
#include <panic.h>


/************************************ Typedefs *****************************/

struct __SPP;


/*!
	@brief The Serial Port Profile structure.
*/
typedef struct __SPP SPP; 


/*! 
    @brief The SPP task recipe for dynamic tasks.
*/
extern const profile_task_recipe spp_recipe;


/*!
	@brief The type of device the SPP instance will be initialised as.
*/
typedef enum
{
    devNone,    /*!< Device type has not been set. */
	devA,   	/*!< SPP master.*/
	devB	    /*!< SPP slave.*/
} spp_device_type;


/*!
	@brief Possible status codes for the SPP_INIT_CFM message.
*/
typedef enum
{
	/*! Successful initialisation.*/
	spp_init_success,			
	/*! RFCOMM channel registration failed.*/
	spp_init_rfc_chan_fail,		
	/*! Service record registration failed.*/	
	spp_init_sdp_reg_fail,		
	/*! The client did not provide (or provided contradictory) SPP 
	initialisation structure.*/
	spp_init_fail				
} spp_init_status;


/*!
	@brief Possible status codes for the SPP_CONNECT_CFM message 
*/
typedef enum
{
	/*! Connect attempt succeeded. */
	spp_connect_success,						
	/*! Service search failed. */
	spp_connect_sdp_fail,						
	/*! Service level connection establishment failed. */
	spp_connect_slc_failed,
	/*! Profile instance already connected. */
	spp_connect_failed_busy,					
	/*! RFCOMM connection failed to be established. */
    spp_connect_failed,							
	/*! Requested server channel not registered by this profile instance. */
    spp_connect_server_channel_not_registered,	
	/*! Connection attempt timed out. */
    spp_connect_timeout,						
	/*! The remote device rejected the connection. */
    spp_connect_rejected,						
	/*! The remote device terminated the connection. */
    spp_connect_normal_disconnect,				
	/*! Unsuccessful due to an abnormal disconnect while establishing the 
    RFCOMM connection. */
    spp_connect_abnormal_disconnect,
    /*! The connection attempt failed because there is already a connection 
    to that remote device on the requested RFCOMM channel. */
    spp_connect_rfcomm_channel_already_open,
	/*! Connect failed due to invalid frame size request from app. */
	spp_connect_invalid_frame_size
} spp_connect_status;


/*!
	@brief Possible status codes for the SPP_DISCONNECT_IND message.
*/
typedef enum
{
	/*! Successful disconnection. */
	spp_disconnect_success,		
	/*! Disconnection ocurred due to link loss. */
	spp_disconnect_link_loss,	
	/*! Disconnect attempt failed, no service level connection. */
	spp_disconnect_no_slc,		
	/*! Disconnect time out. */
	spp_disconnect_timeout,		
	/*! Unsuccessful for some other reason. */
	spp_disconnect_error		
} spp_disconnect_status;


/*!
	@brief Base message number for this library.
*/
#define SPP_MESSAGE_BASE	0x6f00


/*!
	@brief The messages sent by the SPP library to its client.
*/
typedef enum
{
	SPP_INIT_CFM = SPP_MESSAGE_BASE,
	SPP_CONNECT_CFM,
	SPP_CONNECT_IND,	
    SPP_MESSAGE_MORE_DATA,
    SPP_MESSAGE_MORE_SPACE,
	SPP_DISCONNECT_IND,					
	SPP_MESSAGE_TOP
} SppMessageId;


/*!
	@brief This message indicates the result of the library initialisation.
    
    This message is returned by the SPP library in response to a call to
    SppInit(), SppInitEx() or SppInitLazy().
*/
typedef struct
{
    /*! The SPP instance pointer. This is valid only if the library has been 
        initialised by calling SppInit() i.e. non-lazy mode. If using the 
        SPP library in lazy mode this pointer is invalid and is set to zero.
    */
    SPP             *spp;

	/*! The SPP initialisation status.*/
	spp_init_status	status;		
} SPP_INIT_CFM_T;


/*!
	@brief This message indicates the result of a connection attempt.

    This message is returned by the SPP library in response to a connection 
    attempt initiated by calling SppConnect(), SppConnectEx() or
    SppConnectLazy() to inform the application of the outcome (status) of
    the connect attempt. It is also returned when an incoming connection is 
    accepted by calling SppConnectResponse(), SppConnectResponseEx() or 
	SppResponseLazy(), to inform the application of the outcome of its 
    response.
*/
typedef struct
{
    /*! The SPP instance pointer. If this library is running in lazy mode 
    this pointer is only valid if the status is set to success. */
    SPP				        *spp;

	/*! The SPP connection status.*/
	spp_connect_status      status;

	/*! The sink for the connection if the connection succeeded, otherwise
    set to null. */    
    Sink                    sink;
	
	/*! Negotiated frame size for the connection */
	uint16					frame_size;
} SPP_CONNECT_CFM_T;


/*!
	@brief A message indicating an incoming connection.

    This message is sent by the SPP library to indicate that a remote 
    device is attempting to connect to our device. The client task
    must respond by calling SppConnectResponse(), SppConnectResponseEx()
    or SppConnectResponseLazy() (determined by whether the app wishes to
    specify a max frame size and the value of the lazy parameter in the 
    SPP instance pointer).
*/
typedef struct
{
    /*! The SPP instance pointer. */
    SPP		*spp;

	/*! The Bluetooth address of the device attempting to connect. */	
	bdaddr	addr;				
	
	/*! The frame size requested. */
    uint16	frame_size;		
} SPP_CONNECT_IND_T;


/*!
    @brief This message indicates that a source associated with an SPP profile
    task has received data.
*/
typedef struct {
    /*! The SPP instance pointer. */
    SPP     *spp;

    /*! The source that has more data in it. */
    Source  source;    
} SPP_MESSAGE_MORE_DATA_T;


/*!
    @brief This message indicates that a sink associated with an SPP profile
     task has more space.
*/
typedef struct {
    /*! The SPP instance pointer. */
    SPP     *spp;

    /*! The sink that has more space in it. */
    Sink    sink;    
} SPP_MESSAGE_MORE_SPACE_T;


/*!
	@brief This message indicates the result of a disconnect attempt.

    This message is sent to the client task if it requested a 
    disconnect by calling SppDisconnect() or if the remote device
    disconnected the connection (or link loss ocurred). 
*/
typedef struct
{
    /*! The SPP instance pointer. If the library is running in lazy mode,
    once this message has been received the instance pointer is no 
    longer valid and should not be used. */
    SPP						*spp;

	/*! The disconnect status.*/	
	spp_disconnect_status	status; 
} SPP_DISCONNECT_IND_T;


/*!
	@brief Configuration parameters passed into the Spp profile library in
	order for the library to be initialised.

	Optionally, the client (usually the application) may supply a 
    service record. If the service_record pointer is set to null the default 
    service record provided by the spp library is used. If however the 
    pointer is non null, this client supplied service record is used. The
	spp library will still insert the correct rfcomm channel number into the
	service record but it will perform no other error checking on the record
	provided. For the moment the spp library will take a local copy of the 
    service record so the rfcomm channel can be inserted. If the service record
    supplied by the client is not in const it is the client's responsibility
    to free the data once the spp library has been fully initialised. It is 
    recommended that the config parameters are passed in as a constant block 
    of data.
*/
typedef struct
{
    /*! This field should ONLY be filled in if the task initialising the Spp 
    library is a client library of the Spp lib. Applications should always 
    set this to null. */
    const profile_task_recipe *client_recipe;

    /*! Length of the service record provided by the client. Set to
    zero to use SPP library default record. */
	uint16		size_service_record;
	
    /*! Service record provided by the client. This is used when the
    application does not want to use the default service record provided by
	the spp library. Set to null to use SPP library default record. */
	const uint8		*service_record;
	
	/*! Allows the client to specify whether a service record is registered, 
 	set TRUE for no record, FALSE to create default record / use the record 
	passed in */
	bool			no_service_record;
} spp_init_params;


/*!
    @brief Configuration parameters passed to the SPP library when initiating
    a connection.

    Optionally the client of the spp library may supply specific connect
    parameters to be used instead of the default ones provided by the spp
    library. The client may specify a search pattern for a specific
    service that the spp library should search for. Most clients will not need 
    to do this. In this case the search_pattern pointer should be set to null.
	The client may specify a maximum frame size to use for the connection,
	between 24 and 32767 for a request or 23 and 32767 for a response.  If no
	maximum frame size is specified (i.e. set to 0) the default frame size will
	be used
	NB. The maximum frame size specified in a request will always be decremented
	by the connection lib (hence the higher min value than for a response)
	The client may provide a specific rfcomm channel to connect to, or set the 
	rfcomm_channel_number param to 0 to perform an sdp search and connect to the
	first channel found
*/
typedef struct
{
    /*! The length of the data pointed to by the search_pattern pointer below */
    uint16  size_search_pattern;

    /*! The search pattern to be used instead of the default spp one. */
    const uint8   *search_pattern;
	
	/*! The frame size to use (or null for default) */
	uint16  max_frame_size;
	
	/*! The RFCOMM Channel to connect to (or null to perform sdp search and 
 		connect to first RFCOMM Channel used by SPP services) */
    uint8  rfcomm_channel_number;
} spp_connect_params;


/*!
	@brief Initialise the SPP library.

	@param theAppTask The task initialising the Spp library. This is the task 
    where the SPP_INIT_CFM message will be sent.

	@param type_of_device devA(Master), devB(Slave). THIS FIELD HAS NOW BEEN 
    DEPRECATED AND IS NO LONGER USED.

    @param priority The profile instance priority for link policy voting.
    For a set of profile instances with connections on a given ACL the 
    instance with the highest priority value wins and the low power mode 
    on the ACL is set according to it's power table.

    An SPP_INIT_CFM message will be returned to the client task to indicate
    the outcome of the initialisation.
*/
void SppInit(Task theAppTask, spp_device_type type_of_device, uint16 priority); 


/*!
	@brief Initialise the SPP library to support dynamic tasks (lazy mode).

	@param clientTask The task where the SPP_INIT_CFM message will be sent.

    @param connectionTask The task where incoming connection indications 
    will be sent. For applications using the SPP library this will be the 
    same as the clientTask.

	@param config Configuration parameters for initialising the spp library. 
    Must supply a valid structure, do not set to null. 

    An SPP_INIT_CFM message will be returned to the client task to indicate
    the outcome of the initialisation.
*/
void SppInitLazy(Task clientTask, Task connectionTask, const spp_init_params *config); 


/*!
	@brief Initialise the SPP library.

	@param theAppTask The task initialising the Spp library.

    @param priority The profile instance priority for link policy voting.
    For a set of profile instances with connections on a given ACL the 
    instance with the highest priority value wins and the low power mode 
    on the ACL is set according to it's power table.

    @param config Configuration parameters for initialising the spp library. 
    Must supply a valid structure, do not set to null. 

    This function performs the same functionality as SppInit() but allows the 
    client to supply their own service record for the device.

    An SPP_INIT_CFM message will be returned to the client task to indicate
    the outcome of the initialisation.
*/
void SppInitEx(Task theAppTask, uint16 priority, const spp_init_params *config); 


/*!
	@brief Initiate a SPP connection to a remote device.
	
   	@param spp The SPP instance pointer.

	@param bd_addr Bluetooth address of the device to connect to.

    An SPP_CONNECT_CFM message will be returned to the task that initialised the
    SPP library to indicate the outcome of the connect attempt.
*/
void SppConnect(SPP *spp, const bdaddr *bd_addr);


/*!
	@brief Initiate a SPP connection to a remote device.
	
   	@param spp The SPP instance pointer.

	@param bd_addr Bluetooth address of the device to connect to.

    @param config Service search pattern for the Spp library to use. If 
    set to null Spp library default will be used and the library will 
    search for the SPP service. Also contains maximum frame size parameter, 
	if set to 0 the default frame size will be used.  

    This function has the same functionality as SppConnect() but allows the 
    client to supply their own service search pattern to search for a specific 
    service. The client may aslo specify a max frame size for the connection

    An SPP_CONNECT_CFM message will be returned to the task that initialised the 
    SPP library to indicate the outcome of the connect attempt.
*/
void SppConnectEx(SPP *spp, const bdaddr *bd_addr, const spp_connect_params *config);


/*!
	@brief Initiate a SPP connection to a remote device in dynamic tasks (lazy)
    mode.
	
	@param bd_addr Bluetooth address of the device to connect to.

    @param priority The profile instance priority for link policy voting.
    For a set of profile instances with connections on a given ACL the 
    instance with the highest priority value wins and the low power mode 
    on the ACL is set according to it's power table.

    @param appTask The task that will receive all connection related 
    messages from the Spp library. This is usually the application task
    that initialised the Spp library or the client library task using the 
    Spp library.  

    @param config Service search pattern for the Spp library to use. If 
    set to null Spp library default will be used and the library will 
    search for the SPP service.  Also contains maximum frame size parameter,
	if set to 0 the default frame size will be used.  

    An SPP_CONNECT_CFM message will be returned to the client task to indicate 
    the outcome of the connect attempt.
*/
void SppConnectLazy(const bdaddr *bd_addr, uint16 priority, Task appTask, const spp_connect_params *config);


/*!
	@brief Respond to a notification of an incoming connection.

	@param spp The SPP instance pointer.

	@param response TRUE to accept the connection, FALSE to 
    refuse the connection.

	@param bd_addr Bluetooth address of the remote device (pass down the
    address in the SPP_CONNECT_IND message).

    This function is called in response to an SPP_CONNECT_IND message and
    is used to accept or reject the incoming connection. If the response 
    is set to TRUE a SPP_CONNECT_CFM message will be returned to the client
    to indicate the outcome of the connect attempt.
*/
void SppConnectResponse(SPP *spp, bool response, const bdaddr *bd_addr); 


/*!
	@brief Respond to a notification of an incoming connection with specified
    maximum frame size.

	@param spp The SPP instance pointer.

	@param response TRUE to accept the connection, FALSE to 
    refuse the connection.

	@param bd_addr Bluetooth address of the remote device (pass down the
    address in the SPP_CONNECT_IND message).
	
	@param max_frame_size Maximum frame size permitted by the App (Actual
	frame size will be negotiated with other device and returned via
	SPP_CONNECT_CFM)

    This function is called in response to an SPP_CONNECT_IND message and
    is used to accept or reject the incoming connection. If the response 
    is set to TRUE a SPP_CONNECT_CFM message will be returned to the client
    to indicate the outcome of the connect attempt.
*/
void SppConnectResponseEx(SPP *spp, bool response, const bdaddr *bd_addr, uint16 max_frame_size); 


/*!
	@brief Respond to a notification of an incoming connection using 
    dynamic tasks (lazy) mode.

	@param spp The SPP instance pointer.

	@param response TRUE to accept the connection, FALSE to 
    refuse the connection.

	@param bd_addr Bluetooth address of the remote device (pass down the
    address in the SPP_CONNECT_IND message).

    @param priority The profile instance priority for link policy voting.
    For a set of profile instances with connections on a given ACL the 
    instance with the highest priority value wins and the low power mode 
    on the ACL is set according to it's power table.

	@param max_frame_size Maximum frame size permitted by the App (Actual
	frame size will be negotiated with other device and returned via
	SPP_CONNECT_CFM)
	
    This function is called in response to an SPP_CONNECT_IND message and
    is used to accept or reject the incoming connection. If the response 
    is set to TRUE a SPP_CONNECT_CFM message will be returned to the client
    to indicate the outcome of the connect attempt.
*/
void SppConnectResponseLazy(SPP *spp, bool response, const bdaddr *bd_addr, uint16 priority, uint16 max_frame_size); 


/*!
	@brief Disconnect the connection.

	@param spp The SPP instance pointer.

    An SPP_DISCONNECT_IND message will be sent to the client task to indicate
    the outcome of the disconnect attempt. The SPP instance pointer identifies 
    the connection to be disconnected.
*/
void SppDisconnect(SPP *spp);

#endif /* SPP_H_ */
