// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc 2003-2008        http://www.csr.com
// Part of Stereo-Headset-SDK Q2-2008
//
// $Revision$  $Date$
// *****************************************************************************

// *****************************************************************************
// DESCRIPTION
//    Decoder (SBC/MP3) for an audio playing device (non USB)
//
// *****************************************************************************
.include "frame_sync_library.h"
.include "music_example.h"
.include "spi_comm_library.h"
.include "sr_adjustment.h"

.ifndef SELECTED_CODEC_SBC
   .ifndef SELECTED_CODEC_MP3
      .define SELECTED_CODEC_SBC
   .endif
.endif

// select the codec specific options
.ifdef SELECTED_CODEC_SBC
   .define SELECTED_CODEC_FRAME_DECODE_FUNCTION        &$sbcdec.frame_decode
   .define SELECTED_CODEC_RESET_DECODER_FUNCTION       &$sbcdec.reset_decoder
   .define SELECTED_CODEC_SILENCE_DECODER_FUNCTION     &$sbcdec.silence_decoder
   .define SELECTED_CODEC_INITIALISE_DECODER_FUNCTION  $sbcdec.init_decoder
   .define SELECTED_CODEC_LIBRARY_HEADER               "sbc_library.h"
   .define SELECTED_CODEC_GETFS_API                    &$sbcdec.get_sampling_frequency  //to save code/data this can be set to 0 if vm application sends the dac rate
.endif
.ifdef SELECTED_CODEC_MP3
   .define SELECTED_CODEC_FRAME_DECODE_FUNCTION        &$mp3dec.frame_decode
   .define SELECTED_CODEC_RESET_DECODER_FUNCTION       &$mp3dec.reset_decoder
   .define SELECTED_CODEC_SILENCE_DECODER_FUNCTION     &$mp3dec.silence_decoder
   .define SELECTED_CODEC_INITIALISE_DECODER_FUNCTION  $mp3dec.init_decoder
   .define SELECTED_CODEC_LIBRARY_HEADER               "mp3_library.h"
   .define SELECTED_CODEC_GETFS_API                    &$mp3.get_sampling_frequency     //to save code/data this can be set to 0 if vm application sends the dac rate
.endif


// 1.5ms is chosen as the interrupt rate for the audio input/output because:
// adc/dac mmu buffer is 256byte = 128samples
//                               - upto 8 sample fifo in voice interface
//                               = 120samples = 2.5ms @ 48KHz
// assume absolute worst case jitter on interrupts = 1.0ms
// Hence choose 1.5ms between audio input/output interrupts
.define TMR_PERIOD_AUDIO_COPY          1500

// 8ms is chosen as the interrupt rate for the codec input/output as this is a
// good compromise between not overloading the xap with messages and making
// sure that the xap side buffer is emptied relatively often.
.define TMR_PERIOD_CODEC_COPY          8000

// The timer period for copying tones.  We don't want to force the VM to fill
// up the tone buffer too regularly.
.define TMR_PERIOD_TONE_COPY           16000

.CONST $TONE_BUFFER_SIZE               128+66; //128 samples of tone each TMR_PERIOD_TONE_COPY period, plus twice of interpolation filter length

// The frame_sync output cbuffer (dac_out) size does not have to equal the frame_sync input cbuffer (audio_out) size
// The minimum size = NUM_SAMPLES_PER_FRAME + 2*(number of samples per timer interrupt)

.define MAX_SAMPLE_RATE                48000
.define ABSOLUTE_MAXIMUM_CLOCK_MISMATCH_COMPENSATION 0.03 // this is absolute maximum value(3%), it is also capped to value received from vm
.define OUTPUT_AUDIO_CBUFFER_SIZE      $music_example.NUM_SAMPLES_PER_FRAME + 2*(TMR_PERIOD_AUDIO_COPY * MAX_SAMPLE_RATE/1000000)
.define SRA_AVERAGING_TIME 2 //in seconds

// A debug define to force the decoder to use a mono output
//.define FORCE_MONO                     1

// includes
.include "core_library.h"
.include "cbops_library.h"
.include "codec_library.h"
.include SELECTED_CODEC_LIBRARY_HEADER
.define VM_DAC_RATE_MESSAGE_ID 0x7050

.MODULE $M.main;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $main:

   // ** setup ports that are to be used **
   .CONST  $AUDIO_LEFT_OUT_PORT    ($cbuffer.WRITE_PORT_MASK + 0);
   .CONST  $AUDIO_RIGHT_OUT_PORT   ($cbuffer.WRITE_PORT_MASK + 1);
   .CONST  $CODEC_IN_PORT          ($cbuffer.READ_PORT_MASK  + 0);
   .CONST  $TONE_IN_PORT           (($cbuffer.READ_PORT_MASK | $cbuffer.FORCE_PCM_AUDIO)  + 3);


   // ** allocate memory for cbuffers **
   .VAR/DMCIRC $audio_out_left[AUDIO_CBUFFER_SIZE];
   .VAR/DMCIRC $audio_out_right[AUDIO_CBUFFER_SIZE];
   .VAR/DMCIRC $dac_out_left[OUTPUT_AUDIO_CBUFFER_SIZE];
   .VAR/DMCIRC $dac_out_right[OUTPUT_AUDIO_CBUFFER_SIZE];   
   .VAR/DMCIRC $codec_in[CODEC_CBUFFER_SIZE];
   .VAR/DM1CIRC $tone_in[$TONE_BUFFER_SIZE];
   
   // define memory location to receive dac sampling frequency from vm
   // zero(address or value) means not received
   .VAR $current_dac_sampling_frequency = 0;
   .VAR $get_dac_rate_from_vm_message_struc[$message.STRUC_SIZE];
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
   .VAR $dac_out_left_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($dac_out_left),          // size
         &$dac_out_left,                 // read pointer
         &$dac_out_left;                 // write pointer
   .VAR $dac_out_right_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($dac_out_right),         // size
         &$dac_out_right,                // read pointer
         &$dac_out_right;                // write pointer         
   .VAR $tone_in_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($tone_in),               // size
         &$tone_in,                      // read pointer
         &$tone_in;                      // write pointer
  
   // ** allocate memory for timer structures **
   .VAR $codec_in_timer_struc[$timer.STRUC_SIZE];
   .VAR $audio_out_timer_struc[$timer.STRUC_SIZE];
   .VAR $tone_copy_timer_struc[$timer.STRUC_SIZE];


   // ** allocate memory for codec input cbops copy routine **
   .VAR $codec_in_copy_struc[] =
      &$codec_in_copy_op,              // first operator block
      1,                               // number of inputs
      $CODEC_IN_PORT,                  // input
      1,                               // number of outputs
      &$codec_in_cbuffer_struc;        // output

   .BLOCK $codec_in_copy_op;
      .VAR $codec_in_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $codec_in_copy_op.func = &$cbops.copy_op;
      .VAR $codec_in_copy_op.param[$cbops.copy_op.STRUC_SIZE] =
         0,                            // Input index
         1;                            // Output index
   .ENDBLOCK;

   // ** allocate memory for tone input cbops copy routine **
   .VAR $tone_in_copy_struc[] =
      &$tone_write_limit_op,           // first operator block
      1,                               // number of inputs
      $TONE_IN_PORT,                   // input
      1,                               // number of outputs
      &$tone_in_cbuffer_struc;         // output

   .BLOCK $tone_write_limit_op;
      .VAR $tone_write_limit_op.next = &$tone_in_copy_op;
      .VAR $tone_write_limit_op.func = &$cbops.limited_copy;
      .VAR $tone_write_limit_op.mono[$cbops.limited_copy.STRUC_SIZE] =
         $cbops.limited_copy.NO_READ_LIMIT,
         50;
   .ENDBLOCK;

   .BLOCK $tone_in_copy_op;
      .VAR $tone_in_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $tone_in_copy_op.func = &$cbops.shift;
      .VAR $tone_in_copy_op.param[$cbops.shift.STRUC_SIZE] =
         0,                         // Input index
         1,                         // Output index
         3+8;                       // 3 bits amplification for tone(was in original file)
                           // and 8 bits to convert from 16-bit to 24-bit                     
   .ENDBLOCK;



   // ** allocate memory for stereo audio out cbops copy routine **
  .VAR $stereo_out_copy_struc[] =
      &$audio_out_tone_upsample_stereo_mix, // first operator block
      2,                                    // number of inputs
      &$dac_out_left_cbuffer_struc,         // input
      &$dac_out_right_cbuffer_struc,        // input
      2,                                    // number of outputs
      $AUDIO_LEFT_OUT_PORT,                 // output
      $AUDIO_RIGHT_OUT_PORT;                // output

  //tone mixing can not be the last operator as it does upsamling and mixing in place
 .BLOCK $audio_out_tone_upsample_stereo_mix;
      .VAR $audio_out_tone_upsample_stereo_mix.next = &$audio_out_dc_remove_op_left;
      .VAR $audio_out_tone_upsample_stereo_mix.func = &$cbops.auto_upsample_and_mix;
      .VAR $audio_out_tone_upsample_stereo_mix.param[$cbops.auto_resample_mix.STRUC_SIZE] =
         0,                                 // Input index to left channel
         1,                                 // Index to right channel
       &$tone_in_cbuffer_struc,           // cbuffer structure containing tone samples
       &$sra_coeffs,               // 
         &$current_dac_sampling_frequency,  // pointer to variable containing dac frequency received from vm ( zero pointer or zero value = not received any)
         SELECTED_CODEC_GETFS_API,          // api function to decoder sampling frequency 
         8000,                            // samling frequency of the tone (at the moment only 8000 is supported, otherwise a run time error happens)
         0.5,                             // tone volume mixing
         0.5;                             // audio volume mixing
   .ENDBLOCK;


   .BLOCK $audio_out_dc_remove_op_left;
      .VAR audio_out_dc_remove_op_left.next = &$audio_out_dc_remove_op_right;
      .VAR audio_out_dc_remove_op_left.func = &$cbops.dc_remove;
      .VAR audio_out_dc_remove_op_left.param[$cbops.dc_remove.STRUC_SIZE] =
         0,                               // Input index (left cbuffer)
         0;                               // Output index (left cbuffer)
   .ENDBLOCK;
.ifndef HARDWARE_RATE_MATCH    
   .BLOCK $audio_out_dc_remove_op_right;
      .VAR audio_out_dc_remove_op_right.next = &$audio_out_rate_adjustment_and_shift_op_stereo;
      .VAR audio_out_dc_remove_op_right.func = &$cbops.dc_remove;
      .VAR audio_out_dc_remove_op_right.param[$cbops.dc_remove.STRUC_SIZE] =
         1,                               // Input index (right cbuffer)
         1;                               // Output index (right cbuffer)
   .ENDBLOCK;

.VAR/DM1CIRC $sr_hist_left[$cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE];
.VAR/DM1CIRC $sr_hist_right[$cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE];
.BLOCK $audio_out_rate_adjustment_and_shift_op_stereo;
      .VAR $audio_out_rate_adjustment_and_shift_op_stereo.next = $cbops.NO_MORE_OPERATORS;
      .VAR $audio_out_rate_adjustment_and_shift_op_stereo.func = &$cbops.rate_adjustment_and_shift;
      .VAR $audio_out_rate_adjustment_and_shift_op_stereo.param[$cbops.rate_adjustment_and_shift.STRUC_SIZE] =
      0,  //   INPUT1_START_INDEX_FIELD
      2,  //   OUTPUT1_START_INDEX_FIELD
      1,  //   INPUT2_START_INDEX_FIELD
      3,  //   OUTPUT2_START_INDEX_FIELD
      -8, //   SHIFT_AMOUNT_FIELD
      &$sra_coeffs, 
      &$sr_hist_left, 
      &$sr_hist_right,
      &$sra_struct + $sra.SRA_RATE_FIELD,
      $cbops.dither_and_shift.DITHER_TYPE_NONE; //    DITHER_TYPE_FIELD
    .ENDBLOCK;
 .else //.ifndef HARDWARE_RATE_MATCH
   .BLOCK $audio_out_dc_remove_op_right;
      .VAR audio_out_dc_remove_op_right.next = &$audio_out_dither_and_shift_op_left;
      .VAR audio_out_dc_remove_op_right.func = &$cbops.dc_remove;
      .VAR audio_out_dc_remove_op_right.param[$cbops.dc_remove.STRUC_SIZE] =
         1,                               // Input index (right cbuffer)
         1;                               // Output index (right cbuffer)
   .ENDBLOCK;

   .VAR/DM1CIRC $dither_hist_left[$cbops.dither_and_shift.FILTER_COEFF_SIZE];
   .VAR/DM1CIRC $dither_hist_right[$cbops.dither_and_shift.FILTER_COEFF_SIZE];
   .BLOCK $audio_out_dither_and_shift_op_left;
      .VAR $audio_out_dither_and_shift_op_left.next = &$audio_out_dither_and_shift_op_right;
      .VAR $audio_out_dither_and_shift_op_left.func = &$cbops.dither_and_shift;
      .VAR $audio_out_dither_and_shift_op_left.param[$cbops.dither_and_shift.STRUC_SIZE] =
         0,                                        // Input index
         2,                                        // Output index
        -8,                                        // amount of shift after dithering
         $cbops.dither_and_shift.DITHER_TYPE_NONE, // type of dither
         &$dither_hist_left;                       // history buffer for dithering (size = $cbops.dither_and_shift.FILTER_COEFF_SIZE)
   .ENDBLOCK; 
   
   .BLOCK $audio_out_dither_and_shift_op_right;
      .VAR $audio_out_dither_and_shift_op_right.next = $cbops.NO_MORE_OPERATORS;
      .VAR $audio_out_dither_and_shift_op_right.func = &$cbops.dither_and_shift;
      .VAR $audio_out_dither_and_shift_op_right.param[$cbops.dither_and_shift.STRUC_SIZE] =
         1,                                        // Input index
         3,                                        // Output index
        -8,                                        // amount of shift after dithering
         $cbops.dither_and_shift.DITHER_TYPE_NONE, // type of dither
         &$dither_hist_right;                      // history buffer for dithering (size = $cbops.dither_and_shift.FILTER_COEFF_SIZE)
   .ENDBLOCK;
 .endif //.ifndef HARDWARE_RATE_MATCH

   // ** allocate memory for mono audio out cbops copy routine **
   .VAR $mono_out_copy_struc[] =
      &$audio_out_tone_upsample_mono_mix, // first operator block
      1,                                  // number of inputs
      &$dac_out_left_cbuffer_struc,       // input
      1,                                  // number of outputs
     $AUDIO_LEFT_OUT_PORT;                // output
  
   //tone mixing can not be the last operator as it does upsamling and mixing in place
   .BLOCK $audio_out_tone_upsample_mono_mix;
      .VAR $audio_out_tone_upsample_mono_mix.next = &$audio_out_dc_remove_op_mono;
      .VAR $audio_out_tone_upsample_mono_mix.func = &$cbops.auto_upsample_and_mix;
      .VAR $audio_out_tone_upsample_mono_mix.param[$cbops.auto_resample_mix.STRUC_SIZE] =
         0,                               // Input index to left channel
         -1,                              // Index to right channel (-1: no right cchannel)
       &$tone_in_cbuffer_struc,         // cbuffer structure containing tone samples
       &$sra_coeffs,             // 
         &$current_dac_sampling_frequency,// pointer to variable containing dac frequency received from vm ( zero pointer or zero value = not received any)
         SELECTED_CODEC_GETFS_API,        // api function to decoder sampling frequency 
         8000,                            // samling frequency of the tone (at the moment only 8000 is supported, otherwise a run time error happens)
         0.5,                             // tone volume mixing
         0.5;                             // audio volume mixing
   .ENDBLOCK;
   
.ifndef HARDWARE_RATE_MATCH
   .BLOCK $audio_out_dc_remove_op_mono;
      .VAR audio_out_dc_remove_op_mono.next = &$audio_out_rate_adjustment_and_shift_op_mono;
      .VAR audio_out_dc_remove_op_mono.func = &$cbops.dc_remove;
      .VAR audio_out_dc_remove_op_mono.param[$cbops.dc_remove.STRUC_SIZE] =
         0,                               // Input index
         0;                               // Output index
   .ENDBLOCK;

.BLOCK $audio_out_rate_adjustment_and_shift_op_mono;
      .VAR $audio_out_rate_adjustment_and_shift_op_mono.next = $cbops.NO_MORE_OPERATORS;
      .VAR $audio_out_rate_adjustment_and_shift_op_mono.func = &$cbops.rate_adjustment_and_shift;
      .VAR $audio_out_rate_adjustment_and_shift_op_mono.param[$cbops.rate_adjustment_and_shift.STRUC_SIZE] =
      0,  //   INPUT1_START_INDEX_FIELD
      1,  //   OUTPUT1_START_INDEX_FIELD
      -1, //   INPUT2_START_INDEX_FIELD
      -1, //   OUTPUT2_START_INDEX_FIELD
      -8, //   SHIFT_AMOUNT_FIELD
      &$sra_coeffs, 
      &$sr_hist_left, 
      0, 
      &$sra_struct + $sra.SRA_RATE_FIELD,
      $cbops.dither_and_shift.DITHER_TYPE_NONE;  //   DITHER_TYPE_FIELD
    .ENDBLOCK;
.else //.ifndef HARDWARE_RATE_MATCH
   .BLOCK $audio_out_dc_remove_op_mono;
      .VAR audio_out_dc_remove_op_mono.next = &$audio_out_dither_and_shift_op_mono;
      .VAR audio_out_dc_remove_op_mono.func = &$cbops.dc_remove;
      .VAR audio_out_dc_remove_op_mono.param[$cbops.dc_remove.STRUC_SIZE] =
         0,                               // Input index
         0;                               // Output index
   .ENDBLOCK;
   
    .BLOCK $audio_out_dither_and_shift_op_mono;
      .VAR $audio_out_dither_and_shift_op_mono.next = $cbops.NO_MORE_OPERATORS;
      .VAR $audio_out_dither_and_shift_op_mono.func = &$cbops.dither_and_shift;
      .VAR $audio_out_dither_and_shift_op_mono.param[$cbops.dither_and_shift.STRUC_SIZE] =
         0,                                        // Input index
         1,                                        // Output index
        -8,                                        // amount of shift after dithering
         $cbops.dither_and_shift.DITHER_TYPE_NONE, // type of dither
         &$dither_hist_left;                       // history buffer for dithering (size = $cbops.dither_and_shift.FILTER_COEFF_SIZE)
   .ENDBLOCK;

.endif //.ifndef HARDWARE_RATE_MATCH

   //allocatiing memory for  
   .VAR/DM1 $decoder_codec_stream_struc[$codec.av_decode.STRUC_SIZE] =
      SELECTED_CODEC_FRAME_DECODE_FUNCTION,    // frame_decode function
      SELECTED_CODEC_RESET_DECODER_FUNCTION,   // reset_decoder function
      SELECTED_CODEC_SILENCE_DECODER_FUNCTION, // silence_decoder function
      &$codec_in_cbuffer_struc,                // in cbuffer
      &$audio_out_left_cbuffer_struc,          // out left cbuffer
      &$audio_out_right_cbuffer_struc,         // out right cbuffer
      0,                                  // MODE_FIELD          
      0,                                  // STALL_COUNTER_FIELD              
      0,                                  // BUFFERING_THRESHOLD_FIELD        
      GOOD_WORKING_BUFFER_LEVEL,                           
      0,                                  // POORLINK_DETECT_LEVEL - no longer used
      1,                                  // Enable codec in buffer purge when in pause
      &$master_app_reset;


   .BLOCK $frame_process_data_block;
      .VAR $amount_ready[4];
      .VAR $frame_process_struc[$frame_process.STRUC_SIZE] =
         // Stream Connected State Field
         0,
         // Stream Connecting State Field
         0,
         // Pointer to Amount Ready buffer
         &$amount_ready,
         // Minimum Amt Ready
         0,
         // NUM OUTPUTs
         2,
         // Number of Rate Objects
         4;

      // -----------------------------------------------------------------------------
      // CBUFFER STRUCTURE TABLE
      // This table contains cbuffer structures that correspond to the streams used in
      // this system.  Cbuffer structures contain read and write pointer positions
      // into the buffers that they describe.  This information is passed to the
      // processing modules via $frame.distribute_streams and frame.update_streams.
      //
      // Cbuffer Overview using SCO input as an example:
      // SCO input data is copied from the MMU buffer by the cbops.copy operator in
      // the 1ms $audio_copy_handler routine.  As this operator copies new data into
      // the cbuffer, it advances the write pointer into this buffer.  Meanwhile, the
      // main loop is always checking to see if enough data has arrived on all of the
      // connected streams.  When enough data is available, the processing routine is
      // called, which processes the mode function table.  The current read address
      // of the sco_in cbuffer is loaded into all of the modules that read data from
      // this buffer.  Processing modules can also use this address to write data back
      // into this buffer in an in-place fashion.  After all of the processing modules
      // have been called $frame.update_streams updates the read pointer position of
      // the cbuffer.
      //
      // Streams can be added or removed from this table to suit the application
      // being developed.

      .VAR  $cbuffer_strucs[] =
         // Inputs
         &$audio_out_left_cbuffer_struc,
         &$audio_out_right_cbuffer_struc,
         // Outputs        
         &$dac_out_left_cbuffer_struc,
         &$dac_out_right_cbuffer_struc;



   .ENDBLOCK;  // frame process data block

.ifdef SELECTED_CODEC_SBC
    .VAR $codec_type = 0;
.endif
.ifdef SELECTED_CODEC_MP3
    .VAR $codec_type = 1;
.endif

.VAR $sra_struct[$sra.STRUC_SIZE] = 
      (SRA_AVERAGING_TIME*1000000)/TMR_PERIOD_CODEC_COPY,
      $CODEC_IN_PORT,
      &$codec_in_cbuffer_struc,
      &$audio_out_left_cbuffer_struc,
      0.0025, //MAX RATE
      48000*SRA_AVERAGING_TIME;

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

   // initialise the codec decoder library
   call SELECTED_CODEC_INITIALISE_DECODER_FUNCTION;

  // set up message handler for VM_DAC_SAMPLING_FREQUENCY_MESSAGE_ID message
   r1 = &$get_dac_rate_from_vm_message_struc;
   r2 = VM_DAC_RATE_MESSAGE_ID;
   r3 = &$get_dac_rate_from_vm;
   call $message.register_handler;

   // intialize SPI communications library
   call $spi_comm.initialize;

   // Power Up Reset needs to be called once during program execution
   call $music_example.power_up_reset;
   
   r2 = $music_example.VMMSG.READY;
   r3 = $MUSIC_EXAMPLE_SYSID;
   // status
   r4 = M[$music_example.Version];     
   r4 = r4 LSHIFT -8;
   call $message.send_short;

   // tell vm we're ready and wait for the go message
   call $message.send_ready_wait_for_go;
   
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
      // Check Communication
      call $spi_comm.polled_service_routine;
      
     r5 = &$decoder_codec_stream_struc;
     call $codec.av_decode;
     call $sra_calcrate;
 
      // to synchronize frame process to audio interrupt
      r5 = &$decoder_codec_stream_struc;
      r0 = M[r5 + $codec.av_decode.MODE_FIELD ];
      Null = r0 - $codec.SUCCESS;
      if NZ call $frame_sync.1ms_delay;
      
      // to gather stream state, amount ready and min amt ready
      // set up r8 to point to frame process data object
      r8 = &$frame_process_struc;
      call $frame_process;

      // depending on connecting state, purge associated cbuffers
      r3 = M[$frame_process_struc + $frame_process.STREAM_CONNECTING_STATE_FIELD];
      if NZ call $stream_has_connected;

      // check to see if custom process needs to be called
      r0 = M[$amount_ready];
      r1 = M[$amount_ready + 2];
      NULL = r0 - r1;
      if POS r0 = r1;

      r0 = r0 - $music_example.NUM_SAMPLES_PER_FRAME;
      // call processing function if block-size worth of data/space available
      if POS call $music_example_process;

   jump frame_loop;

$stream_has_connected:
    $push_rLink_macro;
    call $block_interrupts;
    r2 = &$audio_out_left_cbuffer_struc;
    call $frame_sync.cbuffer.purge;
    r2 = &$audio_out_right_cbuffer_struc;
    call $frame_sync.cbuffer.purge;
    r2 = &$dac_out_left_cbuffer_struc;
    call $frame_sync.cbuffer.purge;
    r2 = &$dac_out_right_cbuffer_struc;
    call $frame_sync.cbuffer.purge;    
    call $unblock_interrupts;
    jump $pop_rLink_and_rts;


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

   $audio_out_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // If not using cbops.stream_copy, need to manually
   // reset the sync flag.
   M[$cbops.sync_flag]=Null;

   r8 = &$stereo_out_copy_struc;
   r7 = &$mono_out_copy_struc;
   // see if mono or stereo connection, based on whether the right output port
   // is enabled
   r0 = $AUDIO_RIGHT_OUT_PORT;
   call $cbuffer.is_it_enabled;
.ifdef FORCE_MONO
   r0 = 0;
.endif
   if Z r8 = r7;

   // Call the copy routine
   call $cbops.dac_av_copy;

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
   r8 = &$tone_in_copy_struc;
   call $cbops.copy;

   // post another timer event
   r1 = &$tone_copy_timer_struc;
   r2 = TMR_PERIOD_TONE_COPY;
   r3 = &$tone_copy_handler;
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

   $codec_in_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;
   
   call $sra_tagtimes;
   
   // copy data from the port to the cbuffer
   r8 = &$codec_in_copy_struc;
   call $cbops.copy;
  .ifdef HARDWARE_RATE_MATCH
      call $apply_hardware_warp_rate;
  .endif

// just for debugging, will be removed
   .VAR $codec_level;
 r0 = &$codec_in_cbuffer_struc;
 call $cbuffer.calc_amount_data;
 M[$codec_level] = r0;
 
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
//    $get_dac_rate_from_vm
//
// DESCRIPTION: message handler for receiving DAC rate from VM
// 
// INPUTS:
//  r1 = dac sampling rate
//  r2 = maimum clock mismatch to compensate (r2/10)%
//  r3 = bit0: if long term mismatch rate saved, bits(15:1): saved_rate>>5
// *****************************************************************************
.MODULE $M.get_dac_rate_from_vm;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
$get_dac_rate_from_vm:
   // push rLink onto stack
   $push_rLink_macro;
   
   // save sampling rate
   r1 = r1 AND 0xFFFF;
   M[$current_dac_sampling_frequency] = r1;
   r1 = r1 * SRA_AVERAGING_TIME (int);
   M[$sra_struct + $sra.AUDIO_AMOUNT_EXPECTED_FIELD] = r1;
   
   // set maximum rate for clock mismatch compensation
   r2 = r2 AND 0x7F;
   r1 = r2 - 3;         // min 0.3% percent by default
   if NEG r2 = r2 -r1;
   r2 = r2 * 0.001(int);
   r1 = r2 * 0.25(frac); // to cover jitter
   r2 = r2 + r1;
   r1 = r2 - ABSOLUTE_MAXIMUM_CLOCK_MISMATCH_COMPENSATION;
   if POS r2 = r2 - r1;
   M[$sra_struct + $sra.MAX_RATE_FIELD] = r2;
   r2 = 0.5; // just a big number
   M[$sra_struct + $sra.LONG_TERM_RATE_FIELD] = r2;
   
   // see if clock mismatch rate received from vm
   r0 = r3 AND 0x1;
   if Z jump end;
   
   // get saved clock mismatch rate
   r3 = r3 ASHIFT -1;
   r3 = r3 ASHIFT 6;
   
   // make sure it is not out of range
   Null = r3 - M[$sra_struct + $sra.MAX_RATE_FIELD];
   if POS jump end;
   Null = r3 + M[$sra_struct + $sra.MAX_RATE_FIELD];
   if NEG jump end;
   
   // initialize some variables based on the saved rate
   M[$sra_struct + $sra.RATE_BEFORE_FIX_FIELD ] = r3;
   M[$sra_struct + $sra.SRA_RATE_FIELD ] = r3;
   r0 = M[$sra_struct + $sra.AUDIO_AMOUNT_EXPECTED_FIELD];
   r0 = r0 * r3 (frac);
   M[$sra_struct + $sra.HIST_BUFF_FIELD + 0] = r0;
   M[$sra_struct + $sra.HIST_BUFF_FIELD + 1] = r0;
   r0 = 2;
   M[$sra_struct + $sra.HIST_INDEX_FIELD] = r0;
   r0 = 1;
   M[$sra_struct + $sra.LONG_TERM_RATE_DETECTED_FIELD] = r0;
   .ifndef HARDWARE_RATE_MATCH
      M[$audio_out_rate_adjustment_and_shift_op_mono.param + $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD] = r3;
      M[$audio_out_rate_adjustment_and_shift_op_stereo.param + $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD] = r3;
   .else
      //  apply initial hardware warp rate
      M[$current_hw_rate] = r3;
      r4 = r3 * (-1.0/64.0)(frac);
      r2 = $MESSAGE_WARP_DAC;
      r3 = 3;
      call $message.send_short;
   .endif   

   end:
   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;
// *****************************************************************************
//
// Master reset routine, called to clear garbage samples during a pause
//
// *****************************************************************************
.MODULE $M.master_app_reset;
   .CODESEGMENT PM;
   .DATASEGMENT DM;


   $master_app_reset:

   // push rLink onto stack
   $push_rLink_macro;   

   // Purge dac out left buffer
   r0 = M[&$dac_out_left_cbuffer_struc + $cbuffer.WRITE_ADDR_FIELD];   
   M[&$dac_out_left_cbuffer_struc + $cbuffer.READ_ADDR_FIELD] = r0;    

   // Purge dac out right buffer
   r0 = M[&$dac_out_right_cbuffer_struc + $cbuffer.WRITE_ADDR_FIELD];   
   M[&$dac_out_right_cbuffer_struc + $cbuffer.READ_ADDR_FIELD] = r0;        

   // Clear EQ operator
   r4 = &$M.system_config.data.reinitialize_table;
   call $frame.run_function_table;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

.ifdef HARDWARE_RATE_MATCH
// *****************************************************************************
// MODULE:
//   $apply_hardware_warp_rate
// DESCRIPTION:
//   Applies hardware warp rate by sending message to the firmware
//
// *****************************************************************************
.MODULE $M.apply_hardware_warp_rate;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $apply_hardware_warp_rate:

   // push rLink onto stack
   $push_rLink_macro;   
  
  .define HW_APPLY_WARP_RATE_TMR_MS 64 // timer period to apply hardware warp rate (ms)
  .define HW_APPLY_WARP_MOVING_STEP 64 // max rate change in each period (Q.23)
  
  .VAR $current_hw_rate = 0;
  .VAR $rate_apply_cntr;
  
   // see if time to apply hw warp rate
   r0 = M[$rate_apply_cntr];
   r0 = r0 + 1;
   M[$rate_apply_cntr] = r0;
   Null = r0 - ((HW_APPLY_WARP_RATE_TMR_MS*1000)/TMR_PERIOD_CODEC_COPY);
   if NEG jump end_hw_rate_apply;
      // reset counter
      M[$rate_apply_cntr] = 0; 
      
      // slowly move towards the target rate 
      r5 = M[$current_hw_rate];
      r4 = M[$sra_struct + $sra.SRA_RATE_FIELD]; // maximum 3%, otherwise overflow happens      
      
      // calculate moving step (logarithmic then linear)
      r1 = HW_APPLY_WARP_MOVING_STEP;
      r0 = r5 - r4;
      if NEG r0 = -r0;
      r2 = r0 * (0.001*HW_APPLY_WARP_RATE_TMR_MS)(frac);
      Null = r0 - 0.0015;
      if NEG r2 = r1;
      r1 = r2 - (20*HW_APPLY_WARP_MOVING_STEP);
      if POS r2 = r2 - r1;
      r1 = r5 - r4;   
      r0 = r1 - r2;
      if POS r1 = r1 - r0;
      r0 = r1 + r2;
      if NEG r1 = r1 - r0;
      
      // update the current rate
      r5 = r5 - r1;
      r4 = r5 ASHIFT -6; 
      r5 = r4 ASHIFT 6;
      r1 = r5 - M[$current_hw_rate];
      if Z jump end_hw_rate_apply;
         //  apply harware warp rate
         M[$current_hw_rate] = r5;
         r4 = -r4;
         r2 = $MESSAGE_WARP_DAC;
         r3 = 3;
         call $message.send_short;
   end_hw_rate_apply:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
.endif
