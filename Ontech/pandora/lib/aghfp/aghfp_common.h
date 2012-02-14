/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#ifndef AGHFP_COMMON_H_
#define AGHFP_COMMON_H_

#include "aghfp.h"
#include "aghfp_private.h"


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/****************************************************************************
 Update the aghfp state.
*/
void aghfpSetState(AGHFP *aghfp, aghfp_state state);


/*****************************************************************************
 Create a common cfm message (many AGHFP defined messages sent to the app
 have the form of the message below and a common function can be used to
 allocate them). Send the message not forgetting to set the correct 
 message id.
*/
void aghfpSendCommonCfmMessageToApp(uint16 message_id, AGHFP *aghfp, aghfp_lib_status status);


bool supportedProfileIsHfp(aghfp_profile profile);
bool supportedProfileIsHfp15(aghfp_profile profile);
bool supportedProfileIsHsp(aghfp_profile profile);
bool aghfpCodecBitmapToAghfpEnum(unsigned int aghfp_codec_bitmap, uint8 *aghfp_codec_enum);


#endif /* AGHFP_COMMON_H_ */
