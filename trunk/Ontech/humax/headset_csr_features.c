/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2007-2009
*/

/*!
@file    headset_csr_features.c
@brief    handles the csr to csr features 
*/

#include "headset_csr_features.h"
#include "headset_debug.h"

#include "headset_private.h"
#include "panic.h"
#include "vm.h"

/* Header files */
#include <stdlib.h>


#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/dm_prim.h>

#include <vm.h>
#include <panic.h>


#ifdef DEBUG_CSR2CSR
    #define CSR2CSR_DEBUG(x) DEBUG(x)
#else
    #define CSR2CSR_DEBUG(x)
#endif     


/*****************************************************************************/
void csr2csrEnable( HFP * hfp ) 
{
	uint16 codecs = auristream_mask_cvsd | auristream_mask_auristream_2_bit | auristream_mask_auristream_4_bit;
    CSR2CSR_DEBUG(("ENABLE CSR2CSR\n"));
	
	if (theHeadset.features.audio_plugin == 0)
	{
		/* NO DSP plugin, so only use cvsd */
		codecs = auristream_mask_cvsd;
	}
    
	/*enable the features on connection*/
    HfpCsrSupportedFeaturesReq(hfp, FALSE, FALSE, FALSE, FALSE, FALSE, codecs);
}


/*****************************************************************************/
void csr2csrFeatureNegotiationInd(HFP_CSR_FEATURE_NEGOTIATION_IND_T * ind)  
{
	audio_codec_type audio_codec_selected;
	
    CSR2CSR_DEBUG(("CSR2CSR FEAT NEG IND[%d],[%d]\n", ind->indicator, ind->value));
    
    if(ind->indicator == CSR_IND_CODEC)
    {
		/*respond to the ag with the agreed codec*/
		HfpFeatureNegotiationResponse( ind->hfp, CSR_IND_CODEC, ind->value);

		/* Enable Auristream Voice Settings with the firmware */    
	    if(ind->value > 1)
	    {
	        DM_HCI_WRITE_VOICE_SETTING_T *prim = PanicUnlessNew(DM_HCI_WRITE_VOICE_SETTING_T); 
	        
	        prim->common.op_code = DM_HCI_WRITE_VOICE_SETTING; 
	        prim->common.length = sizeof(DM_HCI_WRITE_VOICE_SETTING_T); 
	        
	        prim->voice_setting = 0x63;
	
	        VmSendDmPrim(prim);
	    }
		
		/* The codec value will be in the form of a particular bit set in the
	   		value for the message, e.g. 1, 2, 4, 8, etc. To save on space in globals
	   		this values is stored as part of an enumerated list. The following switch statement
	   		converts to the enumerated value. */
		switch(ind->value)
		{
			case(auristream_mask_cvsd):
				audio_codec_selected = audio_codec_cvsd;
				break;
			case(auristream_mask_auristream_2_bit):
				audio_codec_selected = audio_codec_auristream_2_bit;
				break;
			case(auristream_mask_auristream_4_bit):
				audio_codec_selected = audio_codec_auristream_4_bit;
				break;
			default:
				audio_codec_selected = audio_codec_cvsd;
				break;
    	}
		
		/*store the selected audio codec*/
		theHeadset.SCO_codec_selected = audio_codec_selected;     
    }
}

   
