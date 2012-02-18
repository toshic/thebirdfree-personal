/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    common.c        

DESCRIPTION
    This file contains functions needed by the connection library for
	doing common tasks such as comparing two bluetooth device addresses.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/dm_prim.h>
#include <app/bluestack/rfcomm_prim.h>
#include <app/bluestack/sdc_prim.h>
#include <vm.h>


/****************************************************************************
NAME	
	connectionConvertHciStatus

DESCRIPTION
	Convert the status returned by BlueStack into a connection lib
	defined hci_status.

RETURNS
	hci_status corresponding to the status passed in
*/
hci_status connectionConvertHciStatus(uint16 status)
{
	switch (status)
	{
        case HCI_SUCCESS:
            return hci_success;

        case HCI_ERROR_ILLEGAL_COMMAND:
            return hci_error_illegal_command;

        case HCI_ERROR_NO_CONNECTION:
            return hci_error_no_connection;

        case HCI_ERROR_HARDWARE_FAIL:
            return hci_error_hardware_fail;

        case HCI_ERROR_PAGE_TIMEOUT:
            return hci_error_page_timeout;

        case HCI_ERROR_AUTH_FAIL:
            return hci_error_auth_fail;

        case HCI_ERROR_KEY_MISSING:
            return hci_error_key_missing;

        case HCI_ERROR_MEMORY_FULL:
            return hci_error_memory_full;

        case HCI_ERROR_CONN_TIMEOUT:
            return hci_error_conn_timeout;

        case HCI_ERROR_MAX_NR_OF_CONNS:
            return hci_error_max_nr_of_conns;

        case HCI_ERROR_MAX_NR_OF_SCO:
            return hci_error_max_nr_of_sco;

        case HCI_ERROR_MAX_NR_OF_ACL:
            return hci_error_max_nr_of_acl;

        case HCI_ERROR_COMMAND_DISALLOWED:
            return hci_error_command_disallowed;

        case HCI_ERROR_REJ_BY_REMOTE_NO_RES:
            return hci_error_rej_by_remote_no_res;

        case HCI_ERROR_REJ_BY_REMOTE_SEC:
            return hci_error_rej_by_remote_sec;

        case HCI_ERROR_REJ_BY_REMOTE_PERS:
            return hci_error_rej_by_remote_pers;

        case HCI_ERROR_HOST_TIMEOUT:
            return hci_error_host_timeout;

        case HCI_ERROR_UNSUPPORTED_FEATURE:
            return hci_error_unsupported_feature;

        case HCI_ERROR_ILLEGAL_FORMAT:
            return hci_error_illegal_format;

        case HCI_ERROR_OETC_USER:
            return hci_error_oetc_user;

        case HCI_ERROR_OETC_LOW_RESOURCE:
            return hci_error_oetc_low_resource;

        case HCI_ERROR_OETC_POWERING_OFF:
            return hci_error_oetc_powering_off;

        case HCI_ERROR_CONN_TERM_LOCAL_HOST:
            return hci_error_conn_term_local_host;

        case HCI_ERROR_AUTH_REPEATED:
            return hci_error_auth_repeated;

        case HCI_ERROR_PAIRING_NOT_ALLOWED:
            return hci_error_pairing_not_allowed;

        case HCI_ERROR_UNKNOWN_LMP_PDU:
            return hci_error_unknown_lmp_pdu;

        case HCI_ERROR_UNSUPPORTED_REM_FEATURE:
            return hci_error_unsupported_rem_feature;

        case HCI_ERROR_SCO_OFFSET_REJECTED:
            return hci_error_sco_offset_rejected;

        case HCI_ERROR_SCO_INTERVAL_REJECTED:
            return hci_error_sco_interval_rejected;

        case HCI_ERROR_SCO_AIR_MODE_REJECTED:
            return hci_error_sco_air_mode_rejected;

        case HCI_ERROR_INVALID_LMP_PARAMETERS:
            return hci_error_invalid_lmp_parameters;

        case HCI_ERROR_UNSPECIFIED:
            return hci_error_unspecified;

        case HCI_ERROR_UNSUPP_LMP_PARAM:
            return hci_error_unsupp_lmp_param;

        case HCI_ERROR_ROLE_CHANGE_NOT_ALLOWED:
            return hci_error_role_change_not_allowed;

        case HCI_ERROR_LMP_RESPONSE_TIMEOUT:
            return hci_error_lmp_response_timeout;

        case HCI_ERROR_LMP_TRANSACTION_COLLISION:
            return hci_error_lmp_transaction_collision;

        case HCI_ERROR_LMP_PDU_NOT_ALLOWED:
            return hci_error_lmp_pdu_not_allowed;

        case HCI_ERROR_ENC_MODE_NOT_ACCEPTABLE:
            return hci_error_enc_mode_not_acceptable;

        case HCI_ERROR_UNIT_KEY_USED:
            return hci_error_unit_key_used;

        case HCI_ERROR_QOS_NOT_SUPPORTED:
            return hci_error_qos_not_supported;

        case HCI_ERROR_INSTANT_PASSED:
            return hci_error_instant_passed;

        case HCI_ERROR_PAIR_UNIT_KEY_NO_SUPPORT:
            return hci_error_pair_unit_key_no_support;

        case HCI_ERROR_DIFFERENT_TRANSACTION_COLLISION:
            return hci_error_different_transaction_collision;

        case HCI_ERROR_SCM_INSUFFICIENT_RESOURCES:
            return hci_error_scm_insufficient_resources;

        case HCI_ERROR_QOS_UNACCEPTABLE_PARAMETER:
            return hci_error_qos_unacceptable_parameter;

        case HCI_ERROR_QOS_REJECTED:
            return hci_error_qos_rejected;

        case HCI_ERROR_CHANNEL_CLASS_NO_SUPPORT:
            return hci_error_channel_class_no_support;

        case HCI_ERROR_INSUFFICIENT_SECURITY:
            return hci_error_insufficient_security;

        case HCI_ERROR_PARAM_OUT_OF_MAND_RANGE:
            return hci_error_param_out_of_mand_range;

        case HCI_ERROR_SCM_NO_LONGER_REQD:
            return hci_error_scm_no_longer_reqd;

        case HCI_ERROR_ROLE_SWITCH_PENDING:
            return hci_error_role_switch_pending;

        case HCI_ERROR_SCM_PARAM_CHANGE_PENDING:
            return hci_error_scm_param_change_pending;

        case HCI_ERROR_RESVD_SLOT_VIOLATION:
            return hci_error_resvd_slot_violation;

        case HCI_ERROR_ROLE_SWITCH_FAILED:
            return hci_error_role_switch_failed;

		default:
			CL_DEBUG(("Unrecognised status %d\n",status));
            return hci_error_unrecognised;
	}
}


/****************************************************************************
NAME	
	connectionConvertAuthStatus

DESCRIPTION
	Convert the status returned by BlueStack into a connection lib
	defined authentication_status.

RETURNS
	authentication_status corresponding to the status passed in
*/
authentication_status connectionConvertAuthStatus(uint16 status)
{
	switch (status)
	{
	case HCI_SUCCESS:
		return auth_status_success;
	case HCI_ERROR_PAGE_TIMEOUT:
		return auth_status_timeout;
	case HCI_ERROR_AUTH_REPEATED:
		return auth_status_repeat_attempts;
	case HCI_ERROR_PAIRING_NOT_ALLOWED:
		return auth_status_pairing_not_allowed;
	case HCI_ERROR_UNIT_KEY_USED:
		return auth_status_unit_key_unsupported;
	case HCI_ERROR_SP_NOT_SUPPORTED_BY_HOST:
		return auth_status_simple_pairing_unsupported;
	case HCI_ERROR_HOST_BUSY_PAIRING:
		return auth_status_host_busy_pairing;
	case HCI_ERROR_AUTH_FAIL:
	default:
		return auth_status_fail;
	}
}

	
/****************************************************************************
NAME	
	connectionConvertHciVersion

DESCRIPTION
	Convert the hci version returned by BlueStack into a connection lib
	defined hci_version.

RETURNS
	hci_version corresponding to the status passed in
*/
hci_version connectionConvertHciVersion(hci_version_t version)
{
	switch (version)
	{
        case HCI_VER_1_0:
            return hci_version_1_0;
			
		case HCI_VER_1_1:
			return hci_version_1_1;
			
		case HCI_VER_1_2:
			return hci_version_1_2;
			
		case HCI_VER_2_0:
			return hci_version_2_0;
			
		case HCI_VER_2_1:
			return hci_version_2_1;
			
		default:

		     CL_DEBUG(("Unrecognised version %d\n",version));
            return hci_version_unrecognised;		
	}
}


/****************************************************************************
NAME	
	connectionConvertInquiryMode_t

DESCRIPTION
	Convert inquiry mode into the Bluestack type

RETURNS
	hci inquiry mode corresponding to the mode passed in
*/
uint8 connectionConvertInquiryMode_t(inquiry_mode mode)
{
	switch(mode)
	{	
		case inquiry_mode_standard:
			return HCI_INQUIRY_MODE_STANDARD;
		case inquiry_mode_rssi:
			return HCI_INQUIRY_MODE_WITH_RSSI;
		case inquiry_mode_eir:
			return HCI_INQUIRY_MODE_WITH_EIR;
		default:
			CL_DEBUG(("Unrecognised mode %d\n",mode));
			return HCI_INQUIRY_MODE_STANDARD;
	}
}


/****************************************************************************
NAME	
	connectionConvertInquiryMode

DESCRIPTION
	Convert Bluestack inquiry mode into the CL defined inquiry mode

RETURNS
	CL inquiry mode corresponding to the mode passed in
*/
inquiry_mode connectionConvertInquiryMode(uint8 mode)
{
	switch(mode)
	{	
		case HCI_INQUIRY_MODE_STANDARD:
			return inquiry_mode_standard;
		case HCI_INQUIRY_MODE_WITH_RSSI:
			return inquiry_mode_rssi;
		case HCI_INQUIRY_MODE_WITH_EIR:
			return inquiry_mode_eir;
		default:
			CL_DEBUG(("Unrecognised mode %d\n",mode));
			return inquiry_mode_standard;
	}
}


/****************************************************************************
NAME	
	connectionConvertSdpOpenStatus

DESCRIPTION
	Convert the status returned by BlueStack into a connection lib
	defined sdp_open_status.

RETURNS
	sdp_open_status corresponding to the status passed in
*/
sdp_open_status connectionConvertSdpOpenStatus(uint16 status)
{
	switch(status)
	{
		case SDC_OPEN_SEARCH_OK:
            return sdp_open_search_ok;

        case SDC_OPEN_SEARCH_BUSY:
            return sdp_open_search_busy;

        case SDC_OPEN_SEARCH_FAILED:
            return sdp_open_search_failed;

        case SDC_OPEN_SEARCH_OPEN:
            return sdp_open_search_open;

        case SDC_OPEN_DISCONNECTED:
            return sdp_open_disconnected;

		default:
			CL_DEBUG(("Unrecognised status %d\n",status));
			return sdp_open_unknown;		
	}
}


/****************************************************************************
NAME	
	connectionConvertSdpSearchStatus

DESCRIPTION
	Convert the status returned by BlueStack into a connection lib
	defined sdp_search_status.

RETURNS
	sdp_search_status corresponding to the status passed in
*/
sdp_search_status connectionConvertSdpSearchStatus(uint16 status)
{
	switch (status)
	{
        case SDC_RESPONSE_SUCCESS:
            return sdp_response_success;

        case SDC_ERROR_RESPONSE_PDU:
            return sdp_error_response_pdu;

        case SDC_NO_RESPONSE_DATA:
            return sdp_no_response_data;

        case SDC_CON_DISCONNECTED:
            return sdp_con_disconnected;

        case SDC_CONNECTION_ERROR:
            return sdp_connection_error;

        case SDC_CONFIGURE_ERROR:
            return sdp_configure_error;

        case SDC_SEARCH_DATA_ERROR:
            return sdp_search_data_error;

        case SDC_DATA_CFM_ERROR:
            return sdp_data_cfm_error;

        case SDC_SEARCH_BUSY:
            return sdp_search_busy;

        case SDC_RESPONSE_PDU_HEADER_ERROR:
            return sdp_response_pdu_header_error;

        case SDC_RESPONSE_PDU_SIZE_ERROR:
            return sdp_response_pdu_size_error;

        case SDC_RESPONSE_TIMEOUT_ERROR:
            return sdp_response_timeout_error;

        case SDC_SEARCH_SIZE_TO_BIG:
            return sdp_search_size_to_big;

        case SDC_RESPONSE_OUT_OF_MEMORY:
            return sdp_response_out_of_memory;

        case SDC_RESPONSE_TERMINATED:
            return sdp_response_terminated;

		default:
			CL_DEBUG(("Unrecognised status %d\n",status));
			return sdp_search_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertRfcommDisconnectStatus

DESCRIPTION
	Convert the status returned by BlueStack into a connection lib
	defined rfcomm_disconnect_status.

RETURNS
	rfcomm_disconnect_status corresponding to the status passed in
*/
rfcomm_disconnect_status connectionConvertRfcommDisconnectStatus(reason_code_t status)
{
	switch (status)
	{
        case RFC_SUCCESS:
            return rfcomm_disconnect_success;

        case RFC_CONNECTION_PENDING:
            return rfcomm_disconnect_connection_pending;

        case RFC_CONNECTION_REJ_PSM:
            return rfcomm_disconnect_connection_rej_psm;

        case RFC_CONNECTION_REJ_SECURITY:
            return rfcomm_disconnect_connection_rej_security;

        case RFC_CONNECTION_REJ_RESOURCES:
            return rfcomm_disconnect_connection_rej_resources;

        case RFC_CONNECTION_REJ_NOT_READY:
            return rfcomm_disconnect_connection_rej_not_ready;

        case RFC_CONNECTION_FAILED:
            return rfcomm_disconnect_connection_failed;

        case RFC_CONNECTION_TIMEOUT:
            return rfcomm_disconnect_connection_timeout;

        case NORMAL_DISCONNECT:
            return rfcomm_disconnect_normal_disconnect;

        case ABNORMAL_DISCONNECT:
            return rfcomm_disconnect_abnormal_disconnect;

        case RFC_CONFIG_UNACCEPTABLE:
            return rfcomm_disconnect_config_unacceptable;

        case RFC_CONFIG_REJECTED:
            return rfcomm_disconnect_config_rejected;

        case RFC_CONFIG_UNKNOWN:
            return rfcomm_disconnect_config_unknown;

        case RFC_CONFIG_REJECTED_LOCALLY:
            return rfcomm_disconnect_config_rejected_locally;

        case RFC_CONFIG_TIMEOUT:
            return rfcomm_disconnect_config_timeout;

        case REMOTE_REFUSAL:
            return rfcomm_disconnect_remote_refusal;

        case RACE_CONDITION_DETECTED:
            return rfcomm_disconnect_race_condition_detected;

        case INSUFFICIENT_RESOURCES:
            return rfcomm_disconnect_insufficient_resources;

        case CANNOT_CHANGE_FLOW_CONTROL_MECHANISM:
            return rfcomm_disconnect_cannot_change_flow_control_mechanism;

        case DLC_ALREADY_EXISTS:
            return rfcomm_disconnect_dlc_already_exists;

        case RFC_DLC_REJ_SECURITY:
            return rfcomm_disconnect_dlc_rej_security;

        case GENERIC_REFUSAL:
            return rfcomm_disconnect_generic_refusal;

        case UNEXPECTED_PRIMITIVE:
            return rfcomm_disconnect_unexpected_primitive;

        case INVALID_SERVER_CHANNEL:
            return rfcomm_disconnect_invalid_server_channel;

        case UNKNOWN_MUX_ID:
            return rfcomm_disconnect_unknown_mux_id;

        case LOCAL_ENTITY_TERMINATED_CONNECTION:
            return rfcomm_disconnect_local_entity_terminated_connection;

        case UNKNOWN_PRIMITIVE:
            return rfcomm_disconnect_unknown_primitive;

        case MAX_PAYLOAD_EXCEEDED:
            return rfcomm_disconnect_max_payload_exceeded;

        case INCONSISTENT_PARAMETERS:
            return rfcomm_disconnect_inconsistent_parameters;

        case INSUFFICIENT_CREDITS:
            return rfcomm_disconnect_insufficient_credits;

        case CREDIT_FLOW_CONTROL_PROTOCOL_VIOLATION:
            return rfcomm_disconnect_credit_flow_control_protocol_violation;

        case MUX_ALREADY_OPEN:
            return rfcomm_disconnect_mux_already_open;

        case RFC_RES_ACK_TIMEOUT:
            return rfcomm_disconnect_res_ack_timeout;

		default:
			CL_DEBUG(("Unrecognised status %d\n",status));
			return rfcomm_disconnect_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertBdaddr

DESCRIPTION
	This function converts from CSR bdaddr type to CCL BD_ADDR_T type

RETURNS

*/
void connectionConvertBdaddr_t(BD_ADDR_T* out, const bdaddr* in)
{
	out->lap = (uint32_t)(in->lap);
	out->uap = (uint8_t)(in->uap);
	out->nap = (uint16_t)(in->nap);
}

/****************************************************************************
NAME	
	connectionConvertBdaddr

DESCRIPTION
	This function converts from CCL BD_ADDR_T type to a CSR bdaddr type

RETURNS

*/
void connectionConvertBdaddr(bdaddr* out, const BD_ADDR_T *in)
{
	out->lap = (uint32)(in->lap);
	out->uap = (uint8)(in->uap);
	out->nap = (uint16)(in->nap);
}


/****************************************************************************
NAME	
	connectionConvertPageScanRepMode_t

DESCRIPTION
	Convert the page scan mode returned by BlueStack into a connection lib
	defined page_scan_rep_mode.

RETURNS
	page_scan_rep_mode corresponding to the mode passed in
*/
page_scan_rep_mode connectionConvertPageScanRepMode_t(page_scan_rep_mode_t mode)
{
	switch (mode)
	{
	case HCI_PAGE_SCAN_REP_MODE_R0:
		return page_scan_rep_mode_r0;

	case HCI_PAGE_SCAN_REP_MODE_R1:
		return page_scan_rep_mode_r1;

	case HCI_PAGE_SCAN_REP_MODE_R2:
		return page_scan_rep_mode_r2;

	default:
		CL_DEBUG(("Unrecognised mode %d\n",mode));
		return page_scan_rep_mode_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertPageScanRepMode

DESCRIPTION
	Convert the connection lib defined page_scan_rep_mode into a value to be 
	passed into BlueStack.

RETURNS
	page_scan_rep_mode_t corresponding to the mode passed in
*/
page_scan_rep_mode_t connectionConvertPageScanRepMode(page_scan_rep_mode mode)
{
	switch (mode)
	{
	case page_scan_rep_mode_r0:
		return HCI_PAGE_SCAN_REP_MODE_R0;

	case page_scan_rep_mode_r1:
		return HCI_PAGE_SCAN_REP_MODE_R1;

	case page_scan_rep_mode_r2:
		return HCI_PAGE_SCAN_REP_MODE_R2;

	case page_scan_rep_mode_unknown:
	default:
		CL_DEBUG(("Invalid page scan rep mode passed in 0x%x\n", mode));
		/* the sensible default should get us a connection eventually */
		return HCI_PAGE_SCAN_REP_MODE_R2;
	}
}


/****************************************************************************
NAME	
	connectionConvertPageScanMode_t

DESCRIPTION
	Convert the page scan mode returned by BlueStack into a connection lib
	defined page_scan_mode.

RETURNS
	page_scan_mode corresponding to the mode passed in
*/
page_scan_mode connectionConvertPageScanMode_t(page_scan_mode_t mode)
{
	switch (mode)
	{
		case HCI_PAGE_SCAN_MODE_MANDATORY:
			return page_scan_mode_mandatory;

		case HCI_PAGE_SCAN_MODE_OPTIONAL_1:
			return page_scan_mode_optional_1;

		case HCI_PAGE_SCAN_MODE_OPTIONAL_2:
			return page_scan_mode_optional_2;

		case HCI_PAGE_SCAN_MODE_OPTIONAL_3:
			return page_scan_mode_optional_3;
		
		default:
			CL_DEBUG(("Unrecognised mode %d\n",mode));
			return page_scan_mode_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertPageScanMode

DESCRIPTION
	Convert the connection lib defined page_scan_mode into a value to be 
	passed into BlueStack.

RETURNS
	page_scan_mode_t as defined by BlueStack
*/
page_scan_mode_t connectionConvertPageScanMode(page_scan_mode mode)
{
	switch (mode)
	{
		case page_scan_mode_mandatory:
			return HCI_PAGE_SCAN_MODE_MANDATORY;

		case page_scan_mode_optional_1:
			return HCI_PAGE_SCAN_MODE_OPTIONAL_1;

		case page_scan_mode_optional_2:
			return HCI_PAGE_SCAN_MODE_OPTIONAL_2;

		case page_scan_mode_optional_3:
			return HCI_PAGE_SCAN_MODE_OPTIONAL_3;
		
		case page_scan_mode_unknown:
		default:
			CL_DEBUG(("Invalid page scan mode passed in 0x%x\n", mode));
			/* this is probably the safest default */
			return HCI_PAGE_SCAN_MODE_MANDATORY;
	}
}



/****************************************************************************
NAME	
	connectionConvertProtocolId_t

DESCRIPTION
	Convert the dm_protocol_id_t defined by BlueStack into a connection lib
	defined dm_protocol_id.

RETURNS
	dm_protocol_id corresponding to the protocol id passed in
*/
dm_protocol_id connectionConvertProtocolId_t(dm_protocol_id_t id)
{
	if (id == SEC_PROTOCOL_L2CAP)
		return protocol_l2cap;
	else if (id == SEC_PROTOCOL_RFCOMM)
		return protocol_rfcomm;
	else if (id >= SEC_PROTOCOL_USER)
		return (dm_protocol_id) id;
	else 
	{
		CL_DEBUG(("Unrecognised id %d\n", (uint16) id));
		return protocol_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertProtocolId

DESCRIPTION
	Convert from dm_protocol_id defined in connection.h to dm_protocol_id_t 
	defined by BlueStack.

RETURNS
	dm_protocol_id_t corresponding to the dm_protocol_id passed in
*/
dm_protocol_id_t connectionConvertProtocolId(dm_protocol_id id)
{
	if (id == protocol_l2cap)
		return SEC_PROTOCOL_L2CAP;
	else if (id == protocol_rfcomm)
		return SEC_PROTOCOL_RFCOMM;
	else
		return (dm_protocol_id_t) id;
}


/****************************************************************************
NAME	
	connectionConvertHciScanEnable

DESCRIPTION
	Convert the connection lib defined HCI scan enable into the Bluestack
	defined type.

RETURNS
	Blustack HCI scan mode 
*/
uint8 connectionConvertHciScanEnable(hci_scan_enable mode)
{
	switch (mode)
	{
		case hci_scan_enable_off:
			return HCI_SCAN_ENABLE_OFF;

		case hci_scan_enable_inq:
			return HCI_SCAN_ENABLE_INQ;

		case hci_scan_enable_page:
			return HCI_SCAN_ENABLE_PAGE;

		case hci_scan_enable_inq_and_page:
			return HCI_SCAN_ENABLE_INQ_AND_PAGE;
		
		default:
			CL_DEBUG(("Unrecognised mode %d\n",mode));
			/* safest default is to assume connectable
                           and discoverable */
			return HCI_SCAN_ENABLE_INQ_AND_PAGE;

	}
}


/****************************************************************************
NAME	
	connectionConvertSecurityMode_t

DESCRIPTION
	Convert the connection lib defined security mode into the Bluestack
	defined security mode type

RETURNS
	Bluestack security mode
*/
dm_security_mode_t connectionConvertSecurityMode_t(dm_security_mode mode)
{
	switch(mode)
	{
		case sec_mode0_off:
			return SEC_MODE0_OFF;
			
		case sec_mode1_non_secure:
			return SEC_MODE1_NON_SECURE;
			
		case sec_mode2_service:
			return SEC_MODE2_SERVICE;
			
		case sec_mode3_link:
			return SEC_MODE3_LINK;

		case sec_mode4_ssp:
			return SEC_MODE4_SSP;
		
		case sec_mode_unknown:
		default:
			CL_DEBUG(("Unrecognised mode %d\n",mode));
			/* tricky - probably the safest default is to turn
                           off security - at least the device can then be used
			*/
			return SEC_MODE0_OFF;
	}
}


/****************************************************************************
NAME	
	connectionConvertSecurityLevel_t

DESCRIPTION
	Convert the connection lib defined security level into the Bluestack
	defined security level type

RETURNS
	Bluestack security level
*/
dm_security_level_t connectionConvertSecurityLevel_t(dm_security_level level)
{
	switch(level)
	{
		case secl_none:
			return SECL_NONE;
					
		case secl_in_authorisation:
			return SECL_IN_AUTHORISATION;
					
		case secl_in_authentication:
			return SECL_IN_AUTHENTICATION;
		
		case secl_in_encryption:
			return SECL_IN_ENCRYPTION;
			
		case secl_out_authorisation:
			return SECL_OUT_AUTHORISATION;
		
		case secl_out_authentication:
			return SECL_OUT_AUTHENTICATION;
		
		case secl_out_encryption:
			return SECL_OUT_ENCRYPTION;
		
		case secl_in_connectionless:
			return SECL_IN_CONNECTIONLESS;
		
		case secl_level_unknown:
		default:
			CL_DEBUG(("Unrecognised mode %d\n", level));
			/* tricky - probably the safest default is to turn
                           off security - at least the device can then be used
			*/
			return SECL_NONE;
	}
}


/****************************************************************************
NAME	
	connectionConvertDefaultSecurityLevel_t

DESCRIPTION
	Convert the connection lib defined default security level into the Bluestack
	defined security level type

RETURNS
	Bluestack security level
*/
uint8_t connectionConvertSspSecurityLevel_t(dm_ssp_security_level level, bool outgoing_ok, bool authorised, bool disable_legacy)
{
	uint8 secl;
	if(outgoing_ok)
	{
		switch(level)
		{
			case ssp_secl4_l0:
				secl = SECL4_LEVEL_0;
				break;
			case ssp_secl4_l1:
				secl = SECL4_LEVEL_1;
				break;
			case ssp_secl4_l2:
				secl = SECL4_LEVEL_2;
				break;
			case ssp_secl4_l3:
				secl = SECL4_LEVEL_3;
				break;
			case ssp_secl_level_unknown:
			default:
				CL_DEBUG(("Unrecognised mode %d\n", level));
				/* Use the default given in the 2.1 spec */
				secl = SECL4_LEVEL_2;
				break;
		}
		if(authorised)
			secl |= SECL_AUTHORISATION;
		if(disable_legacy)
			secl |= SECL4_NO_LEGACY;
	}
	else
	{
		switch(level)
		{
			case ssp_secl4_l0:
				secl = SECL4_IN_LEVEL_0;
				break;
			case ssp_secl4_l1:
				secl = SECL4_IN_LEVEL_1;
				break;
			case ssp_secl4_l2:
				secl = SECL4_IN_LEVEL_2;
				break;
			case ssp_secl4_l3:
				secl = SECL4_IN_LEVEL_3;
				break;
			case ssp_secl_level_unknown:
			default:
				CL_DEBUG(("Unrecognised mode %d\n", level));
				/* Use the default given in the 2.1 spec */
				secl = SECL4_IN_LEVEL_2;
				break;
		}
		if(authorised)
			secl |= SECL_IN_AUTHORISATION;
		if(disable_legacy)
			secl |= SECL4_IN_NO_LEGACY;
	}
	return secl;
}


/****************************************************************************
NAME	
	connectionConvertWriteAuthEnable_t

DESCRIPTION
	Convert the connection lib defined write auth enable into the Bluestack
	defined write auth enable

RETURNS
	Bluestack write auth enable
*/
uint8_t connectionConvertWriteAuthEnable_t(cl_sm_wae write_auth_enable)
{
	switch(write_auth_enable)
	{
		case cl_sm_wae_never:
			return DM_SM_WAE_NEVER;
		
		case cl_sm_wae_acl_none:
			return DM_SM_WAE_ACL_NONE;
		
		case cl_sm_wae_acl_owner_none:
			return DM_SM_WAE_ACL_OWNER_NONE;
		
		case cl_sm_wae_acl_owner_app:
			return DM_SM_WAE_ACL_OWNER_APP;
			
		case cl_sm_wae_acl_owner_l2cap:
			return DM_SM_WAE_ACL_OWNER_L2CAP;
			
		case cl_sm_wae_always:
			return DM_SM_WAE_ALWAYS;
		default:
			CL_DEBUG(("Unrecognised wae %d\n", write_auth_enable));
			return DM_SM_WAE_ACL_NONE;
	}
}

/****************************************************************************
NAME	
	connectionConvertAuthenticationRequirements

DESCRIPTION
	Convert the Bluestack defined authentication requirements into the 
    connection lib defined authentication requirements

RETURNS
	connection lib authentication requirements
*/
cl_sm_auth_requirements connectionConvertAuthenticationRequirements(uint8_t authentication_requirements)
{
	switch(authentication_requirements)
	{
		case HCI_MITM_NOT_REQUIRED_NO_BONDING:
			return cl_sm_no_bonding_no_mitm;
		
		case HCI_MITM_REQUIRED_NO_BONDING:
			return cl_sm_no_bonding_mitm;
		
		case HCI_MITM_NOT_REQUIRED_GENERAL_BONDING:
			return cl_sm_general_bonding_no_mitm;
		
		case HCI_MITM_REQUIRED_GENERAL_BONDING:
			return cl_sm_general_bonding_mitm;
			
		case HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING:
			return cl_sm_dedicated_bonding_no_mitm;
			
		case HCI_MITM_REQUIRED_DEDICATED_BONDING:
			return cl_sm_dedicated_bonding_mitm;
		default:
			CL_DEBUG(("Unrecognised authentication requirements %d\n", authentication_requirements));
			return cl_sm_authentication_requirements_unknown;
	}
}


/****************************************************************************
NAME	
	connectionConvertLinkKeyType_t

DESCRIPTION
	Convert the connection lib defined link key type into the Bluestack
	defined link key type

RETURNS
	Bluestack security level
*/
uint8_t connectionConvertLinkKeyType_t(cl_sm_link_key_type link_key_type)
{
	switch(link_key_type)
	{
		case cl_sm_link_key_none:
			return DM_SM_LINK_KEY_NONE;
			
		case cl_sm_link_key_legacy:
			return DM_SM_LINK_KEY_LEGACY;
			
		case cl_sm_link_key_debug:
			return DM_SM_LINK_KEY_DEBUG;
			
		case cl_sm_link_key_unauthenticated:
			return DM_SM_LINK_KEY_UNAUTHENTICATED;
			
		case cl_sm_link_key_authenticated:
			return DM_SM_LINK_KEY_AUTHENTICATED;
			
		case cl_sm_link_key_changed:
			return DM_SM_LINK_KEY_CHANGED;
			
		default:
			CL_DEBUG(("Unrecognised link key type %d\n", link_key_type));
			return DM_SM_LINK_KEY_NONE;
	}
}

/****************************************************************************
NAME	
	connectionConvertLinkKeyType

DESCRIPTION
	Convert the Bluestack defined link key type into the Connection Lib
	defined link key type

RETURNS
	Connection Lib security level
*/
cl_sm_link_key_type connectionConvertLinkKeyType(uint8_t link_key_type)
{
	switch(link_key_type)
	{
		case DM_SM_LINK_KEY_NONE:
			return cl_sm_link_key_none;
			
		case DM_SM_LINK_KEY_LEGACY:
			return cl_sm_link_key_legacy;
			
		case DM_SM_LINK_KEY_DEBUG:
			return cl_sm_link_key_debug;
			
		case DM_SM_LINK_KEY_UNAUTHENTICATED:
			return cl_sm_link_key_unauthenticated;
			
		case DM_SM_LINK_KEY_AUTHENTICATED:
			return cl_sm_link_key_authenticated;
			
		case DM_SM_LINK_KEY_CHANGED:
			return cl_sm_link_key_changed;
			
		default:
			CL_DEBUG(("Unrecognised link key type %d\n", link_key_type));
			return cl_sm_link_key_none;
	}
}

/****************************************************************************
NAME	
	connectionConvertIoCapability_t

DESCRIPTION
	Convert the Connection Lib defined IO capability into the Bluestack
	defined IO capability

RETURNS
	Bluestack IO capability
*/
uint8_t connectionConvertIoCapability_t(cl_sm_io_capability io_capability)
{
	switch(io_capability)
	{
		case cl_sm_io_cap_display_only:
			return HCI_IO_CAP_DISPLAY_ONLY;
			
		case cl_sm_io_cap_display_yes_no:
			return HCI_IO_CAP_DISPLAY_YES_NO;
			
		case cl_sm_io_cap_keyboard_only:
			return HCI_IO_CAP_KEYBOARD_ONLY;
			
		case cl_sm_io_cap_no_input_no_output:
			return HCI_IO_CAP_NO_INPUT_NO_OUTPUT;
			
		default:
			CL_DEBUG(("Unrecognised IO capability %d\n", io_capability));
			return HCI_IO_CAP_NO_INPUT_NO_OUTPUT;
	}
}

/****************************************************************************
NAME	
	connectionConvertIoCapability

DESCRIPTION
	Convert the Bluestack defined IO capability into the Connection Lib
	defined IO capability

RETURNS
	Connection Lib IO capability
*/
cl_sm_io_capability connectionConvertIoCapability(uint8_t io_capability)
{
	switch(io_capability)
	{
		case HCI_IO_CAP_DISPLAY_ONLY:
			return cl_sm_io_cap_display_only;
			
		case HCI_IO_CAP_DISPLAY_YES_NO:
			return cl_sm_io_cap_display_yes_no;
			
		case HCI_IO_CAP_KEYBOARD_ONLY:
			return  cl_sm_io_cap_keyboard_only;
			
		case HCI_IO_CAP_NO_INPUT_NO_OUTPUT:
			return cl_sm_io_cap_no_input_no_output;
			
		default:
			CL_DEBUG(("Unrecognised IO capability %d\n", io_capability));
			return cl_sm_io_cap_no_input_no_output;
	}
}

/****************************************************************************
NAME	
	connectionConvertKeypressType_t

DESCRIPTION
	Convert the connection lib defined keypress type into the Bluestack
	defined keypress notification type

RETURNS
	Bluestack keypress type
*/
uint8_t connectionConvertKeypressType_t(cl_sm_keypress_type type)
{
	switch(type)
	{
		case cl_sm_passkey_started:
			return HCI_NOTIFICATION_TYPE_PASSKEY_STARTED;
		case cl_sm_passkey_digit_entered:
			return HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED;
		case cl_sm_passkey_digit_erased:
			return HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ERASED;
		case cl_sm_passkey_cleared:
			return HCI_NOTIFICATION_TYPE_PASSKEY_CLEARED;
		case cl_sm_passkey_complete:
			return HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED;
		default:
			CL_DEBUG(("Unrecognised keypress type %d\n", type));
			return HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED;
	}
}

/****************************************************************************
NAME	
	connectionConvertKeypressType

DESCRIPTION
	Convert the Bluestack defined keypress notification type into the 
	Connection Lib defined keypress type

RETURNS
	Connection Lib keypress type
*/
cl_sm_keypress_type connectionConvertKeypressType(uint8_t type)
{
	switch(type)
	{
		case HCI_NOTIFICATION_TYPE_PASSKEY_STARTED:
			return cl_sm_passkey_started;
		case HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ENTERED:
			return cl_sm_passkey_digit_entered;
		case HCI_NOTIFICATION_TYPE_PASSKEY_DIGIT_ERASED:
			return cl_sm_passkey_digit_erased;
		case HCI_NOTIFICATION_TYPE_PASSKEY_CLEARED:
			return cl_sm_passkey_cleared;
		case HCI_NOTIFICATION_TYPE_PASSKEY_COMPLETED:
			return cl_sm_passkey_complete;
		default:
			CL_DEBUG(("Unrecognised keypress type %d\n", type));
			return cl_sm_passkey_digit_entered;
	}
}


/****************************************************************************
NAME	
	connectionConvertHciRole

DESCRIPTION
	Convert the mode returned by BlueStack into a connection lib defined 
	hci_role.

RETURNS
	Bluestack role
*/
hci_role connectionConvertHciRole(uint8 role)
{
	switch(role)
	{
		case HCI_MASTER:
			return hci_role_master;
		
		case HCI_SLAVE:
			return hci_role_slave;
				
		default:
			CL_DEBUG(("Unrecognised role %d",role));
			return hci_role_dont_care;
	}
}


/****************************************************************************
NAME	
	connectionConvertHciRole_t

DESCRIPTION
	Convert the connection lib defined hci_role into the Bluestack role

RETURNS
	Bluestack role
*/
uint8 connectionConvertHciRole_t(hci_role role)
{
	switch(role)
	{
		case hci_role_master:
			return HCI_MASTER;			
		
		case hci_role_slave:
			return HCI_SLAVE;

		case hci_role_dont_care:
		default:
			CL_DEBUG(("Unrecognised role %d\n",role));
			return HCI_MASTER_SLAVE_UNKNOWN;			
	}
}


/****************************************************************************
NAME	
	connectionConvertBtVersion

DESCRIPTION
	Convert the Bluestack defined BT Version into the 
	Connection Lib defined BT Version

RETURNS
	Connection Lib version
*/
cl_dm_bt_version connectionConvertBtVersion(uint8_t version)
{
	switch(version)
	{
		case BT_VERSION_2p0:
			return bluetooth2_0;
		case BT_VERSION_2p1:
			return bluetooth2_1;
		case BT_VERSION_CURRENT:
		default:
			CL_DEBUG(("Unrecognised BT Version_t %d \n",version));
			return bluetooth_unknown;
	}
}


/*****************************************************************************/
Task createTaskInstance(const profile_task_recipe *task_recipe, Task clientTask)
{
    uint16 *tsk;

    /* Create the profile instance task */
    Task new_task = (Task) malloc(task_recipe->size_instance);

    if (new_task)
    {
        /* Set the profile task handler */
        new_task->handler = task_recipe->data.handler;

        /* Initialize the profile instance task */
        tsk = malloc(sizeof(uint16));
        *tsk = (uint16) clientTask;
        MessageSend(new_task, task_recipe->msg_id, tsk);
    }

    return new_task;
}
