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
.define $TMR_PERIOD_USB_COPY                 625    // timer period to read the USB port data
.define $TMR_PERIOD_AUDIO_COPY               1000   // timer period to read the audio data in analogue mode (for MP3 and SBC)
.define $FASTSTREAM_TMR_PERIOD_AUDIO_COPY    500    // timer period to read the audio data in analogue mode (for faststream only)
.define $TMR_PERIOD_USB_OUT_AUDIO_COPY       1000   // timer period to write audio data into USB port
.define $TMR_PERIOD_CODEC_COPY               8000   // timer period to write codec output to codec port (for MP3 and SBC)
.define $FASTSTREAM_TMR_PERIOD_CODEC_COPY    1000   // timer period to write codec output to codec port (for faststream only)
.define $TMR_PERIOD_BUFFER_STATE             100000 // timer period to check USB input/output activity, and sending vm a message if required
 
.define $PEQ_COPY_MINIMUM                    25  // based on 1000 audio copy timer period
.define $PEQ_COPY_MAXIMUM                    50  // based on 1000 audio copy timer period

.define $BLOCK_SIZE                    16
.define $RESAMPLER_READ_BACK           50
.define $USB_SAMPLE_RATE 48000
.define $DEFAULT_MINIMIMUM_CODEC_DATA_TO_COPY 1
.define $FASTSTREAM_MINIMUM_CODEC_DATA_TO_COPY 100 //this limit is for faststream copying codec data into the port (slightly less than 3 frames = 3*36=108 words
.define $USB_STALL_TIME_BEFOR_SINLENC_INSERTION 5  // minimum no activity time(in ms) in usb in port before sending silence 

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

.define $PAUSE_SILENCE_DURATION_TO_SEND ($PAUSE_SILENCE_DURATION_TO_SEND_MS*1000/$TMR_PERIOD_USB_COPY)
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
   .CONST  $SCO_IN_PORT       ($cbuffer.READ_PORT_MASK  + $cbuffer.FORCE_PCM_AUDIO + 1);
   .CONST  $SCO_OUT_PORT      ($cbuffer.WRITE_PORT_MASK + $cbuffer.FORCE_PCM_AUDIO + 0);

   .CONST  $USB_IN_PORT       ($cbuffer.READ_PORT_MASK  + 0);
   .CONST  $USB_OUT_PORT      ($cbuffer.WRITE_PORT_MASK + 1);
   
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

   // ** $usb_out_resample is used to upsample SCO input (from 8khz to 48khz) before copying to $USB_OUT_PORT **
   declare_cbuf_and_struc($usb_out_resample, $usb_out_resample_cbuffer_struc, SCO_USB_CBUFFER_SIZE)

   // ** defining cbuffers for resampled input audio **
   // ** these cbuffers are used as final input to codec when sampling rate is not 48000 **
   declare_cbuf_and_struc($audio_in_left_resample,    $audio_in_left_resample_cbuffer_struc,    AUDIO_CBUFFER_SIZE)
   declare_cbuf_and_struc($audio_in_right_resample,   $audio_in_right_resample_cbuffer_struc,   AUDIO_CBUFFER_SIZE)
   
   // ** defining cbuffers for codec output **
   declare_cbuf_and_struc($codec_out,        $codec_out_cbuffer_struc,        CODEC_CBUFFER_SIZE)
   

  // allocate memory for filter coefficients
  // this definition moved to here from $src module, the reason is to share its memory with cbuffesr
  // circular definition is not necessary for source coefss, however to be shared with cbuffers it must be circular
  .VAR/DM2CIRC $src.coeffs[$src.SRC_MAX_UPSAMPLE_RATE*$src.SRC_MAX_FILTER_LEN];
  

  // ** $audio_in_mix_cbuffer_struc is used to mix L/R channels in SCO mode before copying them into  $SCO_OUT_PORT **
  // definition commented, $audio_in_mix now shares memory with $src.coeffs 
  // declare_cbuf_and_struc($audio_in_mix,     $audio_in_mix_cbuffer_struc,     AUDIO_CBUFFER_SIZE)
  .define $audio_in_mix $src.coeffs
  .VAR $audio_in_mix_cbuffer_struc[$cbuffer.STRUC_SIZE] = 
            SCO_USB_CBUFFER_SIZE, 
            &$audio_in_mix,
            &$audio_in_mix;

   // ** some extra alias cbuffer definitions for those can share actual memory (are not used in the same mode) **
   // all data must be purged when switching mode happens
  .define $usb_in_cbuffer_struc  $audio_in_left_resample_cbuffer_struc       
  .define $usb_out_cbuffer_struc $audio_in_right_resample_cbuffer_struc
  .define $sco_in_cbuffer_struc  $usb_out_cbuffer_struc
  .define $sco_out_cbuffer_struc $usb_in_cbuffer_struc
  
   // ** allocate memory for timer structures **
   .VAR $audio_copy_timer_struc[$timer.STRUC_SIZE];         /* DSP -> $USB_OUT_PORT */
   .VAR $av_copy_timer_struc[$timer.STRUC_SIZE];            /* Codec output -> $CODEC_OUT_PORT */
   .VAR $audio_in_timer_struc[$timer.STRUC_SIZE];           /* Analogue input -> DSP */
   .VAR $buffer_state_handler_timer_struc[$timer.STRUC_SIZE];  /* monitor USB and possibly sending vm a message */
   .VAR $usb_copy_timer_struc[$timer.STRUC_SIZE];              /* $USB_IN_PORT -> DSP */

    
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



   /* SCO in copy from port to SCO in cbuffer */
   .VAR $sco_in_copy_struc[] =
             &$sco_in_limit_op,
             1,
             $SCO_IN_PORT,
             1,
             &$sco_in_cbuffer_struc;

    .BLOCK $sco_in_limit_op;
      .VAR $sco_in_limit_op.next = &$sco_in_shift_op;
      .VAR $sco_in_limit_op.func = &$cbops.limited_copy;
      .VAR $sco_in_limit_op.mono[$cbops.limited_copy.STRUC_SIZE] =
         $SCO_IN_LIMIT,
         $cbops.limited_copy.NO_WRITE_LIMIT;
   .ENDBLOCK;

   .BLOCK $sco_in_shift_op;
      .VAR sco_in_shift_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR sco_in_shift_op.func = &$cbops.shift;
      .VAR sco_in_shift_op.param[$cbops.shift.STRUC_SIZE] =
             0,
             1,
             8;
   .ENDBLOCK;


   /* SCO out copy from cbuffer to SCO out port */
   .VAR $sco_out_copy_struc[] =
            &$sco_out_limit,
            1,
            &$sco_out_cbuffer_struc,
            1,
            $SCO_OUT_PORT;

   .BLOCK $sco_out_limit;
      .VAR $sco_out_limit.next = &$sco_out_shift_op;
      .VAR $sco_out_limit.func = &$cbops.limited_copy;
      .VAR $sco_out_limit.mono[$cbops.limited_copy.STRUC_SIZE] =
         $cbops.limited_copy.NO_READ_LIMIT,
         $SCO_OUT_LIMIT;
   .ENDBLOCK;

   .BLOCK $sco_out_shift_op;
      .VAR sco_out_shift_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR sco_out_shift_op.func = &$cbops.shift;
      .VAR sco_out_shift_op.param[$cbops.shift.STRUC_SIZE] =
            0,
            1,
            -8;
   .ENDBLOCK;

      
   /* USB in copy operator stucture */

   .VAR/DM1 $usb_audio_in_copy_struc[$mvdongle.STEREO_COPY_STRUC_SIZE] =
            $USB_IN_PORT,                    // $USB_IN.STEREO_COPY_SOURCE_FIELD
.ifdef USES_PEQ
            &$peq_in_left_cbuffer_struc,   // $USB_IN.STEREO_COPY_LEFT_SINK_FIELD
            &$peq_in_right_cbuffer_struc,  // $USB_IN.STEREO_COPY_RIGHT_SINK_FIELD
.else
            &$audio_in_left_cbuffer_struc,   // $USB_IN.STEREO_COPY_LEFT_SINK_FIELD
            &$audio_in_right_cbuffer_struc,  // $USB_IN.STEREO_COPY_RIGHT_SINK_FIELD
.endif
            $USB_SAMPLE_RATE/250,            // $USB_IN.STEREO_COPY_PACKET_LENGTH_FIELD
            8;                               // $USB_IN.STEREO_COPY_SHIFT_AMOUNT_FIELD

             
            
    /* USB out copy operator stucture */
   .VAR $usb_audio_out_copy_struc[] =
            &$usb_out_shift_op,
            1,
            &$usb_out_resample_cbuffer_struc,
            1,
            $USB_OUT_PORT;
   .BLOCK $usb_out_shift_op;
      .VAR usb_out_shift_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR usb_out_shift_op.func = &$cbops.shift;
      .VAR usb_out_shift_op.param[$cbops.shift.STRUC_SIZE] =
            0,
            1,
            -8;
   .ENDBLOCK;


   /* Operator to mix the input streams together */
   .VAR $usb_audio_in_mix_copy_struc[] =
            &$audio_in_mix_op,
            2,
            &$audio_in_left_cbuffer_struc,
            &$audio_in_right_cbuffer_struc,
            1,
            &$audio_in_mix_cbuffer_struc;
   .BLOCK $audio_in_mix_op;
      .VAR $audio_in_mix_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $audio_in_mix_op.func = &$cbops.two_to_one_chan_copy;
      .VAR $audio_in_mix_op.param[$cbops.two_to_one_chan_copy.STRUC_SIZE] =
            0,                            // Input index (left)
            1,                            // Input index (right)
            2,                            // Output index
            1.0,                          // Left gain
            0.0;                          // Right gain
   .ENDBLOCK;

    // ** defining resample structure and history buffer for downsampling from 48khz to 8khz **
   .VAR/DM1CIRC $hist_8kdown[120];
   .VAR $sco_audio_in_downsample_struct[$src.upsample_downsample.STRUC_SIZE] = 
         &$audio_in_mix_cbuffer_struc,         //left channel input
         0,                                 // no right channel input
         &$usb_in_cbuffer_struc,                   // left channel output 
            0,                              // no right channel output
         &$coeffs_8kdown,                          // anti-aliasing filter coefficients 
         &$hist_8kdown,                            // left input history
         0,                                        // no right input history
         120,                                      // (size of coeffs)/L
         6,                                        // decimation factor (M)
         1,                                        // pre upsampling factor (L)
         6,                                        // Int(M/L)
         0.0,                                      // Frac(M/L)
         1.0/6.0;                                  // 1.0/M.0
         
   // ** defining resample structure and history buffer for upsampling from 8khz to 48khz **
   .VAR/DM1CIRC $hist_8kup[20];
   .VAR $sco_audio_out_upsample_struct[$src.upsample_downsample.STRUC_SIZE] = 
         &$usb_out_cbuffer_struc,           // left channel input
         0,                              // no right channel input
         &$usb_out_resample_cbuffer_struc,     // left channel output 
            0,                           // no right channel output
         &$coeffs_8kup,                        // anti-aliasing filter coefficients (must be in DM2)
         &$hist_8kup,                          // left input history
         0,                                    // no right input history
         20,                                   // (size of coeffs)/L 
         1,                                    // decimation factor (M)
         6,                           // pre upsampling factor (L)
         0,                           // Int(M/L)                  
         1.0/6.0,                     // Frac(M/L)
         1.0;                         // 1.0/M.0
         


   // ** allocate memory for codec output cbops copy routine **
   .VAR $codec_out_copy_struc[] =
            &$codec_out_copy_op,                // first operator block
            1,                                  // number of inputs
            &$codec_out_cbuffer_struc,          // input
            1,                                  // number of outputs
            $CODEC_OUT_PORT;                    // output
   .BLOCK $codec_out_copy_op;
      .VAR $codec_out_copy_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $codec_out_copy_op.func = &$cbops.copy_op;
      .VAR $codec_out_copy_op.param[$cbops.copy_op.STRUC_SIZE] =
               0,                         // Input index
               1;                         // Output index
   .ENDBLOCK;
   
   .VAR $codec_out_copy_two_struc[] =
            &$codec_out_copy_two_op,            // first operator block
            1,                                  // number of inputs
            &$codec_out_cbuffer_struc,          // input
            1,                                  // number of outputs
            $CODEC_OUT_PORT_TWO;                // output
   .BLOCK $codec_out_copy_two_op;
      .VAR $codec_out_copy_two_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $codec_out_copy_two_op.func = &$cbops.copy_op;
      .VAR $codec_out_copy_two_op.param[$cbops.copy_op.STRUC_SIZE] =
               0,                         // Input index
               1;                         // Output index
   .ENDBLOCK;


// *** Dual Stream ***
   .VAR $ds_codec_out_copy_one_struc[] =
            &$ds_codec_out_copy_one_op,                // first operator block
            1,                                  // number of inputs
            &$codec_out_cbuffer_struc,          // input
            1,                                  // number of outputs
            $CODEC_OUT_PORT;                    // output
   .BLOCK $ds_codec_out_copy_one_op;
      .VAR $ds_codec_out_copy_one_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $ds_codec_out_copy_one_op.func = &$cbops.copy_op;
      .VAR $ds_codec_out_copy_one_op.param[$cbops.copy_op.STRUC_SIZE] =
            0,                         // Input index
            1;                         // Output index
   .ENDBLOCK;
   
   .VAR $ds_codec_out_copy_two_struc[] =
            &$ds_codec_out_copy_two_op,                // first operator block
            1,                                  // number of inputs
            &$codec_out_cbuffer_struc,          // input
            1,                                  // number of outputs
            $CODEC_OUT_PORT_TWO;                    // output
   .BLOCK $ds_codec_out_copy_two_op;
      .VAR $ds_codec_out_copy_two_op.next = $cbops.NO_MORE_OPERATORS;
      .VAR $ds_codec_out_copy_two_op.func = &$cbops.copy_op;
      .VAR $ds_codec_out_copy_two_op.param[$cbops.copy_op.STRUC_SIZE] =
            0,                         // Input index
            1;                         // Output index
   .ENDBLOCK;
// *** *** *** ***



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
  
 

  .VAR $audio_in_src_struct[$src.upsample_downsample.STRUC_SIZE] = 
         &$audio_in_left_cbuffer_struc,           //left channel input
         &$audio_in_right_cbuffer_struc,           //right channel input
         &$audio_in_left_resample_cbuffer_struc,   //left channel output 
         &$audio_in_right_resample_cbuffer_struc,  //right channel output
         &$src.coeffs,                   // anti-aliasing filter coefficients
         &$hist_left,                    // left input history
         &$hist_right                    // right input history
         ;                       //rest of the structure is configured by $setup_downsampler
      


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
   
   r1 = &$set_low_bitpool_size_message_struc;
   r2 = SBC_BITPOOL_SET_LOW_MESSAGE;
   r3 = &$sbc_set_low_bitpool_size;
   call $message.register_handler;
.endif

   // *** tell vm we're ready and wait for the go message ***
   call $message.send_ready_wait_for_go;
   
   // in vm side care has been taken that mode and codec type messages are sent before go message
   // this is just to make sure that all messages have been handled when reaching to this point
   call $timer.1ms_delay; 

   /* Start the timer handlers */
   r0 = M[$mv.mode];
   Null = r0 - $mv.mode.ANALOG_MODE;
   
   if NZ jump no_analog_source_mode;
     
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
      
      jump codec_copy_timer_handler;

no_analog_source_mode:


   
   // post a timer event for the audio copy routines to USB output port
   r1 = &$audio_copy_timer_struc;
   r2 = $TMR_PERIOD_USB_OUT_AUDIO_COPY;
   r3 = &$audio_copy_handler;
   call $timer.schedule_event_in;


   // post a timer event for the buffer state monitor routine
   r1 = &$buffer_state_handler_timer_struc;
   r2 = $TMR_PERIOD_BUFFER_STATE;
   r3 = &$buffer_state_handler;
   call $timer.schedule_event_in;
   
   // post a timer event for the audio copy routines from USB input port
   r1 = &$usb_copy_timer_struc;
   r2 = $TMR_PERIOD_USB_COPY;
   r3 = &$usb_copy_handler;
   call $timer.schedule_event_in;
   
codec_copy_timer_handler:

   // post a timer event for copying codec output to CODEC output port
   r1 = &$av_copy_timer_struc;
   r2 = M[$vm.codec_type.timer_period];
   r3 = &$av_copy_handler;
   call $timer.schedule_event_in;

   // *** frame loops ***
  .VAR running_mode = -1;
  .VAR running_mode_function;
  .VAR running_mode_function_table [] = -1,              //idle mode: no background function 
                              &av_function,        //av_mode: src+encoder
                              &analogue_function,     //analogue: encoder only
                              -1,                     //sco mode: no background function 
                              -1;  
  frame_loop:
      
      // get the latest mode that received from vm
      r0 = M[$mv.mode];
     
     // if running mode is not the same as latest one ..
      Null = r0 - M[running_mode];
      
      // update running mode 
      if NZ call change_mode;
     
      // jump to the appropriate function
      r0 = M[running_mode_function];
     if GT jump r0;
   mode_return:
      // all mode funtions return here (r0=0 if successful)
     Null = r0;
     // if not successful wait for a milisecond
     if NZ call $timer.1ms_delay;
  
  jump frame_loop;


// ** funtion that runs for av mode
av_function:
      // check for sampling frequency change
     call SELECTED_ENCODER_GETFS_FUNCTION;
     r1 = M[$codec_sample_rate];
     M[$codec_sample_rate] = r0;
     Null = r1 - r0;
    
      // normally called once right at the beginiing if sample rate is not 48000
     if NZ call $setup_downsampler; 
       
     // running resampler function, automatically configured in USB mode
     // 48000 -> no resampler
     // 32, 24 and 16khz -> $src.upsample_downsample
     // 44.1 and 22.05khz -> $src.fractional_downsample
      r8 = $audio_in_src_struct;
     r0 = M[$resample_function];
     if NZ call r0;
     
     // from this point analogue and av mode share code
 analogue_function:
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
     
jump mode_return;


// ** funtion that runs for av mode
  change_mode:
   $push_rLink_macro;
   // reset the codec (effective in analogue and av modes)
   r0 = 1;
   M[$codec_reset_needed] = r0;
   
   //update the running mode
   r0 = M[$mv.mode];
   M[running_mode] = r0;
   
   // update running function
   r0 = M[r0 + running_mode_function_table];
   M[running_mode_function] = r0;
   
   // as some cbuffers changes functionality when mode changes
   // it is required to purge all data before starting new mode
   
   // first block interrupts, this prevent ISR to alter R/W pointer while purging is done
   call $block_interrupts;   
   
   //purge $sco_in cbuffer data
   r0 = $sco_in_cbuffer_struc;
   r1 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
   
   //purge $sco_out cbuffer data
   r0 = $sco_out_cbuffer_struc;
   r1 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
   
   //purge $audio_in_mix_cbuffer_struc
   r0 = $audio_in_mix_cbuffer_struc;
   r1 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
   
   //reset downsampler (this is necessary if $audio_in_mix and $src.coeffs share physical memory)
   M[$codec_sample_rate] = 0;

   // default:no silence to insert
   r5 =$PAUSE_SILENCE_DURATION_TO_SEND;
   M[$usb_audio_in_copy_struc + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD] = r5;  

   // unblock interrupts
   call $unblock_interrupts;
   
 jump $pop_rLink_and_rts;


.ENDMODULE;



// *********************************************************************************
// MODULE:
//    $setup_downsampler
//
// DESCRIPTION:
//   configure resample structure based on desired output rate, this fuction 
//   configure the application to use either integer or fractional resample
//   function, it also loads the relevant coeffs from flash, it is called when the
//   codec sampling rate changes and normally run once
//
// INPUTS:
//     - r0 =  output sampling rate
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r3, r4, r5, I1, I0, r10, DoLoop
//
// NOTES:
//    -Aassumes that USB input sampling rate is 48000hz
//   - Should be called after codec has been configured to avoide calling twice
// **********************************************************************************
.MODULE $M.setup_downsampler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

$setup_downsampler:
      
      // defining all supported output sample rates 
      .VAR supported_freqs[] = 44100, 32000, 22050, 24000, 16000;
     
      // defining pointer to downsampler function
     .VAR $resample_function;
      
       // table holding coeff adressed in the flash
     .VAR lpf_coeff_table[] = &$src.SrcCoeff44K1, &$src.SrcCoeff32K, &$src.SrcCoeff22K05, &$src.SrcCoeff24K, &$src.SrcCoeff16K;
     
       // look-up table contatining all data needed for downsampler to operate for appropriate sample rate 
      .VAR src_operator_lookuptable [] =  
        //fiter length     //down_rate/r_out       uprate           Int(R)   Frac(R)                     1/DOWNRATE  
        $src.SrcCoeff44K1.SIZE,  147,        $src.SrcCoeff44K1.UPRATE,  1,   (48000.0/44100.0 - 1.0)+1e-7, 44100.0/48000.0, //44100   
        $src.SrcCoeff32K.SIZE,   3,          $src.SrcCoeff32K.UPRATE,   1,   (48000.0/32000.0 - 1.0)+1e-7, 1.0/3.0,         //32000
        $src.SrcCoeff22K05.SIZE, 147,        $src.SrcCoeff22K05.UPRATE, 2,   (48000.0/22050.0 - 2.0)+1e-7, 22050.0/48000.0, //22050
        $src.SrcCoeff24K.SIZE,   2,          $src.SrcCoeff24K.UPRATE,   2,   0,                        1.0/2.0,       //24000
        $src.SrcCoeff16K.SIZE,   3,          $src.SrcCoeff16K.UPRATE,   3,   0,                       1.0/3.0;         //16000

      
      $push_rLink_macro;
     default:
     
     // no src by default
     r1 = 0;                  
     // set default inbut cbuffers for codec
     r2 = &$audio_in_left_cbuffer_struc; //
     r3 = &$audio_in_right_cbuffer_struc;
     // if 48Khz then no downsampling and default
     Null = r0 - $USB_SAMPLE_RATE;
      if Z jump set_downsample; 
     
     // find the frequency in the table 
     M0 = 1;
     I0 = &supported_freqs;
     r1 = M[I0, M0];
     r10 = 5;
     r2 = 0;
      do find_fs_loop;
        Null = r0 - r1, r1 = M[I0, M0];
       if Z jump fs_found;
       r2 = r2 + 1;
     find_fs_loop:    
     
      //   not found, dont do downsampling
     r0 = $USB_SAMPLE_RATE;
     jump default;

     fs_found: 
     // r2 = fs_index     
      r1 = r2* 6(int);     
      I0 = r1 + src_operator_lookuptable;
     
      // config the interpolator now
     r10 = 6;
     I1 = $audio_in_src_struct + $src.upsample_downsample.COEFFSIZE_FIELD;
     do config_op;
      r0 = M[I0, 1];
     M[I1, 1] = r0;
     config_op:
     
      // load coeffs from flash
     r0 = M[lpf_coeff_table + r2];
      r1 = M[$audio_in_src_struct + $src.upsample_downsample.COEFFSIZE_FIELD];
     r2 = M[$audio_in_src_struct + $src.upsample_downsample.UPSAMPLE_RATE_FIELD];
     // size of filter must be coeff_size*uprate
     r1 = r1*r2 (int);
      I0 = &$src.coeffs;
      r2 = M[$flash.data_app.address];
      call $flash.copy_to_dm_32_to_24;
     
      // decide fractional or integer downsampling
      r1 = &$src.upsample_downsample; //integer resampling
      r0 = M[$audio_in_src_struct + $src.upsample_downsample.DECIMATION_RATE_FIELD];
     Null = r0 - 4; // if downrate field is too big then it probably is a fractional downsampling
     if NEG jump no_fractional;
     r1 = &$src.fractional_downsample; //fractinal downsampling
     M[$audio_in_src_struct + $src.upsample_downsample.OUT_COUNTER] = r0;    
     r0 = -1.0;
     M[$audio_in_src_struct + $src.upsample_downsample.RF] = r0;
   no_fractional:  
     
     // set the codec to use resample buffers as input
     r2 = &$audio_in_left_resample_cbuffer_struc;
     r3 = &$audio_in_right_resample_cbuffer_struc;
     
set_downsample:
      // configure the resample function, also codec input buffers
     M[$resample_function] = r1;
     M[$av_encoder_codec_stream_struc + $codec.stream_encode.IN_LEFT_BUFFER_FIELD] = r2;
     M[$av_encoder_codec_stream_struc + $codec.stream_encode.IN_RIGHT_BUFFER_FIELD] = r3;
     
     jump $pop_rLink_and_rts;
    
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
//    $av_copy_handler
//
// DESCRIPTION:
//   Copies codec output into CODEC output port
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// **********************************************************************************
.MODULE $M.av_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $av_copy_handler:
   
.ifdef DEBUG_ON   
    // debug info to measure average bitrate over the air
   .VAR $time_elapsed;
   .VAR $total_bytes_sent[2]=0,0; //48 bit
   // average bitrate(kbps) = ($total_bytes_sent * 8000) / ($time_elapsed*$vm.codec_type.timer_period)
.endif

   // push rLink onto stack
   $push_rLink_macro;
   
 .ifdef DEBUG_ON   
   M[$cbops.amount_to_use] = 0;
 .endif
 
   // copy data from cbuffer to the port
   //r8 = &$codec_out_copy_struc;
   
   // conditional copy is to cover faststream 
   // in fast stream timer period = 1ms, we copy only when at least 108 words of data is available to copy
   // this condition must not be used together with old codec library, as it might cause deadlock
   r0 = M[$av_encoder_codec_stream_struc + $codec.av_encode.OUT_BUFFER_FIELD];
   call $cbuffer.calc_amount_data;
   NULL = r0 - M[$vm.codec_type.minimum_data_to_copy];
   if LT jump no_codec_copy;
 
     
     
      .define SINGLE_STREAM_MODE    0
      .define DUAL_STREAM_MODE      1
      .VAR last_stream_state = -1;
     
     
      r2 = 1;
      r0 = $CODEC_OUT_PORT;
      call $cbuffer.is_it_enabled;
      // NULL = r0;
      if Z r2 = r2 - r2;
      r0 = $CODEC_OUT_PORT_TWO;
      call $cbuffer.is_it_enabled;
      // NULL = r0;
      if NZ r2 = r2 + r2;
      NULL = r2 - 2;
      if EQ jump dual_stream_codec_copy;  
      
      // Single stream copy, this single stream can be the first one or second one.

         r1 = M[&last_stream_state];
         NULL = r1 - DUAL_STREAM_MODE;
         if NZ jump not_from_ds_mode;

         // Need to handle the transition from two streams to one stream.
         r1 = M[ds_codec_out_one_read_prt];
         r2 = M[ds_codec_out_two_read_prt];
         // r0 == second port enable
         NULL = r0;
         if NZ r1 = r2;
         r0 = &$codec_out_cbuffer_struc;
         call $cbuffer.set_read_address;  // update the read pointer
         
                 
      not_from_ds_mode:
         r8 = &$codec_out_copy_struc;
         r7 = &$codec_out_copy_two_struc;
         r0 = $CODEC_OUT_PORT_TWO;
         call $cbuffer.is_it_enabled;
         if NZ r8 = r7;
         call $cbops.copy;

         r0 = SINGLE_STREAM_MODE;
         M[&last_stream_state] = r0;
         jump no_codec_copy_with_bitpool_control;


     // ************* Dual Stream ****************
     // Copy codec out data, handle dual stream special case
      
      dual_stream_codec_copy:
      
       .VAR ds_codec_out_one_read_prt = 0;
       .VAR ds_codec_out_two_read_prt = 0;
       
       .VAR $ds_codec_out_gap_threshold = 500;
       
       

       r1 = M[&last_stream_state];
       r0 = DUAL_STREAM_MODE;
       M[&last_stream_state] = r0;
       
       // *** if first time enter dual stream mode, setup read pointer first
       NULL = r1 - DUAL_STREAM_MODE;
       if Z jump already_in_dual_stream;
       
       // First time into dual stream mode
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.get_read_address_and_size;
       M[&ds_codec_out_two_read_prt] = r0;
       M[&ds_codec_out_one_read_prt] = r0;
       
       
    already_in_dual_stream:


   // Daul Stream free run, read points can drift away. 
   dual_free_run_mode:

       
       r8 = &$ds_codec_out_copy_one_struc;
       r1 = M[&ds_codec_out_one_read_prt];
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.set_read_address;
       call $cbops.copy;
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.get_read_address_and_size;
       M[&ds_codec_out_one_read_prt] = r0;

       r8 = &$ds_codec_out_copy_two_struc;
       r1 = M[&ds_codec_out_two_read_prt];
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.set_read_address;
       call $cbops.copy;
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.get_read_address_and_size;
       M[&ds_codec_out_two_read_prt] = r0;
       
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.get_write_address_and_size;
       // r0 = write address
       // r1 = buffer size
       r4 = M[&ds_codec_out_one_read_prt];
       r5 = M[&ds_codec_out_two_read_prt];
       
       r2 = r0 - r4;
       if NEG r2 = r2 + r1;  // r2 == stream one copy amount
       r3 = r0 - r5;
       if NEG r3 = r3 + r1;  // r3 == stream two copy amount
       r1 = r2 - r3;
       if NEG r4 = r5;
       // r4 = tail read pointer address
       // r1 = the gap between two read pointers
       
       
//.VAR $current_read_pointer_gap = 0;   // debug use
//M[$current_read_pointer_gap] = r1;    // debug use
       
       
       // *** force the slow read pointer to keep up if the gap becomes too large
       r2 = M[&$ds_codec_out_gap_threshold];
       NULL = r1;
       if NEG jump output_one_faster;
          // codec out two is faster (read pointer is closer to write pointer)
          NULL = r1 - r2;
          if LT jump update_combined_read_pointer;
          r4 = M[&ds_codec_out_two_read_prt];
          M[&ds_codec_out_one_read_prt] = r4;
          jump update_combined_read_pointer;
       
       output_one_faster:
          // codec out one is faster
          NULL = r2 + r1;
          if POS jump update_combined_read_pointer;
          r4 = M[&ds_codec_out_one_read_prt];
          M[&ds_codec_out_two_read_prt] = r4;
       
       
    update_combined_read_pointer:
       r1 = r4;
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.set_read_address;
       

       // Adaptive SBC bitpool for handling poor connection
       
no_codec_copy_with_bitpool_control:

.ifdef SELECTED_ENCODER_SBC 

       .define BITPOOL_DOWNGRADE_TH 300
       .define BITPOOL_RESTORE_TH   180
       .define BITPOOL_DELAY_PERIOD 3000
       .define BITPOOL_RST_PERIOD   300
       

       .VAR link_qua_sta = 0;  // 0 - normal quality, 1 - low bitrate quality
       .VAR org_bitpool_size = 0;
       .VAR delay_counter = 0;
       .VAR $low_bitpool_size = 30;
       
       r0 = M[&delay_counter];
       if Z jump normal_routine;
          // within delay time, do no checking
          r0 = r0 - 1;
          M[&delay_counter] = r0;
          jump no_codec_copy;

    normal_routine:
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.calc_amount_space;
       NULL = r0 - BITPOOL_DOWNGRADE_TH;
       if LT jump downgrade_link;
       
       r0 = &$codec_out_cbuffer_struc;
       call $cbuffer.calc_amount_data;
       NULL = r0 - BITPOOL_RESTORE_TH;
       if LT jump restore_link;
       jump no_codec_copy;
       
    downgrade_link:
       NULL = M[&link_qua_sta];
       if NZ jump no_codec_copy;  // already in low bitrate quality

       r0 = M[&$sbcenc.setting_bitpool];
       M[&org_bitpool_size] = r0;
       r0 = M[$low_bitpool_size]; // LOWRATE_BITPOOL;
       M[&$sbcenc.setting_bitpool] = r0;
       r0 = 1;
       M[&link_qua_sta] = r0;
       r0 = BITPOOL_DELAY_PERIOD;
       M[&delay_counter] = r0;
       jump no_codec_copy;
       
    restore_link:
       NULL = M[&link_qua_sta];
       if Z jump no_codec_copy;  // already in normal quality
       
       r0 = M[&org_bitpool_size];
       r1 = M[&$sbcenc.setting_bitpool];
       NULL = r1 - r0;
       if GT r0 = r1;            // Choose the higher one between stored value and current value
       M[&$sbcenc.setting_bitpool] = r0;
       M[&link_qua_sta] = NULL;
       r0 = BITPOOL_RST_PERIOD;
       M[&delay_counter] = r0;
       
.endif

   // ************* Dual Stream ****************
       

   no_codec_copy:
   
.ifdef DEBUG_ON 
   // increment timer count
   r0 = M[$time_elapsed];
   r0 = r0 + 1;
   M[$time_elapsed]=r0;
   
   // update total number of bytes sent to be used for measuring bitrate
   // *** This might not be correct in dual stream ***
   r0 = M[$cbops.amount_to_use];
   r0 = r0 + r0; // word to byte
   // add to 48-bit accumulator
   r1 = M[$total_bytes_sent];  
   r2 = M[$total_bytes_sent+1];
   r1 = r1 + r0;
   r2 = r2 + carry;
   M[$total_bytes_sent] = r1;
   M[$total_bytes_sent+1] = r2;
.endif
   
   // post another timer event
   r1 = &$av_copy_timer_struc;
   r2 = M[$vm.codec_type.timer_period];
   r3 = &$av_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

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

// ***************************************************************************************
// MODULE:
//     $SCO_copy_handler
//
// DESCRIPTION:
//   SCO mode audio copy handler
//   - mixing/downsampling audio received from USB into one channel and copying to SCO port
//   - upsampling receive SCO data to be sent to USB output port
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************
.MODULE $M.SCO_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $SCO_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // USB in port has already been handled by the calling function

   // Mix the two inputs together to form a mono stream
   r8 = &$usb_audio_in_mix_copy_struc;
   call $cbops.copy;
   
   // downsample the input from 48kHz to 8kHz
   r8 = &$sco_audio_in_downsample_struct;
   call $src.upsample_downsample;
   

   // copy the samples to the SCO out port
   r8 = &$sco_out_copy_struc;
   call $cbops.copy;

   // bring in the samples from the SCO in port
   r8 = &$sco_in_copy_struc;
   call $cbops.copy;

   // upsample the output from 8kHz to 48kHz
   r8 = &$sco_audio_out_upsample_struct;
   call $src.upsample_downsample;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***************************************************************************************
// MODULE:
//     $idle_copy_handler
//
// DESCRIPTION:
//    Idle mode audio copy handler
//   - purging possible audio data received fom USB 
//   - keep copying silence to USB output port if consumed
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************
.MODULE $M.idle_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $idle_copy_handler:
   
   // push rLink onto stack
   $push_rLink_macro;

    // USB in port has already been handled by the calling function
   // check that there is at least 1ms data in both channel
    r0 = &$audio_in_left_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   r3 = r0;
   r0 = &$audio_in_right_cbuffer_struc;
   call $cbuffer.calc_amount_data;
   Null = r0 - r3;
   if POS r0 = r3;
   
    //remove proper number of samples from audio buffers
    M0 = 48; //1ms

    // Make sure there is enough data in the buffer
    Null = r0 - M0;
    if NEG jump done_purge_usb_audio;
   
      // purge samples from left buffer
      r0 = &$audio_in_left_cbuffer_struc;
      call $cbuffer.get_read_address_and_size;
      I0 = r0;
      L0 = r1;
   
      // move read pointer
     r0 = M[I0, M0];
     
      // update the read address
      r0 = &$audio_in_left_cbuffer_struc;
      r1 = I0;
      call $cbuffer.set_read_address;
     
       // purge samples from right buffer
      r0 = &$audio_in_right_cbuffer_struc;
      call $cbuffer.get_read_address_and_size;
      I0 = r0;
      L0 = r1;
   
      // move read pointer
     r0 = M[I0, M0];
     
      // update the read address
      r0 = &$audio_in_right_cbuffer_struc;
      r1 = I0;
      call $cbuffer.set_read_address;

      L0 = 0;

   done_purge_usb_audio:
   
   // Make sure there is enough space to write to
   r0 = &$usb_out_resample_cbuffer_struc;
   call $cbuffer.calc_amount_space;      
   Null = r0 - M0;
   if NEG jump done_insert_silence;
      
      // inser silence to the resample buffer
      r0   = &$usb_out_resample_cbuffer_struc;
      r10  = M0;
     call $insert_silence_to_cbuffer;
   done_insert_silence:
   
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// ***************************************************************************************
// MODULE:
//     $buffer_state_handler
//
// DESCRIPTION:
//     Check the usb buffers state and send messege to VM if state changed
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************
.MODULE $M.buffer_state_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   .VAR usb_in_old_state = $mv.usb_port.MOVING;
   .VAR usb_out_old_state = $mv.usb_port.MOVING;
   
   .VAR $usb_in_total_amount_received = 0;
   .VAR $usb_out_total_amount_sent = 0;
   
.ifdef DEBUG_ON   
   .VAR $usb_in_state_change_count = 0;
   .VAR $usb_out_state_change_count = 0;
.endif
   $buffer_state_handler:

   // push rLink onto stack
   $push_rLink_macro;

   // see if there is activity in usb input port
   r3 = 1;
   r0 = M[$usb_in_total_amount_received];          // get the amount of data recived from  USB port during this period
   if Z r3 = r3 - r3;                        // stae = 0 if nothing received, else state = 1
   M[$usb_in_total_amount_received] = 0;            // reset the accumulator for next round
   
    // Write back the new state as the old state 
   r2 = $mv.message_to_vm.USB_IN_STATE;
   r0 = M[usb_in_old_state];
   M[usb_in_old_state] = r3;
   
   // If the state of the USB in buffer has changed, send a message
   NULL = r3 - r0;
   if Z jump no_message_send;
  
      // start sending silence if the idle condition has met
      r0 =r3*$PAUSE_SILENCE_DURATION_TO_SEND(int);
      M[$usb_audio_in_copy_struc + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD] = r0; 
     
      call $message.send;
     
.ifdef DEBUG_ON  
      r0 = M[$usb_in_state_change_count];
      r0 = r0 + 1;
      M[$usb_in_state_change_count] = r0;
.endif  

   no_message_send:

   // see if there is activity in usb output port
   r3 = 1;
   r0 = M[$usb_out_total_amount_sent];               // get the amount of data sent to USB port during this period
   if Z r3 = r3 - r3;                                // stae = 0 if nothing received, else state = 1
   M[$usb_out_total_amount_sent] = 0;                // reset the accumulator for next round
   
   
   // Write back the new state as the old state
   r2 = $mv.message_to_vm.USB_OUT_STATE;
   r0 = M[usb_out_old_state];
   M[usb_out_old_state] = r3;
   
   // If the state of the USB in buffer has changed, send a message
   NULL = r3 - r0;
   if Z jump no_message_send2;
   call $message.send;
   
.ifdef DEBUG_ON  
      r0 = M[$usb_out_state_change_count];
      r0 = r0 + 1;
      M[$usb_out_state_change_count] = r0;
.endif 
   no_message_send2:

   // post another timer event
   r1 = &$buffer_state_handler_timer_struc;
   r2 = $TMR_PERIOD_BUFFER_STATE;
   r3 = &$buffer_state_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***************************************************************************************
// MODULE:
//     $audio_copy_handler
//
// DESCRIPTION:
//     Handler to copy audio between the USB ports and the DSP
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************
.MODULE $M.audio_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   .VAR temp;
   .VAR $wbs_count = 0;

   $audio_copy_handler:
   
   .VAR audio_copy_function_table[] = &$idle_copy_handler,    //idle mode = 0
                        &$usb_out_copy_handler, //av mode = 1
                     0,                      // no function for analogue mode
                     $SCO_copy_handler;      // scod mode = 3
                     
   // push rLink onto stack
   $push_rLink_macro;

   // call the proper function based on  the current mode
   r0 = M[$mv.mode];
   r0 = M[r0 + audio_copy_function_table];
   if NZ call r0; 
   
   // copy data to the USB port if there is any
   r8 = &$usb_audio_out_copy_struc;
   call $cbops.copy;
   
   // accumulates the amount of data copied to USB port
   // is used to monitor  the port
   r0 = M[$cbops.amount_to_use];
   r0 = r0 + M[$usb_out_total_amount_sent];
   M[$usb_out_total_amount_sent] = r0;
   
   // post another timer event
   r1 = &$audio_copy_timer_struc;
   r2 = $TMR_PERIOD_USB_OUT_AUDIO_COPY;
   r3 = &$audio_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***************************************************************************************
// MODULE:
//     $usb_copy_handler
//
// DESCRIPTION:
//     USB audio input copy handler
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************
.MODULE $M.usb_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $usb_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;
   
   // copy the receive data in from the USB
   r8 = &$usb_audio_in_copy_struc;
   call $mvdongle.usb_stereo_audio_copy;
   
   
   // call the PEQ processing routine
.ifdef USES_PEQ
   call $mv_dongle.peq.caller;
.endif   
   
   // post another timer event
   r1 = &$usb_copy_timer_struc;
   // The USB timer is set by the USB copy process!
   r2 = $TMR_PERIOD_USB_COPY;
   r3 = &$usb_copy_handler;
   call $timer.schedule_event_in_period;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***************************************************************************************
// MODULE:
//     $usb_out_copy_handler
//
// DESCRIPTION:
//     inserts silence to USB output port
//
// INPUTS:
//     - none
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// *****************************************************************************************

.MODULE $M.usb_out_copy_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $usb_out_copy_handler:

   // push rLink onto stack
   $push_rLink_macro;


   // We need to do a dummy write to the "microphone" port
   // Then we can detect when the port is being used again
   // as the ports will start moving and the cbuffer pointer
   // will then also move.
   r0 = &$usb_out_resample_cbuffer_struc;
   call $cbuffer.calc_amount_space;
   r1 = r0;
   r0 = &$usb_out_resample_cbuffer_struc;
   r10 = r1 - (( $BLOCK_SIZE + $RESAMPLER_READ_BACK + 8 )*6);
   if GT call $insert_silence_to_cbuffer; 
   
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// ***************************************************************************************
// MODULE:
//     $usb_out_copy_handler
//
// DESCRIPTION:
//     utility funtion to insert silence into a cbuffer
//
// INPUTS:
//     - r0: cbuffer structure address to insert silence
//     - r10: number of silenc samples to insert
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
// NOTE:
// this function assumes that there is enough space to insert samples
// *****************************************************************************************
.MODULE $M.insert_silence_to_cbuffer;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
$insert_silence_to_cbuffer:
      // push rLink onto stack
      $push_rLink_macro;
   
      call $cbuffer.get_write_address_and_size;
      I0 = r0;
      L0 = r1;

      r0 = 0;
      // if we've got to here we just do the copies
      do copy_silence;
           M[I0, 1] = r0;
      copy_silence:

      r0 = &$usb_out_resample_cbuffer_struc;
      r1 = I0;
      call $cbuffer.set_write_address;

      L4 = 0;
      // pop rLink from stack
      jump $pop_rLink_and_rts;
     
 .ENDMODULE;

// *****************************************************************************
// MODULE:
//    $USB_IN.stereo_audio_in_copy
//
// DESCRIPTION:
//    Copy available usb audio data from a read port 2 cbuffers.
//
// INPUTS:
//    - r8 = pointer to operator structure:
//       - STEREO_COPY_SOURCE_FIELD:         usb source port (port ID)
//       - STEREO_COPY_LEFT_SINK_FIELD:      left sink cbuffer (address of 
//                                              cbuffer structure)
//       - STEREO_COPY_RIGHT_SINK_FIELD      right sink cbuffer (address of 
//                                              cbuffer structure)
//       - STEREO_COPY_PACKET_LENGTH_FIELD   USB Packet Size (4 time sample 
//                                              rate)
//       - STEREO_COPY_SHIFT_AMOUNT_FIELD    Amount to Shift audio data after
//                                              reading from 
//                                           USB MMU port
//       - STEREO_COPY_LAST_HEADER_FIELD     USB header info for sync detection
//       - STEREO_COPY_SILENCE_CNT_FIELD     Number of msec of silence. Used to 
//                                              detect when USB data has halted.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r6, r7, I0, I4, I5, L0 = 0, L5 = 0, r10, DoLoop
//
// NOTES:
//    When enumerating on usb as a sound device we get a single byte stream of
//    data.  This consists of a byte of header followed by a number of
//    16bit samples.  If stereo mode is selected then samples are alternately
//    left and then right.  USB 16bit samples are LSbyte first which is the MMU
//    port's default mode.
//
// *****************************************************************************    
.MODULE $M.mvdongle.stereo_audio_in_copy;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
   // Constant used as silence buffer      
   .VAR  ZeroBuff = 0;   
.ifdef DEBUG_ON   
   .VAR $debug_usb_silence_sent = 0;
    .VAR $debug_usb_in_sync_count = 0;
    .VAR $debug_usb_out_sync_count = 0;
.endif   
$mvdongle.usb_stereo_audio_copy:
   // push rLink onto stack
   $push_rLink_macro;
   
   // *** Only Transfer Complete USB packets per iteration ***
   // *** (expect 1 msec of audio) ***

   // find amount of space in left sink
   r0 = M[r8 + $mvdongle.STEREO_COPY_LEFT_SINK_FIELD];
   call $cbuffer.calc_amount_space;
   r3 = r0;
   // find amount of space in right sink
   r0 = M[r8 + $mvdongle.STEREO_COPY_RIGHT_SINK_FIELD];   
   call $cbuffer.calc_amount_space;   
   // calculate max number of locations that we could write to the sinks
   Null = r3 - r0;
   if POS r3 = r0;   
   
   // r7 = samples per channel per USB packet (i.e. 1 msec of audio)
   r7 = M[r8 + $mvdongle.STEREO_COPY_PACKET_LENGTH_FIELD]; 
   r7 = r7 LSHIFT -2;  
   NULL = r3 - r7;
   if NEG jump $pop_rLink_and_rts;
      
   // Assume USB Available - Use Silence Value
   I4 = &ZeroBuff;      

   // Check amount of data in USB Port
   r0 = M[r8 + $mvdongle.STEREO_COPY_SOURCE_FIELD];
   // get the DSP port number
   r6 = r0 AND $cbuffer.PORT_NUMBER_MASK;    
   
.ifdef BC5
   // force an MMU buffer set (BC5 or later)
   Null = M[$PORT_BUFFER_SET];
   // Get amount of data from port
   call $cbuffer.calc_amount_data;   
.else
   // For BC3MM we calc amount of data directly. 
   // The function $cbuffer.calc_amount_data rounds read_port_limit_addr 
   // to word boundries potentially effectively omitting one available byte.
   // In order to purge the buffer we need the exact amount of data.
   // The issue with "stall data" that the rounding prevents is not relevent
   // as we wait for a complete USB packet anyway.
   // check port is still valid
   r1 = M[r6 + $cbuffer.read_port_limit_addr];
   if Z jump lp_bc3_port_done;
      // get limit offset value
      r1 = M[r1];
      r2 = M[r6 + $cbuffer.read_port_local_offset];
      // calculate the amount of data (Limit offset - local read offset)
      r1 = r1 - r2;
      // get buffer size minus one as mask for wrapping
      r2 = M[r6 + $cbuffer.read_port_buffer_size];
      r2 = r2 - 1;
      // mask out any wrap around
      r1 = r1 AND r2;
lp_bc3_port_done:   

.endif

   // r10 is bytes to transfer
   r10 = r1;                                 
   // Verify at least one USB Pack is available  
   r2 = M[r8 + $mvdongle.STEREO_COPY_PACKET_LENGTH_FIELD];
   Null = r1 - r2;
   if POS jump jp_has_data;
      // See if data was transfered in previous period
      r2 = M[r8 + $mvdongle.STEREO_COPY_STALL_FIELD];
     r2 = r2 + 1;
     Null = r2 - $USB_STALL_TIME_BEFOR_SINLENC_INSERTION;
     if POS jump insert_silence;
     M[r8 + $mvdongle.STEREO_COPY_STALL_FIELD] = r2;    
     jump $pop_rLink_and_rts;
insert_silence:
      r0 = M[$mv.mode];
      Null = r0 - $mv.mode.AV_MODE;
      if NZ jump $pop_rLink_and_rts;
     
      // limit the silence duration
      r0 = M[r8 + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD];
      Null = r0 - $PAUSE_SILENCE_DURATION_TO_SEND;
      if POS jump $pop_rLink_and_rts;
   
      // increment silence counter
      r0 = r0 + 1;
     M[r8 + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD] = r0;
     
     r7 = ($TMR_PERIOD_USB_COPY*$USB_SAMPLE_RATE/1000)/1000;     
.ifdef DEBUG_ON
   r0 = M[$debug_usb_silence_sent];
   M[$debug_usb_silence_sent] = r0 + r7;
.endif

      jump jp_transfer_data;    

jp_has_data:
   // for monitoring
   r0 = M[$usb_in_total_amount_received];
   M[$usb_in_total_amount_received] = r0 + r7;
   
   // Signal Packet is being transfered
   M[r8 + $mvdongle.STEREO_COPY_STALL_FIELD] = 0;
   M[r8 + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD] = 0;
   
   // switch to 8bit read mode
   M[r6 + $READ_PORT_CONFIG_BASE] = 0; 
      
   // Increment Previous header
   r1 = M[r8 + $mvdongle.STEREO_COPY_LAST_HEADER_FIELD];
   r1 = r1 + 1;
   r1 = r1 AND 0x7F;
   // read one byte, should be the packet header
   r0 = M[r6 + $READ_PORT_DATA_BASE];  
   r0 = r0 AND 0x7F;

.ifndef BC5
   // BC3MM not reliable on reading byte
   // if not on a word boundary.  So just assume we passed
   r2 = M[r6 + $cbuffer.read_port_local_offset];
   Null = r2 AND 0x1;
   if NZ r0 = r1;
.endif

   M[r8 + $mvdongle.STEREO_COPY_LAST_HEADER_FIELD] = r0;
      
   // check synchronization 
   Null = r0 - r1;
   if NZ jump usb_out_of_sync;
      // set up index register for the usb port and switch to 16bit read mode
      I4 = r6 + $READ_PORT_DATA_BASE;
      r1 = $BITMODE_16BIT;
      M[r6 + $READ_PORT_CONFIG_BASE] = r1;

.ifdef DEBUG_ON   
   r0 = M[$debug_usb_in_sync_count];
   r0 = r0 + 1;
   M[$debug_usb_in_sync_count] = r0;
.endif

      jump jp_transfer_data;
usb_out_of_sync: 
   
.ifdef DEBUG_ON   
   r0 = M[$debug_usb_out_sync_count];
   r0 = r0 + 1;
   M[$debug_usb_out_sync_count] = r0;
.endif

.ifndef BC5
   // For BC3 update cashed pointer
   r0 = M[r6 + $cbuffer.read_port_local_offset];
   r0 = r0 + r10;
   M[r6 + $cbuffer.read_port_local_offset] = r0;
.endif

   // r6=USB Port, r10=bytes in usb 
   // Already Read header byte
   r10 = r10 -1;                    
   // Purge USB Buffer
lp_loop:
      Null = M[r6 + $READ_PORT_DATA_BASE];
      r10 = r10 - 1;
   if GT jump lp_loop;
   
   // Port Purged.  Now add 1 msec of silence to buffer
jp_transfer_data:  
   
   // ***** Get Output Buffers
   r10 = r7;
   // set up index and length registers for the left channel
   r0 = M[r8 + $mvdongle.STEREO_COPY_LEFT_SINK_FIELD];
   call $cbuffer.get_write_address_and_size;
   I0 = r0;    
   L0 = r1;
   // set up index and length registers for the right channel
   r0 = M[r8 + $mvdongle.STEREO_COPY_RIGHT_SINK_FIELD];
   call $cbuffer.get_write_address_and_size;
   I5 = r0;    
   L5 = r1;
   
   r3 = M[r8 + $mvdongle.STEREO_COPY_SHIFT_AMOUNT_FIELD];

   do lp_stereo_loop;
      // read and write the left channel sample
      r0 = M[I4,0];
      r0 = r0 ASHIFT r3;
      // read and write the right channel sample
      r1 = M[I4,0];
      r1 = r1 ASHIFT r3;
      // Write Outputs
      M[I0,1] = r0, M[I5,1] = r1;
lp_stereo_loop:

   // update buffer write address for the left channel
   // and amount of data for frame sync
   r0 = M[r8 + $mvdongle.STEREO_COPY_LEFT_SINK_FIELD];
   r1 = I0;
   call $cbuffer.set_write_address;
   L0 = 0;
   
   // update buffer write address for the right channel
   r0 = M[r8 + $mvdongle.STEREO_COPY_RIGHT_SINK_FIELD];
   r1 = I5;
   call $cbuffer.set_write_address;
   L5 = 0;
    
   // If Silence then done
   Null = I4 - &ZeroBuff;
   if Z jump $pop_rLink_and_rts;
    
   // Not silence.  Reset Counter
   M[r8 + $mvdongle.STEREO_COPY_SILENCE_CNT_FIELD] = Null;
   
   //*******************************************   
   
   // Update USB buffer

.ifndef BC5
   // For BC3MM need to update chached buffer pointer for header bye
   r0 = M[r6 + $cbuffer.read_port_local_offset];
   r0 = r0 + 1;
   M[r6 + $cbuffer.read_port_local_offset] = r0;
   // r2 equals words transfered from USB
   r2 = r7 ASHIFT 1;              
.endif

   r0 = M[r8 + $mvdongle.STEREO_COPY_SOURCE_FIELD]; 
   r1 = I4;
   call $cbuffer.set_read_address;
      
   // If we have sufficient USB data remaining for a packet then repeat 
   r0 = M[r8 + $mvdongle.STEREO_COPY_SOURCE_FIELD];
   call $cbuffer.calc_amount_data;
   $pop_rLink_macro;     
   r2 = M[r8 + $mvdongle.STEREO_COPY_PACKET_LENGTH_FIELD];
   r2 = r2 ASHIFT 1;   // double packet size to adjust for jitter
   NULL = r1 - r2;
   if POS jump $mvdongle.usb_stereo_audio_copy;             // More to Transfer
   rts;  
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


.MODULE $M.sbc_set_low_bitpool_size;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $sbc_set_low_bitpool_size:

   M[&$low_bitpool_size] = r1;

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