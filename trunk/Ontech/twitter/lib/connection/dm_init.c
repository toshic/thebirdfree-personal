/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_init.c        

DESCRIPTION		
    This file contains the functions to initialise the Device Manager 
    component of the connection library

NOTES

*/

/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "dm_init.h"

#include    <vm.h>

/****************************************************************************
NAME
setDefaultLinkPolicy

DESCRIPTION
This Function is called to set the default link policy.  This saves having
to set the link policy for every ACL connection

RETURNS

*/
static void setDefaultLinkPolicy(uint16_t in, uint16 out)
{
	MAKE_PRIM_T(DM_SET_DEFAULT_LINK_POLICY);
	prim->link_policy_settings_in = in;
	prim->link_policy_settings_out = out;
	VmSendDmPrim(prim);
}

/****************************************************************************
NAME
connectionVersionInit

DESCRIPTION
This Function is called on receipt of a DM_AM_REGISTER_CFM and forms the part 
of the DM initiation procedure which sets up the bluetooth version of a dev

RETURNS

*/
void connectionVersionInit (void)
{
	/* Check the HCI version */
	ConnectionReadLocalVersion(connectionGetCmTask());
}

/****************************************************************************
NAME
connectionDmInit

DESCRIPTION
This Function is called to register the Connection Manager with Bluestack.  
The Bluestack message DM_AM_REGISTER_REQ is sent to the Bluestack Device
Manager (DM)

RETURNS

*/
void connectionDmInit(void)
{
     MAKE_PRIM_T(DM_AM_REGISTER_REQ);
     prim->phandle = 0;
     VmSendDmPrim(prim);
	 
	 /* Configure all SCOs to be streams */
	 StreamConfigure(VM_STREAM_SCO_ENABLED, 1);
}


/*****************************************************************************/
void connectionDmInfoInit(void)
{
    /* Set default link policy */
    setDefaultLinkPolicy(ENABLE_MS_SWITCH | ENABLE_SNIFF, ENABLE_MS_SWITCH | ENABLE_SNIFF);

    /* 
        Enable enhanced ACL indications by default 
    
        NOTE: If you see a VM panic at this point please refer to B-10816 in the 
        BlueLab documentation. To avoid this panic either upgrade to firmware that
        supports the BlueStack enhancements primitives or do not use the application
        under xIDE. 
    */
    {
        MAKE_PRIM_T(DM_EN_ENABLE_ENHANCEMENTS_REQ);
	    prim->enhancements = ENHANCEMENT_ACL_INDICATION;
	    VmSendDmPrim(prim);
    }
}
