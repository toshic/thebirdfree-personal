/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_config.c
@brief   Implementation of headset configuration functionality. 
*/


#include "headset_config.h"

#include <panic.h>
#include <ps.h>
#include <stdlib.h>
#include <string.h>

/* Increase this define as new configs are added to default_configs below */
#define LAST_CONFIG_ID	5

/****************************************************************************/

/* Key defintion structure */
typedef struct
{
 	uint16    length;
 	const uint16* data;
}key_type;



/*****************************************************************************/
uint16 get_config_id(uint16 key)
{
 	/* Default to CSR standard configuration */
 	uint16 id = 0;
 
 	/* Read the configuration ID.  This identifies the configuration held in
       constant space */
 	if(PsRetrieve(key, &id, sizeof(uint16)))
 	{
  		if(id >= LAST_CONFIG_ID)
  		{
   			id = 0;
  		}
 	}
 
 	return id;
}


/*****************************************************************************/
uint16 ConfigRetrieve(uint16 config_id, uint16 key_id, void* data, uint16 len)
{
 	uint16 ret_len;
 
 	 	/* Read requested key from PS if it exists */
 	ret_len = PsRetrieve(key_id, data, len);
 
 	return ret_len;
}
