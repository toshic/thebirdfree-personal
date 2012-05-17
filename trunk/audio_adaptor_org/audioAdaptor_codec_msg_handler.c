/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file    audioAdaptor_codec_msg_handler.c
@brief    Handle codec library messages arriving at the app.
*/



#include "audioAdaptor_private.h"
#include "audioAdaptor_codec_msg_handler.h"

#include <codec.h>
#include <pio.h>
#include <panic.h>


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME
    codecMsgHandleLibMessage

DESCRIPTION
    Handles the codec library messages.

*/
void codecMsgHandleLibMessage(MessageId id, Message message )
{  
    switch (id)
    {
        case CODEC_INIT_CFM:
        {
            DEBUG_CODEC(("CODEC_INIT_CFM, Status:[%d]\n", ((CODEC_INIT_CFM_T *)message)->status ));
            DEBUG_CODEC(("Codec Type:[%d]\n", ((CODEC_INIT_CFM_T *)message)->type_of_codec ));
            DEBUG_CODEC(("InputGainRange:[%d]\n", ((CODEC_INIT_CFM_T *)message)->inputGainRange ));
            DEBUG_CODEC(("OutputGainRange:[%d]\n", ((CODEC_INIT_CFM_T *)message)->outputGainRange ));
            
            if(((CODEC_INIT_CFM_T *)message)->status == codec_success)
            {          
                the_app->codecTask = ((CODEC_INIT_CFM_T*)message)->codecTask ;
                
                DEBUG_CODEC(("CODEC_INIT_CFM, Enabled Codec:[%d]\n", CodecGetCodecType(the_app->codecTask) ));
                
                /* set current and voltage to magic values*/
                PioSetMicBiasHwEnabled(1);
                PioSetMicBiasHwCurrent(11);   
                PioSetMicBiasHwVoltage(7);
            }
            else
                Panic();
            break;
        }    
        case CODEC_CONFIGURE_CFM:
        {
            DEBUG_CODEC(("CODEC_CONFIGURE_CFM, status[%d]\n", ((CODEC_INIT_CFM_T *)message)->status ));
            if(((CODEC_INIT_CFM_T *)message)->status == codec_success)
            {   
        #ifdef ENABLE_EXTERNAL_ADC
                CodecSetInputGain(the_app->codecTask, 0x1f, left_and_right_ch);
        #endif
                CodecEnable(the_app->codecTask);
            }
            else
            {
                Panic();
            }
            break;
        }
        default:   
        {
            DEBUG_CODEC(("CODEC UNHANDLED MSG: 0x%x\n",id));
            break;
        }
    }    
}



