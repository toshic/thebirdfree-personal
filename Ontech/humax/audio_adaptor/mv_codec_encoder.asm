// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc %%copyright(2005)        http://www.csr.com
// %%version
//
// $Revision$  $Date$
// *****************************************************************************

// **********************************************************************************************
// NAME:
//    DSP Audio Adaptor Application
//
// DESCRIPTION:
//    This is DSP Audio Adaptor Application which encodes data to stream over the 
//    air, the older version of this application is called MVDongle,
//    the audio input for the codec can be one of these sources:
//    - USB (from PC for example)
//    - Analogue (e.g. internal ADC)
//  in both modes it supports SBC, MP3 and CSR proprietary FastStream codecs
//  the application is configured by vm to operate in one of  the following modes
//  1- AV mode: Music comming from USB is encoded and streamed over the air
//  2- SCO mode: SCO input and output chanels are passed to USB port, no further processing
//     is performed apart from some sample rate conversion where required
//  3- Idle mode: no data is streamed
//  4 - Analogue mode, uses ADC ports as input to the codecs
//
// Monitoring: In all mode except Analogue mode the USB input and output ports are
//    monitored to check the activity on this ports, any change detected will be
//
//    sent to the vm to give it a hint about switching to the proper new mode
// Sample Rate Converter:
//  USB audio from PC is assumed to be 48khz stereo, to make the codec enable to
//  encode in other sample rates, sample-rate-converter(SRC) is used before passing audio
//  data to codecs, SCR is active only in AV-mode and when codec sample rate is not 48khz
//
//
// ***********************************************************************************************


//.define USES_PEQ


 // ** defining timer interrupt periods ** 
.define $TMR_PERIOD_AUDIO_COPY               1000   // timer period to read the audio data in analogue mode (for MP3 and SBC)
.define $FASTSTREAM_TMR_PERIOD_AUDIO_COPY    500    // timer period to read the audio data in analogue mode (for faststream only)
.define $TMR_PERIOD_CODEC_COPY               8000   // timer period to write codec output to codec port (for MP3 and SBC)
.define $FASTSTREAM_TMR_PERIOD_CODEC_COPY    1000   // timer period to write codec output to codec port (for faststream only)
 
.define $PEQ_COPY_MINIMUM                    25  // based on 1000 audio copy timer period
.define $PEQ_COPY_MAXIMUM                    50  // based on 1000 audio copy timer period

.define $BLOCK_SIZE                    16
.define $RESAMPLER_READ_BACK           50
.define $USB_SAMPLE_RATE 48000
.define $DEFAULT_MINIMIMUM_CODEC_DATA_TO_COPY 1
.define $FASTSTREAM_MINIMUM_CODEC_DATA_TO_COPY 100 //this limit is for faststream copying codec data into the port (slightly less than 3 frames = 3*36=108 words

.ifdef SELECTED_ENCODER_SBC 
   .define SELECTED_CODEC_FRAME_ENCODE_FUNCTION        &$sbcenc.frame_encode
   .define SELECTED_CODEC_RESET_ENCODER_FUNCTION       &$sbcenc.reset_encoder
   .define SELECTED_CODEC_INITIALISE_ENCODER_FUNCTION  $sbcenc.init_encoder
   .define SELECTED_ENCODER_LIBRARY_HEADER             "sbc_library.h" 
   .define SBC_BITPOOL_CHANGE_MESSAGE                  0x7070
   .define SBC_BITPOOL_SET_LOW_MESSAGE                 0x7080
   .define SELECTED_ENCODER_GETFS_FUNCTION             &$sbcenc.get_sampling_frequency // used to see if SRC is required
   .define $PAUSE_SILENCE_DURATION_TO_SEND_MS 200     // amount of silence (in ms) streamed when inactivity in USB port is detected
.endif

.ifdef SELECTED_ENCODER_MP3 
   .define SELECTED_CODEC_FRAME_ENCODE_FUNCTION        &$mp3enc.frame_encode
   .define SELECTED_CODEC_RESET_ENCODER_FUNCTION       &$mp3enc.reset_encoder
   .define SELECTED_CODEC_INITIALISE_ENCODER_FUNCTION  &$mp3enc.init_encoder
   .define SELECTED_ENCODER_LIBRARY_HEADER             "mp3enc_library.h" 
    .CONST $MESSAGE_MP3ENC_UPDATE_SETTING               0x7060;
   .define SELECTED_ENCODER_GETFS_FUNCTION             &$mp3enc.get_sampling_frequency // used to see if SRC is required
   .define $PAUSE_SILENCE_DURATION_TO_SEND_MS 700        // amount of silence (in ms) streamed when inactivity in USB port is detected
.endif

// includes required header files
.include SELECTED_ENCODER_LIBRARY_HEADER
.include "core_library.h"
.include "cbops_library.h"
.include "codec_library.h"
.include "mv_codec_encoder.h"
.include "src.h"


.ifdef USES_PEQ

   .CONST $mv_dongle.peq.INPUT_ADDR_FIELD               0;
   
   // Size of input audio stream circular buffer size, '0' if linear buffer
   .CONST $mv_dongle.peq.INPUT_SIZE_FIELD               1;

   // Pointer to output audio stream
   .CONST $mv_dongle.peq.OUTPUT_ADDR_FIELD              2;
   
   // Size of output audio stream circular buffer size, '0' if linear buffer
   .CONST $mv_dongle.peq.OUTPUT_SIZE_FIELD              3;

   // Pointer to PEQ delay buffer, MUST be circular
   // Minimum size of the buffer: 2 * (number of stages + 1)
   .CONST $mv_dongle.peq.DELAYLINE_ADDR_FIELD           4;
   
   // Pointer to PEQ filter coefficients buffer, MUST be circular
   // Minimum size of the buffer: 5 * (number of stages)
   // The filter coefficients should be arranged in the following order:
   // - stage 1: b2(1), b1(1), b0(1), a2(1), a1(1)
   // - stage 2: b2(2), b1(2), b0(2), a2(2), a1(2)
   // . .......: ....., ....., ....., ....., .....
   // - stage n: b2(n), b1(n), b0(n), a2(n), a1(n)
   .CONST $mv_dongle.peq.COEFS_ADDR_FIELD               5;
   
   // Number of stage
   .CONST $mv_dongle.peq.NUM_STAGES_FIELD               6;

   // Size of delay line circular buffer
   // This field is set by initialization routine based on NUM_STAGES_FIELD
   .CONST $mv_dongle.peq.DELAYLINE_SIZE_FIELD           7;
   
   // Size of filter coefficients circular buffer
   // This field is set by initialization routine based on NUM_STAGES_FIELD
   .CONST $mv_dongle.peq.COEFS_SIZE_FIELD               8;
   
   // Size of data block to be processed
   .CONST $mv_dongle.peq.BLOCK_SIZE_FIELD               9;

   // Pointer to scaling buffer
   // Minimum size of the buffer: (number of stages), scaling for each stage
   .CONST $mv_dongle.peq.SCALING_ADDR_FIELD             10;
   
   // Pointer to 'gain exponent' variable
   .CONST $mv_dongle.peq.GAIN_EXPONENT_ADDR_FIELD       11;
   
   // Pointer to 'gain mantisa' variable
   .CONST $mv_dongle.peq.GAIN_MANTISA_ADDR_FIELD        12;
   
   // Structure size of PEQ data object
   .CONST $mv_dongle.peq.STRUC_SIZE                     13;
   
.endif


// *****************************************************************************
// DESCRIPTION
//    Main routine for Audio Adaptor application
//
//    Input: none
//    Output: none
//    Trash: Everything
//
// *****************************************************************************
.MODULE $M.main;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $main:

   // ** setup ports that are to be used **
   .CONST  $AUDIO_LEFT_IN_PORT     ($cbuffer.READ_PORT_MASK + 0);
   .CONST  $AUDIO_RIGHT_IN_PORT    ($cbuffer.READ_PORT_MASK + 1);

   .CONST  $CODEC_OUT_PORT    ($cbuffer.WRITE_PORT_MASK + 2);
   .CONST  $CODEC_OUT_PORT_TWO ($cbuffer.WRITE_PORT_MASK + 3);


   // ** allocate memory for cbuffers and structures **
   
   // ** defining cbuffers for input audio
   
.ifdef USES_PEQ
   declare_cbuf_and_struc($peq_in_left,    $peq_in_left_cbuffer_struc,    AUDIO_CBUFFER_SIZE)
   declare_cbuf_and_struc($peq_in_right,   $peq_in_right_cbuffer_struc,   AUDIO_CBUFFER_SIZE)   
.endif
   
   // ** if sampling rate is 48000 they are directly passed to the codec other wise used as input of resampler **
   declare_cbuf_and_struc($audio_in_left,    $audio_in_left_cbuffer_struc,    AUDIO_CBUFFER_SIZE)
   declare_cbuf_and_struc($audio_in_right,   $audio_in_right_cbuffer_struc,   AUDIO_CBUFFER_SIZE)   

   // ** defining cbuffers for codec output **
   declare_cbuf_and_struc($codec_out,        $codec_out_cbuffer_struc,        CODEC_CBUFFER_SIZE)

  // allocate memory for filter coefficients
  // this definition moved to here from $src module, the reason is to share its memory with cbuffesr
  // circular definition is not necessary for source coefss, however to be shared with cbuffers it must be circular
  .VAR/DM2CIRC $src.coeffs[$src.SRC_MAX_UPSAMPLE_RATE*$src.SRC_MAX_FILTER_LEN];
  

   // ** allocate memory for timer structures **
   .VAR $audio_in_timer_struc[$timer.STRUC_SIZE];           /* Analogue input -> DSP */
    
   // ** Audio PEQ buffers **
   .define MAX_NUM_PEQ_STAGES (5)

.ifdef USES_PEQ   
   .VAR  ZeroValue = 0;            
   .VAR  OneValue = 1.0;
   
   .VAR/DM2CIRC left_peq_delaybuf_dm2[2 * (MAX_NUM_PEQ_STAGES + 1)];
   .VAR/DM2CIRC right_peq_delaybuf_dm2[2 * (MAX_NUM_PEQ_STAGES + 1)];   
   .VAR/DM1CIRC peq_coeffs1[5 * MAX_NUM_PEQ_STAGES] = 
                0x33E7EF,
                0x8CAB31,
                0x42116A,
                0x35F95A,
                0x8CAB31,
                0x2BE882,
                0x9AF23F,
                0x4127C1,
                0x2D1044,
                0x9AF23F,
                0x1FC756,
                0xB6E2D3,
                0x3FFFFF,
                0x1FC756,
                0xB6E2D3,
                0x1D7F9A,
                0xD2A8DE,
                0x7FFFFF,
                0x1D7F9A,
                0xD2A8DE,
                0xE658C4,
                0x465179,
                0x7FFFFF,
                0xE658C4,
                0x465179;

         
   .VAR eq_scale_buf1[MAX_NUM_PEQ_STAGES] = 1,1,1,0,0;
   
   .VAR $left_peq_struc[$mv_dongle.peq.STRUC_SIZE] = 
      0,                                    // PTR_INPUT_DATA_BUFF_FIELD
      0,                                    // INPUT_CIRCBUFF_SIZE_FIELD
      0,                                    // PTR_OUTPUT_DATA_BUFF_FIELD
      0,                                    // OUTPUT_CIRCBUFF_SIZE_FIELD
      &left_peq_delaybuf_dm2,               // PTR_DELAY_LINE_FIELD
      &peq_coeffs1,                         // PTR_COEFS_BUFF_FIELD
      MAX_NUM_PEQ_STAGES,                   // NUM_STAGES_FIELD
      0,                                    // DELAY_BUF_SIZE 
      0,                                    // COEFF_BUF_SIZE
      0,                                    // BLOCK_SIZE_FIELD
      &eq_scale_buf1,                       // PTR_SCALE_BUFF_FIELD
      &ZeroValue,                           // INPUT_GAIN_EXPONENT_PTR
      &OneValue;                            // INPUT_GAIN_MANTISSA_PTR     

   .VAR $right_peq_struc[$mv_dongle.peq.STRUC_SIZE] = 
      0,                                    // PTR_INPUT_DATA_BUFF_FIELD
      0,                                    // INPUT_CIRCBUFF_SIZE_FIELD
      0,                                    // PTR_OUTPUT_DATA_BUFF_FIELD
      0,                                    // OUTPUT_CIRCBUFF_SIZE_FIELD
      &right_peq_delaybuf_dm2,              // PTR_DELAY_LINE_FIELD
      &peq_coeffs1,                         // PTR_COEFS_BUFF_FIELD
      MAX_NUM_PEQ_STAGES,                   // NUM_STAGES_FIELD
      0,                                    // DELAY_BUF_SIZE 
      0,                                    // COEFF_BUF_SIZE
      0,                                    // BLOCK_SIZE_FIELD
      &eq_scale_buf1,                       // PTR_SCALE_BUFF_FIELD
      &ZeroValue,                           // INPUT_GAIN_EXPONENT_PTR
      &OneValue;                            // INPUT_GAIN_MANTISSA_PTR
.endif


   .VAR/DM  $codec_sample_rate = $USB_SAMPLE_RATE; // default sample rate = 48000
   .VAR/DM  $codec_type = 0;                       // default codec type = SBC

   // ** allocate memory for stereo audio out cbops copy routine **
   .VAR $stereo_audio_in_copy_struc[] =
         &$audio_in_shift_op_left,        // first operator block
         2,                               // number of inputs
         $AUDIO_LEFT_IN_PORT,             // input
         $AUDIO_RIGHT_IN_PORT,            // input
         2,                               // number of outputs
.ifdef USES_PEQ
         &$peq_in_left_cbuffer_struc,     // output
         &$peq_in_right_cbuffer_struc;    // output
.else
         &$audio_in_left_cbuffer_struc,   // output
         &$audio_in_right_cbuffer_struc;  // output
.endif


   .BLOCK $audio_in_shift_op_left;
         .VAR audio_in_shift_op_left.next = &$audio_in_dc_remove_op_left;
         .VAR audio_in_shift_op_left.func = &$cbops.shift;
         .VAR audio_in_shift_op_left.param[$cbops.shift.STRUC_SIZE] =
                  0,                      // Input index (left input port)
                  2,                      // Output index (left cbuffer)
                  8;                      // Shift amount
   .ENDBLOCK;

   .BLOCK $audio_in_dc_remove_op_left;
         .VAR audio_in_dc_remove_op_left.next = &$audio_in_shift_op_right;
         .VAR audio_in_dc_remove_op_left.func = &$cbops.dc_remove;
         .VAR audio_in_dc_remove_op_left.param[$cbops.dc_remove.STRUC_SIZE] =
                  2,                      // Input index (left cbuffer)
                  2;                      // Output index (left cbuffer)
   .ENDBLOCK;

   .BLOCK $audio_in_shift_op_right;
         .VAR audio_in_shift_op_right.next = &$audio_in_dc_remove_op_right;
         .VAR audio_in_shift_op_right.func = &$cbops.shift;
         .VAR audio_in_shift_op_right.param[$cbops.shift.STRUC_SIZE] =
                  1,                      // Input index (right input port)
                  3,                      // Output index (right cbuffer)
                  8;                      // Shift amount
   .ENDBLOCK;
   
.ifdef ANALOGUE_NOISE_GATE_OFF

   .BLOCK $audio_in_dc_remove_op_right;
         .VAR audio_in_dc_remove_op_right.next = $cbops.NO_MORE_OPERATORS;
         .VAR audio_in_dc_remove_op_right.func = &$cbops.dc_remove;
         .VAR audio_in_dc_remove_op_right.param[$cbops.dc_remove.STRUC_SIZE] =
                  3,                      // Input index (right cbuffer)
                  3;                      // Output index (right cbuffer)
   .ENDBLOCK;
   
.else // .ifdef ANALOGUE_NOISE_GATE_OFF 

  .BLOCK $audio_in_dc_remove_op_right;
         .VAR audio_in_dc_remove_op_right.next = &$audio_in_noise_gate_op_left;
         .VAR audio_in_dc_remove_op_right.func = &$cbops.dc_remove;
         .VAR audio_in_dc_remove_op_right.param[$cbops.dc_remove.STRUC_SIZE] =
                  3,                      // Input index (right cbuffer)
                  3;                      // Output index (right cbuffer)
   .ENDBLOCK;
   
   .BLOCK $audio_in_noise_gate_op_left;
         .VAR audio_in_noise_gate_op_left.next = &$audio_in_noise_gate_op_right;
         .VAR audio_in_noise_gate_op_left.func = &$cbops.noise_gate;
         .VAR audio_in_noise_gate_op_left.param[$cbops.noise_gate.STRUC_SIZE] =
                  2,                      // Input index (left cbuffer)
                  2;                      // Output index (left cbuffer)
   .ENDBLOCK;

   .BLOCK $audio_in_noise_gate_op_right;
         .VAR audio_in_noise_gate_op_right.next = $cbops.NO_MORE_OPERATORS;
         .VAR audio_in_noise_gate_op_right.func = &$cbops.noise_gate;
         .VAR audio_in_noise_gate_op_right.param[$cbops.noise_gate.STRUC_SIZE] =
                  3,                      // Input index (right cbuffer)
                  3;                      // Output index (right cbuffer)
   .ENDBLOCK;

.endif // .ifdef ANALOGUE_NOISE_GATE_OFF

   // ** allocate memory for mono audio out cbops copy routine **

   .VAR $mono_audio_in_copy_struc[] =
         &$audio_in_shift_op_mono,
         1,     // number of inputs
         $AUDIO_LEFT_IN_PORT,
         1,     // number of outputs
.ifdef USES_PEQ
         &$peq_in_left_cbuffer_struc;
.else
         &$audio_in_left_cbuffer_struc;
.endif

   .BLOCK $audio_in_shift_op_mono;
         .VAR audio_in_shift_op_mono.next = &$audio_in_dc_remove_op_mono;
         .VAR audio_in_shift_op_mono.func = &$cbops.shift;
         .VAR audio_in_shift_op_mono.param[$cbops.shift.STRUC_SIZE] =
                  0,                      // Input index (left input port)
                  1,                      // Output index (left cbuffer)
                  8;                      // Shift amount
   .ENDBLOCK;

.ifdef ANALOGUE_NOISE_GATE_OFF

   .BLOCK $audio_in_dc_remove_op_mono;
         .VAR audio_in_dc_remove_op_mono.next = $cbops.NO_MORE_OPERATORS;
         .VAR audio_in_dc_remove_op_mono.func = &$cbops.dc_remove;
         .VAR audio_in_dc_remove_op_mono.param[$cbops.dc_remove.STRUC_SIZE] =
                  1,                      // Input index (left cbuffer)
                  1;                      // Output index (left cbuffer)
   .ENDBLOCK;
   
.else // .ifdef ANALOGUE_NOISE_GATE_OFF

   .BLOCK $audio_in_dc_remove_op_mono;
         .VAR audio_in_dc_remove_op_mono.next = &$audio_in_noise_gate_op_mono;
         .VAR audio_in_dc_remove_op_mono.func = &$cbops.dc_remove;
         .VAR audio_in_dc_remove_op_mono.param[$cbops.dc_remove.STRUC_SIZE] =
                  1,                      // Input index (left cbuffer)
                  1;                      // Output index (left cbuffer)
   .ENDBLOCK;

   .BLOCK $audio_in_noise_gate_op_mono;
         .VAR audio_in_noise_gate_op_mono.next = $cbops.NO_MORE_OPERATORS;
         .VAR audio_in_noise_gate_op_mono.func = &$cbops.noise_gate;
         .VAR audio_in_noise_gate_op_mono.param[$cbops.noise_gate.STRUC_SIZE] =
                  1,                      // Input index (left cbuffer)
                  1;                      // Output index (left cbuffer)
   .ENDBLOCK;
   
.endif //.ifdef ANALOGUE_NOISE_GATE_OFF 
 
   // memory definitions for SRC left and right history buffers
  .VAR/DM1CIRC $hist_left[$src.SRC_MAX_FILTER_LEN];
  .VAR/DM1CIRC $hist_right[$src.SRC_MAX_FILTER_LEN];
  
 

   // ** allocate memory for message handlers **
   .VAR/DM1 $mv.initialise_message_struc[$message.STRUC_SIZE]; 
   .VAR $mv.codec_type_message_struc[$message.STRUC_SIZE];  //receive codec type from VM
   .VAR $mv.change_mode_message_struc[$message.STRUC_SIZE]; //receive operating mode from vm
.ifdef SELECTED_ENCODER_SBC 
   .VAR $sbc_bitpool_message_struc[$message.STRUC_SIZE];    //SBC bitpool change message struct
   .VAR $set_low_bitpool_size_message_struc[$message.STRUC_SIZE]; // SBC set bitpool low value message sturct
.endif

   // ** allocate memory for av encoder structure **
   .VAR/DM1 $av_encoder_codec_stream_struc[$codec.av_encode.STRUC_SIZE] =
            SELECTED_CODEC_FRAME_ENCODE_FUNCTION,     // frame_encode function
            SELECTED_CODEC_RESET_ENCODER_FUNCTION,    // reset_encoder function
            &$codec_out_cbuffer_struc,                // out cbuffer
            &$audio_in_left_cbuffer_struc,            // in left cbuffer
            &$audio_in_right_cbuffer_struc;           // in right cbuffer
         
 
   .VAR $codec_reset_needed = 1;

    // ** allocate required variables **
   .VAR $mv.mode = $mv.mode.IDLE_MODE;
   
    // codec type is used to configure sbc encoder for fast stream
    // faststream in encoder side can be configured directly from vm by setting the bit 8th
    // of the setting word, however as we will add wbs this message would be required
   .VAR $mv.codec_type = -1;
   .VAR $vm.codec_type.timer_period =  $TMR_PERIOD_CODEC_COPY;
   .VAR $vm.codec_type.audio_timer_period =  $TMR_PERIOD_AUDIO_COPY; 
   .VAR $vm.codec_type.minimum_data_to_copy = $DEFAULT_MINIMIMUM_CODEC_DATA_TO_COPY; 
   
   // initialise the stack library
   call $stack.initialise;
   // initialise the interrupt library
   call $interrupt.initialise;
   // initialise the message library
   call $message.initialise;
   // initialise the cbuffer library
   call $cbuffer.initialise;
   // initialise the pulse_led library
   call $pulse_led.initialise;

   // PEQ initialization
.ifdef USES_PEQ
   r7 = $left_peq_struc;
   call $mv_dongle.peq.initialize;
   r7 = $right_peq_struc;
   call $mv_dongle.peq.initialize;
.endif
  
   // Initialise the codec library
   call SELECTED_CODEC_INITIALISE_ENCODER_FUNCTION;
   
  

   // rgister codec type message handler
   r1 = &$mv.codec_type_message_struc;
   r2 = $mv.message_from_vm.CODEC_TYPE;
   r3 = &$codec_type_message_handler;
   call $message.register_handler;
      

   .ifdef DEBUG_ON
      // initialise the profiler library
      call $profiler.initialise;
   .endif
   

   // *** register handlers to receive reset & change mode commands ***
   r1 = &$mv.change_mode_message_struc;
   r2 = $mv.message_from_vm.CHANGE_MODE;
   r3 = &$change_mode_message_handler;
   call $message.register_handler;
   
.ifdef SELECTED_ENCODER_SBC 
   // set up an interupt for SBC bitpool size change
   r1 = &$sbc_bitpool_message_struc;
   r2 = SBC_BITPOOL_CHANGE_MESSAGE;
   r3 = &$sbc_bitpool_handler;
   call $message.register_handler;
.endif

   // *** tell vm we're ready and wait for the go message ***
   call $message.send_ready_wait_for_go;
   
   // in vm side care has been taken that mode and codec type messages are sent before go message
   // this is just to make sure that all messages have been handled when reaching to this point
   call $timer.1ms_delay; 

      r0 = $AUDIO_RIGHT_IN_PORT;
      call $cbuffer.is_it_enabled;
      if NZ jump right_port_connected;
         // tell codec library that no right buffer
         M[$av_encoder_codec_stream_struc + $codec.stream_encode.IN_RIGHT_BUFFER_FIELD] = 0;
      right_port_connected:

      // left and right audio channels from the mmu have been synced to each other
      // by the vm app but are free running in that the dsp doesn't tell them to
      // start.  We need to make sure that our copying between the cbuffers and
      // the mmu buffers starts off in sync with respect to left and right
      // channels.  To do this we make sure that when we start the copying timers
      // that there is no chance of a buffer wrap around occuring within the timer
      // period.  The easiest way to do this is to start the timers just after a
      // buffer wrap around occurs.

      // wait for ADC buffers to have just wrapped around
      wait_for_adc_buffer_wraparound:
         r0 = $AUDIO_LEFT_IN_PORT;
         call $cbuffer.calc_amount_data;
         // if the amount of data in the buffer is less than 16 samples then a
         // buffer wrap around must have just ocurred.
         Null = r0 - 16;
      if POS jump wait_for_adc_buffer_wraparound;

      // start timer that copies input samples
      r1 = &$audio_in_timer_struc;
      r2 = M[$vm.codec_type.audio_timer_period];
        r3 = &$audio_in_copy_handler;
        call $timer.schedule_event_in;
      
  frame_loop:
      // running reset function if required
      Null = M[$codec_reset_needed];
      if Z jump no_codec_reset;
         r5 = &$av_encoder_codec_stream_struc;
         r0 = M[r5 + $codec.stream_encode.RESET_ADDR_FIELD];
         call r0;
         M[$codec_reset_needed] = 0;
      no_codec_reset:

      // encode a frame
      r5 = &$av_encoder_codec_stream_struc;
      call $codec.av_encode;
     // if for any reason (not enough input/output data/space) codec 
     // doesnt generate any output then wait one milisecond, so conditions to 
     // call the encoder would be met in next try
      r5 = &$av_encoder_codec_stream_struc;
      r0 = M[r5 + $codec.av_encode.MODE_FIELD];
      r0 = r0 - $codec.SUCCESS;   

      // all mode funtions return here (r0=0 if successful)
     Null = r0;
     // if not successful wait for a milisecond
     if NZ call $timer.1ms_delay;
  
  jump frame_loop;
.ENDMODULE;


// *********************************************************************************
// MODULE:
//    $codec_type_message_handler
//
// DESCRIPTION:
//   handler function for receiving codec type from vm and
//   configuring sbc codec for WBS or faststream if required
//
// INPUTS:
//     - r1 =  codec type
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r4
// **********************************************************************************
.MODULE $M.codec_type_message_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
    
    $codec_type_message_handler:
   M[$codec_type] = r1;
   .ifdef SELECTED_ENCODER_SBC 
      r0 = 0;  //no post function
      r2 = $TMR_PERIOD_CODEC_COPY; //normal sbc period
     r3 = $TMR_PERIOD_AUDIO_COPY; //normal sbc
     r4 = $DEFAULT_MINIMIMUM_CODEC_DATA_TO_COPY;                      
     Null = r1 - $mv.codec_type.FASTSTREAM_CODEC;  
     if NZ jump no_faststream;
     r0 = &$faststream.sbcenc_post_func;     //faststream codec copy period is 1ms 
      r2 = $FASTSTREAM_TMR_PERIOD_CODEC_COPY; //post function
     r3 = $FASTSTREAM_TMR_PERIOD_AUDIO_COPY;
     r4= $FASTSTREAM_MINIMUM_CODEC_DATA_TO_COPY;
   no_faststream:
     // update timer period for copying codec data
     M[$vm.codec_type.timer_period] = r2;
     
     // update timer period for copying audio data
     M[$vm.codec_type.audio_timer_period]=r3;
     
     // update post function
      M[&$sbcenc.pre_post_proc_struc + $codec.pre_post_proc.POST_PROC_FUNC_ADDR_FIELD] = r0;
     
     // update minimum codec data to copy to codec port 
     M[$vm.codec_type.minimum_data_to_copy] = r4;
   .endif
    rts;
.ENDMODULE;

// *********************************************************************************
// MODULE:
//    $codec_type_message_handler
//
// DESCRIPTION:
//   Mode change messege handler
//
// INPUTS:
//     - r1 = new mode ID
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    
// **********************************************************************************
.MODULE $M.change_mode_message_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $change_mode_message_handler:

   // store the new state
   M[$mv.mode] = r1;
   
   rts;

.ENDMODULE;
// *********************************************************************************
// MODULE:
//    $audio_in_copy_handler
//
// DESCRIPTION:
//   Copies ADC input ports into audio cbuffers in Analogue mode
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// **********************************************************************************
.MODULE $M.audio_in_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $audio_in_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // copy audio data from the port to the cbuffer
   // (selecting either mono or stereo)
   r8 = &$stereo_audio_in_copy_struc;
   r7 = &$mono_audio_in_copy_struc;
   r0 = $AUDIO_RIGHT_IN_PORT;
   call $cbuffer.is_it_enabled;
   if Z r8 = r7;

   // Copy data from the port(s) to the cbuffer(s)
   call $cbops.adc_av_copy;


   // Call the PEQ processing routine
.ifdef USES_PEQ
   call $mv_dongle.peq.caller;
.endif


   // post another timer event
   r1 = &$audio_in_timer_struc;
   r2 = M[$vm.codec_type.audio_timer_period];
   r3 = &$audio_in_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



// $****************************************************************************
// SBC bitpool change handler
// $****************************************************************************

.ifdef SELECTED_ENCODER_SBC 

.MODULE $M.sbc_bitpool_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $sbc_bitpool_handler:

   NULL = r1 - 30;
   if LT jump $error;
   NULL = r1 - 50;
   if GT jump $error;
   M[&$sbcenc.setting_bitpool] = r1;

   rts;

.ENDMODULE;


.endif



// $****************************************************************************
// NAME:
//    Audio Processing Library PEQ Module (version 2.0.0)
//
// DESCRIPTION:
//    Parametric Equalizer based on multi-stage biquad filter
//
// MODULES:
//    $mv_dongle.peq.initialize
//    $mv_dongle.peq.process
// *****************************************************************************

.ifdef USES_PEQ

.MODULE $M.mv_dongle.peq.initialize;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$mv_dongle.peq.initialize:

   //number of stages
   r0 = M[r7 + $mv_dongle.peq.NUM_STAGES_FIELD];   
   
   // size of delay buffer = (num_stage+1)*2
   r1 = r0 + 1;
   r1 = r1 ASHIFT 1;
   M[r7 + $mv_dongle.peq.DELAYLINE_SIZE_FIELD] = r1;

   // size of coef buffer = (num_stage) * 5
   r1 = r0 ASHIFT 2;
   r1 = r1 + r0;
   M[r7 + $mv_dongle.peq.COEFS_SIZE_FIELD] = r1;

   // Initialize delay buffer to zero
   r1 = M[r7 + $mv_dongle.peq.DELAYLINE_ADDR_FIELD];
   I0 = r1;
   r10 = M[r7 + $mv_dongle.peq.DELAYLINE_SIZE_FIELD];
   // to zero the delay buffer
   r0 = 0;   
   do init_dly_ln_loop;
      M[I0, 1] = r0;    
   init_dly_ln_loop:

   rts; 
.ENDMODULE;



.MODULE $M.mv_dongle.peq.caller;
   .CODESEGMENT   PM;
   .DATASEGMENT DM;

   $mv_dongle.peq.caller:

   $push_rLink_macro;

   // PEQ the left channel

   r7 = $left_peq_struc;
   
   r0 = &$peq_in_left_cbuffer_struc;
   call $cbuffer.get_read_address_and_size;  
   M[r7 + $mv_dongle.peq.INPUT_ADDR_FIELD] = r0;
   M[r7 + $mv_dongle.peq.INPUT_SIZE_FIELD] = r1;
   
   r0 = &$audio_in_left_cbuffer_struc;
   call $cbuffer.get_write_address_and_size;   
   M[r7 + $mv_dongle.peq.OUTPUT_ADDR_FIELD] = r0;
   M[r7 + $mv_dongle.peq.OUTPUT_SIZE_FIELD] = r1;
   
   r0 = &$peq_in_left_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   r4 = r0;
   r0 = $audio_in_left_cbuffer_struc;
   call $cbuffer.calc_amount_space;
   NULL = r0 - r4;
   if GT r0 = r4;

   // Keep each copy amount within a reasonable range
   NULL = r0 - $PEQ_COPY_MINIMUM;
   if LT jump right_channel;
   NULL = r0 - $PEQ_COPY_MAXIMUM;
   if LT jump skip_overwrite_1;
      r0 = $PEQ_COPY_MAXIMUM;
   skip_overwrite_1:
   
   M[r7 + $mv_dongle.peq.BLOCK_SIZE_FIELD] = r0;
   
   call $mv_dongle.peq.process;
   
   //r7 = $left_peq_struc;
   
   r1 = M[r7 + $mv_dongle.peq.INPUT_ADDR_FIELD];
   r0 = &$peq_in_left_cbuffer_struc;
   call $cbuffer.set_read_address;
   
   r1 = M[r7 + $mv_dongle.peq.OUTPUT_ADDR_FIELD];
   r0 = &$audio_in_left_cbuffer_struc;
   call $cbuffer.set_write_address;
   

   // PEQ the right channel
right_channel:
   r7 = $right_peq_struc;
   
   r0 = $peq_in_right_cbuffer_struc;
   call $cbuffer.get_read_address_and_size;  
   M[r7 + $mv_dongle.peq.INPUT_ADDR_FIELD] = r0;
   M[r7 + $mv_dongle.peq.INPUT_SIZE_FIELD] = r1;
   
   r0 = $audio_in_right_cbuffer_struc;
   call $cbuffer.get_write_address_and_size;   
   M[r7 + $mv_dongle.peq.OUTPUT_ADDR_FIELD] = r0;
   M[r7 + $mv_dongle.peq.OUTPUT_SIZE_FIELD] = r1;
   
   r0 = $peq_in_right_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   r4 = r0;
   r0 = $audio_in_right_cbuffer_struc;
   call $cbuffer.calc_amount_space;
   NULL = r0 - r4;
   if GT r0 = r4;
   
   // Keep each copy amount within a reasonable range
   NULL = r0 - $PEQ_COPY_MINIMUM;
   if LT jump all_done;
   NULL = r0 - $PEQ_COPY_MAXIMUM;
   if LT jump skip_overwrite_2;
      r0 = $PEQ_COPY_MAXIMUM;
   skip_overwrite_2:

   M[r7 + $mv_dongle.peq.BLOCK_SIZE_FIELD] = r0;
   
   call $mv_dongle.peq.process;

   r1 = M[r7 + $mv_dongle.peq.INPUT_ADDR_FIELD];
   r0 = &$peq_in_right_cbuffer_struc;
   call $cbuffer.set_read_address;
   
   r1 = M[r7 + $mv_dongle.peq.OUTPUT_ADDR_FIELD];
   r0 = &$audio_in_right_cbuffer_struc;
   call $cbuffer.set_write_address;

all_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;



.MODULE $M.mv_dongle.peq.process;
   .CODESEGMENT   PM;
   .DATASEGMENT DM;

$mv_dongle.peq.process:
    
   M1 = 1;
   I3 = r7;
   M0 = -1;
   // Read INPUT_ADDR_FIELD
   r0 = M[I3,M1];                                              
   // I4 = ptr to ip buffer,   Read INPUT_SIZE_FIELD
   I4 = r0, r0 = M[I3,M1];                                    
   // L4 = length of buffer,   Read OUTPUT_ADDR_FIELD
   L4 = r0, r0 = M[I3,M1];                                    
   // I0 = ptr to op buffer,   Read OUTPUT_SIZE_FIELD
   I0 = r0, r0 = M[I3,M1];                                    
   // L0 = length of buffer,   Read DELAYLINE_ADDR_FIELD
   L0 = r0, r0 = M[I3,M1];                                    
   // I5 = ptr to delay line,  Read COEFS_ADDR_FIELD
   I5 = r0, r0 = M[I3,M1];                                    
   // I1 = ptr to coefs buffer,Read NUM_STAGES_FIELD
   I1 = r0, r1 = M[I3,M1];                                    
   // M2 = num stages,        Read DELAYLINE_SIZE_FIELD
   M2 = r1, r1 = M[I3,M1];                           
   // L5 = delay buffer size, Read COEFS_SIZE_FIELD
   L5 = r1, r1 = M[I3,M1];                                    
   // L1 = coeff buffer size, Read BLOCK_SIZE_FIELD   
   L1 = r1, r4 = M[I3,M1];                           
   // Read SCALING_ADDR_FIELD
   r10 = Null, r1 = M[I3,M1];                                           
   // I2 = scale buffer,      Read GAIN_EXPONENT_ADDR_FIELD
   I2 = r1 + M2, r1 = M[I3,M1];                      
   // needed for bug in index feed forward, M2 = -num stages
   M2 = Null - M2;  
   // INPUT_GAIN_EXPONENT
   r6 = M[r1];                                     
   // Add 2-bit head room     Read GAIN_MANTISA_ADDR_FIELD
   r6 = r6 + M0, r5 = M[I3,M1];                      
   r6 = r6 + M0;                       
   // INPUT_GAIN_MANTISA   
   r5 = M[r5];                                     
   // this loop excutes for each sample in the block
peq_block_loop:                                             
      // get new input sample
      // number of Biquad stages used, get new input sample
      r10 = r10 - M2, r0 = M[I4,M1];
      // Apply mantisa,Exp to front end gain
      rMAC = r0 * r5, r0 = M[I2,M2];
      r0 = rMAC ASHIFT r6;
      do biquad_loop;
         // get x(n-2), get coef b2
         r1 = M[I5,M1], r2 = M[I1,M1];                            
         // b2*x(n-2), get x(n-1), get coef b1
         rMAC = r1 * r2, r1 = M[I5,M0], r2 = M[I1,M1];             
         // +b1*x(n-1), store new x(n-2), get coef b0
         rMAC = rMAC + r1 * r2, M[I5,M1] = r1, r2 = M[I1,M1];      
         // +b0*x(n),store new x(n-1)
         rMAC = rMAC + r0 * r2, M[I5,M1] = r0;                    
         // get y(n-2), get coef a2
         r1 = M[I5,M1], r2 = M[I1,M1];                           
         // -a2*y(n-2), get y(n-1), get coef a1
         rMAC = rMAC - r1 * r2, r1 = M[I5,M0], r2 = M[I1,M1];        
         // -a1*y(n-1),get the scalefactor
         rMAC = rMAC - r1 * r2, r3 = M[I2,M1];                    
         // get y(n)
         r0 = rMAC ASHIFT r3;                                     
biquad_loop:
      // store new y(n-2)
      M[I5,M1] = r1;
      // store new y(n-1)
      M[I5,M1] = r0;                                         
      // Restore Head room
      r0 = r0 ASHIFT 2;                            
      // Decrement the block counter,write back o/p sample
      r4 = r4 - M1,  M[I0,M1] = r0;                           
   if NZ jump peq_block_loop;
      
   // Update the I/O buffer pointers back into PEQ data object before leaving.
   L0 = Null;
   L4 = Null;
   L1 = Null; 
   L5 = Null;
   r0 = I4;
   // Update the input buffer pointer field
   M[r7 + $mv_dongle.peq.INPUT_ADDR_FIELD] = r0;    
   r0 = I0;
   // Update the output buffer pointer field
   M[r7 + $mv_dongle.peq.OUTPUT_ADDR_FIELD] = r0;   
    
   rts; 

.ENDMODULE;

.endif