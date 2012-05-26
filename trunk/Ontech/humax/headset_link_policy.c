/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file	headset_link_policy.c
@brief  Implementation of headset link policy control functions.
*/
#include <connection.h>


/****************************************************************************
	Header files
*/
#include "headset_a2dp_stream_control.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_link_policy.h"
#include "headset_private.h"
#include "headset_statemanager.h"

#include <bdaddr.h>
#include <sink.h>


#ifdef DEBUG_LINK_POLICY
#define POLICY_DEBUG(x) DEBUG(x)
#else
#define POLICY_DEBUG(x) 
#endif   


/* Lower power table for the HFP. */
static const lp_power_table hfp_powertable[]=
{
	/* mode,    	min_interval, max_interval, attempt, timeout, duration */
	{lp_passive,	0,	    	  0,			0,		 0,		30},    /* Active mode for 1 second */
    {lp_sniff,		800,	      800,			1,		 8,	    0 }     /* Enter sniff mode (500mS)*/
};

/* Lower power table for the HFP when an audio connection is open */
static const lp_power_table hfp_powertable_sco[]=
{
	/* mode,    	min_interval, max_interval, attempt, timeout, duration */
	{lp_passive,		0,	    	  0,			0,		 0,		30},   /*Passive mode */
    {lp_sniff,		  160,	        160,			1,		 8,	    0}     /* Enter sniff mode (500mS)*/
};

/* Lower power table for the A2DP. */
static const lp_power_table a2dp_signalling_powertable[]=
{
	/* mode,    	min_interval, max_interval, attempt, timeout, duration */
	{lp_active,		0,	    	  0,			0,		 0,		1},    /* Active mode for 1 sec */	
	{lp_passive,	0,	    	  0,			0,		 0,		4},    /* Passive mode for 4 secs */	
	{lp_sniff,		32,	      	  200,			1,		 8,	    0}	   /* Enter sniff mode (20-40mS) */
};

/* Lower power table for the A2DP. */
static const lp_power_table a2dp_stream_powertable[]=
{
	/* mode,    	min_interval, max_interval, attempt, timeout, duration */
	{lp_passive,	0,	    	  0,			0,		 0,		0} 		/* Go into passive mode and stay there */
};


static void setSLCLinkPolicy(void)
{
	uint16 i;
	Sink slc_sink = HfpGetSlcSink(theHeadset.hfp_hsp);
	
    /* SCO isn't active */
	if (theHeadset.user_power_table && theHeadset.user_power_table->SLCentries)
	{
		ConnectionSetLinkPolicy(slc_sink, theHeadset.user_power_table->SLCentries, &theHeadset.user_power_table->powertable[0]);			
		for (i=0;i<(uint16)(theHeadset.user_power_table->SLCentries);i++)
			POLICY_DEBUG(("LINK POLICY : User SLC table entries [0x%x] state [0x%x] min[0x%x] max[0x%x] attempt[0x%x] timeout[0x%x] time[0x%x]\n",theHeadset.user_power_table->SLCentries,
						  theHeadset.user_power_table->powertable[ i ].state,
						  theHeadset.user_power_table->powertable[ i ].min_interval,
						  theHeadset.user_power_table->powertable[ i ].max_interval,
						  theHeadset.user_power_table->powertable[ i ].attempt,
						  theHeadset.user_power_table->powertable[ i ].timeout,
						  theHeadset.user_power_table->powertable[ i ].time));
	}
	else
	{
       	ConnectionSetLinkPolicy(slc_sink, 2, hfp_powertable);
	}
	
	/* set up our sniff sub rate params for SLC link - use signalling parameters */
    ConnectionSetSniffSubRatePolicy(slc_sink,
                                        theHeadset.config->ssr_data.signalling_params.max_remote_latency,
                                        theHeadset.config->ssr_data.signalling_params.min_remote_timeout,
                                        theHeadset.config->ssr_data.signalling_params.min_local_timeout);    
	
    ConnectionSetLinkSupervisionTimeout(slc_sink, theHeadset.Timeouts.LinkSupervisionTimeoutHfp_s);
	
	POLICY_DEBUG(("LINK POLICY : Set HFP power table for sink 0x%x LST 0x%x\n",(uint16)slc_sink,theHeadset.Timeouts.LinkSupervisionTimeoutHfp_s));
}


static void setA2dpSignallingLinkPolicy(void)
{
	uint16 i = 0;
	Sink a2dp_sink = A2dpGetSignallingSink(theHeadset.a2dp);
	
	if (theHeadset.user_power_table && theHeadset.user_power_table->A2DPSigEntries)
	{
		ConnectionSetLinkPolicy(a2dp_sink, theHeadset.user_power_table->A2DPSigEntries, &theHeadset.user_power_table->powertable[theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries]);			
		for (i=0;i<(uint16)(theHeadset.user_power_table->A2DPSigEntries);i++)
			POLICY_DEBUG(("LINK POLICY : User A2DP table entries [0x%x] state [0x%x] min[0x%x] max[0x%x] attempt[0x%x] timeout[0x%x] time[0x%x]\n",theHeadset.user_power_table->A2DPSigEntries,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].state,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].min_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].max_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].attempt,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].timeout,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries ].time));
	}
	else
	{
		ConnectionSetLinkPolicy(a2dp_sink, 3, a2dp_signalling_powertable);
	}
	
	/* set up our sniff sub rate params for signalling link - use signalling parameters */
    ConnectionSetSniffSubRatePolicy(a2dp_sink,
                                        theHeadset.config->ssr_data.signalling_params.max_remote_latency,
                                        theHeadset.config->ssr_data.signalling_params.min_remote_timeout,
                                        theHeadset.config->ssr_data.signalling_params.min_local_timeout);
	
	ConnectionSetLinkSupervisionTimeout(a2dp_sink, theHeadset.Timeouts.LinkSupervisionTimeoutA2dp_s);
		
	POLICY_DEBUG(("LINK POLICY : Set A2DP Sig power table for sink 0x%x LST 0x%x\n",(uint16)a2dp_sink,theHeadset.Timeouts.LinkSupervisionTimeoutA2dp_s));
}


/****************************************************************************/
void linkPolicySLCconnect(void)
{
	Sink slc_sink = HfpGetSlcSink(theHeadset.hfp_hsp);
	
	if (!slc_sink)
		return;
	
	if (IsA2dpSourceAnAg())
	{
		/* HFP and A2DP connected to same remote device */
		theHeadset.combinedDevice = TRUE;
		if (!theHeadset.features.LinkPolicyA2dpSigBeatsSLC && !A2dpGetMediaSink(theHeadset.a2dp))
		{
			/* SLC link policy wins if no stream active and it beats A2DP signalling */
			setSLCLinkPolicy();
		}
	}
	else
	{
		
		Sink a2dp_sink = A2dpGetSignallingSink(theHeadset.a2dp);
		/* Always use SLC link policy as this is the only connection active to the remote device */
		setSLCLinkPolicy();
		
		if (a2dp_sink && !theHeadset.features.DisableRoleSwitching)
		{
			/* Both A2DP and HFP are now connected. Set headest to be Master of 
				A2DP and HFP links as they are with different devices. */
			ConnectionSetRole(&theHeadset.task, a2dp_sink, hci_role_master);
			ConnectionSetRole(&theHeadset.task, slc_sink, hci_role_master);
		}
	}
}


/****************************************************************************/
void linkPolicySLCdisconnect(void)
{
	if (theHeadset.combinedDevice)
	{
		/* HFP and A2DP were connected to same remote device */
		if (!theHeadset.features.LinkPolicyA2dpSigBeatsSLC && !A2dpGetMediaSink(theHeadset.a2dp))
		{
			/* Set A2dp signalling link policy if no stream active and if SLC was used in preference */
			setA2dpSignallingLinkPolicy();
		}
		theHeadset.combinedDevice = FALSE;
	}
}


/****************************************************************************/
void linkPolicyA2dpSigConnect(void)
{
	Sink a2dp_sink = A2dpGetSignallingSink(theHeadset.a2dp);
	
	if (!a2dp_sink)
		return;
	
	if (IsA2dpSourceAnAg())
	{
		/* HFP and A2DP connected to same remote device */
		theHeadset.combinedDevice = TRUE;
		if (theHeadset.features.LinkPolicyA2dpSigBeatsSLC && !HfpGetAudioSink(theHeadset.hfp_hsp))
		{
			/* A2dp signalling link policy wins if no SCO active and it beats SLC */
			setA2dpSignallingLinkPolicy();
		}
	}
	else
	{
		Sink slc_sink = HfpGetSlcSink(theHeadset.hfp_hsp);
		
		/* Always use A2dp signalling link policy as this is the only connection active to the remote device */
		setA2dpSignallingLinkPolicy();
		
		if (slc_sink && !theHeadset.features.DisableRoleSwitching)
		{
			/* Both A2DP and HFP are now connected. Set headest to be Master of 
				A2DP and HFP links as they are with different devices. */
			ConnectionSetRole(&theHeadset.task, a2dp_sink, hci_role_master);
			ConnectionSetRole(&theHeadset.task, slc_sink, hci_role_master);
		}
	}
}


/****************************************************************************/
void linkPolicyA2dpSigDisconnect(void)
{
	if (theHeadset.combinedDevice)
	{
		/* HFP and A2DP were connected to same remote device */
		if (theHeadset.features.LinkPolicyA2dpSigBeatsSLC && !HfpGetAudioSink(theHeadset.hfp_hsp))
		{
			/* Set SLC link policy if no SCO active and if A2dp signalling was used in preference */
			setSLCLinkPolicy();
		}
		theHeadset.combinedDevice = FALSE;
	}
}


/****************************************************************************/
void linkPolicySCOconnect(void)
{
	uint16 i = 0;
	Sink slc_sink = HfpGetSlcSink(theHeadset.hfp_hsp);
	
	if (!slc_sink)
		return;
	
	if (theHeadset.user_power_table && theHeadset.user_power_table->SCOentries)
	{
		ConnectionSetLinkPolicy(slc_sink, theHeadset.user_power_table->SCOentries ,&theHeadset.user_power_table->powertable[ theHeadset.user_power_table->SLCentries ]);               
		for (i=0;i<(uint16)(theHeadset.user_power_table->SCOentries);i++)
			POLICY_DEBUG(("LINK POLICY : User SCO table entries [0x%x] state [0x%x] min[0x%x] max[0x%x] attempt[0x%x] timeout[0x%x] time[0x%x]\n",theHeadset.user_power_table->SCOentries,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].state,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].min_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].max_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].attempt,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].timeout,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries ].time));
	}
	else
	{
        ConnectionSetLinkPolicy(slc_sink, 2, hfp_powertable_sco);
	}
	
	/* set up our sniff sub rate params for SCO link - use streaming parameters */
    ConnectionSetSniffSubRatePolicy(slc_sink,
                                        theHeadset.config->ssr_data.streaming_params.max_remote_latency,
                                        theHeadset.config->ssr_data.streaming_params.min_remote_timeout,
                                        theHeadset.config->ssr_data.streaming_params.min_local_timeout);

    POLICY_DEBUG(("LINK POLICY : Set SCO power table for sink 0x%x\n",(uint16)slc_sink));
}


/****************************************************************************/
void linkPolicySCOdisconnect(void)
{
	if (IsA2dpSourceAnAg())
	{
		/* HFP and A2DP connected to same remote device */
		if (!theHeadset.features.LinkPolicyA2dpSigBeatsSLC)
		{
			/* SLC link policy wins if it beats A2DP signalling */
			setSLCLinkPolicy();
		}
		else
		{
			/* A2DP signalling beats SLC link policy */
			setA2dpSignallingLinkPolicy();
		}
	}
	else
	{
		/* Always use SLC link policy as this is the only connection active to the remote device */
		setSLCLinkPolicy();
	}
}


/****************************************************************************/
void linkPolicyStreamConnect(void)
{
	uint16 i = 0;
	Sink a2dp_sink = A2dpGetSignallingSink(theHeadset.a2dp);
	
	if (!a2dp_sink)
		return;
	
	if (theHeadset.user_power_table && theHeadset.user_power_table->A2DPStreamEntries)
	{
		ConnectionSetLinkPolicy(a2dp_sink, theHeadset.user_power_table->A2DPStreamEntries, &theHeadset.user_power_table->powertable[theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries]);			
		for (i=0;i<(uint16)(theHeadset.user_power_table->A2DPStreamEntries);i++)
			POLICY_DEBUG(("LINK POLICY : User A2DP table entries [0x%x] state [0x%x] min[0x%x] max[0x%x] attempt[0x%x] timeout[0x%x] time[0x%x]\n",theHeadset.user_power_table->A2DPStreamEntries,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].state,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].min_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].max_interval,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].attempt,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].timeout,
						  theHeadset.user_power_table->powertable[ i + theHeadset.user_power_table->SLCentries + theHeadset.user_power_table->SCOentries + theHeadset.user_power_table->A2DPSigEntries ].time));
	}
	else
	{
		ConnectionSetLinkPolicy(a2dp_sink, 1, a2dp_stream_powertable);
	}
	
	/* set up our sniff sub rate params for A2DP link - use streaming parameters */
	ConnectionSetSniffSubRatePolicy(a2dp_sink,
                                        theHeadset.config->ssr_data.streaming_params.max_remote_latency,
                                        theHeadset.config->ssr_data.streaming_params.min_remote_timeout,
                                        theHeadset.config->ssr_data.streaming_params.min_local_timeout);
		
	POLICY_DEBUG(("LINK POLICY : Set A2DP Stream power table for sink 0x%x\n",(uint16)a2dp_sink));
}


/****************************************************************************/
void linkPolicyStreamDisconnect(void)
{
	if (IsA2dpSourceAnAg())
	{
		/* HFP and A2DP connected to same remote device */
		if (!theHeadset.features.LinkPolicyA2dpSigBeatsSLC)
		{
			/* SLC link policy wins if it beats A2DP signalling */
			setSLCLinkPolicy();
		}
		else
		{
			/* A2DP signalling beats SLC link policy */
			setA2dpSignallingLinkPolicy();
		}
	}
	else
	{
		/* Always use A2DP signalling link policy as this is the only connection active to the remote device */
		setA2dpSignallingLinkPolicy();
	}
}
