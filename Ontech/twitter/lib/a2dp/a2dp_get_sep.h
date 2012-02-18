/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_get_sep.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_GET_SEP_H_
#define A2DP_GET_SEP_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
	getSepInstanceBySeid

DESCRIPTION
	Get a pointer to the Stream End Point information, based on the supplied ID. 

*/
sep_type *getSepInstanceBySeid(A2DP *a2dp, uint16 seid);


#endif /* A2DP_GET_SEP_H_ */
