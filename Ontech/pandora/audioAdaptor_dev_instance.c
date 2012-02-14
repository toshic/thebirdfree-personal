/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Maintains a device instance for each remote device the audio adaptor is communicating with.
*/


#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_a2dp.h"
#include "audioAdaptor_cl_msg_handler.h"

#include <stdlib.h>
#include <bdaddr.h>


/****************************************************************************
  LOCAL FUNCTIONS
*/
      
/****************************************************************************
NAME
    devInstanceInit

DESCRIPTION
    Initialise the values for the device instance specified.

*/
static void devInstanceInit(devInstanceTaskData *theDev)
{
    /* Initialise Task handler */
    theDev->task.handler = devInstanceHandleMessage;
    
    /* Initialise A2DP state */    
    theDev->a2dp = NULL;
    theDev->a2dp_state = A2dpStateDisconnected;  
    theDev->a2dp_sig_sink = 0;
    theDev->a2dp_media_sink = 0;
    
    /* Initialise AVRCP state */
    theDev->avrcp = NULL;
    theDev->avrcp_state = AvrcpStateDisconnected;
    
    /* Initialise AGHFP state */
    theDev->aghfp = NULL;
    theDev->aghfp_state = AghfpStateDisconnected;
    
    /* Initialise pin variables */
    theDev->start_pin_idx = 1;
    theDev->pin_idx = 1;
    theDev->pin_requested = FALSE;
    theDev->pin_authorised = FALSE;
    theDev->pin_wrapped = FALSE;
    theDev->pin_list_exhausted = FALSE;
    
    /* Initialise A2DP variables */ 
    theDev->a2dp_seid = 0;
    theDev->a2dp_audio_quality = A2DP_AUDIO_QUALITY_UNKNOWN;
    
    /* Initialise AGHFP variables */ 
    theDev->audio_sink = 0;
    
    /* Initialise profile variables */ 
    theDev->remote_profiles = ProfileNone;
    theDev->responding_profiles = ProfileNone;    
    theDev->paired_list_read = FALSE;
    theDev->available_profiles_connected = FALSE;    
    theDev->a2dp_closing = FALSE;
    theDev->a2dp_reopen = FALSE;    
    theDev->sbc_min_bitpool = SBC_BITPOOL_MIN;
    theDev->sbc_max_bitpool = SBC_BITPOOL_HIGH_QUALITY; 
    theDev->role = hci_role_dont_care;
}


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    devInstanceFindFromBddr

DESCRIPTION
    Returns the device instance associated with the specified Bluetooth address.
    If create_instance is set to TRUE then a new device instance will be created
    if no current one is found.
    If create_instance is set to FALSE then NULL will be returned
    if no current device instance is found.
 
*/
devInstanceTaskData *devInstanceFindFromBddr(const bdaddr *bd_addr, bool create_instance)
{
    devInstanceTaskData *theInst = NULL;
    int instance;
    
    /* Look in table to find entry with matching Bluetooth address */
    for (instance = 0; instance < MAX_NUM_DEV_CONNECTIONS; instance++)
    {
        theInst = the_app->dev_inst[instance];
        if (theInst && BdaddrIsSame(bd_addr, &theInst->bd_addr))
        {
            DEBUG_DEV(("DEV: Retrieved dev instance from bdaddr inst:[0x%x]\n",(uint16)theInst));
            /* cancel any destroy messages */
            MessageCancelAll(&theInst->task, APP_INTERNAL_DESTROY_REQ);

            return theInst;
        }
    }

    theInst = NULL;
    
    if (create_instance)
    {
        /* No match found, create new instance */
        theInst = devInstanceCreate(bd_addr);        
    }       
    
    return theInst;
}


/****************************************************************************
NAME 
    devInstanceFindFromAG

DESCRIPTION
    Returns the device instance associated with the specified AGHFP pointer.
 
*/
devInstanceTaskData *devInstanceFindFromAG(AGHFP *aghfp)
{
    devInstanceTaskData *theInst;
    int instance;
    
    /* Look in table to find entry with matching Bluetooth address */
    for (instance = 0; instance < MAX_NUM_DEV_CONNECTIONS; instance++)
    {
        theInst = the_app->dev_inst[instance];
        if (theInst && (aghfp == theInst->aghfp))
        {
            DEBUG_DEV(("DEV: Retrieved dev instance from AG inst:[0x%x]\n",(uint16)theInst));
            return theInst;
        }
    }

    /* No match found! */
    return NULL;
}


/****************************************************************************
NAME 
    devInstanceCreate

DESCRIPTION
    Creates a new device instance entry if space exists.
 
*/
devInstanceTaskData *devInstanceCreate(const bdaddr *bd_addr)
{
    int instance;
    
    /* Look in table to find empty entry */
    for (instance = 0; instance < MAX_NUM_DEV_CONNECTIONS; instance++)
    {
        if (the_app->dev_inst[instance] == NULL)
        {
            devInstanceTaskData *theInst;
        
            /* Allocate new instance */
            theInst = PanicUnlessNew(devInstanceTaskData);
            the_app->dev_inst[instance] = theInst;
    
            /* Initialise instance */
            devInstanceInit(theInst);

            /* Set Bluetooth address of remote device */
            theInst->bd_addr = *bd_addr;           
    
            DEBUG_DEV(("DEV: Create dev instance inst:[0x%x]\n",(uint16)theInst));
            
            /* Return pointer to new instance */
            return theInst;
        }
    }
    
    DEBUG_DEV(("DEV: Create dev instance - couldn't create!\n"));
    /* No free entry in table */
    return NULL;
}


/****************************************************************************
NAME 
    devInstanceDestroy

DESCRIPTION
    Destroys the specified device instance entry if all connections are idle.
 
*/
void devInstanceDestroy(devInstanceTaskData *theInst)
{
    int instance;
    
    /* Destroy instance once all state machines are disconnected */
    if ((theInst != NULL) && profileSlcAreAllProfilesDisconnected(theInst))
    {
        /* Check there are no profile library instances */
        PanicFalse(theInst->a2dp == NULL);
        PanicFalse(theInst->avrcp == NULL);
        PanicFalse(theInst->aghfp == NULL);

        /* Flush any messages still pending delivery */
        MessageFlushTask(&theInst->task);
    
        /* Look in table to find matching entry */
        for (instance = 0; instance < MAX_NUM_DEV_CONNECTIONS; instance++)
        {
            if (the_app->dev_inst[instance] == theInst)
            {
                /* Clear entry and free instance */
                the_app->dev_inst[instance] = NULL;
                free(theInst);
                
                DEBUG_DEV(("DEV: Destroy dev instance inst:[0x%x]\n",(uint16)theInst));
                
                return;
            }
        }
    
        /* No match found, should really exist here! */
    }
}


/****************************************************************************
NAME 
    devInstanceHandleMessage

DESCRIPTION
    Device instance message handler.
 
*/
void devInstanceHandleMessage(Task task, MessageId id, Message message)
{
    devInstanceTaskData *theInst = (devInstanceTaskData *)task;
    
    /* Route message correctly for this device */ 
    if (A2DP_MESSAGE(id))    
    {
        a2dpMsgHandleInstanceMessage(theInst, id, message);
    } 
    else if (AVRCP_MESSAGE(id))    
    {
        avrcpMsgHandleInstanceMessage(theInst, id, message);
    }
    else if (APP_MESSAGE(id))    
    {
        eventHandleInstanceMessage(theInst, id, message);
    }  
    else
    {
        DEBUG_DEV(("DEV: Unhandled Message %d\n",id));
    }
}
