/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    a2dp_sbc.h
    
DESCRIPTION
	
*/

#ifndef A2DP_SBC_H_
#define A2DP_SBC_H_


/* Define the maximum data rate for 2 and 1 channel modes.  */
#define SBC_TWO_CHANNEL_RATE    (361500) /* half of Bluetooth max rate */
#define SBC_ONE_CHANNEL_RATE    (220000)


/*************************************************************************
NAME    
	getSbcConfigSettings
    
DESCRIPTION
    Get the sampling rate and channel mode from the codec config settings.
   
*/
void getSbcConfigSettings(const uint8 *service_caps, uint32 *rate, a2dp_channel_mode *channel_mode);


/*************************************************************************
NAME    
     selectOptimalSbcCapsSink
    
DESCRIPTION
    Selects the optimal configuration for SBC playback by setting a single 
	bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the AV profiles.
   
*/
void selectOptimalSbcCapsSink(const uint8 *local_caps, uint8 *caps);


/*************************************************************************
NAME    
    selectOptimalSbcCapsSource
    
DESCRIPTION
    Selects the optimal configuration for SBC playback by setting a single 
	bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the AV profiles.
   
*/
void selectOptimalSbcCapsSource(const uint8 *local_caps, uint8 *caps);


/****************************************************************************
NAME    
    a2dpSbcFormatFromConfig
    
DESCRIPTION
    Converts an A2DP SBC Configuration bitfield to the SBC header format.
    It assumes a valid configuration request with one bit in each field.

*/
uint8 a2dpSbcFormatFromConfig(const uint8 *config);


/****************************************************************************
NAME    
    a2dpSbcSelectBitpoolAndPacketSize
    
DESCRIPTION
    Calculates the optimum bitpool and packet size for the requested data rate using
    the specified SBC format.
    - Get frame size from rate.
    - Find optimum L2CAP PDU size for baseband packets.
    - Find integer multiple of frame size to fit in RTP packet.
    - Calculate bitpool required for this frame size (round down).
    
*/
uint8 a2dpSbcSelectBitpoolAndPacketSize(uint8 sbc_header, uint32 rate, uint16 l2cap_mtu, uint16 *packet_size);


#endif /* A2DP_SBC_H_ */
