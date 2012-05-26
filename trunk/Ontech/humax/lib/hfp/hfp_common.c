/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_common.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_common.h"

#include <panic.h>


/*****************************************************************************/
ptr hfpSkipSpace(ptr p, ptr e)
{
    if(p)
    {
        while(p != e && (*p == ' ' || *p == ',' || *p == ';'))
            ++p;
    }
    return p;
}


/*****************************************************************************/
static __inline__ char toUpper(char c)
{
	return 'a' <= c && c <= 'z' ? c +'A'-'a' : c; 
}


/*****************************************************************************/
ptr hfpMatchChar(ptr p, ptr e, char ch)
{
    return p && p != e && toUpper((int) *p) == toUpper((int) ch) ? p+1 : 0;
}


/*****************************************************************************/
ptr hfpSkipPastChar(ptr p, ptr e, char ch)
{
    if(p)
        while(p != e)
            if(*p++ == (uint8)ch)
                return p;
    return 0;
}


/*****************************************************************************/
void hfpSetState(HFP *hfp, hfpState state)
{
	hfp->state = state;
}


/*****************************************************************************/
void hfpSendCommonCfmMessageToApp(uint16 message_id, HFP *hfp, hfp_lib_status status)
{
	MAKE_HFP_MESSAGE(HFP_COMMON_CFM_MESSAGE);
	message->status = status;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, message_id, message);
}


/*****************************************************************************/
hfp_number_type hfpConvertNumberType( uint8 type )
{
    switch ( type & 0x70 )
    {
    case 0x00:
        return hfp_number_unknown;
        break;
        
    case 0x10:
        return hfp_number_international;
        break;
        
    case 0x20:
        return hfp_number_national;
        break;
        
    case 0x30:
        return hfp_number_network;
        break;
        
    case 0x40:
        return hfp_number_dedicated;
        break;
        
    default:
        return hfp_number_unknown;
        break;
    }
}


/*****************************************************************************/
bool supportedProfileIsHfp(hfp_profile profile)
{
    if ((profile == hfp_handsfree_profile) || (profile == hfp_handsfree_15_profile))
        return 1;
    else
        return 0;
}


/*****************************************************************************/
bool supportedProfileIsHfp15(hfp_profile profile)
{
    if (profile == hfp_handsfree_15_profile)
        return 1;
    else
        return 0;
}


/*****************************************************************************/
bool supportedProfileIsHsp(hfp_profile profile)
{
    if (profile == hfp_headset_profile)
        return 1;
    else
        return 0;
}
