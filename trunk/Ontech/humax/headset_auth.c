/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_auth.c        
@brief    Implementation of the authentication functionality for the stereo headset application
*/

/****************************************************************************
    Header files
*/

#include "headset_a2dp_connection.h"
#include "headset_auth.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_statemanager.h"
#include "uart.h"

#include "panic.h"
#include <connection.h>
#include <ps.h>
#include <stdlib.h>

#ifdef DEBUG_AUTH
    #define AUTH_DEBUG(x) DEBUG(x)    
#else
    #define AUTH_DEBUG(x) 
#endif   


/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

static bool AuthCanHeadsetConnect ( void ) ;

static bool AuthCanHeadsetPair ( void ) ;


/****************************************************************************
  FUNCTIONS
*/

/**************************************************************************/
void headsetHandlePinCodeInd(const CL_SM_PIN_CODE_IND_T* ind)
{
    uint16 pin_length = 0;
    uint8 pin[16];
    
    if ( AuthCanHeadsetPair () )
    {
	    
		AUTH_DEBUG(("auth: Can Pin\n")) ;
			
    	/* Do we have a fixed pin in PS, if not reject pairing */
    	if ((pin_length = PsFullRetrieve(PSKEY_FIXED_PIN, pin, 16)) == 0 || pin_length > 16)
    	{
        	/* Set length to 0 indicating we're rejecting the PIN request */
        	AUTH_DEBUG(("auth : failed to get pin\n")) ;
        	pin_length = 0; 
    	}	
	} 
    /* Respond to the PIN code request */
    ConnectionSmPinCodeResponse(&ind->bd_addr, pin_length, pin); 
}


/*****************************************************************************/
void headsetHandleAuthoriseInd(const CL_SM_AUTHORISE_IND_T *ind)
{
	
	bool lAuthorised = FALSE ;
	
	if ( AuthCanHeadsetConnect() )
	{
		lAuthorised = TRUE ;
	}
	
	AUTH_DEBUG(("auth: Authorised [%d]\n" , lAuthorised)) ;
	    
    /*complete the authentication with the authorised or not flag*/		
    ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, lAuthorised);
}


/*****************************************************************************/
void headsetHandleAuthenticateCfm(const CL_SM_AUTHENTICATE_CFM_T *cfm)
{
    /* Leave bondable mode if successful unless we got a debug key */
    if (cfm->status == auth_status_success && cfm->key_type != cl_sm_link_key_debug)
    {
        /* Send a user event to the app for indication purposes */
        MessageSend (&theHeadset.task , EventPairingSuccessful , 0 );
    }
	
	/* Set up some default params and shuffle PDL */
	if(cfm->bonded)
	{	
		uint8 lAttributes[ATTRIBUTE_SIZE];
		/* Default volume set */
		lAttributes[attribute_hfp_volume] = theHeadset.gHfpVolumeLevel;	
		lAttributes[attribute_a2dp_volume] = theHeadset.gAvVolumeLevel;
		/* No profile set */
		lAttributes[attribute_hfp_hsp_profile] = 0;	
		lAttributes[attribute_a2dp_profile] = 0;	
		/* No SEID */
		lAttributes[attribute_seid] = 0;
		/* No clock rate mismatch */
		lAttributes[attribute_clock_mismatch] = 0;
	
		/* Write params to PS */
		ConnectionSmPutAttribute(PSKEY_ATTRIBUTE_BASE, &cfm->bd_addr, ATTRIBUTE_SIZE, lAttributes); 
		/* Shuffle the PDL around the device */
		ConnectionSmUpdateMruDevice(&cfm->bd_addr);
		
	}

    /* reset pairing info if we timed out on confirmation */
    AuthResetConfirmationFlags();
}


/*****************************************************************************
NAME    
     headsetHandleUserConfirmationInd
    
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_CONFIRMATION_REQ_IND

RETURNS
     void
*/
void headsetHandleUserConfirmationInd(const CL_SM_USER_CONFIRMATION_REQ_IND_T* ind)
{
    if (AuthCanHeadsetPair())
    {
#if 1  
        theHeadset.confirmation = TRUE;
        AUTH_DEBUG(("auth: can confirm %ld\n", ind->numeric_value));
        /* should use text to speech here */
        theHeadset.confirmation_addr = (bdaddr*)PanicUnlessMalloc(sizeof(bdaddr));
        *theHeadset.confirmation_addr = ind->bd_addr;
#else
        ConnectionSmUserConfirmationResponse(&ind->bd_addr, TRUE);
        /* infor ssp numeric_value */
        UartPrintf("\r\n+SSP=%ld\r\n",ind->numeric_value);
#endif        
    }
    else
    {
        /* reject the confirmation request */
        AUTH_DEBUG(("auth: rejecting confirmation req\n"));
        ConnectionSmUserConfirmationResponse(&ind->bd_addr, FALSE);
    }
}


/*****************************************************************************
NAME    
     headsetHandleUserPasskeyInd
    
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_PASSKEY_IND

RETURNS
     void
*/
void headsetHandleUserPasskeyInd(const CL_SM_USER_PASSKEY_REQ_IND_T* ind)
{
    /* reject the passkey request */
    AUTH_DEBUG(("auth: rejecting passkey req\n"));
    ConnectionSmUserPasskeyResponse(&ind->bd_addr, TRUE, 0);
}


/*****************************************************************************
NAME    
     headsetHandleUserPasskeyNotificationInd
    
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_PASSKEY_NOTIFICATION_IND

RETURNS
     void
*/
void headsetHandleUserPasskeyNotificationInd(const CL_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind)
{
    AUTH_DEBUG(("auth: Passkey [%ld]\n", ind->passkey));
}


/*****************************************************************************
NAME    
     headsetHandleIoCapabilityInd
    
DESCRIPTION
     This function is called on receipt on an CL_SM_IO_CAPABILITY_REQ_IND

RETURNS
     void 
*/
void headsetHandleIoCapabilityInd(const CL_SM_IO_CAPABILITY_REQ_IND_T* ind)
{
    /* Only send IO capabilities if we are pairable */
    if (AuthCanHeadsetPair())
    {
        cl_sm_io_capability local_io_caps = cl_sm_io_cap_display_yes_no;/*cl_sm_io_cap_no_input_no_output;*/
        AUTH_DEBUG(("auth: sending IO capability\n"));
        ConnectionSmIoCapabilityResponse(&ind->bd_addr, local_io_caps, FALSE, TRUE, FALSE, 0, 0);
    }
    else /* send a reject response */
    {
        AUTH_DEBUG(("auth: rejecting IO capability req\n"));
        ConnectionSmIoCapabilityResponse(&ind->bd_addr, cl_sm_reject_request, FALSE, FALSE, FALSE, 0, 0);
    }
}


/*****************************************************************************
NAME    
     headsetHandleRemoteIoCapabilityInd
    
DESCRIPTION
     This function is called on receipt on an CL_SM_REMOTE_IO_CAPABILITY_IND
 
RETURNS
     void 
*/
void headsetHandleRemoteIoCapabilityInd(const CL_SM_REMOTE_IO_CAPABILITY_IND_T* ind)
{
    AUTH_DEBUG(("auth: Incoming Authentication Request\n"));
}


/****************************************************************************
NAME    
    AuthCanHeadsetPair 
    
DESCRIPTION
    Helper function to indicate if pairing is allowed.

RETURNS
    bool
*/
static bool AuthCanHeadsetPair ( void )
{
	bool lCanPair = FALSE ;
	
	lCanPair = TRUE ;
 
    return lCanPair ;
}


/****************************************************************************
NAME    
    AuthCanHeadsetConnect 
    
DESCRIPTION
    Helper function to indicate if connecting is allowed.

RETURNS
    bool
*/
static bool AuthCanHeadsetConnect ( void )
{
	bool lCanConnect = FALSE ;
	
	/* If the headset is on then authorise */
	if (stateManagerGetHfpState() != headsetPoweringOn)
	{
		lCanConnect = TRUE ;
		AUTH_DEBUG(("auth: headset is on\n")) ;
	}
		
    return lCanConnect ;
}


/****************************************************************************
NAME    
    headsetPairingAcceptRes 
    
DESCRIPTION
    Respond correctly to a pairing info request ind

RETURNS
    void
*/
void headsetPairingAcceptRes(void)
{
    if (AuthCanHeadsetPair() && theHeadset.confirmation)
    {
        AUTH_DEBUG(("auth: accepted confirmation req\n"));
        ConnectionSmUserConfirmationResponse(theHeadset.confirmation_addr, TRUE);
    }
    else
    {
        AUTH_DEBUG(("auth: invalid state for confirmation\n"));
    }
}

/****************************************************************************
NAME    
    headsetPairingRejectRes 
    
DESCRIPTION
    Respond reject to a pairing info request ind

RETURNS
    void
*/
void headsetPairingRejectRes(void)
{
    if (AuthCanHeadsetPair() && theHeadset.confirmation)
    {
        AUTH_DEBUG(("auth: rejected confirmation req\n"));
        ConnectionSmUserConfirmationResponse(theHeadset.confirmation_addr, FALSE);
    }
    else
    {
        AUTH_DEBUG(("auth: invalid state for confirmation\n"));
    }
}


/****************************************************************************
NAME    
    AuthResetConfirmationFlags
    
DESCRIPTION
    Helper function to reset the confirmations flag and associated BT address

RETURNS
    void 
*/
void AuthResetConfirmationFlags(void)
{
    AUTH_DEBUG(("auth: reset confirmation flags\n"));
    if (theHeadset.confirmation_addr != NULL)
    {
        AUTH_DEBUG(("auth: free confirmation addr\n"));
        free(theHeadset.confirmation_addr);
    }
    theHeadset.confirmation_addr = NULL;
    theHeadset.confirmation = FALSE;
}



/****************************************************************************
NAME    
    AuthGetPDLEntries
    
DESCRIPTION
    Returns the number of paired device list (PDL) entries

RETURNS
    bool 
*/
bool AuthGetPDLEntries(void)
{
	uint16 i;
	uint16 lNumDevices = 0;
	uint16 ltdi[sizeof(uint16) * MAX_PAIRED_DEVICES];
	
	if (PsRetrieve(41 , &ltdi , sizeof(uint16) * MAX_PAIRED_DEVICES))
 	{
		for (i = 0 ; i < MAX_PAIRED_DEVICES ; i ++)
    	{
    		if (ltdi[i] > lNumDevices )
    		{
    			lNumDevices = ltdi[i];
    		}
    	}
	}
	
	return lNumDevices;
}
