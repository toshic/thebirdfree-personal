/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avctp_connect.c
    
DESCRIPTION
*/

#include "avrcp_connect_handler.h"
#include "avrcp_sdp_handler.h"


/*****************************************************************************/
static void avrcpSendInternalConnectResp(AVRCP  *avrcp, 
                                         uint16 connection_id, 
                                         bool   accept)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_RES);
    message->connection_id = connection_id;
    message->accept = accept;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_RES, message);
}


/*****************************************************************************/
static void avrcpSendInternalConnectReq(AVRCP *avrcp, const bdaddr *bd_addr)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_REQ);
    message->bd_addr = *bd_addr;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_REQ, message);
}


/****************************************************************************
*NAME    
*    AvrcpConnect    
*
*DESCRIPTION
*  API function to establish AVRCP Connection.
*    
*PARAMETERS
*   avrcp            - Task
*   bd_addr          - Remote Device Bluetooth Address
*
*RETURN
*   AVRCP_CONNECT_CFM
*******************************************************************************/
void AvrcpConnect(AVRCP *avrcp, const bdaddr *bd_addr)
{
#ifdef AVRCP_DEBUG_LIB    
    if (!bd_addr)
    {
        AVRCP_DEBUG(("Null Bluetooth address pointer\n"));
    }
#endif
    
    avrcpSendInternalConnectReq(avrcp, bd_addr);
}

/****************************************************************************
*NAME    
*    AvrcpConnectLazy    
*
*DESCRIPTION
*  API function to establish AVRCP Connection in Lazy Task mode.
*    
*PARAMETERS
*   avrcp            - Task
*   bd_addr          - Remote Device Bluetooth Address
*   config           - Same initialization parameters used in AvrcpInitLazy()
*
*RETURN
*  AVRCP_CONNECT_CFM
*******************************************************************************/
void AvrcpConnectLazy(Task               clientTask, 
              const bdaddr              *bd_addr, 
              const avrcp_init_params   *config)
{
    if (!config)
    {
        /* Client must pass down a valid config. report connect fail */
        MAKE_AVRCP_MESSAGE(AVRCP_CONNECT_CFM);
        message->status = avrcp_fail;
        message->sink = 0;
        message->avrcp = 0;
        MessageSend(clientTask, AVRCP_CONNECT_CFM, message);
    }
    else
    {
        AVRCP *avrcp = PanicUnlessNew(AVRCP);
        avrcpInitTaskData(avrcp, clientTask, avrcpReady,
                         config->device_type, 
                         config->supported_controller_features, 
                         config->supported_target_features, 
                         config->profile_extensions, 1);

        avrcpSendInternalConnectReq(avrcp, bd_addr);
    }
}

/****************************************************************************
*NAME    
*    AvrcpConnectResponseLazy    
*
*DESCRIPTION
*  API function to send Connect Response in Lazy Task mode.
*    
*PARAMETERS
*   avrcp            - Task
*   connection_id    - Same ID received in AVRCP_CONNECT_IND
*   accept           - TRUE to accept connection and FALSE to reject.
*   config           - Same initialization parameters used in AvrcpInitLazy()
*
*RETURN
*******************************************************************************/
void AvrcpConnectResponseLazy(  AVRCP   *avrcp, 
                                uint16  connection_id, 
                                bool    accept, 
             const avrcp_init_params    *config)
{
    if (!config)
    {
        /* Client must pass down a valid config. Reject the connection */
        avrcpSendInternalConnectResp(avrcp, connection_id, 0);

        /* Report connect fail and delete the task*/
        avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, avrcp_fail, 0, avrcp);
    }
    else
    {
        /* Finish off task instance initialisation before responding to 
           connect request. */
        avrcpInitTaskData(avrcp, avrcp->clientTask, avrcp->state,
                          config->device_type, 
                          config->supported_controller_features, 
                          config->supported_target_features, 
                          config->profile_extensions, 1);
        avrcpSendInternalConnectResp(avrcp, connection_id, accept);
    }
}


/****************************************************************************
*NAME    
*    AvrcpConnectResponseLazy    
*
*DESCRIPTION
*  API function to send Connect Response
*    
*PARAMETERS
*   avrcp            - Task
*   connection_id    - Same ID received in AVRCP_CONNECT_IND
*   accept           - TRUE to accept connection and FALSE to reject.
*
*RETURN
*******************************************************************************/
void AvrcpConnectResponse(AVRCP *avrcp, uint16 connection_id, bool accept)
{
    avrcpSendInternalConnectResp(avrcp, connection_id, accept);
}


/****************************************************************************
*NAME    
*    AvrcpDisconnect    
*
*DESCRIPTION
*  This function is called to request an AVRCP disconnection.  
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
    AVRCP_DISCONNECT_IND
*******************************************************************************/
void AvrcpDisconnect(AVRCP *avrcp)
{
    MessageSend(&avrcp->task, AVRCP_INTERNAL_DISCONNECT_REQ, 0);
}


/****************************************************************************
NAME    
    AvrcpGetSink    

DESCRIPTION
    This function is called to retrieve the connection Sink.  

PARAMETER RETURNED
    The connection sink. This will be 0 if no connection exists.
******************************************************************************/
Sink AvrcpGetSink(AVRCP *avrcp)
{
    if (!avrcp)
    {
#ifdef AVRCP_DEBUG_LIB
        AVRCP_DEBUG(("AvrcpGetSink NULL AVRCP instance\n"));
#endif
        return (Sink)0;
    }

    return avrcp->sink;
}


/****************************************************************************
*NAME    
*    AvrcpGetSupportedFeatures    
*
*DESCRIPTION
*  This function is used by the CT to retrieve the supported features
*  of the TG. This will include  which category commands are supported, 
*  and if player application settings or group navigation is supported for
*  Metadata extensions.
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
*  AVRCP_GET_SUPPORTED_FEATURES_CFM 
*******************************************************************************/
void AvrcpGetSupportedFeatures(AVRCP *avrcp)
{
    bdaddr my_addr;

    if (SinkGetBdAddr(avrcp->sink, &my_addr))
    {
        MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_GET_FEATURES,
        0, &avrcp->sdp_search_mode);
    }
    else
    {
        avrcpSendGetSupportedFeaturesCfm(avrcp, avrcp_device_not_connected, 0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpGetProfileExtensions    
*
*DESCRIPTION
* This function is used by the CT to retrieve if any profile
*    extensions are available on the TG.
*    At the moment this will just return if AVRCP Metadata extensions at the
*    remote end.
*    
*PARAMETERS
*   avrcp            - Task
*
*MESSAGE RETURNED
*  AVRCP_GET_EXTENSIONS_CFM 
*******************************************************************************/

void AvrcpGetProfileExtensions(AVRCP *avrcp)
{
    bdaddr my_addr;

    if (SinkGetBdAddr(avrcp->sink, &my_addr))
    {
        MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_GET_EXTENSIONS,
        0, &avrcp->sdp_search_mode);
    }
    else
    {
        avrcpSendGetExtensionsCfm(avrcp, avrcp_device_not_connected, 0);    
    }
}


/****************************************************************************
*NAME    
*    AvrcpSourceProcessed    
*
*DESCRIPTION
*   When  the application has finished with the Source data provided
*   by the library, this API MUST be called, otherwise the library won't
*   process any more data arriving until the application calls any other 
*   request or response API.
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
*******************************************************************************/
void AvrcpSourceProcessed(AVRCP *avrcp)
{
    avrcpSourceProcessed(avrcp, FALSE);
}
