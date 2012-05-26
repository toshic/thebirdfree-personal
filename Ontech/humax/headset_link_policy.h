/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_link_policy.h
@brief  Interface to the link policy controls.
*/

#ifndef HEADSET_LINK_POLICY_H_
#define HEADSET_LINK_POLICY_H_


#include "headset_states.h"


/*************************************************************************
NAME    
    linkPolicySLCconnect
    
DESCRIPTION
    Called to update link policy when SLC connects.
    
*/
void linkPolicySLCconnect(void);


/*************************************************************************
NAME    
    linkPolicySLCdisconnect
    
DESCRIPTION
    Called to update link policy when SLC disconnects.
    
*/
void linkPolicySLCdisconnect(void);


/*************************************************************************
NAME    
    linkPolicyA2dpSigConnect
    
DESCRIPTION
    Called to update link policy when A2DP signalling connects.
    
*/
void linkPolicyA2dpSigConnect(void);


/*************************************************************************
NAME    
    linkPolicyA2dpSigDisconnect
    
DESCRIPTION
    Called to update link policy when A2DP signalling disconnects.
    
*/
void linkPolicyA2dpSigDisconnect(void);


/*************************************************************************
NAME    
    linkPolicySCOconnect
    
DESCRIPTION
    Called to update link policy when SCO connects.
    
*/
void linkPolicySCOconnect(void);


/*************************************************************************
NAME    
    linkPolicySCOdisconnect
    
DESCRIPTION
    Called to update link policy when SCO disconnects.
    
*/
void linkPolicySCOdisconnect(void);


/*************************************************************************
NAME    
    linkPolicyStreamConnect
    
DESCRIPTION
    Called to update link policy when A2DP Streaming begins.
    
*/
void linkPolicyStreamConnect(void);


/*************************************************************************
NAME    
    linkPolicyStreamDisconnect
    
DESCRIPTION
    Called to update link policy when A2DP Streaming stops.
    
*/
void linkPolicyStreamDisconnect(void);


#endif /* HEADSET_LINK_POLICY_H_ */
