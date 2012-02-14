/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles discovery of remote devices.
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_scan.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_a2dp_stream_control.h"

#include <ps.h>
#include <bdaddr.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
    Definitions used in EIR data setup
*/
/* Macro to acquire byte n from a multi-byte word w */
#define GET_BYTE(w, n) (((w) >> ((n) * 8)) & 0xFF)

/* EIR tags */
#define EIR_TYPE_LOCAL_NAME_COMPLETE        (0x09)
#define EIR_TYPE_LOCAL_NAME_SHORTENED       (0x08)
#define EIR_TYPE_UUID16_COMPLETE            (0x03)
#define EIR_TYPE_UUID16_SHORTENED           (0x02)
#define EIR_TYPE_INQUIRY_TX                 (0x0A)

/* Device UUIDs */
#define BT_UUID_SERVICE_CLASS_AGHFP         (0x111F)
#define BT_UUID_SERVICE_CLASS_AGHSP         (0x1112)
#define BT_UUID_SERVICE_CLASS_AUDIO_SOURCE  (0x110A)
#define BT_UUID_SERVICE_CLASS_AVRCP_TARGET  (0x110C)
#define BT_UUID_SERVICE_CLASS_A2DP          (0x110D)

/* Remote UUIDs */
#define BT_UUID_SERVICE_CLASS_HFP            (0x111E)
#define BT_UUID_SERVICE_CLASS_HSP            (0x1108)
#define BT_UUID_SERVICE_CLASS_AUDIO_SINK     (0x110B)
#define BT_UUID_SERVICE_CLASS_AVRCP          (0x110E)

/* Packet size defines */
#define MAX_PACKET_SIZE_DH1     (27)
#define EIR_MAX_SIZE            (MAX_PACKET_SIZE_DH1)
#define INQUIRY_TX_DATA_SIZE    (2)

/* Service search patterns */
/* DataEl(0x35), Length(0x03), AVDTP(0x19), Headset(0x1108) */
static const uint8 s_hsp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x08};
/* DataEl(0x35), Length(0x03), AVDTP(0x19), Handsfree(0x111E) */
static const uint8 s_hfp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x1E};
/* DataEl(0x35), Length(0x03), AVDTP(0x19), Advanced Audio Distribution(0x110D) */
static const uint8 s_a2dp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x0D};
/* DataEl(0x35), Length(0x03), AVCTP(0x17), A/V Remote Control(0x110E) */
static const uint8 s_avrcp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x0E};

/* Service search list */
static const struct
{
    mvdProfiles   profile;
    bool          required;        /* Further SDP searches on device will be abandoned if it does not support the profile */
    uint16        size;
    const uint8 * pattern;
} s_service_searches[] =
{
    { ProfileAghsp, FALSE, sizeof(s_hsp_service_search_pattern)  , s_hsp_service_search_pattern   },
    { ProfileAghfp, FALSE, sizeof(s_hfp_service_search_pattern)  , s_hfp_service_search_pattern   },
    { ProfileA2dp , FALSE, sizeof(s_a2dp_service_search_pattern) , s_a2dp_service_search_pattern  },
    { ProfileAvrcp, FALSE, sizeof(s_avrcp_service_search_pattern), s_avrcp_service_search_pattern }
};


static uint16 s_profileSearchIdx;
static mvdInquiryScanData *s_inquiry_scan_data = NULL;


/****************************************************************************
  LOCAL FUNCTIONS
*/
      
/****************************************************************************
NAME
    mvdSizeUuids

DESCRIPTION
    Calculate the space required to include supported profile UUIDS in EIR.
*/
static uint8 mvdSizeUuids (mvdProfiles supported_profiles)
{
    uint8 k = 0;
    uint8 size_profiles = 0;
    
    while (supported_profiles != 0)
    {
        if (supported_profiles & (0x01 << k))
        {
            /* Add 2 bytes for UUID (4 for A2DP) */
            size_profiles += ((0x01 << k) == ProfileA2dp) ? 4 : 2;
            /* Mask out this bit */
            supported_profiles &= ~(0x01 << k);
        }
        k++;
    }
    
    /* Add on space for field ID */
    if(size_profiles)
        size_profiles += 1;
    
    return size_profiles;
}


/****************************************************************************
NAME
    mvdSetUuids

DESCRIPTION
    Write supported profile UUIDS data to eir_loc.
*/
static void mvdSetUuids (uint8* eir_loc, mvdProfiles supported_profiles)
{
    /* Set field to UUID16 Complete */
    *eir_loc++ = EIR_TYPE_UUID16_COMPLETE;
    
    /* Set up Profile UUIDS */
    if(supported_profiles & ProfileAghfp)
    {
        /* Write AGHFP UUID */
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AGHFP, 0);
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AGHFP, 1);
    }
    if(supported_profiles & ProfileAghsp)
    {
        /* Write AGHSP UUID */
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AGHSP, 0);
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AGHSP, 1);
    }
    if(supported_profiles & ProfileA2dp)
    {
        /* Write Audio Source UUID */
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AUDIO_SOURCE, 0);
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AUDIO_SOURCE, 1);
        
        /* Write A2DP UUID */
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_A2DP, 0);
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_A2DP, 1);
    }
    if(supported_profiles & ProfileAvrcp)
    {
        /* Write AVRCP Target UUID */
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AVRCP_TARGET, 0);
        *eir_loc++ = GET_BYTE(BT_UUID_SERVICE_CLASS_AVRCP_TARGET, 1);
    }
}


/****************************************************************************
NAME
    cancelInquiry

DESCRIPTION
    Cancels the inquiry process.
    
*/
static void cancelInquiry (void)
{
    scanFlushInquireResults();
    ConnectionInquireCancel(&the_app->task);
    MessageCancelAll(&the_app->task, CL_DM_INQUIRE_RESULT);
}


/****************************************************************************
NAME
    parseEirUuids

DESCRIPTION
    Returns the profiles supported from the EIR UUID data.
    
*/
static mvdProfiles parseEirUuids(const uint8 size_uuids, const uint8 *uuids)
{
    mvdProfiles profiles = ProfileNone;
    uint8 k;
    uint16 uuid;
    
    for ( k = 0; k < size_uuids ; k += 2 )
    {
        /* Get UUID from data */
        uuid = uuids[k] | (uuids[k+1] << 8);
            
        /* Is this something we can connect to */
        if (uuid == BT_UUID_SERVICE_CLASS_HFP)
            profiles |= ProfileAghfp;
        else if (uuid == BT_UUID_SERVICE_CLASS_HSP)
            profiles |= ProfileAghsp;
        else if (uuid == BT_UUID_SERVICE_CLASS_AUDIO_SINK)
            profiles |= ProfileA2dp;
        else if (uuid == BT_UUID_SERVICE_CLASS_AVRCP)
            profiles |= ProfileAvrcp;
        
    }
    return profiles;
}


/****************************************************************************
NAME
    parseEirData

DESCRIPTION
    Returns the data retrieved from the EIR data.
    
*/
static mvdEirData parseEirData(const uint8 size_eir_data, const uint8* eir_data)
{
    const uint8* p = eir_data;
    mvdEirData    result;
    
    /* Default to max possible Inquiry Tx Power */
    result.path_loss = 127;
    result.profiles = ProfileNone;
    result.profiles_complete = FALSE;
    
    /* Search until no more to read */
    for ( p = eir_data ; (uint8)(p - eir_data) != size_eir_data ; p += (*p)+1 )
    {
        /* Check type fields for UUIDs or Inquiry Tx Power */
        if (*(p+1) == EIR_TYPE_INQUIRY_TX)
        {
            /* Initially set path loss to Inquiry Tx Power*/
            result.path_loss = *p+2;
        }
        else if ((*(p+1) == EIR_TYPE_UUID16_COMPLETE) || (*(p+1) == EIR_TYPE_UUID16_SHORTENED))
        {
            /* Get the profile information */
            result.profiles_complete = (*(p+1) == EIR_TYPE_UUID16_COMPLETE);
            result.profiles = parseEirUuids(*p , p+2);
        }
    }
    
    /* Return number of UUIDs found in EIR data */
    return result;
}


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    scanWriteEirData

DESCRIPTION
    Write media dongle EIR data.
    
*/
void scanWriteEirData (CL_DM_LOCAL_NAME_COMPLETE_T *message)
{
#define SIZE_UUIDS (size_uuids ? size_uuids + 1 : 0)
    
/* Length of EIR data with all fields complete */
#define EIR_DATA_SIZE_FULL (message->size_local_name + 2 + SIZE_UUIDS + 1 + INQUIRY_TX_DATA_SIZE + 1)

/* Whether the EIR data is shortened or not. */
#define EIR_DATA_SHORTENED (EIR_DATA_SIZE_FULL > EIR_MAX_SIZE)

/* Maximum length the local name can be to fit EIR data into DH1 */
#define EIR_NAME_MAX_SIZE (EIR_MAX_SIZE - (2 + SIZE_UUIDS + 1 + INQUIRY_TX_DATA_SIZE + 1))

/* Actual length of the local name put into the EIR data */
#define EIR_NAME_SIZE (EIR_DATA_SHORTENED ? EIR_NAME_MAX_SIZE : message->size_local_name)

    uint8 size_uuids = mvdSizeUuids(the_app->supported_profiles);
    
    /* Determine length of EIR data */
    uint16 size = EIR_DATA_SHORTENED ? EIR_MAX_SIZE : EIR_DATA_SIZE_FULL;
    
    /* Just enough for the UUID16, Inquiry Tx and name fields and null termination */
    uint8 *const eir = (uint8 *)PanicUnlessMalloc(size * sizeof(uint8));
    uint8 *p = eir;    
    
    *p++ = EIR_NAME_SIZE + 1;  /* Device Name Length Field */
    *p++ = EIR_DATA_SHORTENED ? EIR_TYPE_LOCAL_NAME_SHORTENED : EIR_TYPE_LOCAL_NAME_COMPLETE;
    memcpy(p, message->local_name, EIR_NAME_SIZE);
    p += EIR_NAME_SIZE;
        
    *p++ = INQUIRY_TX_DATA_SIZE;      /* Inquiry Tx Length Field */
    *p++ = EIR_TYPE_INQUIRY_TX;
    *p++ = the_app->inquiry_tx;
       
    if (size_uuids)
    {
        *p++ = size_uuids; /* UUIDs length field */
        mvdSetUuids (p, the_app->supported_profiles);
        p += size_uuids;
    }
    
    *p++ = 0x00; /* Termination. p ends up pointing one off the end */
    
    ConnectionWriteEirData(FALSE, size, eir);
        
    /* Free the EIR data */
    free(eir);
}


/****************************************************************************
NAME
    scanMakeDiscoverable

DESCRIPTION
    Make the dongle connectable and discoverable.
    
*/
void scanMakeDiscoverable (void)
{
    DEBUG_SCAN(("Connectable/Discoverable\n"));
    ConnectionWritePagescanActivity(0x800, 0x12);
    ConnectionWriteInquiryscanActivity(0x800, 0x12);
    ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);
}


/****************************************************************************
NAME
    scanMakeConnectable

DESCRIPTION
    Make the dongle connectable.
    
*/
void scanMakeConnectable (void)
{
    DEBUG_SCAN(("Connectable\n"));
    ConnectionWritePagescanActivity(0x800, 0x12);
    ConnectionWriteScanEnable(hci_scan_enable_page);
}


/****************************************************************************
NAME
    scanMakeUnconnectable

DESCRIPTION
    Make device neither connectable or discoverable.
    
*/
void scanMakeUnconnectable (void)
{
    DEBUG_SCAN(("Unconnectable\n"));
    ConnectionWriteScanEnable(hci_scan_enable_off);
}


/****************************************************************************
NAME
    scanKickInquiryScan

DESCRIPTION
    Starts inquiry process.
    
*/
void scanKickInquiryScan (void)
{
    DEBUG_SCAN(("Kick inquiry scan\n"));
    the_app->remote_profiles = ProfileNone;
    if ( s_inquiry_scan_data == NULL )
    {
        s_inquiry_scan_data = (mvdInquiryScanData *)malloc( sizeof(mvdInquiryScanData) );
        PanicNull( s_inquiry_scan_data );
    }
    s_inquiry_scan_data->read_idx = 0;
    s_inquiry_scan_data->write_idx = 0;
    
    ConnectionInquire(&the_app->task, 0x9e8b33, INQUIRY_SCAN_BUFFER_SIZE, 4, (uint32)AV_MAJOR_DEVICE_CLASS);
}


/****************************************************************************
NAME
    scanFlushInquireResults

DESCRIPTION
    Clears all inquiry results.
    
*/
void scanFlushInquireResults (void)
{
    if ( s_inquiry_scan_data != NULL )
    {
        free ( s_inquiry_scan_data );
        s_inquiry_scan_data = NULL;
    }
}


/****************************************************************************
NAME
    scanStoreInquireResult

DESCRIPTION
    Stores the data returned in an inquiry result message.
    
*/
void scanStoreInquireResult (const CL_DM_INQUIRE_RESULT_T *prim)
{
    /* Default EIR values */
    mvdEirData    result;
    
    DEBUG_SCAN(("Found device: 0x%X 0x%X 0x%lX\n", (uint16)prim->bd_addr.nap, (uint16)prim->bd_addr.uap, (uint32)prim->bd_addr.lap));
    
    result = parseEirData(prim->size_eir_data, prim->eir_data);
            
    result.profiles &= the_app->supported_profiles;
    
    /* If device doesn't support anything we can use forget it */
    if ((result.profiles == ProfileNone) && (result.profiles_complete))
    {        
        DEBUG_SCAN(("Device Unsupported\n"));
        return;
    }
    
    /* Subtract RSSI from Inquiry Tx Power, assume min possible RSSI if unknown */
    result.path_loss -= (prim->rssi == CL_RSSI_UNKNOWN) ? -127 : prim->rssi;
    
    DEBUG_SCAN(("path loss: %i\n",result.path_loss));
    
    if ( (s_inquiry_scan_data != NULL) && (s_inquiry_scan_data->write_idx < INQUIRY_SCAN_BUFFER_SIZE) )
    {
        uint8 j,k;    
        
        /* Start by checking if we have already found this device */
        for ( k = 0 ; k < s_inquiry_scan_data->write_idx ; k++ )
        {
            /* Found duplicate */
            if (BdaddrIsSame(&prim->bd_addr, &s_inquiry_scan_data->buffer[k]))
            {
                DEBUG_SCAN(("Device already found\n"));
                
                /* If we got a lower path loss this time */
                if (result.path_loss < s_inquiry_scan_data->eir_data[k].path_loss)
                {
                    /* Remove the old record and put this one in */
                    DEBUG_SCAN((" - updating\n"));
                    s_inquiry_scan_data->write_idx--;
                    for ( j=k ; j < s_inquiry_scan_data->write_idx ; j++)
                    {
                        memcpy(&s_inquiry_scan_data->buffer[j],&s_inquiry_scan_data->buffer[j+1], sizeof(bdaddr));
                        s_inquiry_scan_data->eir_data[j] = s_inquiry_scan_data->eir_data[j+1];
                    }
                    break;
                }
                else
                {
                    /* No point updating */
                    DEBUG_SCAN((" - returning\n"));
                    return;
                }
            }
        }
        
        /* Move up the found devices list */
        for ( k = k ; k > 0 ; k-- )
        {
            /* If next dev up has lower path loss we have found our place */
            if (s_inquiry_scan_data->eir_data[k-1].path_loss <= result.path_loss)
                break;
        }
        
        DEBUG_SCAN(("Adding device at %d of %d\n",k,s_inquiry_scan_data->write_idx));
        
        
        /* Shuffle down other entries if required */
        for ( j = s_inquiry_scan_data->write_idx ; j > k ; j-- )
        {
            memcpy(&s_inquiry_scan_data->buffer[j],&s_inquiry_scan_data->buffer[j-1], sizeof(bdaddr));
            s_inquiry_scan_data->eir_data[j] = s_inquiry_scan_data->eir_data[j-1];
        }
        
        /* Add device at k and increment write index */
        memcpy(&s_inquiry_scan_data->buffer[k], &prim->bd_addr, sizeof(bdaddr));
        s_inquiry_scan_data->eir_data[k] = result;
        s_inquiry_scan_data->write_idx++;
    }
}


/****************************************************************************
NAME
    scanLoadNextInquireResult

DESCRIPTION
    Retrieves the data associated with the next inquiry result.
    
*/
bool scanLoadNextInquireResult (void)
{
    if ( (s_inquiry_scan_data != NULL) && (s_inquiry_scan_data->read_idx < s_inquiry_scan_data->write_idx) )
    {
        memcpy(&the_app->search_bdaddr, &s_inquiry_scan_data->buffer[s_inquiry_scan_data->read_idx], sizeof(bdaddr));
        the_app->remote_profiles = s_inquiry_scan_data->eir_data[s_inquiry_scan_data->read_idx].profiles;
        the_app->remote_profiles_complete = s_inquiry_scan_data->eir_data[s_inquiry_scan_data->read_idx].profiles_complete;
        s_inquiry_scan_data->read_idx++;
        return TRUE;
    }

    return FALSE;
}


/****************************************************************************
NAME
    scanHaveInquireResults

DESCRIPTION
    Returns if any inquiry results exists.
    
*/
bool scanHaveInquireResults (void)
{
    return ((s_inquiry_scan_data != NULL) && (s_inquiry_scan_data->read_idx < s_inquiry_scan_data->write_idx)) ? TRUE : FALSE;
}


/****************************************************************************
NAME
    scanRepeatSdpSearch

DESCRIPTION
    Repeat an SDP service search.
    
*/
bool scanRepeatSdpSearch (void)
{
    if (the_app->supported_profiles & (0x01 << s_profileSearchIdx))
    {
        ConnectionSdpServiceSearchRequest(&the_app->task, &the_app->search_bdaddr, 0x32, s_service_searches[s_profileSearchIdx].size, s_service_searches[s_profileSearchIdx].pattern);
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    scanKickFirstSdpSearch

DESCRIPTION
    Kick off an SDP search if the supported remote profiles need to be discovered.
    If they are already known then a connection can be performed without doing
    the SDP search.
    
*/
void scanKickFirstSdpSearch (void)
{
    /* Check if we already know what profiles are supported */
    if (the_app->remote_profiles_complete || (the_app->remote_profiles == the_app->supported_profiles))
    {
        /* Hack to connect via message, direct call would be recursive */
        eventHandleSendKickCommActionMessage(CommActionConnect); 
        
        /* The device to connect to is the one currently stored as the search device */
        the_app->connect_bdaddr = the_app->search_bdaddr;
        
        /* Jump straight to connection (we can skip pin code setup too as 2.l) */
        DEBUG_SCAN(("Skip SDP\n"));
        setAppState(AppStateIdle);
    }
    else
    {
        /* Create dev instance to keep track of supported profiles */
        devInstanceTaskData *inst = devInstanceFindFromBddr(&the_app->search_bdaddr, TRUE);;
        
        /* Kick SDP search */
        DEBUG_SCAN(("Kick SDP\n"));
             
        if (inst != NULL)
        {                       
            s_profileSearchIdx = (uint16)-1;            
            ConnectionSdpOpenSearchRequest(&the_app->task, &the_app->search_bdaddr);  /* Generates a CL_SDP_OPEN_SEARCH_CFM */
        }
        else
        {        
            ConnectionSdpCloseSearchRequest(&the_app->task);  /* Generates a CL_SDP_CLOSE_SEARCH_CFM */
        }
    }
}


/****************************************************************************
NAME
    scanKickNextSdpSearch

DESCRIPTION
    Kick off SDP search of next supported profile. If all profiles have been
    searched for then the SDP search ends.
    
*/
bool scanKickNextSdpSearch (void)
{
    mvdProfiles searchProfile;
    
    while ((searchProfile = ProfileAll & (0x01 << ++s_profileSearchIdx)))
    {
        /* Dont search if we already know about this profile from EIR data */
        if ((the_app->supported_profiles & searchProfile) && !(the_app->remote_profiles & searchProfile))
        {
            ConnectionSdpServiceSearchRequest(&the_app->task, &the_app->search_bdaddr, 0x32, s_service_searches[s_profileSearchIdx].size, s_service_searches[s_profileSearchIdx].pattern);
            return TRUE;
        }
    }
    
    ConnectionSdpCloseSearchRequest(&the_app->task);  /* Generates a CL_SDP_CLOSE_SEARCH_CFM */
    s_profileSearchIdx = (uint16)-1;
    
    return FALSE;
}


/****************************************************************************
NAME
    scanCancelSdpSearch

DESCRIPTION
    Cancel any pending SDP searches.
    
*/
void scanCancelSdpSearch (void)
{
    ConnectionSdpTerminatePrimitiveRequest(&the_app->task);
    ConnectionSdpCloseSearchRequest(&the_app->task);  /* Generates a CL_SDP_CLOSE_SEARCH_CFM */
    s_profileSearchIdx = (uint16)-1;
    MessageCancelAll(&the_app->task, CL_SDP_SERVICE_SEARCH_CFM);
}


/****************************************************************************
NAME
    scanHandleSdpSearchResult

DESCRIPTION
    Handle an SDP search result.
    
*/
bool scanHandleSdpSearchResult (CL_SDP_SERVICE_SEARCH_CFM_T *prim)
{
    devInstanceTaskData *inst = devInstanceFindFromBddr(&prim->bd_addr, FALSE);

#ifdef DEBUG_ENABLED
    uint16 n = prim->size_records;
    uint16 i;
    DEBUG_SCAN(("  Sdp service search result [ "));
    for (i = 0; i < n; i++)
    {
        DEBUG_SCAN(("0x%X ",prim->records[i]));
    }
    DEBUG_SCAN(("]\n"));
#endif

    if (prim->status == sdp_response_success)
    {
        DEBUG_SCAN(("  Device supports profile %u\n", s_service_searches[s_profileSearchIdx].profile));
        if (inst != NULL)
            inst->remote_profiles |= s_service_searches[s_profileSearchIdx].profile;
        return TRUE;
    }
    else
    {
        DEBUG_SCAN(("  Device does NOT support %u\n", s_service_searches[s_profileSearchIdx].profile));
        if (inst != NULL)
            inst->remote_profiles &= ~s_service_searches[s_profileSearchIdx].profile; 
        return s_service_searches[s_profileSearchIdx].required ? FALSE : TRUE;
    }
}


/****************************************************************************
NAME
    scanIsSuitableDevice

DESCRIPTION
    Returns if the local and remote device have a profile in common.
    
*/
bool scanIsSuitableDevice (devInstanceTaskData *inst)
{
    bool suitable = FALSE;
    
    DEBUG_SCAN(("Suitable profiles:\n"));
    if (inst->remote_profiles & the_app->supported_profiles & ProfileAghfp)
    {
        DEBUG_SCAN(("  AGHFP\n"));
        suitable = TRUE;
    }
    else if (inst->remote_profiles & the_app->supported_profiles & ProfileAghsp)
    {
        DEBUG_SCAN(("  AGHSP\n"));
        suitable = TRUE;
    }

    if (inst->remote_profiles & the_app->supported_profiles & ProfileA2dp)
    {
        DEBUG_SCAN(("  A2DP\n"));
        suitable = TRUE;
    }
    
    if (inst->remote_profiles & the_app->supported_profiles & ProfileAvrcp)
    {
        DEBUG_SCAN(("  AVRCP\n"));
        suitable = TRUE;
    }
    
    return suitable;
}


/****************************************************************************
NAME
    scanStartAppInquiryTimer

DESCRIPTION
    Starts the timer for inquiry. Inquiry mode will end once the timer fires.
    
*/
void scanStartAppInquiryTimer (void)
{
    if (!the_app->inquiring)    /* Don't restart a timer that is already running */
    {
        MessageCancelAll(&the_app->task, APP_INQUIRY_TIMER);
        MessageSendLater(&the_app->task, APP_INQUIRY_TIMER, 0, 120000);
        the_app->inquiring = TRUE;
        the_app->app_inquiry_timer_expired = FALSE;
    }
}


/****************************************************************************
NAME
    scanStopAppInquiryTimer

DESCRIPTION
    Stops the timer for inquiry.
    
*/
void scanStopAppInquiryTimer (void)
{
    MessageCancelAll(&the_app->task, APP_INQUIRY_TIMER);
    the_app->inquiring = FALSE;
    the_app->app_inquiry_timer_expired = FALSE;
}


/****************************************************************************
NAME
    scanCancelInquiryScan

DESCRIPTION
    Cancels inquiry mode.
    
*/
void scanCancelInquiryScan (void)
{
    switch (the_app->app_state)
    {
        case AppStateInquiring:
        case AppStateSearching:
        {
            scanCancelSdpSearch();
            cancelInquiry();
            scanStopAppInquiryTimer();
            eventHandleCancelCommAction();
            setAppState(AppStateIdle);
            break;
        }
        default:
        {
            break;
        }
    }  
}


/****************************************************************************
NAME
    scanInquiryScanReq

DESCRIPTION
    Begins inquiry if dongle is idle.
    
*/
void scanInquiryScanReq (void)
{
    DEBUG_SCAN(("APP_INQUIRY_SCAN_REQ\n"));
    
    /* Start looking for remote devices */
    switch ( the_app->app_state )
    {
        case AppStateIdle:  
        case AppStateConnecting:  
        {
            if ( the_app->app_inquiry_timer_expired )
            {    /* ...inquiry scan time out has occured - stop discovering */
                scanStopAppInquiryTimer();
                scanMakeConnectable();  /* Ensure device is no longer in discoverable state */
                eventHandleCancelCommAction();
                a2dpStreamRestartAudioStream();
            }
            else
            {
                setAppState(AppStateInquiring);
                scanStartAppInquiryTimer();
                scanKickInquiryScan();
            }
            break;
        }
        default:    
        {
            /* Silently ignore request */
            DEBUG_SCAN((" - ignored\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    scanSdpSearchReq

DESCRIPTION
    Begins SDP search.
    
*/
void scanSdpSearchReq (void)
{
    DEBUG_SCAN(("APP_SDP_SEARCH_REQ 0x%X 0x%X 0x%lX\n", the_app->search_bdaddr.nap, the_app->search_bdaddr.uap, the_app->search_bdaddr.lap));
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateInquiring:
        {
            setAppState(AppStateSearching);
            scanKickFirstSdpSearch();
            break;
        }
        default:
        {
            /* Silently ignore request */
            DEBUG_SCAN((" - ignored\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    scanProcessNextInquireResult

DESCRIPTION
    Interface to processing next inquiry result.
    
*/
void scanProcessNextInquireResult (void)
{
    DEBUG_SCAN(("scanProcessNextInquireResult\n"));
    switch ( the_app->app_state )
    {
        case AppStateIdle:    /* Will get here via a failed connect attempt during inquiry */
        {
            setAppState(AppStateInquiring);
            /* Fall through */
        }
        case AppStateInquiring:
        {
            if ( scanLoadNextInquireResult() )
            {    /* Inquiry scan found a remote device, kick off a SDP search on it */
                scanSdpSearchReq();
            }
            else
            {
                if ( the_app->app_inquiry_timer_expired )
                {   /* ...inquiry scan time out has occured - stop discovering */
                    scanCancelInquiryScan();
                    scanMakeConnectable();    /* Ensure we drop out of discoverable mode */
                    eventHandleCancelCommAction();
                    a2dpStreamRestartAudioStream();
                }
                else
                {   /* No more devices in list - start another inquiry scan */
                    scanKickInquiryScan();
                }
            }
            break;
        }
        default:
        {
            DEBUG_SCAN((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    scanSearchComplete

DESCRIPTION
    Determines what action should be taken after all sdp searches have completed.
    
*/
void scanSearchComplete (devInstanceTaskData *inst)
{
    if ((inst != NULL) && scanIsSuitableDevice(inst) )
    {    /* Remote device supports A2DP + HSP/HFP, attempt to connect to it */
        setAppState(AppStateIdle);
        
        /* The device to connect to is the one currently stored as the search device */
        the_app->connect_bdaddr = the_app->search_bdaddr;
        
        kickCommAction(CommActionConnect);
    }
    else
    {    
        if (inst != NULL)
        {
            /* Attempt to delete device instance as SDP complete */
            MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
        }
        
        if ( the_app->comm_action != CommActionInquire )
        {    /* No response from last remote device - give up */
            setAppState(AppStateIdle);
            eventHandleCancelCommAction();
        }
        else
        {    /* Remote device does not support either HSP/HFP, try another device */
            setAppState(AppStateInquiring);
            MessageSend(&the_app->task, APP_PROCESS_INQUIRE_RESULT, 0);
        }
    }
}
