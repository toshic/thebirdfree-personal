/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    hid.h
DESCRIPTION
	Header file for the HID library.
*/

/*!
@file	hid.h
@brief	Header file for the Human Interface Device library
*/

#ifndef HID_H_
#define HID_H_

#include <connection.h>
#include <message.h>
#include <panic.h>

#include <app/hid/hid_if.h> /* hid_report_type */

/*!
	@brief Defines if HID profile library supports device or host, default to HID device & host.
*/
#ifndef HID_DEVICE
#ifndef HID_HOST
#define HID_DEVICE
#define HID_HOST
#endif
#endif

/*!
	@brief Defines MTU size that L2CAP will advertise.
*/
#ifndef HID_L2CAP_MTU
#define HID_L2CAP_MTU	(672)
#endif

/************************************ Typedefs *****************************/

/*!
	@brief This identifies the device as a general device.
*/
#define HID_MAJOR_DEVICE_CLASS  0x0500
#define	HID_MINOR_MOUSE         0x000080
#define	HID_MINOR_KEYBOARD      0x000040

struct __HID;
/*!
	@brief The Human Interface Device Profile structures.
*/
typedef struct __HID HID;
typedef struct __HID_LIB HID_LIB;

/*!
    @brief Base message number for this library.
*/
#define HID_MESSAGE_BASE	0x7100

/*!
	@brief Messages sent from HID library to the application.
*/
typedef enum
{
	HID_INIT_CFM = HID_MESSAGE_BASE,
	HID_CONNECT_IND,
	HID_CONNECT_CFM,
	HID_DISCONNECT_IND,
    HID_CONTROL_IND,

#ifdef HID_DEVICE
	HID_GET_REPORT_IND,
    HID_SET_REPORT_IND,
    HID_GET_PROTOCOL_IND,
    HID_SET_PROTOCOL_IND,
    HID_GET_IDLE_IND,
    HID_SET_IDLE_IND,
    HID_DATA_IND,
#endif

#ifdef HID_HOST
	HID_GET_REPORT_CFM,
    HID_SET_REPORT_CFM,
    HID_GET_PROTOCOL_CFM,
    HID_SET_PROTOCOL_CFM,
    HID_GET_IDLE_CFM,
    HID_SET_IDLE_CFM,
#endif
	HID_MESSAGE_TOP
} HidMessageId;

/*!
	@brief Possible status codes for the initialiation confirmation.
*/
typedef enum
{
	hid_init_success,			/*!< Successful initialisation.*/
	hid_init_sdp_reg_fail,		/*!< SDP registration failed.*/
    hid_init_l2cap_reg_fail     /*!< L2CAP PSM registration failed.*/
} hid_init_status;

/*!
	@brief Possible status codes for the connection.
*/
typedef enum
{
	hid_connect_success,			/*!< Successful connection.*/
	hid_connect_failed,				/*!< Connection failed. */
	hid_connect_out_of_resources,	/*!< Out of resource. */
	hid_connect_timeout,			/*!< Timeout waiting for connection. */
	hid_connect_disconnected		/*!< Disconnected remotely during setup */
} hid_connect_status;

/*!
	@brief Possible status codes for the disconnection.
*/
typedef enum
{
	hid_disconnect_success,        /*!< Successful disconnection.*/
	hid_disconnect_link_loss,      /*!< Unsuccessful due to the link being lost.*/
	hid_disconnect_timeout,        /*!< Unsuccessful due to time out.*/
    hid_disconnect_violation,      /*!< Disconnection due to protocol violation */
	hid_disconnect_error,		   /*!< Unsuccessful for some other reason.*/
    hid_disconnect_virtual_unplug  /*!< Virtual unplug disconnection */
} hid_disconnect_status;

/*!
	@brief Possible result codes for the HANDSHAKE messages.  See HID profile 7,4,1 table 5.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_success = 0,                /*!< Successful operation.*/
    hid_busy = 1,                   /*!< Device is busy with previous operation.*/
    hid_invalid_id = 2,             /*!< Invalid report ID.*/
    hid_unsupported = 3,            /*!< Operation is unsupported.*/
    hid_invalid_param = 4,          /*!< Invalid parameter.*/
    hid_failed = 14,                /*!< Unknown failure.*/
    hid_timeout = 15,               /*!< Operation timeout.*/
    hid_resource_error = 16         /*!< Not enough resource to complete operation. (Local error, never reported to remote host/device)*/
} hid_status;

/*!
	@brief HID boot modes.  See HID profile 7.4.5, table 10 & 7.4.6, table 11.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_protocol_boot = 0,
    hid_protocol_report = 1
} hid_protocol;

/*!
	@brief HID control operations. See HID profile 7.4.2, table 6.  NOTE: These values must match those defined in the HID profile specification.
*/
typedef enum
{
    hid_control_op_nop = 0,
    hid_control_op_hard_reset = 1,
    hid_control_op_soft_reset = 2,
    hid_control_op_suspend = 3,
    hid_control_op_exit_suspend = 4,
    hid_control_op_unplug = 5
} hid_control_op;

/*!
    @brief HID library configuration structure.
*/
typedef struct
{
    uint16 service_record_size;
    const uint8 *service_record;
} hid_config;

/*!
    @brief HID connection configuration structure.
*/
typedef struct
{
    uint32 latency;
	bool qos_guaranteed;
} hid_connection_config;

/*!
	@brief This message is sent to the application when the HID library has initialised.  The application must check the status code to verifiy that initialisation was successful.  The hid_lib pointer should be stored by the application as it is required when making outgoing connections using HidConnect.
*/
typedef struct
{
	HID_LIB	*hid_lib;           /*!< The HID library structure. */
	hid_init_status	status;		/*!< The HID initialisation status.*/
} HID_INIT_CFM_T;

/*!
	@brief This message is sent to the application when the remote host/device it attempting to create a connection. The application must respond by calling HidConnectResponse to indicate whether to allow the conenction or not.
*/
typedef struct
{
	HID	*hid;                   /*!< The newly created HID instance. */
	bdaddr	bd_addr;			/*!< The Bluetooth address of the device connected to.*/
} HID_CONNECT_IND_T;

/*!
	@brief This message is sent to the application when the connection to the remote host/device has been setup or if it has failed.
*/
typedef struct
{
    HID	*hid;                   /*!< The HID instance. */
	hid_connect_status status;	/*!< The HID connection status.*/
    Sink interrupt_sink;        /*!< Sink for the interrupt channel */
} HID_CONNECT_CFM_T;

/*!
	@brief This message is sent to the application whenever the connection to the remote host/device has been disconnected, this can either be from a local or remote disconnection.
*/
typedef struct
{
	HID	*hid;                      /*!< The HID connection instance. */
	hid_disconnect_status status;  /*!< The HID disconnect status.*/
} HID_DISCONNECT_IND_T;

/*!
    @brief This message is sent to the application whenever a HID_CONTROL request is received from the remote host/device.  See HID Profile 7.4.2.
*/
typedef struct
{
    HID *hid;                   /*!< The HID connection instance. */
    hid_control_op operation;   /*!< Control operation. */ 
} HID_CONTROL_IND_T;

#ifdef HID_DEVICE
/*!
    @brief This message is sent to the application whenever a GET_REPORT request is received from the host.  See HID Profile 7.4.3.  The application must respond by calling HidGetReportResponse to send the required report to the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_report_type report_type;
    uint8 report_id;
    uint16 report_length;			/*!< Length of report data. */
} HID_GET_REPORT_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_REPORT request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 report_type;				
    uint16 size_report_data;		/*!< Length of report data. */
    uint8 report_data[1];			/*!< Start of SET_REPORT data, the rest of the data follows. */
} HID_SET_REPORT_IND_T;

/*!
    @brief This message is sent to the application whenever a DATA packet received on the interrupt channel when the applciation hasn't attached a stream.
*/
typedef HID_SET_REPORT_IND_T HID_DATA_IND_T;

/*!
    @brief This message is sent to the application whenever a GET_IDLE request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
} HID_GET_IDLE_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_IDLE request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 idle_rate;
} HID_SET_IDLE_IND_T;

/*!
    @brief This message is sent to the application whenever a GET_PROTOCOL request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
} HID_GET_PROTOCOL_IND_T;

/*!
    @brief This message is sent to the application whenever a SET_PROTOCOL request is received from the host.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    uint8 protocol;
} HID_SET_PROTOCOL_IND_T;

#endif /* HID_DEVICE */

#ifdef HID_HOST
/*!
    @brief This message is sent to the application whenever a GET_REPORT response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
    uint16 size_report_data;		/*!< Length of report data. */
    uint8 report_data[1];			/*!< Start of GET_REPORT data, the rest of the data follows. */    
} HID_GET_REPORT_CFM_T;

/*!
    @brief This message is sent to the application whenever a SET_REPORT response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
} HID_SET_REPORT_CFM_T;

/*!
    @brief This message is sent to the application whenever a GET_IDLE response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
    uint8 idle_rate;
} HID_GET_IDLE_CFM_T;

/*!
    @brief This message is sent to the application whenever a SET_IDLE response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
} HID_SET_IDLE_CFM_T;

/*!
    @brief This message is sent to the application whenever a GET_PROTOCOL response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
    hid_protocol protocol;    
} HID_GET_PROTOCOL_CFM_T;

/*!
    @brief This message is sent to the application whenever a SET_PROTOCOL response is received from the device.
*/
typedef struct
{
    HID *hid;                       /*!< The HID connection instance. */
    hid_status status;
} HID_SET_PROTOCOL_CFM_T;

#endif /* HID_HOST */

/*!
	@brief Initialise the HID library for a device.
	@param theAppTask The current application task.
	@param config Pointer to HID configuration structure.
*/
void HidInit(Task theAppTask, const hid_config *config); 

/*!
	@brief Called to initiate a new connection,
	@param hid_lib HID library structure.
	@param Task Task that connection should be associated with.
	@param bd_addr Bluetooth address of device to connect to.
	@param config Connection configuration structure.
*/
void HidConnect(HID_LIB *hid_lib, Task task, const bdaddr *bd_addr, const hid_connection_config *config);

/*!
	@brief Called by application in reponse to a HID_CONNECT_IND.
	@param hid HID connection instance.
	@param Task Task that connection should be associated with.
	@param response TRUE to connect, FALSE to refuse the connection.
	@param config Connection configuration structure, only used if response is TRUE.
*/
void HidConnectResponse(HID *hid, Task task, bool accept, const hid_connection_config *config);

/*!
	@brief Called to disconnect the specified HID connection.  Application will receive HID_DISCONNECT_IND once connection has been disconnected.
	@param hid HID connection instance.
*/
void HidDisconnect(HID * hid);

/*!
	@brief Called to send a HID_CONTROL message.
	@param hid HID connection instance.
	@param hid_control_op Control operation to send
*/
void HidControl(HID *hid, hid_control_op op);

/*!
	@brief Called by application to send a report on the interrupt channel.
	@param hid HID connection instance.
    @param report_type Type of report being sent to remove device.
    @param report_length Length of report.
    @param data Pointer to report data, the data is copied by the HID library.
*/
void HidInterruptReport(HID *hid, hid_report_type report_type, uint16 size_report_data, const uint8 *report_data);

#ifdef HID_DEVICE
/*!
	@brief Called by application in response to a HID_GET_REPORT_IND message
	@param hid HID connection instance.
    @param report_type Type of report: other, input, output or feature.
    @param report_length Length of report data in bytes.
    @param data Pointer to report data, the data is copied by the HID library.
*/
void HidGetReportResponse(HID *hid, hid_status status, hid_report_type report_type, uint16 size_report_data, const uint8 *report_data);

/*!
	@brief Called by application in response to a HID_SET_REPORT_IND message.
	@param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetReportResponse(HID *hid, hid_status status);

/*!
	@brief Called by application in response to a HID_GET_PROTOCOL_IND message.
	@param hid HID connection instance.
    @param status Response code to send to host.
    @param protocol Protocol currently being used, only valid if status is hid_success.
*/
void HidGetProtocolResponse(HID *hid, hid_status status, hid_protocol protocol);

/*!
	@brief Called by application in response to a HID_SET_PROTOCOL_IND message.
	@param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetProtocolResponse(HID *hid, hid_status status);

/*!
	@brief Called by application in response to a HID_GET_IDLE_IND message.
	@param hid HID connection instance.
    @param status Response code to send to host.
    @param idle_rate Idle rate to report to host, only valid if status is hid_success.
*/
void HidGetIdleResponse(HID *hid, hid_status status, uint8 idle_rate);

/*!
	@brief Called by application in response to a HID_SET_IDLE_IND message.
	@param hid HID connection instance.
    @param status Response code to send to host.
*/
void HidSetIdleResponse(HID *hid, hid_status status);

#endif /* HID_DEVICE */

#ifdef HID_HOST
/*!
	@brief Called by application to request a report from the remote device.
	@param hid HID connection instance.
    @param report_type Type of report requested: other, input, output or feature.
    @param report_id Optional report ID, 0 if not required.
    @param report_length Maximum size of report accepted.
*/
void HidGetReport(HID *hid, hid_report_type report_type, uint8 report_id, uint16 report_length);

/*!
	@brief Called by application to send a report to the remote device.
	@param hid HID connection instance.
    @param report_type Type of report being sent to remove device.
    @param report_length Length of report.
    @param data Pointer to report data, the data is copied by the HID library.
*/
void HidSetReport(HID *hid, hid_report_type report_type, uint16 size_data, const uint8 *data);

/*!
	@brief Called by application to get the current remote device protocol mode.  The application will receive a HID_GET_PROTOCOL_CFM reply from the remove device.
	@param hid HID connection instance.
*/
void HidGetProtocol(HID *hid);

/*!
	@brief Called by application to set the remote device protocol, The application will receive a HID_SET_PROTOCOL_CFM reply from remote device.
	@param hid HID connection instance.
    @param protocol New protocol mode that the remote device should use.
*/
void HidSetProtocol(HID *hid, hid_protocol protocol);

/*!
	@brief Called by application to get the current remove device idle rate.  The application will receive a HID_GET_IDLE_CFM reply from the remote device.
	@param hid HID connection instance.
*/
void HidGetIdle(HID *hid);

/*!
	@brief Called by the application to set the remove device idle rate.  The application will receive a HID_SET_IDLE_CFM reply from the remote device.
	@param hid HID connection instance.
    @param idle_rate New idle_rate that the remote device should use.
*/
void HidSetIdle(HID *hid, uint8 idle_rate);

#endif /* HID_HOST */

/*!
	@brief Status of pin code entry.
*/
typedef enum
{
    hid_pin_ok,
    hid_pin_full,
    hid_pin_complete,
    hid_pin_add,
	hid_pin_delete,
	hid_pin_cleared,
    hid_pin_error
} hid_pin_status;

/*!
	@brief Maximum length of pin code.
*/
#define HID_PIN_MAX_LENGTH (16)

/*!
	@brief Pin code state structure.
*/
typedef struct
{
    bool                pin_entered;
    uint8               pin_code[HID_PIN_MAX_LENGTH];
    int                 pin_code_length;
    uint8               key_down;
} hid_pin;

/*!
	@brief Called by application start pin code entry.
	@param pin Pointer to pin code structure to use.
*/
void HidPinInit(hid_pin *pin);

/*!
	@brief Called by application to handle a keyboard input report.
	@param pin Pointer to pin code structure to use.
    @param data Pointer to report data.
    @param length Length of report data.
*/
hid_pin_status HidPinCodeHandleReport(hid_pin *pin, const uint8 *data, int length);

/*!
	@brief Called by application to retreive entered pin code.
	@param pin Pointer to pin code structure to use.
*/
const uint8 *HidPinCodeData(hid_pin *pin);

/*!
	@brief Called by application to retreive length of entered pin code.
	@param pin Pointer to pin code structure to use.
*/
int HidPinCodeLength(hid_pin *pin);

#endif /* HID_H_ */
