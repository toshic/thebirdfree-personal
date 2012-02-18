/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp.h
    
DESCRIPTION 

    Header file for the Audio/Visual Remote Control Profile library.  This
    profile library implements the AVRCP using the services of the AVCTP
    library which is hidden from the Client application by this library
    
    The library exposes a functional downstream API and an upstream message 
    based API.
    
     CLIENT APPLICATION
      |	     |
      |	AVRCP Library
      |	     |
      |      |
     CONNECTION Library
             |
         BLUESTACK
    
*/
/*!
@file	avrcp.h
@brief	Interface to the Audio Video Remote Control Profile library.

        This profile library implements the AVRCP.  This library permits one
        device known as the controller (CT) to send dedicated user actions to
        another device known as the target (TG).
        
        Note: This library does not handle audio streaming, this is implemented
        in the GAVDP library.
    
        The library exposes a functional downstream API and an upstream message
        based API.
*/


#ifndef AVRCP_H_
#define AVRCP_H_


#include <bdaddr_.h>
#include <message.h>
#include <stream.h>
#include <connection.h>


struct __AVRCP;
/*!
    @brief The Audio Video Remote Control Profile structure.
*/
typedef struct __AVRCP AVRCP;


/*!
    @brief The page data length.
*/
#define PAGE_DATA_LENGTH    (4)


/*! 
    @brief AVRCP responses 
*/
typedef enum
{
    /*! The request is not implemented */
    avctp_response_not_implemented = 0x08,	
    /*! The request has been accepted */
    avctp_response_accepted = 0x09,			
    /*! The request has been rejected */
    avctp_response_rejected = 0x0a,			
    /*! The target is in a state of transition */
    avctp_response_in_transition = 0x0b,	
    /*! A stable response */
    avctp_response_stable = 0x0c,			
    /*! The target devices state has changed */
    avctp_response_changed = 0x0d,			
    /*! The response is an interim response */
    avctp_response_interim = 0x0f,			
    /*! The specified profile is not acceptable */
    avctp_response_bad_profile				
} avrcp_response_type;


/*!
    @brief AVRCP status codes 
*/
typedef enum
{
    /*! Operation was successful. */
    avrcp_success = (0),			
    /*! Operation failed. */
    avrcp_fail,						
    /*! Not enough resources. */
    avrcp_no_resource,				
    /*! Request is not supported in the current state. */
    avrcp_bad_state,				
    /*! Operation timed out before completion. */
    avrcp_timeout,					
    /*! Device specified is not connected. */
    avrcp_device_not_connected,		
    /*! Operation is already in progress. */
    avrcp_busy,						
    /*! Requested operation is not supported. */
    avrcp_unsupported,				
    /*! Sink supplied was invalid. */
    avrcp_invalid_sink,
    /*! Link loss occurred. */
    avrcp_link_loss,
	/*! Link key missing on remote device*/
	avrcp_key_missing
} avrcp_status_code;


/*! 
    @brief Operation ID, used to identify operation. See table 9.21 AV/C Panel
    Subunit spec. 1.1 #
*/
typedef enum
{
    opid_select             = (0x0),
    opid_up,
    opid_down,
    opid_left,
    opid_right,
    opid_right_up,
    opid_right_down,
    opid_left_up,
    opid_left_down,
    opid_root_menu,
    opid_setup_menu,
    opid_contents_menu,
    opid_favourite_menu,
    opid_exit,
    /* 0x0e to 0x1f Reserved */
    opid_0                  = (0x20),
    opid_1,
    opid_2,
    opid_3,
    opid_4,
    opid_5,
    opid_6,
    opid_7,
    opid_8,
    opid_9,
    opid_dot,
    opid_enter,
    opid_clear,
    /* 0x2d - 0x2f Reserved */
    opid_channel_up         = (0x30),
    opid_channel_down,
    opid_sound_select,
    opid_input_select,
    opid_display_information,
    opid_help,
    opid_page_up,
    opid_page_down,
    /* 0x39 - 0x3f Reserved */
    opid_power              = (0x40),
    opid_volume_up,
    opid_volume_down,
    opid_mute,
    opid_play,
    opid_stop,
    opid_pause,
    opid_record,
    opid_rewind,
    opid_fast_forward,
    opid_eject,
    opid_forward,
    opid_backward,
    /* 0x4d - 0x4f Reserved */
    opid_angle              = (0x50),
    opid_subpicture,
    /* 0x52 - 0x70 Reserved */
    opid_f1                 = (0x71),
    opid_f2,
    opid_f3,
    opid_f4,
    opid_f5,
    opid_vendor_unique      = (0x7e)
    /* Ox7f Reserved */
} avc_operation_id; 


/*!
    @brief Subunit types 
*/
typedef enum
{
    subunit_monitor         = (0x0),
    subunit_audio,
    subunit_printer,
    subunit_disc,
    subunit_tape_recorder_player,
    subunit_tuner,
    subunit_CA,
    subunit_camera,
    subunit_reserved,
    subunit_panel,
    subunit_bulletin_board,
    subunit_camera_storage,
    /* 0x0c - 0x1b Reserved */
    subunit_vendor_unique   = (0x1c),
    subunit_reserved_for_all,
    subunit_extended,
    subunit_unit
} avc_subunit_type;


/*!
    @brief AVRCP device type

    The avrcp library can be configured to be either a target or a controller
    device.
*/
typedef enum
{
    avrcp_device_none,
    avrcp_target,
    avrcp_controller,
    avrcp_target_and_controller
} avrcp_device_type;


/*!
    @brief AVRCP initialisation parameters

    The initialisation parameters allow the profile instance to be configured
    either as a controller or target. 
*/

typedef struct
{
    avrcp_device_type device_type;
} avrcp_init_params;


/*!
    @brief AV/C protocol - Used to form the targets address
*/
typedef uint16 avc_subunit_id;

/*! 
    @brief Upstream AVRCP library messages base.
*/
#define AVRCP_MESSAGE_BASE  0x6b00

/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
    /* Library initialisation */
    AVRCP_INIT_CFM = AVRCP_MESSAGE_BASE,
            
    /* Connection/ disconnection management */
    AVRCP_CONNECT_CFM,
    AVRCP_CONNECT_IND,
    AVRCP_DISCONNECT_IND,
    
    /* AV/C Specific */
    AVRCP_PASSTHROUGH_CFM,
    AVRCP_PASSTHROUGH_IND,
    AVRCP_UNITINFO_CFM,
    AVRCP_UNITINFO_IND,
    AVRCP_SUBUNITINFO_IND,
    AVRCP_SUBUNITINFO_CFM,
    AVRCP_VENDORDEPENDENT_CFM,
    AVRCP_VENDORDEPENDENT_IND,

    AVRCP_MESSAGE_TOP
} AvrcpMessageId;
#endif

/* Upstream messages from the AVRCP library */

/*!
    @brief This message is generated as a result of a call to AvrcpInit.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance that was initialised.*/
    AVRCP               *avrcp;		
    /*! The current AVRCP status. */	
    avrcp_status_code   status;		
} AVRCP_INIT_CFM_T;

/*!
    @brief This message is generated as a result of a call to AvrcpConnect.
*/
typedef struct
{
    AVRCP               *avrcp; /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code   status; /*!< The current AVRCP status. */
    Sink                sink;   /*!< Connection handle */	
} AVRCP_CONNECT_CFM_T;

/*!
    @brief This message indicates that a remote device wishes to connect.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP		*avrcp;				
    /*! The Bluetooth Device Address of device connecting */
    bdaddr		bd_addr;			
    /*! Connection identifier */	
    uint16		connection_id;		
} AVRCP_CONNECT_IND_T;


/*!
    @brief This message indicates that a remote device wishes to disconnect.
*/
typedef struct
{
    AVRCP               *avrcp; /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code   status; /*!< The current AVRCP status. */
    Sink                sink;   /*!< Connection handle */	
} AVRCP_DISCONNECT_IND_T;

/*!
    @brief This message is generated as a result of a call to AvrcpPassthrough.
*/
typedef struct
{
    AVRCP               *avrcp; /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code   status; /*!< The current AVRCP status. */
    Sink                sink;   /*!< Connection handle */	
} AVRCP_PASSTHROUGH_CFM_T;


/*!
    @brief This message indicates that the CT device has send a Passthrough
    command
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP               *avrcp;						
    /*! The transaction. */
    uint16              transaction;				
    /*! The number of packets. */
    uint16              no_packets;					
    /*! The sink. */
    Sink                sink;						
    /*! AV/C protocol - Used to form the targets address.*/
    avc_subunit_type    subunit_type;				
    /*! The subunit identifier. */
    avc_subunit_id      subunit_id;					
    /*! Identifies the button pressed. */
    avc_operation_id    opid;						
    /*! Indicates the user action of pressing or releasing the button
      identified by opid.  Active low.*/
    bool                state;						
    /*! Length of following operation_data */
    uint16              size_op_data;				
    /*! The op_data field is required for the Vendor Unique operation. For
      other operations, op_data length and data fields should be zero. The
      client should not attempt to free this pointer, the memory will be freed
      when the message is destroyed. If the client needs access to this data
      after the message has been destroyed it is the client's responsibility to
      copy it. */
    uint8               op_data[1];					
} AVRCP_PASSTHROUGH_IND_T;


/*!
    @brief This message is generated as a result of a call to AvrcpUnitInfo.

    Only valid when the result is avrcp_res_success 
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP               *avrcp;				
    /*! The current AVRCP status. */
    avrcp_status_code   status;				
    /*! The sink. */
    Sink                sink;				
    /*! The unit type. */
    avc_subunit_type    unit_type;			
    /*! The unit. */
    uint16              unit;				
    /*! The company identifier. */	
    uint32              company_id;			
} AVRCP_UNITINFO_CFM_T;


/*!
    @brief This message indicates that a remote device is requesting unit
    information.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP               *avrcp;				
    /*! The sink */	
    Sink                sink;				
} AVRCP_UNITINFO_IND_T;


/*!
    @brief This message is generated as a result of a call to AvrcpSubUnitInfo
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP               *avrcp;							
    /*! The current AVRCP status. */
    avrcp_status_code   status;							
    /*! Connection handle */
    Sink                sink;							
    /*! Requested page on the target device. */
    uint8               page;							
    /*! Four entries from the subunit table for the requested page on the
      target device.*/	
    uint8               page_data[PAGE_DATA_LENGTH];
} AVRCP_SUBUNITINFO_CFM_T;


/*!
    @brief This message indicates that a remote device is requesting subunit
    information.
*/
typedef struct
{
    AVRCP	*avrcp;     /*!< Pointer to avrcp profile instance. */
    Sink	sink;       /*!< Connection handle */
    uint8	page;       /*!< Accept/Reject this request. */	
} AVRCP_SUBUNITINFO_IND_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpVendorDependent.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP               *avrcp;	   
    /*! The current AVRCP status. */
    avrcp_status_code   status;    
    /*! Connection handle */
    Sink                sink;      
    /*! Response from remote end if applicable. */ 	
    uint8               response;  
} AVRCP_VENDORDEPENDENT_CFM_T;

/*!
    @brief This message indicates that a remote device is requesting vendor
    dependant information.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP               *avrcp;					
    /*! The transaction. */
    uint16              transaction;			
    /*! The number of packets. */
    uint16              no_packets;				
    /*! The subunit type. */
    avc_subunit_type    subunit_type;			
    /*! The subunit identifier. */
    avc_subunit_id      subunit_id;				
    /*! The company identifier. */
    uint32              company_id;				
    /*! The command type. */
    uint8               command_type;           
    /*! The sink. */
    Sink                sink;					
    /*! The length of op_data. */
    uint16              size_op_data;			
    /*! The operation data. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */	
    uint8               op_data[1];				
} AVRCP_VENDORDEPENDENT_IND_T;



/*!
    @brief Initialise the AVRCP library.

    @param theAppTask The current application task.	
    @param config Specifies how the AVRCP library should be configured.
    This function also takes care of registering a service record for an AVRCP
    device.  No further library functions should be called until the
    AVRCP_INIT_CFM message has been received by the client.
 
    AVRCP_INIT_CFM message will be received by the application. 
*/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config);


/*!
    @brief Initialise the AVRCP library to use dynamic tasks (lazy) mode.

    @param clientTask The task where the AVRCP_INIT_CFM message will be sent.

    @param connectionTask The task where incoming connection indications 
    will be sent. For applications using the Avrcp library this will be the 
    same as the clientTask.

    @param config Configuration parameters for initialising the Avrcp library
    instance. This must not be set to null.

    This function initialises the Avrcp library in dynamic tasks mode. It 
    does everything the AvrcpInit() function does. 
 
    An AVRCP_INIT_CFM message will be returned to the clientTask once 
    initialisation has completed. Avrcp library functions should not be
    called until a successful initialisation has completed. 
*/
void AvrcpInitLazy(Task clientTask, Task connectionTask, const avrcp_init_params *config);


/*!
    @brief Initiate an AVRCP connection to a remote device.

    @param avrcp The profile instance which will be used.
    @param bd_addr The Bluetooth address of the remote device.

    AVRCP_CONNECT_CFM message will be received by the application. 
*/
void AvrcpConnect(AVRCP *avrcp, const bdaddr *bd_addr);


/*!
    @brief Initiate an AVRCP connection to a remote device.

    @param clientTask The task where all messages relating to this
    connection will be sent. 

    @param bd_addr The Bluetooth address of the remote device.

    @param config Configuration parameters for initialising the Avrcp library
    instance. @param config Configuration parameters for initialising the Avrcp library
    instance. As the library instance is created dynamically the library client has
    to set the type of device. This must not be set to null.

    AVRCP_CONNECT_CFM message will be received by the application. 
*/

void AvrcpConnectLazy(Task clientTask, const bdaddr *bd_addr, const avrcp_init_params *config);


/*!
    @brief Either accept or reject the incoming connection from the 
    remote device.

    @param avrcp The profile instance which will be used.
    @param connection_id Connection identifier
    @param accept Yes(TRUE) or No(FALSE)

    This function is called on receipt of an AVRCP_CONNECT_IND message.  The
    AVRCP_CONNECT_IND message contains the remote devices Bluetooth address.

    AVRCP_CONNECT_IND message will be received by the application. 
*/
void AvrcpConnectResponse(AVRCP *avrcp, uint16 connection_id, bool accept);


/*!
    @brief Response function to accept or reject an incoming AVRCP connection.

    @param avrcp The AVRCP profile instance.
    
    @param connection_id Connection identifier
    
    @param accept Yes(TRUE) or No(FALSE)

    @param config Configuration parameters for initialising the Avrcp library
    instance. As the library instance is created dynamically on an incoming
    connection, it is not fully initialised until the library client has
    set the type of device. This must not be set to null.

    This function is called on receipt of an AVRCP_CONNECT_IND message.  The
    AVRCP_CONNECT_IND message contains the remote devices Bluetooth address.

    AVRCP_CONNECT_IND message will be received by the application. 
*/
void AvrcpConnectResponseLazy(Task clientTask, AVRCP *avrcp, uint16 connection_id, bool accept, const avrcp_init_params *config);


/*!
    @brief Request an AVRCP disconnection.  
    @param avrcp The profile instance which will be used.

    AVRCP_DISCONNECT_IND message will be received by the application.
*/
void AvrcpDisconnect(AVRCP *avrcp);


/*!
    @brief Request that a Pass Through control command is sent to the target on
    the the connection identified by the specified sink.
    
    @param avrcp The profile instance which will be used.

    @param subunit_type The subunit type.

    @param subunit_id AV/C protocol. Used to form the targets address.

    @param state Indicates the user action of pressing or releasing the button
    identified by opid.  Active low.

    @param opid Identifies the button pressed.

    @param size_operation_data Size of operation_data. Should be zero if no
    operation_data passed.

    @param operation_data Required for the Vendor Unique operation. The app
    should use StreamRegionSource as defined in stream.h, and pass the source
    returned as this parameter.

    The Passthrough command is used to convey the proper user operation to 
    the target(transparently to the user).
    
    AVRCP_PASSTHROUGH_CFM message will be received by the application.
*/
/*
                     -----------------------------------------------
                    | MSB |		|	  |		|	  |		|	  |	LSB |
                    |-----------------------------------------------|
    opcode			|		      PASSTHROUGH (0x7C) 				|
                     -----------------------------------------------
    operand(0)		|state|			operation_id					|
                     -----------------------------------------------
    operand(1)		|  		operation data field length				|
                     -----------------------------------------------
    operand(2)		|	 operation data(operation id dependant)		|
        :			|												|
                     -----------------------------------------------

*/
void AvrcpPassthrough(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, bool state, avc_operation_id opid, uint16 size_operation_data, Source operation_data);


/*!
    @brief Verify the data that was sent.
    @param avrcp The profile instance which will be used.
    @param response The AVRCP response.	

    This function is called in response to an AVRCP_PASSTHROUGH_IND message.
*/
void AvrcpPassthroughResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief Request that a UnitInfo control commandis sent to the target on the
    connection identified by the specified sink.
    
    @param avrcp The profile instance which will be used.

    The UnitInfo command is used to obtain information that pertains to the
    AV/C unit as a whole
    
    AVRCP_UNITINFO_CFM message will be received by the application.
    This message contains the unit_type and a unique 24-bit Company ID.
*/
/*		
                     -----------------------------------------------
                    | MSB |		|	  |		|	  |		|	  |	LSB |
                    |-----------------------------------------------|
    opcode			|		       UNITINFO (0x30) 					|
                     -----------------------------------------------
    operand[0]		|	  				0xFF						|
                     -----------------------------------------------
    operand[1]		|  				    0xFF						|
                     -----------------------------------------------
    operand[2]		|  				    0xFF						|
                     -----------------------------------------------
    operand[3]		|  				    0xFF						|
                     -----------------------------------------------
    operand[4]		|  				    0xFF						|
                     -----------------------------------------------
*/
void AvrcpUnitInfo(AVRCP *avrcp);


/*!
    @brief Respond to an AVRCP_UNITINFO_IND message requesting information
    about this device.
    
    @param avrcp The profile instance which will be used.
    @param accept Yes(TRUE) or No(FALSE).
    @param unit_type The unit type.
    @param company_id The company identifier.

*/
void AvrcpUnitInfoResponse(AVRCP *avrcp, bool accept, avc_subunit_type unit_type, uint8 unit, uint32 company_id);


/*!
    @brief Request that a SubUnitInfo control command is sent to the target on
    the connection identified by the specified sink.
    
    @param avrcp The profile instance which will be used.

    @param page Specifies which part of the subunit table is to be returned.
    Each page consists of at most four subunits, and each AV/C unit contains up
    to 32 AV/C subunits

    The UnitInfo command is used to obtain information about the subunit(s) of
    a device. The extension code is not used at present, should always be 0x7.

    AVRCP_SUBUNITINFO_CFM message will be received by the application.
*/
/*
                     -----------------------------------------------
                    | MSB |		|	  |		|	  |		|	  |	LSB |
                    |-----------------------------------------------|
    opcode			|		      SUBUNITINFO (0x31) 				|
                     -----------------------------------------------
    operand[0]		|  0  |       Page      |  0  | Extension code	|
                     -----------------------------------------------
    operand[1]		|  				    0xFF						|
                     -----------------------------------------------
    operand[2]		|  				    0xFF						|
                     -----------------------------------------------
    operand[3]		|  				    0xFF						|
                     -----------------------------------------------
    operand[4]		|  				    0xFF						|
                     -----------------------------------------------

*/
void AvrcpSubUnitInfo(AVRCP *avrcp, uint8 page);


/*!
    @brief Obtain information about the subunit(s) of a device.
    
    @param avrcp The profile instance which will be used.

    @param accept Flag accepting or rejecting request for SubUnitInfo.

    @param page_data Four entries from the subunit table for the requested page
    on the target device.

*/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data);


/*!
    @brief Call to send vendor specific data to the peer entity.
    
    If the data_length is greater than the l2cap mtu then the message becomes
    fragmented.
    
    @param avrcp The profile instance which will be used.

    @param subunit_type The subunit type.

    @param subunit_id  AV/C protocol. Used to form the targets address.

    @param ctype The ctype.

    @param company_id 24-bit unique ID obtained from IEEE RAC.

    @param size_data Length of the data field.

    @param data Required for the Vendor Unique operation. The app should use
    StreamRegionSource as defined in stream.h, and pass the source returned as
    this parameter.
    
    AVRCP_VENDORDEPENDENT_CFM message is returned, indicating the status of the
    connection request.
*/
/*
                     -----------------------------------------------
                    | MSB |	|     |     |     |     |     | LSB |
                    |-----------------------------------------------|
    opcode          |      VENDOR-DEPENDENT (0x00)                  |
                     -----------------------------------------------
    operand(0)	    |MSB                                            |
    operand(1)	    | 	                company_id                  |
    operand(2)	    |  	                                    LSB     |
                    |-----------------------------------------------| 
    operand(3)	    |	                                            |
        :	    |	            vendor_dependent_data           |
    operand(n)	    |	                                            |
                     -----------------------------------------------
*/
void AvrcpVendorDependent(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, uint8 ctype, uint32 company_id, uint16 size_data, Source data);


/*!
    @brief Verify the data that was sent. 

    @param avrcp The profile instance which will be used.
    @param response The AVRCP response.
    @param identifier The identifier.

    This function is called in response to an AVRCP_VENDORDEPENDENT_IND
    message.
    
*/
void AvrcpVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief Retrieve the Sink of the AVRCP connection. This will be 0 if no connection exists. 

    @param avrcp The profile instance which will be used.
*/
Sink AvrcpGetSink(AVRCP *avrcp);


#endif /* AVRCP_H_ */
