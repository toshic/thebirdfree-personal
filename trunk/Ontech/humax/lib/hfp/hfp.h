/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp.h

DESCRIPTION
	Header file for the HFP profile library.
*/

/*!
@file	hfp.h
@brief	Header file for the Hands Free Profile library.

		This file documents the Hands Free Profile library API for BlueLab3.
*/

#ifndef HFP_H_
#define HFP_H_

#include <bdaddr_.h>
#include <connection.h>
#include <message.h>


struct __HFP;
/*!
	@brief The Hands Free Profile structure.
*/
typedef struct __HFP HFP;



/*! \name Hfp Supported Features Flag Defines

	These flags can be or'd together and used as the supported_features field
	of an hfp_init_params structure.
*/
/*! \{ */
/*!
	@brief Setting this flag indicates that this device implements noise
	reduction / echo cancellation.
*/
#define HFP_NREC_FUNCTION           (1)
/*!
	@brief Setting this flag indicates that this device supports three way call
	control.
*/
#define HFP_THREE_WAY_CALLING       (1<<1)
/*!
	@brief Setting this flag indicates that this device can display calling
	line identification information.
*/
#define HFP_CLI_PRESENTATION        (1<<2)
/*!
	@brief Setting this flag indicates that this device can enable voice
	recognition on the AG.
*/
#define HFP_VOICE_RECOGNITION       (1<<3)
/*!
	@brief Setting this flag indicates that the AG can control this device's
	local volume.
*/
#define HFP_REMOTE_VOL_CONTROL	    (1<<4)
/*!
	@brief Setting this flag indicates that this device can request a list of
	current calls from the AG and also receive call status indicators from the AG.
*/
#define HFP_ENHANCED_CALL_STATUS    (1<<5)
/*!
	@brief Setting this flag indicates that this device can use the extended
	three-way calling features of the AG.
*/
#define HFP_ENHANCED_CALL_CONTROL   (1<<6)
/*! \} */


/*!
	@brief The supported profiles.
*/
typedef enum
{
	/*! The supported profile has not been set for this profile instance. */
	hfp_no_profile,
	/*! As defined in part K6 of the Bluetooth specification.*/
	hfp_headset_profile,
	/*! As defined in version 1.0 of the handsfree profile specification.*/
	hfp_handsfree_profile,
	/*! As defined in version 1.5 of the handsfree profile specification.*/
	hfp_handsfree_15_profile
} hfp_profile;


/*!
	@brief Configuration parameters passed into the hfp profile library in
	order for an Hfp profile instance to be created and initialised.

	The library client (usually the application) must specify the profile to be
	supported. In the case of hands free it must also specify the set of
	supported features for that profile instance. Optionally, the client may
	also supply a service record. If the service_record pointer is set to null
	the default service record provided by the hfp library is used. If however
	the pointer is non null, this client supplied service record is used. The
	hfp library will still insert the correct rfcomm channel number into the
	service record but it will perform no other error checking on the record
	provided.  The client provided service record may be fairly large so the
	hfp library will not attempt to take a local copy of the data. For the
	moment the hfp library assumes that the service record pointer is in const
	or that the client will not free the pointer for the lifetime of the
	application.
*/
typedef struct
{
	/*! Either headset or handsfree profile.*/
	hfp_profile	supported_profile;
	/*! See supported features flags.*/
	uint16		supported_features;
	/*! Length of the service record provided by the client. */
	uint16		size_service_record;
	/*! Service record provided by the client. This is used when the
		application does not want to use the default service record provided by
		the hfp library.*/
	uint8		*service_record;
} hfp_init_params;


/*!
	@brief Connection parameters for setting up an eSCO/SCO connection
*/
typedef struct
{
   uint32            bandwidth;
   uint16            max_latency;
   uint16            voice_settings;
   sync_retx_effort  retx_effort;
} hfp_audio_params;


/*!
	 @brief Generic hfp status.
*/
typedef enum
{
	hfp_success,						/*!< Success.*/
	hfp_fail,							/*!< Failure.*/
    hfp_ag_failure,                     /*!< Failure - AG failure.*/
    hfp_no_connection_to_phone,         /*!< Failure - No connection to phone.*/
    hfp_operation_not_allowed,          /*!< Failure - Operation not allowed.*/
    hfp_operation_not_supported,        /*!< Failure - Operation not supported.*/
    hfp_ph_sim_pin_required,            /*!< Failure - PH-SIM PIN required.*/
    hfp_sim_not_inserted,               /*!< Failure - SIM not inserted.*/
    hfp_sim_pin_required,               /*!< Failure - SIM PIN required.*/
    hfp_sim_puk_required,               /*!< Failure - SIM PUK required.*/
    hfp_sim_failure,                    /*!< Failure - SIM failure.*/
    hfp_sim_busy,                       /*!< Failure - SIM busy.*/
    hfp_incorrect_password,             /*!< Failure - Incorrect password.*/
    hfp_sim_pin2_required,              /*!< Failure - SIM PIN2 required.*/
    hfp_sim_puk2_required,              /*!< Failure - SIM PUK2 required.*/
    hfp_memory_full,                    /*!< Failure - Memory full.*/
    hfp_invalid_index,                  /*!< Failure - Invalid index.*/
    hfp_memory_failure,                 /*!< Failure - Memory failure.*/
    hfp_text_string_too_long,           /*!< Failure - Text string too long.*/
    hfp_invalid_chars_in_text_string,   /*!< Failure - Invalid characters in text string.*/
    hfp_dial_string_too_long,           /*!< Failure - Dial string too long.*/
    hfp_invalid_chars_in_dial_string,   /*!< Failure - Invalid characters in dial string.*/
    hfp_no_network_service,             /*!< Failure - No network service.*/
    hfp_network_not_allowed,            /*!< Failure - Network not allowed, emergency calls only.*/

    hfp_csr_not_inited,			/*!< Failure - CSR extensions have not been initialised. */
	hfp_csr_no_slc,				/*!< Failure - CSR extension request failed due to no SLC being present. */
	hfp_csr_invalid_params,		/*!< Failure - CSR extention request failed due to being supplied with invalid parameters from the application. */
	hfp_csr_invalid_ag_params,	/*!< Failure - CSR extention request failed due to being supplied with invalid parameters from the AG. */
	hfp_csr_mod_ind_no_mem		/*!< Failure - CSR extension modify indicators failed due to not being able to allocate
												memory for AT Command generation.  A request with fewer indictors may succeed */
} hfp_lib_status;


/*!
	@brief Return status for the HFP_INIT_CFM message
*/
typedef enum
{
	/*! Successful initialisation.*/
	hfp_init_success,
	/*! Unsuccessful due to RFCOMM channel registration failure.*/
	hfp_init_rfc_chan_fail,
	/*! Unsuccessful due to a service record registration failure.*/
	hfp_init_sdp_reg_fail
} hfp_init_status;


/*!
	@brief Return status for the HFP_SLC_CONNECT_CFM message
*/
typedef enum
{
	/*! Successful connection.*/
	hfp_connect_success,
	/*! Unsuccessful due to a service search failure.*/
	hfp_connect_sdp_fail,
	/*! Unsuccessful due to a service level connection failure.*/
	hfp_connect_slc_failed,
	/*! Unsuccessful due to service level connection already established.*/
	hfp_connect_failed_busy,
	/*! Unsuccessful due to RFCOMM connection failing to be established.*/
    hfp_connect_failed,
	/*! Unsuccessful due to attempt to connect to unallocated server channel.*/
    hfp_connect_server_channel_not_registered,
	/*! Unsuccessful due to connection attempt timing out.*/
    hfp_connect_timeout,
	/*! Unsuccessful due to remote device rejecting connection.*/
    hfp_connect_rejected,
	/*! Unsuccessful due to remote device terminating the connection.*/
    hfp_connect_normal_disconnect,
	/*! Unsuccessful due to an abnormal disconnect while establishing an rfcomm
	  connection.*/
    hfp_connect_abnormal_disconnect,
    /*! Connection failed due to bad parameters supplied by the application. */
    hfp_connect_fail_bad_params
} hfp_connect_status;


/*!
	@brief Return status for the HFP_SLC_DISCONNECT_IND message
*/
typedef enum
{
	/*! Successful disconnection.*/
	hfp_disconnect_success,
	/*! Unsuccessful due to abnormal link loss.*/
	hfp_disconnect_link_loss,
	/*! Unsuccessful due to no current connection.*/
	hfp_disconnect_no_slc,
	/*! Unsuccessful due to RFCOMM connection attempt timeout.*/
	hfp_disconnect_timeout,
	/*! Unsuccessful due to RFCOMM connection attempt error.*/
	hfp_disconnect_error
} hfp_disconnect_status;


/*!
	@brief Return status for the HFP_AUDIO_CONNECT_CFM message.
*/
typedef enum
{
    /*! Successful audio connection.*/
    hfp_audio_connect_success,
    /*! Unsuccessful due to negotiation failure.*/
    hfp_audio_connect_failure,
    /*! Unsuccessful due to audio already being with device.*/
    hfp_audio_connect_have_audio,
    /*! Unsuccessful due to an audio connect already being attempted.*/
    hfp_audio_connect_in_progress,
    /*! Unsuccessful due to one or more parameters specified being invalid.*/
    hfp_audio_connect_invalid_params,
    /*! Unsuccessful due to library being in incorrect state.*/
    hfp_audio_connect_error
} hfp_audio_connect_status;


/*!
	@brief Return status for the HFP_AUDIO_DISCONNECT_IND message.
*/
typedef enum
{
    /*! Successful audio disconnection.*/
    hfp_audio_disconnect_success,
    /*! Unsuccessful due to failure indication from firmware.*/
    hfp_audio_disconnect_failure,
    /*! Unsuccessful due to audio being with AG.*/
    hfp_audio_disconnect_no_audio,
    /*! Unsuccessful due to an audio disconnect already being attempted.*/
    hfp_audio_disconnect_in_progress,
    /*! Unsuccessful due to library being in incorrect state.*/
    hfp_audio_disconnect_error
} hfp_audio_disconnect_status;


/*!
	@brief Transfer direction for audio connection.
*/
typedef enum
{
	/*! Transfer the audio to the HFP device.*/
	hfp_audio_to_hfp,
	/*! Transfer the audio to the audio gateway.*/
	hfp_audio_to_ag,
	/*! Toggle direction of current audio.*/
	hfp_audio_transfer
} hfp_audio_transfer_direction;


/*!
	@brief Values of the call_setup indicator.
*/
typedef enum
{
	/*! No call currently being established.*/
	hfp_no_call_setup,
	/*! HFP device currently ringing.*/
	hfp_incoming_call_setup,
	/*! Call currently being dialed.*/
	hfp_outgoing_call_setup,
	/*! Remote end currently ringing.*/
	hfp_outgoing_call_alerting_setup
} hfp_call_setup;


/*!
	@brief Values of the subscriber number service.
*/
typedef enum
{
	/*! Asynchronous modem.*/
    hfp_service_async_modem,
	/*! Synchronous modem.*/
    hfp_service_sync_modem,
	/*! PAD access (synchronous).*/
    hfp_service_pad_access,
	/*! Packet access (asynchronous).*/
    hfp_service_packet_access,
	/*! Voice.*/
    hfp_service_voice,
	/*! Fax.*/
    hfp_service_fax
} hfp_subscriber_service;


/*!
    @brief Call direction used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call from AG to network.*/
    hfp_call_mobile_originated,
    /*! Call from network to AG.*/
    hfp_call_mobile_terminated
} hfp_call_direction;


/*!
    @brief Call status used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call is currently active.*/
    hfp_call_active,
    /*! Call is currently held.*/
    hfp_call_held,
    /*! Call is being dialled - mobile originated only.*/
    hfp_call_dialling,
    /*! Call is alerting - mobile originated only.*/
    hfp_call_alerting,
    /*! Call is incoming - mobile terminated only.*/
    hfp_call_incoming,
    /*! Call is waiting - mobile terminated only.*/
    hfp_call_waiting
} hfp_call_status;


/*!
    @brief Call mode used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Voice call.*/
    hfp_call_voice,
    /*! Data call.*/
    hfp_call_data,
    /*! FAX call.*/
    hfp_call_fax
} hfp_call_mode;


/*!
    @brief Call multiparty status used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call is not multiparty.*/
    hfp_not_multiparty_call,
    /*! Call is multiparty.*/
    hfp_multiparty_call
} hfp_call_multiparty;


/*!
    @brief Response hold status used in HFP_RESPONSE_HOLD_STATUS_IND message.
*/
typedef enum
{
    /*! An incoming call has been put on hold.*/
    hfp_incoming_call_held,
    /*! A previously held incoming call has been accepted.*/
    hfp_held_call_accepted,
    /*! A previously held incoming call has been rejected.*/
    hfp_held_call_rejected
} hfp_response_hold_state;


/*!
    @brief Used to identify type of number specified in HFP_SUBSCRIBER_NUMBER_IND,
           HFP_CALLER_ID_IND, HFP_CALL_WAITING_IND and HFP_CURRENT_CALLS_IND.
*/
typedef enum
{
    /*! Type of number is unknown.*/
    hfp_number_unknown,
    /*! Number is an international number.*/
    hfp_number_international,
    /*! Number is a national number.*/
    hfp_number_national,
    /*! Number is a network specific number.*/
    hfp_number_network,
    /*! Number is a dedicated access, short code.*/
    hfp_number_dedicated
} hfp_number_type;

/*!
    @brief Power source status.
*/
typedef enum
{
	/*! Device in using battery power. */
	hfp_csr_pwr_rep_battery,
	/*! Device is using an external power source. */
	hfp_csr_pwr_rep_external
} hfp_csr_power_status_report;

/*!
	@brief Indicators supported by CSR 2 CSR extensions
*/
typedef enum
{
	/*! Caller Name indication */
	hfp_csr_ind_name = 1,
	/*! Unsolicited text */
	hfp_csr_ind_text,
	/*! SMS Notification */
	hfp_csr_ind_sms,
	/*! Battery indications */
	hfp_csr_ind_battery,
	/*! Power source indications */
	hfp_csr_ind_power
} hfp_csr_indicator;

/*!
	@brief CSR 2 CSR extended feature to change.
	
	Used to inform the AG of the HF changing it's supported indications, 
	and for the AG to inform the HF of it's supported indications.
*/
typedef struct
{
	/*! Indicator to change */
	hfp_csr_indicator indicator;
	/*! New value */
	uint16 value;
} hfp_csr_mod_indicator;

/*!
	@brief Define the types for the upstream messages sent from the Hfp profile
	library to the application.
*/
#define HFP_MESSAGE_BASE	0x6e00

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	HFP_INIT_CFM = HFP_MESSAGE_BASE,    

	/* Service Level Connection messages */
	HFP_SLC_CONNECT_CFM,                
	HFP_SLC_CONNECT_IND,                
	HFP_SLC_DISCONNECT_IND,             
	HFP_SINK_CFM,                       

	/* Audio messages */
    HFP_AUDIO_CONNECT_CFM,              
    HFP_AUDIO_CONNECT_IND,              
    HFP_AUDIO_DISCONNECT_IND,           

	/* Indicators */
	HFP_SERVICE_IND,
	HFP_REMOTE_AG_PROFILE15_IND,
	HFP_CALL_IND,
	HFP_CALL_SETUP_IND,
	HFP_SIGNAL_IND,
	HFP_ROAM_IND,
	HFP_BATTCHG_IND,
	HFP_CALLHELD_IND,
	HFP_RING_IND,
	HFP_IN_BAND_RING_IND,
	HFP_CALLER_ID_ENABLE_CFM,
	HFP_CALLER_ID_IND,
    HFP_CALLER_ID_NAME_IND,

	/* Call control */
	HFP_ANSWER_CALL_CFM,
	HFP_REJECT_CALL_CFM,
	HFP_TERMINATE_CALL_CFM,
	HFP_CALL_WAITING_ENABLE_CFM,
	HFP_CALL_WAITING_IND,
	HFP_CALL_WAITING_NAME_IND,
	HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM,
	HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM,
	HFP_RELEASE_SPECIFIED_ACCEPT_OTHER_CALL_CFM,
	HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM,
	HFP_REQUEST_PRIVATE_HOLD_OTHER_CALL_CFM,
	HFP_ADD_HELD_CALL_CFM,
	HFP_EXPLICIT_CALL_TRANSFER_CFM,
    HFP_RESPONSE_HOLD_STATUS_CFM,
    HFP_RESPONSE_HOLD_HELD_CFM,
    HFP_RESPONSE_HOLD_ACCEPT_CFM,
    HFP_RESPONSE_HOLD_REJECT_CFM,
    HFP_RESPONSE_HOLD_STATUS_IND,

	/* Dialling */
	HFP_LAST_NUMBER_REDIAL_CFM,                  
	HFP_DIAL_NUMBER_CFM,                         
	HFP_DIAL_MEMORY_CFM,                         
	HFP_VOICE_RECOGNITION_ENABLE_CFM,            
	HFP_VOICE_RECOGNITION_IND,                   

	/* HSP messages */
	HFP_HS_BUTTON_PRESS_CFM,                     

	/* Volume controls */
	HFP_SPEAKER_VOLUME_CFM,                      
	HFP_SPEAKER_VOLUME_IND,                      
	HFP_MICROPHONE_VOLUME_CFM,                   
	HFP_MICROPHONE_VOLUME_IND,                   

	/* Other */
	HFP_DISABLE_NREC_CFM,
	HFP_VOICE_TAG_NUMBER_CFM,
	HFP_DTMF_CFM,
	HFP_UNRECOGNISED_AT_CMD_IND,
	HFP_EXTRA_INDICATOR_INDEX_IND,
	HFP_EXTRA_INDICATOR_UPDATE_IND,
	HFP_ENCRYPTION_CHANGE_IND,
	HFP_ENCRYPTION_KEY_REFRESH_IND,
    HFP_SUBSCRIBER_NUMBER_CFM,
    HFP_SUBSCRIBER_NUMBER_IND,
    HFP_CURRENT_CALLS_CFM,
    HFP_CURRENT_CALLS_IND,
    HFP_NETWORK_OPERATOR_IND,
    HFP_NETWORK_OPERATOR_CFM,
    HFP_EXTENDED_ERROR_IND,
    HFP_EXTENDED_ERROR_CFM,
    
    /* CSR Extended Features */
    HFP_CSR_SUPPORTED_FEATURES_CFM,
    HFP_CSR_TXT_IND,
    HFP_CSR_MODIFY_INDICATORS_CFM,
    HFP_CSR_NEW_SMS_IND,
    HFP_CSR_NEW_SMS_NAME_IND,
    HFP_CSR_SMS_CFM,
    HFP_CSR_MODIFY_AG_INDICATORS_IND,
    HFP_CSR_AG_INDICATORS_DISABLE_IND,
    HFP_CSR_AG_REQUEST_BATTERY_IND,
    HFP_CSR_FEATURE_NEGOTIATION_IND ,

	HFP_MESSAGE_TOP
}HfpMessageId ;
#endif /*end of DO_NOT_DOCUMENT*/


/*
	The upstream messages from the Hfp profile library to the application are
	defined below. Each message has an hfp parameter which is a pointer to the
	profile instance that the message applies to.
*/


/*!
	@brief This message is generated as a result of a call to HfpInit.

	The application creates a profile instance by calling HfpInit. This
	function is responsible for creating a task for the profile instance,
	initialising that task and its associated state and performing all other
	initialisation related tasks, e.g. registering a service record.  Once the
	initialisation is completed this message will be returned to the
	application.
*/
typedef struct
{
	/*! Pointer to hfp profile instance. This will point to a valid Hfp profile
	  instance if the initialisation succeeded. If however, any part of the
	  initialisation process failed, the hfp pointer will not be valid.*/
    HFP				*hfp;
	/*! If the task was created and initialised successfully then this will be
	  set to hfp_init_success otherwise it will be set to indicate why the
	  initialisation failed. */
	hfp_init_status	status;
} HFP_INIT_CFM_T;


/*!
	@brief This message returns the result of an attempt to establish a Service
	Level Connection.

	This message is sent to the application once a Service Level Connection has
	been established or the connect attempt has failed. It is sent to inform
	the application that the Service Level Connection establishment phase has
	completed. The application will receive this message regardless of whether
	it initiated the Service Level Connection establishment or whether this was
	done by the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP					*hfp;
	/*! Indicates whether or not the connection was successfully established.*/
	hfp_connect_status	status;
	/*! The sink associated with this HFP Service Level Connection. */
	Sink				sink;
} HFP_SLC_CONNECT_CFM_T;


/*!
	@brief This message is sent when a remote device wishes to establish a
	Service Level Connection.

	This message is sent to the application to indicate that a remote device
	(the AG) is requesting to establish a Service Level Connection to this
	device.  The application must respond to this message even if it wants to
	reject the incoming Service Level Connection.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The Bluetooth address of the remote device.*/
	bdaddr	addr;
} HFP_SLC_CONNECT_IND_T;


/*!
	@brief This message is sent when the profile version of remote device has 
     been come out.

	This message is sent to the application to indicate that the profile version of 
	a remote device	(the AG) is HFP version 1.5.  The application must respond to this message 
    and send a command AT+BTRH? to AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The supported profile of the remote device.*/
	hfp_profile	   agSupportedProfile;
	
} HFP_REMOTE_AG_PROFILE15_IND_T;



/*!
	@brief This message is sent when a Service Level Connection has been
	disconnected.

	This message informs the application that the Service Level Connection has
	been disconnected.  This disconnect can be the result of a locally
	initiated action, a disconnect initiated from the AG or an abnormal
	disconnect e.g. link loss.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP						*hfp;
	/*! Indicates the reason for the disconnect.*/
	hfp_disconnect_status	status;
} HFP_SLC_DISCONNECT_IND_T;


/*!
	@brief This messages returns the sink associated with a Service Level
	Connection.

*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If a valid sink is returned, status will be set to hfp_success,
	  otherwise it will be set to hfp_fail.*/
	hfp_lib_status	status;
	/*! If we currently have a Service Level connection and status is set to
	  hfp_success, this will hold the sink associated with the currently
	  established Service level connection. Otherwise it will be set to zero.*/
	Sink			sink;
} HFP_SINK_CFM_T;


/*!
	@brief This message informs the app that an audio (SCO/eSCO) connection is being requested by the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
	/* pass address up through lib as SinkGetBdAddr is failing due to a race condition on hsv5 */
    bdaddr 			bd_addr;
} HFP_AUDIO_CONNECT_IND_T;


/*!
	@brief This message informs the app that the audio (SCO/eSCO) has been connected or that the
	attempt to open a SCO/eSCO connection has failed.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP                        *hfp;		    
    /*! If the return status from the connection library is hci_success, then status will be 
        set to hfp_success. Otherwise it will be set to hfp_fail. */	
	hfp_audio_connect_status    status;	
	/*! Indicates whether a SCO or eSCO link was created. */	    
	sync_link_type              link_type;
	/*! If a SCO/eSCO connection has been created the audio_sink will be the corresponding sink
	    for the SCO/eSCO data, otherwise it will be set to zero. */
	Sink                        audio_sink;
	/*! Receive bandwith in bytes per second. */   
	uint32                      rx_bandwidth;
	/*!< Transmit bandwith in bytes per second. */
	uint32                      tx_bandwidth;
} HFP_AUDIO_CONNECT_CFM_T;


/*!
	@brief This message informs the app that the audio (SCO/eSCO) has been disconnected.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection. */
	HFP				           *hfp;
	/*! If the return status from the connection library is hci_success then status
		is set to hfp_success. Otherwise the status is set to hfp_fail. */
	hfp_audio_disconnect_status status;      
} HFP_AUDIO_DISCONNECT_IND_T;


/*!
	@brief This message informs the application of a change in the service
	indicator's status.

	This message is sent on Service Level Connection establishment and
	subsequently whenever a service indicator update is received.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The new value of the service indicator.*/
	uint16	service;
} HFP_SERVICE_IND_T;


/*!
	@brief This message informs the application of a change in the call
	indicator's status.

	This message is sent on Service Level Connection establishment and
	subsequently whenever a call indicator update is received.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The new value of the call indicator.*/
	uint16	call;
} HFP_CALL_IND_T;


/*!
	@brief This message informs the application of a change in the call_setup
	indicator's status.

	This message is sent on Service Level Connection establishment and
	subsequently whenever a call_setup indicator update is received.

	The call_setup indicator is not supported by HFP v0.96 compliant AGs so the
	Hfp library has to, in some cases, assume the current state of the call
	process at the AG. In this case the Hfp library will send this message to
	the application informing it of what it thinks is the current state of the
	call establishment.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! The new value of the call_setup indicator.*/
	hfp_call_setup	call_setup;
} HFP_CALL_SETUP_IND_T;


typedef struct
{
	uint16   signal;    /*!< The new value of the call_setup indicator.*/
	HFP     *hfp;       /*!< Pointer to hfp profile instance that is handling the Service Level Connection.*/
} HFP_SIGNAL_IND_T;

typedef struct
{
	uint16   roam;    /*!< The new value of the call_setup indicator.*/
	HFP     *hfp;     /*!< Pointer to hfp profile instance that is handling the Service Level Connection.*/
} HFP_ROAM_IND_T;

typedef struct
{
	uint16   battchg;    /*!< The new value of the call_setup indicator.*/
	HFP     *hfp;        /*!< Pointer to hfp profile instance that is handling the Service Level Connection.*/
} HFP_BATTCHG_IND_T;


/*!
	@brief This message informs the application of a change in the callheld
	indicator's status.

	This message is sent on Service Level Connection establishment and
	subsequently whenever a callheld indicator update is received.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP     *hfp;
	/*! The new value of the call_setup indicator.*/
	uint16   callheld;
} HFP_CALLHELD_IND_T;


/*!
	@brief This message is sent to the application whenever a RING indication
	is received from the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
	HFP     *hfp;
} HFP_RING_IND_T;


/*!
	@brief This message informs the application of the current in band ring
	setting of the AG.

	This message is sent to the application on obtaining the AG's supported
	features and subsequently whenever the AG sends an AT indication informing
	the Hfp library of its in band ring setting.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! Enabled(TRUE) or disabled(FALSE).*/
	bool	ring_enabled;
} HFP_IN_BAND_RING_IND_T;


/*!
	@brief This message informs the application of the outcome of this enable/
	disable command sent to the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_CALLER_ID_ENABLE_CFM_T;


/*!
	@brief If the application has enabled caller id notifications at the AG,
	this message will be sent to the application whenever a caller id
	notification is received from the AG.

*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
	HFP		       *hfp;
	/*! Type of number. */
	hfp_number_type number_type;
	/*! The number of bytes pointed to by number.*/
	uint16	        size_caller_number;
	/*! The number of bytes pointed to by name. 
	  It is mainly used by applications to decide whether 
	  caller name has been displayed or not.    */
	uint16          size_caller_name;
	/*! Pointer to the number of the incoming call (if available). The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
	uint8	        caller_number[1];
} HFP_CALLER_ID_IND_T;


/*!
    @brief Alphanumeric representation of the number in the preceeding
    HFP_CALLER_ID_IND message.

    The AG may optionally include an alphanumeric representation of the caller
    id number in the CLIP indication it sends to teh HF. If this string is
    included the hfp library will send a single HFP_CALLER_ID_NAME_IND message
    to the client containing the name in the CLIP indication.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
	/*! The number of bytes in the caller_name string. */
    uint16  size_caller_name;
	/*! The alphanumeric representation of the caller id number. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   caller_name[1];
} HFP_CALLER_ID_NAME_IND_T;


/*!
	@brief This message is sent in response to a request from the application
	to accept an incoming call.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_ANSWER_CALL_CFM_T;


/*!
	@brief This message is sent in response to a request from the application
	to reject an incoming call.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_REJECT_CALL_CFM_T;


/*!
	@brief This message is sent in response to a request from the application
	to terminate an outgoing call process or hang up an active call.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_TERMINATE_CALL_CFM_T;


/*!
	@brief This message is sent in response to a request from the application
	to the AG to enable /disable call waiting notifications.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_CALL_WAITING_ENABLE_CFM_T;


/*!
	@brief This message is sent from the Hfp profile library to the application
	whenever a call waiting notification is received from the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
	HFP		           *hfp;
	/*! Type of number. */
	hfp_number_type     number_type;
	/*! Specifies the number of bytes pointed to by number.*/
	uint16	            size_caller_number;
	/*! Pointer to the phone number of the waiting call (if available). The
	  client should not attempt to free this pointer, the memory will be freed
	  when the message is destroyed. If the client needs access to this data
	  after the message has been destroyed it is the client's responsibility to
	  copy it.*/
	uint8	            caller_number[1];
} HFP_CALL_WAITING_IND_T;

/*!
    @brief Alphanumeric representation of the number in the preceeding
    HFP_CALL_WAITING_IND message.
    
	The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
	/*! The number of bytes in the caller_name string. */
    uint16  size_caller_name;
	/*! The alphanumeric representation of the caller. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   caller_name[1];
} HFP_CALL_WAITING_NAME_IND_T;

/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=0 message.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=1 message.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=1,<idx> message.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RELEASE_SPECIFIED_ACCEPT_OTHER_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=2 message.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=2,<idx> message.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_REQUEST_PRIVATE_HOLD_OTHER_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=3 message.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_ADD_HELD_CALL_CFM_T;


/*!
	@brief This message informs the application of the AG's response to the
	AT+CHLD=4 message.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_EXPLICIT_CALL_TRANSFER_CFM_T;


/*!
	@brief This message informs the application of the outcome of this dial
	request.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_LAST_NUMBER_REDIAL_CFM_T;


/*!
	@brief This message informs the application of the outcome of a request to
	dial a specific number.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_DIAL_NUMBER_CFM_T;


/*!
	@brief This message is sent in response to a request to the AG to dial a
	number from a particular memory location.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_DIAL_MEMORY_CFM_T;


/*!
	@brief This message is sent in response to the voice recognition enable/
	disable command being sent to the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_VOICE_RECOGNITION_ENABLE_CFM_T;


/*!
	@brief This is an unsolicited message sent in response to a voice
	recognition indication received from the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The current status of the AG's voice recognition function. Enabled
	  (TRUE) or disabled (FALSE).*/
	uint16	enable;
} HFP_VOICE_RECOGNITION_IND_T;


/*!
	@brief This message is sent in response to the AT button press command (as
	defined in the HSP specification) being sent to the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HFP and not HSP.*/
	hfp_lib_status	status;
} HFP_HS_BUTTON_PRESS_CFM_T;


/*!
	@brief This message is sent in response to the AT command notifying the AG
	of a change in the speaker gain setting on the local device.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. */
	hfp_lib_status	status;
} HFP_SPEAKER_VOLUME_CFM_T;


/*!
	@brief This message is sent in response to the AT command notifying the AG
	of a change in the microphone gain setting on the local device.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. */
	hfp_lib_status	status;
} HFP_MICROPHONE_VOLUME_CFM_T;


/*!
	@brief This message is sent in response to a speaker gain indication being
	received from the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The value from the AT indication sent by the AG.*/
	uint16	volume_gain;
} HFP_SPEAKER_VOLUME_IND_T;


/*!
	@brief This message is sent in response to a microphone gain indication
	being received from the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The value from the AT indication sent by the AG.*/
	uint16	mic_gain;
} HFP_MICROPHONE_VOLUME_IND_T;


/*!
	@brief This message is sent in response to the AT command requesting the AG
	disables its Noise Reduction(NR) /Echo Cancellation (EC) functions.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*!< If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_DISABLE_NREC_CFM_T;


/*!
	@brief This message is sent in response to the AT command requesting the AG
	supplies a number to attach to a voice tag.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*!< If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
	/*! The number of bytes pointed to by the number pointer.*/
	uint16			size_phone_number;
	/*! Pointer to the phone number. The client should not attempt to free this
	  pointer, the memory will be freed when the message is destroyed. If the
	  client needs access to this data after the message has been destroyed it
	  is the client's responsibility to copy it.*/
	uint8			phone_number[1];
} HFP_VOICE_TAG_NUMBER_CFM_T;


/*!
	@brief This message is sent to the application to inform it of the outcome
	of sending a DTMF command to the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*!<If the AG responds with OK to the AT command, status is set to
	  hfp_success. If the AG responds with ERROR or the profile instance is in
	  the wrong state to send out the command the status will be set to
	  hfp_fail. An error will also be sent if the application supplies a
	  profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_DTMF_CFM_T;


/*!
	@brief This message contains the strings that the AT command parser cannot
	parse.

	The strings that the auto generated AT command parser supports are
	specified in hfp_parse.parse along with the handler function to be called
	once a command has been parsed. However, if the parser cannot parse a
	particular AT string it will pass it to the application for further
	processing.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
	HFP		*hfp;
	/*! The number of bytes pointed to by data.*/
	uint16	size_data;
	/*! The data that could not be parsed. The client should not attempt to
	  free this pointer, the memory will be freed when the message is
	  destroyed. If the client needs access to this data after the message has
	  been destroyed it is the client's responsibility to copy it. */
	uint8	data[1];
} HFP_UNRECOGNISED_AT_CMD_IND_T;


/*!
	@brief This message returns an indicator supported by the AG.

	The application may request to be notified if the AG supports a particular
	indicator when initiating an Service Level Connection or accepting an
	incoming connection request. It does this by passing a configuration string
	to the Hfp profile library. If the +CIND response received from the AG does
	contain one of the indicators in the configuration string this message will
	be sent to the application. One message will be sent for each indicator
	found.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The index of the indicator in the configuration string the application
	  passed to the Hfp library on a connect request response.*/
	uint16	indicator_register_index;
	/*! The index of the indicator in the CIND string returned by the AG.*/
	uint16	indicator_index;
	/*! Minimum value allowed for this indicitor.*/
	uint16	min_range;
	/*! Maximum value allowed for this indicitor.*/
	uint16	max_range;
} HFP_EXTRA_INDICATOR_INDEX_IND_T;


/*!
	@brief This message returns the value of an indicator the AG supports.

	If the application registers that it is interested in a particular
	indicator (other than the ones defined by the HFP specification) it will be
	sent this message whenever an indicator status update (+CIEV) is received
	from the AG. All indicator updates will be sent to the application even if
	they are not for the indicator the application initially registered an
	interest in. It is the application's responsibility to determine whather
	the indicator update is of interest.

*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! The index of the indicator as initially reported in the CIND string
	  (i.e. indicator_index in the HFP_EXTRA_INDICATOR_INDEX_IND message).*/
	uint16	index;
	/*! The new value for this indicator. */
	uint16	value;
} HFP_EXTRA_INDICATOR_UPDATE_IND_T;


/*!
	@brief This is an unsolicited message sent to the application whenever the
	encryption status of the sink owned by this HFP profile instance changes.

	This message is generated as a result of a procedure initiated by the
	remote device. The application does not have to take any action on
	receiving the message.  However, some applications may choose not to
	proceed with a connection if encyrption is disabled on it (by the remote
	device).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP		*hfp;
	/*! Encryption status of this connection, TRUE if encrypted, FALSE
	  otherwise. */
	bool	encrypted;
} HFP_ENCRYPTION_CHANGE_IND_T;


/*!
	@brief This is a message sent to the application whenever the encryption key 
	of the sink owned by this HFP profile instance is refreshed.

	This message is generated as a result of a procedure initiated by either the
	local or remote device. The application does not have to take any action on
	receiving the message.  However, some applications may choose not to proceed 
	with a connection if the encryption key refresh fails.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP				*hfp;
	/*! Status of the key refresh attempt. */
	hfp_lib_status	status;
} HFP_ENCRYPTION_KEY_REFRESH_IND_T;


/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_SUBSCRIBER_NUMBER_CFM_T;


/*!
	@brief The application will receive this message for each subscriber number sent by the AG in response to a
    request command.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP                    *hfp;
	/*! Service relating to the phone number.*/
	hfp_subscriber_service  service;
	/*! Type of number. */
	hfp_number_type         number_type;
	/*! The number of bytes in the array containing the subscriber number.*/
	uint16                  size_number;
	/*! Array containing the subscriber number.*/
	uint8                   number[1];
} HFP_SUBSCRIBER_NUMBER_IND_T;



/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RESPONSE_HOLD_STATUS_CFM_T;


/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RESPONSE_HOLD_HELD_CFM_T;


/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RESPONSE_HOLD_ACCEPT_CFM_T;


/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_RESPONSE_HOLD_REJECT_CFM_T;


/*!
	@brief The application will receive this message for each change in the AG's Response/Hold state or
	when the application requests the AG's current Response/Hold status.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP		               *hfp;
	/*! AG's response hold state.*/
    hfp_response_hold_state state;
} HFP_RESPONSE_HOLD_STATUS_IND_T;


/*!
	@brief This message informs the application of the outcome of this request
	command sent to the AG.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. An error will also be sent if the application
      supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_CURRENT_CALLS_CFM_T;


/*!
	@brief The application will receive one of these messages for each active call on the AG in response
	to a request to obtain the list of current calls.
*/
typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP                    *hfp;
	/*! Index number of call e.g. for referencing with AT+CHLD commands.*/
    uint16                  call_idx;
    /*! Indicates if the call is AG originated or not.*/
    hfp_call_direction      direction;
    /*! State of the call.*/
    hfp_call_status         status;
    /*! Indicates the mode of the call - bearer/teleservice.*/
    hfp_call_mode           mode;
    /*! Indicates if the call is a multi-party call or not.*/
    hfp_call_multiparty     multiparty;
	/*! Type of number. */
	hfp_number_type         number_type;
    /*! Length of array, in bytes, containing the phone number.*/
    uint16                  size_number;
    /*! Array containing the phone number. */
    uint8                   number[1];
} HFP_CURRENT_CALLS_IND_T;


typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
    HFP    *hfp;
    /*! Network operator selection mode.*/
    uint8   mode;
    /*! Length of operator_name, in bytes.*/
    uint16  size_operator_name;
    /*! Operator name string.*/
    uint8   operator_name[1];
} HFP_NETWORK_OPERATOR_IND_T;

typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
	    responds with ERROR or the profile instance is in the wrong state to send out the
		command the status will be set to hfp_fail. An error will also be sent if the application
		supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_NETWORK_OPERATOR_CFM_T;


typedef struct
{
    /*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP				*hfp;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
		responds with ERROR or the profile instance is in the wrong state to send out the
		command the status will be set to hfp_fail. An error will also be sent if the application
		supplies a profile instance that was initialised as HSP and not HFP.*/
	hfp_lib_status	status;
} HFP_EXTENDED_ERROR_CFM_T;

/*!
	@brief Type of CSR extended feateures supported by the AG.
	
	This message will be received in response to a HfpCsrSupportedFeaturesReq call.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP *hfp;
	/*! Command status. */
	hfp_lib_status status;
	/*! Caller name supported by the AG. */
	bool callerName;
	/*! Unsolicited raw text supported by the AG. */
	bool rawText;
	/*! SMS Indications supported by the AG. */
	bool smsInd;
	/*! Battery level reporting supported by the AG. */
	bool battLevel;
	/*! Power source reporting supported by the AG. */
	bool pwrSource;
	/*! Bit mask of codecs supported by the AG */
	bool codecs;
} HFP_CSR_SUPPORTED_FEATURES_CFM_T;

/*!
	@brief Unsolicited Text 2 Speech text has been received.
	
	The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
	/*! The number of bytes in the text string. */
    uint16  size_text;
	/*! The text that needs to be presented to the user. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   text[1];
} HFP_CSR_TXT_IND_T;

/*! @brief Result of a HfpCsrModifyIndicators or HfpCsrModifyIndicatorsDisable call.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP *hfp;
	/*! Command status. */
	hfp_lib_status status;
} HFP_CSR_MODIFY_INDICATORS_CFM_T;

/*! @brief A new SMS has arrived.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP *hfp;
	/*! SMS Index value. */
	uint16 index;
	/*! The number of bytes pointed to by sender_number.*/
	uint16 size_sender_number;
	/*! Number of the sender of the SMS message. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   sender_number[1];
} HFP_CSR_NEW_SMS_IND_T;

/*! @brief A new SMS has arrived from the named sender.

	The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level Connection.*/
	HFP *hfp;
	/*! SMS Index value. */
	uint16 index;
	/*! The number of bytes pointed to by sender_name.*/
	uint16 size_sender_name;
	/*! ID of the sender of the SMS message. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   sender_name[1];
} HFP_CSR_NEW_SMS_NAME_IND_T;

/*!
	@brief Body of requested text message
	
	The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
	/*! Command status. */
	hfp_lib_status status;
	/*! The number of bytes in the text string. */
    uint16  size_sms;
	/*! Body of the SMS message. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    uint8   sms[1];
} HFP_CSR_SMS_CFM_T;

/*!
	@brief AG has requested that a Battery report be sent.
	
	Use HfpCsrPowerLevel to send the report.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
} HFP_CSR_AG_REQUEST_BATTERY_IND_T;
/*!
	@brief Body of requested text message
	
	The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
	/*! The number of inidicators to change. */
    uint16  size_indicators;
	/*! Body of the SMS message. The client
	  should not attempt to free this pointer, the memory will be freed when
	  the message is destroyed. If the client needs access to this data after
	  the message has been destroyed it is the client's responsibility to copy
	  it. */
    hfp_csr_mod_indicator indicators[1];
} HFP_CSR_MODIFY_AG_INDICATORS_IND_T;

/*!
	@brief Disable all CSR indicators for the AG.
*/
typedef struct
{
	/*! Pointer to hfp profile instance that is handling the Service Level
	  Connection.*/
    HFP     *hfp;
} HFP_CSR_AG_INDICATORS_DISABLE_IND_T;


/*todo*/
typedef struct
{
    HFP * hfp ;
    uint16 indicator ; 
    uint16 value ;
}
HFP_CSR_FEATURE_NEGOTIATION_IND_T ;

/*!
	@brief Initialise an instance of the HFP library.

	@param theAppTask The current application task.

	@param config Profile supported (HSP or HFP) plus any supported features.

	The application registers its own task, theAppTask, with the HFP library so
	that return messages can be routed to the correct task.

	The config parameter is used to configure the profile this HFP instance
	will support (HSP or HFP). If the HFP is being supported the supported
	features should also be supplied. These are passed in as a bit mask, as
	defined in the HFP specification. If the profile instance is being
	configured as HSP the supported features should be set to zero as they are
	not used.

	The application will receive an HFP_INIT_CFM message from the library to
	indicate the success or otherwise of the initialisation.
*/
void HfpInit(Task theAppTask, const hfp_init_params *config);



/*!
	@brief Initate the creation of a Service Level Connection.

	@param hfp The profile instance which will be used to create the
	connection.

	@param bd_addr The address of the remote device (AG) to which the
	connection will be created.

	@param extra_indicators Additional indicators the AG may support.

	The HFP defines three indicators, (service, call and call_setup) that the
	AG must send to the HFP device. However, the GSM specification defines a
	number of other indicators the AG may support. The application can use
	extra_indicators to specify that it wants to be notified whether the AG
	supports a particular indicator and of its index in the CIND response.

	The extra_indicators should be specified as a string with carriage returns
	(\r) separating each indicator. For example, the extra_indicators string
	might looks as follows, "battchg\rroam\rsounder". It is important that
	spaces are not inserted into the string. The application will be notified
	of the indicator's index (if it is supported by the AG) using a
	HFP_EXTRA_INDICATOR_INDEX_IND message.

	If extra indicator reporting is enabled then all indicator updates will be
	passed to the application (using a HFP_EXTRA_INDICATOR_UPDATE_IND message)
	and not just the indicators the application has registered an interest
	in. It is left up to the application to determine whether the indicator
	update is of interest. Most applications will not want to enable this
	functionality, in which case extra_indicators should be set to null.

	The application will receive an HFP_SLC_CONNECT_CFM message from the
	library to indicate the success or otherwise of the connection attempt.

*/
void HfpSlcConnect(HFP *hfp, const bdaddr *bd_addr, const uint8 *extra_indicators);


/*!
	@brief Allow the application to respond to a HFP_SLC_CONNECT_IND message.

	@param hfp The profile instance which will be used to create the
	connection.

	@param response Accept (TRUE) or reject (FALSE) the incomming connection
	attempt.

	@param bd_addr The Bluetooth address of the device being replied to.

	@param extra_indicators Additional indicators the AG may support.

	If the AG initiates an Service Level Connection to the HFP device the
	application will be notified about the incoming connection using a
	HFP_SLC_CONNECT_IND message. The application must respond to the message
	using this function.  The profile instance to which the connection will be
	created is specified by hfp. This must be the same as the profile instance
	in the HFP_SLC_CONNECT_IND message as the AG will be attempting to connect
	to a particular service on the local device.

	The Bluetooth address of the device that initiated to the Service Level
	Connection is passed in using bd_addr. This should be the same as the
	address in the HFP_SLC_CONNECT_IND message.

	The extra_indicators field is the same as the field in HfpInit.

	The application will receive an HFP_SLC_CONNECT_CFM message from the
	library to indicate the success or otherwise of the connection attempt.
*/
void HfpSlcConnectResponse(HFP *hfp, bool response, const bdaddr *bd_addr, const uint8 *extra_indicators);


/*!
	@brief Initiate the disconnection of an Service Level Connection for a
	particular profile instance (hfp).

	@param hfp The profile instance which is to be disconnected.

	The application will receive an HFP_SLC_DISCONNECT_IND message from the
	library to indicate the success or otherwise of the diconnection attempt.
*/
void HfpSlcDisconnect(HFP *hfp);


/*!
	@brief Obtain the the Sink associated with the Service Level Connection for
	a particular profile instance (hfp)

	@param hfp The profile instance which has a current Service Level
	Connection.

	If the profile instance supplied currently does not have an Service Level
	Connection established, the message returned will contain an error status
	and an invalid Sink.

	The application will receive an HFP_SINK_CFM message from the library.
*/
void HfpGetSink(HFP *hfp);


/*!
	@brief Enable/ disable caller id indications from the AG.

	@param hfp The profile instance we wish to alter caller id indications for.

	@param enable Enable(TRUE) or Disable(FALSE)

	A Service Level Connection for the supplied profile instance (hfp) must
	already be established before calling this function. The enable flag
	determines whether a command will be sent to the AG to enable or disable
	caller id notifications. The message returned indicates whether the command
	was recognised by the AG or not. When enabled, every caller id notification
	received from the AG will be sent to the application using a
	HFP_CALLER_ID_IND message. 
	
	The application will receive an HFP_CALLER_ID_ENABLE_CFM message from the
	library.
*/
void HfpCallerIdEnable(HFP *hfp, bool enable);


/*!
	@brief Enable / disable voice recognition function at the AG.

	@param hfp The profile instance we wish to alter voice recognition for.

	@param enable Enable(TRUE) or Disable(FALSE)

	A Service Level Connection for the supplied profile instance (hfp) must
	already be established before calling this function. The enable flag
	determines whether a request will be made to the AG to enable or disable
	its voice recognition function. The message returned indicates whether the
	command was recognised by the AG or not.

	The AG may autonomously notify the HFP device of a change in the state of
	its voice recognition function. This notification will be passed on to the
	application using a HFP_VOICE_RECOGNITION_IND message.

	The application will receive an HFP_VOICE_RECOGNITION_ENABLE_CFM message
	from the library.
*/
void HfpVoiceRecognitionEnable(HFP *hfp, bool enable);


/*!
	@brief Transfer the audio connection either from the AG to the HFP or vice versa.

	@param hfp The profile instance we want the audio paths transfer performed on.
	@param direction The direction we want the audio connection transferred to.
	@param packet_type The packet type to be used on the audio connection.
	@param audio_params Used to specify bandwith, latency, voice setting and retransmission effort for eSCO packet types.

	The direction in which to attempt the audio transfer is specified by
	direction. This can be either from the AG to the headset (hfp_audio_to_hfp), from
	the headset to the AG (hfp_audio_to_ag) or a request to transfer the audio to the
	other device to where it currently is (hfp_audio_transfer) i.e. if the audio
	paths are currently routed to the headset then transfer the audio to the AG
	or vice versa.

	It is the application's responsibility to ensure that the remote device
	supports the requested packet type.

	The application will receive an HFP_AUDIO_CONNECT_CFM or
	HFP_AUDIO_DISCONNECT_IND message from the library depending on the direction
	of audio transfer.
*/
void HfpAudioTransferConnection(HFP *hfp, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, const hfp_audio_params *audio_params);


/*!
	@brief Create an audio connection from HFP to AG.

	@param hfp The profile instance we want the audio connection to be performed on.
	@param packet_type The packet type to be used for the audio connection (SCO/eSCO).
	@param audio_params Used to specify bandwith, latency, voice setting and retransmission effort for eSCO packet types.

	The application will receive a HFP_AUDIO_CONNECT_IND message from the library indicating the
	outcome of the connection request.
*/
void HfpAudioConnect(HFP *hfp, sync_pkt_type packet_type, const hfp_audio_params *audio_params);


/*!
	@brief Accept or reject an incoming audio connection request from the AG.

	@param hfp The profile instance we want the audio connection to be performed on.
	@param response Set to TRUE to accept incoming connection request or FALSE to reject it.
	@param packet_type The packet type to be used for the audio connection (SCO/eSCO).
	@param audio_params Used to specify bandwith, latency, voice setting and retransmission effort for eSCO packet types.

	The library will inform an application of an incoming audio connection request via the
	HFP_AUDIO_CONNECT_IND message.  The application should respond using this function to accept
	or reject the request.  When accepting the connection request the application can specify its
	own set of operating parameters which will be used by the firmware to negotiate a mutually
	acceptable connection.

	The application will receive a HFP_AUDIO_CONNECT_IND message from the library indicating the
	outcome of the connection request.
*/
void HfpAudioConnectResponse(HFP *hfp, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params, bdaddr bd_addr);


/*!
	@brief Disconnect an existing audio connection.

	@param hfp The profile instance we want the audio disconnection to be performed on.

	The application will receive a HFP_AUDIO_DISCONNECT_IND message from the library to indicate the
	outcome of the disconnect request.
*/
void HfpAudioDisconnect(HFP *hfp);


/*!
	@brief Answer an incoming call.

	@param hfp The AT command to answer the call will be sent out on this
	instance's Service Level Connection

	The application will receive an HFP_ANSWER_CALL_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpAnswerCall(HFP *hfp);


/*!
	@brief Reject an incoming call.

	@param hfp The AT command to reject the call will be sent out on this
	instances Service Level Connection

	The application will receive an HFP_REJECT_CALL_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpRejectCall(HFP *hfp);


/*!
	@brief Hang up an active call or terminate an outgoing call process before
	it has been completed.

	@param hfp The AT command to reject the call will be sent out on this
	instances Service Level Connection

	The application will receive an HFP_TERMINATE_CALL_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpTerminateCall(HFP *hfp);


/*!
	@brief Issue a request to the AG to dial the supplied number.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param size_number The length in bytes of the number to be sent.

	@param number The number to dial.

	The number is specified as an array and can include valid dial characters
	such as '+'.

	The application will receive an HFP_DIAL_NUMBER_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpDialNumber(HFP *hfp, uint16 size_number, const uint8 *number);


/*!
	@brief Issue a request to the AG to dial from the the supplied
	memory_location (the HFP specification defines the command but it is AG
	implementation dependent how this is implemented).

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param size_memory_location The length in bytes of the memory_location to
	be sent.

	@param memory_location The location the AG must dial from. The message
	returned indicates whether the command was recognised by the AG or not.

	The application will receive an HFP_DIAL_MEMORY_CFM message from the
	library to indicate whether or not the command was recognised by the AG.

*/
void HfpDialMemoryLocation(HFP *hfp, uint16 size_memory_location, const uint8 *memory_location);


/*!
	@brief Issue a request to the AG to perform a last number redial.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	The application will receive an HFP_LAST_NUMBER_REDIAL_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpLastNumberRedial(HFP *hfp);


/*!
	@brief Request to send the local speaker setting (volume) to the AG.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param volume The volume to use.

	The HFP specification limits the value of volume to be in the range 0-15
	and the Hfp profile library will enforce this.

	The AG may autonomously send volume gain indications to the HFP device, the
	application will be notified of these using the HFP_SPEAKER_VOLUME_IND
	message.

	The application will receive an HFP_SPEAKER_VOLUME_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpSendSpeakerVolume(HFP *hfp, uint16 volume);


/*!
	@brief Request to send the local microphone setting (volume) to the AG.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param volume The volume to use.

	The HFP specification limits the value of volume to be in the range 0-15
	and the Hfp profile library will enforce this.

	The AG may autonomously send microphone gain indications to the HFP device.
	The application will be notified of these using the
	HFP_MICROPHONE_VOLUME_IND message.

	The application will receive an HFP_MICROPHONE_VOLUME_CFM message from the
	library to indicate whether or not the command was recognised by the AG.
*/
void HfpSendMicrophoneVolume(HFP *hfp, uint16 volume);


/*!
	@brief Request a number from the AG to attach to a voice tag.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	The application will receive an HFP_VOICE_TAG_NUMBER_CFM message from the
	library to indicate whether the command was recognised by the AG or not and
	the number supplied by the AG (if any).
*/
void HfpRequestNumberForVoiceTag(HFP *hfp);


/*!
	@brief Request to transmit a DTMF code to the AG.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param dtmf The dtfm character must be one from the set 0-9, #, *, A-D.

	The application will receive an HFP_DTMF_CFM message from the library to
	indicate whether the command was recognised by the AG or not.

*/
void HfpSendDtmf(HFP *hfp, uint8 dtmf);


/*!
	@brief Enable /disable call waiting notification from the AG.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	@param enable Enable(TRUE) or Disable(FALSE) call waiting notifications.

	Once call waiting notifications have been enabled at the AG they will be
	sent (whenever there is a waiting call) until explicitly disabled or until
	the Service Level Connection is disconnected. The AT command to enable /
	disable call waiting notifications will only be sent if both the AG and
	headset claim support for this functionality in their supported features.

	The HFP_CALL_WAITING_IND message will be used to notify the application of
	waiting calls.

	The application will receive an HFP_CALL_WAITING_ENABLE_CFM message from
	the libraryto indicate whether the command was recognised by the AG or not.
*/
void HfpCallWaitingEnableNotification(HFP *hfp, bool enable);


/*!
	@brief Depending on the state of the calls either release all held calls
	or set User Determined User Busy (UDUB) for a waiting call.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=0 command to the AG.

	The application will receive an HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM
	message from the library to indicate whether the command was recognised by
	the AG or not.
*/
void HfpMultipleCallsReleaseHeldOrRejectWaiting(HFP *hfp);


/*!
	@brief Release all active calls (if any) and accept the other (held or
	waiting) call.

  	@param hfp The request will be sent out on this instances Service Level
  	Connection.

	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=1 to the AG.

	The application will receive an HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM
	message from the library to indicate whether the command was recognised by
	the AG or not.
*/
void HfpMultipleCallsReleaseActiveAcceptOther(HFP *hfp);


/*!
	@brief Release all active calls (if any) and accept the other (held or waiting) call.

  	@param hfp The request will be sent out on this instances Service Level Connection.
    @param call_idx

	This function is used when handling multiparty calls. This corresponds to sending an AT+CHLD=1,<call_idx>
	to the AG.

	The application will receive an HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM message from the library
	to indicate whether the command was recognised by the AG or not.
*/
void HfpMultipleCallsReleaseSpecifiedAcceptOther(HFP *hfp, uint16 call_idx);


/*!
	@brief Place all active calls (if any) on hold and accept the other (held
	or waiting) call.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=2 to the AG.

	The application will receive an HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM
	message from the library to indicate whether the command was recognised by
	the AG or not.
*/
void HfpMultipleCallsHoldActiveAcceptOther(HFP *hfp);


/*!
	@brief Place all active calls (if any) on hold and accept the other (held or waiting) call.

	@param hfp The request will be sent out on this instances Service Level Connection.
    @param call_idx

	This function is used when handling multiparty calls. This corresponds to sending an AT+CHLD=2,<call_idx>
	to the AG.

	The application will receive an HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM message from the library to
	indicate whether the command was recognised by the AG or not.
*/
void HfpMultipleCallsRequestPrivateHoldOther(HFP *hfp, uint16 call_idx);


/*!
	@brief Add the held call to the conversation.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=3 to the AG.

	The application will receive an HFP_ADD_HELD_CALL_CFM message from the
	library to indicate whether the command was recognised by the AG or not.
*/
void HfpMultipleCallsAddHeldCall(HFP *hfp);


/*!
	@brief Connect the two external calls and disconnect the subscriber from
	the conversation (Explicit Call Transfer).

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=4 to the AG.

	Support for this is optional for the HF.

	The application will receive an HFP_EXPLICIT_CALL_TRANSFER_CFM message from
	the library to indicate whether the command was recognised by the AG or
	not.
*/
void HfpMultipleCallsExplicitCallTransfer(HFP *hfp);


/*!
	@brief Request the AG to disable its Noise Reduction (NR) / Echo
	Cancellation (EC) functions.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	This function must be called before an audio connection has been
	established.

	The application will receive an HFP_DISABLE_NREC_CFM message from the
	library to indicate whether the command was recognised by the AG or not.
*/
void HfpDisableNrEc(HFP *hfp);


/*!
	@brief Request to send a headset profile compliant button press to the AG.

	@param hfp The request will be sent out on this instances Service Level
	Connection.

	The HFP profile library can also initialise an HSP profile instance that
	provides an implementation of the HSP specification. The HSP defines only a
	single button press AT command (in addition to the volume/ microphone
	control commands).

	Please note that the profile instance passed in must be initialised as HSP
	and not HFP.

	The application will receive an HFP_HS_BUTTON_PRESS_CFM message from the
	library to indicate whether the command was recognised by the AG or not.
*/
void HfpSendHsButtonPress(HFP *hfp);


/*!
	@brief Request to obtain list of subscriber numbers from the AG.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_SUBSCRIBER_NUM_IND message from the library for each subscriber
	number sent by the AG.  In the instance that no subscriber numbers are held by the AG, the application
	will not receive any of these indication messages.  A HFP_SUBSCRIBER_NUM_CFM will be sent to indicate
	that the command has completed.
*/
void HfpGetSubscriberNumbers(HFP *hfp);


/*!
	@brief Requests the current response hold state of the AG.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_RESPONSE_HOLD_STATUS_IND message indicating the AG's current
	response hold state.  A HFP_RESPONSE_HOLD_STATUS_CFM message will be sent to indicate that the
	command has been completed.
*/
void HfpGetResponseHoldStatus(HFP *hfp);


/*!
	@brief Requests that the AG places the current incoming call on hold.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_RESPONSE_HOLD_STATUS_IND message indicating the AG's current
	response hold state.  A HFP_RESPONSE_HOLD_HELD_CFM message will be sent to indicate that the
	command has been completed.
*/
void HfpHoldIncomingCall(HFP *hfp);


/*!
	@brief Requests that the AG accepts the currently held incoming call.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_RESPONSE_HOLD_STATUS_IND message indicating the AG's current
	response hold state.  A HFP_RESPONSE_HOLD_ACCEPT_CFM message will be sent to indicate that the
	command has been completed.
*/
void HfpAcceptHeldIncomingCall(HFP *hfp);


/*!
	@brief Requests that the AG rejects the currently held incoming call.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_RESPONSE_HOLD_STATUS_IND message indicating the AG's current
	response hold state.  A HFP_RESPONSE_HOLD_REJECT_CFM message will be sent to indicate that the
	command has been completed.
*/
void HfpRejectHeldIncomingCall(HFP *hfp);


/*!
	@brief Requests list of current calls from the AG.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_CURRENT_CALL_IND message from the library for each entry in the AG's
	current call list.  In the instance that AG's current call list is empty, the application will not
	receive any of these indication messages.  A HFP_CURRENT_CALL_CFM will be sent to indicate that the
	command has completed.
*/
void HfpGetCurrentCalls(HFP *hfp);


/*!
	@brief Requests the Network Operator selection mode and name from the AG.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_NETWORK_OPERATOR_IND message indicating the AG's current
	network operator mode and name.  A HFP_NETWORK_OPERATOR_CFM message will be sent to indicate that the
	command has been completed.
*/
void HfpGetNetworkOperator(HFP *hfp);


/*!
	@brief Requests that the AG sends out extended AT error reporting codes.

	@param hfp The request will be sent out on this instances Service Level Connection.

	The application will receive a HFP_EXTENDED_ERROR_CFM to indicate the AG has acknowledged this request.
	The AG will then replace any AT "ERROR" indications with "+CME ERROR:<err>" where appropriate.
	Any extended error codes will be reported to the AG via the hfp_lib_status parameter contained in HFP IND
	and CFM messages.
*/
void HfpEnableExtendedErrors(HFP *hfp);


/*! @brief Inform the AG of Supported Features

	@param hfp A pointer to the profile instance.
	@param callerName HF Device supports the presentation of Caller Names in CLIP and CCWA
    @param rawText HF Device supports the presentation of raw text from a CSRTXT
    @param smsInd HF Device supports the receipt of SMS indications
    @param battLevel HF Device will send battery level indications (see HfpCsrPowerLevel)
    @param pwrSource HF Device will send power source indications (see HfpCsrPowerSource)
    @param codecs bitmask of the curpported codecs supported by the HF device 
    This function informs the AG which supported features that the HF supports. A HFP_CSR_SUPPORTED_FEATURES_CFM 
    message, containing the AG's supported features, will be received on completion.
    
    An SLC must be fully established before calling this function.
*/
void HfpCsrSupportedFeaturesReq(HFP *hfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource , uint16 codecs);

/*! @brief Send a power status report to the AG

	@param hfp A pointer to the profile instance.
	@param pwr_level Current battery level from 0 (empty) to 9 (full).
    @param pwr_status Current power source..

    This function informs the AG of the current power state of the HF device.  There is no return message on completion.
    
    An SLC must be fully established before calling this function.
*/
void HfpCsrPowerLevel(HFP *hfp, uint16 pwr_level);

/*! @brief Send a power source report to the AG

	@param hfp A pointer to the profile instance.
    @param pwr_status Current power source..

    This function informs the AG of the current power source of the HF device.  There is no return message on completion.
    
    An SLC must be fully established before calling this function.
*/
void HfpCsrPowerSource(HFP *hfp, hfp_csr_power_status_report pwr_status);

/*! @brief Send modifed indicators to the AG

	@param hfp A pointer to the profile instance.
	@param size_indicators Number of indicators to modify
	@param inidicators Array of indicators to modify
	
	This function informs the AG of a list of indicators to modify.  Indicators 
	ommitted from the list are left the same.  A HFP_CSR_MODIFY_INDICATORS_CFM message will be
	received on completion.
	An SLC must be fully established before calling this function.
*/
void HfpCsrModifyIndicators(HFP *hfp, uint16 size_indicators, const hfp_csr_mod_indicator *indicators);

/*! @brief Send diable indicators command to the AG

	@param hfp A pointer to the profile instance.
	
	This function instructs the AG to disable all indicators.A HFP_CSR_MODIFY_INDICATORS_CFM message will 
	be received on completion.  HfpCsrModifiyIndicators should be used to reenable the indicators when required.
	An SLC must be fully established before calling this function.
*/
void HfpCsrMofifyIndicatorsDisable(HFP *hfp);

/*! @brief Retreive an SMS from the AG.

	@param hfp A pointer to the profile instance.
	@param index Index of the SMS to retreive.
	
	
*/
void HfpCsrGetSms(HFP *hfp, uint16 index);


/*todo*/
void HfpFeatureNegotiationResponse (HFP * hfp , uint16 indicator, uint16 value ) ;


/*!
	@brief Obtain the Sink associated with the Service Level Connection for
	a particular profile instance (hfp)

	@param hfp The profile instance which has a current Service Level
	Connection.

	If the profile instance supplied currently does not have an Service Level
	Connection established, the return value will be 0.

    @returns Sink The SLC sink associated with the profile instance.
*/
Sink HfpGetSlcSink(HFP *hfp);


/*!
	@brief Obtain the audio Sink associated with a particular profile instance (hfp)

	@param hfp The HFP profile instance.

	If the profile instance supplied currently does not have an audio sink,
    the return value will be 0.

    @returns Sink The audio sink associated with the profile instance.
*/
Sink HfpGetAudioSink(HFP *hfp);


#endif /* HFP_H_ */
