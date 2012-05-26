/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvc_common_if.h

DESCRIPTION
    Message definitions for cVc plugins.           
   
*/

/*!
@file csr_cvc_common_if.h
  
@brief 
   Message definitions for cVc plugins.
  
   @description
      This file provides documentation for the messaging API between the VM
      application and the cVc algorithm running on the kalimba
      DSP.  All cVc systems share a common API, except where noted.
      
      A high level description of the boot sequence is:\n
      1) VM starts cVc using the KalimbaLoad function.\n
      2) VM receives CVC_READY_MSG and responds by:\n
         - sending CVC_LOADPARAMS_MSG.
         - muting the DAC.
         - sending the SET_SCOTYPE_MSG message.
         - connecting the streams to the DSP.
         - sending the CVC_SETMODE_MSG message.
         - sending the CVC_VOLUME_MSG message.
         
      Volume is handled with cVc in the following manner:\n
      1) VM sends the CVC_VOLUME_MSG.\n
      2) cVc does internal calculations and sends the CVC_CODEC_MSG.\n
      3) VM receives CVC_CODEC_MSG and sets CODEC gains.\n
      
      Tone mixing is handled with cVc in the following manner:\n
      1) VM defines the tone.\n
      2) VM sets the digital tone gain value using CVC_VOLUME_MSG.\n
      3) VM connects the tone stream to port 3 on the kalimba.\n
      
      Note that there are 3 gain values that control the tone volume:\n
      1) the volume used in the VM's definition of the tone.\n
      2) the digital gain value specified in CVC_VOLUME_MSG.\n
      3) the DAC gain that is used while the tone is playing.\n
      
*/

#ifndef _CSR_CVC_COMMON_INTERFACE_H_
#define _CSR_CVC_COMMON_INTERFACE_H_

/* Messages between Kalimba and VM */
typedef enum 
{
/*!
   MESSAGE:  cVc is ready to recieve messages (KALIMBA --> VM APPLICATION)

   @brief
   Indicates that cVc has powered up and is ready to receive messages from
   the VM application. This is the first message that cVc sends after the 
   VM application has loaded cVc using the KalimbaLoad function.  The VM
   application should respond by sending the CVC_LOADPARAMS_MSG message and
   by connecting the SCO, DAC, and ADC streams to the DSP ports.
   
   @param AlgID (0), Algorithm ID (4 digit hexidecimal)
   @param BuildNumber (1), Software Build Number (4 digit hexidecimal)\n
*/
    CVC_READY_MSG       = 0x1000 ,
            
   /*!
  MESSAGE:  VM sets cVc's mode of operation (VM APPLICATION --> KALIMBA)

   @brief
   The VM controls the mode of the cVc software using this message.  The VM
   must do this after it sends the CVC_LOADPARAMS_MSG because the
   CVC_LOADPARAMS_MSG will put cVc into SYSMODE_STANDBY.  The VM can also 
   send this message anytime a mode change is necessary.
   
   @param Mode (0), SYSMODE_HFK, SYSMODE_NS, SYSMODE_PSTHRGH, SYSMODE_LPBACK
        or SYSMODE_STANDBY
   @param NotUsed (1), This parameter is not used.
   @param CallState (2), CALLST_CONNECTING, CALLST_CONNECTED, or CALLST_MUTE\n
*/
    CVC_SETMODE_MSG     = 0x1001 ,

   /*!
   MESSAGE:  VM requests DAC gain change from cVc
             (VM APPLICATION --> KALIMBA)

   @brief
   The VM application needs to send this message to cVc to change the DAC
   gain.  cVc uses the DAC gain for internal calculations and then sends the
   gain back to the VM application using the CVC_CODEC_MSG.
   
   @param NotUsed (0), This parameter is not used.
   @param AuxGain (1), Q5.19 gain value applied to the Auxiliary Stream.
   @param DacGain (2), valid range is 0 through 0xf.\n 
*/
    CVC_VOLUME_MSG      = 0x1002 ,
    
   /*!
   MESSAGE:  VM changes one cVc parameter (VM APPLICATION --> KALIMBA)

   @brief
   This message is generally unused, but it is available if needed.
   Not supported in cvc_headset_276.
   
   @param ParameterID (0), Index into cVc Parameters Array.
   @param MSW (1), MSW of the value.  (would be  0x0012, for 0x123456)
   @param LSW (2), LSW of the value.  (would be  0x3456, for 0x123456)
   @param Status (3), should be zero if the VM is immediately sending more      
                      parameters, which will put the system into
                      SYSMODE_STANDBY.  Should be 1 if the VM is done sending
                      parameters.  The VM will then need to send a
                      CVC_SETMODE_MSG to restore the mode.\n
*/   
    CVC_SETPARAM_MSG    = 0x1004 ,

   /*!
   MESSAGE:  cVc tells VM to set the DAC gain (KALIMBA  --> VM APPLICATION)

   @brief
   This message tells the VM application which CODEC gain values to use with
   the internal CODEC. 
   
   @param DacGain (0), should be used as argument to CodecSetOuputGainNow
                       function.
   @param AdcGain (1), if bit 15 is high the mic preamp should be enabled,
                       otherwise it should be disabled.  Bits 0 through 3
                       should be masked and used as the argument to
                       CodecSetInputGainNow.\n
*/  
    CVC_CODEC_MSG       = 0x1006 ,
    
/*!
   MESSAGE:  VM asks cVc if it is processing data (VM APPLICATION --> KALIMBA)

   @brief
   This message requests the frame counter value from cVc.   
   This message does not have any parameters.\n   
   Not supported in cvc_headset_276.
*/
    CVC_PING_MSG        = 0x1008 ,
  
/*!
   MESSAGE:  cVc tells VM that it is processing data (KALIMBA  --> VM APPLICATION)

   @brief
   This message sends a counter value to the VM, which is incremented each
   time cVc processes a block of data.  It is sent in response to
   CVC_PING_MSG.
   Not supported in cvc_headset_276.
   @param FrameCounter (0), counter value.\n
*/
    CVC_PINGRESP_MSG    = 0x1009 ,

/*!
   MESSAGE:  VM tells DSP application framework to set buffer copy limits
             according to the link_type   (KALIMBA  --> VM APPLICATION)

   @brief
   This message sends the connection type to the DSP so that the DSP can set
   the appropriate buffer copy limits. 

   @param ConnectionType (0), WIRED_LINK_TYPE, SCO_LINK_TYPE, ESCO_LINK_TYPE.\n 
*/
    
    SET_SCOTYPE_MSG     = 0x100d ,
 
/*!
   MESSAGE:  cVc tells VM that the cVc security key is valid
            (KALIMBA  --> VM APPLICATION)

   @brief
   cVc sends this message during start up to indicate that the security key
   stored in PS_USER_KEY 28 is consistent with the Bluetooth address.
   
   This message does not have any parameters.\n 
*/    
    
    CVC_SECPASSED_MSG   = 0x100c ,
    
/*!
   MESSAGE:  VM requests a parameter value from cVc (VM APPLICATION --> KALIMBA)

   @brief
   This message is used to retrieve the value of the parameter specified by P0.
   Not supported in cvc_headset_276.
   @param ParameterId (0), index into cVc parameters arrary\n
*/ 
    
    CVC_GETPARAM        = 0x1010 ,
    
/*!
   MESSAGE:  cVc tells VM a parameter value from cVc (KALIMBA --> VM APPLICATION)

   @brief
   cVc sends this message as a response to CVC_GETPARAM.
   Not supported in cvc_headset_276.
   @param ParameterId (0), index of parameter being requested.
   @param ReturnedId (1), index of parameter being returned.
   @param ParameterId (2), MSW of value being returned.
   @param ReturnedId (3), LSW of value being returned.\n     
*/ 
    CVC_GETPARAM_RESP   = 0x1011 ,
    
    /*!
   MESSAGE:  VM tells cVc the primary Pskey that contains the cVc parameters.
             (VM APPLICATION --> KALIMBA)

   @brief
   The VM application tells cVc the primary Pskey that contains the
   cVc parameters so that the DSP can retrieve values from PS directly.
   In addition to containing cVc parameters, this key contains the USER IDs
   of any other Pskeys containing cVc parameters.
      
   Each cVc system has a corresponding Windows Parameter Manager that must be
   used to create parameters that are stored in PS.  Only values that are tuned
   to be different from their defaults are stored.  Each key can hold up to 40
   cVc parameter values.  The Parameter Manager allows the user to specify which
   USER KEYS the cVc parameters get stored in.

   @param PrimaryKey (0), contains cVc parameters and a list of sub-keys
                          containing cVc parameters\n      
*/     
    CVC_LOADPARAMS_MSG  = 0x1012,
                          
/*!
   MESSAGE:  cVc tells VM that the cVc security key is invalid
            (KALIMBA  --> VM APPLICATION)

   @brief
   cVc sends this message during start up to indicate that the security key
   stored in PS_USER_KEY 48 is not consistent with the Bluetooth address.
   
   This message does not have any parameters.\n 
*/    
    CVC_SECFAILED_MSG   = 0x1013

} CVC_MESSAGE_T ;

/*!
   @brief PSKEY base for narrowband cVc applications is PSKEY_DSP40.\n
*/
#define CVC_PS_BASE 0x2280

/*!
   @brief PSKEY base for wideband cVc applications is PSKEY_DSP44.\n
*/
#define CVC_PS_BASE_2 0x2284

/* cVc mode values */
typedef enum 
{
/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   All cVc processing modules enabled.\n
*/
    SYSMODE_HFK         = 1,
/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   Acoustic echo canceller and sidetone are disabled.\n
*/
    SYSMODE_ASR         = 2,
/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   All processing is disabled\n
   ADC is routed to BT-TX and BT-RX is routed to DAC.\n
   Sidetone is disabled.\n
*/
    SYSMODE_PSTHRGH     = 3,
/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   All processing is disabled.\n
   ADC is routed to DAC and BT-RX is routed to BT-TX.\n
   Sidetone is disabled.\n
*/
    SYSMODE_LPBACK      = 5,

/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   All processing is disabled and all audio is muted.\n
*/
    SYSMODE_STANDBY     = 6
                      
} CVC_MODES_T ;

/* cVc call state values */
typedef enum 
{
/*!  
   @brief Parameter value for CVC_SETMODE_MSG.\n
   Puts the AEC into a full-duplex operational
   state.  This call state is used once a call is answered and is maintained
   throughout a conversation. This call state only affects the algorithm while
   running in the SYSMODE_HFK mode.\n
*/
    CALLST_CONNECTED    = 1,

/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   Puts the AEC into a half-duplex operational
   state.  It is used during an incoming ring-tone and blocks any acoustic
   ring-tone energy that might be picked up by the microphone from entering the
   phone network.  This call state only affects the algorithm while running in the
   SYSMODE_HFK mode.\n
*/
    CALLST_CONNECTING    = 2, 
    
/*!
   @brief Parameter value for CVC_SETMODE_MSG.\n
   Mutes the send-out output of cVc
   (i.e. output on the far-side).  It is essential that the VM programmer
   sends this mute message to mute the audio instead of muting the microphone
   input (ADC gain).  This call state affects the algorithm in SYSMODE_HFK
   and SYSMODE_ASR processing modes.\n
*/
    CALLST_MUTE           = 3
    
 } CVC_CALL_STATES_T ;

/* cVc framework values */
typedef enum 
{
/*!
   @brief Parameter value for SET_SCOTYPE_MSG.\n
   Sets the copy limits according to the link_type.\n
*/
    DEFAULT_LINK_TYPE     = 0,
/*!
   @brief Parameter value for SET_SCOTYPE_MSG.\n
   Sets the copy limits according to the link_type.\n
*/
    SCO_LINK_TYPE         = 1,
/*!
   @brief Parameter value for SET_SCOTYPE_MSG.\n
   Sets the copy limits according to the link_type.\n
*/
    ESCO_LINK_TYPE        = 2 
                            
 } CVC_LINK_TYPE_T ;    
    
/*!
   @brief Parameter value for CVC_VOLUME_MSG.\n
   This is 24 dB of gain in Q5.19 format.\n
*/
#define TWENTY_FOUR_dB     0x7fff  

/*!
   @brief Parameter value for CVC_VOLUME_MSG.\n
   This is a 5.19 digital gain value applied to the auxiliary audio stream.
*/
#define TONE_DSP_GAIN      TWENTY_FOUR_dB

/*!
   @brief This define is used to set the DAC gain to minus 45 dB.\n
*/
#define MINUS_45dB         0x0         /* value used with SetOutputGainNow VM trap */

/*!
   @brief This define is used to set the DAC gain to minus 45 dB.\n
*/
#define DAC_MUTE           MINUS_45dB


/* cVc plugin types */
/* Values for the selecting the plugin variant in the CvcPluginTaskdata structure  */
typedef enum
{

   CVSD_CVC_1_MIC_HEADSET        =  0,

   CVSD_CVC_2_MIC_HEADSET        =  1,

   CVSD_CVC_1_MIC_HANDSFREE      =  2,

   AURI_2BIT_CVC_1_MIC_HEADSET   =  3,

   AURI_2BIT_CVC_2_MIC_HEADSET   =  4,

   AURI_2BIT_CVC_1_MIC_HANDSFREE =  5,
   
   AURI_4BIT_CVC_1_MIC_HEADSET   =  6,

   AURI_4BIT_CVC_2_MIC_HEADSET   =  7,

   AURI_4BIT_CVC_1_MIC_HANDSFREE =  8,
   
   SBC_CVC_1_MIC_HEADSET         =   9,
   
   CVSD_CVC_LITE_1_MIC_HEADSET   =  10,
   
   CVSD_PURESPEECH_1_MIC_HEADSET =  11,
   
   SBC_CVC_1_MIC_HANDSFREE       =  12

}CVC_PLUGIN_TYPE_T;

/* Different types of SCO stream data encoders  */
typedef enum
{
   SCO_ENCODING_CVSD    =  0,

   SCO_ENCODING_AURI    =  1,

   SCO_ENCODING_SBC     =  2

}SCO_ENCODING_TYPE_T;

/* The CODEC type selected from VM  application */
typedef enum
{
   AUDIO_CODEC_CVSD     =  1,

   AUDIO_CODEC_2BIT_AURI   =  2,

   AUDIO_CODEC_4BIT_AURI   =  4

}CODEC_TYPE_T;

typedef enum
{
    CSR_CVC_HFK_ENABLE ,            /*  0   */
    CSR_CVC_PSTHRU_ENABLE           /*  1   */
} cvc_extended_parameters_t ;



#endif

