/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_sdp_handler.c
    
DESCRIPTION
*/

#include <panic.h>
#include <region.h>
#include <service.h>
#include <string.h>

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_sdp_handler.h"
#include "init.h"


/* SDP record for the target device role */
static const uint8 serviceRecordTarget[] =
{
  0x09, /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35, /* DataElSeq 3 bytes */
  0x03,
    0x19, /* uuid - TARGET */
    0x11,
    0x0C,
  0x09, /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35, /* DataElSeq 16 bytes */
  0x10,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0017 */
        0x00,
        0x17,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVCTP(0x0017) */
      0x00,
      0x17,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35, /* DataElSeq 8 bytes */
  0x08,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid RemoteControl(0x110e) */
      0x11,
      0x0e,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* SupportedFeatures(0x0311) = "0x0002" */
    0x03,
    0x11,
  0x09, /* uint16 0x0002 - category 2 */
    0x00,
    0x02
}; /* 48 bytes */


/* SDP record for the controller device role */
static const uint8 serviceRecordController[] =
{
  0x09, /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35, /* DataElSeq 3 bytes */
  0x03,
    0x19, /* uuid - CONTROLLER */
    0x11,
    0x0E,
  0x09, /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35, /* DataElSeq 16 bytes */
  0x10,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0017 */
        0x00,
        0x17,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVCTP(0x0017) */
      0x00,
      0x17,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35, /* DataElSeq 8 bytes */
  0x08,
    0x35, /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid RemoteControl(0x110e) */
      0x11,
      0x0e,
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09, /* uint16 0x0001 - category 1 */
    0x00,
    0x01
}; /* 48 bytes */


/*****************************************************************************/
void avrcpRegisterServiceRecord(AVRCP *avrcp)
{
    uint16 size_record = 0;
    const uint8 *service_record = 0;

    if (avrcp->device_type == avrcp_target)
    {
        size_record = sizeof(serviceRecordTarget);
        service_record = serviceRecordTarget;
    }
    else if (avrcp->device_type == avrcp_controller)
    {
        size_record = sizeof(serviceRecordController);
        service_record = serviceRecordController;
    }
    else if (avrcp->device_type == avrcp_target_and_controller)
    {
        /* 
            If we need to register both records try and keep track of which 
            one we need to register next. Register target first. 
        */
        size_record = sizeof(serviceRecordTarget);
        service_record = serviceRecordTarget;
        avrcp->device_type = avrcp_device_none;
    }
    else if (avrcp->device_type == avrcp_device_none)
    {
        /* Time to register the controller service record */
        size_record = sizeof(serviceRecordController);
        service_record = serviceRecordController;
        avrcp->device_type = avrcp_target_and_controller;
    }
    else
    {
        /* Some unknown value has been passed in so fail the init. */
        avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, avrcp_fail);
        return;
    }

    /* Register the service record */
    ConnectionRegisterServiceRecord(&avrcp->task, size_record, service_record);
}


/*****************************************************************************/
void avrcpHandleSdpRegisterCfm(AVRCP *avrcp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    if (avrcp->device_type == avrcp_device_none)
    {
        /* One more service record to register */
        avrcpRegisterServiceRecord(avrcp);
    }
    else
    {
	    /* Send an initialisation confirmation with the result of the SDP registration */
        avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, cfm->status==success ? avrcp_success : avrcp_fail);
    }
}
