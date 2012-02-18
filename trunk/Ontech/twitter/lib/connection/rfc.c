/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    rfc.c        

DESCRIPTION
	File containing the rfcomm API function implementations.		

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"


/* 
	RFCOMM Flag Defines
	These flags are used in the establishment of an RFCOMM connection.
*/

/* Bit mask with value of RFCOMM break signal. */
#define DEFAULT_RFCOMM_BREAK_SIGNAL         (0)

/* Bit mask with value of RFCOMM modem status signal. */
#define DEFAULT_RFCOMM_MODEM_STATUS_SIGNAL  (0)
/* Bit mask with value of RFCOMM flow control. */
#define DEFAULT_RFCOMM_FLOW_CTRL_MASK       (0)
/* Bit mask with value of RFCOMM parameter mask. */
#define DEFAULT_RFCOMM_PARAMETER_MASK       (PM_BIT_RATE | PM_DATA_BITS | PM_STOP_BITS | PM_PARITY | PM_PARITY_TYPE | PM_XON | PM_XOFF | PM_XONXOFF_INPUT | PM_XONXOFF_OUTPUT | PM_RTR_INPUT | PM_RTR_OUTPUT | PM_RTC_INPUT | PM_RTC_OUTPUT)



/****************************************************************************

DESCRIPTION
	This function is called to initialise the RFCOMM configuration parameters
*/
static void initConfigParams(rfcomm_config_params *config)
{
    config->max_frame_size = RFCOMM_DEFAULT_FRAME_SIZE;
    config->break_signal = DEFAULT_RFCOMM_BREAK_SIGNAL;
    config->modem_status = DEFAULT_RFCOMM_MODEM_STATUS_SIGNAL;
    config->timeout = D_SEC(DEFAULT_RFCOMM_CONNECTION_TIMEOUT);
    config->request = TRUE;
    config->port_params.port_speed = PORT_SPEED_UNUSED;
    config->port_params.data_bits = DATA_BITS_8;
    config->port_params.stop_bits = STOP_BITS_ONE;
    config->port_params.parity = PARITY_ON;
    config->port_params.parity_type = PARITY_TYPE_ODD;
    config->port_params.flow_ctrl_mask = DEFAULT_RFCOMM_FLOW_CTRL_MASK;
    config->port_params.xon = XON_CHAR_DEFAULT;
    config->port_params.xoff = XOFF_CHAR_DEFAULT;
    config->port_params.parameter_mask = DEFAULT_RFCOMM_PARAMETER_MASK;
}


/*****************************************************************************/
static void sendInternalRfcommRegisterMessage(Task appTask, Task connectTask, const profile_task_recipe *recipe)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_REGISTER_REQ);
    message->theAppTask = appTask;
    message->theConnectTask = connectTask;
	message->task_recipe = recipe;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_REGISTER_REQ, message);
}


/*****************************************************************************/
void ConnectionRfcommAllocateChannel(Task theAppTask)
{
    /* Send an internal message */
	sendInternalRfcommRegisterMessage(theAppTask, 0, 0);
}


/*****************************************************************************/
void ConnectionRfcommAllocateChannelLazy(Task clientTask, Task connectionTask, const profile_task_recipe *recipe)
{
	/* Send an internal message */
    sendInternalRfcommRegisterMessage(clientTask, connectionTask, recipe);
}


/*****************************************************************************/
void ConnectionRfcommConnectRequest(Task theAppTask, const bdaddr* bd_addr, uint8 local_server_chan, uint8 remote_server_chan, const rfcomm_config_params *config)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif
    {
	    /* Send an internal message */
	    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_REQ);
	    message->theAppTask = theAppTask;
        message->bd_addr = *bd_addr;
        message->remote_server_channel = remote_server_chan;
        message->local_server_channel = local_server_chan;

        /* Copy configuration parameters */
        if(config)
        {
            message->config = *config;
        }
        else
        {
            /* The supplied config pointer is NULL, load default RFCOMM parameters */
            initConfigParams(&message->config);
        }
        MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionRfcommConnectResponse(Task theAppTask, bool response, const bdaddr* bd_addr, uint8 local_server_channel, const rfcomm_config_params *config)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif

    {
	    /* Send an internal message */
	    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_RES);
	    message->theAppTask = theAppTask;
        message->response = response;
        message->bd_addr = *bd_addr;
        message->server_channel = local_server_channel;

        if(config)
        {
            message->config = *config;
        }
        else
        {
            /* The supplied config pointer is NULL, load default RFCOMM parameters */
            initConfigParams(&message->config);
        }
        MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_RES, message);
    }
}


/*****************************************************************************/
void ConnectionRfcommDisconnectRequest(Task appTask, Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_DISCONNECT_REQ);
	message->theAppTask = appTask;
    message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_DISCONNECT_REQ, message);
}


/*****************************************************************************/
void ConnectionRfcommControlSignalRequest(Task appTask, Sink sink, uint16 break_signal, uint16 modem_signal)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONTROL_REQ);
	message->theAppTask = appTask;
    message->sink = sink;
    message->break_signal = break_signal;
    message->modem_signal = modem_signal;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONTROL_REQ, message);
}

