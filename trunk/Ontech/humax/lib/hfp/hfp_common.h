/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_common.h
    
DESCRIPTION
	
*/

#ifndef HFP_COMMON_H_
#define HFP_COMMON_H_

#include "hfp.h"
#include "hfp_private.h"

typedef const uint8 *ptr;


/****************************************************************************
NAME	
	hfpSkipSpace

DESCRIPTION
	Skip one or more consecutive spaces (also , and ;) in the parse string.

RETURNS
	ptr - to the string portion after the spaces that have been skipped.
*/
ptr hfpSkipSpace(ptr p, ptr e);


/****************************************************************************
NAME	
	hfpMatchChar

DESCRIPTION
	Does the ptr to the currect char match the char passed in.

RETURNS
	ptr - to the string portion after the matched char or 0 (if no match) 
*/
ptr hfpMatchChar(ptr p, ptr e, char ch);


/****************************************************************************
NAME	
	hfpSkipPastChar

DESCRIPTION
	Skip along the parse string to the point just after the specified char.

RETURNS
	ptr - to the string portion after the match or 0 (if no match) 
*/
ptr hfpSkipPastChar(ptr p, ptr e, char ch);


/****************************************************************************
NAME	
	hfpSetState

DESCRIPTION
	Update the hfp state.
*/
void hfpSetState(HFP *hfp, hfpState state);


/****************************************************************************
NAME	
	hfpSendCommonCfmMessageToApp

DESCRIPTION
	Create a common cfm message (many HFP defined messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.
*/
void hfpSendCommonCfmMessageToApp(uint16 message_id, HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME
    hfpConvertNumberType

DESCRIPTION
    Converts type of number to HFP library specific value.
*/
hfp_number_type hfpConvertNumberType( uint8 type );


/****************************************************************************
NAME
    supportedProfileIsHfp

DESCRIPTION
    Returns true if the profile filed passed in indicates support for HFP,
    returns false otherwise.
*/
bool supportedProfileIsHfp(hfp_profile profile);


/****************************************************************************
NAME
    supportedProfileIsHfp15

DESCRIPTION
    Returns true if the profile filed passed in indicates support for HFP 1.5,
    returns false otherwise.
*/
bool supportedProfileIsHfp15(hfp_profile profile);


/****************************************************************************
NAME
    supportedProfileIsHsp

DESCRIPTION
    Returns true if the profile filed passed in indicates support for HSP,
    returns false otherwise.
*/
bool supportedProfileIsHsp(hfp_profile profile);


#endif /* HFP_COMMON_H_ */
