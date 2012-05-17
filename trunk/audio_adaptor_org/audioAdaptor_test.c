/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Test mode functionality.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_test.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_statemanager.h"

#include <pio.h>


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    testCancelDfuModeRequest

DESCRIPTION
    Cancel entering DFU mode.
    
*/
void testCancelDfuModeRequest (void)
{
    if (the_app->app_state == AppStateEnteringDfu)
    {
        MessageCancelAll(&the_app->task, APP_ENTER_DFU_MODE);
        profileSlcStartConnectionProcess();
    }
}


/****************************************************************************
NAME
    testEnterDutMode

DESCRIPTION
    Enter DFU mode immediately.
    
*/
bool testEnterDutMode (void)
{
    if ( (PioGet() & DUT_PIO) == DUT_PIO )
    {   /* PIO line to enter DUT mode is asserted */
        MessageCancelAll(&the_app->task, APP_ENTER_DFU_MODE);
        setAppState(AppStateUninitialised);
        ConnectionEnterDutMode();
        return TRUE;
    }
    
    return FALSE;
}
