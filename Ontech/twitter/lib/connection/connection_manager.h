/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    connection_manager.h
    
DESCRIPTION
    This file contains the type definitions and function prototypes for the 
    Connection Management Entity
*/

#ifndef	CONNECTION_MANAGER_H_
#define	CONNECTION_MANAGER_H_

/* Header files */
#include "connection.h"

#include <stdlib.h>
#include <panic.h>

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/l2cap_prim.h>
#include <app/bluestack/rfcomm_prim.h>


/* Connection type */
typedef enum
{
    conn_l2cap,
    conn_rfcomm
} conn_type;


/* Connection identifier (Key)*/
/* L2CAP connections */
typedef struct
{
	uint16	psm;
    cid_t   cid;
} l2cap_conn_id;

/* RFCOMM connections */
typedef struct
{
    bdaddr			bd_addr;
    mux_id_t        mux_id;
    server_chan_t   server_channel;
} rfcomm_conn_id;

typedef union
{   
    l2cap_conn_id   l2cap_id;
    rfcomm_conn_id  rfcomm_id;
} conn_id;


/* Connection configuration */
/* L2CAP connections */
typedef struct
{
	Task					app_task;
	l2cap_config_params		config;
	uint16					config_half_done;
	l2cap_connect_status	connect_status;
	identifier_t			id;
} l2cap_config;

/* RFCOMM connections */
typedef enum
{
    disconnected,
    mux_phase,
    parneg_phase,
    establish_phase,
    modem_status_phase,
    connected
} rfcomm_state;

typedef struct
{
    Task                    app_task;
    rfcomm_state            state;
    uint16                  maxMtu;
    server_chan_t           remote_server_channel;
    uint8_t				    credit_flow_ctrl;
    rfcomm_config_params    params;
} rfcomm_config;


typedef union
{
    l2cap_config       l2cap;
    rfcomm_config      rfcomm;
} conn_config;



/* Connection instance */
typedef struct conn_instance
{
    conn_type               type;
    conn_id                 id;
    conn_config             config;
	Task					appTask;
	bdaddr					addr;	/* TODO - Can optimise memory here by storing Sink instead of bdaddr */
    struct conn_instance*   next;
} conn_instance;


/* Multiple channels instance. Hold complete connections with multiple channels for one Task. */
typedef struct multiple_channels_instance
{
	Task					appTask;
	cid_t					cid;
	uint16					psm;
	bdaddr					addr;	/* TODO - Can optimise memory here by storing Sink instead of bdaddr */
} multiple_channels_instance;


/* API */
/* Macro to create a connection instance based on type and id */
#define CREATE_L2CAP_CONN_INSTANCE(p, c) conn_instance* conn = PanicUnlessNew(conn_instance); conn->type = conn_l2cap; conn->id.l2cap_id.psm = p; conn->id.l2cap_id.cid = c; conn->next = NULL;
#define CREATE_RFCOMM_CONN_INSTANCE(addr, muxid, chan) conn_instance* conn = PanicUnlessNew(conn_instance); conn->type = conn_rfcomm; conn->id.rfcomm_id.bd_addr = *(bdaddr*)addr; conn->id.rfcomm_id.mux_id = muxid; conn->id.rfcomm_id.server_channel = chan;conn->next = NULL;


/* Macros to create a connection map */
#define CREATE_L2CAP_CONN_ID(p, c) conn_id new_id; new_id.l2cap_id.psm = p; new_id.l2cap_id.cid = c;
#define CREATE_RFCOMM_CONN_ID(addr, chan, muxid) conn_id new_id = connectionGetId((bdaddr*)addr, chan, muxid);


typedef union
{
	uint16			psm;
	server_chan_t	channel;
} map_id;

typedef struct 
{
	conn_type	                type;
    map_id		                id;
    Task                        appTask;
	const profile_task_recipe   *task_recipe;
} conn_task_map;



/****************************************************************************
NAME	
    connectionAddInstance

DESCRIPTION
    This function is called to create a placeholder for connection 
    establishment state information.  This state is only valid and maintained 
    during the connection establishment phase

RETURNS
    Pointer the new connection instance
	
*/
conn_instance* connectionAddInstance(conn_instance* conn_new);


/****************************************************************************
NAME	
    connectionDeleteInstance

DESCRIPTION
    This function is called to remove and free the memory used to store
    connection establishment state information

RETURNS
    TRUE or FALSE indicating if the instance was successfully deleted
	
*/
bool connectionDeleteInstance(conn_type type, conn_id id, const bdaddr *bd_addr);


/****************************************************************************
NAME	
    connectionGetInstance

DESCRIPTION
    This function is called to get a pointer to the connection instance
    identified by type and id

RETURNS
    A pointer to the connection instance identified by type and id, otherwise
    NULL
*/
conn_instance* connectionGetInstance(conn_type type, conn_id id, const bdaddr *bd_addr);


/****************************************************************************
NAME	
    connectionGetInstanceByState

DESCRIPTION
    Retriev an rfcomm connection instance based in the current state

RETURNS
    A pointer to the connection instance identified by the state, otherwise
    NULL
*/
conn_instance* connectionGetInstanceByState(rfcomm_state state);


/****************************************************************************
NAME	
    connectionGetId

DESCRIPTION
    This function is called to get create a instance of a connection ID object

RETURNS
    A connection ID object
*/
conn_id connectionGetId(const bdaddr* bd_addr, server_chan_t channel, mux_id_t mux_id);


/****************************************************************************
NAME	
	connectionAddTaskMap
DESCRIPTION    
	Add an entry into the array keeping track of which task supports which
	psm/ server channel. This is needed so we know where to route 
	connection indications.

RETURNS
	TRUE if new entry added successfully, FALSE otherwise.
*/
bool connectionAddTaskMap(conn_type type, map_id id, Task task, const profile_task_recipe *recipe);


/****************************************************************************
NAME	
	connectionDeleteTaskMap

DESCRIPTION    
	Find the entry that matches the supplied fields and delete it from the 
	array.

RETURNS
	TRUE if entry found and deleted, FALSE otherwise.
*/
bool connectionDeleteTaskMap(conn_type type, map_id id);


/****************************************************************************
NAME	
	connectionUpdateTaskMap

DESCRIPTION    
	Find the entry that matches the supplied fields and update it.

RETURNS
	TRUE if entry found and updated, FALSE otherwise.
*/
bool connectionUpdateTaskMap(conn_type type, map_id id, map_id new_id);


/****************************************************************************
NAME	
	connectionCheckTaskMap

DESCRIPTION    
	Check if the supplied id is in the task map.

RETURNS
	TRUE if id entry found in the task map, FALSE otherwise.
*/
bool connectionCheckIdInTaskMap(conn_type type, map_id id);


/****************************************************************************
NAME	
	connectionFindTaskMap

DESCRIPTION    
	Retrieve a task map based on the id.

RETURNS
	Task map if found, null otherwise.
*/
conn_task_map *connectionFindTaskMap(conn_type type, map_id id);


/****************************************************************************
NAME	
	connectionGetIdFromTaskMap

DESCRIPTION    
	Retrieve the id (l2cap psm or rfcomm server channel) based on the 
    handler function registered for that profile instance.

RETURNS
	Connection id if found, otherwise id set to invalid paarameter.
*/
map_id connectionGetIdFromTaskMap(conn_type type, Task task);


/****************************************************************************
NAME	
	connectionGetTaskFromMap

DESCRIPTION    
	Retrieve the task stored in the map for a given connection identifier
    (rfcomm channel or l2cap psm)

RETURNS
	The associated task if found, or 0.
*/
Task connectionGetTaskFromMap(conn_type type, map_id id);


/****************************************************************************
NAME	
	connectionGetPsmTaskMatch

DESCRIPTION    
	Retrieve the pending connection instance with the required PSM and appTask.

RETURNS
	Number of pending connection instances that match the PSM and Task.
*/
uint16 connectionGetPsmTaskMatch(uint16 psm_local, Task theAppTask);


/****************************************************************************
NAME	
	connectionGetPsmBdaddrMatch

DESCRIPTION    
	Retrieve the pending connection instance with the required PSM and bdaddr.

RETURNS
	The app Task for this pending connection instance.
*/
Task connectionGetPsmBdaddrMatch(uint16 psm_local, bdaddr addr, uint16 *instances);


/****************************************************************************
NAME	
	connectionCompletedConnectionsGetPsmTaskMatch

DESCRIPTION    
	Retrieves the number of completed connection instances with the required PSM and Task.

RETURNS
	The number of matching instances.
*/
uint16 connectionCompletedConnectionsGetPsmTaskMatch(uint16 psm_local, Task theAppTask);


/****************************************************************************
NAME	
	connectionCompletedConnectionsGetPsmBdaddrMatch

DESCRIPTION    
	Retrieves the number of completed connection instances with the required PSM and bdaddr.

RETURNS
	The app Task that the (PSM, addr) pair maps to.
*/
Task connectionCompletedConnectionsGetPsmBdaddrMatch(uint16 psm_local, bdaddr addr, uint16 *instances);


/****************************************************************************
NAME	
	connectionStoreCompletedConnection

DESCRIPTION    
	Stores a connection instance for a successful connection, where more than one channel can map to the same 
	Task (eg. an AVDTP signalling and media channel will use the same A2DP library Task).

RETURNS
	None
*/
void connectionStoreCompletedConnection(Task app_task, bdaddr addr, uint16 psm, cid_t cid);


/****************************************************************************
NAME	
	connectionRemoveCompletedConnection

DESCRIPTION    
	Removes a connection instance for a successful connection, where more than one channel can map to the same 
	Task (eg. an AVDTP signalling and media channel will use the same A2DP library Task).

RETURNS
	None
*/
void connectionRemoveCompletedConnection(cid_t cid);


#endif	/* CONNECTION_MANAGER_H_ */
