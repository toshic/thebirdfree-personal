// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc %%copyright(2003)        http://www.csr.com
// %%version
//
// $Revision$  $Date$
// *****************************************************************************

// *****************************************************************************
// DESCRIPTION
//    Decoder (SBC/MP3/AAC) for an audio playing device (non USB)
//
// *****************************************************************************

   .define SELECTED_CODEC_AAC

   // Comment this line and ".include SELECTED_CODEC_LIBRARY_HEADER" if use library build   
   //.define SELECTED_CODEC_LIBRARY_HEADER               "aac_library.h"


   .define SELECTED_CODEC_FRAME_DECODE_FUNCTION        &$aacdec.frame_decode_latm
   .define SELECTED_CODEC_RESET_DECODER_FUNCTION       &$aacdec.reset_decoder
   .define SELECTED_CODEC_SILENCE_DECODER_FUNCTION     &$aacdec.silence_decoder
   .define SELECTED_CODEC_INITIALISE_DECODER_FUNCTION  &$aacdec.init_decoder


   .define AUDIO_CBUFFER_SIZE                      1600
   .define CODEC_CBUFFER_SIZE                      1290
   .define COMFORT_NOISE_GAIN                      0
   .define GOOD_WORKING_BUFFER_LEVEL               0.65
   .define POORLINK_DETECT_LEVEL                   0.25



// 1.5ms is chosen as the interrupt rate for the audio input/output because:
// adc/dac mmu buffer is 256byte = 128samples
//                               - upto 8 sample fifo in voice interface
//                               = 120samples = 2.5ms @ 48KHz
// assume absolute worst case jitter on interrupts = 1.0ms
// Hence choose 1.5ms between audio input/output interrupts
.define TMR_PERIOD_AUDIO_COPY          1300


// This 20ms timer period is decided from a realtime playback buffer trace.
// In general, it takes 2 to 3 attempts to find new spaces in the buffer. 
.define TMR_PERIOD_CODEC_COPY          20000


// The timer period for copying tones.  We don't want to force the VM to fill
// up the tone buffer too regularly.
.define TMR_PERIOD_TONE_COPY           8000

.CONST $TONE_BUFFER_SIZE               128; //128 samples of tone each TMR_PERIOD_TONE_COPY period, plus twice of interpolation filter length


// includes
.include "core_library.h"
.include "cbops_library.h"
.include "codec_library.h"
//.include SELECTED_CODEC_LIBRARY_HEADER


.MODULE $M.main;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $main:

   // ** setup ports that are to be used **
   .CONST  $AUDIO_LEFT_OUT_PORT    ($cbuffer.WRITE_PORT_MASK + 0);
   .CONST  $AUDIO_RIGHT_OUT_PORT   ($cbuffer.WRITE_PORT_MASK + 1);
   .CONST  $CODEC_IN_PORT          ($cbuffer.READ_PORT_MASK  + 0); //$cbuffer.FORCE_16BIT_WORD);
   .CONST  $TONE_IN_PORT           (($cbuffer.READ_PORT_MASK | $cbuffer.FORCE_PCM_AUDIO)  + 3);

   .VAR $current_dac_sampling_frequency = 44100;
   // ** allocate memory for cbuffers **
   .VAR/DM1CIRC $audio_out_left[AUDIO_CBUFFER_SIZE];
   .VAR/DM2CIRC $audio_out_right[AUDIO_CBUFFER_SIZE];   
   .VAR/DM1CIRC $codec_in[CODEC_CBUFFER_SIZE];
   .VAR/DM1CIRC $tone_in_left[$TONE_BUFFER_SIZE];
   .VAR/DM1CIRC $tone_in_right[$TONE_BUFFER_SIZE];

   // ** allocate memory for cbuffer structures **
   .VAR $codec_in_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH($codec_in),              // size
          &$codec_in,                     // read pointer
          &$codec_in;                     // write pointer
   .VAR $audio_out_left_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH($audio_out_left),        // size
          &$audio_out_left,               // read pointer
          &$audio_out_left;               // write pointer
   .VAR $audio_out_right_cbuffer_struc[$cbuffer.STRUC_SIZE] =
          LENGTH($audio_out_right),       // size
          &$audio_out_right,              // read pointer
          &$audio_out_right;              // write pointer

  .VAR $tone_in_left_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($tone_in_left),              // size
         &$tone_in_left,                     // read pointer
         &$tone_in_left;                     // write pointer
   .VAR $tone_in_right_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($tone_in_right),              // size
         &$tone_in_right,                     // read pointer
         &$tone_in_right;                     // write pointer


   // ** allocate memory for timer structures **
   .VAR $codec_in_timer_struc[$timer.STRUC_SIZE];
   .VAR $audio_out_timer_struc[$timer.STRUC_SIZE];
   .VAR $tone_copy_timer_struc[$timer.STRUC_SIZE];
   
   .VAR $volume_change_message_struc[$message.STRUC_SIZE];
   

   // ** allocate memory for codec input cbops copy routine **
   .VAR $codec_in_copy_struc[] =
      &$codec_in_copy_op,             // first operator block
      1,                              // number of inputs
      $CODEC_IN_PORT,                 // input
      1,                              // number of outputs
      &$codec_in_cbuffer_struc;       // output

   .BLOCK $codec_in_copy_op;
      .VAR $codec_in_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $codec_in_copy_op.func = &$cbops.copy_op;
      .VAR $codec_in_copy_op.param[$cbops.copy_op.STRUC_SIZE] =
         0,                         // Input index
         1;                         // Output index
   .ENDBLOCK;



  // ** allocate memory for tone input cbops copy routine **
   .VAR $tone_in_copy_struc[] =
      &$tone_write_limit_op, /*&$tone_in_copy_op,*/              // first operator block
      1,                               // number of inputs
      $TONE_IN_PORT,                   // input
      2,                               // number of outputs
      &$tone_in_left_cbuffer_struc,
     &$tone_in_right_cbuffer_struc;         // output

   .BLOCK $tone_write_limit_op;
      .VAR $tone_write_limit_op.next = &$tone_in_left_copy_op;
      .VAR $tone_write_limit_op.func = &$cbops.limited_copy;
      .VAR $tone_write_limit_op.mono[$cbops.limited_copy.STRUC_SIZE] =
         $cbops.limited_copy.NO_READ_LIMIT,
         50;
   .ENDBLOCK;

   .BLOCK $tone_in_left_copy_op;
      .VAR $tone_in_left_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $tone_in_left_copy_op.func = &$cbops.shift;
      .VAR $tone_in_left_copy_op.param[$cbops.shift.STRUC_SIZE] =
         0,                         // Input index
         1,                         // Output index
         3+5;                       // 3 bits amplification for tone(was in original file)
                           // and 5 bits to convert from 16-bit to 21-bit                     
   .ENDBLOCK;
   
   

   // ** allocate memory for stereo audio out cbops copy routine **
   .VAR $left_out_copy_struc[] =
      &$audio_out_tone_upsample_mono_left_mix,
      1,                                  // number of inputs
      &$audio_out_left_cbuffer_struc,     // input
      1,                                  // number of outputs
      $AUDIO_LEFT_OUT_PORT;               // output


   //tone mixing can not be the last operator as it does upsamling and mixing in place
   .BLOCK $audio_out_tone_upsample_mono_left_mix;
      .VAR $audio_out_tone_upsample_mono_left_mix.next = &$left_out_copy_op;
      .VAR $audio_out_tone_upsample_mono_left_mix.func = &$cbops.auto_upsample_and_mix;
      .VAR $audio_out_tone_upsample_mono_left_mix.param[$cbops.auto_resample_mix.STRUC_SIZE] =
         0,                               // Input index to left channel
         -1,                              // Index to right channel (-1: no right cchannel)
       &$tone_in_left_cbuffer_struc,         // cbuffer structure containing tone samples
       &$sra_coeffs,             // 
         &$current_dac_sampling_frequency,// pointer to variable containing dac frequency received from vm ( zero pointer or zero value = not received any)
         0,        // api function to decoder sampling frequency 
         8000,                            // samling frequency of the tone (at the moment only 8000 is supported, otherwise a run time error happens)
         0.5,                             // tone volume mixing
         0.5;                             // audio volume mixing
   .ENDBLOCK;
   

   // DC remove and compressor are not needed for aac output
   .BLOCK $left_out_copy_op;
      .VAR $left_out_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $left_out_copy_op.func = &$cbops.shift;
      .VAR $left_out_copy_op.param[$cbops.shift.STRUC_SIZE] =
         0,                         // Input index
         1,                         // Output index
         -5;
   .ENDBLOCK;

   .VAR $right_out_copy_struc[] =
      &$audio_out_tone_upsample_mono_right_mix,
      1,                                  // number of inputs
      &$audio_out_right_cbuffer_struc,    // input
      1,                                  // number of outputs
      $AUDIO_RIGHT_OUT_PORT;              // output


   .BLOCK $audio_out_tone_upsample_mono_right_mix;
      .VAR $audio_out_tone_upsample_mono_right_mix.next = &$right_out_copy_op;
      .VAR $audio_out_tone_upsample_mono_right_mix.func = &$cbops.auto_upsample_and_mix;
      .VAR $audio_out_tone_upsample_mono_right_mix.param[$cbops.auto_resample_mix.STRUC_SIZE] =
         0,                               // Input index to left channel
         -1,                              // Index to right channel (-1: no right cchannel)
       &$tone_in_right_cbuffer_struc,         // cbuffer structure containing tone samples
       &$sra_coeffs,             // 
         &$current_dac_sampling_frequency,// pointer to variable containing dac frequency received from vm ( zero pointer or zero value = not received any)
         0,        // api function to decoder sampling frequency 
         8000,                            // samling frequency of the tone (at the moment only 8000 is supported, otherwise a run time error happens)
         0.5,                             // tone volume mixing
         0.5;                             // audio volume mixing
   .ENDBLOCK;


   // DC remove and Compressor are not needed for aac output
   .BLOCK $right_out_copy_op;
      .VAR $right_out_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $right_out_copy_op.func = &$cbops.shift;
      .VAR $right_out_copy_op.param[$cbops.shift.STRUC_SIZE] =
         0,                         // Input index
         1,                         // Output index
         -5;
   .ENDBLOCK;


   // ** allocate memory for codec stream structure **
   .VAR/DM1 $decoder_codec_stream_struc[$codec.av_decode.STRUC_SIZE] =
            SELECTED_CODEC_FRAME_DECODE_FUNCTION,    // frame_decode function
            SELECTED_CODEC_RESET_DECODER_FUNCTION,   // reset_decoder function
            SELECTED_CODEC_SILENCE_DECODER_FUNCTION, // silence_decoder function
            &$codec_in_cbuffer_struc,                // in cbuffer
            &$audio_out_left_cbuffer_struc,          // out left cbuffer
            &$audio_out_right_cbuffer_struc,         // out right cbuffer
            0,
            0,
            0,
            GOOD_WORKING_BUFFER_LEVEL,
            POORLINK_DETECT_LEVEL,
            0,                                  // Disable codec in buffer purge when in pause
            0;


   // initialise the stack library
   call $stack.initialise;
   // initialise the interrupt library
   call $interrupt.initialise;
   // initialise the message library
   call $message.initialise;
   // initialise the cbuffer library
   call $cbuffer.initialise;
   // initialise the pskey library
   call $pskey.initialise;
   .ifdef DEBUG_ON
      // initialise the profiler library
      call $profiler.initialise;
   .endif


   // initialise Flash
   call $flash.init_pm;


   // This handler must be set before read and wait for go message
   // in order to set the initial volume correctly. 
   r1 = &$volume_change_message_struc;
   r2 = 0x1002; // 0x1002 == $music_example.VMMSG.VOLUME
   r3 = &$volume_change_handler;
   call $message.register_handler;
   

   // initialise the codec decoder library
   call SELECTED_CODEC_INITIALISE_DECODER_FUNCTION;

   r2 = 0x1000; // 0x1000 == $music_example.VMMSG.READY;
   r3 = 0xE000; // 0xE000 == $MUSIC_EXAMPLE_SYSID;
   r4 = 0;      // status
   call $message.send_short; // not-used message to comply the new Music Mgr interface


   // tell vm we're ready and wait for the go message
   call $message.send_ready_wait_for_go;
   

   // see if left output port is connected
   r0 = $AUDIO_LEFT_OUT_PORT;
   call $cbuffer.is_it_enabled;
   if NZ jump left_port_connected;
      // tell codec library that no left buffer
      M[$decoder_codec_stream_struc + $codec.stream_decode.OUT_LEFT_BUFFER_FIELD] = 0;
   left_port_connected:


   // see if right output port is connected
   r0 = $AUDIO_RIGHT_OUT_PORT;
   call $cbuffer.is_it_enabled;
   if NZ jump right_port_connected;
      // tell codec library that no right buffer
      M[$decoder_codec_stream_struc + $codec.stream_decode.OUT_RIGHT_BUFFER_FIELD] = 0;
   right_port_connected:


   // wait for DAC buffers to have just wrapped around
   wait_for_dac_buffer_wraparound:
      r0 = $AUDIO_LEFT_OUT_PORT;
      call $cbuffer.calc_amount_space;
      // if the amount of space in the buffer is less than 16 bytes then a
      // buffer wrap around must have just ocurred.
      Null = r0 - 16;
   if POS jump wait_for_dac_buffer_wraparound;


   // start timer that copies output samples
   r1 = &$audio_out_timer_struc;
   r2 = TMR_PERIOD_AUDIO_COPY;
   r3 = &$audio_out_copy_handler;
   call $timer.schedule_event_in;

   // start timer that copies codec input data
   r1 = &$codec_in_timer_struc;
   r2 = TMR_PERIOD_CODEC_COPY;
   r3 = &$codec_in_copy_handler;
   call $timer.schedule_event_in;


  // start timer that copies tone samples
   r1 = &$tone_copy_timer_struc;
   r2 = TMR_PERIOD_TONE_COPY;
   r3 = &$tone_copy_handler;
   call $timer.schedule_event_in;


   // continually decode codec frames
   frame_loop:

      // decode a frame
      r5 = &$decoder_codec_stream_struc;
      call $codec.av_decode;
      
      // idle as much as we can
      //r5 = &$decoder_codec_stream_struc;
      //r0 = M[r5 + $codec.av_decode.MODE_FIELD ];
      //Null = r0 - $codec.SUCCESS;
      //if NZ call $timer.1ms_delay;
      r0 = 3;
      call $timer.n_ms_delay;

   jump frame_loop;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $audio_out_copy_handler
//
// DESCRIPTION:
//    Function called on a timer interrupt to perform the mono or stereo copying
//    of decoded samples to the output.
//
// *****************************************************************************
.MODULE $M.audio_out_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   swap_some_internal_data:
   
    //saving  tone_mixing inter internal data as here it is called twice
   .VAR  sample_count   = 0;
   .VAR  hist_samples_no = 33;
   .VAR  upsample_state;
   .VAR  normal_counter;
   
    //saving  dac_av_copy inter internal data as here it is called twice
   .VAR port_mnt_data_avg;                 
   .VAR prev_port_mnt_data;               
   .VAR prev_buffer_write_addr;            
   .VAR buffer_write_addr_nochange_counter; 

   
    r0 = M[$M.cbops.auto_upsample_and_mix.main.sample_count];
    r1 = M[sample_count];  
   M[sample_count] = r0;
   M[$M.cbops.auto_upsample_and_mix.main.sample_count] = r1;
   
    r0 = M[$M.cbops.auto_upsample_and_mix.main.hist_samples_no];
    r1 = M[hist_samples_no];  
   M[hist_samples_no] = r0;
   M[$M.cbops.auto_upsample_and_mix.main.hist_samples_no] = r1;
   
    r0 = M[$M.cbops.auto_upsample_and_mix.main.upsample_state];
    r1 = M[upsample_state];   
   M[upsample_state] = r0;
   M[$M.cbops.auto_upsample_and_mix.main.upsample_state] = r1;
   
    r0 = M[$M.cbops.auto_upsample_and_mix.main.normal_counter];
    r1 = M[normal_counter];   
   M[normal_counter] = r0;
   M[$M.cbops.auto_upsample_and_mix.main.normal_counter] = r1;
   
    r0 = M[$M.cbops.adc_av_copy.port_mnt_data_avg];
    r1 = M[port_mnt_data_avg];   
   M[port_mnt_data_avg] = r0;
   M[$M.cbops.adc_av_copy.port_mnt_data_avg] = r1;

    r0 = M[$M.cbops.adc_av_copy.prev_port_mnt_data];
    r1 = M[prev_port_mnt_data];  
   M[prev_port_mnt_data] = r0;
   M[$M.cbops.adc_av_copy.prev_port_mnt_data] = r1;
   
    r0 = M[$M.cbops.adc_av_copy.prev_buffer_write_addr];
    r1 = M[prev_buffer_write_addr]; 
   M[prev_buffer_write_addr] = r0;
   M[$M.cbops.adc_av_copy.prev_buffer_write_addr] = r1;
   
    r0 = M[$M.cbops.adc_av_copy.buffer_write_addr_nochange_counter];
    r1 = M[buffer_write_addr_nochange_counter]; 
   M[buffer_write_addr_nochange_counter] = r0;
   M[$M.cbops.adc_av_copy.buffer_write_addr_nochange_counter] = r1;
   

    rts;

   $audio_out_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   
   r8 = &$left_out_copy_struc;
   call $cbops.dac_av_copy;
   
  

   r0 = $AUDIO_RIGHT_OUT_PORT;
   call $cbuffer.is_it_enabled;
.ifdef FORCE_MONO
   r0 = 0;
.endif
   if Z jump mono_mode_output;

   call swap_some_internal_data; // context switch for SRA

   r8 = &$right_out_copy_struc;
   call $cbops.dac_av_copy;
   
   call swap_some_internal_data; // context switch for SRA

   jump skip_mono_mode_code;
   
mono_mode_output:
   // mono mode: purge right channel buffer
   r0 = &$audio_out_right_cbuffer_struc;
   call $cbuffer.get_write_address_and_size;
   r1 = r0;
   r0 = &$audio_out_right_cbuffer_struc;
   call $cbuffer.set_read_address;


skip_mono_mode_code:



   // post another timer event
   r1 = &$audio_out_timer_struc;
   r2 = TMR_PERIOD_AUDIO_COPY;
   r3 = &$audio_out_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $codec_in_copy_handler
//
// DESCRIPTION:
//    Function called on a timer interrupt to perform the copying of encoded
//    samples from the input.
//
// *****************************************************************************
.MODULE $M.codec_in_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   .VAR temp;

   $codec_in_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // copy data from the port to the cbuffer
   r8 = &$codec_in_copy_struc;
   call $cbops.copy;


   // post another timer event
   r1 = &$codec_in_timer_struc;
   r2 = TMR_PERIOD_CODEC_COPY;
   r3 = &$codec_in_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $tone_copy_handler
//
// DESCRIPTION:
//    Function called on a timer interrupt to perform the copying of tone
//    samples.
//
// *****************************************************************************

.MODULE $M.tone_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $tone_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // copy tone data from the port
   
   //read the current mnt_data
   r0 = &$tone_in_left_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   push r0;

   r8 = &$tone_in_copy_struc;
   call $cbops.copy;

    //read the new mnt_data
   r0 = &$tone_in_left_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   pop r1;
   r10 = r0 - r1;
   //r10 number of new tone samples
   if LE jump no_tone_data;
   
   //get the write pointer for left channel
   r0 = &$tone_in_left_cbuffer_struc;
   call $cbuffer.get_write_address_and_size;
   I0 = r0;
   L0 = r1;
   M0 = -r10;
   r0 = M[I0, M0];
   //get back to the first new sample
   r0 = &$tone_in_right_cbuffer_struc;
   call $cbuffer.get_write_address_and_size;
   L1 = r1;
   I1 = r0;
   r0 = M[I1, M0];
   //write new samples to the right buffer
   do copy_tone_to_right_buffer;
      r0 = M[I0, 1];
     M[I1, 1] = r0;
   copy_tone_to_right_buffer:
   L0 = 0;
   L1 = 0;
  

no_tone_data:
   // post another timer event
   r1 = &$tone_copy_timer_struc;
   r2 = TMR_PERIOD_TONE_COPY;
   r3 = &$tone_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;


// *****************************************************************************
// DESCRIPTION: This is adopted from the Music Mgr enabled SBC/MP3 app
//              in order to comply the VM interface ($MsgMusicExampleSetVolume)
//  r1 = N/A
//  r2 = N/A      
//  r3 = DAC L gain
//  r4 = DAC R gain
// *****************************************************************************
.MODULE $M.volume_change_handler;

   .CODESEGMENT   PM;
   
   $volume_change_handler:
   
   // push rLink onto stack
   $push_rLink_macro;

   // Limit Gains
   r3 = r3 AND 0xF;   
   r4 = r4 AND 0xF;   
   
   r2 = 0x1006; // 0x1006 == $music_example.VMMSG.CODEC;
   call $message.send_short;

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

