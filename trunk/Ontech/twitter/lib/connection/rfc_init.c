/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    rfc_init.c        

DESCRIPTION
    This file contains the functions to initialise the RFCOMM component of 
    the connection library	

NOTES

*/

/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "rfc_init.h"

#include    <vm.h>


/****************************************************************************
NAME
connectionRfcInit

DESCRIPTION
This Function is called to initialise the RFCOMM protocol layer in Bluestack

RETURNS

*/
void connectionRfcInit(void)
{
	/* Send the RFCOMM init message. */
	{
		MAKE_PRIM_T(RFC_INIT_REQ);
		prim->phandle = 0;
		prim->psm_local = RFCOMM_PSM;
		prim->use_flow_control = 1;
		prim->fc_type = 1;
		prim->fc_threshold = 0;
		prim->fc_timer = 1000;
		prim->rsvd_4 = 0;
		prim->rsvd_5 = 0;
		VmSendRfcommPrim(prim);
	}
}
