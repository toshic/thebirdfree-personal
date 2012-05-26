/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp.h
    
DESCRIPTION 

    Header file for the Audio/Visual Remote Control Profile library.  This
    profile library implements the AVRCP using the services of the AVCTP
    library which is hidden from the Client application by this library
    
    The library exposes a functional downstream API and an upstream message 
    based API.
    
     CLIENT APPLICATION
      |         |
      |    AVRCP Library
      |         |
      |      |
     CONNECTION Library
             |
         BLUESTACK
    
*/
/*!
@file    avrcp.h
@brief    Interface to the Audio Video Remote Control Profile library.

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

/*! \name AVRCP Supported Features Flag Defines

    These flags can be or'd together and used as the supported_features field
    of an avrcp_init_params structure. 
*/
/*! \{ */
/*!
    @brief Setting this flag when the library is initialised indicates that 
    this device implements category 1 commands.
*/
#define AVRCP_CATEGORY_1                    0x01
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 2 commands.
*/
#define AVRCP_CATEGORY_2                    0x02
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 3 commands.
*/
#define AVRCP_CATEGORY_3                    0x04
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 4 commands.
*/
#define AVRCP_CATEGORY_4                    0x08
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements player application settings. Player application
     settings are valid only for Category 1 Target devices.
*/
#define AVRCP_PLAYER_APPLICATION_SETTINGS    0x10
/*!
    @brief Setting this flag when the library is initialised indicates that 
    this device implements Group Navigation. Group Navigation is valid only
    for  Category 1 Target devices.
*/
#define AVRCP_GROUP_NAVIGATION                0x20
/*!
    @brief Setting this flag when the library is initialised indicates that
     this Target device implements Multiple Media Player applications.
     It is valid  only for Category 1 Target devices.
*/
#define AVRCP_MULTIPLE_MEDIA_PLAYERS           0x80
/*! \} */


/*! \name AVRCP Extensions Flag Defines

    These flags can be or'd together and used as the profile_extensions field
    of an avrcp_init_params structure.
*/
/*! \{ */
/*!
    @brief Setting this flag when the library is initialised indicates that
    this device implements the Metadata extensions. If this bit is not set
    in profile_extensions field for target device, AVRCP Lib will act as 
    v1.0 Library.
*/
#define AVRCP_EXTENSION_METADATA            0x01

/*! \} */


/*! \name List of Media Attributes Defines.
    Application must use defined values for Media Attributes while framing the 
    attributes Data for AvrcpGetElementAttributes() and 
    AvrcpGetElementAttributesResponse() APIs.
*/
/*! \{ */
/*!    @brief Title of the media.*/
#define AVRCP_MEDIA_ATTRIBUTE_TITLE     0x01

/*!    @brief Name of the artist.*/
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST    0x02

/*!    @brief Name of the album*/
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM     0x03

/*! @brief Number of the media(e.g. Track number of the CD) */
#define AVRCP_MEDIA_ATTRIBUTE_NUMBER    0x04

/*! @brief Total number of the media (e.g.Total track number of the CD)*/
#define AVRCP_MEDIA_ATTRIBUTE_TOTAL_NUMBER  0x05

/*! @brief Genre*/
#define AVRCP_MEDIA_ATTRIBUTE_GENRE     0x06

/*! @brief Playing Time in milliseconds*/
#define AVRCP_MEDIA_ATTRIBUTE_PLAYING_TIME 0x07

/*! \} */
/*! \name  List of defined Player Application Settings Attributes and Values.    
    Application should use defined values for Player Application Settings
    Attributes and values while framing the attributes and values for the APIs. 

    AvrcpListAppSettingAttributeResponse(),
    AvrcpListAppSettingValue(),
    AvrcpListAppSettingValueResponse(),
    AvrcpGetCurrentAppSettingValue(),
    AvrcpGetCurrentAppSettingValueResponse(),
    AvrcpSetAppSettingValue(),
    AvrcpGetAppSettingAttributeText(),
    AvrcpGetAppSettingAttributeTextResponse() and
    AvrcpGetAppSettingValueText().
*/
/*! \{ */

/*!  @brief Equalizer ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_EQUALIZER      0x01

/*!  @brief Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_REPEAT_MODE    0x02

/*!  @brief Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SHUFFLE        0x03

/*!  @brief Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN           0x04

/*!  @brief OFF status value all Equalizer ON/OFF Attribute ID.*/
#define AVRCP_PLAYER_VALUE_EQUALIZER_OFF      0x01

/*!  @brief ON status value all Equalizer ON/OFF Attribute ID.*/
#define AVRCP_PLAYER_VALUE_EQUALIZER_ON       0x02

/*!  @brief OFF value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_OFF    0x01

/*!  @brief Single Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_SINGLE  0x02

/*!  @brief All Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_ALL     0x03

/*!  @brief Group Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_GROUP   0x04

/*!  @brief OFF value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_OFF         0x01   

/*!  @brief All Track Shuffle value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_ALL         0x02   

/*!  @brief Group Track Shuffle value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_GROUP       0x03   

/*!  @brief OFF value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_OFF        0x01

/*!  @brief All Track value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_ALL        0x02

/*!  @brief Group Track value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_GROUP      0x03

/*! \} */

/*! \name List of AVRCP PDU IDs defined in AVRCP 1.4 specification.
    Application should use these values for requesting or aborting 
    the Continuation response if the last metadata response from 
    the Target device was a fragmented response.
*/  

/*! \{ */

/*!  @brief PDU ID of GetCapabilities Command */
#define AVRCP_GET_CAPS_PDU_ID                       0x10

/*!  @brief ListPlayerApplicationSettingAttributes command PDU ID*/
#define AVRCP_LIST_APP_ATTRIBUTES_PDU_ID            0x11

/*!  @brief ListPlayerApplicationSettingValues command PDU ID */
#define AVRCP_LIST_APP_VALUE_PDU_ID                 0x12

/*!  @brief GetCurrentPlayerApplicationSettingValue command PDU ID*/
#define AVRCP_GET_APP_VALUE_PDU_ID                  0x13

/*!  @brief SetPlayerApplicationSettingValue command PDU ID*/
#define AVRCP_SET_APP_VALUE_PDU_ID                  0x14

/*!  @brief GetPlayerApplicationSettingAttributeText command PDU ID*/
#define AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID         0x15

/*!  @brief GetPlayerApplicationSettingValueText command PDU ID*/
#define AVRCP_GET_APP_VALUE_TEXT_PDU_ID             0x16

/*!  @brief InformDisplayableCharacterSet command PDU ID*/
#define AVRCP_INFORM_CHARACTER_SET_PDU_ID           0x17

/*!  @brief GetElementAttributes command PDU ID*/
#define AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID         0x20

/*! \} */

/*! \name  AVRCP Response and Status values.
    Target application must use the defined avrcp_response_type values while 
    using the Response APIs. AVRCP library uses the avrcp_status_code in the
    confirmation message to the application on completion of requested
    operation.
*/
/*! \{ */

/*! 
    @brief AVRCP responses. Values are defined in AV/C Digital Interface spec.
    Additional library defined responses have been added.
*/
typedef enum
{
    /*! The specified profile is not acceptable. */
    avctp_response_bad_profile = 0x01,
    /*! The request is not implemented. */
    avctp_response_not_implemented = 0x08,    
    /*! The request has been accepted. This response should be used when
        accepting commands    with AV/C command type of CONTROL. */
    avctp_response_accepted = 0x09,            
    /*! The request has been rejected. */
    avctp_response_rejected = 0x0A,            
    /*! The target is in a state of transition. */
    avctp_response_in_transition = 0x0B,    
    /*! A stable response. This response should be used when accepting commands
    with AV/C command type of STATUS. */
    avctp_response_stable = 0x0C,            
    /*! The target devices state has changed. This response should be used 
        when accepting commands    with AV/C command type of NOTIFY. */
    avctp_response_changed = 0x0D,            
    /*! The response is an interim response. This response should be used when
        accepting commands    with AV/C command type of NOTIFY. */
    avctp_response_interim = 0x0F, 
           
    /*! More specific error status responses for rejecting the Meta Data AVC
        commands and Browsing commands are as follows.Error status codes
        defined in the AVRCP1.4 specification can be retrieved by masking msb
        (0x80) of the defined response codes here. Ensure to keep the same values
        while inserting or modifying following enum values.  
    */

       /*! The request has been rejected if TG received a PDU that it did not 
        understand. This is valid for all Command Responses*/
    avrcp_response_rejected_invalid_pdu = 0x80,

    /*! The request has been rejected with reason - invalid parameter.
        If the TG received a PDU with a parameter ID that it did not 
        understand.
        Send if there is only one parameter ID in the PDU. Valid for all
        Commands.
    */
    avrcp_response_rejected_invalid_param, /* 0x80 + 0x01 */

    /*! The request has been rejected with reason - invalid content.
       Send if the parameter ID is understood, but content is wrong or 
       corrupted.
       Valid for all commands.*/
    avrcp_response_rejected_invalid_content, /* 0x80 + 0x2 */

    /*! The request has been rejected with reason - internal error.
        Valid for all commands*/
    avrcp_response_rejected_internal_error, /* 0x80 + 0x03*/

    /*! The request has been rejected with reason - UID Changed. 
        The UIDs on the device have changed */
    avrcp_response_rejected_uid_changed = 0x85, /* 0x80 + 0x05 */

    /*!  The request has been rejected with reason -  Invalid Direction. 
         The Direction parameter is invalid. Response code is valid only for 
         ChangePath command */
    avrcp_response_rejected_invalid_direction = 0x87, /* 0x80 + 0x07 */

    /*! The request has been rejected with reason - Not a Directory. 
        The UID provided does not refer to a folder item. Response code is
        valid only for ChangePath command */
    avrcp_response_rejected_not_directory, /*0x80 + 0x08 */

    /*! The request has been rejected with reason - Does not exist.
        The UID provided does not refer to any currently valid Item. 
        This response code is valid for commands - Change Path, PlayItem, 
        AddToNowPlaying, GetItemAttributes */
    avrcp_response_rejected_uid_not_exist, /*0x80 + 0x09 */

    /*! The request has been rejected with reason - Invalid Scope.
        The scope parameter is invalid. This response code is valid for 
        commands
        - GetFolderItems, PlayItem, AddToNowPlayer, GetItemAttributes.
    */
    avrcp_response_rejected_invalid_scope, /*0x80 + 0x0A */

    /*! The request has been rejected with reason - Range Out of Bounds 
        The start of range provided is not valid. This response is valid for 
        GetFolderItems command.*/
    avrcp_response_rejected_out_of_bound, /* 0x80 + 0x0B */

    /*! The request has been rejected with reason - UID is a Directory.
        The UID provided refers to a directory, which cannot be handled by
        this media player. This response is valid for commands - PlayItem and
        AddToNowPlaying
    */
    avrcp_response_rejected_uid_directory, /*0x80 + 0x0C */

    /*! The request has been rejected with reason - Media in Use.
        The media is not able to be used for this operation at this time.
        This response is valid for commands - PlayItem and AddToNowPlaying.
     */
    avrcp_response_rejected_media_in_use, /*0x80 + 0x0D */

    /*! The request has been rejected with reason - Now Playing List Full.
        No more items can be added to the Now Playing List.
        This response is valid for command - AddToNowPlaying
    */
    avrcp_response_rejected_play_list_full, /*0x80 + 0x0E */

    /*! This request has been rejected with reason - Search Not Supported.
        The Browsed Media Player does not support search.
        This response is valid for command - Search.
    */
    avrcp_response_rejected_search_not_supported, /*0x80 + 0x0F*/

    /*! This request has been rejected with reason - Search in Progress
        A search operation is already in progress.
        This response is valid for command - Search
    */
    avrcp_response_rejected_search_in_progress, /*0x80 + 0x10*/

    /*! This request has been rejected with reason - Invalid Player Id
        The specified Player Id does not refer to a valid player.
        This response is valid for commands - SetAddressedPlayer and
        SetBrowsedPlayer
    */
    avrcp_response_rejected_invalid_player_id, /*0x80 + 0x11*/

    /*! This request has been rejected with reason - Player Not Browsable
        The Player Id supplied refers to a Media Player which does not 
        support browsing. This response is valid for commands - 
        SetBrowsedPlayer
    */
    avrcp_response_rejected_player_not_browsable, /*0x80 + 0x12*/

    /*! This request has been rejected with reason - Player Not Addressed.
        The Player Id supplied refers to a player which is not currently
        addressed, and the command is not able to be performed if the player
        is not set as addressed. This response is valid for commands - 
        Search and SetBrowsedPlayer.
    */
    avrcp_response_rejected_player_not_addressed, /*0x80 + 0x13*/

    /*! This request has been rejected with reason - No valid Search Results.
        The Search result list does not contain valid entries, e.g. after 
        being invalidated due to change of browsed player - This response is
        valid for GetFolderItems
    */
    avrcp_response_rejected_no_valid_search_results, /*0x80 + 0x14*/

    /*! This request has been rejected with reason - No available players */
    avrcp_response_rejected_no_available_players, /*0x80 + 0x15*/

    /*! This request has been rejected with reason - Addressed Player Changed.
        This is valid for command - Register Notifications*/
    avrcp_response_rejected_addressed_player_changed, /*0x80 + 0x16 */ 

    /* Dummy Place Holder */
    avrcp_response_guard_reserved = 0xFF

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
    /*! The operation was rejected. */
    avrcp_rejected=0x0A,
    /*! Operation was successful, but have only received an interim response.*/
    avrcp_interim_success=0x0F,

    /* Below status codes depends on the error status code received from the 
       remote device. Retain the same values while inserting new values or
       modifying this enum */

    /*! The operation was rejected with reason - invalid PDU. */
    avrcp_rejected_invalid_pdu = 0x80,
    /*! The operation was rejected with reason - invalid parameter. */
    avrcp_rejected_invalid_param,
    /*! The operation was rejected with reason - invalid content. */
    avrcp_rejected_invalid_content,
    /*! The operation was rejected with reason - internal error. */
    avrcp_rejected_internal_error,

    /*! The operation was rejected with reason - UID Changed. */
    avrcp_rejected_uid_changed = 0x85, 
    /*! The command has been rejected with reason -Invalid Direction.*/
    avrcp_rejected_invalid_direction = 0x87, 
    /*! The command has been rejected with reason -Not a Directory.*/
    avrcp_rejected_not_directory, 
    /*! The command has been rejected with reason -Does not exist.*/
    avrcp_rejected_uid_not_exist, 
    /*! The command has been rejected with reason -Invalid Scope.*/
    avrcp_rejected_invalid_scope, 
    /*! The command has been rejected with reason - Range Out of Bounds.*/
    avrcp_rejected_out_of_bound, 
    /*! The command has been rejected with reason - UID is a Directory.*/
    avrcp_rejected_uid_directory, 
    /*! The command has been rejected with reason - Media in Use.*/
    avrcp_rejected_media_in_use, 
    /*! The command has been rejected with reason - Now Playing List Full.*/
    avrcp_rejected_play_list_full, 
    /*! The command has been rejected with reason - Search Not Supported.*/
    avrcp_rejected_search_not_supported, 
    /*! The command has been rejected with reason - Search in Progress.*/
    avrcp_rejected_search_in_progress, 
    /*! This command has been rejected with reason - Invalid Player ID.*/
    avrcp_rejected_invalid_player_id, 
    /*! This command has been rejected with reason - Player Not Browsable.*/
    avrcp_rejected_player_not_browsable,
    /*! This command has been rejected with reason - Player Not Addressed.*/
    avrcp_rejected_player_not_addressed, 
    /*! This command has been rejected with reason - No valid Search Results.*/
    avrcp_rejected_no_valid_search_results, 
    /*! This command has been rejected with reason - No available players.*/
    avrcp_rejected_no_available_players, 
    /*! This command has been rejected with reason - Addressed Player Changed.*/
    avrcp_rejected_addressed_player_changed,

    /* Dummy Place Holder */
    avrcp_status_guard_reserverd = 0xFF
} avrcp_status_code;


/*! \} */
/*! 
    @brief Operation ID, used to identify operation. See table 9.21 AV/C Panel
    Subunit spec. 1.1 #
*/
typedef enum
{
    opid_select                = (0x0),
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
    /* 0x0E to 0x1F Reserved */
    opid_0                    = (0x20),
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
    /* 0x2D - 0x2F Reserved */
    opid_channel_up            = (0x30),
    opid_channel_down,
    opid_sound_select,
    opid_input_select,
    opid_display_information,
    opid_help,
    opid_page_up,
    opid_page_down,
    /* 0x39 - 0x3F Reserved */
    opid_power                = (0x40),
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
    /* 0x4D - 0x4F Reserved */
    opid_angle                = (0x50),
    opid_subpicture,
    /* 0x52 - 0x70 Reserved */
    opid_f1                    = (0x71),
    opid_f2,
    opid_f3,
    opid_f4,
    opid_f5,
    opid_vendor_unique        = (0x7E)
    /* Ox7F Reserved */
} avc_operation_id; 


/*!
    @brief Subunit types 
*/
typedef enum
{
    subunit_monitor            = (0x0),
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
    /* 0x0C - 0x1B Reserved */
    subunit_vendor_unique    = (0x1C),
    subunit_reserved_for_all,
    subunit_extended,
    subunit_unit
} avc_subunit_type;


/*!
    @brief AVRCP device type

    The AVRCP library can be configured to be either a target or a controller
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
    @brief AVRCP Metadata transfer capability ID.

    The capability ID type for capabilities supported by the target. 
*/
typedef enum
{
    avrcp_capability_company_id = 0x02,
    avrcp_capability_event_supported = 0x03
} avrcp_capability_id;


/*!
    @brief AVRCP Metadata transfer event IDs.

    The specification mandates the TG to support a number of event IDs.
    Optionally it may also support a number of other events not defined 
    by the specification. This type covers all events defined by the 
    Metadata transfer specification.
*/
typedef enum 
{
    avrcp_event_playback_status_changed = 0x01,
    avrcp_event_track_changed,                      /* 0x02 */
    avrcp_event_track_reached_end,                  /* 0x03 */
    avrcp_event_track_reached_start,                /* 0x04 */
    avrcp_event_playback_pos_changed,               /* 0x05 */
    avrcp_event_batt_status_changed,                /* 0x06 */
    avrcp_event_system_status_changed,              /* 0x07 */
    avrcp_event_player_app_setting_changed,         /* 0x08 */
    avrcp_event_now_playing_content_changed,        /* 0x09 */
    avrcp_event_available_players_changed,          /* 0x0A */
    avrcp_event_addressed_player_changed,           /* 0x0B */
    avrcp_event_uids_changed,                       /* 0x0C */
    avrcp_event_volume_changed                      /* 0x0D */
} avrcp_supported_events;


/*!
    @brief AVRCP play status events.

    Possible values of play status.
*/
typedef enum 
{
    avrcp_play_status_stopped = 0x00,
    avrcp_play_status_playing = 0x01,
    avrcp_play_status_paused = 0x02,
    avrcp_play_status_fwd_seek = 0x03,
    avrcp_play_status_rev_seek = 0x04,
    avrcp_play_status_error = 0xFF
} avrcp_play_status;


/*!
    @brief AVRCP battery status events.

    Possible values of battery status.
*/
typedef enum 
{
    avrcp_battery_status_normal = 0x00,
    avrcp_battery_status_warning,
    avrcp_battery_status_critical,
    avrcp_battery_status_external,
    avrcp_battery_status_full_charge
} avrcp_battery_status;


/*!
    @brief AVRCP system status events.

    Possible values of system status.
*/
typedef enum 
{
    avrcp_system_status_power_on = 0x00,
    avrcp_system_status_power_off,
    avrcp_system_status_unplugged
} avrcp_system_status;


/*!
    @brief AVRCP character sets.

    character set values defined in IANA character set document available at 
    http://www.iana.org/assignments/character-sets
*/
typedef enum
{
    avrcp_char_set_ascii = 3,
    avrcp_char_set_iso_8859_1 = 4,
    avrcp_char_set_jis_x0201 = 15,
    avrcp_char_set_shift_jis = 17,
    avrcp_char_set_ks_c_5601_1987 = 36,
    avrcp_char_set_utf_8 = 106,
    avrcp_char_set_ucs2 = 1000,
    avrcp_char_set_utf_16be = 1013,
    avrcp_char_set_gb2312 = 2025,
    avrcp_char_set_big5 = 2026
} avrcp_char_set;

/*!
    @brief AVRCP initialisation parameters

    The initialisation parameters allow the profile instance to be configured
    either as a controller or target. 
*/

typedef struct
{
    /*! Specifies if this device is controller (CT), target (TG), or both. */ 
    avrcp_device_type device_type;

    /*! The supported controller features must be filled in if the device 
        supports the controller (CT) role or the library will default to a 
        possibly undesired default setting. The features for the CT must state
        which categories are supported (1 - 4). See the AVRCP Supported Features 
        Flag Defines at the top of avrcp.h. This value reflects the Supported
        features attribute value in the SDP service record of controller. */
    uint8 supported_controller_features;

    /*! The supported target features must be filled in if the device supports
        the target (TG) role or the library will default to a possibly undesired
        default setting. The features for the TG must state which categories are 
        supported (1 - 4),and can also indicate support for the Player 
        Application settings and Group Navigation. 
        See the AVRCP Supported Features Flag Defines at the top of avrcp.h. */
    uint8 supported_target_features;

    /*! Set to zero if no extensions are supported in the Target application. 
        If this bit is not set, library acts as v1.0 . If
        supported_target_features sets the Player Application settings or Group 
        Navigation bits, this value must be set.
        If extensions are supported (eg. AVRCP Metadata extensions), 
        use the AVRCP Extensions Flag Defines from the top of avrcp.h. 
     */
    uint8 profile_extensions;

} avrcp_init_params;


/*!
    @brief AV/C protocol - Used to form the targets address
*/
typedef uint16 avc_subunit_id;


/*! This flag enables the Deprecated part of APIs and Code */
#define AVRCP_ENABLE_DEPRECATED 1

/*
    Do not document this part.
*/
#ifndef DO_NOT_DOCUMENT

/* 
    Upstream AVRCP library messages base. Do not document this.
*/
#define AVRCP_MESSAGE_BASE    0x6B00


typedef enum
{
    /* Library initialisation */
    AVRCP_INIT_CFM = AVRCP_MESSAGE_BASE,        /* 0x6B00 */
            
    /* Connection/ disconnection management */
    AVRCP_CONNECT_CFM,                          /* 0x6B01 */
    AVRCP_CONNECT_IND,                          /* 0x6B02 */
    AVRCP_DISCONNECT_IND,                       /* 0x6B03 */
    
    /* AV/C Specific */
    AVRCP_PASSTHROUGH_CFM,                      /* 0x6B04 */
    AVRCP_PASSTHROUGH_IND,                      /* 0x6B05 */
    AVRCP_UNITINFO_CFM,                         /* 0x6B06 */
    AVRCP_UNITINFO_IND,                         /* 0x6B07 */
    AVRCP_SUBUNITINFO_IND,                      /* 0x6B08 */
    AVRCP_SUBUNITINFO_CFM,                      /* 0x6B09 */
    AVRCP_VENDORDEPENDENT_CFM,                  /* 0x6B0A */
    AVRCP_VENDORDEPENDENT_IND,                  /* 0x6B0B */

    /* AVRCP Metadata transfer extension */
    AVRCP_GET_CAPS_CFM,                         /* 0x6B0C */
    AVRCP_GET_CAPS_IND,                         /* 0x6B0D */
    AVRCP_LIST_APP_ATTRIBUTE_CFM,               /* 0x6B0E */
    AVRCP_LIST_APP_ATTRIBUTE_IND,               /* 0x6B0F */
    AVRCP_LIST_APP_VALUE_IND,                   /* 0x6B10 */
    AVRCP_LIST_APP_VALUE_CFM,                   /* 0x6B11 */
    AVRCP_GET_APP_VALUE_CFM,                    /* 0x6B12 */
    AVRCP_GET_APP_VALUE_IND,                    /* 0x6B13 */
    AVRCP_SET_APP_VALUE_CFM,                    /* 0x6B14 */
    AVRCP_SET_APP_VALUE_IND,                    /* 0x6B15 */
    AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM,           /* 0x6B16 */
    AVRCP_GET_APP_ATTRIBUTE_TEXT_IND,           /* 0x6B17 */
    AVRCP_GET_APP_VALUE_TEXT_CFM,               /* 0x6B18 */
    AVRCP_GET_APP_VALUE_TEXT_IND,               /* 0x6B19 */
    AVRCP_GET_ELEMENT_ATTRIBUTES_CFM,           /* 0x6B1A */
    AVRCP_GET_ELEMENT_ATTRIBUTES_IND,           /* 0x6B1B */
    AVRCP_GET_PLAY_STATUS_CFM,                  /* 0x6B1C */
    AVRCP_GET_PLAY_STATUS_IND,                  /* 0x6B1D */
    AVRCP_REGISTER_NOTIFICATION_CFM,            /* 0x6B1E */
    AVRCP_REGISTER_NOTIFICATION_IND,            /* 0x6B1F */
    AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND,    /* 0x6B20 */
    AVRCP_EVENT_TRACK_CHANGED_IND,              /* 0x6B21 */
    AVRCP_EVENT_TRACK_REACHED_END_IND,          /* 0x6B22 */
    AVRCP_EVENT_TRACK_REACHED_START_IND,        /* 0x6B23 */
    AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND,       /* 0x6B24 */
    AVRCP_EVENT_BATT_STATUS_CHANGED_IND,        /* 0x6B25 */
    AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND,      /* 0x6B26 */
    AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND, /* 0x6B27 */
    AVRCP_REQUEST_CONTINUING_RESPONSE_CFM,      /* 0x6B28 */
    AVRCP_ABORT_CONTINUING_RESPONSE_CFM,        /* 0x6B29 */
    AVRCP_NEXT_GROUP_CFM,                       /* 0x6B2A */
    AVRCP_NEXT_GROUP_IND,                       /* 0x6B2B */
    AVRCP_PREVIOUS_GROUP_CFM,                   /* 0x6B2C */
    AVRCP_PREVIOUS_GROUP_IND,                   /* 0x6B2D */
    AVRCP_INFORM_BATTERY_STATUS_CFM,            /* 0x6B2E */
    AVRCP_INFORM_BATTERY_STATUS_IND,            /* 0x6B2F */
    AVRCP_INFORM_CHARACTER_SET_CFM,             /* 0x6B30 */
    AVRCP_INFORM_CHARACTER_SET_IND,             /* 0x6B31 */
    AVRCP_GET_SUPPORTED_FEATURES_CFM,           /* 0x6B32 */
    AVRCP_GET_EXTENSIONS_CFM,                   /* 0x6B33 */
    AVRCP_SET_ABSOLUTE_VOLUME_IND,              /* 0x6B34 */
    AVRCP_SET_ABSOLUTE_VOLUME_CFM,              /* 0x6B35 */
    AVRCP_EVENT_VOLUME_CHANGED_IND,             /* 0x6B36 */

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
    AVRCP                *avrcp;        
    /*! The current AVRCP status. */    
    avrcp_status_code    status;        
} AVRCP_INIT_CFM_T;

/*!
    @brief This message is generated as a result of a call to AvrcpConnect.
*/
typedef struct
{
    AVRCP                *avrcp;  /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /*!< The current AVRCP status. */
    Sink                 sink;    /*!< Connection handle */    
} AVRCP_CONNECT_CFM_T;

/*!
    @brief This message indicates that a remote device wishes to connect.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP        *avrcp;                
    /*! The Bluetooth Device Address of device connecting */
    bdaddr        bd_addr;            
    /*! Connection identifier */    
    uint16        connection_id;        
} AVRCP_CONNECT_IND_T;


/*!
    @brief This message indicates that the link has disconnected.
*/
typedef struct
{
    AVRCP                *avrcp;  /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /*!< The current AVRCP status. */
    Sink                 sink;    /*!< Connection handle */    
} AVRCP_DISCONNECT_IND_T;

/*!
    @brief This message is generated as a result of a call to AvrcpPassthrough.
*/
typedef struct
{
    AVRCP                *avrcp;   /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;   /*!< The current AVRCP status. */
    Sink                sink;      /*!< Connection handle */    
} AVRCP_PASSTHROUGH_CFM_T;


/*!
    @brief This message indicates that the CT device has send a Passthrough
    command
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;                        

#ifdef AVRCP_ENABLE_DEPRECATED
   /*! This field has be deprecated */
    uint16                 transaction;                
    /*! This field has been deprecated */
    uint16                 no_packets;  
#endif

    /*! The sink. */
    Sink                sink;                        
    /*! AV/C protocol - Used to form the targets address.*/
    avc_subunit_type     subunit_type;                
    /*! The subunit identifier. */
    avc_subunit_id         subunit_id;                    
    /*! Identifies the button pressed. */
    avc_operation_id     opid;                        
    /*! Indicates the user action of pressing or releasing the button
      identified by opid.  Active low.*/
    bool                 state;                        
    /*! Length of following operation_data */
    uint16                size_op_data;                
    /*! The op_data field is required for the Vendor Unique operation. For
      other operations, op_data length and data fields should be zero. The
      client should not attempt to free this pointer, the memory will be freed
      when the message is destroyed. If the client needs access to this data
      after the message has been destroyed it is the client's responsibility to
      copy it. */
    uint8                op_data[1]; 
} AVRCP_PASSTHROUGH_IND_T;


/*!
    @brief This message is generated as a result of a call to AvrcpUnitInfo.

    Only valid when the result is avrcp_res_success 
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                
    /*! The current AVRCP status. */
    avrcp_status_code    status;                
    /*! The sink. */
    Sink                sink;                
    /*! The unit type. */
    avc_subunit_type    unit_type;            
    /*! The uint. */
    uint16                unit;                
    /*! The company identifier. */    
    uint32                company_id;            
} AVRCP_UNITINFO_CFM_T;


/*!
    @brief This message indicates that a remote device is requesting unit
    information.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                
    /*! The sink */    
    Sink                 sink;                
} AVRCP_UNITINFO_IND_T;


/*!
    @brief This message is generated as a result of a call to AvrcpSubUnitInfo
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                            
    /*! The current AVRCP status. */
    avrcp_status_code    status;                            
    /*! Connection handle */
    Sink                sink;                            
    /*! Requested page on the target device. */
    uint8                page;                            
    /*! Four entries from the subunit table for the requested page on the
      target device.*/    
    uint8                page_data[PAGE_DATA_LENGTH];
} AVRCP_SUBUNITINFO_CFM_T;



/*!
    @brief This message indicates that a remote device is requesting subunit
    information.
*/
typedef struct
{
    AVRCP                *avrcp;    /*!< Pointer to avrcp profile instance. */
    Sink                sink;       /*!< Connection handle */
    uint8                page;      /*!< Requested page. */    
} AVRCP_SUBUNITINFO_IND_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpVendorDependent.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;       
    /*! The current AVRCP status. */
    avrcp_status_code    status;    
    /*! Connection handle */
    Sink                sink;  

#ifdef AVRCP_ENABLE_DEPRECATED    
    /*! Response from remote end if applicable. "status" field 
        gives the respective status for the received response. It is not 
        recommended to use this field since this has be deprecated and 
        will be removed in future.
     */     
    uint8               response; 
#endif  
} AVRCP_VENDORDEPENDENT_CFM_T;

/*!
    @brief This message indicates that a remote device is requesting vendor
    dependant information.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;                    

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! The transaction. This field will be deprecated */
    uint16                transaction;            
    /*! This field has been deprecated.*/
    uint16                no_packets;        
#endif             

    /*! The subunit type. */
    avc_subunit_type    subunit_type;            
    /*! The subunit identifier. */
    avc_subunit_id        subunit_id;                
    /*! The company identifier. */
    uint32                company_id;                
    /*! The command type. */
    uint8               command_type;           
    /*! The sink. */
    Sink                sink;                    
    /*! The length of op_data. */
    uint16                size_op_data;            
    /*! The operation data. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */    
    uint8                op_data[1];    
} AVRCP_VENDORDEPENDENT_IND_T;


/*!
    @brief This message indicates the outcome of the request to get the
    supported capabilities of the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP               *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code   status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The type of the capability returned (only valid if status indicates 
        success). Only valid for first packet in fragmented response.*/
    avrcp_capability_id caps;
    /*! The total number of capabilities returned. Only valid for first 
       Metadata packet in a fragmented response. */
    uint16              number_of_caps;
    /*! The size, in bytes, of the list of supported capabilities in 
        this packet (only valid if status indicates success). */
    uint16              size_caps_list; 
    /*! The list of supported capabilities (only valid if status indicates 
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source              caps_list;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif


} AVRCP_GET_CAPS_CFM_T; 


/*!
    @brief This message indicates that a request for the supported
    capabilities has been received from the remote device.
   
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The type of the capability requested. */
    avrcp_capability_id caps;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated */
    uint16              transaction;
#endif

} AVRCP_GET_CAPS_IND_T;


/*!
    @brief This message contains the player application settings attributes
    of the  remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first packet 
        in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success). The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_LIST_APP_ATTRIBUTE_CFM_T;


/*!
    @brief This message indicates that a request for the player application 
    settings attributes has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The transaction. */
    uint16              transaction;
} AVRCP_LIST_APP_ATTRIBUTE_IND_T;


/*!
    @brief This message contains the entire player application settings
    values supported by the  remote device, for the attribute requested.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16 metadata_packet_type;
    /*! The total number of values returned. Only valid for first packet 
        in fragmented response. */
    uint16 number_of_values;
    /*! The size, in bytes, of the list of supported values */
    uint16 size_values;
    /*! The list of supported values (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it has
        finished processing the Source data. */
    Source values;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_LIST_APP_VALUE_CFM_T;


/*!
    @brief This message indicates that a request for the supported player
    application settings  values, for the specific attribute, has been 
    received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The player application setting attribute ID for which the possible 
        values should be returned. */
    uint16 attribute_id;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* This field will be deprecated */
    uint16 transaction;
#endif
} AVRCP_LIST_APP_VALUE_IND_T;


/*!
    @brief This message contains the current player application settings 
    values of the remote device for each attribute requested.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16 metadata_packet_type;
    /*! The total number of values returned. Only valid for first packet 
        in fragmented response. */
    uint16 number_of_values;
    /*! The size, in bytes, of the list of supported values */
    uint16 size_values;
    /*! The list of supported values (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source values;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_APP_VALUE_CFM_T;


/*!
    @brief This message indicates that a request for the current player 
    application settings  values has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes for which values are requested.
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it 
        has finished processing the Source data.*/
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_GET_APP_VALUE_IND_T;


/*!
    @brief This message is confirmation of setting the player application 
    settings values of the  remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! The transaction. */
    uint16 transaction;
#endif
} AVRCP_SET_APP_VALUE_CFM_T;


/*!
    @brief This message indicates that a request to set the player application
   settings  values has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after 
        it has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_SET_APP_VALUE_IND_T;


/*!
    @brief This message is confirmation of retrieving the attribute text of the 
    remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first 
        packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16               data_offset;
#endif

} AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T;


/*!
    @brief This message indicates that a request to get attribute text has 
    been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* The total number of attributes returned. Only valid for first packet
     in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates 
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;


#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16                transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16                data_offset;
#endif
} AVRCP_GET_APP_ATTRIBUTE_TEXT_IND_T;


/*!
    @brief This message is confirmation of retrieving the value text of the 
    remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* The total number of attributes returned. Only valid for first packet 
       in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_GET_APP_VALUE_TEXT_CFM_T;


/*!
    @brief This message indicates that a request to get value text has been
     received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! Attribute ID to retrieve values for. */
    uint16                attribute_id;
    /*! The total number of attributes returned. Only valid for first packet
       in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates 
        success). The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_APP_VALUE_TEXT_IND_T;


/*!
    @brief This message contains the attributes of the element requested.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first packet
        in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_T;


/*!
    @brief This message indicates that a request to get element attributes
    has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* Top 4 bytes of identifier to identify an element on TG */
    uint32                identifier_high;
    /* Bottom 4 bytes of identifier to identify an element on TG */
    uint32                identifier_low;
    /* The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_ELEMENT_ATTRIBUTES_IND_T;


/*!
    @brief This message contains the current play status of the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The total length of the playing song in milliseconds. */
    uint32 song_length;
    /*! The current position of the playing song in milliseconds elapsed. */
    uint32 song_elapsed;
    /*! Current status of playing media. */
    avrcp_play_status play_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
#endif

} AVRCP_GET_PLAY_STATUS_CFM_T;


/*!
    @brief This message indicates that a request to get the current play 
    status been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated */
    uint16 transaction;
#endif
} AVRCP_GET_PLAY_STATUS_IND_T;


/*!
    @brief This message contains the confirmation of the register notification
    event.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The event that the device asked to be notified of changes. */
    avrcp_supported_events event_id;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REGISTER_NOTIFICATION_CFM_T;


/*!
    @brief This message indicates the remote device has requested to be 
    notified for changes in state for a certain event.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! Event for which the CT is interested in receiving notifications. */
    avrcp_supported_events event_id;
    /*! Time interval, in seconds, at which the change in playback position 
        should be notified. Only applicable for EVENT_PLAYBACK_POS_CHANGED */
    uint32 playback_interval;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REGISTER_NOTIFICATION_IND_T;


/*!
    @brief This message indicates the PLAYBACK_STATUS event on the remote 
    device has changed status. 
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current play status on the remote device. */
    avrcp_play_status play_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the TRACK_CHANGED event on the remote device
    has changed status. 
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! Index of the current track - upper 4 bytes */
    uint32 track_index_high;
    /*! Index of the current track - lower 4 bytes */
    uint32 track_index_low;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_CHANGED_IND_T;


/*!
    @brief This message indicates the track has reached the end on the remote
    device. 
*/
typedef struct
{
    AVRCP *avrcp;

    /*! The response from the remote device. */
    avrcp_response_type response;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_REACHED_END_IND_T;


/*!
    @brief This message indicates the track has reached the start on the
    remote device.  
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;


#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_REACHED_START_IND_T;


/*!
    @brief This message indicates the PLAYBACK_POS_CHANGED event on the 
    remote device has changed status. 
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current playback position in milliseconds. */
    uint32 playback_pos;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND_T;


/*!
    @brief This message indicates the BATTERY_STATUS event on the remote
    device has changed status. 
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current battery status. */
    avrcp_battery_status battery_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_BATT_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the SYSTEM_STATUS event on the remote
     device has changed status. 
*/
typedef struct
{
    AVRCP *avrcp;

    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current system status. */
    avrcp_system_status system_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the DEVICE_SETTING event on the remote
    device has changed status. 
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The response. */
    avrcp_response_type response;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet (only
        valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it 
        has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND_T;


/*!
    @brief This message contains the confirmation of the request continuing
    response command.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The target PDU. */
    uint16 pdu_id;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REQUEST_CONTINUING_RESPONSE_CFM_T;


/*!
    @brief This message contains the confirmation of the abort continuing
    response command.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated. */
    uint16 transaction;
#endif

} AVRCP_ABORT_CONTINUING_RESPONSE_CFM_T;


/*!
    @brief This message contains the confirmation of the request for
    previous group.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;


#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_PREVIOUS_GROUP_CFM_T;


/*!
    @brief This message indicates that a request for the previous group has 
    been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16              transaction;
#endif
} AVRCP_PREVIOUS_GROUP_IND_T;


/*!
    @brief This message contains the confirmation of the request for
    next group.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
     /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_NEXT_GROUP_CFM_T;


/*!
    @brief This message indicates that a request for the next group has
     been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
     /*! This field has been deprecated. */
    uint16              transaction;
#endif
} AVRCP_NEXT_GROUP_IND_T;


/*!
    @brief This message is confirmation of retrieving the battery status 
    of the  remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_BATTERY_STATUS_CFM_T;


/*!
    @brief This message is a request to return the current battery status.
*/
typedef struct
{
    AVRCP *avrcp;
    /* The battery status */
    avrcp_battery_status battery_status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_BATTERY_STATUS_IND_T;


/*!
    @brief This message is confirmation of informing the remote device 
    of the character sets supported.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_CHARACTER_SET_CFM_T;


/*!
    @brief This message tells the TG what characters sets are supported 
    on the CT.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* The total number of attributes for which values are requested.
       Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_INFORM_CHARACTER_SET_IND_T;


/*!
    @brief This message is confirmation of getting the supported features 
    of the remote device.
*/
typedef struct
{
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! The features supported by the remote device. */
    uint16                features;
} AVRCP_GET_SUPPORTED_FEATURES_CFM_T;


/*!
    @brief This message is confirmation of getting the supported extensions 
    of the remote device.
*/
typedef struct
{
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! The extensions supported on the remote device. */
    uint16                extensions;
} AVRCP_GET_EXTENSIONS_CFM_T;

/*!
    @brief This message is an indication to the Target on 
    receiving SetAbsoluteVolume request from the Controller device.
*/
typedef struct{
    /*! Profile Instance */
    AVRCP             *avrcp; 
    /*! Requested %Volume. 0 as 0% and 0x7F as 100%. Scaling should
       be applied to achieve other between these two*/ 
    uint8             volume; 
                                
                                
}AVRCP_SET_ABSOLUTE_VOLUME_IND_T;

/*!
    @brief Library at Controller returns this message after completing the
operation for AvrcpSetAbsoluteVolume() request.
*/
typedef struct{
    /*! Profile Instance */
    AVRCP             *avrcp; 
    /*! Outcome of the request,returns avrcp_success on success */
    avrcp_status_code status; 
   /*! Volume at TG on success */ 
    uint8             volume; 
}AVRCP_SET_ABSOLUTE_VOLUME_CFM_T;


/*!
    @brief This message is an indication to the controller on
receiving EVENT_VOLUME_CHANGED event from the Target device.
*/
typedef struct{
    AVRCP               *avrcp;     /* Profile instance */        
    avrcp_response_type response;  /* The response from the remote device. */
    uint8               volume;    /* Volume at TG */

}AVRCP_EVENT_VOLUME_CHANGED_IND_T;



/*!
    @brief Initialise the AVRCP library.

    @param theAppTask The current application task.    
    @param config Configuration parameters for initialising the AVRCP library
    instance. This must not be set to null.
    This function also takes care of registering a service record for an AVRCP
    device.  No further library functions should be called until the
    AVRCP_INIT_CFM message has been received by the client.
*/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config);


/*!
    @brief Initialise the AVRCP library to use dynamic tasks (lazy) mode.

    @param clientTask The task where the AVRCP_INIT_CFM message will be sent.

    @param connectionTask The task where incoming connection indications 
    will be sent. For applications using the AVRCP library this will be the 
    same as the clientTask.

    @param config Configuration parameters for initialising the AVRCP library
    instance. This must not be set to null.

    This function initialises the AVRCP library in dynamic tasks mode. It 
    does everything the AvrcpInit() function does. 
 
    An AVRCP_INIT_CFM message will be returned to the clientTask once 
    initialisation has completed. AVRCP library functions should not be
    called until a successful initialisation has completed. 
*/
void AvrcpInitLazy( Task                    clientTask,
                    Task                    connectionTask,
                    const avrcp_init_params *config);


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

    @param config Configuration parameters for initialising the AVRCP library
    instance. @param config Configuration parameters for initialising the
    AVRCP library instance. As the library instance is created dynamically 
    the library client has to set the type of device. 
    This must not be set to null.
   
    AVRCP_CONNECT_CFM message will be received by the application. 
*/

void AvrcpConnectLazy(Task                      clientTask, 
                      const bdaddr              *bd_addr,
                      const avrcp_init_params   *config);


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
void AvrcpConnectResponseLazy(AVRCP                  *avrcp,
                              uint16                  connection_id,
                              bool                    accept,
                              const avrcp_init_params *config);


/*!
    @brief Request an AVRCP disconnection.  
    @param avrcp The profile instance which will be used.

    AVRCP_DISCONNECT_IND message will be received by the application.
*/
void AvrcpDisconnect(AVRCP *avrcp);


/*!
    @brief Request that a Pass Through control command is sent to the target on
    the connection identified by the profile instance. The Application shall pay
    attention on the state flag. On each button press and release. It 
    shall call this function with state flag=0 for press and state flag=1 for 
    release. The Library never tracks the state of the button whether it is 
    pressed down or released. Application shall ensure there is a button press
    and a release and maintain the time requirements between each.
    
    @param avrcp The profile instance which will be used.

    @param subunit_type The subunit type. This shall be PANEL (0x09) for AVRCP

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
                    | MSB |      |     |     |     |     |    | LSB |
                    |-----------------------------------------------|
    opcode          |           PASSTHROUGH (0x7C)                  |
                     -----------------------------------------------
    operand(0)      |state|         operation_id                    |
                      -----------------------------------------------
    operand(1)      |       operation data field length             |
                     -----------------------------------------------
    operand(2)      |    operation data(operation id dependant)     |
        :           |                                               |
                     -----------------------------------------------

*/
void AvrcpPassthrough(AVRCP             *avrcp, 
                      avc_subunit_type  subunit_type,
                      avc_subunit_id    subunit_id,
                      bool              state, 
                      avc_operation_id  opid, 
                      uint16            size_operation_data, 
                      Source            operation_data);


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
                    | MSB |     |    |      |     |      |    | LSB |
                    |-----------------------------------------------|
    opcode          |               UNITINFO (0x30)                 |
                     -----------------------------------------------
    operand[0]      |                      0xFF                     |
                     -----------------------------------------------
    operand[1]      |                      0xFF                     |
                     -----------------------------------------------
    operand[2]      |                      0xFF                     |
                     -----------------------------------------------
    operand[3]      |                      0xFF                     |
                     -----------------------------------------------
    operand[4]      |                      0xFF                     |
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
void AvrcpUnitInfoResponse( AVRCP               *avrcp, 
                            bool                accept, 
                            avc_subunit_type    unit_type, 
                            uint8               unit, 
                            uint32              company_id);


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
                    | MSB |     |    |      |     |      |     |LSB |
                    |-----------------------------------------------|
    opcode          |              SUBUNITINFO (0x31)               |
                     -----------------------------------------------
    operand[0]      |  0  |       Page      |  0  | Extension code  |
                      -----------------------------------------------
    operand[1]      |                      0xFF                     |
                     -----------------------------------------------
    operand[2]      |                      0xFF                     |
                     -----------------------------------------------
    operand[3]      |                      0xFF                     |
                     -----------------------------------------------
    operand[4]      |                      0xFF                     |
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
void AvrcpSubUnitInfoResponse(AVRCP *avrcp,bool accept,const uint8 *page_data);


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
                    | MSB |      |      |      |    |     |    |LSB |
                    |-----------------------------------------------|
    opcode          |           VENDOR-DEPENDENT (0x00)             |
                     -----------------------------------------------
    operand(0)      |MSB                                            |
    operand(1)      |                 company_id                    |
    operand(2)      |                                        LSB    |
                    |-----------------------------------------------| 
    operand(3)      |                                               |
        :           |              vendor_dependent_data            |
    operand(n)      |                                               | 
                     -----------------------------------------------
*/
void AvrcpVendorDependent(  AVRCP            *avrcp, 
                            avc_subunit_type subunit_type, 
                            avc_subunit_id   subunit_id, 
                            uint8            ctype, 
                            uint32           company_id, 
                            uint16           size_data, 
                            Source           data);


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
    @brief Some of the Metadata messages sent up to the application contain 
    Sources which hold the message data (eg. caps_list of AVRCP_GET_CAPS_CFM 
    message). When  the application has finished with the Source data, this
    API MUST be called, otherwise the library won't process any more data
    arriving until the application calls any other request or response API.
    If the application is not using this API, the Library flushes the Source
    before sending the next packet to the peer, until then the library will not
    able to receive any messages from the peer. ( A Controller application
    calling a request API triggers the library to flush the Source data 
    associated with the previous transaction. A Target application 
    responding to a request triggers the library to flush the Source data 
    associated with the request indication message.)
   
    @param avrcp The profile instance which will be used.

    This function is called after the application has finished processing 
    Metadata Source data  contained in an upstream message.
*/
void AvrcpSourceProcessed(AVRCP *avrcp);


/*!
    @brief Request the capabilities supported by the remote device (target).

    @param avrcp The profile instance which will be used.

    @param caps The type of the requested capabilities (currently only 
    company ID and supported events defined by the specification).

    This function is called to request the capabilities supported by the 
    remote (target) device. An AVRCP_GET_CAPS_CFM message will be returned
    to indicate the outcome of this request and return the supported
    capabilities if successful.
*/
void AvrcpGetCapabilities(AVRCP *avrcp, avrcp_capability_id caps);


/*!
    @brief Respond to a request for the supported capabilities.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param caps The type of the requested capabilities (currently only 
    company ID and supported events defined by the specification).

    @param size_caps_list Length, in bytes, of the list of supported
    capabilities.

    @param caps_list The list of supported capabilities.

    This function is called in response to an AVRCP_GET_CAPS_IND message being
    received from the remote device (the controller). If either on or both of
    the size_caps_list and caps_list parameters are set to zero then only the
    mandatory capabilities (as defined in the AVRCP specification) are sent out.
    
    If the client wishes to supply other capabilities then they must be passed 
    to the avrcp library using this function. The client does NOT need to supply
    the mandatory capabilities as defined in the AVRCP specification as those
    will be inserted automatically by the avrcp library. If the client does 
    supply other capabilities then they are passed on directly as is. The avrcp 
    library cannot and does not sanity check them.

    The application owns the caps_list source (if supplied) so the the source
    data must be available until all the data has been sent to the remote
    device. Once the data has been sent the source will be emptied by the
    library.
*/
void AvrcpGetCapabilitiesResponse(  AVRCP               *avrcp, 
                                    avrcp_response_type response,
                                    avrcp_capability_id caps, 
                                    uint16              size_caps_list, 
                                    Source              caps_list);


/*!
    @brief Request the player application settings attributes supported by 
    the TG

    @param avrcp The profile instance which will be used.

    This function is called from the controller (CT) to obtain the target
    supported player application setting attributes. An 
    AVRCP_LIST_APP_ATTRIBUTE_CFM message will be returned to the client to
    indicate the outcome of this request and pass on any attributes returned.
*/
void AvrcpListAppSettingAttribute(AVRCP *avrcp);


/*!
    @brief Respond to a request for a list of the player application 
    setting attributes.

      @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The supported attributes.

    This function is sent in response to a AVRCP_LIST_APP_ATTRIBUTE_IND
    message being received by the client. It is used to send the player
    application setting attributes supported by the TG to the CT.

*/
void AvrcpListAppSettingAttributeResponse(AVRCP                *avrcp,
                                          avrcp_response_type  response, 
                                          uint16               size_attributes,
                                          Source               attributes);


/*!
    @brief Request the player application settings values supported by the TG

    @param avrcp The profile instance which will be used.

    @param attribute_id The application setting attribute to retrieve the 
    possible values of from the TG.

    This function is called from the controller (CT) to obtain the target
    supported player application setting values. A AVRCP_LIST_APP_VALUE_CFM 
    message will be returned to the client to indicate the outcome of this 
    request and pass on any values returned for the attribute.
*/
void AvrcpListAppSettingValue(AVRCP *avrcp, uint16 attribute_id);


/*!
    @brief Respond to a request for a list of the player application 
    setting attributes.

      @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_values The length of the supplied values (in bytes)

    @param values The supported values.

    This function is sent in response to a AVRCP_LIST_APP_VALUE_IND
    message being received by the client. It is used to send the player
    application setting values supported by the TG to the CT.
*/
void AvrcpListAppSettingValueResponse(AVRCP               *avrcp, 
                                      avrcp_response_type response, 
                                      uint16              size_values, 
                                      Source              values);


/*!
    @brief Request for the current set values for the provided player 
    application  setting attributes list.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute IDs for which the corresponding
    current set value is requested.

    This function is sent to the TG to request it to provide the current set
    values on the target for the provided  player application setting
    attributes list. A AVRCP_GET_CURRENT_APP_VALUE_CFM message will be returned
    to the client to indicate the outcome of this request and pass on any
    values returned for the attributes.
*/
void AvrcpGetCurrentAppSettingValue(AVRCP *avrcp, 
                                    uint16 size_attributes, 
                                    Source attributes);


/*!
    @brief Response to a request for the current set values for the 
    provided player application  setting attributes list.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_values The length of the supplied values (in bytes)

    @param values The list of attribute/value pairs for which the 
    corresponding current set value is requested.

    This function is sent in response to a AVRCP_GET_APP_VALUE_IND
    message being received by the client. It is used to send the current player
    application setting values to the CT.
*/
void AvrcpGetCurrentAppSettingValueResponse(AVRCP               *avrcp,
                                            avrcp_response_type response,
                                            uint16              size_values, 
                                            Source              values);


/*!
    @brief Request to set change the value on the TG, for the provided 
    player application attribute.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute/value pairs for which the value
    should be set.

    A AVRCP_SET_APP_VALUE_CFM message will be returned 
    to the client to indicate the outcome of this request.
*/
void AvrcpSetAppSettingValue(AVRCP *avrcp, 
                             uint16 size_attributes, 
                             Source attributes);


/*!
    @brief Response to a request to set the values for the provided player 
    application  setting attributes.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This function is sent in response to a AVRCP_SET_APP_VALUE_IND
    message being received by the client. It informs the CT if the values 
    were set correctly.
*/
void AvrcpSetAppSettingValueResponse(AVRCP *avrcp,avrcp_response_type response);


/*!
    @brief This function is used to request the TG to return displayable text
    for provided PlayerAppSettingAttributeIDs.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes).

    @param attributes The list of attributes for which the text should be
    retrieved.

    A AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM message will be returned 
    to the client to indicate the outcome of this request.
*/
void AvrcpGetAppSettingAttributeText(AVRCP *avrcp, 
                                     uint16 size_attributes, 
                                     Source attributes);

/*!
    @brief This function is used to return the displayable text for
     provided PlayerAppSettingAttributeIDs.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_attributes Number of attributes for which information
    is returned.

    @param size_attributes The length of the supplied attribute text(in bytes).

    @param attributes The list of attribute text information.

    This function is sent in response to a AVRCP_GET_APP_ATTRIBUTE_TEXT_IND
    message being received by the client.

*/
void AvrcpGetAppSettingAttributeTextResponse(AVRCP *avrcp,
                                avrcp_response_type response,
                                uint16              number_of_attributes,
                                uint16              size_attributes, 
                                Source              attributes);


/*!
    @brief This function is used to request TG to return displayable text 
    for provided Player Application value.

    @param avrcp The profile instance which will be used.

    @param attribute_id Attribute ID for which the value text should
    be retrieved.

    @param size_values The length of the supplied values (in bytes).

    @param values The list of values for which the text should be retrieved.

    A AVRCP_GET_APP_VALUE_TEXT_CFM message will be returned 
    to the client to indicate the outcome of this request.
*/
void AvrcpGetAppSettingValueText(AVRCP *avrcp,
                                 uint16 attribute_id, 
                                 uint16 size_values, 
                                 Source values);


/*!
    @brief This function is used to return displayable text for provided
    Player Application values.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_values Number of values for which information is returned.

    @param size_values The length of the supplied values (in bytes).

    @param values The list of values for which the text is sent.

    This function is sent in response to a AVRCP_GET_APP_ATTRIBUTE_VALUE_IND
    message being received by the client.

*/
void AvrcpGetAppSettingValueTextResponse(AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_values,
                                         uint16 size_values,
                                         Source values);


/*!
    @brief Request the attributes of the element specified in remote 
    device (target).

    @param avrcp The profile instance which will be used.

    @param identifier_high Top 4 bytes of identifier of element on TG.

    @param identifier_low Bottom 4 bytes of identifier of element on TG.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute IDs of the attributes to be
     retrieved.

    This function is called to request the attributes of the element 
    specified in  the parameter. A AVRCP_GET_ELEMENT_ATTRIBUTES_CFM 
    message will be returned to  indicate the outcome of this request.
*/
void AvrcpGetElementAttributes(AVRCP *avrcp,
                               uint32 identifier_high, 
                               uint32 identifier_low, 
                               uint16 size_attributes, 
                               Source attributes);


/*!
    @brief Respond with the element attribute data, that was requested
    by the CT.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_attributes Number of attributes supplied.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    This function is sent in response to a AVRCP_GET_ELEMENT_ATTRIBUTES_IND
    message being received by the client.
*/ 
void AvrcpGetElementAttributesResponse(AVRCP *avrcp, 
                                       avrcp_response_type response,
                                       uint16 number_of_attributes,
                                       uint16 size_attributes, 
                                       Source attributes);


/*!
    @brief Used by the CT to request the status of the currently playing
     media at the TG.

    @param avrcp The profile instance which will be used.

    A AVRCP_GET_PLAY_STATUS_CFM message will be returned to 
    indicate the outcome of this request.
*/
void AvrcpGetPlayStatus(AVRCP *avrcp);


/*!
    @brief Used by the TG to respond with the status of the currently
     playing media.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param song_length The total length of the playing song in milliseconds.

    @param song_elapsed The current position of the playing song in
     milliseconds elapsed.

    @param play_status Current status of playing media.

    This function is sent in response to a AVRCP_GET_PLAY_STATUS_IND
    message being received by the client.
*/
void AvrcpGetPlayStatusResponse(AVRCP               *avrcp, 
                                avrcp_response_type response, 
                                uint32              song_length, 
                                uint32              song_elapsed, 
                                avrcp_play_status   play_status);


/*!
    @brief Register with the TG to get notifications based on the specific
     events occurring.

    @param avrcp The profile instance which will be used.

    @param event_id Event to receive notification of.

    @param playback_interval Only valid for EVENT_PLAYBACK_POS_CHANGED event. 
           Time interval which notification will be sent.

    A AVRCP_REGISTER_NOTIFICATION_CFM message will be returned to 
    indicate the outcome of this request.
*/
void AvrcpRegisterNotification( AVRCP                   *avrcp, 
                                avrcp_supported_events  event_id, 
                                uint32                  playback_interval);


/*!
    @brief Send event to the CT on playback status changing if the CT 
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param play_status Current play status of TG.

    The TG must call this function when the playback status has changed, 
    if this event was previously  registered by the CT. 
    A AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND message will arrive at
    the CT end to inform it that a state change has occurred.
*/
void AvrcpEventPlaybackStatusChangedResponse(AVRCP              *avrcp, 
                                            avrcp_response_type response, 
                                            avrcp_play_status   play_status);


/*!
    @brief Send event to the CT on track changing if the CT registered
    to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param track_index_high Top 4 bytes of the currently selected track.

    @param track_index_low Bottom 4 bytes of the currently selected track.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT. A AVRCP_EVENT_TRACK_CHANGED_IND
    message will arrive at the CT end to inform it that a state change
    has occurred.
*/
void AvrcpEventTrackChangedResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint32              track_index_high, 
                                    uint32              track_index_low);


/*!
    @brief Send event to the CT on track reaching end if the CT registered
    to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT. A AVRCP_EVENT_TRACK_REACHED_END_IND 
    message will arrive at the CT end to inform it that a state change
    has occurred.
*/
void AvrcpEventTrackReachedEndResponse(AVRCP               *avrcp, 
                                       avrcp_response_type response);


/*!
    @brief Send event to the CT on track reaching end if the CT registered
     to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT. A AVRCP_EVENT_TRACK_REACHED_START_IND
    message will arrive at the CT end to inform it that a state change has
    occurred.
*/
void AvrcpEventTrackReachedStartResponse(AVRCP               *avrcp,
                                         avrcp_response_type response);


/*!
    @brief Send event to the CT on playback position changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param playback_pos Current playback position in milliseconds.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT. 
    A AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND message will arrive at the CT end
    to inform it that a state change has occurred.
*/
void AvrcpEventPlaybackPosChangedResponse(AVRCP                 *avrcp, 
                                          avrcp_response_type   response, 
                                          uint32                playback_pos);


/*!
    @brief Send event to the CT on battery status changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param battery_status The current battery status.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT.

    A AVRCP_EVENT_BATT_STATUS_CHANGED_IND message will arrive at the CT end
    to inform it that a state change has occurred.
*/
void AvrcpEventBattStatusChangedResponse(AVRCP                *avrcp, 
                                         avrcp_response_type  response, 
                                         avrcp_battery_status battery_status);


/*!
    @brief Send event to the CT on system status changing if the CT 
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param system_status The current system status.

    The TG must call this function when the track has changed, if this event
    was previously  registered by the CT.

    A AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND message will arrive at the CT end
    to inform it that a state change has occurred.
*/
void AvrcpEventSystemStatusChangedResponse(AVRCP                *avrcp,
                                           avrcp_response_type  response, 
                                           avrcp_system_status  system_status);


/*!
    @brief Send event to the CT on a player app setting changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    The TG must call this function when the track has changed, if this 
    event was previously  registered by the CT. 

    A AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND message will arrive at the 
    CT end to inform it that a state change has occurred.
*/
void AvrcpEventPlayerAppSettingChangedResponse(AVRCP        *avrcp, 
                                        avrcp_response_type response, 
                                        uint16              size_attributes, 
                                        Source              attributes);


/*!
    @brief Used by CT to request for continuing response packets for the 
    sent PDU command, that has not completed. 
    This command will be invoked by CT after receiving a response with
    Packet Type - Start or Continue.

    @param avrcp The profile instance which will be used.

    @param pdu_id PDU ID of the last fragmented response to request for 
    further continuation responses. All meta data PDU IDs are defined above.     

    A AVRCP_REQUEST_CONTINUING_RESPONSE_CFM message will only arrive as 
    confirmation if an error occurred. Otherwise the response will be
    the continuation data from the TG.
*/
void AvrcpRequestContinuing(AVRCP *avrcp, uint16 pdu_id);


/*!
    @brief Used by CT to abort continuing response. 
    This command will be invoked by CT after receiving a response with
    Packet Type - Start or Continue.

    @param avrcp The profile instance which will be used.

    @param pdu_id PDU ID of the last fragmented response to abort any 
    further continuation responses. All meta data PDU IDs are defined above.     

    A AVRCP_ABORT_CONTINUING_RESPONSE_CFM message will be returned with 
    the result of the abort request.
*/
void AvrcpAbortContinuing(AVRCP *avrcp, uint16 pdu_id);


/*!
    @brief This function is used to move to the first song in the next group.

    @param avrcp The profile instance which will be used.

    A AVRCP_NEXT_GROUP_CFM message will be returned with the result of 
    this request.
*/
void AvrcpNextGroup(AVRCP *avrcp);


/*!
    @brief This function is used in response to move to the first song 
    in the next group.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected.

    This will be called in response to a AVRCP_NEXT_GROUP_IND message.
*/
void AvrcpNextGroupResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief This function is used to move to the first song in the 
    previous group.

    @param avrcp The profile instance which will be used.

    A AVRCP_PREVIOUS_GROUP_CFM message will be returned with the result 
    of this request.
*/
void AvrcpPreviousGroup(AVRCP *avrcp);


/*!
    @brief This function is used in response to move to the first song
    in the previous group.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected.

    This will be called in response to a AVRCP_PREVIOUS_GROUP_IND message.
*/
void AvrcpPreviousGroupResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief This function is used to inform TG when the CT's battery status 
    has changed.

    @param avrcp The profile instance which will be used.

    @param battery_status The current status of the battery. 

    A AVRCP_INFORM_BATTERY_STATUS_CFM message will be returned with the result
    of this request.
*/
void AvrcpInformBatteryStatusOfCt(AVRCP                *avrcp, 
                                  avrcp_battery_status battery_status);


/*!
    @brief This function is used by TG to respond to CT's inform battery 
    status command.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This will be called in response to a AVRCP_INFORM_BATTERY_STATUS_IND
    message.
*/
void AvrcpInformBatteryStatusOfCtResponse(AVRCP               *avrcp, 
                                          avrcp_response_type response);


/*!
    @brief This function is used to move to inform TG of the character set
     to use when sending strings.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    A AVRCP_INFORM_CHARACTER_SET_CFM message will be returned with the
    result of this request.

*/
void AvrcpInformDisplayableCharacterSet(AVRCP   *avrcp, 
                                        uint16  size_attributes, 
                                        Source  attributes);


/*!
    @brief This function is used by TG to respond to the CT's character 
    set command.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This will be called in response to a AVRCP_INFORM_CHARACTER_SET_IND message.
*/
void AvrcpInformDisplayableCharacterSetResponse(AVRCP               *avrcp, 
                                                avrcp_response_type response);

/*!
    @brief This function is used by the CT (Category 2) to set the absolute
    volume at category 2 TG.

    @param avrcp The profile instance.

    @param volume Relative volume specified as max %, 0 as 0% and 0x7F as 100%. 
    This API sets the maximum value for this parameter as 0x7F.

    A AVRCP_SET_ABSOLUTE_VOLUME_CFM  message will be returned after executing
    this request.
*/
void  AvrcpSetAbsoluteVolume( AVRCP *avrcp, uint8  volume);

/*!
    @brief This function is used by the TG (Category 2) to respond to
    SetAbsoluteVolume request received from CT.

    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    responses are avctp_response_accepted or avctp_response_rejected.

    @param volume Relative volume at TG (Sink device), 0 as Min and 0x7F as
    Maximum.  This API sets the maximum value for this parameter as 0x7F.
   
    This will be called in response to a AVRCP_SET_ABSOLUTE_VOLUME_IND message.
*/

void AvrcpSetAbsoluteVolumeResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint8               volume);  
/*!
    @brief This function is used by the TG (Category 2) to notify CT on 
    volume change events at TG if CT has registered for EVENT_VOLUME_CHANGED
    event.

    @param avrcp The profile instance.

    @param response Response indicating whether the volume has been changed or
    the notification was rejected. All valid responses are defined in 
    avrcp_response_type. Expected response values avctp_response_changed, 
    avctp_response_rejected.

    @param volume Relative volume at TG (Sink device), 0 as Min and 0x7F as
    Maximum.  This API sets the maximum value for this parameter as 0x7F.
  
    The TG must call this function when the volume has changed, if 
    EVENT_VOLUME_CHANGED notification was previously registered by the CT. 
*/

void AvrcpEventVolumeChangedResponse(AVRCP               *avrcp, 
                                     avrcp_response_type response,
                                     uint8               volume);

 /*!
    @brief This function is used by the CT to retrieve the supported features
    of the TG. This will include  which category commands are supported, 
    and if player application settings or group navigation is supported for
    Metadata extensions.

    @param avrcp The profile instance which will be used.

    A AVRCP_GET_SUPPORTED_FEATURES_CFM message will be returned with the
    result of this request.
*/
void AvrcpGetSupportedFeatures(AVRCP *avrcp);


/*!
    @brief This function is used by the CT to retrieve if any profile
    extensions are available on the TG.
    At the moment this will just return if AVRCP Metadata extensions at the
    remote end.

    @param avrcp The profile instance which will be used.

    A AVRCP_GET_EXTENSIONS_CFM message will be returned with the result of 
    this request.
*/
void AvrcpGetProfileExtensions(AVRCP *avrcp);


/*!
    @brief Retrieve the Sink of the AVRCP connection. This will be 0 if no
    connection exists. 

    @param avrcp The profile instance which will be used.
*/
Sink AvrcpGetSink(AVRCP *avrcp);


#endif /* AVRCP_H_ */
