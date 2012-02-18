/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    connection_manager.c
    
DESCRIPTION
    The Connection Management entity is responsible for holding connection
    state during connection establishment.  As soon as a connection is
    established this state is discarded.
*/


/* Header files */
#include "connection_manager.h"
#include "connection_private.h"
#include "common.h"

#include <bdaddr.h>
#include <string.h>


/* Local pointer to the connection list */
static conn_instance *conn_list_head = NULL;

/* Local pointer to the multiple channel list */
static multiple_channels_instance *multiple_channels_list_head = NULL;
static uint16 multiple_channels_entries = 0;


/* Local ptr to the task id to connection id map */
static conn_task_map *map = 0;
static uint16 mapSize = 0;


/*****************************************************************************/
static bool id_match(const conn_instance* p, conn_type type, conn_id id)
{
    bool match = FALSE;

    if(p != NULL)
    {
        if(p->type == type)
        {
            switch(type)
            {
            case conn_rfcomm:
                /* Searching for an RFCOMM connection instance is complicated as only limited
                   data is available during the early phases of connection establishment.  It's
                   not until the completion of parameter negotiation that the mux id and server 
                   channel have been assigned */
                if(id.rfcomm_id.server_channel == INVALID_SERVER_CHANNEL)
                {
                    /* Invalid Server Channel, match on Bluetooth Device Address and Multiplexer id */
                    if(BdaddrIsSame(&p->id.rfcomm_id.bd_addr, &id.rfcomm_id.bd_addr) || (p->id.rfcomm_id.mux_id == id.rfcomm_id.mux_id))
                    {
                        match = TRUE;
                    }
                    if(BdaddrIsZero(&id.rfcomm_id.bd_addr) && (p->id.rfcomm_id.mux_id == id.rfcomm_id.mux_id))
                    {
                        if(p->id.rfcomm_id.server_channel == 0xff)
                            match = TRUE;
                    }
                }
                else if(id.rfcomm_id.mux_id == INVALID_MUX_ID)
                {
                    /* Invalid Multiplexer ID, match on Bluetooth Address and Server Channel */
                    if (BdaddrIsSame(&p->id.rfcomm_id.bd_addr, &id.rfcomm_id.bd_addr) && (p->id.rfcomm_id.server_channel == id.rfcomm_id.server_channel))
                        match = TRUE;
                }
                else
                {
                    /* Match on Multiplexer ID and Server Channel */
                    if((p->id.rfcomm_id.mux_id == id.rfcomm_id.mux_id && p->id.rfcomm_id.server_channel == id.rfcomm_id.server_channel))
                    {
                        match = TRUE;
                    }
                }
                break;

            case conn_l2cap:
                if(p->id.l2cap_id.cid == id.l2cap_id.cid && id.l2cap_id.cid != L2CA_CID_INVALID)
                    match = TRUE;
				else if ((p->id.l2cap_id.psm == id.l2cap_id.psm) && (id.l2cap_id.psm != L2CA_PSM_INVALID) &&
					(id.l2cap_id.cid != L2CA_CID_INVALID) &&(p->id.l2cap_id.cid == L2CA_CID_INVALID))
					match = FALSE;
                else if (p->id.l2cap_id.psm == id.l2cap_id.psm && id.l2cap_id.psm != L2CA_PSM_INVALID && 
                    p->id.l2cap_id.cid == L2CA_CID_INVALID)
                    match = TRUE;
                break;
            }
        }
    }

    return match;
}


/*****************************************************************************/
static void deleteElement(uint16 match_index)
{
	conn_task_map *new_map;

	/* Reduce the size by one */
	mapSize--;

	/* Realocate the array with one fewer element */
	new_map = (conn_task_map *) PanicNull(malloc(sizeof(conn_task_map) * mapSize));

	/* Copy all the elements apart from the one being deleted */
	if (match_index == 0)
	{
		memcpy(new_map, map+1, sizeof(conn_task_map)*mapSize);
	}
	else
	{
		/* Copy the elements before the one being remove */
		memcpy(new_map, map, sizeof(conn_task_map)*match_index);

		/* Copy the elements after the one being removed if its not the last one */
		if ((mapSize-match_index) > 0)
			memcpy(new_map+match_index, map+match_index+1, sizeof(conn_task_map)*(mapSize-match_index));
	}

	/* Free the old map array */
	free(map);

	/* Need to point to the new one */
	map = new_map;
}


/*****************************************************************************/
conn_instance* connectionAddInstance(conn_instance* conn_new)
{
    conn_new->next = conn_list_head;
    conn_list_head = conn_new;

    return conn_list_head;
}


/*****************************************************************************/
bool connectionDeleteInstance(conn_type type, conn_id id, const bdaddr *bd_addr)
{
    conn_instance*   curr = conn_list_head;
    conn_instance*   prev = curr;

    while(curr != NULL)
    {
        if(id_match(curr, type, id))
        {
   			/* Check address matches if it is specified */
			if (!bd_addr || BdaddrIsSame(&curr->addr, bd_addr))
			{
				/* Check for empty connection list */
				if (!conn_list_head->next)
				{
					/* Connection list is empty set the ptr to null */
					conn_list_head = NULL;
				}
				else
				{
					/* If we're removing the first item in the list update the list ptr */
					if (curr == conn_list_head)
						conn_list_head = curr->next;
					else
						prev->next = curr->next;
				}
			    free(curr);
				return TRUE;
			}
        }
        prev = curr;
        curr = curr->next;
    }

    return FALSE;
}


/*****************************************************************************/
conn_instance* connectionGetInstance(conn_type type, conn_id id, const bdaddr *bd_addr)
{
    conn_instance*   curr = conn_list_head;

    while(curr != NULL)
    {
        if (id_match(curr, type, id))
        {
   			/* Check address matches if it is specified */
			if (!bd_addr || BdaddrIsSame(&curr->addr, bd_addr))
			{
			    break;
			}
        }
        curr = curr->next;
    }

    return curr;
}


/****************************************************************************/
conn_instance* connectionGetInstanceByState(rfcomm_state state)
{
    conn_instance*   curr = conn_list_head;

    while(curr != NULL)
    {
        if((curr->type == conn_rfcomm) && (curr->config.rfcomm.state == state))
        {
            break;
        }
        curr = curr->next;
    }

    return curr;
}


/*****************************************************************************/
conn_id connectionGetId(const bdaddr* bd_addr, uint8 channel, mux_id_t mux_id)
{
    conn_id     id;
    bdaddr   addr;

    if(!bd_addr)
        BdaddrSetZero(&addr);
    else
        addr = *bd_addr;
     
    id.rfcomm_id.bd_addr = addr; 
    id.rfcomm_id.server_channel = channel; 
    id.rfcomm_id.mux_id = mux_id;

    return id;
}

/*****************************************************************************/
static bool findMatchInMap(conn_type type, map_id id, uint16 *index)
{
	uint16 i;

	/* Iterate through the map entries */
	for (i=0; i<mapSize; i++)
	{
		if (map[i].type == type)
		{
			switch(type)
			{
			case conn_l2cap:
				/* If both psm and cid are set make sure we match both */
				if (map[i].id.psm == id.psm)
				{
					*index = i;
					return 1;
				}
				break;

			case conn_rfcomm:
				if (map[i].id.channel == id.channel)
				{
					*index = i;
					return 1;
				}
				break;

			default:
				break;
			}
		}
	}

	/* Match not found */
	return 0;
}


/*****************************************************************************/
bool connectionAddTaskMap(conn_type type, map_id id, Task task, const profile_task_recipe *recipe)
{
	uint16 match_index;

	/* Check to see if we already have this entry if we do return false */
	if (findMatchInMap(type, id, &match_index))
    {
		return FALSE;
    }
	else
	{
		/* Create a new map array */
		conn_task_map new_map;
		new_map.type = type;        
		new_map.id = id;
        new_map.appTask = task;
        new_map.task_recipe = recipe;

		/* Allocate one more element in the array  - initially map_ptr should be null */
		map = (conn_task_map *) PanicNull(realloc(map, (sizeof(conn_task_map) * (mapSize+1))));

		/* Add new element */
		map[mapSize] = new_map;

		/* Don't forget to increment the size count */
		mapSize++;

		/* New element added successfully! */
		return TRUE;
	}
}


/*****************************************************************************/
bool connectionDeleteTaskMap(conn_type type, map_id id)
{
	uint16 match_index;

	if (findMatchInMap(type, id, &match_index))
	{
		/* Found entry */		
		if (mapSize == 1)
		{
			/* Only one element in the array so delete it */
			free(map);
			map = 0;
			mapSize = 0;
		}
		else
		{
			/* Remove element from the array */
			deleteElement(match_index);
		}				

		return TRUE;
	}
	else
		/* Entry not in map so not delelted */
		return FALSE;
}


/*****************************************************************************/
bool connectionUpdateTaskMap(conn_type type, map_id id, map_id new_id)
{
    uint16 match_index;

	if (findMatchInMap(type, id, &match_index))
    {
        /* Update the map entry with the new value */
        map[match_index].id = new_id;
        return TRUE;
    }
    else 
        /* Entry not found in map. */
        return FALSE;
}


/*****************************************************************************/
bool connectionCheckIdInTaskMap(conn_type type, map_id id)
{
    uint16 match_index;

	return (findMatchInMap(type, id, &match_index));
}


/*****************************************************************************/
conn_task_map *connectionFindTaskMap(conn_type type, map_id id)
{
    uint16 match_index;

	if (findMatchInMap(type, id, &match_index))
        return &map[match_index];
    else
        return 0;
}


/*****************************************************************************/
map_id connectionGetIdFromTaskMap(conn_type type, Task task)
{
    uint16 i;
    map_id id;

	/* Iterate through the map entries */
	for (i=0; i<mapSize; i++)
	{
		if (map[i].type == type)
		{
			switch(type)
			{
			case conn_l2cap:
				/* If both psm and cid are set make sure we match both */
				/* TODO implement for l2cap  */
                id.psm = L2CA_PSM_INVALID;
				break;

			case conn_rfcomm:
                {
                    const profile_task_recipe *recipe;
                    id.channel = INVALID_SERVER_CHANNEL;

                    for (recipe = map[i].task_recipe; recipe != NULL; recipe = recipe->parent_profile)
                    {
				        if (recipe->data.handler == task->handler)
                        {
					        id.channel = map[i].id.channel;
                            return id;
                        }
                    }
                }
				break;

			default:
				break;
			}
		}
	}

	return id;
}


/*****************************************************************************/
Task connectionGetTaskFromMap(conn_type type, map_id id)
{
	uint16 match_index = 0;

	if (findMatchInMap(type, id, &match_index))
	{
		/* Return the matching task id */
		return map[match_index].appTask;
	}
	else
	{
		/* Return an invalid task id */
		return 0;
	}
}


/*****************************************************************************/
uint16 connectionGetPsmTaskMatch(uint16 psm_local, Task theAppTask)
{
	conn_instance*   curr = conn_list_head;
	uint16 instances = 0;

    while(curr != NULL)
    {
        if((curr->type == conn_l2cap) && (curr->appTask == theAppTask) && (curr->id.l2cap_id.psm == psm_local))
        {
            instances++;
        }
        curr = curr->next;
    }

    return instances;
}


/*****************************************************************************/
Task connectionGetPsmBdaddrMatch(uint16 psm_local, bdaddr addr, uint16 *instances)
{
	conn_instance*   curr = conn_list_head;
	Task appTask = NULL;
	
	*instances = 0;

    while(curr != NULL)
    {
        if((curr->type == conn_l2cap) && BdaddrIsSame(&curr->addr, &addr) && (curr->id.l2cap_id.psm == psm_local))
        {
            *instances = *instances + 1;
			appTask = curr->appTask;
        }
        curr = curr->next;
    }

    return appTask;
}


/*****************************************************************************/
uint16 connectionCompletedConnectionsGetPsmTaskMatch(uint16 psm_local, Task theAppTask)
{
	multiple_channels_instance*   curr = multiple_channels_list_head;
	uint16 instances = 0;
	uint16 i = 0;

    for (i = 0; i < multiple_channels_entries; i++)
    {
        if((curr->appTask == theAppTask) && (curr->psm == psm_local))
        {
            instances++;
        }
        curr++;
    }

	return instances;
}

/*****************************************************************************/
Task connectionCompletedConnectionsGetPsmBdaddrMatch(uint16 psm_local, bdaddr addr, uint16 *instances)
{
	multiple_channels_instance*   curr = multiple_channels_list_head;
	Task appTask = NULL;
	uint16 i = 0;

	*instances = 0;

    for (i = 0; i < multiple_channels_entries; i++)
    {
        if(BdaddrIsSame(&curr->addr, &addr) && (curr->psm == psm_local))
        {
            *instances = *instances + 1;
			appTask = curr->appTask;
        }
        curr++;
    }

    return appTask;
}


void connectionStoreCompletedConnection(Task app_task, bdaddr addr, uint16 psm, cid_t cid)
{
	conn_task_map *map;
	multiple_channels_instance* conn = NULL;
	multiple_channels_instance data;
    map_id id;

    id.psm = psm;

    /* Get the task map based on the id as this has the task recipe */
    map = connectionFindTaskMap(conn_l2cap, id);

	if (map && map->task_recipe && (map->task_recipe->max_channels > 1))
	{
		/* Add new completed connection */

		if (multiple_channels_list_head == NULL)
		{
			conn = PanicUnlessNew(multiple_channels_instance);
		}
		else
		{	
			conn = (multiple_channels_instance*) realloc(multiple_channels_list_head, sizeof(multiple_channels_instance) * (multiple_channels_entries+1));

			if (!conn)
				Panic();
		}

		data.appTask = app_task;
		data.psm = psm;
		data.addr = addr;
		data.cid = cid;

		memcpy(conn + multiple_channels_entries, &data, sizeof(multiple_channels_instance));

		multiple_channels_entries++;
		multiple_channels_list_head = conn;
	}
}


void connectionRemoveCompletedConnection(cid_t cid)
{
	multiple_channels_instance*   curr = multiple_channels_list_head;

    uint16 i = 0;

    for (i = 0; i < multiple_channels_entries; i++)
    {
        if(curr->cid == cid)
        {
			/* Delete Instance */
		
			/* Check for empty connection list */
			if (multiple_channels_entries == 1)
			{
				/* Connection list is empty set the ptr to null */
				free(multiple_channels_list_head);
				multiple_channels_list_head = NULL;
				multiple_channels_entries = 0;
			}
			else
			{
				if (i != (multiple_channels_entries - 1))
					memcpy(multiple_channels_list_head + i, multiple_channels_list_head + i + 1, sizeof(multiple_channels_instance) * (multiple_channels_entries-i-1)); 

				multiple_channels_entries--;

				curr = (multiple_channels_instance*) realloc(multiple_channels_list_head, sizeof(multiple_channels_instance) * multiple_channels_entries);

				multiple_channels_list_head = curr;
			}
		
            return;
        }
	
        curr++;
    }
}

