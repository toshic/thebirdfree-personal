/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_events.h
@brief    Defines Headset user events
*/
#ifndef HEADSET_EVENTS_H
#define HEADSET_EVENTS_H


#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>


#define BC5_STEREO_MESSAGE_BASE 0x0
#define EVENTS_EVENT_BASE BC5_STEREO_MESSAGE_BASE


/* This enum is used as an index in an array - do not edit - without thinking */
typedef enum headsetEventsTag
{
/*0x00*/    EventInvalid = BC5_STEREO_MESSAGE_BASE,    
/*0x01*/    EventPowerOn , 
/*0x02*/    EventPowerOff ,  
/*0x03*/    EventEnterPairing ,
    
/*0x04*/    EventInitateVoiceDial ,
/*0x05*/    EventLastNumberRedial ,
/*0x06*/    EventAnswer , 
/*0x07*/    EventReject , 
    
/*0x08*/    EventCancelEnd , 
/*0x09*/    EventTransferToggle ,
/*0x0A*/    EventToggleMute   ,
/*0x0B*/    EventVolumeUp  ,    
/*0x0C*/    EventVolumeDown ,
            
/*0x0D*/    EventResetPairedDeviceList,            
/*0x0E*/    EventEnterDutState,            
/*0x0F*/    EventButtonLockingOn ,                        
            
/*0x10*/    EventPairingFail,
/*0x11*/    EventPairingSuccessful,
    
/*0x12*/    EventSCOLinkOpen ,
/*0x13*/    EventSCOLinkClose,
/*0x14*/    EventLowBattery,
/*0x15*/    EventEndOfCall ,
    
/*0x16*/    EventEstablishSLC ,
/*0x17*/    EventTrickleCharge,            
/*0x18*/    EventFastCharge,      
/*0x19*/    EventAutoSwitchOff,            
            
/*0x1A*/    EventOkBattery,
/*0x1B*/    EventChargerConnected,
/*0x1C*/    EventChargerDisconnected,

/*0x1D*/    EventSLCDisconnected ,
/*0x1E*/    EventHfpReconnectFailed ,
/*0x1F*/    EventLinkLoss ,
/*0x20*/    EventEnterDutMode ,

/*0x21*/    EventMuteOn ,
/*0x22*/    EventMuteOff ,
/*0x23*/    EventMuteReminder ,
/*0x24*/    EventResetComplete,            
            
/*0x25*/    EventSLCConnected ,
/*0x26*/    EventError,
/*0x27*/    EventTone1,
/*0x28*/    EventTone2,

/*0x29*/    EventToggleButtonLocking,
/*0x2A*/    EventChargeError,            
/*0x2B*/    EventA2dpReconnectFailed ,            
/*0x2C*/    EventLEDEventComplete,
/*0x2D*/    EventEnableLEDS,
/*0x2E*/    EventDisableLEDS,
/*0x2F*/    EventCancelLedIndication,
            
/*0x30*/    EventPlay,
/*0x31*/    EventPause,            
/*0x32*/    EventStop,            
/*0x33*/    EventFFWDPress,            
/*0x34*/    EventFFWDRelease,            
/*0x35*/    EventRWDPress,            
/*0x36*/    EventRWDRelease,            
/*0x37*/    EventSkipForward,            
/*0x38*/    EventSkipBackward,            
/*0x39*/    EventEstablishA2dp,            
/*0x3A*/    EventA2dpConnected,
/*0x3B*/	EventA2dpDisconnected,
			
/*0x3C*/	EventVolumeMax,			
/*0x3D*/	EventVolumeMin,
			
/*0x3E*/	EventEnterTXContTestMode,
/*0x3F*/	EventEnterDFUMode,
/*0x40*/	EventButtonLockingOff,

/*0x41*/    EventConfirmationAccept,
/*0x42*/    EventConfirmationReject,
/*0x43*/    EventToggleDebugKeys,

/*0x44*/    EventSwitchA2dpSource,

/*0x45*/    EventThreeWayReleaseAllHeld,
/*0x46*/    EventThreeWayAcceptWaitingReleaseActive,
/*0x47*/    EventThreeWayAcceptWaitingHoldActive,
/*0x48*/    EventThreeWayAddHeldTo3Way,
/*0x49*/    EventThreeWayConnect2Disconnect,
			
/*0x4A*/	EventBatteryLevelRequest,
/*0x4B*/    EventGasGauge0,
/*0x4C*/    EventGasGauge1,
/*0x4D*/    EventGasGauge2,
/*0x4E*/	EventSWversioncheckTest,
/*0x4F*/	EventAudioLoopback,
/*0x50*/	EventTestSinetone,
/*0x51*/	EventKeyTest,
/*0x52*/	EventSwitchAudioMode,
/*0x53*/	EventToggleLEDS,
/*0x54*/    EventRssiPair,
/*0x55*/    EventRssiPairReminder,
/*0x56*/	EventRssiPairTimeout
} headsetEvents_t; 

#define EVENTS_LAST_EVENT EventRssiPairTimeout

#define EVENTS_MAX_EVENTS ( (EVENTS_LAST_EVENT - EVENTS_EVENT_BASE) + 1 )


#endif
