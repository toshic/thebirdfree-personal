/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_csr_features.h      
@brief    handles the csr to csr features 
*/

#ifndef _CSR2CSRFEATURES_
#define _CSR2CSRFEATURES_

#include <hfp.h>

#include "headset_private.h"


/****************************************************************************
NAME    
    csr2csrEnable
    
DESCRIPTION
  	Enable CSR to CSR features
    
*/
void csr2csrEnable(HFP * hfp);


/****************************************************************************
NAME    
    csr2csrHandleSupportedFeaturesCfm
    
DESCRIPTION
  	Handle a CSR supported features CFM
    
*/
void csr2csrHandleSupportedFeaturesCfm(HFP_CSR_SUPPORTED_FEATURES_CFM_T * cfm);


/****************************************************************************
NAME    
    csr2csrFeatureNegotiationInd
    
DESCRIPTION
  	Handle a CSR feature negotiation IND
    
*/
void csr2csrFeatureNegotiationInd(HFP_CSR_FEATURE_NEGOTIATION_IND_T * ind);


/* Supported Indicators */

#define CSR_IND_CODEC 0x06
    
#endif /* _CSR2CSRFEATURES_ */
