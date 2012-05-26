/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_l2cap_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_L2CAP_HANDLER_H_
#define AVRCP_L2CAP_HANDLER_H_

#include "avrcp_sdp_handler.h"
#include "avrcp_init.h"


/****************************************************************************
NAME    
    avrcpHandleL2capRegisterCfm

DESCRIPTION
    This function is called on receipt of an CL_L2CAP_REGISTER_CFM.
*/
void avrcpHandleL2capRegisterCfm(AVRCP              *avrcp,
                     const CL_L2CAP_REGISTER_CFM_T *cfm);

        
/****************************************************************************
NAME    
    avrcpHandleL2capConnectCfm

DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
    indicating the outcome of the connect attempt.
*/
void avrcpHandleL2capConnectCfm(AVRCP            *avrcp, 
                    const CL_L2CAP_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME    
    avrcpHandleL2capConnectInd

DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_IND message.
    This message indicates that a remote device is attempting to establish
    an L2CAP connection to this device on the AVCTP PSM.
*/
void avrcpHandleL2capConnectInd(AVRCP           *avrcp, 
            const CL_L2CAP_CONNECT_IND_T        *ind);


/****************************************************************************
NAME    
    avrcpHandleL2capConnectIndReject

DESCRIPTION
    The profile instance is in the wrong state, automatically reject the 
    connect request.
*/
void avrcpHandleL2capConnectIndReject(AVRCP      *avrcp,
                 const CL_L2CAP_CONNECT_IND_T    *ind);


/****************************************************************************
NAME    
    avrcpHandleL2capDisconnectInd

DESCRIPTION
    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
    This message indicates that an L2CAP connection has been disconnected.
*/
void avrcpHandleL2capDisconnectInd(AVRCP        *avrcp, 
                const CL_L2CAP_DISCONNECT_IND_T *ind);


#endif /* AVRCP_L2CAP_HANDLER_H_ */
