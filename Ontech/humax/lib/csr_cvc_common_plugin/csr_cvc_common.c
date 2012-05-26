/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvc_common.c
    
DESCRIPTION
NOTES
*/

#ifndef SCO_DSP_BOUNDARIES
#define SCO_DSP_BOUNDARIES
#endif

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <print.h>
#include <kalimba.h>
#include <file.h>
#include <stream.h>     /*for the audio_note*/
#include <connection.h> /*for the link_type */
#include <string.h>
#include <kalimba_standard_messages.h>
#include "audio_plugin_if.h"        /*for the audio_mode*/
#include "csr_cvc_common_if.h"      /*for things common to all CVC systems*/
#include "csr_cvc_common_plugin.h"
#include "csr_cvc_common.h"

#include <source.h>
#include <app/vm/vm_if.h>

#define MESSAGE_SCO_CONFIG       (0x2000)

/*helper functions*/
static void CvcConnectAudio (CvcPluginTaskdata *task,  bool stereo );
static void CvcCodecMessage (CvcPluginTaskdata *task, uint16 input_gain_l,uint16 input_gain_r, uint16 output_gain ) ;
uint32 CsrCvcCalcDacRate ( CVC_PLUGIN_TYPE_T cvc_plugin_variant, uint32 rate );
static FILE_INDEX CsrCvcSelectKapFile(CVC_PLUGIN_TYPE_T cvc_plugin_variant, uint32 dac_rate);

typedef struct audio_Tag
{
        /*! Whether or not CVC is running */
   unsigned cvc_running:1 ;
   unsigned reserved:5 ;
        /*! mono or stereo*/
    unsigned stereo:1;
        /*! The current CVC mode */
    unsigned mode:8 ; 
        /*! whether mic switch active */
    unsigned mic_switch:1;
        /*! The codec being used*/
    Task codec_task ; 
        /*! The audio sink being used*/
    Sink audio_sink ;
        /*! The link_type being used*/
    AUDIO_SINK_T link_type ;
        /*! The current volume level*/
    uint16 volume ;
        /*! The current tone volume level*/
    uint16 tone_volume ;  
        /*! Over the air rate  */
    uint32  dac_rate;
        /*! Audio rate - used for mic switch */
    uint32 audio_rate;
}CVC_t ;

/* The CVC task instance pointer*/
static CVC_t * CVC = NULL;

/* dsp message structure*/
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;


static FILE_INDEX CsrCvcSelectKapFile(CVC_PLUGIN_TYPE_T cvc_plugin_variant, uint32 dac_rate)
{		
	FILE_INDEX index = 0;
	char* kap_file = NULL ;
	
	switch(cvc_plugin_variant)
	{
      case CVSD_CVC_1_MIC_HEADSET:
            PRINT(("CVC: load cvsd 1mic headset kap file\n"));
            kap_file = "cvc_headset/cvc_headset.kap";		
    	break;
    	case CVSD_CVC_2_MIC_HEADSET:
            PRINT(("CVC: load cvsd 2mic headset kap file\n"));
    		kap_file = 	"cvc_headset_2mic/cvc_headset_2mic.kap";
    	break;
    	case CVSD_CVC_1_MIC_HANDSFREE:
            PRINT(("CVC: load cvsd 1mic handsfree kap file\n"));
    		kap_file = 	"cvc_handsfree/cvc_handsfree.kap";
    	break;
    	case AURI_2BIT_CVC_1_MIC_HEADSET:
		case AURI_4BIT_CVC_1_MIC_HEADSET:
            PRINT(("CVC: load auri 1mic headset kap file\n"));
    		if(dac_rate ==8000)
    			kap_file = 	"cvc_headset/cvc_headset.kap";
    		else if(dac_rate == 16000)
    			kap_file = 	"cvc_headset_wb/cvc_headset_wb.kap";
    	break;
    	case AURI_2BIT_CVC_2_MIC_HEADSET:
		case AURI_4BIT_CVC_2_MIC_HEADSET:
			PRINT(("CVC: load auri 2mic headset kap file\n"));
    		if(dac_rate ==8000)
    		kap_file = 	"cvc_headset_2mic/cvc_headset_2mic.kap";
    		else if(dac_rate == 16000)
    			kap_file = 	"cvc_headset_wb/cvc_headset_wb.kap";
    	break;
    	case AURI_2BIT_CVC_1_MIC_HANDSFREE:
		case AURI_4BIT_CVC_1_MIC_HANDSFREE:
			PRINT(("CVC: load auri 1mic handsfree kap file\n"));
    		if(dac_rate == 8000)
    		kap_file = 	"cvc_handsfree/cvc_handsfree.kap";
    	break;
        case SBC_CVC_1_MIC_HEADSET:
            PRINT(("CVC: load sbc 1mic headset kap file\n"));
            kap_file = "cvc_headset_wb_wbs/cvc_headset_wb_wbs.kap";		
    	break;
      case SBC_CVC_1_MIC_HANDSFREE:
            PRINT(("CVC: load sbc 1mic handsfree kap file\n"));
            kap_file = "cvc_handsfree_wb_wbs/cvc_handsfree_wb_wbs.kap";		
    	break;
        case CVSD_CVC_LITE_1_MIC_HEADSET:
            PRINT(("CVC: load cvsd 1mic LITE headset kap file\n"));
            kap_file = "cvc_headset_lite/cvc_headset.kap";		
    	break ; 
		
		case CVSD_PURESPEECH_1_MIC_HEADSET:
            PRINT(("CVC: load cvsd PURESPEECH 1 mic kap file\n"));
            kap_file = "purespeech_1mic/purespeech_1mic.kap";		
    	break ; 
    	
    	default:
    		PRINT(("CVC: No Corresponding Kap file\n")) ;
    		Panic() ;
    	break;
	}
	
    index = FileFind(FILE_ROOT,(const char *) kap_file ,strlen(kap_file));
    
    if( index == FILE_NONE )
    {
         PRINT(("CVC: No File\n"));
	     Panic();
    }
   
	
	return(index);
}

uint32 CsrCvcCalcDacRate ( CVC_PLUGIN_TYPE_T cvc_plugin_variant, uint32 rate )
{
    uint32 dac_rate = 0 ;
	  /* Calculate the DAC rate based on over the air rate value and the type of codec
      It holds true for both 8K and 16K connection and calculates the rate correctly */
	
	switch ( cvc_plugin_variant ) 
	{
    	case AURI_2BIT_CVC_1_MIC_HEADSET:
		case AURI_2BIT_CVC_2_MIC_HEADSET:
		case AURI_2BIT_CVC_1_MIC_HANDSFREE:
            dac_rate = rate * 4;
	    break ;
		case AURI_4BIT_CVC_1_MIC_HEADSET:
		case AURI_4BIT_CVC_2_MIC_HEADSET:
		case AURI_4BIT_CVC_1_MIC_HANDSFREE:
	         dac_rate = rate * 2;
	    break ;	                
		/* For SBC case, we are currently hard coding the dac_rate to be 16K */
        case SBC_CVC_1_MIC_HEADSET:
		case SBC_CVC_1_MIC_HANDSFREE:  
            dac_rate = 16000;  
            break;
	    default :
			dac_rate = rate * 1;
        break ;
    }
    
    return dac_rate ;
}

/****************************************************************************
NAME	
	CsrCvcPluginConnect

DESCRIPTION
	This function connects cvc to the stream subsystem

RETURNS
	void
*/
void CsrCvcPluginConnect( CvcPluginTaskdata *task, Sink audio_sink , AUDIO_SINK_T sink_type, Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params, bool mic_switch )
{
    FILE_INDEX index; 
	
	/*signal that the audio is busy until the kalimba / parameters are fully loaded so that no tone messages etc will arrive*/
    AUDIO_BUSY = (TaskData*) task;    
    
    if (CVC) Panic();
    CVC = PanicUnlessNew ( CVC_t ); 
    
	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
    if (volume > 0xf)
		volume = 0xf;

    CVC->cvc_running     = FALSE;
    CVC->codec_task      = codec_task;                                                        
    CVC->link_type       = sink_type ;                                                        
    CVC->volume          = volume;
    CVC->audio_sink      = audio_sink;
    CVC->mode            = mode;
	CVC->stereo          = stereo;
    CVC->tone_volume     = volume;
    CVC->audio_rate      = rate;
    CVC->mic_switch      = mic_switch;
	
	 PRINT(("CVC: connect [%x] [%x]\n", CVC->cvc_running , (int)CVC->audio_sink));
	  
    /* Enable MetaData */
    SourceConfigure(StreamSourceFromSink( CVC->audio_sink ),VM_SOURCE_SCO_METADATA_ENABLE,1);

    /* Clear all routing to the PCM subsysytem*/
    PcmClearAllRouting();

	/* Calculate the DAC rate based on the over-the-air rate value passed in from VM */
	CVC->dac_rate = CsrCvcCalcDacRate( (CVC_PLUGIN_TYPE_T)task->cvc_plugin_variant, rate );
   
    /*ensure that the messages received are from the correct kap file*/ 
    (void) MessageCancelAll( (TaskData*) task, MESSAGE_FROM_KALIMBA);
    MessageKalimbaTask( (TaskData*) task );
   
	   /* Select which Kap file to be loaded based on the plugin selected */
    index = CsrCvcSelectKapFile((CVC_PLUGIN_TYPE_T)task->cvc_plugin_variant, CVC->dac_rate); 
    
    /* Load the cvc algorithm into Kalimba*/
    if( !KalimbaLoad( index ) )
    {
        PRINT(("CVC: Kalimba load fail\n"));
        Panic();
    }
   
    /* Now the kap file has been loaded, wait for the CVC_READY_MSG message from the
       dsp to be sent to the message_handler function. */     
}

/****************************************************************************
NAME	
	CsrCvcPluginDisconnect

DESCRIPTION
	Disconnect CVC and power off the Kalimba DSP core
    
RETURNS
	void
*/
void CsrCvcPluginDisconnect( CvcPluginTaskdata *task )
{
    if (!CVC)
        Panic() ;
    if ( CVC->cvc_running == FALSE )
        Panic() ;
 
    CodecSetOutputGainNow( CVC->codec_task, DAC_MUTE, left_and_right_ch );    
    
    PRINT(("CVC: discon_mic\n"));
    StreamDisconnect(StreamPcmSource(0), 0); 
    StreamDisconnect(StreamKalimbaSource(1), 0); 
	/* Disconnect PCM stream from mic_2 if we are unloading 2mic CVC  */
	if( task->two_mic )
	{
		PRINT(("CVC: disconnect PCM source 1\n"));
		StreamDisconnect(StreamPcmSource(1), 0);
	}
        
    PRINT(("CVC: discon_spkr\n"));
    StreamDisconnect(0, StreamPcmSink(0));  
    StreamDisconnect(0, StreamKalimbaSink(1) );

    CVC->cvc_running = FALSE;	
    CVC->audio_sink = NULL;
    CVC->link_type = 0;
    
    PRINT(("CVC: Disconnect\n"));
 
    /* Cancel any outstanding cvc messages */
    MessageCancelAll( (TaskData*)task , MESSAGE_FROM_KALIMBA);
    MessageCancelAll( (TaskData*)task , MESSAGE_STREAM_DISCONNECT);
				    
    free (CVC);
    CVC = NULL;                
    
    KalimbaPowerOff();        
}

/****************************************************************************
NAME	
	CsrCvcPluginSetVolume

DESCRIPTION
	Tell CVC to update the volume.

RETURNS
	void
*/
void CsrCvcPluginSetVolume( uint16 volume )
{
    if (!CVC)
        Panic() ;
 
	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
	if (volume > 0xf)
		volume = 0xf;
    
	CVC->volume = volume;
          
    PRINT(("CVC: DAC GAIN SET[%x]\n", CVC->volume ));
    
    /* Only update the volume if not in a mute mode */
    if ( CVC->cvc_running && !( (CVC->mode==AUDIO_MODE_MUTE_SPEAKER ) || (CVC->mode==AUDIO_MODE_MUTE_BOTH ) ) )
    { 
       KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume); 
    }         
}

/****************************************************************************
NAME	
	CsrCvcPluginSetMode

DESCRIPTION
	Set the CVC mode

RETURNS
	void
*/
void CsrCvcPluginSetMode ( AUDIO_MODE_T mode , const void * params )
{
    
    if (!CVC)
        Panic();            
    if ( CVC->cvc_running == FALSE )
        Panic();
    
    CVC->mode = mode;
    
    if (params)
    {
        cvc_extended_parameters_t * cvc_params = (cvc_extended_parameters_t *) params;
        switch (*cvc_params)
        {
        case CSR_CVC_HFK_ENABLE:
        {
            KalimbaSendMessage(CVC_SETMODE_MSG , SYSMODE_HFK , 0, CALLST_CONNECTED, 0 );
            KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume);
            PRINT(("CVC : Set Mode SYSMODE_HFK CALLST_CONNECTED\n"));
        }    
        break;
        case CSR_CVC_PSTHRU_ENABLE:
        {
            KalimbaSendMessage (CVC_SETMODE_MSG , SYSMODE_PSTHRGH , 0, CALLST_CONNECTED, 0 ) ;
            KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, 0);
   	        PRINT(("CVC : Set Mode SYSMODE_PSTHRGH CALLST_CONNECTED\n"));
        }
        break;
        default:
        break;
        }        
        free(cvc_params);
    }
    else
    {
    switch (mode)
    {
	    case AUDIO_MODE_MUTE_SPEAKER:
	    {     
           KalimbaSendMessage(CVC_SETMODE_MSG, SYSMODE_ASR, 0, CALLST_CONNECTED, 0 );
           KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, DAC_MUTE, CVC->tone_volume);
           PRINT(("CVC: Set DAC gain to 0 and SYMODE_ASR\n"));
		 }
	    break ;
	    case AUDIO_MODE_CONNECTED:
	    {
           KalimbaSendMessage(CVC_SETMODE_MSG , SYSMODE_HFK , 0, CALLST_CONNECTED, 0 );
           KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume);
           PRINT(("CVC: Set Mode SYSMODE_HFK CALLST_CONNECTED\n"));
	    }
	    break ;
	    case AUDIO_MODE_MUTE_MIC:
	    {
		   KalimbaSendMessage (CVC_SETMODE_MSG , SYSMODE_HFK , 0, CALLST_MUTE, 0 );          
           KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume);
	       PRINT(("CVC: Set Mode SYSMODE_HFK CALLST_MUTE\n"));
	    }
	    break ;      
	    case AUDIO_MODE_MUTE_BOTH:
	    {
		   KalimbaSendMessage (CVC_SETMODE_MSG , SYSMODE_STANDBY, 0, 0, 0);          
           KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, DAC_MUTE, CVC->tone_volume);
	       PRINT(("CVC: Set Mode SYSMODE_HFK MUTE BOTH - Standby\n"));
	    }
	    break;    
	    case AUDIO_MODE_SPEECH_REC:
	    {
           KalimbaSendMessage(CVC_SETMODE_MSG, SYSMODE_ASR, 0, CALLST_CONNECTED, 0 );
           KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume);
           PRINT(("CVC: Set Mode SYMODE_ASR CALLST_CONNECTED\n"));
		 }
	    break;   
	    default:
	    {    /*do not send a message and return false*/    
	       PRINT(("CVC: Set Mode Invalid [%x]\n" , mode ));
		 }
		break ;
    }
    }   /* for the else */
}

/****************************************************************************
NAME	
	CsrCvcPluginPlayTone

DESCRIPTION

    queues the tone if can_queue is TRUE and there is already a tone playing

RETURNS
	false if a tone is already playing
    
*/
void CsrCvcPluginPlayTone (CvcPluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo)  
{   

    Source lSource ;  
    Sink lSink ; 
        
    if (!CVC)
        Panic() ;
    if ( CVC->cvc_running == FALSE )
        Panic() ;       

    PRINT(("CVC: Tone Start\n"));

	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
	if (tone_volume > 0xf)
		tone_volume = 0xf;

    /* set DAC gain to a suitable level for tone play */
    if (tone_volume != CVC->tone_volume)
    {
      CVC->tone_volume = tone_volume;
      KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, CVC->volume, CVC->tone_volume); 
    }

    lSink = StreamKalimbaSink(3);
    
    /*request an indication that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink , (TaskData*) task );

    /*connect the tone*/
    lSource = StreamAudioSource ( (const audio_note *) (tone) );    
 
	/*mix the tone to the CVC*/    
    StreamConnectAndDispose( lSource , lSink );

}

/****************************************************************************
NAME	
	CsrCvcPluginStopTone

DESCRIPTION
	Stop a tone from playing

RETURNS
	whether or not the tone was stopped successfully
*/
void CsrCvcPluginStopTone ( void ) 
{
    if (!CVC)
        Panic() ;
        
    StreamDisconnect( 0 , StreamKalimbaSink(3) ) ; 
}


/****************************************************************************
DESCRIPTION
	Connect the audio stream (Speaker and Microphone)
*/
static void CvcConnectAudio (CvcPluginTaskdata *task, bool stereo )
{             
    bool r1, r2, r3 =0, r4 = 0;
    if ( CVC->audio_sink )
    {	            
        /* DSP is up and running */
        CVC->cvc_running = TRUE ;
        
        /* Set DAC gain to minimum value before connecting streams */
        CodecSetOutputGainNow( CVC->codec_task, 0 , left_and_right_ch );        

        /* Configure port 0 to be routed to internal ADC and DACs */
	    if (stereo & !(task->two_mic))         /* configure A_AND_B only if 1mic and in stereo  */
        	PanicFalse(PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) CVC->dac_rate, (uint32) CVC->dac_rate, VM_PCM_INTERNAL_A_AND_B));
        else
        	PanicFalse(PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) CVC->dac_rate, (uint32) CVC->dac_rate, VM_PCM_INTERNAL_A));
		       
        if( task->two_mic )
        {
            PRINT(("CVC: Connect PCM source 2\n"));
   			PanicFalse(PcmRateAndRoute(1, 0, (uint32) CVC->dac_rate, (uint32) CVC->dac_rate, VM_PCM_INTERNAL_B));
        }

        if( CVC->mic_switch )
        {
            if( task->two_mic )
    		{
                /* connect mic 1 */
                PRINT(("ADC_LEFT->DSP\n"));
                PanicFalse(r3 = StreamConnect(StreamPcmSource(0),StreamKalimbaSink(2)));   /* ADC_LEFT->DSP */
                PRINT(("DSP WP 2 -> DAC 0\n"));
                PanicFalse(r4 = StreamConnect(StreamKalimbaSource(2),StreamPcmSink(0)));    /* DSP WP 2 ->DAC 0 */
                PRINT(("r4 = %d\n", r4));
    		}

            /* Connect Ports to DSP */
            PRINT(("ADC_RIGHT->DSP\n"));
            PanicFalse(r1 = StreamConnect( StreamPcmSource(1),StreamKalimbaSink(0)));  /* ADC_RIGHT->DSP */
            PRINT(("DSP WP 0 -> DAC 1\n"));
            PanicFalse(r2 = StreamConnect(StreamKalimbaSource(0),StreamPcmSink(1)));   /* DSP WP 0 ->DAC 1*/

            PRINT(("CVC: Streams connected: adc_r= %d, dac_r= %d, adc_l= %d, dac_l= %d\n",r1,r2,r3,r4)); 

        }
        
        else
        {
            if( task->two_mic )
		    {
                PanicFalse(r3 = StreamConnect( StreamPcmSource(1),StreamKalimbaSink(2)));  /* ADC_RIGHT->DSP */
                PanicFalse(r4 = StreamConnect(StreamKalimbaSource(2),StreamPcmSink(1)));   /* DSP WP 2 ->DAC 1 */
    		}

    		/* Connect Ports to DSP */
            PanicFalse(r1 = StreamConnect( StreamPcmSource(0),StreamKalimbaSink(0)));  /* ADC_LEFT->DSP */
    		PanicFalse(r2 = StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0)));   /* DSP WP 0 ->DAC 0*/

            PRINT(("CVC: Streams connected: adc_l= %d, dac_l= %d, adc_r= %d, dac_r= %d\n",r1,r2,r3,r4)); 
    
        }
        
        r1 = StreamConnect(StreamSourceFromSink( CVC->audio_sink ),StreamKalimbaSink(1)); /* SCO->DSP */  
        r2 = StreamConnect( StreamKalimbaSource(1), CVC->audio_sink ); /* DSP->SCO */
        PRINT(("CVC: Encoded streams connected: SCO->DSP= %d, DSP->SCO= %d \n",r1,r2));  
        
		KalimbaSendMessage(MESSAGE_SCO_CONFIG, task->sco_encoder, task->sco_config , 0, 0);

		if((task->cvc_plugin_variant == SBC_CVC_1_MIC_HEADSET) || (task->cvc_plugin_variant == SBC_CVC_1_MIC_HANDSFREE))
        {
						/* Configure SBC encoding format */
						/*   bit 8:    force word (16 bit) aligned packets  = 0
						bit 6-7:  sampling frequency                   = 0
						bit 4-5:  blocks                               = 3
						bit 2-3:  channel_mode                         = 0
						bit 1:    allocation method                    = 0
						bit 0:    subbands                             = 1 */
				KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_PARAMS, 0x0031, 0, 0, 0);
      
				/* Pass bit pool value to DSP */
				KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_BITPOOL, 27, 0, 0, 0);
        } 
  

        /* Set the mode */
        CsrCvcPluginSetMode ( CVC->mode , NULL );
    }
    else
    {   
        /*Power Down*/
        CsrCvcPluginDisconnect(task);
    }
}                

/****************************************************************************
DESCRIPTION
	Handles a CVC_CODEC message received from CVC
*/
static void CvcCodecMessage (CvcPluginTaskdata *task, uint16 input_gain_l, uint16 input_gain_r, uint16 output_gain )
{   
    PRINT(("CVC: Output gain = 0x%x\n" , output_gain ));
    PRINT(("CVC: Input gain  = 0x%x ,0x%x\n" , input_gain_l,input_gain_r ));
    
    /* check pointer validity as there is a very small window where a message arrives
       as the result of playing a tone after CVC has been powered down */
    if(CVC)
    {
        /*Set the output Gain immediately*/
        CodecSetOutputGainNow( CVC->codec_task, output_gain, left_and_right_ch);
    
        /*only enable the pre amp if asked to do so*/
        /*ie if the top bit (0x8000) is set */
        CodecEnableMicInputGainA( ( input_gain_l >> 15 ) & 0x1 );
	    
    
        /* Clear the upper bytes of the input gain argument */
		if( task->two_mic )
		{
			CodecEnableMicInputGainB( ( input_gain_r >> 15 ) & 0x1 );
			CodecSetInputGainNow( CVC->codec_task, (input_gain_l & 0xFF), left_ch);
			CodecSetInputGainNow( CVC->codec_task, (input_gain_r & 0xFF), right_ch);
		}
		else
		{
			CodecEnableMicInputGainB( ( input_gain_l >> 15 ) & 0x1 );
			CodecSetInputGainNow( CVC->codec_task, (input_gain_l & 0xFF), left_and_right_ch);

		}
    }
}


/****************************************************************************
DESCRIPTION
	handles the internal cvc messages /  messages from the dsp
*/
void CsrCvcPluginInternalMessage( CvcPluginTaskdata *task ,uint16 id , Message message ) 
{
	switch(id)
	{
        case MESSAGE_FROM_KALIMBA:
		{
			const DSP_REGISTER_T *m = (const DSP_REGISTER_T *) message;
	        PRINT(("CVC: msg id[%x] a[%x] b[%x] c[%x] d[%x]\n", m->id, m->a, m->b, m->c, m->d));
		
            switch ( m->id ) 
			{
              case CVC_READY_MSG:
                {
                    if(CVC->dac_rate == 8000)
						KalimbaSendMessage(CVC_LOADPARAMS_MSG, CVC_PS_BASE, 0, 0, 0);
					else if (CVC->dac_rate == 16000)
						KalimbaSendMessage(CVC_LOADPARAMS_MSG, CVC_PS_BASE_2, 0, 0, 0);
					else{
						PRINT(("CVC: Unknown dac_rate.\n"));
				        Panic();
					}
                    
                        /*cvc is now loaded, signal that tones etc can be scheduled*/
                    AUDIO_BUSY = NULL ;
                    
                    PRINT(("CVC: CVC_READY, SysId[%x] BuildVersion[%x] \n",m->a, m->b));
                    
                    CvcConnectAudio (task, CVC->stereo) ;                    
                }
                break;				
			    case CVC_CODEC_MSG:
	            {                    	            
                    uint16 lOutput_gain = m->a;
                    uint16 lInput_gain_l  = m->b;
					uint16 lInput_gain_r  = m->c;
      
                    CvcCodecMessage (task, lInput_gain_l,lInput_gain_r, lOutput_gain );    
	            }
                break;
				
			    case CVC_SECPASSED_MSG:
				       PRINT(("CVC:  Sec passed.\n"));
				    break;

			    case CVC_SECFAILED_MSG:
                  PRINT(("CVC: Security has failed.\n"));
                  break;
            }
		}
		break;
	
        default:
        break ;
	}		
}	

/****************************************************************************
DESCRIPTION
	a tone has completed
*/
void CsrCvcPluginToneComplete( void ) 
{
   
   /* Restore the DAC gain to mute if in mute mode */
   if ( CVC->cvc_running && (CVC->mode==AUDIO_MODE_MUTE_SPEAKER || CVC->mode==AUDIO_MODE_MUTE_BOTH ) )
      KalimbaSendMessage(CVC_VOLUME_MSG, 0, 0, DAC_MUTE, CVC->tone_volume);   
   
   /* We no longer want to receive stream indications */
   MessageSinkTask (StreamKalimbaSink(3) , NULL);   
}

/****************************************************************************
NAME
    CsrCvcPluginMicSwitch

DESCRIPTION
    Swap between the microphone inputs for production test

RETURNS
    
*/
void CsrCvcPluginMicSwitch( CvcPluginTaskdata *task )
{
    Sink audio_sink;
    AUDIO_SINK_T sink_type;
    Task codec_task;
    uint16 volume;
    uint32 rate;
    bool stereo;
    AUDIO_MODE_T mode;
    
    if ( CVC->audio_sink )
    {
        /* CVC parameters are lost with audio disconnect */
        audio_sink = CVC->audio_sink;
        sink_type = CVC->link_type;
        codec_task = CVC->codec_task;
        volume = CVC->tone_volume;
        rate = CVC->audio_rate;
        stereo = CVC->stereo;
        mode = CVC->mode;
        
        /* disconnect audio */
        CsrCvcPluginDisconnect(task);
        
        /* reconnect audio with swapped mics */
        CsrCvcPluginConnect(task, audio_sink, sink_type, codec_task, volume, rate, stereo, mode, NULL, TRUE);

    }
}
