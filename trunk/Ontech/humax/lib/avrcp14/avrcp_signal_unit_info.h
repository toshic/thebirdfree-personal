/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2


FILE NAME
    avrcp_signal_unit_info.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SIGNAL_UNIT_INFO_H_
#define AVRCP_SIGNAL_UNIT_INFO_H_

#include "avrcp_common.h"

/****************************************************************************
NAME    
    avrcpSendUnitInfoCfmToClient

DESCRIPTION
    This function creates a AVRCP_UNITINFO_CFM message and sends it to 
    the client task.
*/
void avrcpSendUnitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status,
                                 uint16 unit_type, uint16 unit, uint32 company);


/****************************************************************************
NAME    
    avrcpHandleInternalUnitInfoReq

DESCRIPTION
    This function internally handles unit info message request.
*/
void avrcpHandleInternalUnitInfoReq(AVRCP *avrcp);


/****************************************************************************
NAME    
    avrcpHandleInternalUnitInfoRes

DESCRIPTION
    This function internally handles unit info message result.
*/
void avrcpHandleInternalUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_UNITINFO_RES_T *req);


/****************************************************************************
NAME    
    avrcpHandleUnitInfoCommand

DESCRIPTION
    This function internally handles unit info command received from a remote device.
*/
void avrcpHandleUnitInfoCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleUnitInfoResponse

DESCRIPTION
    This function internally handles unit info response received from a remote device.
*/
void avrcpHandleUnitInfoResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME    
    avrcpSendSubunitInfoCfmToClient

DESCRIPTION
    This function creates a AVRCP_SUBUNITINFO_CFM message and sends it to 
    the client task.
*/
void avrcpSendSubunitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 page, const uint8 *page_data);


/****************************************************************************
NAME    
    avrcpHandleInternalSubUnitInfoReq

DESCRIPTION
    This function internally handles subunit info message request.
*/
void avrcpHandleInternalSubUnitInfoReq(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *req);


/****************************************************************************
NAME    
    avrcpHandleInternalSubUnitInfoRes

DESCRIPTION
    This function internally handles subunit info message result.
*/
void avrcpHandleInternalSubUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleSubUnitInfoCommand

DESCRIPTION
    This function internally handles subunit info command received
     from a remote device.
*/
void avrcpHandleSubUnitInfoCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleSubUnitInfoResponse

DESCRIPTION
    This function internally handles subunit info response received 
    from a remote device.
*/
void avrcpHandleSubUnitInfoResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


#endif /* AVRCP_SIGNAL_UNIT_INFO_H_ */

