/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#ifndef _AGHFP_CSR_SUPPORTED_FEATURES_H_

#define _AGHFP_CSR_SUPPORTED_FEATURES_H_



#include "aghfp_parse.h"


/*void aghfpHandleFeatureNegotiation(Task task, const struct aghfpHandleFeatureNegotiation * feature ) ;*/


/*void aghfpHandleReponseCSRSupportedFeatures(Task task , const struct aghfpHandleReponseCSRSupportedFeatures * features ) ;*/

/*! @brief Sends an AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND to the app.

@param aghfp A pointer to the profile instance.
*/
void aghfpSendCsrFeatureNegotiationReqInd(AGHFP * aghfp);

#endif


