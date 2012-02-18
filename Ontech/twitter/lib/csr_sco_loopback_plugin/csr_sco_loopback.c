/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_sco_loopback.c
    
DESCRIPTION
NOTES
*/


#ifndef SCO_LOOPBACK_BOUNDARIES
#define SCO_LOOPBACK_BOUNDARIES
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
#include <source.h>
#include <app/vm/vm_if.h>
#include "audio_plugin_if.h"        /*for the audio_mode*/
#include "csr_sco_loopback_if.h"      /*for things common to all CSR_COMMON_EXAMPLE systems*/
#include "csr_sco_loopback_plugin.h"
#include "csr_sco_loopback.h"


/*helper functions*/
static void ConnectAudio (ExamplePluginTaskdata *task,  bool stereo );
static void CodecMessage (ExamplePluginTaskdata *task, uint16 input_gain_l,uint16 input_gain_r, uint16 output_gain ) ;
static FILE_INDEX CsrSelectKapFile(void);

typedef struct audio_Tag
{
        /*! Whether or not CSR_COMMON_EXAMPLE is running */
   unsigned running:1 ;
   unsigned reserved:6 ;
        /*! mono or stereo*/
    unsigned stereo:1;
        /*! The current CSR_COMMON_EXAMPLE mode */
    unsigned mode:8 ;    
        /*! The codec being used*/
    Task codec_task ; 
        /*! The audio sink being used*/
    Sink audio_sink ;
        /*! The link_type being used*/
    sync_link_type link_type ;
        /*! The current volume level*/
    uint16 volume ;
        /*! The current tone volume level*/
    uint16 tone_volume ;
    /*! Codec type selected: CVSD or Auristream mode (2 bit or 4 bit) */
    uint16 codec_mode ;    
        /*! Over the air rate  */
    uint32  dac_rate;	
}EXAMPLE_t ;

/* The task instance pointer*/
static EXAMPLE_t * CSR_COMMON_EXAMPLE = NULL;



static FILE_INDEX CsrSelectKapFile(void)
{		
	FILE_INDEX index = 0;
	char* kap_file = "sco_dsp_kalimba/sco_dsp_kalimba.kap";		
	
    index = FileFind(FILE_ROOT,(const char *) kap_file ,strlen(kap_file));
    
    if( index == FILE_NONE )
    {
         PRINT(("CSR_COMMON_EXAMPLE: No File\n"));
	     Panic();
    }
	
	return(index);
}

/****************************************************************************
NAME	
	CsrCvcPluginConnect

DESCRIPTION
	This function connects cvc to the stream subsystem

RETURNS
	void
*/
void CsrExamplePluginConnect( ExamplePluginTaskdata *task, Sink audio_sink , AUDIO_SINK_T sink_type, Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params )
{
    FILE_INDEX index; 
	
	/*signal that the audio is busy until the kalimba / parameters are fully loaded so that no tone messages etc will arrive*/
    AUDIO_BUSY = (TaskData*) task;    
    
    if (CSR_COMMON_EXAMPLE) Panic();
    CSR_COMMON_EXAMPLE = PanicUnlessNew ( EXAMPLE_t ); 
    
	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
    if (volume > 0xf)
		volume = 0xf;

    CSR_COMMON_EXAMPLE->running     = FALSE;
    CSR_COMMON_EXAMPLE->codec_task      = codec_task;                                                        
    CSR_COMMON_EXAMPLE->link_type       = (sync_link_type)sink_type ;                                                        
    CSR_COMMON_EXAMPLE->volume          = volume;
    CSR_COMMON_EXAMPLE->audio_sink      = audio_sink;
    CSR_COMMON_EXAMPLE->mode            = mode;
	CSR_COMMON_EXAMPLE->stereo          = stereo;
    CSR_COMMON_EXAMPLE->tone_volume     = volume;
    CSR_COMMON_EXAMPLE->codec_mode = (uint16)params;
	
	 PRINT(("CSR_COMMON_EXAMPLE: connect [%x] [%x]\n", CSR_COMMON_EXAMPLE->running , (int)CSR_COMMON_EXAMPLE->audio_sink));

    /* Clear all routing to the PCM subsysytem*/
    PcmClearAllRouting();

	/* Calculate the DAC rate based on the over-the-air rate value passed in from VM */
	CSR_COMMON_EXAMPLE->dac_rate = rate ;
   
    /*ensure that the messages received are from the correct kap file*/ 
    (void) MessageCancelAll( (TaskData*) task, MESSAGE_FROM_KALIMBA);
    MessageKalimbaTask( (TaskData*) task );
   
	   /* Select which Kap file to be loaded based on the plugin selected */
    index = CsrSelectKapFile(); 
    
    /* Load the cvc algorithm into Kalimba*/
    if( !KalimbaLoad( index ) )
    {
        PRINT(("Kalimba load fail\n"));
        Panic();
    }
    
    ConnectAudio (task, CSR_COMMON_EXAMPLE->stereo) ;
    AUDIO_BUSY = NULL;
 }

/****************************************************************************
NAME	
	CsrCvcPluginDisconnect

DESCRIPTION
	Disconnect CSR_COMMON_EXAMPLE and power off the Kalimba DSP core
    
RETURNS
	void
*/
void CsrExamplePluginDisconnect( ExamplePluginTaskdata *task )
{
    if (!CSR_COMMON_EXAMPLE)
        Panic() ;
    if ( CSR_COMMON_EXAMPLE->running == FALSE )
        Panic() ;
 
    CodecSetOutputGainNow( CSR_COMMON_EXAMPLE->codec_task, CODEC_MUTE, left_and_right_ch );    
    
    PRINT(("CSR_COMMON_EXAMPLE: discon_mic\n"));
    StreamDisconnect(StreamPcmSource(0), 0); 
    StreamDisconnect(StreamKalimbaSource(1), 0); 
        
    PRINT(("CSR_COMMON_EXAMPLE: discon_spkr\n"));
    StreamDisconnect(0, StreamPcmSink(0));  
    StreamDisconnect(0, StreamKalimbaSink(1) );

    CSR_COMMON_EXAMPLE->running = FALSE;	
    CSR_COMMON_EXAMPLE->audio_sink = NULL;
    CSR_COMMON_EXAMPLE->link_type = 0;
    
    PRINT(("CSR_COMMON_EXAMPLE: Disconnect\n"));
 
    /* Cancel any outstanding cvc messages */
    MessageCancelAll( (TaskData*)task , MESSAGE_FROM_KALIMBA);
    MessageCancelAll( (TaskData*)task , MESSAGE_STREAM_DISCONNECT);
				    
    free (CSR_COMMON_EXAMPLE);
    CSR_COMMON_EXAMPLE = NULL;                
    
    KalimbaPowerOff();        
}

/****************************************************************************
NAME	
	CsrCvcPluginSetVolume

DESCRIPTION
	Tell CSR_COMMON_EXAMPLE to update the volume.

RETURNS
	void
*/
void CsrExamplePluginSetVolume( ExamplePluginTaskdata *task, uint16 volume )
{
    if (!CSR_COMMON_EXAMPLE)
        Panic() ;
 
	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
	if (volume > 0xf)
		volume = 0xf;
    
	CSR_COMMON_EXAMPLE->volume = volume;
          
    PRINT(("CSR_COMMON_EXAMPLE: DAC GAIN SET[%x]\n", CSR_COMMON_EXAMPLE->volume ));
    
    /* Only update the volume if not in a mute mode */
    if ( CSR_COMMON_EXAMPLE->running && !( (CSR_COMMON_EXAMPLE->mode==AUDIO_MODE_MUTE_SPEAKER ) || (CSR_COMMON_EXAMPLE->mode==AUDIO_MODE_MUTE_BOTH ) ) )
    { 
       CodecMessage (task, 0x800a,0x800a, CSR_COMMON_EXAMPLE->volume  );
    }         
}

/****************************************************************************
NAME	
	CsrCvcPluginSetMode

DESCRIPTION
	Set the CSR_COMMON_EXAMPLE mode

RETURNS
	void
*/
void CsrExamplePluginSetMode ( ExamplePluginTaskdata *task, AUDIO_MODE_T mode , const void * params )
{
    
    if (!CSR_COMMON_EXAMPLE)
        Panic();            
    if ( CSR_COMMON_EXAMPLE->running == FALSE )
        Panic();
    
    CSR_COMMON_EXAMPLE->mode = mode;
    
    switch (mode)
    {
	    case AUDIO_MODE_MUTE_SPEAKER:
	    {     
           CodecMessage (task, 0x800a,0x800a, CODEC_MUTE);
		 }
	    break ;
	    case AUDIO_MODE_CONNECTED:
	    {
           KalimbaSendMessage(MESSAGE_SETMODE , SYSMODE_PSTHRGH , 0, 0, 0 );
           CodecMessage (task, 0x800a,0x800a, CSR_COMMON_EXAMPLE->volume  );
           PRINT(("CSR_COMMON_EXAMPLE: Set Mode connected \n"));
	    }
	    break ;
	    case AUDIO_MODE_MUTE_MIC:
	    {
		   CodecMessage (task, CODEC_MUTE,CODEC_MUTE, CSR_COMMON_EXAMPLE->volume  );
	       PRINT(("CSR_COMMON_EXAMPLE: Set Mode Mute Mic \n"));
	    }
	    break ;      
	    case AUDIO_MODE_MUTE_BOTH:
	    {
		   CodecMessage (task, CODEC_MUTE,CODEC_MUTE, CODEC_MUTE );
	       PRINT(("CSR_COMMON_EXAMPLE: Set Mode SYSMODE_HFK MUTE BOTH - Standby\n"));
	    }
	    break;    
	    default:
	    {    /*do not send a message and return false*/    
	       PRINT(("CSR_COMMON_EXAMPLE: Set Mode Invalid [%x]\n" , mode ));
		 }
		break ;
    }
   
}

/****************************************************************************
NAME	
	CsrCvcPluginPlayTone

DESCRIPTION

    queues the tone if can_queue is TRUE and there is already a tone playing

RETURNS
	false if a tone is already playing
    
*/
void CsrExamplePluginPlayTone (ExamplePluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo)  
{   

    Source lSource ;  
    Sink lSink ; 
        
    if (!CSR_COMMON_EXAMPLE)
        Panic() ;
    if ( CSR_COMMON_EXAMPLE->running == FALSE )
        Panic() ;       

    PRINT(("CSR_COMMON_EXAMPLE: Tone Start\n"));

	/* The DAC gain must be limited to 0 dB so that no distortion occurs and so the echo canceller works. */
	if (tone_volume > 0xf)
		tone_volume = 0xf;

    /* set DAC gain to a suitable level for tone play */
    if (tone_volume != CSR_COMMON_EXAMPLE->tone_volume)
    {
      CSR_COMMON_EXAMPLE->tone_volume = tone_volume;
      CodecMessage (task, 0x800a,0x800a, CSR_COMMON_EXAMPLE->tone_volume  ); 
    }

    lSink = StreamKalimbaSink(3);
    
    /*request an indication that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink , (TaskData*) task );

    /*connect the tone*/
    lSource = StreamAudioSource ( (const audio_note *) (tone) );    
 
	/*mix the tone to the CSR_COMMON_EXAMPLE*/    
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
void CsrExamplePluginStopTone ( void ) 
{
    if (!CSR_COMMON_EXAMPLE)
        Panic() ;
        
    StreamDisconnect( 0 , StreamKalimbaSink(3) ) ; 
}


/****************************************************************************
DESCRIPTION
	Connect the audio stream (Speaker and Microphone)
*/
static void ConnectAudio (ExamplePluginTaskdata *task, bool stereo )
{             
    bool r1, r2;
    if ( CSR_COMMON_EXAMPLE->audio_sink )
    {	            
        /* DSP is up and running */
	     CSR_COMMON_EXAMPLE->running = TRUE ;
        
	   
        /* Set DAC gain to minimum value before connecting streams */
        CodecSetOutputGainNow( CSR_COMMON_EXAMPLE->codec_task, 0 , left_and_right_ch );        

        /* Configure port 0 to be routed to internal ADC and DACs */
	    if (stereo)         
        	PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) CSR_COMMON_EXAMPLE->dac_rate, (uint32) CSR_COMMON_EXAMPLE->dac_rate, VM_PCM_INTERNAL_A_AND_B);
        else
        	PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) CSR_COMMON_EXAMPLE->dac_rate, (uint32) CSR_COMMON_EXAMPLE->dac_rate, VM_PCM_INTERNAL_A);


		
		StreamDisconnect(  StreamPcmSource(0) , StreamPcmSink(0) ) ; 
		StreamDisconnect(  StreamKalimbaSource(0) , StreamKalimbaSink(0) ) ; 
		StreamDisconnect(  StreamKalimbaSource(1) , StreamKalimbaSink(1) ) ; 
        /* Connect Ports to DSP */
#if 0
        r1 = StreamConnect( StreamPcmSource(0),StreamKalimbaSink(0));  /* ADC_LEFT->DSP */
		r2 = StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0));   /* DSP->DAC */
#else
        r1 = r2 = StreamConnect( StreamKalimbaSource(0),StreamKalimbaSink(0));  /* loopback */
#endif
		
        PRINT(("CSR_COMMON_EXAMPLE: connect_mic_spkr %d %d \n",r1,r2)); 

        r1 = StreamConnect(StreamSourceFromSink( CSR_COMMON_EXAMPLE->audio_sink ),StreamKalimbaSink(1)); /* SCO->DSP */  
        r2 = StreamConnect( StreamKalimbaSource(1), CSR_COMMON_EXAMPLE->audio_sink ); /* DSP->SCO */
        PRINT(("CSR_COMMON_EXAMPLE: connect_sco %d %d \n",r1,r2));  

        if (!KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0))
        {   /* Failed to send message to DSP, abort */
            Panic();
        }
    }
    else
    {   
        /*Power Down*/
        CsrExamplePluginDisconnect(task);
    }
}                

/****************************************************************************
DESCRIPTION
	Handles a CVC_CODEC message received from CSR_COMMON_EXAMPLE
*/
static void CodecMessage (ExamplePluginTaskdata *task, uint16 input_gain_l, uint16 input_gain_r, uint16 output_gain )
{   
    PRINT(("CSR_COMMON_EXAMPLE: Output gain = 0x%x\n" , output_gain ));
    PRINT(("CSR_COMMON_EXAMPLE: Input gain  = 0x%x ,0x%x\n" , input_gain_l,input_gain_r ));
    
    /* check pointer validity as there is a very small window where a message arrives
       as the result of playing a tone after CSR_COMMON_EXAMPLE has been powered down */
    if(CSR_COMMON_EXAMPLE)
    {
        /*Set the output Gain immediately*/
        CodecSetOutputGainNow( CSR_COMMON_EXAMPLE->codec_task, output_gain, left_and_right_ch);
    
        /*only enable the pre amp if asked to do so*/
        /*ie if the top bit (0x8000) is set */
        CodecEnableMicInputGainA( ( input_gain_l >> 15 ) & 0x1 );
	    
    
        /* Clear the upper bytes of the input gain argument */
		CodecEnableMicInputGainB( ( input_gain_l >> 15 ) & 0x1 );
		CodecSetInputGainNow( CSR_COMMON_EXAMPLE->codec_task, (input_gain_l & 0xFF), left_and_right_ch);
    }
}


/****************************************************************************
DESCRIPTION
	handles the internal cvc messages /  messages from the dsp
*/
void CsrExamplePluginInternalMessage( ExamplePluginTaskdata *task ,uint16 id , Message message ) 
{
	switch(id)
	{
      case MESSAGE_FROM_KALIMBA:
		{
		   const DSP_REGISTER_T *m = (const DSP_REGISTER_T *) message;
	      PRINT(("CSR_CVSD_8K_1MIC: msg id[%x] a[%x] b[%x] c[%x] d[%x]\n", m->id, m->a, m->b, m->c, m->d));
		
         switch ( m->id ) 
			{
      		/* Case statements for messages from the kalimba can be added here.
               The example application as shipped does not send any messages
               to the DSP. */
            default:
            break;
         }
		}
		
      break;
	
      /* Message is not from DSP.  The example plugin as shipped does not
         send any messages to this handler. */
      default:
      break;
	}		
}	

/****************************************************************************
DESCRIPTION
	a tone has completed
*/
void CsrExamplePluginToneComplete( ExamplePluginTaskdata *task) 
{
   
   /* Restore the DAC gain to mute if in mute mode */
   if ( CSR_COMMON_EXAMPLE->running && (CSR_COMMON_EXAMPLE->mode==AUDIO_MODE_MUTE_SPEAKER || CSR_COMMON_EXAMPLE->mode==AUDIO_MODE_MUTE_BOTH ) )
      CodecMessage (task, 0x800a,0x800a,CODEC_MUTE );   
   else
      CodecMessage (task, 0x800a,0x800a, CSR_COMMON_EXAMPLE->volume  );
   
   /* We no longer want to receive stream indications */
   MessageSinkTask (StreamKalimbaSink(3) , NULL);   
}
