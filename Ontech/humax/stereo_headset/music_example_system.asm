// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc %%copyright(2008) http://www.csr.com
// %%version
//
// $Revision$  $Date$
// *****************************************************************************

// *****************************************************************************
// NAME:
//    two_mic_example_system.asm
//
// DESCRIPTION:
//    This file defines the functions that are used by the two_mic_example
//    system.
//
// *****************************************************************************

.include "stack.h"
.include "music_example.h"
.include "core_library.h"
.include "spi_comm_library.h"
.include "mips_profile.h"
.include "music_example_config.h"



// *****************************************************************************
// MODULE:
//    $M.music_example_process
//
// DESCRIPTION:
// This routine is called from the main loop when a frame of data is ready
// to be processed.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// CPU USAGE:
//    CODE memory:    9  words
//    DATA memory:    2  words
// *****************************************************************************
.MODULE $M.music_example_process;
 .CODESEGMENT  PM;
 .DATASEGMENT  DM;
   .VAR  TimeStamp;
   .VAR  PeakMips;

$music_example_process:
   $push_rLink_macro;
.if (defined(BC5MM) || defined(BC5ROM))
   M[$ARITHMETIC_MODE] = NULL;
.endif

// Check for Initialization
   NULL = M[$music_example.reinit];
   if NZ call $music_example_reinitialize;

// Get Current System Mode
   r1 = M[$music_example.sys_mode];

   // Override Mode & CallState
   r0 = M[$music_example.SysControl];
   // r1 = mode (from above)
   r4 = M[$music_example.OvrMode];
   
   Null = r0 AND $M.music_example.CONTROL.MODE_OVERRIDE;
   if NZ r1 = r4;
   
   // see if mono or stereo connection, based on whether the right output port
   // is enabled
   r3 = $music_example.SYSMODE.MONO;
   r2 = $music_example.SYSMODE.MONO_PASSTHRU;
   Null = r1 - $music_example.SYSMODE.MONO;
   if POS jump end_mono_handle;
   Null = r1 - $music_example.SYSMODE.FULLPROC;
   if Z r2 = r3;
   r0 = $AUDIO_RIGHT_OUT_PORT;
   call $cbuffer.is_it_enabled;
   .ifdef FORCE_MONO
   r0 = 0;
   .endif
   if Z r1 = r2;
   end_mono_handle:

   r2 = $music_example.SYSMODE.PASSTHRU;
   r0 = M[$current_dac_sampling_frequency];

   // load the existing value of dither type
   r4 = M[$M.system_config.data.dithertype];

   // Set the Mode to PASSTHRU if the sample rate is not equal to 44.1kHz or 48kHz
   NULL = r0 - 44100;
   if Z jump no_fs_force_passthru;
   NULL = r0 - 48000;
   if Z jump no_fs_force_passthru;
   
   // Set r1 = PASSTHRU if fs not equal to 44100 or 48000
   r1 = r2;
no_fs_force_passthru:
   M[$music_example.CurMode] = r1;
   r2 = 0;
   
   // Disable dither if system is in passthru mode
   NULL = r1 - $music_example.SYSMODE.PASSTHRU;
   if Z r4 = r2;

   // Disable dither if fs < 44.1kHz
   NULL = r0 - 44100;   
   if NEG r4 = r2;
   .ifndef HARDWARE_RATE_MATCH
      M[$audio_out_rate_adjustment_and_shift_op_stereo.param + $cbops.rate_adjustment_and_shift.DITHER_TYPE_FIELD] = r4;
      M[$audio_out_rate_adjustment_and_shift_op_mono.param + $cbops.rate_adjustment_and_shift.DITHER_TYPE_FIELD] = r4;   
   .else
      M[$audio_out_dither_and_shift_op_left.param + $cbops.dither_and_shift.DITHER_TYPE_FIELD] = r4;
      M[$audio_out_dither_and_shift_op_right.param + $cbops.dither_and_shift.DITHER_TYPE_FIELD] = r4;         
      M[$audio_out_dither_and_shift_op_mono.param + $cbops.dither_and_shift.DITHER_TYPE_FIELD] = r4;
   .endif
   
// Call processing table that corresponds to the current mode 
   r4 = M[$M.system_config.data.mode_table + r1];
   call $frame.run_function_table;

   jump $pop_rLink_and_rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $M.music_example_reinitialize
//
// DESCRIPTION:
// This routine is called by music_example_process when the algorithm needs to
// be reinitialized.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// CPU USAGE:
//    CODE memory:    6  words
//    DATA memory:    0  words
// *****************************************************************************
.MODULE $M.music_example_reinitialize;
 .CODESEGMENT PM;

$music_example_reinitialize:
   
// Transfer Parameters to Modules.
// Assumes at least one value is copied
   M1 = 1;
   I0 = &$M.system_config.data.ParameterMap;
   // Get Source (Pre Load)
   r0 = M[I0,M1];   
lp_param_copy: 
      // Get Destination
      r1 = M[I0,M1];            
      // Read Source,  
      r0 = M[r0];               
      // Write Destination,  Get Source (Next)  
      M[r1] = r0, r0 = M[I0,M1];   
      // Check for NULL termination    
      Null = r0;              
   if NZ jump lp_param_copy;
   
   // copy current config word to codec specefic config word
   // this is just to keep the two words synchronized
   r0 = M[&$M.system_config.data.CurParams + $music_example.PARAMETERS.OFFSET_CONFIG];
   M[&$M.system_config.data.CurParams + $music_example.CODEC_CONFIG] = r0;

// Call Module Initialize Functions
   $push_rLink_macro;
   r4 = &$M.system_config.data.reinitialize_table;
   call $frame.run_function_table;

// Clear Reinitialization Flag
   M[$music_example.reinit]    = NULL;
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $M.music_example.peq.process
//
// DESCRIPTION:
//    front end for peq's. Exits if 0-stage.
//
// INPUTS:
//    - r7 = pointer to peq object
//
// OUTPUTS:
//    - none
//
// CPU USAGE:
//    CODE memory:    6  words
//    DATA memory:    0  words
//
// *****************************************************************************

.MODULE $music_example.peq;
   .CODESEGMENT PM;

initialize:
   // check if SE is bypassed
   r0 = M[r7 + $music_example.peq.PEQ_CONFIG_FIELD];
   r1 = M[r7 + $music_example.peq.COEFF_SELECT_FIELD];
   
   r5 = $music_example.peq.BS_STRUC_SIZE;
   r1 = r1 AND r0;
   
   if NZ r8 = r8 + r5;
   
   // Increment BS pointer to account for sample frequency
   r5 = $music_example.peq.2X_BS_STRUC_SIZE;
   r1 = M[$current_dac_sampling_frequency];
   Null = r1 - 48000;
   if Z r8 = r8 + r5;

   r0 = M[r8 + $music_example.peq.BS_COEFFS_PTR_FIELD];
   M[r7 + $music_example.peq.COEFS_ADDR_FIELD] = r0;
   r0 = M[r8 + $music_example.peq.BS_SCALE_PTR_FIELD];   
   M[r7 + $music_example.peq.SCALING_ADDR_FIELD] = r0;
   r0 = M[r8 + $music_example.peq.BS_NUMSTAGES_FIELD];
   r0 = M[r0];
   M[r7 + $music_example.peq.NUM_STAGES_FIELD] = r0;

   jump $audio_proc.peq.initialize;

process:
   // check if SE is bypassed
   r0 = M[r7 + $music_example.peq.PEQ_CONFIG_FIELD];
   r1 = M[r7 + $music_example.peq.ENABLE_BIT_MASK_FIELD];
   r1 = r1 AND r0;
   if NZ rts;

   jump $audio_proc.peq.process;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $M.music_example.cmpd100.initialize/analysis/applygain
//
// DESCRIPTION:
//    front end for cmpd100 functions
//
// INPUTS:
//    - r7 = pointer to 44.1kHz cmpd100 object
//    - r8 = pointer to 48kHz cmpd100 object
//
// OUTPUTS:
//    - none
//
// CPU USAGE:
//    CODE memory:    12  words
//    DATA memory:    0  words
//
// *****************************************************************************

.MODULE $music_example.cmpd100;
   .CODESEGMENT PM;

initialize:
   // Set r8 dependent on the sample rate
   // There are two data objects for the compander. One for 44.1kHz and the other
   // for 48kHz.
   r0 = M[$current_dac_sampling_frequency];
   Null = r0 - 48000;
   if NZ r8 = r7;

   jump $cmpd100.initialize;

analysis:
   // Set r8 dependent on the sample rate
   // There are two data objects for the compander. One for 44.1kHz and the other
   // for 48kHz.
   r0 = M[$current_dac_sampling_frequency];
   Null = r0 - 48000;
   if NZ r8 = r7;

   jump $cmpd100.analysis;
   
applygain:
   // Set r8 dependent on the sample rate
   // There are two data objects for the compander. One for 44.1kHz and the other
   // for 48kHz.
   r0 = M[$current_dac_sampling_frequency];
   Null = r0 - 48000;
   if NZ r8 = r7;

   jump $cmpd100.applygain;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $music_example.mix
//
// DESCRIPTION:
//   Function to copy data from one buffer into another.
//
// INPUTS:
//   r7 -  data object containing pointers to input and output data.  Also
//         contains the number of samples to process.
//
// OUTPUTS:
//   - none
//
// CPU USAGE:
//   CODE memory:   27  words
//   DATA memory:    0  words (externally defined in data object)
// *****************************************************************************

.MODULE $music_example.mix;
 .codesegment PM;
process:

// get data from data object
   I3 = r7;
   M1 = 1;
   r0 = M[I3, M1];
   I0 = r0,   r0 = M[I3,M1];  // INPUT_CH1_PTR_BUFFER_FIELD
   L0 = r0,   r0 = M[I3,M1];  // INPUT_CH1_CIRCBUFF_SIZE_FIELD
   I4 = r0,   r0 = M[I3,M1];  // INPUT_CH2_PTR_BUFFER_FIELD
   L4 = r0,   r0 = M[I3,M1];  // INPUT_CH2_CIRCBUFF_SIZE_FIELD   
   I1 = r0,   r0 = M[I3,M1];  // OUTPUT_PTR_BUFFER_FIELD
   L1 = r0,   r0 = M[I3,M1];  // OUTPUT_CIRCBUFF_SIZE_FIELD
   r3 = r0,   r0 = M[I3,M1];  // INPUT_CH1_GAIN_FIELD
   r4 = r0,   r0 = M[I3,M1];  // INPUT_CH2_GAIN_FIELD   
   r10 = r0;                  // NUM_SAMPLES

// ADC-> SCO OUT  : SCO IN -> DAC
   do loop_copy_data;
      r0 = M[I0,1], r2 = M[I4,1];
      rMAC = r0 * r3;
      rMAC = rMAC + r2 * r4;
      M[I1,1] = rMAC;
   loop_copy_data:

// Update data object
   I3 = r7;
   M2 = 2;
   r0 = I0;
   r0 = I4,  M[I3,M2] = r0;   // INPUT_CH1_PTR_BUFFER_FIELD
   r0 = I1,  M[I3,M2] = r0;   // INPUT_CH2_PTR_BUFFER_FIELD
             M[I3,M2] = r0;   // OUTPUT_CH1_PTR_BUFFER_FIELD


// Clear L registers
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;
   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $M.music_example.send_ready_msg 
//
// DESCRIPTION:
//    This function sends a ready message to the VM application signifying that
//    it is okay for the VM application to connect streams to the kalimba.  The
//    application needs to call this function just prior to scheduling the audio
//    interrupt handler.  
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//          
// CPU USAGE:
//    cycles = 
//    CODE memory:    5  words
//    DATA memory:    4  words
// *****************************************************************************
.MODULE $M.music_example.power_up_reset;
 .DATASEGMENT    DM;
 .CODESEGMENT    PM;

// Entries can be added to this table to suit the system being developed.
   .VAR  message_handlers[] =
// Message Struc Ptr  Message ID  Message Handler  Registration Function
   &$M.music_example_message.set_mode_message_struc,    $music_example.VMMSG.SETMODE,      &$M.music_example_message.SetMode.func,     $message.register_handler,
   &$M.music_example_message.volume_message_struc,      $music_example.VMMSG.VOLUME,       &$M.music_example_message.Volume.func,      $message.register_handler,
   &$M.music_example_message.load_params_message_struc, $music_example.VMMSG.LOADPARAMS,   &$M.music_example.LoadParams.func,          $message.register_handler,
   &$M.music_example_spi.status_message_struc,          $M.music_example.SPIMSG.STATUS,    &$M.music_example.GetStatus.func,      $spi_comm.register_handler,
   &$M.music_example_spi.version_message_struc,         $M.music_example.SPIMSG.VERSION,   &$M.music_example.GetVersion.func,     $spi_comm.register_handler,
   &$M.music_example_spi.control_message_struc,         $M.music_example.SPIMSG.CONTROL,   &$M.music_example.GetControl.func,     $spi_comm.register_handler,
   &$M.music_example_spi.reinit_message_struc,          $M.music_example.SPIMSG.REINIT,    &$M.music_example.ReInit.func,         $spi_comm.register_handler,
   &$M.music_example_spi.parameter_message_struc,       $M.music_example.SPIMSG.PARAMS,    &$M.music_example.GetParams.func,      $spi_comm.register_handler,
      0;

$music_example.power_up_reset:
   $push_rLink_macro; 
   
   // Copy default parameters into current parameters
   call $M.music_example.load_default_params.func; 

   r4 = &message_handlers;
   call $frame.register_handlers;
   jump $pop_rLink_and_rts;
.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $M.music_example.load_default_params
//
// DESCRIPTION:
//    This function copies the default parameter values into the parameters
//    block;
//                                      
// INPUTS:
//    NONE
//
// OUTPUTS:
//    loads parameter values into parameter block (I4).
//
// TRASHED REGISTERS:
//    r0, r10, I0, I4     
//
// CPU USAGE:
//    cycles = 
//    CODE memory:     7 words
//    DATA memory:     0 words
// *****************************************************************************
.MODULE $M.music_example.load_default_params;

   .CODESEGMENT PM;   

func:  
   r10 = $music_example.PARAMETERS.STRUCT_SIZE;
   I4 = &$M.system_config.data.CurParams;
   I0 = &$M.system_config.data.DefaultParameters;
   r0 = M[I0,1]; 
   do lp_param_copy;
      r0 = M[I0,1], M[I4,1] = r0;     
lp_param_copy:   
   rts;
.ENDMODULE;
.ifdef SELECTED_CODEC_FASTSTREAM
// *****************************************************************************
// MODULE:
//    $M.music_example.extract_faststream_info
//
// DESCRIPTION:
//    utility function to extract some info from codec, this info is 
//    required when sending Statistic struct to music manager
//                                      
// INPUTS:
//    NONE
//
// OUTPUTS:
//    NONE
//
// TRASHED REGISTERS:
//    r0
// *****************************************************************************
.MODULE $M.music_example.extract_faststream_info;

   .CODESEGMENT PM;   
func:  
   // get info only when a frame successfully decoded
   r0 = M[&$decoder_codec_stream_struc + $codec.av_decode.MODE_FIELD];
   Null = r0 - $codec.SUCCESS;
   if NZ rts;
   
   // copy sampling rate and bitbool for just decoded frame
   r0 = M[$sbc.sampling_freq];
   M[&$music_example.dec_sampling_freq] = r0;
   r0 = M[$sbc.bitpool];
   M[&$music_example.dec_bitpool] = r0;   
   rts;
.ENDMODULE;
.endif //.ifdef SELECTED_CODEC_FASTSTREAM
// *****************************************************************************
// MODULE:
//    $M.music_example 
//
// DESCRIPTION:
//    music_example data object.
//
// *****************************************************************************
.MODULE $M.MUSIC_EXAMPLE_VERSION_STAMP;
   .DATASEGMENT DM;
   .BLOCK VersionStamp;
   .VAR  h1 = 0xbeef;
   .VAR  h2 = 0xbeef;
   .VAR  h3 = 0xbeef;  
   .VAR  SysID = $MUSIC_EXAMPLE_SYSID;
   .VAR  BuildVersion = MUSIC_EXAMPLE_VERSION;
   .VAR  h4 = 0xbeef;
   .VAR  h5 = 0xbeef;
   .VAR  h6 = 0xbeef;
   .ENDBLOCK;
.ENDMODULE;

// System Configuration is saved in kap file.
.MODULE $M.MUSIC_EXAMPLE_MODULES_STAMP;
   .DATASEGMENT DM;
   .BLOCK ModulesStamp;
      .VAR  s1 = 0xfeeb;
      .VAR  s2 = 0xfeeb;
      .VAR  s3 = 0xfeeb;
      .VAR  CompConfig = MUSIC_EXAMPLE_CONFIG_FLAG;
      .VAR  s4 = 0xfeeb;
      .VAR  s5 = 0xfeeb;
      .VAR  s6 = 0xfeeb;
   .ENDBLOCK; 
.ENDMODULE;

.MODULE $music_example;
 .DATASEGMENT DM;
   .VAR  Version    = MUSIC_EXAMPLE_VERSION; 
   .VAR  sys_mode   = $music_example.SYSMODE.FULLPROC;
   .VAR  reinit     = $music_example.REINITIALIZE;
   .VAR  num_modes  = 4;
   
// SPI System Control
.BLOCK SpiSysControl;
   // Bit-wise flag for tuning control
   .VAR  SysControl = 0;                
   // Override for DAC gain
   .VAR  OvrDACgain = 9;
   .VAR  OvrCallState = 0;   
   .VAR  OvrMode    = 0;
.ENDBLOCK;

   .VAR  CurDacL            = 11;
   .VAR  CurDacR            = 11;

.BLOCK Statistics;
   .VAR  CurMode            = 0;
   .VAR  PeakMipsFunc       = 0;
   .VAR  PeakMipsDecoder    = 0;
   .VAR  PeakPcmInL         = 0;                            
   .VAR  PeakPcmInR         = 0;                             
   .VAR  PeakDacL           = 0;                           
   .VAR  PeakDacR           = 0;
   .ifdef SELECTED_CODEC_FASTSTREAM
   .VAR  dec_sampling_freq;
   .VAR  dec_bitpool;
   .VAR  PeakMipsEncoder;
   .endif

.ENDBLOCK;

.ENDMODULE;
