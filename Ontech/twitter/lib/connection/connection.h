/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    connection.h
    
DESCRIPTION
	Header file for the Connection library.
*/


/*!
@file	connection.h
@brief	Header file for the Connection library.

		This file provides documentation for the BlueLab3 connection library
		API.
*/

#ifndef	CONNECTION_H_
#define	CONNECTION_H_


#include <bdaddr_.h>
#include <message.h>
#include <sink.h>
#include <stream.h>

/*! \name Connection attempt timeouts.

	Once a connection has been initiated this timeout is set and if the
	connection has not been established by the time this expires, then the
	connect attempt is terminated.
*/
/*! \{ */
/*!
	@brief Default timeout for RFCOMM connection establishment.
*/
#define DEFAULT_RFCOMM_CONNECTION_TIMEOUT   (60)
/*!
	@brief Default timeout for L2CAP connection establishment.
*/
#define	DEFAULT_L2CAP_CONNECTION_TIMEOUT	(60)
/*! \} */

/*! 
	@brief Generic Connection library return status.
*/
typedef enum 
{
	/*! The operation has completed successfully. */
	success,					
	/*! The requested operation has failed. */
	fail						
} connection_lib_status;


/*! 
	@brief HCI command status.

	The status code returned in many primitives from BlueStack is as defined in
	the HCI section of the Bluetooth specification. If a response is being sent
	from the Connection library to a client task this status code is copied
	directly into the message. These error codes correspond to those in the
	Bluetooth specification, and therefore the descriptions are taken from the
	Bluetooth specification documentation.
*/
typedef enum
{
	/*! The command was successful.*/
	hci_success,									
	/*! Indicates that the controller does not understand the HCI Command
	  Packet OpCode that the host sent. The OpCode given might not correspond
	  to any of the OpCodes specified in this document, or any vendor-specific
	  OpCodes, or the command may not have been implemented.*/
    hci_error_illegal_command,						
	/*! Indicates that a command was sent from the host that should identify a
	  connection, but that connection does not exist.*/
    hci_error_no_connection,						
	/*! Indicates to the host that something in the controller has failed in a
	  manner that cannot be described with any other error code.*/
    hci_error_hardware_fail,						
	/*! Indicates that a page timed out because of the Page Timeout
	  configuration parameter. This error code may occur only with the
	  HCI_Remote_Name_Request and HCI_Create_Connection commands. */
    hci_error_page_timeout,							
	/*! Indicates that pairing or authentication failed due to incorrect
	  results in the pairing or authentication procedure. This could be due to
	  an incorrect PIN or Link Key.*/
    hci_error_auth_fail,							
	/*! Used when pairing failed because of a missing PIN.*/
    hci_error_key_missing,							
	/*! Indicates to the host that the controller has run out of memory to
	  store new parameters.*/
    hci_error_memory_full,							
	/*! Indicates that the link supervision timeout has expired for a given
	  connection.*/
    hci_error_conn_timeout,							
	/*! Indicates that an attempt to create another connection failed because
	  the controller is already at its limit of the number of connections it
	  can support.*/
    hci_error_max_nr_of_conns,						
	/*! Indicates that the controller has reached the limit to the number of
	  synchronous connections that can be achieved to a device. */
    hci_error_max_nr_of_sco,						
	/*! Indicates that the controller has reached the limit to the number of
	  asynchronous connections that can be achieved to a device. */
    hci_error_max_nr_of_acl,						
	/*! Indicates that the command requested cannot be executed because the
	  controller is in a state where it cannot process this command at this
	  time. This error shall not be used for command OpCodes where the error
	  code Unknown HCI Command is valid.*/
    hci_error_command_disallowed,					
	/*! Indicates that an incoming connection was rejected due to limited
	  resources. */
    hci_error_rej_by_remote_no_res,					
	/*! Indicates that a connection was rejected due to security requirements
	  not being fulfilled, like authentication or pairing. */
    hci_error_rej_by_remote_sec,					
	/*! Indicates that a connection was rejected because this device does not
	  accept the BD_ADDR. This may be because the device will only accept
	  connections from specific BD_ADDRs.*/
    hci_error_rej_by_remote_pers,					
	/*! Indicates that the Connection Accept Timeout has been exceeded for this
	  connection attempt */
    hci_error_host_timeout,							
	/*! Indicates that a feature or parameter value in an LMP message or HCI
	  Command is not supported.*/
    hci_error_unsupported_feature,					
	/*! Indicates that at least one of the HCI command parameters is invalid.*/
    hci_error_illegal_format,						
	/*! Indicates that the user on the remote device terminated the
	  connection.*/
    hci_error_oetc_user,							
	/*! Indicates that the remote device terminated the connection because of
	  low resources. */
    hci_error_oetc_low_resource,					
	/*! Indicates that the remote device terminated the connection because the
	  device is about to power off.*/
    hci_error_oetc_powering_off,					
	/*! Indicates that the local device terminated the connection.*/
    hci_error_conn_term_local_host,					
	/*! Indicates that the controller is disallowing an authentication or
	  pairing procedure because too little time has elapsed since the last
	  authentication or pairing attempt failed. */
    hci_error_auth_repeated,						
	/*! Indicates that the device does not allow pairing. For example, when a
	  device only allows pairing during a certain time window after some user
	  input allows pairing.*/
    hci_error_pairing_not_allowed,					
	/*! Indicates that the controller has received an unknown LMP opcode. */
    hci_error_unknown_lmp_pdu,						
	/*! Indicates that the remote device does not support the feature
	  associated with the issued command or LMP PDU.*/
    hci_error_unsupported_rem_feature,				
	/*! Indicates that the offset requested in the LMP_SCO_link_req message has
	  been rejected.*/
    hci_error_sco_offset_rejected,					
	/*! Indicates that the interval requested in the LMP_SCO_link_req message
	  has been rejected. */
    hci_error_sco_interval_rejected,				
	/*! Indicates that the air mode requested in the LMP_SCO_link_req message
	  has been rejected. */
    hci_error_sco_air_mode_rejected,				
	/*! Indicates that some LMP message parameters were invalid. This shall be
	  used when :
	  - The PDU length is invalid.
	  - A parameter value must be even.
	  - A parameter is outside of the specified range.
	  - Two or more parameters have inconsistent values. */
    hci_error_invalid_lmp_parameters,				
	/*! Indicates that no other error code specified is appropriate to use. */
    hci_error_unspecified,							
	/*! Indicates that an LMP message contains at least one parameter value
	  that is not supported by the controller at this time. This is normally
	  used after a long negotiation procedure, for example during an
	  LMP_hold_req, LMP_sniff_req and LMP_encryption_key_size_req message
	  exchanges.*/
    hci_error_unsupp_lmp_param,						
	/*! Indicates that a controller will not allow a role change at this
	  time. */
    hci_error_role_change_not_allowed,				
	/*! Indicates that an LMP transaction failed to respond within the LMP
	  response timeout. */
    hci_error_lmp_response_timeout,					
	/*! Indicates that an LMP transaction has collided with the same
	  transaction that is already in progress.*/
    hci_error_lmp_transaction_collision,			
	/*! Indicates that a controller sent an LMP message with an opcode that was
	  not allowed.*/
    hci_error_lmp_pdu_not_allowed,					
	/*! Indicates that the requested encryption mode is not acceptable at this
	  time. */
    hci_error_enc_mode_not_acceptable,				
	/*! Indicates that a link key can not be changed because a fixed unit key
	  is being used.*/
    hci_error_unit_key_used,						
	/*! Indicates that the requested Quality of Service is not supported.*/
    hci_error_qos_not_supported,					
	/*! Indicates that an LMP PDU that includes an instant can not be performed
	  because the instant when this would have occurred has passed.*/
    hci_error_instant_passed,						
	/*! Indicates that it was not possible to pair as a unit key was requested
	  and it is not supported.*/
    hci_error_pair_unit_key_no_support,				
	/*! Indicates that an LMP transaction was started that collides with an
	  ongoing transaction.*/
    hci_error_different_transaction_collision,		
	/*! Insufficient resources.*/
    hci_error_scm_insufficient_resources,			
	/*! Indicates that the specified quality of service parameters could not be
	  accepted at this time, but other parameters may be acceptable.*/
    hci_error_qos_unacceptable_parameter,			
	/*! Indicates that the specified quality of service parameters can not be
	  accepted and QoS negotiation should be terminated.*/
    hci_error_qos_rejected,							
	/*! Indicates that the controller can not perform channel classification
	  because it is not supported.*/
    hci_error_channel_class_no_support,				
	/*! Indicates that the HCI command or LMP message sent is only possible on
	  an encrypted link.*/
    hci_error_insufficient_security,				
	/*! Indicates that a parameter value requested is outside the mandatory
	  range of parameters for the given HCI command or LMP message.*/
    hci_error_param_out_of_mand_range,				
	/*! No longer required.*/
    hci_error_scm_no_longer_reqd,					
	/*! Indicates that a Role Switch is pending. This can be used when an HCI
	  command or LMP message can not be accepted because of a pending role
	  switch. This can also be used to notify a peer device about a pending
	  role switch.*/
    hci_error_role_switch_pending,					
	/*! Parameter change pending. */
    hci_error_scm_param_change_pending,				
	/*! Indicates that the current Synchronous negotiation was terminated with
	  the negotiation state set to Reserved Slot Violation.*/
    hci_error_resvd_slot_violation,					
	/*! Indicates that a role switch was attempted but it failed and the
	  original piconet structure is restored. The switch may have failed
	  because the TDD switch or piconet switch failed.*/
    hci_error_role_switch_failed,					
	/*! Unrecognised error. */
	hci_error_unrecognised							
} hci_status;


/*! 
	@brief The device's role.
*/
typedef enum
{
	hci_role_master,			/*!< The device is Master.*/
	hci_role_slave,				/*!< The device is Slave.*/
	hci_role_dont_care			/*!< Don't request a role switch. */
} hci_role;


/*! 
	@brief The HCI bluetooth version.
*/
typedef enum
{
	hci_version_unrecognised,				/*!< HCI Version Unrecognised*/
	hci_version_1_0,						/*!< HCI Version 1.0.*/
	hci_version_1_1,						/*!< HCI Version 1.1.*/
	hci_version_1_2,						/*!< HCI Version 1.2.*/
	hci_version_2_0,						/*!< HCI Version 2.0.*/
	hci_version_2_1							/*!< HCI Version 2.1.*/
} hci_version;


/*! 
	@brief The host bluetooth version (will return unknown if HCI version is pre 2.1).
*/
typedef enum
{
	bluetooth_unknown,			/*!< Version Unknown.*/
	bluetooth2_0,				/*!< Bluetooth 2.0.*/
	bluetooth2_1				/*!< Bluetooth 2.1.*/
} cl_dm_bt_version;


/*! 
	@brief Synchronous connection link type.
*/
typedef enum
{
    sync_link_unknown,
    sync_link_sco,
    sync_link_esco
} sync_link_type;


/*! 
	@brief Synchronous connection input coding.
*/
typedef enum
{
    sync_input_coding_linear   = (0 << 8),
    sync_input_coding_ulaw     = (1 << 8),
    sync_input_coding_alaw     = (2 << 8),
    sync_input_coding_reserved = (3 << 8)
} sync_input_coding;

/*! 
	@brief Synchronous connection input data format.
*/
typedef enum
{
    sync_input_format_ones_complement = (0 << 6),
    sync_input_format_twos_complement = (1 << 6),
    sync_input_format_sign_magnitude  = (2 << 6),
    sync_input_format_unsigned        = (3 << 6)
} sync_input_format;

/*! 
	@brief Synchronous connection input sample size.
*/
typedef enum
{
    sync_input_size_8_bit  = (0 << 5),
    sync_input_size_16_bit = (1 << 5)
} sync_input_size;

/*! 
	@brief Synchronous connection linear PCM bit position.
*/
typedef enum
{
    sync_pcm_pos_bit_0 = (0 << 2),
    sync_pcm_pos_bit_1 = (1 << 2),
    sync_pcm_pos_bit_2 = (2 << 2),
    sync_pcm_pos_bit_3 = (3 << 2),
    sync_pcm_pos_bit_4 = (4 << 2),
    sync_pcm_pos_bit_5 = (5 << 2),
    sync_pcm_pos_bit_6 = (6 << 2),
    sync_pcm_pos_bit_7 = (7 << 2)
} sync_pcm_pos;

/*! 
	@brief Synchronous connection air coding format.
*/
typedef enum
{
    sync_air_coding_cvsd        = (0 << 0),
    sync_air_coding_ulaw        = (1 << 0),
    sync_air_coding_alaw        = (2 << 0),
    sync_air_coding_transparent = (3 << 0)
} sync_air_coding;


/*! 
	@brief Synchronous packet types.
*/
typedef enum
{
    sync_hv1           = 0x0001,
    sync_hv2           = 0x0002,
    sync_hv3           = 0x0004,
    sync_all_sco       = sync_hv3 | sync_hv2 | sync_hv1,
    sync_ev3           = 0x0008,
    sync_ev4           = 0x0010,
    sync_ev5           = 0x0020,
    sync_all_esco      = sync_ev5 | sync_ev4 | sync_ev3,
    sync_2ev3          = 0x0040,
    sync_3ev3          = 0x0080,
    sync_2ev5          = 0x0100,
    sync_3ev5          = 0x0200,
    sync_all_edr_esco  = sync_3ev5 | sync_2ev5 | sync_3ev3 | sync_2ev3,
    sync_all_pkt_types = sync_all_edr_esco | sync_all_esco | sync_all_sco
} sync_pkt_type;

/*! 
	@brief Synchronous retransmission effort
*/
typedef enum
{
    sync_retx_disabled,          /*!< No retransmissions. */
    sync_retx_power_usage,       /*!< At least one retransmission, optimise for power consumption. */
    sync_retx_link_quality,      /*!< At least one retransmission, optimise for link quality. */
    sync_retx_dont_care = 0xFF   /*!< Don't care. */
} sync_retx_effort;


/*! 
	@brief Recipe for creating a profile instance task for dynamic task creation.
*/
typedef struct profile_task_recipe
{
    /*! The size of the profile task instance */
    uint16                      size_instance;

    /*! The message the connection library will send when the task is created. 
        This messsage always contains the task that has been registered to handle 
        incoming connection requests. In most cases this will be the application task. */
    MessageId                   msg_id;

    /*! The task data for the profile task instance i.e. the profile handler function.
        This MUST be the main profile handler function for the profile library
        i.e. the function that handles all messages from the connection library. */
    TaskData                    data;

	/*! The number of channels that the profile task instance will handle. For the A2DP
		profile this will be set to 2 as each instance must handle a signalling channel
		and a media channel from the same device. */
    uint16						max_channels;

    /*! The task recipe of the parent profile. The Bluetooth specification allows 
        profiles to reuse some or all of the functionality of other profiles, in
        this case the functionality defined in the parent profile is reused
        by the child. */
    const struct profile_task_recipe   *parent_profile;
} profile_task_recipe;


/*! 
	@brief Synchronous connection configuration parameters
*/
typedef struct
{
   uint32            tx_bandwidth;
   uint32            rx_bandwidth;
   uint16            max_latency;
   uint16            voice_settings;
   sync_retx_effort  retx_effort;
   sync_pkt_type     packet_type;
} sync_config_params;


/*! 
	@brief Synchronous connection negotiated link parameters
*/
typedef struct
{
    uint32          rx_bandwidth;   /*!< eSCO receive bandwidth.  Will be zero for SCO. */
    uint32          tx_bandwidth;   /*!< eSCO transmit bandwith.  Will be zero for SCO. */
    sync_link_type  link_type;      /*!< Specifies whether a SCO or eSCO packet type was obtained. */
} sync_link_params;


/*! 
	@brief Page scan repetition modes.
*/
typedef enum
{
	/*! Page scan interval <=1.28 seconds and = Page scan window. */
	page_scan_rep_mode_r0,			
	/*! Page scan interval <=1.28 seconds. */
	page_scan_rep_mode_r1,			
	/*! Page scan interval <=2.56 seconds. */
	page_scan_rep_mode_r2,			
	/*! Unknown page scan mode. */
	page_scan_rep_mode_unknown		
} page_scan_rep_mode;


/*! 
	@brief Page scan mode.
*/
typedef enum
{
	page_scan_mode_mandatory,		/*!< Mandatory page scan mode.*/
	page_scan_mode_optional_1,		/*!< Optional page scan mode.*/
	page_scan_mode_optional_2,		/*!< Optional page scan mode.*/
	page_scan_mode_optional_3,		/*!< Optional page scan mode.*/
	page_scan_mode_unknown			/*!< Unknown page scan mode.*/
} page_scan_mode;


/*! 
	@brief Scan enable values.

	The scan enable parameter controls the page and inquiry scan mode to the
	device.
*/
typedef enum
{
	/*! No scans enabled. */
	hci_scan_enable_off,			
	/*! Enable inquiry scan. Page scan disabled. */
    hci_scan_enable_inq,			
	/*! Enable page scan. Inquiry scan disabled. */
    hci_scan_enable_page,			
	/*! Enable inquiry and page scan. */
    hci_scan_enable_inq_and_page	
} hci_scan_enable;


/*! 
	@brief Inquiry mode.
*/
typedef enum
{
	inquiry_mode_standard,
	inquiry_mode_rssi,
	inquiry_mode_eir
} inquiry_mode;


/*! 
	@brief Inquiry status.

	This is the status returned in a CL_DM_INQUIRE_RESULT message an indicates
	whether the message contains a valid inquiry result or is just a
	notification that the inquiry has completed.
*/
typedef enum 
{
	/*! Notification that the inquiry process has completed. */
    inquiry_status_ready,	
	/*! The CL_DM_INQUIRE_RESULT message contains a valid inquiry result */
    inquiry_status_result	
} inquiry_status;

/*! 
	@brief Specifies that RSSI parameter returned in CL_DM_INQUIRE_RESULT 
	primitive is unknown.
*/
#define CL_RSSI_UNKNOWN ((int16)0x7FFF)

/*! 
	@brief Definition of Authentication (Pairing status).
*/
typedef enum 
{
    auth_status_success,					/*!< Authentication was successful. */
    auth_status_timeout,					/*!< Authentication timed out. */
    auth_status_fail,						/*!< Authentication failed. */
	auth_status_repeat_attempts,			/*!< Authentication failed due to too many repeat attempts. */
	auth_status_pairing_not_allowed,		/*!< Authentication failed as remote device is not allowing pairing. */
	auth_status_unit_key_unsupported,		/*!< Authentication failed as unit keys are not supported. */
	auth_status_simple_pairing_unsupported, /*!< Authentication failed as simple pairing is not supported. */
	auth_status_host_busy_pairing			/*!< Authentication failed as host is already busy pairing. */
} authentication_status;


/*! 
	@brief DM protocol identifier.
*/
typedef enum
{
	protocol_l2cap,				/*!< L2CAP protocol. */
	protocol_rfcomm,			/*!< RFCOMM protocol.*/
	protocol_unknown			/*!< Unknown protocol.*/
} dm_protocol_id;


/*! 
	@brief HCI encryption mode.
*/
typedef enum
{
	/*! Encryption off.*/
	hci_enc_mode_off,					
	/*! Encrypt point to point traffic.*/
	hci_enc_mode_pt_to_pt,				
	/*! Encrypt point to point and broadcast traffic.*/
	hci_enc_mode_pt_to_pt_and_bcast		
} encryption_mode;


/*! 
	@brief DM security mode.

	Descriptions from the Bluetooth specification documentation.
*/
typedef enum
{
	/*! Security off.*/
	sec_mode0_off,				
	/*! When a Bluetooth device is in security mode 1 it shall never initiate
	  any security procedure.*/
    sec_mode1_non_secure,		
	/*! When a Bluetooth device is in security mode 2 it shall not initiate any
	  security procedure before a channel establishment request
	  (L2CAP_ConnectReq) has been received or a channel establishment procedure
	  has been initiated by itself.*/
    sec_mode2_service,			
	/*! When a Bluetooth device is in security mode 3 it shall initiate
	  security procedures before it sends LMP_link_setup_complete.*/
    sec_mode3_link,		
	/*! When a bluetooth device is in mode 4 it enforces it's security requirements
	  before it attempts to access services offered by a remote device and before it
	  grants access to services it offers to remote devices.*/
	sec_mode4_ssp,
	/*! Unknown security mode.*/
	sec_mode_unknown			
} dm_security_mode;


/*! 
	@brief DM security level.
*/
typedef enum
{
	secl_none,					/*!< No security.*/
    secl_in_authorisation,		/*!< In authorisation.*/
    secl_in_authentication,		/*!< In authentication.*/
    secl_in_encryption,			/*!< In encryption.*/
    secl_out_authorisation,		/*!< Out authorisation.*/
    secl_out_authentication,	/*!< Out authentication.*/
    secl_out_encryption,		/*!< Out encryption.*/
    secl_in_connectionless,		/*!< In connectionless.*/
	secl_level_unknown			/*!< Level unknown.*/
} dm_security_level;

/*! 
	@brief DM SSP security levels (as defined in 2.1 spec).
*/
typedef enum
{
	ssp_secl4_l0,					/*!< Mode 4 Level 0 - No Security.*/
	ssp_secl4_l1,					/*!< Mode 4 Level 1 - SSP.*/
	ssp_secl4_l2,					/*!< Mode 4 Level 2 - No MITM.*/
	ssp_secl4_l3,					/*!< Mode 4 Level 3 - MITM.*/
	ssp_secl_level_unknown			/*!< Leave security level unchanged.*/
} dm_ssp_security_level;

/*! 
	@brief Write Auth Enable, determines under which circumstances the device will drop existing ACLs to pair during
	link setup when pairing with mode 3 devices. 
*/
typedef enum
{
	cl_sm_wae_never,				/*!< Never pair during link setup.*/
	cl_sm_wae_acl_none,				/*!< Pair during link setup if there are no existing ACLs.*/	
	cl_sm_wae_acl_owner_none,		/*!< Pair during link setup, first bringing down any existing ownerless ACLs.*/
	cl_sm_wae_acl_owner_app,		/*!< Pair during link setup, first bringing down any existing ownerless and application owned ACLs.*/
	cl_sm_wae_acl_owner_l2cap,		/*!< Pair during link setup, first bringing down any existing ownerless and L2CAP owned ACLs.*/
	cl_sm_wae_always				/*!< Always pair during link setup bringing down any existing ACLs.*/
} cl_sm_wae;

/*!
  @brief Bonding requirements
*/
typedef enum
{
	cl_sm_no_bonding_no_mitm,					/*!< No bonding will take place (ie. link key not stored) no MITM protection .*/
	cl_sm_no_bonding_mitm,						/*!< No bonding will take place (ie. link key not stored) plus MITM protection .*/
	cl_sm_general_bonding_no_mitm,				/*!< General bonding (ie. connection established without dropping ACL after bonding) no MITM protection.*/	
	cl_sm_general_bonding_mitm,					/*!< General bonding (ie. connection established without dropping ACL after bonding) plus MITM protection.*/	
	cl_sm_dedicated_bonding_no_mitm,			/*!< Dedicated bonding (ie. bonding occurs and ACL is dropped) no MITM protection.*/
	cl_sm_dedicated_bonding_mitm,				/*!< Dedicated bonding (ie. bonding occurs and ACL is dropped) plus MITM protection.*/
	cl_sm_authentication_requirements_unknown	/*!< Unrecognised authentication requirements received.*/
} cl_sm_auth_requirements;

/*!
  @brief Link Key Types
*/
typedef enum
{
	cl_sm_link_key_none,			/*!< No link key.*/
    cl_sm_link_key_legacy,			/*!< Legacy link key.*/
	cl_sm_link_key_debug,			/*!< Debug link key.*/
	cl_sm_link_key_unauthenticated,	/*!< Unauthenticated link key.*/
	cl_sm_link_key_authenticated,	/*!< Authenticated link key.*/
	cl_sm_link_key_changed			/*!< Cant tell if authenticated or not.*/
} cl_sm_link_key_type;				

/*!
  @brief IO Capability
*/
typedef enum
{
	cl_sm_io_cap_display_only,		/*! Display Only.*/
	cl_sm_io_cap_display_yes_no,	/*! Display Yes/No.*/
	cl_sm_io_cap_keyboard_only,		/*! Keyboard Only.*/
	cl_sm_io_cap_no_input_no_output,/*! No IO.*/
	cl_sm_reject_request			/*! Use this to reject the IO capability request */
} cl_sm_io_capability;		

/*!
  @brief Keypress Types (used during passkey entry)
*/
typedef enum
{
	cl_sm_passkey_started, 			/*! User started entering passkey.*/
	cl_sm_passkey_digit_entered, 	/*! User entered digit.*/
	cl_sm_passkey_digit_erased,		/*! User erased digit.*/
	cl_sm_passkey_cleared,			/*! User cleared entire passkey.*/
	cl_sm_passkey_complete			/*! User completed passkey entry.*/
} cl_sm_keypress_type;


/*! 
	@brief Base value for SDP close status enums.
*/
#define SDP_CLOSE_STATUS_BASE	(0x02)


/*! 
	@brief SDP close status.
*/
typedef enum 
{
	/*! A search is not open.*/
	sdp_search_not_open = SDP_CLOSE_STATUS_BASE,	
	/*! Task requesting the search be closed did not open it.*/
	sdp_task_did_not_open_search					
} sdp_close_status;


/*! 
	@brief SDP open status.
*/
typedef enum
{
	sdp_open_search_ok,         /*!< The open search attempt was successful.*/
    sdp_open_search_busy,       /*!< The SDP server is busy.*/
    sdp_open_search_failed,		/*!< The open search attempt failed.*/
    sdp_open_search_open,		/*!< A search is already open.*/
    sdp_open_disconnected,		/*!< The connection was lost.*/
	sdp_open_unknown			/*!< Unknown status.*/
} sdp_open_status;


/*! 
	@brief SDP search status.
*/
typedef enum
{
    sdp_response_success,				/*!< The search was successful..*/
    sdp_error_response_pdu,				/*!< Error response PDU.*/
    sdp_no_response_data,				/*!< No response data.*/
    sdp_con_disconnected,				/*!< Connection disconnected.*/
    sdp_connection_error,				/*!< Connection error.*/
    sdp_configure_error,				/*!< Configuration error.*/
    sdp_search_data_error,				/*!< Search data error.*/
    sdp_data_cfm_error,					/*!< Data confirmation error.*/
    sdp_search_busy,					/*!< Search busy.*/
    sdp_response_pdu_header_error,		/*!< Invalid PDU header.*/
    sdp_response_pdu_size_error,		/*!< Invalid PDU size.*/
    sdp_response_timeout_error,			/*!< Search timeout error.*/
    sdp_search_size_to_big,				/*!< Size too big error.*/
    sdp_response_out_of_memory,			/*!< Out of memory error.*/
    sdp_response_terminated,			/*!< Response terminated.*/
	sdp_search_unknown					/*!< Unknown status.*/
} sdp_search_status;


/*! 
	@brief L2CAP connection status.

	This is the status returned in a CL_L2CAP_CONNECT_CFM message indicating
	the outcome of the connection attempt.
*/
typedef enum
{
	/*! L2CAP connection successfully established. */
	l2cap_connect_success,					
	/*! The L2CAP connect attempt failed because either the local or remote end
	  issued a disconnect before the connection was fully established. */
    l2cap_connect_failed,					
	/*! The connection attempt failed because the configuration received from
	  the remote device had the continuation flag set. The Connection library
	  does not currently support this flag. */
	l2cap_connect_failed_more_flag_set,		
	/*! The connection attempt failed due to an internal error in the
	  Connection library. */
	l2cap_connect_failed_internal_error,	
	/*! The connection attempt failed because the remote end rejected the
	  connection request. */
	l2cap_connect_failed_remote_reject,		
	/*! The connection attempt failed because the remote device rejected our
	  configuration request. */
	l2cap_connect_failed_config_rejected,	
	/*! The connection attempt failed because the remote device did not respond
	  to our initial connect request. */
	l2cap_connect_failed_remote_disc,		
	/*! The connection attempt timed out. */
	l2cap_connect_timeout,					
	/*! The connection attempt failed because the client issued a connect
	  request on a PSM that it had not previously registered with the
	  Connection library. */
    l2cap_psm_not_registered,		
    /*! The connect attempt failed because the connection related state
      was not found. This coule be either due to a remote disconnect or
      because the client passed in the wrong parameters. */
    l2cap_connect_instance_not_found,
	/*! The connect attempt failed because the remote device deleted its
	  link key. */
	l2cap_connect_failed_key_missing
} l2cap_connect_status;


/*! 
	@brief L2CAP disconnect status.

	This is the status returned in an CL_L2CAP_DISCONNECT_IND message
	indicating that the L2CAP connection has been disconnected.
*/
typedef enum
{
	/*! The L2CAP connection was disconnected successfully. */
	l2cap_disconnect_successful,	
	/*! The L2CAP disconnect attempt timed out. */
	l2cap_disconnect_timed_out,		
	/*! The L2CAP disconnect attempt returned an error. */
	l2cap_disconnect_error,			
	/*! The L2CAP connection could not be disconnected because a null sink was
	  passed in. */
	l2cap_disconnect_no_connection,
	/*! The L2CAP connection was disconnected due to link loss. */
	l2cap_disconnect_link_loss
} l2cap_disconnect_status;


/*! 
	@brief Power mode.
*/
enum lp_power_mode
{
	/*! Use active power mode.*/
    lp_active,				
	/*! Use sniff power mode.*/
    lp_sniff,				
	/*! Use the passive power mode. Passive mode is a "don't care" setting
	  where the local device will not attempt to alter the power mode. */
	lp_passive = 0xff		
};

typedef uint8 lp_power_mode;


/*! 
	@brief HCI mode.
*/
typedef enum
{
	hci_mode_active,			/*!< HCI active mode.*/
	hci_mode_hold,				/*!< HCI hold mode.*/
	hci_mode_sniff,				/*!< HCI sniff mode.*/
	hci_mode_park,				/*!< HCI park mode.*/
	hci_mode_unrecognised		/*!< Unknown HCI mode.*/
} hci_mode;


/*! 
	@brief Power table.
*/
typedef struct
{
	/*! The power mode.*/
	lp_power_mode   state;			
	/*! The minimum interval.*/
	uint16          min_interval;	
	/*! The maximum interval.*/
	uint16			max_interval;	
	/*! Determines for how many slots the slave shall listen when the slave is
	  not treating this as a scatternet link.*/
	uint16			attempt;		
	/*! Determines for how many additional slots the slave shall listen when
	  the slave is not treating this as a scatternet link.*/
	uint16			timeout;		
	/*! The time.*/
	uint16          time;			
} lp_power_table;


/*! 
	@brief L2CAP Quality of Service Parameters

	The Quality of Service parameters are negotiated before an L2CAP connection
	is established.  A detailed explanation of each of these parameters can be
	found in the L2CAP section of the Bluetooth specification.
*/
typedef struct
{
	/*! Level of the service required e.g. best effort. */
	uint8	service_type;	
	/*! Average data rate with which data is transmitted. */
    uint32	token_rate;     
	/*! Specifies a limit on the "burstiness" with which data may be
	  transmitted. */
    uint32	token_bucket;   
	/*! This limits how fast L2CAP packets can be sent back-to-back. */
    uint32	peak_bw;        
	/*! Maximum acceptable latency of an L2CAP packet. */
    uint32	latency;        
	/*! Difference between the maximum and minimum acceptable delay of an L2CAP
	  packet. */
    uint32	delay_var;      
} qos_flow;


/*! 
	@brief L2CAP connection configuration parameters.
*/
typedef struct
{
	/*! This is the L2CAP MTU the local device will advertise during the L2CAP
	  connection configuration. The L2CAP MTU advertised by a device is defined
	  in the L2CAP section of the Bluetooth specification.*/
	uint16		mtu_local;					
	/*! If the remote device advertises an MTU smaller than this value then the
	  Connection library will not proceed with the L2CAP connection to that
	  device. This field should be used when a profile mandates a specific
	  minimum MTU since a device attempting to use a smaller MTU would be
	  breaking that particular profile specification. Unless such an MTU value
	  is specified in the profile being implemenmted it is recommended that
	  this value is set to MINIMUM_MTU (as definedin l2cap_prim.h) in order to
	  ensure better interoperability i.e. we allow the remote device to
	  advertise any size of MTU as long as it is greater than the minimum MTU
	  allowed in the L2CAP specification.*/
	uint16		mtu_remote_min;				
	/*! Informs the remote end of the local device's flush timeout settings.*/
	uint16		flush_timeout;				
	/*! If set to TRUE we will connect to a device regardless of its flush
	  timeout setting. If set to FALSE we will not proceed with the L2CAP
	  connection unless the remote device is using the default flush
	  timeout. Once a connection is established the client will be informed of
	  the remote device's flush timeout.*/
	bool		accept_non_default_flush;	
	/*! The local device's Quality of Service settings. */
	qos_flow	qos;						
	/*! If set to TRUE the local device accepts the Quality of Service settings
	  of the remote device whatever they are. If FALSE the local device will
	  only proceed with the connection if the remote device has set its Quality
	  of Service settings to "best effort". Once a connection is established
	  the client will be informed of the Quality of Service settings of the
	  remote device.*/
	bool		accept_qos_settings;		
	/*! Connection attempt timeout specified in seconds. If the connection is
	  not established before this timeout expires then the connect attempt is
	  aborted. */
	uint32		timeout;					
} l2cap_config_params;


/*! 
	@brief Port parameters.
*/
typedef struct 
{
    uint8	port_speed;				/*!< The port speed.*/
    uint8	data_bits;				/*!< The data bits.*/
    uint8	stop_bits;				/*!< The stop bits.*/
	uint8	parity;					/*!< Parity.*/
    uint8	parity_type;			/*!< The parity type.*/
	uint8	flow_ctrl_mask;			/*!< The flow control mask.*/
	uint8	xon;					/*!< xon*/
	uint8	xoff;					/*!< xoff*/
	uint16	parameter_mask;			/*!< The parameter mask.*/
} port_par;


/*! 
	@brief RFCOMM configuration parameters.
*/
typedef struct
{
    uint16      max_frame_size;		/*!< The maximum frame size.*/
    uint16      break_signal;		/*!< Break signal.*/
    uint16      modem_status;		/*!< The modem status.*/
    uint32      timeout;			/*!< Time out to use.*/
    bool	    request;			/*!< Request.*/
    port_par	port_params;		/*!< The port parameters to use.*/
} rfcomm_config_params;


/*! 
	@brief RFCOMM connection status.
*/
typedef enum
{
	/*! The connection was successful.*/
	rfcomm_connect_success,						
	/*! The connection failed.*/
    rfcomm_connect_failed,						
	/*! The server channel was not registered.*/
    rfcomm_server_channel_not_registered,		
	/*! The connection timed out.*/
    rfcomm_connect_timeout,						
	/*! The connection was rejected.*/
    rfcomm_connect_rejected,					
	/*! The connection was disconnected normally.*/
    rfcomm_normal_disconnect,					
	/*! The connection was disconnected abnormally.*/
    rfcomm_abnormal_disconnect,
    /*! The client has attempted to connect to a server channel that has already been connected to. */
    rfcomm_connect_channel_already_open, 
	/*! The connection failed because the remote device has deleted it's link key. */
	rfcomm_connect_key_missing
} rfcomm_connect_status;						


/*! 
	@brief RFCOMM disconnection status.
*/
typedef enum
{
	/*! The disconnection was successful.*/
	rfcomm_disconnect_success,									
	/*! A disconnection attempt is pending.*/
    rfcomm_disconnect_connection_pending,						
	/*! The disconnection attempt was rejected due to psm.*/
    rfcomm_disconnect_connection_rej_psm,						
	/*! The disconnection attempt was rejected due to security.*/
    rfcomm_disconnect_connection_rej_security,					
	/*! The disconnection attempt was rejected due to resources.*/
    rfcomm_disconnect_connection_rej_resources,					
	/*! The disconnection attempt was rejected due as the remote device was not
	  ready.*/
    rfcomm_disconnect_connection_rej_not_ready,					
	/*! The disconnection attempt failed.*/
    rfcomm_disconnect_connection_failed,						
	/*! The disconnection attempt timed out.*/
    rfcomm_disconnect_connection_timeout,						
	/*! The disconnection attempt disconnected normally.*/
    rfcomm_disconnect_normal_disconnect,						
	/*! The disconnection attempt disconnected abnormally.*/
    rfcomm_disconnect_abnormal_disconnect,						
	/*! The disconnection attempt configuration was unacceptable.*/
    rfcomm_disconnect_config_unacceptable,						
	/*! The disconnection attempt was rejected due to the configuration.*/
    rfcomm_disconnect_config_rejected,							
	/*! The disconnection attempt had an unknown configuration.*/
    rfcomm_disconnect_config_unknown,							
	/*! The disconnection attempt was rejected locally.*/
    rfcomm_disconnect_config_rejected_locally,					
	/*! The disconnection attempt timed out due to the configuration.*/
    rfcomm_disconnect_config_timeout,							
	/*! The disconnection attempt was refused remotely.*/
    rfcomm_disconnect_remote_refusal,							
	/*! A race condition was detected during the disconnection attempt.*/
    rfcomm_disconnect_race_condition_detected,					
	/*! There were insufficient resources for the disconnection attempt.*/
    rfcomm_disconnect_insufficient_resources,					
	/*! The disconnection attempt could not change the flow control
	  mechanism.*/
    rfcomm_disconnect_cannot_change_flow_control_mechanism,		
	/*! Already exists.*/
    rfcomm_disconnect_dlc_already_exists,						
	/*! Rejected security.*/
    rfcomm_disconnect_dlc_rej_security,							
	/*! A generic refusal occured.*/
    rfcomm_disconnect_generic_refusal,							
	/*! An unexpected primitive was encountered.*/
    rfcomm_disconnect_unexpected_primitive,						
	/*! Invalid server channel.*/
    rfcomm_disconnect_invalid_server_channel,					
	/*! Unknown MUX identifier.*/
    rfcomm_disconnect_unknown_mux_id,							
	/*! Local entity terminated connection.*/
    rfcomm_disconnect_local_entity_terminated_connection,		
	/*! Unknown primitive encountered.*/
    rfcomm_disconnect_unknown_primitive,						
	/*! Maximum payload exceeded.*/
    rfcomm_disconnect_max_payload_exceeded,						
	/*! Inconsistent parameters.*/
    rfcomm_disconnect_inconsistent_parameters,					
	/*! Insufficient credits.*/
    rfcomm_disconnect_insufficient_credits,						
	/*! Credit flow control protocol violation.*/
    rfcomm_disconnect_credit_flow_control_protocol_violation,	
	/*! MUX is already open.*/
    rfcomm_disconnect_mux_already_open,							
	/*! Acknowledgement time out.*/
    rfcomm_disconnect_res_ack_timeout,							
	/*! Disconnection for unknown reason.*/
	rfcomm_disconnect_unknown									
} rfcomm_disconnect_status;


/*! 
	@brief The message filter is a bitmap that controls which status messages the connection library will send to the application.  Each bit controls one or more messages, when the bit is set the connection library will send the message to the application.  See msg_group enum for definitions of the message groups.
*/
typedef uint16 msg_filter;


/*! 
	@brief Message filter groups.  Defines which status messages will be sent to the application.  The msg_filter will have a combination of these values bitwise Or'ed together.
*/
typedef enum
{
	/*! Send CL_DM_ACL_OPEN_IND/CL_DM_ACL_CLOSED_IND messages to application.*/
	msg_group_acl = 0x0001,

	/*! Send CL_DM_MODE_CHANGE_EVENT message to application. */
	msg_group_mode_change = 0x0002,

    /*! Send CL_DM_SYNC_CONNECT_CFM message to the application */
    msg_group_sync_cfm = 0x0004
} msg_group;


/*! 
	@brief The base number for connection library messages.
*/
#define CL_MESSAGE_BASE     0x7000

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
    CL_INIT_CFM = CL_MESSAGE_BASE,
    CL_DM_ROLE_CFM,
    CL_DM_ROLE_IND,
	CL_DM_LINK_SUPERVISION_TIMEOUT_IND,
	CL_DM_SNIFF_SUB_RATING_IND,

    CL_DM_INQUIRE_RESULT,
    CL_DM_REMOTE_NAME_COMPLETE,
    CL_DM_LOCAL_NAME_COMPLETE,
	CL_DM_READ_INQUIRY_TX_CFM,
    CL_DM_CLASS_OF_DEVICE_CFM,

    CL_DM_SYNC_REGISTER_CFM,
    CL_DM_SYNC_UNREGISTER_CFM,
    CL_DM_SYNC_CONNECT_CFM,
    CL_DM_SYNC_CONNECT_IND,
    CL_DM_SYNC_DISCONNECT_IND,
    CL_DM_SYNC_RENEGOTIATE_IND,
    
    CL_DM_LOCAL_BD_ADDR_CFM,
    CL_DM_LINK_QUALITY_CFM,
    CL_DM_RSSI_CFM,
    CL_DM_REMOTE_FEATURES_CFM,
	CL_DM_LOCAL_VERSION_CFM,
	CL_DM_REMOTE_VERSION_CFM,
    CL_DM_CLOCK_OFFSET_CFM,
	CL_DM_READ_BT_VERSION_CFM,
	CL_DM_ACL_OPENED_IND,
	CL_DM_ACL_CLOSED_IND,
	
	CL_SM_READ_LOCAL_OOB_DATA_CFM,
    CL_SM_AUTHENTICATE_CFM,
    CL_SM_SECURITY_LEVEL_CFM,
	CL_SM_SEC_MODE_CONFIG_CFM,
    CL_SM_PIN_CODE_IND,
	CL_SM_IO_CAPABILITY_REQ_IND,
	CL_SM_REMOTE_IO_CAPABILITY_IND,
	CL_SM_USER_CONFIRMATION_REQ_IND,
	CL_SM_USER_PASSKEY_REQ_IND,
	CL_SM_USER_PASSKEY_NOTIFICATION_IND,
	CL_SM_KEYPRESS_NOTIFICATION_IND,
    CL_SM_AUTHORISE_IND,
	CL_SM_ENCRYPT_CFM,
	CL_SM_ENCRYPTION_KEY_REFRESH_IND,
	CL_SM_ENCRYPTION_CHANGE_IND,

	CL_SDP_REGISTER_CFM,
	CL_SDP_UNREGISTER_CFM,
	CL_SDP_OPEN_SEARCH_CFM,
	CL_SDP_CLOSE_SEARCH_CFM,
	CL_SDP_SERVICE_SEARCH_CFM,
	CL_SDP_ATTRIBUTE_SEARCH_CFM,
	CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM,

	CL_L2CAP_REGISTER_CFM,
	CL_L2CAP_UNREGISTER_CFM,
	CL_L2CAP_CONNECT_CFM,
	CL_L2CAP_CONNECT_IND,
	CL_L2CAP_DISCONNECT_IND,

    CL_RFCOMM_REGISTER_CFM,
    CL_RFCOMM_CONNECT_CFM,
    CL_RFCOMM_CONNECT_IND,
    CL_RFCOMM_DISCONNECT_IND,
    CL_RFCOMM_CONTROL_IND,

    CL_DM_LINK_POLICY_IND,
	CL_DM_DUT_CFM,
	CL_DM_MODE_CHANGE_EVENT,
	CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM,
	CL_DM_WRITE_INQUIRY_MODE_CFM,
	CL_DM_READ_INQUIRY_MODE_CFM,
	CL_DM_READ_EIR_DATA_CFM,

	CL_SM_GET_ATTRIBUTE_CFM,
	CL_SM_GET_INDEXED_ATTRIBUTE_CFM,
	CL_SM_ADD_AUTH_DEVICE_CFM,
	CL_SM_GET_AUTH_DEVICE_CFM,
	
	CL_MESSAGE_TOP
} ConnectionMessageId;
#endif /*end of DO_NOT_DOCUMENT*/


/*! 
	@brief A Connection Library entity has completed it's initialisation
	process.
*/
typedef struct
{
    connection_lib_status status;		/*!< The connection library status. */
	cl_dm_bt_version	  version;		/*!< The host bluetooth version. */
} CL_INIT_CFM_T;


/*! 
	@brief Return as a result of an attempt to switch or discover the role.
	
	If status is set to hci_success then role contains the currently set role.
*/
typedef struct
{
    Sink        sink;					/*!< The sink.*/
    hci_status	status;					/*!< The HCI status.*/
    hci_role	role;					/*!< The HCI role.*/
} CL_DM_ROLE_CFM_T;


/*! 
	@brief Unsolicited intication of a role change
	
	This message is sent in response to an unsolicited role change even being 
    received from BlueStack. A message is sent for every sink on the affected ACL. 
    If status is set to hci_success then role contains the currently set role.
*/
typedef struct
{
    bdaddr      bd_addr;				/*!< The Bluetooth address of the remote device.*/
    hci_status	status;					/*!< The HCI status.*/
    hci_role	role;					/*!< The HCI role.*/
} CL_DM_ROLE_IND_T;

/*! 
	@brief Notification that we are sniff subrating with the remote device.
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	bdaddr		bd_addr;				/*!< The Bluetooth address of the remote device.*/
	hci_status	status;					/*!< The HCI Status.*/
	uint16		transmit_latency;		/*!< The Transmit Latency.*/
    uint16      receive_latency;		/*!< The Receive Latency.*/
    uint16      remote_timeout;			/*!< The Remote Sniff Timeout.*/
    uint16      local_timeout;			/*!< The Local Sniff Timeout.*/
} CL_DM_SNIFF_SUB_RATING_IND_T;

/*! 
	@brief Notification that the remote device has changed the link supervision timeout
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	bdaddr		bd_addr;				/*!< The Bluetooth address of the remote device.*/
	uint16		timeout;				/*!< The link supervision timeout.*/
} CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T;


/*!
	@brief Specifies that RSSI parameter returned in CL_DM_INQUIRE_RESULT 
	primitive is unknown.
*/
#define CL_RSSI_UNKNOWN ((int16)0x7FFF)

/*! 
	@brief Message informing the client of an inquiry result or that the
	inquiry has completed.

	For each device discovered during an inquiry the Connection library will
	send a CL_DM_INQUIRE_RESULT message to the task that started the
	inquiry. In this case the status field will be set to result.  When the
	inquiry completes (either because it has timed out or the maximum number of
	devices have been discovered) a CL_DM_INQUIRE_RESULT will be sent to the
	client task with the status set to ready.  If the status is set to ready
	then the remaining fields in the message will not be valid.
	
	This message has been extended to include EIR data for BT2.1
*/
typedef struct
{
	/*! Status indicating whether the message contains a valid inquiry result
	  or is merely an indication that the inquiry has completed. */
    inquiry_status		status;					
	/*! Bluetooth address of the discovered device. */
    bdaddr				bd_addr;				
	/*! Class of device of the discovered device. */
    uint32				dev_class;				
	/*! Clock offset of the discovered device. */
    uint16				clock_offset;			
	/*! Page scan repetition mode of the discovered device. */
	page_scan_rep_mode	page_scan_rep_mode;		
	/*! Page scan mode of the discovered device. */
	page_scan_mode	    page_scan_mode;		
	/*! RSSI of the discovered device.  Set to CL_RSSI_UNKNOWN if value not available. */
	int16				rssi;
	/*! The size of the EIR data recovered. */
	uint8				size_eir_data;
	/*! The EIR data recovered. */
	uint8				eir_data[1];
} CL_DM_INQUIRE_RESULT_T;


/*! 
	@brief This message indicates that a call to ConnectionReadRemoteName has
	completed.

	A connection library client may request to read the remote name of
	device. This message is returned in response to such a request. If the
	remote name was read successfully then the status field should indicate
	this and the message will contain the name read along with its length.
	NOTES: 1. The maximum length of any name read is 31 bytes.  All names are
	NULL terminated.  2. DO NOT Free memory allocated to hold the remote name
	after use.
*/
typedef struct
{
	/*! Indicates the success or failure of the function call.*/           
    hci_status	status;					
	/*! The Bluetooth address of the remote device.*/
	bdaddr		bd_addr;				
	/*! Length of the remote name read.*/
	uint16		size_remote_name;		
	/*! Pointer to the remote name. The client should not attempt to free this
	  pointer, the memory will be freed when the message is destroyed. If the
	  client needs access to this data after the message has been destroyed it
	  is the client's responsibility to copy it. */ 
	uint8		remote_name[1];			
} CL_DM_REMOTE_NAME_COMPLETE_T;

/*! 
	@brief This message indicates that a call to ConnectionReadLocalName has
	completed.

	A connection library client may request to read the local name of
	device. This message is returned in response to such a request. If the
	local name was read successfully then the status field should indicate
	this and the message will contain the name read along with its length.
	NOTES: 1. The maximum length of any name read is 31 bytes.  All names are
	NULL terminated.  2. DO NOT Free the memory allocated to hold the local name 
	after use.
*/
typedef struct
{
	/*! Indicates the success or failure of the function call.*/           
    hci_status	status;					
	/*! Length of the local name read.*/
	uint16		size_local_name;		
	/*! Pointer to the remote name. The client should not attempt to free this
	  pointer, the memory will be freed when the message is destroyed. If the
	  client needs access to this data after the message has been destroyed it
	  is the client's responsibility to copy it. */ 
	uint8		local_name[1];			
} CL_DM_LOCAL_NAME_COMPLETE_T;


/*! 
	@brief Message containing the local inquiry tx power.

	A connection library client may request to read the local inquiry tx
	power. This message is returned in response to such a request. If the
	local inquiry tx power was read successfully then the status field should
	indicate this and the message will contain a valid inquiry tx power.
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! Indicates whether the request to read the local class of device was
	  completed successfully. */
    hci_status	status;     
	/*! The local inquiry tx power if the request was succesful, otherwise
	  invalid. */
    int8		tx_power;	
} CL_DM_READ_INQUIRY_TX_CFM_T;


/*! 
	@brief Message containing the local class of device.

	A connection library client may request to read the local class of
	device. This message is returned in response to such a request. If the
	local class of device was read successfully then the status field should
	indicate this and the message will contain a valid device class.
*/
typedef struct
{
	/*! Indicates whether the request to read the local class of device was
	  completed successfully. */
    hci_status	status;     
	/*! The local class of device if the request was succesful, otherwise
	  invalid. */
    uint32		dev_class;	
} CL_DM_CLASS_OF_DEVICE_CFM_T;


/*! 
	@brief Message received after an attempt is made to register for synchronous connection notifications.
*/
typedef struct
{
    connection_lib_status   status; /*!< Indicates the success or failure of the register attempt. */
} CL_DM_SYNC_REGISTER_CFM_T;


/*! 
	@brief Message received after an attempt is made to de-register from synchronous connection notifications.
*/
typedef struct
{
    connection_lib_status   status; /*!< Indicates the success or failure of the unregister attempt. */
} CL_DM_SYNC_UNREGISTER_CFM_T;


/*! 
	@brief Message received due to a call to ConnectionSyncConnect or ConnectionSyncResponse.
*/
typedef struct
{
    hci_status      status;         /*!< Indicates the success or failure of the connection attempt. */
	sync_link_type  link_type;      /*!< Specifies whether a SCO or eSCO packet type was obtained. */
	Sink            audio_sink;     /*!< The Synchronous connection sink. */
	uint32          rx_bandwidth;   /*!< Receive bandwidth. */
    uint32          tx_bandwidth;   /*!< Transmit bandwith. */
    uint8			sco_handle;		/*!< Link Manager SCO Handle. */
} CL_DM_SYNC_CONNECT_CFM_T;


/*! 
	@brief Message received when a remote device wishes to establish a connection.
*/
typedef struct
{
    bdaddr      bd_addr;    /*!< Bluetooth address of remote device. */
} CL_DM_SYNC_CONNECT_IND_T;


/*! 
	@brief Message received when the either the local or remote device has attempted a disconnect.
*/
typedef struct 
{
    hci_status	status;         /*!< HCI status code indicating success or failure of the disconnection.*/
    hci_status	reason;         /*!< HCI status code indicating reason for a remote initiated disconnection.*/
    Sink		audio_sink;     /*!< The Synchronous connection sink.*/
} CL_DM_SYNC_DISCONNECT_IND_T;


/*! 
	@brief Message received when the either the local or remote device has attempted a renegotiation of an 
	existing synchronous connection's parameters.
*/
typedef struct 
{
    hci_status	status;         /*!< HCI status code indicating success or failure of the renegotiation.*/
    Sink		audio_sink;     /*!< The Synchronous connection sink.*/
} CL_DM_SYNC_RENEGOTIATE_IND_T;


/*! 
	@brief Message received due to a call to ConnectionReadLocalAddr. 

	Contains the local devices Bluetooth address.
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status	status;         
	/*! Bluetooth address of the local device.*/		
    bdaddr		bd_addr;		
} CL_DM_LOCAL_BD_ADDR_CFM_T;	


/*! 
	@brief Message received due to a call to ConnectionGetLinkQuality.
*/
typedef struct
{
    hci_status	status;         /*!< HCI status code.*/
    uint8		link_quality;	/*!< The link quality setting.*/
    Sink		sink;			/*!< The sink.*/
} CL_DM_LINK_QUALITY_CFM_T;


/*! 
	@brief Message received due to a call to ConnectionGetRssi.
*/
typedef struct
{
    hci_status	status;         /*!< HCI status code.*/
    uint8		rssi;			/*!< RSSI value.*/
    Sink		sink;			/*!< The sink.*/
} CL_DM_RSSI_CFM_T;


/*! 
	@brief Message received due to a call to ConnectionReadRemoteSuppFeatures.
*/
typedef struct
{
    hci_status	status;         /*!< HCI status code.*/
    Sink		sink;			/*!< The sink.*/
    uint16      features[4];	/*!< Features.*/
} CL_DM_REMOTE_FEATURES_CFM_T;


 /*! 
 	@brief Message received due to a call to ConnectionReadRemoteVersion.
 */
typedef struct
{
    hci_status		status;             /*!< HCI status code.*/
	hci_version	 	hciVersion;			/*!< HCI version.*/
	uint16			hciRevision;		/*!< HCI revision.*/
    uint8       	lmpVersion;         /*!< The LMP version.*/
    uint16      	manufacturerName;  	/*!< The manufacturer name.*/
    uint16      	lmpSubVersion;      /*!< The LMP sub version.*/
} CL_DM_LOCAL_VERSION_CFM_T;


 /*! 
 	@brief Message received due to a call to ConnectionReadRemoteVersion.
 */
typedef struct
{
    hci_status	status;             /*!< HCI status code.*/
    uint8       lmpVersion;         /*!< The LMP version.*/
    uint16      manufacturerName;  /*!< The manufacturer name.*/
    uint16      lmpSubVersion;      /*!< The LMP sub version.*/
} CL_DM_REMOTE_VERSION_CFM_T;


/*! 
	@brief Message received due to a call to ConnectionReadClockOffset.
*/
typedef struct
{
    hci_status	status;         /*!< HCI status code.*/
    Sink		sink;			/*!< The sink.*/
    uint16      clock_offset;	/*!< The clock offset value.*/
} CL_DM_CLOCK_OFFSET_CFM_T;


/*! 
	@brief Message received due to a call to ConnectionReadBtVersion.
*/
typedef struct
{
    hci_status			status;     /*!< HCI status code.*/
	cl_dm_bt_version	version;	/*!< The Host Bluetooth Version.*/
} CL_DM_READ_BT_VERSION_CFM_T;


/*! 
	@brief Unsolicited informational message alerting the client that an ACL
	has been opened.
*/
typedef struct
{
	/*! The Bluetooth address of the remote device. */
	bdaddr	bd_addr;		
	/*! Flag indicating peer-initiated ACL (TRUE) or locally-initiated
	  (FALSE). */
	bool	incoming;		
	/*! The class of device of the remote device. Valid for incoming
	  connections only. */
	uint32	dev_class;
    /*! HCI status code. If the primitive received from BlueStack contains an HCI status
        code its value will be used to set this field, otherwise the status will be set 
        to hci_error_unrecognised to indicate this field is not valid and should be ignored. */
    hci_status	status;
} CL_DM_ACL_OPENED_IND_T;


/*! 
	@brief Unsolicited informational message alerting the client that an ACL
	has been closed.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device. */
	bdaddr	bd_addr;    
    /*! HCI status code. If the primitive received from BlueStack contains an HCI status
        code its value will be used to set this field, otherwise the status will be set 
        to hci_error_unrecognised to indicate this field is not valid and should be ignored. */
    hci_status	status;
} CL_DM_ACL_CLOSED_IND_T;


/*! 
	@brief Message received due to a call to ConnectionWriteInquiryAccessCode. 
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status	status;         
} CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM_T;	

/*! 
	@brief Message received due to a call to ConnectionWriteInquiryMode. 
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status	status;         
} CL_DM_WRITE_INQUIRY_MODE_CFM_T;	

/*! 
	@brief Message received due to a call to ConnectionReadInquiryMode. 
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status		status;
    inquiry_mode	mode;
} CL_DM_READ_INQUIRY_MODE_CFM_T;	

/*! 
	@brief Message received due to a call to ConnectionReadInquiryMode. 
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status		status;
	bool			fec_required;
	uint8			size_eir_data;
	uint8			eir_data[1];
} CL_DM_READ_EIR_DATA_CFM_T;

/*!
	@brief Size of out of band data.
*/
#define	CL_SIZE_OOB_DATA					(16)

/*! 
	@brief Message received due to a call to ConnectionSmReadLocalOobData.

	This message will only be sent if the local device is BT2.1
	
*/
typedef struct
{
	/*! HCI status code.*/
    hci_status		status;	
	/*! Pointer to the hash C value to send the remote device out of band.*/
	uint8			oob_hash_c[CL_SIZE_OOB_DATA];
	/*! Pointer to the rand R value to send the remote device out of band.*/	
	uint8			oob_rand_r[CL_SIZE_OOB_DATA];
} CL_SM_READ_LOCAL_OOB_DATA_CFM_T;

/*! 
	@brief Message received due to a call to ConnectionSmAuthenticate.

    This message has been extended to include extra information for BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr			        bd_addr;
	/*! The authentication status. */
    authentication_status   status;		
	/*! The type of key generated. */
	cl_sm_link_key_type		key_type;
	/*! The bonding status */
	bool					bonded;
} CL_SM_AUTHENTICATE_CFM_T;


/*! 
	@brief Message received when the remote device is requesting a pin code.
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr	bd_addr;			
} CL_SM_PIN_CODE_IND_T;


/*! 
	@brief Message received when the remote device is requesting IO capability.
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
	
    bdaddr	bd_addr;			
} CL_SM_IO_CAPABILITY_REQ_IND_T;

/*! 
	@brief Message detailing the capabilities of the remote device.
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr							bd_addr;	
	/*! Remote authentication requirements.*/
	cl_sm_auth_requirements 	authentication_requirements;
	/*! The IO capability of the remote device.*/
	cl_sm_io_capability				io_capability;
	/*! If there is any OOB data present on the remote device.*/
	bool							oob_data_present;
} CL_SM_REMOTE_IO_CAPABILITY_IND_T;


/*! 
	@brief Message received when the DM is requesting user confirmation that
	the passkey in the message is the same as displayed on the remote dev.
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr	bd_addr;
	/*! The value for the user to compare.*/
	uint32	numeric_value;
} CL_SM_USER_CONFIRMATION_REQ_IND_T;

/*! 
	@brief Message received when the DM is requesting the user enter the passkey
	being displayed on the remote dev
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr	bd_addr;
} CL_SM_USER_PASSKEY_REQ_IND_T;

/*! 
	@brief Message received when the DM sends up a passkey to display to the user
	(to be entered on the remote device)
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr	bd_addr;
	/*! The passkey to display.*/
	uint32 passkey;
} CL_SM_USER_PASSKEY_NOTIFICATION_IND_T;

/*! 
	@brief Message received when the DM sends up a passkey to display to the user
	(to be entered on the remote device)
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr	bd_addr;
	/*! The passkey to display.*/
	cl_sm_keypress_type type;
} CL_SM_KEYPRESS_NOTIFICATION_IND_T;


/*! 
	@brief This is a request from the Bluestack Security Manager for
	authorisation from the application when an untrusted or unknown device is
	attempting to access a service that requires authorisation in security mode
	2.
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr			bd_addr;			
	/*! The protocol identifier (protocol_l2cap or protocol_rfcomm).*/
    dm_protocol_id	protocol_id;		
	/*!< The channel*/
    uint32			channel;			
	/*! TRUE for incoming connection, FALSE for outgoing connection */
    bool	        incoming;           
} CL_SM_AUTHORISE_IND_T;


/*! 
	@brief This message is received in response to a call to
	ConnectionSmEncrypt.
*/
typedef struct
{
	/*! The status of the encrypt request. */	
	connection_lib_status	status;		
	/*! The sink identifying the connection whose encryption status has been
	  changed if request was successful. */
	Sink					sink;		
	/*! Encryption status of this connection, TRUE if encrypted, FALSE
	  otherwise. */
	bool					encrypted;	
} CL_SM_ENCRYPT_CFM_T;


/*! 
	@brief This message is received when the encryption key has been refreshed
	(either by a call to ConnectionSmEncryptionKeyRefresh or by the remote dev)
	
	This message will only be sent if the local device is BT2.1
*/
typedef struct
{
	/*! The status of the encryption key refresh request. */	
	hci_status				status;		
	/*! The sink identifying the connection whose encryption has been refreshed. */
	Sink					sink;		
} CL_SM_ENCRYPTION_KEY_REFRESH_IND_T;


/*! 
	@brief This message is sent in response to a notification from BlueStack of
	a possible change in the encryption status of a link due to an encryption
	procedure initiated by the remote device.
*/
typedef struct
{
	/*! The sink identifying the connection whose encryption status has been
	  changed. */
	Sink	sink;		
	/*! Encryption status of this connection, TRUE if encrypted, FALSE
	  otherwise. */
	bool	encrypted;	
} CL_SM_ENCRYPTION_CHANGE_IND_T;


/*! 
	@brief Message sent due to a call to ConnectionSmSetSecurityMode.
*/
typedef struct
{
    bool success;				/*!< Success (TRUE) or Failure (FALSE).*/
} CL_SM_SECURITY_LEVEL_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionSmSecModeConfig.
*/
typedef struct
{
    bool 		success;		/*!< TRUE if successfully entered/left debug mode, FALSE otherwise.*/
	cl_sm_wae	wae;			/*!< The new write auth enable setting.*/
	bool		indications;	/*!< The new access indication setting.*/
	bool		debug_keys;		/*!< TRUE if we are in debug mode, FALSE otherwise.*/
} CL_SM_SEC_MODE_CONFIG_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionRegisterServiceRecord.
*/
typedef struct
{
	/*! The connection library status.*/	
	connection_lib_status	status;			
	/*! The service handle.*/
	uint32					service_handle;	
} CL_SDP_REGISTER_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionUnregisterServiceRecord
*/
typedef struct
{
	/*! The connection library status.*/
	connection_lib_status	status;				
	/*! The service handle.*/
	uint32					service_handle;		
} CL_SDP_UNREGISTER_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionSdpOpenSearchRequest.
*/
typedef struct
{
	sdp_open_status	status;			/*!< The SDP open status.*/
} CL_SDP_OPEN_SEARCH_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionSdpCloseSearchRequest.
*/
typedef struct
{
	sdp_close_status status;		/*!< The SDP close status.*/
} CL_SDP_CLOSE_SEARCH_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionSdpServiceSearchRequest.
*/
typedef struct
{
	/*!< The SDP search status.*/
	sdp_search_status	status;				
	/*!< The number of records.*/
	uint16				num_records;		
	/*!< The error code.*/
	uint16				error_code;			
	/*!< The Bluetooth address.*/
	bdaddr				bd_addr;			
	/*!< The size of the record list.*/
    uint16				size_records;		
	/*!< The record list. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
    uint8				records[1];			
} CL_SDP_SERVICE_SEARCH_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionSdpAttributeSearchRequest.
*/
typedef struct
{
	/*! The SDP search status.*/
	sdp_search_status	status;					
	/*! The error code.*/
	uint16				error_code;				
	/*! The Bluetooth address.*/
	bdaddr				bd_addr;				
	/*! The size of the attribute list.*/
	uint16				size_attributes;		
	/*! The attribute list. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
	uint8				attributes[1];			
} CL_SDP_ATTRIBUTE_SEARCH_CFM_T;


/*! 
	@brief Message sent due to a call to
	ConnectionSdpServiceSearchAttributeRequest.
*/
typedef struct
{
	/*! The SDP search status.*/
	sdp_search_status	status;					
	/*! Is more information to come. Yes(TRUE) or No(FALSE).*/
	bool				more_to_come;			
	/*! The error code.*/
    uint16				error_code;				
	/*! The Bluetooth address.*/
	bdaddr				bd_addr;				
	/*! The size of the attribute list returned.*/
	uint16				size_attributes;		
	/*! The attribute list. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
    uint8				attributes[1];			

} CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T;


/*! 
	@brief This message is sent in response to a request to register a PSM with
	the Connection library.
*/
typedef struct
{
	/*! If the PSM was successfully registered the status will be set to
	  success, otherwise it will be set to fail. */
	connection_lib_status	status;		
	/*! The PSM the client task requested to register. */
	uint16					psm;		
} CL_L2CAP_REGISTER_CFM_T;


/*! 
	@brief This message is sent in response to a request to unregister a PSM
	from the Connection library.
*/
typedef struct
{
	/*! If the PSM was successfully unregistered the status will be set to
	  success, otherwise it will be set to fail.*/
	connection_lib_status	status;		
	/*! The PSM the client requested to unregister. */
	uint16					psm;		
} CL_L2CAP_UNREGISTER_CFM_T;


/*! 
	@brief This message returns the result of the L2CAP connect attempt.

	This message is returned to both the initiator and acceptor of the L2CAP
	connection and is used to inform them whether the connection was
	successfully established or not.  Once this message has been received the
	connection can be used to transfer data.
*/
typedef struct
{
	/*! Indicates whether or not the connection was successfully
	  established. */
	l2cap_connect_status	status;					
	/*! The local PSM that was connected to. */
	uint16					psm_local;				
	/*! Sink identifying the connection. The sink is used to send data to the
	  remote device and must be stored by the client task. */
	Sink					sink;					
	/*! Unique identifier for the connection attempt, allows the client to
	  match this cfm message to the response sent tot he connection lib where
	  multiple connections are being established simultaneously to the same
	  device. */
    uint16          		connection_id;	    	
	/*! The MTU advertised by the remote device. */
	uint16					mtu_remote;				
	/*! The flush timeout in use by the remote device. */
	uint16					flush_timeout_remote;	
	/*! The Quality of Service settings of the remote device. */
	qos_flow				qos_remote;				
} CL_L2CAP_CONNECT_CFM_T;


/*! 
	@brief This message informs the client task of an incoming L2CAP
	connection.

	This message is used to notify the client task that a remote device is
	attempting to create an L2CAP connection to this device. The client task
	that registered the PSM being connected to will be sent this message. This
	message should not be ignored, the client must respond even if it wishes to
	reject the connection attempt.
*/
typedef struct
{
	/*! Bluetooth address of the remote device that initiated the
	  connection. */
	bdaddr		bd_addr;			
	/*! Local PSM that the remote device is attempting to connect to. */
	uint16		psm;				
	/*! Unique identifier for the connection attempt, should be copied directly
	  into the response function.*/
	uint16		connection_id;		
} CL_L2CAP_CONNECT_IND_T;


/*! 
	@brief This message informs the client that an L2CAP connection that it
	owns has been disconnected.

	This message is sent to the client to indicate the L2CAP disconnect
	regardless of which side initiated the disconnect.  The sink is included in
	the message so that if the client has multiple L2CAP connections open it
	can determine which one has been closed.  Obviously once this message has
	been received the sink is no longer valid and cannot be used to send data
	to the remote end.
*/
typedef struct
{
	/*! Indicates the L2CAP connection has been disconnected and the status of
	  the disconnect. */
	l2cap_disconnect_status	status;		
	/*! Sink identifying the L2CAP connection that was disconnected. */
	Sink					sink;		
} CL_L2CAP_DISCONNECT_IND_T;


/*! 
	@brief The RFCOMM register confirm message.
*/
typedef struct
{
	/*! The connection library status.*/
	connection_lib_status	status;				
	/*! The server channel.*/
	uint8					server_channel;		
} CL_RFCOMM_REGISTER_CFM_T;


/*! 
	@brief Message sent due to a call to ConnectionRfcommConnectRequest.
*/
typedef struct
{
	/*! The RFCOMM connection status.*/
	rfcomm_connect_status   status;				
	/*! The server channel.*/
	uint8				    server_channel;		
	/*! The frame size.*/
	uint16					frame_size;			
	/*! The sink.*/
    Sink                    sink;				
} CL_RFCOMM_CONNECT_CFM_T;


/*! 
	@brief Received when a remote device wishes open an RFCOMM connection.
*/
typedef struct
{
	/*! The Bluetooth address of the remote device.*/
    bdaddr  bd_addr;					
	/*! The local server channel.*/
	uint8	server_channel;				
	/*! The frame size.*/
    uint16	frame_size;					
} CL_RFCOMM_CONNECT_IND_T;


/*! 
	@brief Received when a remote device wishes to close an RFCOMM connection.
*/
typedef struct
{
	rfcomm_disconnect_status	status;	/*!< The RFCOMM disconnection status.*/
    Sink						sink;	/*!< The sink.*/
} CL_RFCOMM_DISCONNECT_IND_T;


/*! 
	@brief Received when a remote device wishes to send control signals.
*/
typedef struct
{
    Sink	sink;					/*!< The sink.*/
    uint16  break_signal;			/*!< The break signal.*/
    uint16	modem_status;			/*!< The modem status.*/
} CL_RFCOMM_CONTROL_IND_T;


/*! 
	@brief Send to the task that initialised the connection library to confirm
	that Device Under Test (DUT) mode has been entered.
*/
typedef struct
{
	/*! Set to success if DUT mode was entered successfully, fail otherwise. */
	connection_lib_status	status;		
} CL_DM_DUT_CFM_T;

/*! 
	@brief Send to the task 
*/
typedef struct
{
	/*! The Bluetooth address of the remote device. */
	bdaddr	bd_addr;		
	/*! New power mode. */
	lp_power_mode	mode;
	/*! Sniff internal. */
    uint16			interval;
} CL_DM_MODE_CHANGE_EVENT_T;

/*! 
	@brief Send to the task that initialised the connection library to send
	back the requested attribute
*/
typedef struct
{
	/*! Set to success if attribute was successfully read, fail otherwise. */
	connection_lib_status	status;		
	/*! The size of the attribute data. */
	uint16	size_psdata;				
	/*! The attribute data. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
	uint8	psdata[1];					
} CL_SM_GET_ATTRIBUTE_CFM_T;


/*! 
	@brief Send to the task that initialised the connection library to send
	back the requested attribute and Bluetooth address.
*/
typedef struct
{
	/*! Set to success if attribute was successfully read, fail otherwise. */
	connection_lib_status	status;		
	/*! Bluetooth address of requested device. */
	bdaddr bd_addr;
	/*! The size of the attribute data. */
	uint16	size_psdata;				
	/*! The attribute data. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
	uint8	psdata[1];					
} CL_SM_GET_INDEXED_ATTRIBUTE_CFM_T;


/*! 
	@brief Send to the task that initialised the connection library to confirm
	that the Device was added to the trusted device list.
*/
typedef struct
{
	/*! Set to success if device was successfully added. */
	connection_lib_status	status;		

    /*! Bluetooth address of the remote device */
    bdaddr bd_addr;
} CL_SM_ADD_AUTH_DEVICE_CFM_T;

/*! 
	@brief Send to the task that initialised the connection library to confirm
	that the Device was added to the trusted device list.
*/
typedef struct
{
	/*! Set to success if device was found. */
	connection_lib_status	status;		

    /*! Bluetooth address of the remote device */
    bdaddr bd_addr;
	/*! Is the device trusted */
	bool trusted;
	/*! Type of link key */
	cl_sm_link_key_type link_key_type;
	/*! Size of the linkkey */
	uint16 size_link_key;
	/*! The linkkey. The client should not attempt to free this pointer,
	  the memory will be freed when the message is destroyed. If the client
	  needs access to this data after the message has been destroyed it is the
	  client's responsibility to copy it. */
	uint8 link_key[1];
} CL_SM_GET_AUTH_DEVICE_CFM_T;


/****************************************************************************
 Public API definition
 ***************************************************************************/

/*! 
	@brief This function is called to initialise the Multipoint Connection
	Manager.

	@param theAppTask The client task.
*/
void ConnectionInit(Task theAppTask);

/*! 
	@brief This function is called to initialise the Multipoint Connection
	Manager.  It also allows the application to define which status messages
	it would like to receive.

	@param theAppTask The client task.
	@param msgFilter Pointer to message filter.  See documentation for msg_filter and msg_group for more details.
*/
void ConnectionInitEx(Task theAppTask, const msg_filter *msgFilter);

/*! 
	@brief This function will allow a profile library to configure the Security
	Manager to permit SDP browsing without the need for authentication.

	@param enable By setting this parameter to TRUE the security manager
	disables authentication.  Set to FALSE to re-enable security (i.e. return
	to default setting).

	The default for all connections is to enforce authentication before a
	remote device is permitted to browse service records.
*/
void ConnectionSmSetSdpSecurityIn(bool enable);


/*! 
	@brief This function will allow a profile library to configure the Security
	Manager to permit the local device to initiate SDP browsing without locally
	enforcing authentication.

	@param enable Set this parameter to TRUE to disable SDP security and FALSE
	to re-enable it (i.e. to return to default setting).

	@param bd_addr The Bluetooth address of the remote device. The policy will
	be applied to this remote device only.

    The default for all connections is to enforce Authentication before a
    remote device is permitted to browse service records.
*/
void ConnectionSmSetSdpSecurityOut(bool enable, const bdaddr *bd_addr);


/*! 
	@brief This function changes the current role for a particular connection.
	@param theAppTask The client task.
	@param sink The remote sink of the connection.
	@param hci_role The new role.
	
	A CL_DM_ROLE_CFM message which contains the outcome of this function call
	is returned.
*/
void ConnectionSetRole(Task theAppTask, Sink sink, hci_role role);


/*! 
	@brief This function returns the current role for a particular connection.
	@param theAppTask The client task.
	@param sink The remote sink of the connection.
		
	A CL_DM_ROLE_CFM message containing the current role is generated as a
	result of this function call.
*/
void ConnectionGetRole(Task theAppTask, Sink sink);


/*! 
	@brief Configures the inquiry mode.

	@param theAppTask The client task. This is the task that the inquiry
	results will be sent to. In most cases this will be the task initiating the
	inquiry.
	
	@param mode The mode for any subsequent inquiry operation.
	
	A CL_DM_WRITE_INQUIRY_MODE_CFM message which contains the outcome of this 
	function call is returned.
*/	
void ConnectionWriteInquiryMode(Task theAppTask, inquiry_mode mode);


/*! 
	@brief Obtains the current inquiry mode.

	@param theAppTask The client task. This is the task that the inquiry
	results will be sent to. In most cases this will be the task initiating the
	inquiry.
	
	A CL_DM_READ_INQUIRY_MODE_CFM message which contains the current inquiry mode 
	is generated as a result of this function call.
*/	
void ConnectionReadInquiryMode(Task theAppTask);


/*! 
	@brief Initiate an inquiry to discover other Bluetooth devices.

	@param theAppTask The client task. This is the task that the inquiry
	results will be sent to. In most cases this will be the task initiating the
	inquiry.
	
	@param inquiry_lap The LAP from which the inquiry access code should be
	derived.  See the "Bluetooth Assigned Numbers" document in the Bluetooth
	specification. The General/ Unlimited Inquiry Access Code (GIAC) is
	specified in this document as 0x9e8b33 and this is the code most
	applications will use when performing an inquiry.
	
	@param max_responses Maximum number of responses. Once this many devices
	have been discovered the inquiry process will terminate and the client task
	will be notified. Set to zero to indicate unlimited number of devices. In
	this case the inquiry will terminate when the timeout expires. It is not
	recommended that this field is set to zero or a value higher than ten as
	this may result in firmware stability issues.

	@param timeout Maximum amount of time for the inquiry before it is
	terminated. The timeout is specified as defined in the HCI section of the
	Bluetooth specification.  The time the inquiry is performed for is in fact
	timeout * 1.28 seconds. The allowed values of timeout are in the range 0x01
	to 0x30. This corresponds to an inquiry timeout range of 1.28 to 61.44
	seconds.
	
	@param class_of_device Class of device filter. Set to zero to be notified
	of all devices within range regardless of their class of device. If the
	class_of_device is set only devices with those bits set in their class of
	device will be reported to the client task. This filter can be made as
	specific or as general as necessary. For example, to discover all devices
	with major device class set to audio/ video set the class of device field
	to 0x400. This will result in devices with, for example, class of device
	set to 0x200404 and 0x200408 to be reported.  If, however, we want to limit
	our device search even further and only discover devices with their class
	of device set to headset, for example, the class_of_device field should be
	set to 0x200404. Please note that the filter reports devices that have a
	minimum of the specified bits set, however it is possible that a device
	also has other bits set in its class of device field. In that case that
	device will be reported back to the client task.
		
	This function initiates a Bluetooth inquiry to locate Bluetooth devices
	currently in range and discoverable. It allows a class of device filter to
	be specified so only devices with a specific class of device are reported
	back to the client task.

	Inquiry results are sent to the task specified in the theAppTask parameter
	by sending a CL_DM_INQUIRE_RESULT message with its status set to indicate
	that the message contains a valid result. Once the inquiry process
	completes (either because the maximum number of devices has been reached or
	because the inquiry timeout has expired) a CL_DM_INQUIRE_RESULT message
	with status set to ready will be sent to the client task.
*/
void ConnectionInquire(Task theAppTask, uint32 inquiry_lap, uint8 max_responses, uint16 timeout, uint32 class_of_device);


/*! 
	@brief Cancel an active inquiry process.
	@param theAppTask The client task cancelling the inquiry.

	Once the inquiry has been cancelled successfully a CL_DM_INQUIRE_RESULT
	message will be sent to the client task with its status set to ready to
	indicate that the inquiry process has been terminated.
*/
void ConnectionInquireCancel(Task theAppTask);


/*! 
	@brief This function is called to read the remote name of the device with
	the specified Bluetooth device address.
	
	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.

	A CL_DM_REMOTE_NAME_COMPLETE message will be sent to the initiating task on
	completion of the request.
*/
void ConnectionReadRemoteName(Task theAppTask, const bdaddr *bd_addr);

/*! 
	@brief This function is called to read the local name of the device.
	
	@param theAppTask The client task.

	A CL_DM_LOCAL_NAME_COMPLETE message will be sent to the initiating task on
	completion of the request.
*/
void ConnectionReadLocalName(Task theAppTask);


/*! 
	@brief This function is called to write the inquiry tx power.
	
	@param theAppTask The client task.
	
	@param tx_power The tx power to write.

    This is a BT2.1 only feature
*/
void ConnectionWriteInquiryTx(int8 tx_power);


/*! 
	@brief This function is called to read the inquiry tx power.
	
	@param theAppTask The client task.

	A CL_DM_READ_INQUIRY_TX_CFM message will be sent to the initiating task on
	completion of the request.

    This is a BT2.1 only feature
*/
void ConnectionReadInquiryTx(Task theAppTask);


/*! 
	@brief This function is called to set the page scan parameters for the
	device.
	
	@param interval The interval to use.
	@param window The window to use.
*/
void ConnectionWritePagescanActivity(uint16 interval, uint16 window);


/*! 
	@brief This function is called to set the inquiry scan parameters for the
	device.
	
	@param interval The interval to use.
	@param window The window to use.
*/
void ConnectionWriteInquiryscanActivity(uint16 interval, uint16 window);

/*! 
	@brief This function is called to set the inquiry access code the device will
	respond to when in inquiry scan mode.

	@param iac Pointer to table of access codes.
	@param num_iam Number of entries in table.
*/
void ConnectionWriteInquiryAccessCode(Task theAppTask, const uint32 *iac, uint16 num_iac);

/*! 
	@brief This function is called to set the scan mode.
	@param mode Scan mode
	- hci_scan_enable_off          0
	- hci_scan_enable_inq          1
	- hci_scan_enable_page         2
	- hci_scan_enable_inq_and_page 3
*/
void ConnectionWriteScanEnable(hci_scan_enable mode);


/*! 
	@brief This function is called to change the local name of the device.
	
	@param size_local_name The length of the local_name string in bytes.
	
	@param local_name The name to change the local name to. This can be a
	maximum of 32 characters.
*/
void ConnectionChangeLocalName(uint16 size_local_name, const uint8 *local_name);


/*!
	@brief This function is called to change the Extended Inquiry Response
	data of the device.

	@param fec_required 0x01 if FEC encoding is required, 0x00 if not. 

	@param size_eir_data The length of the EIR data in bytes. 

	@param eir_data Pointer to the EIR data. The data can be maximum 240 bytes. Any data over 240-byte will be ignored.

    This is a BT2.1 only feature
*/
void ConnectionWriteEirData(uint8 fec_required, uint8 size_eir_data, const uint8 *eir_data);


/*! 
	@brief This function is called to read the local eir data.
	
	@param theAppTask The client task.
 
	A CL_DM_READ_EIR_DATA_CFM message will be sent to the initiating task on
	completion of the request.

    This is a BT2.1 only feature
*/
void ConnectionReadEirData(Task theAppTask);


/*! 
	@brief This function is called to set the class of device of the local
	device to the supplied value.
	
	@param cod Class of Device.
*/
void ConnectionWriteClassOfDevice(uint32 cod);


/*! 
	@brief Read the local class of device.

	@param theAppTask The client task that the result will be returned to. In
	most cases this will be the task originating the request but this does not
	have to be the case.

	This function is called to read the local class of device. The value read
	is returned in a CL_DM_CLASS_OF_DEVICE_CFM message.
*/
void ConnectionReadClassOfDevice(Task theAppTask);


/*! 
	@brief This function is called to cache the page mode for a specific
	device.  

	@param bd_addr The Bluetooth address of the remote device.  
	@param page_scan_mode The page scan mode.  
	@param page_scan_rep_mode Page scan repetition mode.
*/
void ConnectionWriteCachedPageMode(const bdaddr *addr, page_scan_mode page_scan_mode, page_scan_rep_mode page_scan_rep_mode);


/*! 
	@brief This function is called to cache the clock offset for a specific
	device.

	@param bd_addr The Bluetooth address of the remote device.
	@param clk_offset The clock offset.
*/
void ConnectionWriteCachedClockOffset(const bdaddr *addr, uint16 clk_offset);


/*! 
	@brief Read the local bluetooth version
 
	@param theAppTask The client task that the result will be returned to. In
	most cases this will be the task originating the request but this does not
	have to be the case.
	
    This function results in a CL_DM_READ_BT_VERSION_CFM message
*/
void ConnectionReadBtVersion(Task theAppTask);


/*! 
	@brief This function is called to clear the cache of parameters stored for
	a specific device.
	
	@param bd_addr The Bluetooth address of the remote device.
*/
void ConnectionClearParameterCache(const bdaddr *addr);


/*! 
	@brief Set the flush timeout parameter for the specified connection. 

	@param sink The sink specifying the connection whose flush timeout
	parameter is being set. The flush timeout is set on a per ACL basis so
	calling this function will affect all profile instances using the ACL.

	@param flush_timeout The flush timeout value to be set. This is specified
	as in the HCI section of the Bluetooth specification. A flush timeout value
	of zero means no automatic flush. The allowed range of values for this
	parameter is 0x0001 - 0x07FF where the timeout is calculated in multiples
	of 625us. This gives an allowed range for the flush timeout of 0.625ms -
	1279.375ms.

	The flush timeout allows ACL packets to be flushed automatically by the
	baseband if the timeout expires and the packet has not bee tansmitted (for
	a more comprehensive explanation please refer to the Bluetooth
	specification). By default the flush timeout is set to be infinite which
	means that the retransmissions are carried out until physical link loss
	occurrs.
*/
void ConnectionWriteFlushTimeout(Sink sink, uint16 flush_timeout);


/*! 
	@brief This function is used by tasks to indicate to the connection library that the task 
	may be setting up a Synchronous connection (or that the device it is connected to might initiate one).
	@param theAppTask The client task.
*/
void ConnectionSyncRegister(Task theAppTask);


/*! 
	@brief This function is used by tasks to indicate to the connection library that they do not expect to use Synchronous connections. 
	@param theAppTask The client task.
*/
void ConnectionSyncUnregister(Task theAppTask);


/*! 
	@brief The function initiates the creation of a Synchronous connection to a remote device.
	@param theAppTask The client task.
	@param sink The remote sink to connect to.
	@param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncConnectRequest(Task theAppTask, Sink sink, const sync_config_params *config_params);


/*! 
	@brief This function is used to accept or reject an incoming Synchronous connection request.
	@param theAppTask The client task.
	@param bd_addr The address of the remote device, passed to the client in the CL_DM_SYNC_CONNECT_IND message
	@param response The response to indicate acceptance, or otherwise, of the incoming request.
	@param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncConnectResponse(Task theAppTask, const bdaddr *addr, bool accept, const sync_config_params *config_params);


/*! 
	@brief The function requests disconnection of an existing Synchronous connection.
	@param sink The remote sink.
	@param reason The reason for the disconnection.
*/
void ConnectionSyncDisconnect(Sink sink, hci_status reason);


/*! 
	@brief This function is used to adjust the parameters for an existing Synchronous connection.
	@param theAppTask The client task.
	@param sink The remote sink.
	@param config_params Specifies the connection type and operating parameters.
*/
void ConnectionSyncRenegotiate(Task theAppTask, Sink sink, const sync_config_params *config_params);


/*! 
	@brief This function reads the local device address.

	A CL_DM_LOCAL_BD_ADDR_CFM message containing the address will be sent as a
	result of this function call.
*/
void ConnectionReadLocalAddr(Task theAppTask);


/*! 
	@brief This function is used to obtain the link quality as defined in the
	HCI specification.

	@param theAppTask The client task.
	@param sink The sink to use.

	A CL_DM_LINK_QUALITY_CFM message will be sent as a result of this function
	call.
*/
void ConnectionGetLinkQuality(Task theAppTask, Sink sink);


/*! 
	@brief This function is used to obtain the RSSI value as defined in the 
    HCI specification.
	@param theAppTask The client task.
	@param sink The sink to use.

	A CL_DM_RSSI_CFM message will be sent as a result of this function call. 
*/
void ConnectionGetRssi(Task theAppTask, Sink sink);


/*! 
	@brief This function is used to obtain the supported features of the remote
	device.

	@param theAppTask The client task.
	@param sink The sink to use.

	A CL_DM_REMOTE_FEATURES_CFM message will be sent as a result of this
	function call.
*/
void ConnectionReadRemoteSuppFeatures(Task theAppTask, Sink sink);


/*! 
	@brief This function is used to obtain the version informaton of the 
	local device.

	@param theAppTask The client task.

	A CL_DM_LOCAL_VERSION_CFM message will be sent as a result of this
	function call.
*/
void ConnectionReadLocalVersion(Task theAppTask);


/*! 
	@brief This function is used to obtain the version informaton of the 
	remote device.

	@param theAppTask The client task.
	@param sink The sink to use.

	A CL_DM_REMOTE_VERSION_CFM message will be sent as a result of this
	function call.
*/
void ConnectionReadRemoteVersion(Task theAppTask, Sink sink);


/*! 
	@brief This function is used to obtain the clock offset of the remote
	device.

	@param theAppTask The client task.
	@param sink The sink to use.
	
	A CL_DM_CLOCK_OFFSET_CFM message will be sent as a result of this function
	call.
*/
void ConnectionReadClockOffset(Task theAppTask, Sink sink);


/*! 
	@brief This function is called to request to place the Bluestack Security
	Manager into the specified security mode.

	@param theAppTask The client task.
	@param sec_mode The security mode to use.
	- sec_mode0_off
	- sec_mode1_non_secure
	- sec_mode2_service
	- sec_mode3_link
	- sec_mode4_ssp
	@param enc_mode The encryption mode to use.
	- hci_enc_mode_off
	- hci_enc_mode_pt_to_pt
	- hci_enc_mode_pt_to_pt_and_bcast

    A DM_SM_SECURITY_LEVEL_CFM message will be sent as a result of this
    function call.

    It is recommended that the security mode be changed only when the system is
    not in the process of creating or accepting connections.  This
    recomendation will be enforced by the Security Manager. Once a device has 
	entered sec_mode4_ssp the Security Manager will not allow it to change to
	a legacy security mode.
*/
void ConnectionSmSetSecurityMode(Task theAppTask, dm_security_mode sec_mode, encryption_mode enc_mode);


/*! 
	@brief This function is called to register the security requirements for
	a service when the Bluestack Security Controller is in Security Mode 4. 
	Security requirements registered using this function can be unregistered 
	using ConnectionSmUnRegisterIncomingService

	@param protocol_id The protocol ID
	
	@param channel The channel or PSM to which the security level is to be applied
	
	@param sec_level_default The default security level for bluestack to use
	- ssp_secl4_l0
	- ssp_secl4_l1
	- ssp_secl4_l2
	- ssp_secl4_l3
	
	@param outgoing_ok Set this TRUE to set the level for incoming AND outgoing
		   connections, FALSE for just incoming connections. This parameter does
		   not apply when setting the default level.
		   
	@param authorised Set this TRUE to require authorisation, FALSE otherwise
	
	@param disable_legacy Set this TRUE to block pairing with legacy devices, FALSE 
		   to enable pairing with legacy devices. 
*/
void ConnectionSmSetSecurityLevel(dm_protocol_id protocol_id, uint32 channel, dm_ssp_security_level ssp_sec_level, bool outgoing_ok, bool authorised, bool disable_legacy);


/*! 
	@brief This configures the DM's security mode settings

	@param theAppTask The client task.
	@param write_auth_enable Determines under which conditions we tear down 
	an existing ACL to pair with mode 3 legacy devices during link setup
	@param debug_keys Set TRUE to enable use of debug keys
	@param legacy_auto_pair_missing_key Enables pairing on authentication failure
	with key missing status for legacy devices. This option should only be used to
	maintain interoperability where it is not possible to prompt the user to delete
	the link key manually and retry.
	
	A CL_SM_SEC_MODE_CONFIG_CFM message will be sent as a result of this
	function call.
*/
void ConnectionSmSecModeConfig(Task theAppTask, cl_sm_wae write_auth_enable, bool debug_keys, bool legacy_auto_pair_missing_key);


/*! 
	@brief This function is called to register the security requirements for
	access to a service when the Bluestack Security Controller is in Security
	Mode 2 or 3.

	@param protocol_id The protocol identifier (protocol_l2cap or
	protocol_rfcomm).
	
	@param channel Channel for the protocol defined by the protocol_id that the
	access is being requested on (e.g. RFCOMM server channel number).
	
	@param security_level The security level to use.
    - secl_none
    - secl_in_authorisation
    - secl_in_authentication
    - secl_in_encryption
    - secl_out_authorisation
    - secl_out_authentication
    - secl_out_encryption
    - secl_in_connectionless
	

	The registered security level is applied to all incoming connections on the
	specified 'channel'.
*/
void ConnectionSmRegisterIncomingService(dm_protocol_id protocol_id, uint32 channel, dm_security_level security_level);


/*! 
	@brief This function is called to unregister the security requirements for
	a service previously registered.
	
	@param protocol_id The protocol identifier (protocol_l2cap or
	protocol_rfcomm).
	
	@param channel Channel for the protocol defined by the protocol_id that the
	access is being requested on (e.g. RFCOMM server channel number).
*/
void ConnectionSmUnRegisterIncomingService(dm_protocol_id protocol_id, uint32 channel);


/*! 
	@brief This function is called to register the security requirements
	outgoing connections on the specified protocol and channel on the specified
	remote device.

	@param bd_addr The Bluetooth address of the remote device.

	@param protocol_id The protocol identifier (protocol_l2cap or
	protocol_rfcomm).

	@param channel Channel for the protocol defined by the protocol_id that the
	access is being requested on (e.g. RFCOMM server channel number).

	@param security_level The security level to use.
    - secl_none
    - secl_in_authorisation
    - secl_in_authentication
    - secl_in_encryption
    - secl_out_authorisation
    - secl_out_authentication
    - secl_out_encryption
    - secl_in_connectionless

	This is typically used to control security when connecting to a remote
	RFCOMM server channel.
*/
void ConnectionSmRegisterOutgoingService(const bdaddr* bd_addr, dm_protocol_id protocol_id, uint32 channel, dm_security_level security_level);


/*! 
	@brief This function is called to unregister the security requirements for
	a service that was previously registered.

	@param bd_addr The Bluetooth address of the remote device.

	@param protocol_id The protocol identifier (protocol_l2cap or
	protocol_rfcomm).

	@param channel Channel for the protocol defined by the protocol_id that the
	access is being requested on (e.g. RFCOMM server channel number).
*/
void ConnectionSmUnRegisterOutgoingService(const bdaddr* bd_addr, dm_protocol_id protocol_id, uint32 channel);


/*! 
	@brief This function is called to retreive a new set of Out Of Band parameters to be
	exchanged in an out of band inquiry. Only the most recently retrieved hash C and rand R
	may be used by the remote dev during pairing. Before a new OOB transfer this command must 
	be used	to obtain a new hash C and rand R. 

	@param theAppTask The client task.

	A CL_SM_READ_LOCAL_OOB_DATA_CFM message will be sent as a result of this function
	call.

    This is a BT2.1 only feature
*/
void ConnectionSmReadLocalOobData(Task theAppTask);


/*! 
	@brief This function is called to authenticate a remote device.

	@param theAppTask The client task.

	@param bd_addr The Bluetooth address of the remote device.

	@param timeout Specifies the maximum amount of time in seconds for the
	bonding process. If this time is exceeded then the process is aborted.
	This value must be set to a minimum of 60 seconds to allow 30 seconds
	for the IO capability response and 30 seconds for user input, less than
	this would violate the 2.1 spec. The recommended value for this timeout
	is 90 seconds.

	This normally involves going through the Bonding or Pairing process.  

	A CL_SM_AUTHENTICATE_CFM message will be sent as a result of this function
	call.
*/
void ConnectionSmAuthenticate(Task theAppTask, const bdaddr* bd_addr, uint16 timeout);


/*! 
	@brief This function is called to cancel an authentication request

	@param theAppTask The client task.

	@param bd_addr The Bluetooth address of the remote device.
	
	@param force Force bringing down the ACL to end the bonding process
	even if L2CAP connections are open.
*/
void ConnectionSmCancelAuthenticate(Task theAppTask, bool force);


/*! 
	@brief This function is called to enable /disable encryption on a link
	where security modes 2 or 3 are active.
	
    @param theAppTask The client task.

	@param sink The sink to use.

	@param encrypt Set to TRUE to enable encryption on the link specified by
	sink, or to FALSE to disable it.

	If this function is called with the security mode set to 1, the 
    request will be rejected.

    A CL_SM_ENCRYPT_CFM message will be sent as a result of this function call.
*/
void ConnectionSmEncrypt(Task theAppTask, Sink sink, uint16 encrypt);


/*! 
	@brief This function is called to refresh the encryption key on a link

	@param bd_addr The Bluetooth address of the remote device.

	This is a BT2.1 only feature

    A CL_SM_ENCRYPTION_KEY_REFRESH_IND message will be sent as a result of this function call.
*/
void ConnectionSmEncryptionKeyRefresh(const bdaddr* bd_addr);


/*! 
	@brief This function is called to refresh the encryption key on a link

	@param sink The sink to use.

	This is a BT2.1 only feature

    A CL_SM_ENCRYPTION_KEY_REFRESH_IND message will be sent as a result of this function call.
*/
void ConnectionSmEncryptionKeyRefreshSink(Sink sink);


/*! 
	@brief This function is called to respond to DM_SM_AUTHORISE_IND. 

	@param bd_addr The Bluetooth address of the remote device.  

	@param protocol_id The protocol identifier (protocol_l2cap or
	protocol_rfcomm).

	@param channel Channel for the protocol defined by the protocol_id that the
	access is being requested on (e.g. RFCOMM server channel number).  

	@param incoming TRUE for incoming connection request. FALSE for outgoing
	connection request

    @param authorised TRUE for granted. FALSE for refused.

    This is a request from the Bluestack Security Manager for authorisation 
	from the application when an untrusted or unknown device is attempting 
	to access a service that requires authorisation in security mode 2.
*/
void ConnectionSmAuthoriseResponse(const bdaddr* bd_addr, dm_protocol_id protocol_id, uint32 channel, bool incoming, bool authorised);


/*! 
	@brief This function is called in response to CL_DM_PIN_CODE_IND

	@param bd_addr The Bluetooth address of the remote device.
	@param size_pin_code The length of the pin code.
	@param pin_code The pin code to use.

    If the Security Manager does not have a link key code for the required
    device then it will request a pin code from the application task.  If the
    pin code is not entered then the length will be set to 0.
*/
void ConnectionSmPinCodeResponse(const bdaddr* bd_addr, uint16 size_pin_code, const uint8* pin_code); 


/* 
    @brief This function is called to change the link key for a connection with
	the specified device
	
	@param sink The sink to use.
	
	On return the link key will have been changed (currently the change complete
	message from bluestack is ignored)
*/
void ConnectionSmChangeLinkKey(Sink sink);


/*
	@brief This function is called to drop the ACL connection. Do not use this if
	it can be avoided
	
	@param sink The sink to use.
	
	@param reason The HCI Reason code (as defined in hci.h NOT connection lib.. for now)
*/
void ConnectionDmAclDetach(bdaddr* bd_addr, uint8 reason);


/*! 
	@brief This function is called to remove an authenticated device from the
	paired device list.

	@param bd_addr The Bluetooth address of the remote device.

	On return, the device will have been deleted from the paired device list.
*/
		

void ConnectionSmDeleteAuthDevice(const bdaddr* bd_addr);


/*! 
	@brief This function is called to remove all authenticated devices from the
	paired device list.

	@param ps_base The base user persistent store key

	On return, all devices will have been deleted from the paired device list.  The
	attribute entries will also be deleted started from the key defined by ps_base.
*/
void ConnectionSmDeleteAllAuthDevices(uint16 ps_base);


/*!
	@brief This function is called to add an authenticated device to the paired
	device list.

    @param theAppTask The client task.
	@param peer_bd_addr The Bluetooth address of the remote device.
	@param link_key_type The type of link key
	@param size_link_key Size of link key in octets.
	@param link_key Pointer to link key.
	@param trusted TRUE(trusted) or FALSE(not trusted).
	@param bonded TRUE(bonded) or FALSE(not bonded).
*/
void ConnectionSmAddAuthDevice(Task theAppTask, const bdaddr *peer_bd_addr, cl_sm_link_key_type link_key_type, uint16 size_link_key, const uint8* link_key, uint16 trusted, uint16 bonded);

/*!
	@brief This function is called to get an authenticated device from the paired
	device list.

    @param theAppTask The client task.
	@param peer_bd_addr The Bluetooth address of the remote device.
	
	A CL_SM_GET_AUTH_DEVICE_CFM message will be sent to the client task containing 
	the device information.
*/
void ConnectionSmGetAuthDevice(Task theAppTask, const bdaddr *peer_bd_addr);


/*! 
	@brief This function is called in response to CL_SM_IO_CAPABILITY_REQ_IND

	@param bd_addr The Bluetooth address of the remote device.
	
	@param io_capability The IO capability of the device - set this to reject to send a negative response
	
	@param force_mitm Set this to TRUE to force MITM protection when pairing (This may cause pairing 
    to fail if the remote device cannot support the required io)
				 
    @param bond If this is set TRUE the CL will consider the remote device bonded. If this is set FALSE the
	CL will consider the remote device non bonded, or will reject the IO capability request if dedicated
	bonding is in progress, whether initiated locally or remotely.
	Bonded devices will be added to the PDL and registered with the DM on successful completion of pairing.
	Non Bonded devices will be registered with the DM on successful completion of pairing.
       
	@param oob_data_present Should be set TRUE if the app is sending down out of band data, FALSE otherwise
	@param oob_hash_c The remote device's out of band hash value (should be set NULL if unavailable)
	@param oob_rand_r The remote device's out of band random value (should be set NULL if unavailable)

	This is a BT2.1 only feature
*/
void ConnectionSmIoCapabilityResponse(const bdaddr* bd_addr, cl_sm_io_capability io_capability, bool force_mitm, bool bonding, bool oob_data_present, uint8* oob_hash_c, uint8* oob_rand_r); 


/*! 
	@brief This function is called in response to CL_SM_USER_CONFIRMATION_REQ_IND

	@param bd_addr The Bluetooth address of the remote device.
	@param confirm TRUE for confirmed, FALSE otherwise

    This is a BT2.1 only feature
*/
void ConnectionSmUserConfirmationResponse(const bdaddr* bd_addr, bool confirm);
	

/*! 
	@brief This function is called in response to CL_SM_USER_PASSKEY_REQ_IND

	@param bd_addr The Bluetooth address of the remote device.
	@param cancelled TRUE if user cancelled entry, FALSE otherwise
	@param passkey The passkey the user entered

    This is a BT2.1 only feature
*/
void ConnectionSmUserPasskeyResponse(const bdaddr* bd_addr, bool cancelled, uint32 passkey);


/*! 
	@brief This function is called to notify the remote device
	of a user keypress during passkey entry.

	@param bd_addr The Bluetooth address of the remote device.
	@param type The type of keypress to notify remote device of

    This is a BT2.1 only feature
*/
void ConnectionSmSendKeypressNotificationRequest(const bdaddr* bd_addr, cl_sm_keypress_type type);


/*! 
	@brief This function is called to write attribute data to the persistent
	store assuming there is already an entry for the device exists in the
	paired device list.

	@param ps_base The base user persistent store key
	@param bd_addr The Bluetooth address of the device
	@param size_psdata The length of the data to be written
	@param psdata Pointer to the data

	On return, the data will have been written into the the persistent store
	key at the key defined by ps_base + index of the device in the trusted
	device list
*/
void ConnectionSmPutAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata, const uint8* psdata);


/*! 
	@brief This function is called to write attribute data to the persistent
	store assuming there is already an entry for the device exists in the
	paired device list.

	@param ps_base The base user persistent store key
	@param bd_addr The Bluetooth address of the device
	@param size_psdata The length of the data to be written

	On return, the requested attribute data will be returned
*/
void ConnectionSmGetAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata);


/*! 
	@brief This function is called to retrieve Bluetooth address and attribute
	data from the persistent store for a device stored in the trusted device table,
	assuming there is already an entry for the device exists.

	@param ps_base The base user persistent store key
	@param index Index into trusted device table (0 is most recently paired device)
	@param size_psdata The length of the data to be written

	On return, the requested attribute data and Bluetooth address will be returned
*/
void ConnectionSmGetIndexedAttribute(uint16 ps_base, uint16 index, uint16 size_psdata);


/*! 
	@brief This function is called to mark a device as trusted or untrusted.

	@param bd_addr The Bluetooth address of the remote device.
	@param trusted TRUE(trusted) or FALSE(not trusted).

	If a device attempts to make a connection and it is not trusted then an 
    CL_DM_AUTHORISE_IND message will be sent to request whether this device
    is allowed to connect. This procedure applies even though the two devices
    have exchanged link keys

    If the device is not in the paired device list, no changes are made.
*/
void ConnectionSmSetTrustLevel(const bdaddr* bd_addr, uint16 trusted);


/*!
	@brief This function is called update the trusted device index with the most recently
	used device

	@param bd_addr The Bluetooth address of the most recently used device
*/
void ConnectionSmUpdateMruDevice(const bdaddr *bd_addr);


/*! 
	@brief Register the supplied service record with the local SDP server.
	Will cause a CL_SDP_REGISTER_CFM message to be sent back to the specified
	task.

	@param theAppTask The client task.
	@param size_service_record The number of bytes in the service record.
	@param service_record The service record to register.
*/
void ConnectionRegisterServiceRecord(Task theAppTask, uint16 size_service_record, const uint8 *service_record);


/*! 
	@brief Unregister a record that has previously been registered with the SDP
	server

	@param theAppTask The client task.
	@param service_record_hdl The service record handle.
*/
void ConnectionUnregisterServiceRecord(Task theAppTask, uint32 service_record_hdl);


/*! 
	@brief Configure the SDP server to use a particular size of L2CAP MTU.

	@param mtu The size of L2CAP MTU to use.
*/
void ConnectionSetSdpServerMtu(uint16 mtu);


/*! 
	@brief Configure the SDP client to use a particular size of L2CAP MTU.

	@param mtu The size of L2CAP MTU to use.
*/
void ConnectionSetSdpClientMtu(uint16 mtu);


/*! 
	@brief Open a search session to the specified remote device.

	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.
*/
void ConnectionSdpOpenSearchRequest(Task theAppTask, const bdaddr* bd_addr);


/*! 
	@brief Close the existing SDP search session.

	@param theAppTask The client task.
*/
void ConnectionSdpCloseSearchRequest(Task theAppTask);


/*! 
	@brief Perform a service search on the specified remote device. Will issue
	a CL_SDP_SERVICE_SEARCH_CFM message.

	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.
	@param max_num_recs The maximum number of records.
	@param size_srch_pttrn The size of search_pattern.
	@param search_pattern The pattern to search for.
*/
void ConnectionSdpServiceSearchRequest(Task theAppTask, const bdaddr *bd_addr, uint16 max_num_recs, uint16 size_search_pattern, const uint8 *search_pattern);


/*! 
	@brief Perform an attribute search on the specified remote device

	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.
	@param max_num_recs The maximum number of records.
	@param service_hdl The service handle.
	@param size_attribute_list The size of the attribute_list.
	@param attribute_list The attributes list.
*/
void ConnectionSdpAttributeSearchRequest(Task theAppTask, const bdaddr *bd_addr, uint16 max_num_recs, uint32 service_hdl, uint16 size_attribute_list, const uint8 *attribute_list);


/*! 
	@brief Perform a service and an attribute search on the specified remote
	device.

	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.
	@param max_attributes The maximum number of attributes.
	@param size_srch_pttrn The size of the search_pattern.
	@param search_pattern The pattern to search for.
	@param size_attribute_list The size of attribute_list.
	@param attribute_list The attribute list.

*/
void ConnectionSdpServiceSearchAttributeRequest(Task theAppTask, const bdaddr *addr, uint16 max_attributes, uint16 size_search_pattern, const uint8 *search_pattern, uint16 size_attribute_list, const uint8 *attribute_list);


/*! 
	@brief Terminate the current SDP primative search.

	@param theAppTask The client task.
*/
void ConnectionSdpTerminatePrimitiveRequest(Task theAppTask);


/*! 
	@brief Register an L2CAP PSM with the Connection library.

	@param theAppTask The client task. The CL_L2CAP_REGISTER_CFM message is 
	returned to this task. This is also the task associated with the PSM
	being registered. All incoming connection indications for this PSM will
	be forwarded to this client task.
	
	@param psm The PSM being registered with the Connection library.

	Before a connection can be accepted or initiated on a particular PSM it 
	must firstly be registered with the Connection library. If a connection 
	indication is received for a PSM that does not have a task associated 
	with it this connection will be rejected by the Connection library.

	The client task will receive a CL_L2CAP_REGISTER_CFM message from the 
	Connection library indicating the outcome of this request.
*/
void ConnectionL2capRegisterRequest(Task theAppTask, uint16 psm);


/*! 
	@brief Register an L2CAP PSM with the Connection library.

    @param clientTask The client task where the CL_L2CAP_REGISTER_CFM message
    will be sent.

    @param connectionTask The task where incoming connection indications 
    will be sent.

    @param psm The PSM being registered with the Connection library.
    
	@param recipe Structure passed in from a profile library which can be
    used by the connection library to create a profile library instance for
    handling an incoming L2CAP connection.	

	Before a connection can be accepted or initiated on a particular PSM it 
	must firstly be registered with the Connection library. If a connection 
	indication is received for a PSM that does not have a task associated 
	with it this connection will be rejected by the Connection library.

	The client task will receive a CL_L2CAP_REGISTER_CFM message from the 
	Connection library indicating the outcome of this request. All messages 
    for incoming connections will be sent to the connectionTask.
*/
void ConnectionL2capRegisterRequestLazy(Task clientTask, Task connectionTask, uint16 psm, const profile_task_recipe *recipe);


/*! 
	@brief Unregister an L2CAP PSM from the Connection library.

	@param theAppTask The client task. The CL_L2CAP_UNREGISTER_CFM message is 
	retuned to this task. Only the task that registered the PSM with the 
	Connection library can unregister it.

	@param psm The PSM being unregistered from the Connection library.

	Once a PSM has been unregistered connections can no longer be accepted 
	or initiated on that PSM. All incoming connection requests will be 
	automatically rejected by the Connection library.

	The client task will receive a CL_L2CAP_UNREGISTER_CFM message from the 
	Connection library indicating the outcome of this request.
*/
void ConnectionL2capUnregisterRequest(Task theAppTask, uint16 psm);


/*! 
	@brief Initiate the creation of an L2CAP connection to a particular device.

	@param theAppTask The client task initiating the L2CAP connection. The
	response indicating the outcome of the connect request will be returned to
	this task.
	
	@param addr The address of the remote device being connected to. 
	
	@param psm_local The local PSM being connected. This PSM must have been
	registered with the Connection library before a connection can be
	initiated.

	@param psm_remote The remote PSM being connected to.

	@param config The configuration parameters for the L2CAP connection. To use
	default parameters set this pointer to null. For an explanation of each
	member of the l2cap_config_params structure please refer to its definition.

	The client task will receive a CL_L2CAP_CONNECT_CFM message from the 
	Connection library indicating the outcome of this request.
*/
void ConnectionL2capConnectRequest(Task theAppTask, const bdaddr *addr, uint16 psm_local, uint16 psm_remote, const l2cap_config_params *config);


/*! 
	@brief Response to a notification of an incoming L2CAP connection.

	@param theAppTask The client task that the CL_L2CAP_CONNECT_IND message 
	was sent to.
	
	@param response Set to TRUE to accept the incoming connection or FALSE 
	to reject it. If rejecting the connection the psm and connection_id fields 
	still have to be set but the config parameter can be set to null.
	
	@param psm The PSM on the local device that the remote end is trying to 
	connect to. The psm field from the CL_L2CAP_CONNECT_IND message can be
	copied directly.
	
	@param connection_id The connection identifier. This is the connection_id
	field from the CL_L2CAP_CONNECT_IND message that is being responded to.
	This field should be copied from the CL_L2CAP_CONNECT_IND message directly.
	It is necessary so that if two connections to the same PSM are being
	established the Connection library knows which particular connection is
	being reponded to.
	
	@param config The configuration parameters for the L2CAP connection. To use
	default parameters set this pointer to null. For an explanation of each
	member of the l2cap_config_params structure please refer to its definition.

	On an indication of an incoming L2CAP connection the Connection library
	sends the client task that registered the PSM being connected to a 
	CL_L2CAP_CONNECT_IND message. In response to this message the client task 
	must call this function to indicate whether it wishes to proceed with this
	connection or not.

	The client task will receive a CL_L2CAP_CONNECT_CFM message from the 
	Connection library indicating the outcome of the connection attempt.
*/
void ConnectionL2capConnectResponse(Task theAppTask, bool response, uint16 psm, uint16 connection_id, const l2cap_config_params *config);


/*! 
	@brief Request to disconnect an L2CAP connection.

	@param theAppTask The client task. This should be the task that owns the 
	L2CAP connection. Only a task that owns a particular connection can issue 
	a disconnect request for it. 

	@param sink The sink identifying the L2CAP connection. 

	The client task will receive a CL_L2CAP_DISCONNECT_IND message from the 
	Connection library indicating the outcome of this request.  
*/
void ConnectionL2capDisconnectRequest(Task theAppTask, Sink sink);


/*! 
	@brief This function is called to allocate an RFCOMM server channel. In
	response, a CL_RFCOMM_RGISTER_CFM message will be sent to the specified
	task.

    @param theAppTask The client task where the cfm message will be sent.
*/
void ConnectionRfcommAllocateChannel(Task theAppTask);


/*! 
	@brief This function is called to allocate an RFCOMM server channel. In
	response, a CL_RFCOMM_RGISTER_CFM message will be sent to the specified
	task.

    @param clientTask The client task where the cfm message will be sent.

    @param connectionTask The task where incoming connection indications 
    will be sent.

	@param recipe Structure passed in from a profile library which can be
    used by the connection library to create a profile library instance for
    handling an incoming RFCOMM connection.
*/
void ConnectionRfcommAllocateChannelLazy(Task clientTask, Task connectionTask, const profile_task_recipe *recipe);


/*! 
	@brief This function is called to request an RFCOMM connection to the
	device with the specified Bluetooth device Address and server channel.

	@param theAppTask The client task.
	@param bd_addr The Bluetooth address of the remote device.
	@param local_server_chan The local server channel.
	@param remote_server_chan The remote server channel.
	@param config The RFCOMM configuration parameters.
*/
void ConnectionRfcommConnectRequest(Task theAppTask, const bdaddr* bd_addr, uint8 local_server_chan, uint8 remote_server_chan, const rfcomm_config_params *config);


/*! 
	@brief This function is called to respond to a CL_RFCOMM_CONNECT_IND

	@param theAppTask The client task.
	@param response Accept connection(TRUE) or refuse (FALSE).
	@param bd_addr The Bluetooth address of the remote device.
	@param server_chan The server channel.
	@param config The RFCOMM configuration parameters.

*/
void ConnectionRfcommConnectResponse(Task theAppTask, bool response, const bdaddr* bd_addr, uint8 local_server_channel, const rfcomm_config_params *config);


/*!
	@brief This function is called to request a disconnection of the RFCOMM
	channel.  

	@param theAppTask The client task.  
	@param sink The RFCOMM channel to disconnect.
*/
void ConnectionRfcommDisconnectRequest(Task theAppTask, Sink sink);


/*!
	@brief This function is called to send control signals to the remote
	device.

	@param theAppTask The client task.
	@param sink The sink.
	@param break_signal The break signal.
	@param modem_signal The modem signal.

	This function should only be called if the Rfcomm connection is active.
*/
void ConnectionRfcommControlSignalRequest(Task theAppTask, Sink sink, uint16 break_signal, uint16 modem_signal);


/*!
	@brief This function is called to place the device in "Device Under Test"
	(DUT) mode.

	This function places the local device into DUT mode. It makes the device
	discoverable/ connectable, disables security and then issues the
	DM_HCI_ENABLE_DEVICE_UT_MODE message to BlueStack. A CL_DM_DUT_CFM message
	is returned to the task that initialised the connection library to indicate
	whether DUT mode was entered successfully.
*/
void ConnectionEnterDutMode(void);

/*!
	@brief This function is called to set the link supervision timeout.

	@param theAppTask The client task.
	@param sink The sink.
	@param timeout The timeout in 0.625ms units.
*/
void ConnectionSetLinkSupervisionTimeout(Sink sink, uint16 timeout);


/*!
	@brief This function is called to use the passed in link policy power table.

	@param theAppTask The client task.
	@param sink The sink.
	@param size_power_table The number of entries in the power table.
	@param power_table The power table.
*/
void ConnectionSetLinkPolicy(Sink sink, uint16 size_power_table, lp_power_table const *power_table);


/*!
	@brief This function is called to set up preferred subrating parameters to be used when the 
	device enters sniff mode.

	@param theAppTask The client task.
	@param sink The sink.
	@param max_remote_latency The maximum time the remote device need not be present when subrating
	(in 0.625ms units).
	@param min_remote_timeout The minimum time the remote device should stay in sniff before entering 
	subrating mode (in 0.625ms units).
	@param min_local_timeout The minimum time the local device should stay in sniff before entering 
	subrating mode (in 0.625ms units).

    This is a BT2.1 only feature
*/
void ConnectionSetSniffSubRatePolicy(Sink sink, uint16 max_remote_latency, uint16 min_remote_timeout, uint16 min_local_timeout);


/*!
	@brief This function is called to change the default role switch behaviour.

	@param rs_table The role switch table. Set to 0 to use default settings which do not issue a role switch.
*/
void ConnectionSetRoleSwitchParams(const uint16 *rs_table);


#endif	/* CONNECTION_H_ */
