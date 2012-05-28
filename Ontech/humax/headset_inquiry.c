/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_inquiry.c
@brief   Handles headset proximity pairing.
*/

#include "headset_a2dp_connection.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_inquiry.h"
#include "headset_statemanager.h"

#include <bdaddr.h>
#include <hfp.h>
#include <panic.h>
#include <ps.h>
#include <stdlib.h>


#ifdef DEBUG_INQUIRY
#define INQUIRY_DEBUG(x) DEBUG(x)
#else
#define INQUIRY_DEBUG(x) 
#endif


#define INQUIRY_LAP						0x9E8B33
#define INQUIRY_MAX_RESPONSES			16
#define INQUIRY_TIMEOUT					5
#define INQUIRY_WAIT_TIME_SEC			2
#define INQUIRY_REMINDER_TIMEOUT_SECS	5


/****************************************************************************
  FUNCTIONS
*/


/*****************************************************************************/
void inquiryClearData(void)
{
    if(theHeadset.inquiry_data)
    {
		INQUIRY_DEBUG(("INQUIRY: inquiryClearData, set RSSI %d\n",theHeadset.config->rssi.threshold));
        /* Reset address & peak RSSI value */
        BdaddrSetZero(&theHeadset.inquiry_data[0].bd_addr);
        BdaddrSetZero(&theHeadset.inquiry_data[1].bd_addr);
        theHeadset.inquiry_data[0].rssi = theHeadset.inquiry_data[1].rssi = theHeadset.config->rssi.threshold;
    }
}


/*****************************************************************************/
void inquiryContinue(void)
{
	if(theHeadset.inquiry_data)
    {
		inquiryClearData();
	
		/* Decide if to change between HFP device inquiry and A2DP device inquiry */
		if (theHeadset.inquiry_data[0].hfp_inquiry)
		{
			theHeadset.inquiry_data[0].hfp_inquiry = FALSE;
			INQUIRY_DEBUG(("INQUIRY: Inquiry Continue - A2DP device\n"));
			ConnectionInquire(&theHeadset.task, INQUIRY_LAP, INQUIRY_MAX_RESPONSES, INQUIRY_TIMEOUT, theHeadset.config->rssi.a2dp_cod_filter);
			return;
		}
	
		theHeadset.inquiry_data[0].hfp_inquiry = TRUE;
		INQUIRY_DEBUG(("INQUIRY: Inquiry Continue - HFP device\n"));
		ConnectionInquire(&theHeadset.task, INQUIRY_LAP, INQUIRY_MAX_RESPONSES, INQUIRY_TIMEOUT, theHeadset.config->rssi.hfp_cod_filter);
	}
}
		

/*****************************************************************************/
void inquiryStart( void )
{
    if(!theHeadset.inquiry_data)
    {
        INQUIRY_DEBUG(("INQUIRY: Start Inquiry\n"));
        
        /* Go discoverable (and disconnect any active connections) */
        stateManagerEnterConnDiscoverableState() ;
        
        /* Allocate space to store inquiry results */
        theHeadset.inquiry_data = PanicUnlessMalloc(2 * sizeof(inquiry_data_t));
		
		/* If HFP profile is enabled then try and find HFP devices first */ 
		theHeadset.inquiry_data[0].hfp_inquiry = TRUE;

        inquiryClearData();
	
		/* Decide if to start with HFP device inquiry or A2DP device inquiry */
		if (theHeadset.inquiry_data[0].hfp_inquiry)
		{
			INQUIRY_DEBUG(("INQUIRY: Inquiry Start - HFP device\n"));
			ConnectionInquire(&theHeadset.task, INQUIRY_LAP, INQUIRY_MAX_RESPONSES, INQUIRY_TIMEOUT, theHeadset.config->rssi.hfp_cod_filter);
		}
		else
		{
			INQUIRY_DEBUG(("INQUIRY: Inquiry Start - A2DP device\n"));
			ConnectionInquire(&theHeadset.task, INQUIRY_LAP, INQUIRY_MAX_RESPONSES, INQUIRY_TIMEOUT, theHeadset.config->rssi.a2dp_cod_filter);
		}

        /* Send a reminder event */
        inquiryReminder();

        /* Send timeout if enabled */
        if(theHeadset.Timeouts.InquiryTimeout_s)
            MessageSendLater(&theHeadset.task, EventRssiPairTimeout, 0, D_SEC(theHeadset.Timeouts.InquiryTimeout_s));
    }
}

/*****************************************************************************/
void inquiryStop( void )
{
    INQUIRY_DEBUG(("INQUIRY: Inquiry Stop\n"));
    
    if(theHeadset.inquiry_data)
    {
		/* Free space used for inquiry results */
        free(theHeadset.inquiry_data);
        theHeadset.inquiry_data = NULL;

        /* Cancel the inquiry */
        INQUIRY_DEBUG(("INQUIRY: Cancel Inquiry\n"));
        ConnectionInquireCancel(&theHeadset.task);
        MessageCancelAll(&theHeadset.task, CL_DM_INQUIRE_RESULT);
		MessageCancelFirst(&theHeadset.task, APP_INQUIRY_CONTINUE);
        MessageCancelFirst(&theHeadset.task, EventRssiPairReminder);
        if(theHeadset.Timeouts.InquiryTimeout_s)
            MessageCancelFirst(&theHeadset.task, EventRssiPairTimeout);
    }
}

/*****************************************************************************/
void inquiryHandleResult( CL_DM_INQUIRE_RESULT_T* result )
{
    /* Check inquiry data is valid (if not we must have cancelled) */
    if(theHeadset.inquiry_data)
    {
        INQUIRY_DEBUG(("INQUIRY: Inquiry Result %x Addr %x,%x,%lx RSSI: %d\n", result->status,
                                                                       result->bd_addr.nap,
                                                                       result->bd_addr.uap,
                                                                       result->bd_addr.lap, 
                                                                       result->rssi));

        if(result->status == inquiry_status_result)
        {
            /* Cache result if RSSI is higher */
			INQUIRY_DEBUG(("INQUIRY: result RSSI %d  stored RSSI %d\n",result->rssi,theHeadset.inquiry_data[0].rssi));
            if (result->rssi > theHeadset.inquiry_data[0].rssi)
            {
                /* Check if address is different from previous peak */          
                if (!BdaddrIsSame(&result->bd_addr, &theHeadset.inquiry_data[0].bd_addr))
                {
					INQUIRY_DEBUG(("INQUIRY: Store previous peak RSSI %d\n",theHeadset.inquiry_data[1].rssi));
								   
                    /* Store previous peak RSSI */
                    theHeadset.inquiry_data[1].rssi    = theHeadset.inquiry_data[0].rssi;
                    theHeadset.inquiry_data[1].bd_addr = theHeadset.inquiry_data[0].bd_addr;
                
                    /* Store new address */        
                    theHeadset.inquiry_data[0].bd_addr = result->bd_addr;
                }
         
                /* Store peak RSSI */				
                theHeadset.inquiry_data[0].rssi = result->rssi;
				INQUIRY_DEBUG(("INQUIRY: Store peak RSSI %d\n",theHeadset.inquiry_data[0].rssi));
            }
            else if (result->rssi > theHeadset.inquiry_data[1].rssi)
            {
                /* Check if address is different from peak */          
                if (!BdaddrIsSame(&result->bd_addr, &theHeadset.inquiry_data[0].bd_addr))
                {
                    /* Store next highest RSSI */
                    theHeadset.inquiry_data[1].rssi = result->rssi;
					INQUIRY_DEBUG(("INQUIRY: Store next highest RSSI %d\n",theHeadset.inquiry_data[1].rssi));
                }
            }     
        }
        else
        {
            INQUIRY_DEBUG(("INQUIRY: Inquiry Complete, 0 RSSI %d 1 RSSI %d diff %d\n",theHeadset.inquiry_data[0].rssi,theHeadset.inquiry_data[1].rssi,(int)theHeadset.config->rssi.diff_threshold));
            /* Inquiry finished - Check if RSSI peak is sufficently higher than next */	
            if ((theHeadset.inquiry_data[0].rssi - theHeadset.inquiry_data[1].rssi) >= (int)(theHeadset.config->rssi.diff_threshold))
            {
                INQUIRY_DEBUG(("INQUIRY: Inquire Connect Addr %x,%x,%lx RSSI: %d\n", theHeadset.inquiry_data[0].bd_addr.nap,
                                                                             theHeadset.inquiry_data[0].bd_addr.uap,
                                                                             theHeadset.inquiry_data[0].bd_addr.lap, 
                                                                             theHeadset.inquiry_data[0].rssi));                																		

                /* Attempt to connect to HFP device */
				if (theHeadset.inquiry_data[0].hfp_inquiry)
				{
					if (hfpSlcConnectBdaddrRequest(hfp_handsfree_profile, &theHeadset.inquiry_data[0].bd_addr))
					{
						/* Ensure the link key for this device is deleted before the connection attempt,
		 				   prevents reconnection problems with BT2.1 devices.*/
						ConnectionSmDeleteAuthDevice(&theHeadset.inquiry_data[0].bd_addr);	
						INQUIRY_DEBUG(("INQUIRY: Connect HFP\n"));
						return;
					}
				}		
				/* This was an A2DP inquiry so try and connect A2DP */
				else if (a2dpConnectBdaddrRequest(&theHeadset.inquiry_data[0].bd_addr,FALSE))
				{
					/* Ensure the link key for this device is deleted before the connection attempt,
	 				   prevents reconnection problems with BT2.1 devices.*/
					ConnectionSmDeleteAuthDevice(&theHeadset.inquiry_data[0].bd_addr);	
					INQUIRY_DEBUG(("INQUIRY: Connect A2DP\n"));
					return;
				}			                
            }
           
			/* Wait a specified time before continuing with a new inquiry */
			MessageSendLater(&theHeadset.task, APP_INQUIRY_CONTINUE, 0, D_SEC(INQUIRY_WAIT_TIME_SEC));				                
       
        }
    }
}


void inquiryReminder(void)
{
	MessageSendLater(&theHeadset.task, EventRssiPairReminder, 0, D_SEC(INQUIRY_REMINDER_TIMEOUT_SECS));	
}
