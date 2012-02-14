/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    spp_private.h
    
DESCRIPTION
	Header file for the SPP profile library containing private members
	
*/

#ifndef SPP_PRIVATE_H_
#define SPP_PRIVATE_H_

#include "spp.h"

#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>
#include <stdlib.h>


#ifdef SPP_DEBUG_LIB
#include <stdio.h>
#define SPP_DEBUG(x) {printf x; Panic();}
#define SPP_PRINT(x) {printf x;}
#else
#define SPP_DEBUG(x)
#define SPP_PRINT(x)
#endif


/* Macro for creating messages */
#define MAKE_SPP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);


/* SPP Profile Library private messages */
#define SPP_MSG_BASE     (0x0)

/* Enum for internal messages */
enum
{
    SPP_INTERNAL_TASK_INIT_REQ = SPP_MSG_BASE,
    SPP_INTERNAL_TASK_DELETE_REQ,
    SPP_INTERNAL_INIT_REQ,
    SPP_INTERNAL_CONNECT_REQ, 
	SPP_INTERNAL_CONNECT_RES,
	SPP_INTERNAL_RFCOMM_CONNECT_REQ,
	SPP_INTERNAL_DISCONNECT_REQ, 
	SPP_INTERNAL_SEND_CFM_TO_APP
};


typedef struct
{
    Task                        connectionTask;
    const profile_task_recipe   *recipe;
} SPP_INTERNAL_INIT_REQ_T;


typedef struct
{
	bdaddr	      addr;
    uint16        length_service_search_pattern;
    const uint8   *service_search_pattern;
	uint16        max_frame_size;
	uint8         rfcomm_channel_number;
} SPP_INTERNAL_CONNECT_REQ_T;


typedef struct
{
	bdaddr	addr;
	bool	response;
	uint16	max_frame_size;
} SPP_INTERNAL_CONNECT_RES_T;


typedef struct
{
	bdaddr	addr;
	uint8	rfc_channel;
} SPP_INTERNAL_RFCOMM_CONNECT_REQ_T;

typedef struct
{
    spp_connect_status status;
	SPP *spp;
} SPP_INTERNAL_SEND_CFM_TO_APP_T;

typedef enum 
{
	sppInitialising,
	sppReady,		
    sppSearching,
	sppConnecting,			
	sppConnected			
} sppState;


/* SPP profile library instance data structure */
struct __SPP
{
	TaskData				task;

    /* Task SPP messages are sent to - application or client library. */
	Task					clientTask;

    /* Local state of the SPP instance */
	sppState				state;

    /* SPP service record handle as issued by BlueStack. */
	uint32					sdp_record_handle;

    /* SPP library instance priority for link policy voting. */
    uint16                  priority;

    /* Sink identifying the RFCOMM connection */
	Sink					sink;

    /* Local RFCOMM server channel the SPP instance is registered on. */
    uint8                   local_server_channel;
	
    /* Identifies whether the SPP library is running in lazy task mode. */
    uint16                  lazy;

    /* Length of the user supplied service record (or null for default). */
    uint16                  length_sr;

    /* Ptr to the user supplied service record (or null for default). */
    const uint8             *sr;
	
	/* The frame size to use (or null for default) */
    uint16					max_frame_size;
};

/****************************************************************************
NAME	
	sppProfileHandler

DESCRIPTION
	All messages for this profile library are handled by this function
*/
void sppProfileHandler(Task task, MessageId id, Message message);


#endif /* SPP_PRIVATE_H_ */
