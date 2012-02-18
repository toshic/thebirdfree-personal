/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp.h
    
DESCRIPTION
	Header file for the A2DP profile library. The library exposes a functional 
    downstream API and an upstream message based API.
	
*/
/*!
@file	a2dp.h
@brief	Interface to the Advanced Audio Distribution Profile library.		
		
		When a device wishes to start streaming audio content, the device must
		first set up a streaming connection.  During the stream setup, the
		devices select the most suitable audio streaming parameters.
		Application service capability and transport service capability are
		configured during this stream setup procedure.
		
		Once a streaming connection is established and the start streaming
		procedure is executed, audio can be streamed from the Source (SRC) 
        to the Sink (SNK).
		
		This library provides the low level services to permit an audio stream
		to be configured, started, stopped and suspended.  This library
		provides the stream configuration and control.  The actual streaming of
		data is performed by the underlying firmware.  The audio stream is
		routed to the Digital Signal Processor (DSP) present on CSR BlueCore
		Multimedia devices.  The CPU intensive operation of encoding/decoding a
		media stream is performed by the DSP.
	
		The library exposes a functional downstream API and an upstream message
		based API.
*/

#ifndef A2DP_H_
#define A2DP_H_


#include <connection.h>


/*! 
	@name Service Categories.

	These are service categories to be used in service capabilities of a Stream
	End Point (SEP).
	
*/
/*! \{ */

/*!
	@brief The capability to stream media. This is manditory for the Advance Audio
	Distribution Profile.
*/
#define AVDTP_SERVICE_MEDIA_TRANSPORT		(1)		
/*!
	@brief The reporting capability. This is not currently supported.
*/
#define AVDTP_SERVICE_REPORTING				(2)		
/*!
	@brief The recovery capability. This is not currently supported.
*/
#define AVDTP_SERVICE_RECOVERY				(3)		
/*!
	@brief The content protection capability.
*/
#define AVDTP_SERVICE_CONTENT_PROTECTION	(4)		
/*!
	@brief The header compression capability. This is not currently supported.
*/
#define AVDTP_SERVICE_HEADER_COMPRESSION	(5)		
/*!
	@brief The multiplexing capability. This is not currently supported.
*/
#define AVDTP_SERVICE_MULTIPLEXING			(6)		
/*!
	@brief The codec capability for the Stream End Point.
*/
#define AVDTP_SERVICE_MEDIA_CODEC			(7)		
/*! \} */

/*! 
	@name Service information.

	Used to fill out the fields in a media codec capabilities structure.
*/
/*! \{ */

/*!
	@brief Defines the codec type as audio.
*/
#define AVDTP_MEDIA_TYPE_AUDIO				(0)		
/*!
	@brief Defines the codec type as video.
*/
#define AVDTP_MEDIA_TYPE_VIDEO				(1)		
/*!
	@brief Defines the codec type as multimedia.
*/
#define AVDTP_MEDIA_TYPE_MULTIMEDIA			(2)		
/*!
	@brief Defines the codec as SBC. Manditory to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_SBC				(0)		
/*!
	@brief Defines the codec as MPEG1/2. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO		(1)		
/*!
	@brief Defines the codec as AAC. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_MPEG2_4_AAC		(2)		
/*!
	@brief Defines the codec as ATRAC. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_ATRAC				(4)		
/*!
	@brief Defines a codec not supported in the A2DP profile.
*/
#define AVDTP_MEDIA_CODEC_NONA2DP			(0xff)	
/*!
	 @brief SCMS CP_TYPE value for the content protection capabilities (LSB).
*/
#define AVDTP_CP_TYPE_SCMS_LSB				(0x02)
/*!
	 @brief SCMS CP_TYPE value for the content protection capabilities (MSB).
*/
#define AVDTP_CP_TYPE_SCMS_MSB				(0x00)

/*! \} */


/*! 
	@name Role defines when initialising the A2DP library. A device could register Source and Sink service records.
*/
/*! \{ */

/*!
	@brief Bit to indicate that the device supports the Sink role. Use for the role in A2dpInit to register the default service record. 
*/
#define A2DP_INIT_ROLE_SOURCE					(1)		
/*!
	@brief Bit to indicate that the device supports the Source role. Use for the role in A2dpInit to register the default service record. 
*/
#define A2DP_INIT_ROLE_SINK						(2)		


/*! 
	@name CSR Faststream IDs.
*/
/*! \{ */

/*!
	@brief The CSR Vendor ID.
*/
#define A2DP_CSR_VENDOR_ID						(0x0a000000)	

/*!
	@brief The CSR Faststream Codec ID.
*/
#define A2DP_CSR_FASTSTREAM_CODEC_ID			(0x0100)	


struct __A2DP;


/*!
	@brief The Advanced Audio Distribution Profile structure.
*/
typedef struct __A2DP A2DP;

/*!
	@brief The structure holding the information about the Stream End Points available on the device.
*/
typedef struct _device_sep_list device_sep_list;

/*!
	@brief Stream End Point type (source or sink). 

    The Stream End Point type is defined in the AVDTP 
    specification.

*/
typedef enum
{
	/*!  This states the device or Stream End Point takes the Source role. */
	a2dp_source,
	/*!  This states the device or Stream End Point takes the Sink role. */
	a2dp_sink	
} a2dp_role_type;


/*!
	@brief Stream End Point (SEP) Media type. 

    The Media type of a SEP is defined in the Bluetooth assigned numbers 
    document.
*/
typedef enum
{
	sep_media_type_audio,		/*!< Audio.*/
	sep_media_type_video,		/*!< Video.*/
	sep_media_type_multimedia	/*!< Multimedia.*/
} a2dp_sep_media_type;


/*!
    @brief Status code returned in messages from the A2DP library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    a2dp_success,					/*!< The operation succeeded. */
    a2dp_invalid_parameters,		/*!< Invalid parameters supplied by the client. */
    a2dp_sdp_fail,					/*!< SDP registration has failed. */
    a2dp_l2cap_fail,				/*!< L2CAP registration has failed. */
	a2dp_operation_fail,			/*!< The operation has failed. */
	a2dp_insufficient_memory,		/*!< No memory to perform the required task. */
	a2dp_wrong_state,				/*!< The library is in the wrong state to perform the operation. */
	a2dp_no_signalling_connection,	/*!< No signalling connection. */
	a2dp_no_media_connection,		/*!< No media connection. */
	a2dp_rejected_by_remote_device,	/*!< Was rejected by the remote device. */
	a2dp_disconnect_link_loss,		/*!< Link loss occured. */
	a2dp_closed_by_remote_device,	/*!< Closed by remote device. */
	a2dp_aborted,					/*!< Connection was aborted. */	
    a2dp_key_missing                /*!< Remote device missing link key. */
} a2dp_status_code;


/*! 
	@brief Type of content protection in use.
*/
typedef enum
{
	/*! No content protection in use. */
	avdtp_no_protection = (0),
	/*! SCMS-T content protection in use. */
	avdtp_scms_protection
} a2dp_content_protection;


/*! 
	@brief Audio stream channel mode.
	
	The specification defines the following channel modes. The SNK must support
	all modes. It is mandatory for the SRC to support mono and at least one of
	the remaining three modes.
*/
typedef enum
{
	a2dp_mono,									/*!< Mono channel mode. */
	a2dp_dual_channel,							/*!< Dual channel mode. */
	a2dp_stereo,								/*!< Stereo channel mode. */
	a2dp_joint_stereo							/*!< Joint stereo channel mode. */
} a2dp_channel_mode;


/*! 
	@brief Stream End Point (SEP) Information.
	
	Contains details about a local SEP. The information here is constant for the lifetime of the SEP. 
*/
typedef struct
{
	uint8 seid;									/*!< Unique ID for the SEP. */
	uint8 resource_id;							/*!< Resource ID associated with the SEP. If a SEP is configured then all SEPs with the same resource ID will become in use. */
	unsigned media_type:2;						/*!< The media type of the SEP. The a2dp_sep_media_type enum details the possible options. */
	unsigned role:1;							/*!< The role of the SEP. The a2dp_role_type enum details the possible options. */
	unsigned library_selects_settings:1;		/*!< The library_selects_settings is set to TRUE so that the library selects the optimal settings for a codec when initiating a media connection. Setting it to FALSE will allow the application to choose how the SEP should be configured based on the remote SEP capabilities. */
	uint16 flush_timeout;						/*!< The flush timeout for the SEP. Should be set to 0 to use the default timeout. */
	uint16 size_caps;							/*!< The size of the capabilities for the SEP. */
	const uint8 *caps;							/*!< The capabilities for the SEP. These can be taken from one of the default codec capability header files that are supplied by CSR. The service capabilities section of the AVDTP specification details the format of these capabilities. */
} sep_config_type;


/*! 
	@brief Holds the details of one local SEP. 
*/
typedef struct sep_data_type
{
	const sep_config_type *sep_config;			/*!< Pointer to the constant details of the SEP. */
	unsigned in_use:1;							/*!< Used to indicate if the SEP is initially in use. */
} sep_data_type;


/*! 
	@brief Holds details about the configured codec.
	
	These details about the configured codec are returned to the application, so it can supply this information to the audio library.
	The audio library will use this information to configure the DSP and route the audio depending on the codec in use.
*/
typedef struct
{
	uint8 content_protection;					/*!< The content protection in use. */
	uint32 voice_rate;							/*!< The voice rate. */
	unsigned bitpool:8;							/*!< The bitpool value. */
	unsigned format:8;							/*!< The format. */
	uint16 packet_size;							/*!< The packet size. */
} codec_data_type;


/*! 
	@brief Used to register up to 2 service records on initialisation of the A2DP library.
*/
typedef struct
{
	uint16 size_service_record_a;				/*!< Size of the first service record to be registered. */
	const uint8 *service_record_a;					/*!< The first service record to be registered. */
	uint16 size_service_record_b;				/*!< Size of the second service record to be registered. */
	const uint8 *service_record_b;					/*!< The second service record to be registered. */
} service_record_type;


#define A2DP_MESSAGE_BASE	0x6d00

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Library initialisation */
    A2DP_INIT_CFM = A2DP_MESSAGE_BASE,		/* 6d00 */
	A2DP_SIGNALLING_CHANNEL_CONNECT_IND,	/* 6d01 */
	A2DP_SIGNALLING_CHANNEL_CONNECT_CFM,	/* 6d02 */
	A2DP_CONNECT_OPEN_CFM,					/* 6d03 */
	A2DP_CONFIGURE_CODEC_IND,				/* 6d04 */
    A2DP_OPEN_IND,							/* 6d05 */
	A2DP_OPEN_CFM,							/* 6d06 */
	A2DP_CODEC_SETTINGS_IND,				/* 6d07 */
	A2DP_START_IND,							/* 6d08 */
	A2DP_START_CFM,							/* 6d09 */
	A2DP_SUSPEND_IND,						/* 6d0a */
	A2DP_SUSPEND_CFM,						/* 6d0b */
	A2DP_RECONFIGURE_CFM,					/* 6d0c */
	A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND,	/* 6d0d */
	A2DP_CLOSE_IND,							/* 6d0e */
	A2DP_CLOSE_CFM,							/* 6d1f */
	A2DP_ENCRYPTION_CHANGE_IND,				/* 6d10 */
	A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM,	/* 6d11 */

    A2DP_MESSAGE_TOP
} A2dpMessageId;
#endif


/*!
	@brief This message is sent as a response to calling A2dpInit.

	This message indicates the outcome of initialising the library.
*/
typedef struct
{
	/*! Status of the profile initialisation. */	
	a2dp_status_code    status;		
	/*! Pointer to the local Stream End Point list. */	
	device_sep_list		*sep_list;
} A2DP_INIT_CFM_T;


/*!
	@brief This message indicates an incoming AVDTP signalling 
	connection from a remote device.
*/
typedef struct
{
    /*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;				
	/*! The ID for this connection. Must be returned as part of the A2dpConnectSignallingChannelResponse API. */
	uint16				connection_id;
	/*! The Bluetooth address of the connecting device. */
	bdaddr				addr;
} A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T;


/*!
	@brief This message is sent as a response to calling A2dpConnectSignallingChannel, or it is sent as an indication 
	that the remote end has successfully connected an AVDTP signalling channel .
*/
typedef struct
{
    /*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;				
	/*! The current A2DP status. */
	a2dp_status_code	status;
	/*! Sink for the signalling channel. */
	Sink 				sink;
} A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T;


/*!
	@brief This message is sent in response to a call to A2dpConnectOpen.

    This message indicates the outcome of an attempt to create an
    A2DP connection to a remote device.
	If a status indicating success is returned it can be assumed that both signalling and media channels 
    have been opened and the SEP is in the OPEN state (as defined in the specification).
	If the status indicates failure it can be assumed that no signalling or media channels will have been opened.
	If the signalling channel becomes connected but a media Stream End Point can't be configured, then the A2DP library 
	will remove the signalling channel again.
*/
typedef struct
{
    /*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;				
	/*! The current A2DP status. */
	a2dp_status_code	status;
	/*! Sink for the signalling channel. Only valid if the signalling and media is opened successfully. */
	Sink 				signalling_sink;
	/*! Sink for the media transport channel. Only valid if the signalling and media is opened successfully. */
	Sink 				media_sink;
	/*! Local SEID used for this connection. Only set if the opening of the signalling and media succeeded. */
	uint8				seid;
} A2DP_CONNECT_OPEN_CFM_T;


/*!
	@brief This message is sent to indicate that the application must choose how the codec should be configured, 
	based on the remote codec capabilities.
	The application must respond immediately using the A2dpConfigureCodesResponse API. The application must choose the 
	required parameters to use for this codec.

	The codec_service_caps will start with AVDTP_SERVICE_MEDIA_CODEC as the first byte, and then the other codec information follows.
	The format is detailed in the 'Media Codec Capabilities' section of the AVDTP profile specification.
*/
typedef struct
{				
	/*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;	
	/*! The size of the codec capabilities supplied. */
	uint16				size_codec_service_caps;     
   	/*! The codec capabilities of the remote device. */
	uint8				codec_service_caps[1];      		
} A2DP_CONFIGURE_CODEC_IND_T;


/*!
	@brief This message is sent in response to a call to A2dpOpen

    This message indicates the outcome of an attempt to create an
    A2DP connection to a remote device. If a confirm indicating success is 
    received it can be assumed that both signalling and media channels 
    have been opened and the SEP is in the OPEN state (as defined in the
    specification).
*/
typedef struct
{
    /*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;				
	/*! The current A2DP status. */
	a2dp_status_code	status;
	/*! Local SEID used for this connection. Only set if the Open succeeded. */
	uint8				seid;
	/*! Sink for the media transport channel. Only valid if the media is opened successfully. */
	Sink 				media_sink;
} A2DP_OPEN_CFM_T;


/*!
	@brief This message informs the application that a remote device has opened
	a local Stream End Point.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP	*a2dp;					
	/*! Local Stream Endpoint ID. */
	uint8	seid;						
	/*! Sink for the media transport channel. */
	Sink	media_sink;						
} A2DP_OPEN_IND_T;


/*! 
	@brief This message is sent with the parameters for the configured codec. The parameters can be supplied to the Audio library when audio streaming begins.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP					*a2dp;				
	/*! The sampling rate for the PCM hardware. Can be used when calling AudioConnect. */
	uint32      			rate;				
	/*! The channel mode for the audio being streamed. Can be used when calling AudioConnect.*/
	a2dp_channel_mode		channel_mode;		
	/*! The local SEID in use. This is for informational purposes only. */
	uint8					seid;
	/*! The codec parameters to pass into the audio library. Can be used when calling AudioConnect. */
	codec_data_type			codecData;
	/*! Size of the configured capabilities. */
	uint16					configured_codec_caps_size;
	/*! The configured capabilities. They will have the form: [AVDTP_SERVICE_MEDIA_TRANSPORT] [0] [AVDTP_SERVICE_MEDIA_CODEC] [size media codec data] ... etc.*/
	uint8					configured_codec_caps[1];
} A2DP_CODEC_SETTINGS_IND_T;


/*! 
	@brief This message sent to indicate a remote device has successfully issued an AVDTP_START command.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP					*a2dp;				
	/*! The media sink. */
	Sink					media_sink;
} A2DP_START_IND_T;


/*! 
	@brief This message is sent in response to A2dpStart.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP					*a2dp;				
	/*! The status code. */
	a2dp_status_code		status;
	/*! The media sink. */
	Sink					media_sink;
} A2DP_START_CFM_T;


/*! 
	@brief This message sent to indicate a remote device has successfully issued an AVDTP_SUSPEND command.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP					*a2dp;				
	/*! The media sink. */
	Sink					media_sink;
} A2DP_SUSPEND_IND_T;


/*! 
	@brief This message is sent in response to A2dpSuspend.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP					*a2dp;				
	/*! The status code. */
	a2dp_status_code		status;
	/*! The media sink. */
	Sink					media_sink;
} A2DP_SUSPEND_CFM_T;


/*!
	@brief This message sent to indicate the result of trying to reconfigure the stream.
*/
typedef struct
{
    /*! Pointer to A2DP profile instance. */
    const A2DP				*a2dp;	
	/*! The status code. */
	a2dp_status_code		status;
} A2DP_RECONFIGURE_CFM_T;


/*!
	@brief This message indicates the signalling channel has been closed.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP   *a2dp;		
	/*!< Sink for the signalling channel. */
	Sink 	sink;
	/*! The status of this disconnect. */
	a2dp_status_code status;
} A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T;


/*!
	@brief This message informs the application that the media connection has been
	closed.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;		
	/*! The current A2DP status. */
	a2dp_status_code	status;		
	/*! Sink for the media transport channel. */	
    Sink				media_sink;	
} A2DP_CLOSE_IND_T;


/*!
	@brief This message informs the application that the media connection has been
	closed. Sent in response to A2dpClose.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;		
	/*! The current A2DP status. */
	a2dp_status_code	status;		
	/*! Sink for the media transport channel. */	
    Sink				media_sink;	
} A2DP_CLOSE_CFM_T;


/*!
	@brief This is an unsolicited message sent to the application whenever the
	encryption status of the sink owned by this A2DP profile instance changes.

	This message is generated as a result of a procedure initiated by the
	remote device. The application does not have to take any action on
	receiving the message.  However, some applications may choose not to
	proceed with a connection if encyrption is disabled on it (by the remote
	device).
*/
typedef struct
{
	/*! Pointer to a2dp profile instance that is handling the signalling
	  connection.*/
    A2DP	*a2dp;
	/*! Encryption status of this connection, TRUE if encrypted, FALSE
	  otherwise. */
	bool	encrypted;
} A2DP_ENCRYPTION_CHANGE_IND_T;


/*!
	@brief This message informs the application of the capabilities of the currently configured Stream End Point.
*/
typedef struct
{
	/*! Pointer to A2DP profile instance. */
    A2DP				*a2dp;		
	/*! The current A2DP status. */
	a2dp_status_code	status;		
	/*! The size of the returned caps. */	
    uint16				size_caps;	
	/*! The returned caps. The format of these caps will be the same as the Service Capabilities data of the AVDTP_GET_CONFIGURATION response message.
	This is detailed in section 8.9.2 of the AVDTP specification. */	
    uint8				caps[1];	
} A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM_T;


/*!
	@brief Initialise the A2DP library.  

    @param clientTask The client task (usually the application) initialising the library.
	@param role The role of the device as defined in the A2DP specification (SRC or SNK).
    @param service_records The service records to use or NULL.
	@param size_seps The number of Stream End Points supported on this device.
	@param seps The Stream End Points supported on this device.

    The call to A2dpInit initialises the A2dp library. The initialisation function should be called only once.
	
	If the client has supplied one or two service records then these will be used. Alternatively if the service_records 
	parameter is set to NULL then the library can register its default service record(s) based on the role(s) supplied. 
	The role should be a bit mask using the A2DP_INIT_ROLE_SOURCE and A2DP_INIT_ROLE_SINK definitions. If no role or 
	service records are supplied then it is up to the application to register service records on a separate occasion.

	All the SEPs supported on the local device must also be supplied here. The library will allocate 
	memory for this SEP information then pass a pointer back to the application in the A2DP_INIT_CFM
	message. This pointer must then be supplied when new connections are being created so the A2DP 
	library instances knows about all the available SEPs.

	A2DP_INIT_CFM message will be sent to the clientTask indicating the result of the
	library initialisation.
	
	No further library functions should be called until the A2DP_INIT_CFM
	message has been received by the clientTask.
*/
void A2dpInit(Task clientTask, uint16 role, service_record_type *service_records, uint16 size_seps, sep_data_type *seps);


/*!
	@brief Create an A2DP signalling connection to a remote device.  

    @param clientTask The client task (usually the application) initialising the library.
	@param addr The address of the remote device.
	@param sep_list The Stream End Points supported on this device. This pointer should be the one returned in the A2DP_INIT_CFM message.
  
	A2DP_SIGNALLING_CHANNEL_CONNECT_CFM message will be sent to the clientTask indicating the result of the
	connection attempt.
*/
void A2dpConnectSignallingChannel(Task clientTask, const bdaddr *addr, device_sep_list *sep_list);


/*!
	@brief Response to an attempt from a remote device to open an AVDTP signalling channel.

	@param a2dp The profile instance which will be used.
    @param accept Indicates if this end should accept the signalling connection.
	@param connection_id The ID of the connection. The should be the one sent in the A2DP_SIGNALLING_CHANNEL_CONNECT_IND message.
	@param sep_list The Stream End Points supported on this device. This pointer should be the one returned in the A2DP_INIT_CFM message.

	A2DP_SIGNALLING_CHANNEL_CONNECT_CFM message will be sent to the clientTask indicating the result of the
	connection attempt.
*/
void A2dpConnectSignallingChannelResponse(Task clientTask, A2DP *a2dp, bool accept, uint16 connection_id, device_sep_list *sep_list);


/*!
	@brief Change the configuration for the currently open stream.  

	@param a2dp The profile instance which will be used.
	@param size_sep_caps The size of sep_caps.
	@param sep_caps The capabilities to use. The library will only store a pointer to the capabilities, so any memory allocated should remain until the application has received notification that the operation has completed.

    Sends an AVDTP_RECONFIGURE command to the remote device. An A2DP_RECONFIGURE_CFM message will be returned to the application.

    The capabilities passed into this API can only be the Media Codec Capabilities or Content Protection Capabilities as defined in section 8.10 of the AVDTP specification.
    If the Media Codec Capabilities are to be reconfigured, the data passed in for sep_caps must be the same as defined in 8.19.5 (Media Codec Capabilities) of the AVDTP spec.
    If the Content Protection is to be reconfigured, the data passed in for sep_caps must be the same as defined in 8.19.6 (Content Protection Capabilities) of the AVDTP spec.
*/
void A2dpReconfigure(A2DP *a2dp, uint16 size_sep_caps, uint8 *sep_caps);


/*!
	@brief Request a stream connection with a remote device. An AVDTP signalling channel must already exist otherwise this API call will fail.

	@param a2dp The a2dp instance if a signalling connection currently exists, otherwise NULL should be passed in if this is a new connection.
	@param size_local_seids The number of local SEPs to try when opening a media connection.
	@param local_seids The local SEPs to try when opening a media connection. The library will go through each SEP in turn until it finds compatible capabilities on the remote end, or runs out of local SEPs to try.

    An A2DP_OPEN_CFM message will be returned.

*/
void A2dpOpen(A2DP *a2dp, uint16 size_local_seids, uint8 *local_seids);


/*!
	@brief Request a stream connection with a remote device when no AVDTP signalling channel exists.

    @param clientTask The client task (usually the application) initiating the connection.
	@param addr The address of the remote device.
	@param size_local_seids The number of local SEPs to try when opening a media connection.
	@param local_seids The local SEPs to try when opening a media connection. The library will go through each SEP in turn until it finds compatible capabilities on the remote end, or runs out of local SEPs to try.
	@param sep_list The Stream End Points supported on this device. This pointer should be the one returned in the A2DP_INIT_CFM message.

	This API will attempt both a signalling a stream connection to a remote device. An A2DP_CONNECT_OPEN_CFM message will be returned in response to this API.
	If the returned message indictates failure then the application can assume that no signalling or media connection exists to the remote device.
	If the returned message indictates success then the application can assume that both signalling and media connections exists to the remote device.

*/
void A2dpConnectOpen(Task clientTask, const bdaddr *addr, uint16 size_local_seids, uint8 *local_seids, device_sep_list *sep_list);


/*!
	@brief Reponse to an A2DP_CONFIGURE_CODEC_IND message.

	@param a2dp The profile instance which will be used.
	@param accept If these capabilities have been accepted. If they are not accepted the opening of the media will not continue.
	@param size_codec_service_caps The size of codec_service_caps.
	@param codec_service_caps The codec capabilites that the application has chosen to use.

	The A2DP_CONFIGURE_CODEC_IND message will only be sent if the local side is initiating the media stream 
	and the library_selects_settings parameter for the SEP that is selected is set to FALSE. If the library_selects_settings 
	parameter is set to TRUE then the library will choose the best settings for the codec in use.
*/
void A2dpConfigureCodecResponse(A2DP *a2dp, bool accept, uint16 size_codec_service_caps, uint8 *codec_service_caps);


/*!
	@brief Start media streaming.

	@param a2dp The profile instance which will be used.

	Request to start media streaming on previously opened connection.

	An A2DP_START_CFM message is returned.
*/
void A2dpStart(A2DP *a2dp);


/*!
	@brief Suspend media streaming.

	@param a2dp The profile instance which will be used.

	Request to suspend media streaming on previously opened and started connection.

	An A2DP_SUSPEND_CFM message is returned.
*/
void A2dpSuspend(A2DP *a2dp);


/*!
	@brief Close media streaming.

	@param a2dp The profile instance which will be used.

	Request to close media streaming on previously opened connection.

	An A2DP_CLOSE_CFM message is returned.
*/
void A2dpClose(A2DP *a2dp);


/*!
	@brief Disconnect any connected signalling or media channel associated with this library instance.

	@param a2dp The profile instance which will be used.

	Request to close all open signalling and media connections.

	An A2DP_CLOSE_IND message is sent if any media connections are closed.
	An A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND message is sent if any signalling connections are closed.
*/
void A2dpDisconnectAll(A2DP *a2dp);


/*!
	@brief Retrieves the capabilities of the currently configured Stream-End Point.

	@param a2dp The profile instance which will be used.

	This API can be called prior to an reconfigure, so it is known what capabilities the remote end supports.

	An A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM message is returned to the application.
*/
void A2dpGetCurrentSepCapabilities(A2DP *a2dp);


/*!
	@brief Retrieves the current configuration of the configured Stream-End Point.

	@param a2dp The profile instance which will be used.	
    @param size_caps A variable which will be set to the size (in bytes) of the array returned by this function.

    If the a2dp library currently has a SEP configured it will allocate memory for the capabilities
    array and return this to the caller. From that point on it is the caller's responsibility to free
    the memory once they have finished using it. If the function call below fails, a NULL pointer
    will be returned and the size_caps parameter will be set to zero.
*/
uint8 *A2dpGetCurrentSepConfiguration(A2DP *a2dp, uint16 *size_caps);
 

/*!
	@brief Sets the in_use status of a Stream End Point.

	@param sep_list The list of Stream End Points supported on the device. This pointer should be the one returned in the A2DP_INIT_CFM message.
	@param seid The Stream End Point ID.
	@param in_use Set to TRUE so that the SEP can't be configured. Set to FALSE so the SEP can be configured.

	If a Stream End Point is in use then no remote device can connect to that End Point.
	If a Stream End Point is NOT in use then a remote device may be able connect to that End Point, although it will also depend if a remote device is already connected to the End Point, or to an End Point with the same Resource ID.
*/
void A2dpSetSepInUse(device_sep_list *sep_list, uint8 seid, bool in_use);


/*!
	@brief Retrieve the Sink of the A2DP signalling connection. This will be 0 if no connection exists. 

  	@param avrcp The profile instance which will be used.
*/
Sink A2dpGetSignallingSink(A2DP *a2dp);


/*!
	@brief Retrieve the Sink of the A2DP media connection. This will be 0 if no connection exists. 

  	@param avrcp The profile instance which will be used.
*/
Sink A2dpGetMediaSink(A2DP *a2dp);


#endif /* A2DP_H_ */
