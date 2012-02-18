/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1

*/

#ifndef AGHFP_RECEIVE_DATA_H_
#define AGHFP_RECEIVE_DATA_H_


/****************************************************************************
	Called when we get an indication from the firmware that there's more data 
	received and waiting in the RFCOMM buffer. Parse it.
*/
void aghfpHandleReceivedData(AGHFP *aghfp, Source source);


#endif /* AGHFP_RECEIVE_DATA_H_ */
