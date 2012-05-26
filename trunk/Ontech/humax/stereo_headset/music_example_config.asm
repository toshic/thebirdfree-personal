// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc %%copyright(2009) http://www.csr.com
// %%version
//
// $Revision$  $Date$
// *****************************************************************************


// *****************************************************************************
// DESCRIPTION
//    Static configuration file that includes function references and data
//    objects for the music_example system.
//
//    Developers can use the music_example system as a starting point for
//    development.
//
//    The functionality of the system can be expanded by adding entries to the
//    funtion table along with corresponding data object declarations.  New
//    modes of operation can be created by additional function tables.
//   
//    This file, as shipped in the SDK, includes support for four modes.
//    STANDBY --
//    FULL -- Stereo 3d Enhancement, compander, dither, and PEQ are enabled
//    PASSTHROUGH -- Audio is routed from the decoded music stream to the dacs.
//                   No algorithms are enabled.
//    MONO -- The stereo decoded music stream is mixed down to one channel (left)
//            and is routed through the peq and compander and then to the dac.
//
//    INTERRUPT DRIVEN PROCESSES:
//    Data is transfered between firmware mmu buffers and dsp cbuffers via
//    operators, which run in the timer interrupt driven $audio_copy_handler
//    routines.
//
//    Data streams are synchronized using rate_match operators provided by
//    the frame_sync library.
//
//    MAIN APPLICATION PROCESSES:
//    Data is copied from the Decoded Music Outuput cbuffers to the output cbuffers
//    using the stereo_3d_enhancement function in full processing mode or through
//    the stream_copy function if stereo_3d_enhancement is not defined or in any other
//    mode.
//    These routines execute when a block size of data is available.
//
//    Tones are mixed with the left and right input signals using the
//    $cbops.auto_upsample_and_mix operator defined in codec_decoder.asm
//
// *****************************************************************************
.include "music_example.h"
.include "music_example_config.h"
.include "stream_copy.h"
.include "audio_proc_library.h"
.include "cbops_library.h"
.include "codec_library.h"
.include "core_library.h"

.define MAX_NUM_PEQ_STAGES (5)

.MODULE $M.system_config.data;
   .DATASEGMENT DM;
   
   // Temp Variable to handle disabled modules.
   .VAR  ZeroValue = 0;            
   .VAR  OneValue = 1.0;
   .VAR  config;

   .VAR  DefaultParameters[$music_example.PARAMETERS.STRUCT_SIZE] =
         0,                                            // OFFSET_CONFIG
         0,                                            // OFFSET_PEQ1_NUMSTAGES
         0x000000,0x000000,0x000000,0x000000,0x000000, // PEQ1_COEFFS
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         1,0,0,0,0,                                    // OFFSET_PEQ1_SCALE
         0,                                            // OFFSET_PEQ2_NUMSTAGES
         0x000000,0x000000,0x000000,0x000000,0x000000, // PEQ2_COEFFS
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         1,0,0,0,0,                                    // OFFSET_PEQ2_SCALE
         0,                                            // OFFSET_PEQ3_NUMSTAGES
         0x000000,0x000000,0x000000,0x000000,0x000000, // PEQ3_COEFFS
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         1,0,0,0,0,                                    // OFFSET_PEQ3_SCALE
         0,                                            // OFFSET_PEQ4_NUMSTAGES
         0x000000,0x000000,0x000000,0x000000,0x000000, // PEQ4_COEFFS
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         0x000000,0x000000,0x000000,0x000000,0x000000,
         1,0,0,0,0,                                    // OFFSET_PEQ4_SCALE		 
         15,                                           // DAC GAIN L
         15,                                           // DAC GAIN R
         $stereo_3d_enhancement.REFLECTION_DELAY,      // OFFSET_REFLECTION_DELAY
         0x7fffff,                                     // OFFSET_MIX
         0xF9B037,                                     // OFFSET_EXPAND_THRESHOLD1 q.23
         0xFA0541,                                     // OFFSET_LINEAR_THRESHOLD1 q.23
         0xFE56CB,                                     // OFFSET_COMPRESS_THRESHOLD1 q.23
         0xFF8070,                                     // OFFSET_LIMIT_THRESHOLD1 q.23
         0x100000,                                     // OFFSET_INV_EXPAND_RATIO1 q4.19
         0x080000,                                     // OFFSET_INV_LINEAR_RATIO1 q4.19         
         0x015555,                                     // OFFSET_INV_COMPRESS_RATIO1 q4.19
         0x00CCCD,                                     // OFFSET_INV_LIMIT_RATIO1 q4.19         
         0x420B8C,                                     // OFFSET_EXPAND_ATTACK_TIME1
         0x030F11,                                     // OFFSET_EXPAND_DECAY_TIME1
         0x420B8C,                                     // OFFSET_LINEAR_ATTACK_TIME1
         0x00ECE9,                                     // OFFSET_LINEAR_DECAY_TIME1
         0x62032E,                                     // OFFSET_COMPRESS_ATTACK_TIME1
         0x009E22,                                     // OFFSET_COMPRESS_DECAY_TIME1
         0x7C997B,                                     // OFFSET_LIMIT_ATTACK_TIME1
         0x0076AC,                                     // OFFSET_LIMIT_DECAY_TIME1
         0x080000,                                     // OFFSET_MAKEUP_GAIN1 q4.19
         0xF9B037,                                     // OFFSET_EXPAND_THRESHOLD2 q.23
         0xFA0541,                                     // OFFSET_LINEAR_THRESHOLD2 q.23
         0xFE56CB,                                     // OFFSET_COMPRESS_THRESHOLD2 q.23
         0xFF8070,                                     // OFFSET_LIMIT_THRESHOLD2 q.23
         0x100000,                                     // OFFSET_INV_EXPAND_RATIO2 q4.19
         0x080000,                                     // OFFSET_INV_LINEAR_RATIO2 q4.19         
         0x015555,                                     // OFFSET_INV_COMPRESS_RATIO2 q4.19
         0x00CCCD,                                     // OFFSET_INV_LIMIT_RATIO2 q4.19         
         0x3E4859,                                     // OFFSET_EXPAND_ATTACK_TIME2
         0x02D026,                                     // OFFSET_EXPAND_DECAY_TIME2
         0x3E4859,                                     // OFFSET_LINEAR_ATTACK_TIME2
         0x00D9BA,                                     // OFFSET_LINEAR_DECAY_TIME2
         0x5E4273,                                     // OFFSET_COMPRESS_ATTACK_TIME2
         0x009150,                                     // OFFSET_COMPRESS_DECAY_TIME2
         0x7B6F09,                                     // OFFSET_LIMIT_ATTACK_TIME2
         0x006D0B,                                     // OFFSET_LIMIT_DECAY_TIME2
         0x080000,                                     // OFFSET_MAKEUP_GAIN q4.19         
         0x000000,                                     // OFFSET_DITHER_NOISE_SHAPE
         0,0,0,0,0,0,0,0,0,0;                          // OFFSET_USER_PARAMS0-9

   // Parameter to Module Map
   .VAR/DM2	ParameterMap[] =
.if uses_PEQ   
      &CurParams + $music_example.PARAMETERS.OFFSET_CONFIG,
      &left_peq_dm2 + $music_example.peq.PEQ_CONFIG_FIELD,
      &CurParams + $music_example.PARAMETERS.OFFSET_CONFIG,
      &right_peq_dm2 + $music_example.peq.PEQ_CONFIG_FIELD,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_NUMSTAGES,
      &numstages_eq1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_NUMSTAGES,
      &numstages_eq2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_NUMSTAGES,
      &numstages_eq3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_NUMSTAGES,
      &numstages_eq4,	  
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B2,
      &peq_coeffs1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B1,
      &peq_coeffs1 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B0,
      &peq_coeffs1 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_A2,
      &peq_coeffs1 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_A1,
      &peq_coeffs1 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B2,
      &peq_coeffs1 + 5,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B1,
      &peq_coeffs1 + 6,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B0,
      &peq_coeffs1 + 7,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_A2,
      &peq_coeffs1 + 8,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_A1,
      &peq_coeffs1 + 9,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B2,
      &peq_coeffs1 + 10,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B1,
      &peq_coeffs1 + 11,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B0,
      &peq_coeffs1 + 12,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_A2,
      &peq_coeffs1 + 13,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_A1,
      &peq_coeffs1 + 14,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B2,
      &peq_coeffs1 + 15,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B1,
      &peq_coeffs1 + 16,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B0,
      &peq_coeffs1 + 17,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_A2,
      &peq_coeffs1 + 18,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_A1,
      &peq_coeffs1 + 19,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B2,
      &peq_coeffs1 + 20,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B1,
      &peq_coeffs1 + 21,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B0,
      &peq_coeffs1 + 22,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_A2,
      &peq_coeffs1 + 23,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_A1,
      &peq_coeffs1 + 24,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_SCALE1,
      &eq_scale_buf1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_SCALE2,
      &eq_scale_buf1 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_SCALE3,
      &eq_scale_buf1 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_SCALE4,
      &eq_scale_buf1 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ1_SCALE5,
      &eq_scale_buf1 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B2,
      &peq_coeffs2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B1,
      &peq_coeffs2 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B0,
      &peq_coeffs2 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_A2,
      &peq_coeffs2 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_A1,
      &peq_coeffs2 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B2,
      &peq_coeffs2 + 5,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B1,
      &peq_coeffs2 + 6,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B0,
      &peq_coeffs2 + 7,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_A2,
      &peq_coeffs2 + 8,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_A1,
      &peq_coeffs2 + 9,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B2,
      &peq_coeffs2 + 10,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B1,
      &peq_coeffs2 + 11,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B0,
      &peq_coeffs2 + 12,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_A2,
      &peq_coeffs2 + 13,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_A1,
      &peq_coeffs2 + 14,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B2,
      &peq_coeffs2 + 15,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B1,
      &peq_coeffs2 + 16,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B0,
      &peq_coeffs2 + 17,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_A2,
      &peq_coeffs2 + 18,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_A1,
      &peq_coeffs2 + 19,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B2,
      &peq_coeffs2 + 20,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B1,
      &peq_coeffs2 + 21,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B0,
      &peq_coeffs2 + 22,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_A2,
      &peq_coeffs2 + 23,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_A1,
      &peq_coeffs2 + 24,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_SCALE1,
      &eq_scale_buf2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_SCALE2,
      &eq_scale_buf2 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_SCALE3,
      &eq_scale_buf2 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_SCALE4,
      &eq_scale_buf2 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ2_SCALE5,
      &eq_scale_buf2 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B2,
      &peq_coeffs3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B1,
      &peq_coeffs3 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B0,
      &peq_coeffs3 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_A2,
      &peq_coeffs3 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_A1,
      &peq_coeffs3 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B2,
      &peq_coeffs3 + 5,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B1,
      &peq_coeffs3 + 6,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B0,
      &peq_coeffs3 + 7,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_A2,
      &peq_coeffs3 + 8,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_A1,
      &peq_coeffs3 + 9,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B2,
      &peq_coeffs3 + 10,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B1,
      &peq_coeffs3 + 11,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B0,
      &peq_coeffs3 + 12,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_A2,
      &peq_coeffs3 + 13,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_A1,
      &peq_coeffs3 + 14,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B2,
      &peq_coeffs3 + 15,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B1,
      &peq_coeffs3 + 16,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B0,
      &peq_coeffs3 + 17,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_A2,
      &peq_coeffs3 + 18,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_A1,
      &peq_coeffs3 + 19,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B2,
      &peq_coeffs3 + 20,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B1,
      &peq_coeffs3 + 21,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B0,
      &peq_coeffs3 + 22,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_A2,
      &peq_coeffs3 + 23,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_A1,
      &peq_coeffs3 + 24,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_SCALE1,
      &eq_scale_buf3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_SCALE2,
      &eq_scale_buf3 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_SCALE3,
      &eq_scale_buf3 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_SCALE4,
      &eq_scale_buf3 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ3_SCALE5,
      &eq_scale_buf3 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B2,
      &peq_coeffs4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B1,
      &peq_coeffs4 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B0,
      &peq_coeffs4 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_A2,
      &peq_coeffs4 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_A1,
      &peq_coeffs4 + 4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B2,
      &peq_coeffs4 + 5,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B1,
      &peq_coeffs4 + 6,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B0,
      &peq_coeffs4 + 7,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_A2,
      &peq_coeffs4 + 8,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_A1,
      &peq_coeffs4 + 9,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B2,
      &peq_coeffs4 + 10,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B1,
      &peq_coeffs4 + 11,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B0,
      &peq_coeffs4 + 12,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_A2,
      &peq_coeffs4 + 13,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_A1,
      &peq_coeffs4 + 14,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B2,
      &peq_coeffs4 + 15,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B1,
      &peq_coeffs4 + 16,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B0,
      &peq_coeffs4 + 17,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_A2,
      &peq_coeffs4 + 18,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_A1,
      &peq_coeffs4 + 19,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B2,
      &peq_coeffs4 + 20,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B1,
      &peq_coeffs4 + 21,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B0,
      &peq_coeffs4 + 22,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_A2,
      &peq_coeffs4 + 23,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_A1,
      &peq_coeffs4 + 24,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_SCALE1,
      &eq_scale_buf4,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_SCALE2,
      &eq_scale_buf4 + 1,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_SCALE3,
      &eq_scale_buf4 + 2,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_SCALE4,
      &eq_scale_buf4 + 3,
      &CurParams + $music_example.PARAMETERS.OFFSET_PEQ4_SCALE5,
      &eq_scale_buf4 + 4,	  
.endif      
.if uses_STEREO_ENHANCEMENT      
      &CurParams + $music_example.PARAMETERS.OFFSET_CONFIG,
      &stereo_3d_obj + $stereo_3d_enhancement.SE_CONFIG_FIELD,
      &CurParams + $music_example.PARAMETERS.OFFSET_REFLECTION_DELAY,
      &stereo_3d_obj + $stereo_3d_enhancement.REFLECTION_DELAY_SAMPLES_FIELD,
      &CurParams + $music_example.PARAMETERS.OFFSET_SE_MIX,
      &stereo_3d_obj + $stereo_3d_enhancement.MIX_FIELD,
.endif
.if uses_CMPD
      &CurParams + $music_example.PARAMETERS.OFFSET_CONFIG,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_CONTROL_WORD,
      &CurParams + $music_example.PARAMETERS.OFFSET_CONFIG,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_CONTROL_WORD,      
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_THRESHOLD1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_EXPAND_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_THRESHOLD1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LINEAR_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_THRESHOLD1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_COMPRESS_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_THRESHOLD1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LIMIT_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_EXPAND_RATIO1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_INV_EXPAND_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_LINEAR_RATIO1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_INV_LINEAR_RATIO,      
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_COMPRESS_RATIO1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_INV_COMPRESS_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_LIMIT_RATIO1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_INV_LIMIT_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_ATTACK_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_EXPAND_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_DECAY_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_EXPAND_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_ATTACK_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LINEAR_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_DECAY_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LINEAR_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_ATTACK_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_COMPRESS_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_DECAY_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_COMPRESS_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_ATTACK_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LIMIT_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_DECAY_TC1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_LIMIT_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_MAKEUP_GAIN1,
      &cmpd100_obj_44kHz + $cmpd100.OFFSET_MAKEUP_GAIN,
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_THRESHOLD2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_EXPAND_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_THRESHOLD2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LINEAR_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_THRESHOLD2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_COMPRESS_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_THRESHOLD2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LIMIT_THRESHOLD,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_EXPAND_RATIO2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_INV_EXPAND_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_LINEAR_RATIO2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_INV_LINEAR_RATIO,      
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_COMPRESS_RATIO2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_INV_COMPRESS_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_INV_LIMIT_RATIO2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_INV_LIMIT_RATIO,
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_ATTACK_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_EXPAND_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_EXPAND_DECAY_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_EXPAND_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_ATTACK_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LINEAR_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LINEAR_DECAY_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LINEAR_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_ATTACK_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_COMPRESS_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_COMPRESS_DECAY_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_COMPRESS_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_ATTACK_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LIMIT_ATTACK_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_LIMIT_DECAY_TC2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_LIMIT_DECAY_TC,
      &CurParams + $music_example.PARAMETERS.OFFSET_MAKEUP_GAIN2,
      &cmpd100_obj_48kHz + $cmpd100.OFFSET_MAKEUP_GAIN,
.endif
.if uses_DITHER
      // Update both the stereo and mono operators with the noise shape field
      &CurParams + $music_example.PARAMETERS.OFFSET_DITHER_NOISE_SHAPE,
      &dithertype,
.endif
      0;

   .VAR  CurParams[$music_example.PARAMETERS.STRUCT_SIZE];
// -----------------------------------------------------------------------------
// DATA OBJECTS USED WITH PROCESSING MODULES
//
// This section would be updated if more processing modules with data objects
// were to be added to the system.

// Data object used with $stream_copy.pass_thru function
   .VAR pass_thru_obj[$stream_copy.STRUC_SIZE] =
    0,                                            // INPUT_CH1_PTR_BUFFER_FIELD
    0,                                            // INPUT_CH1_CIRCBUFF_SIZE_FIELD
    0,                                            // INPUT_CH2_PTR_BUFFER_FIELD
    0,                                            // INPUT_CH2_CIRCBUFF_SIZE_FIELD    
    0,                                            // OUTPUT_CH1_PTR_BUFFER_FIELD
    0,                                            // OUTPUT_CH1_CIRCBUFF_SIZE_FIELD
    0,                                            // OUTPUT_CH2_PTR_BUFFER_FIELD
    0,                                            // OUTPUT_CH2_CIRCBUFF_SIZE_FIELD    
    $music_example.NUM_SAMPLES_PER_FRAME;         // NUM_SAMPLES

// Data object used with $music_example.mix function
   .VAR mix_dm1[$music_example.mix.STRUC_SIZE] =
    0,                                            // INPUT_CH1_PTR_BUFFER_FIELD
    0,                                            // INPUT_CH1_CIRCBUFF_SIZE_FIELD
    0,                                            // INPUT_CH2_PTR_BUFFER_FIELD
    0,                                            // INPUT_CH2_CIRCBUFF_SIZE_FIELD    
    0,                                            // OUTPUT_CH1_PTR_BUFFER_FIELD
    0,                                            // OUTPUT_CH1_CIRCBUFF_SIZE_FIELD
    0.5,                                          // INPUT_CH1_GAIN
    0.5,                                          // INPUT_CH2_GAIN
    $music_example.NUM_SAMPLES_PER_FRAME;         // NUM_SAMPLES

.if uses_PEQ
   .VAR/DM2CIRC left_peq_delaybuf_dm2[2 * (MAX_NUM_PEQ_STAGES + 1)];
   .VAR/DM2CIRC right_peq_delaybuf_dm2[2 * (MAX_NUM_PEQ_STAGES + 1)];   
   .VAR/DM1CIRC peq_coeffs1[5 * MAX_NUM_PEQ_STAGES];
   .VAR/DM1CIRC peq_coeffs2[5 * MAX_NUM_PEQ_STAGES];
   .VAR/DM1CIRC peq_coeffs3[5 * MAX_NUM_PEQ_STAGES];
   .VAR/DM1CIRC peq_coeffs4[5 * MAX_NUM_PEQ_STAGES];   
   .VAR eq_scale_buf1[MAX_NUM_PEQ_STAGES];
   .VAR eq_scale_buf2[MAX_NUM_PEQ_STAGES];
   .VAR eq_scale_buf3[MAX_NUM_PEQ_STAGES];
   .VAR eq_scale_buf4[MAX_NUM_PEQ_STAGES];   
   .VAR numstages_eq1 = 0;
   .VAR numstages_eq2 = 0;
   .VAR numstages_eq3 = 0;
   .VAR numstages_eq4 = 0;   
   .VAR/DM2 left_peq_dm2[$music_example.peq.STRUC_SIZE] = 
      0,                                    // PTR_INPUT_DATA_BUFF_FIELD
      0,                                    // INPUT_CIRCBUFF_SIZE_FIELD
      0,                                    // PTR_OUTPUT_DATA_BUFF_FIELD
      0,                                    // OUTPUT_CIRCBUFF_SIZE_FIELD
      &left_peq_delaybuf_dm2,               // PTR_DELAY_LINE_FIELD
      &peq_coeffs1,                         // PTR_COEFS_BUFF_FIELD
      1,                                    // NUM_STAGES_FIELD
      0,                                    // DELAY_BUF_SIZE 
      0,                                    // COEFF_BUF_SIZE
      $music_example.NUM_SAMPLES_PER_FRAME, // BLOCK_SIZE_FIELD
      &eq_scale_buf1,                       // PTR_SCALE_BUFF_FIELD
      &ZeroValue,                           // INPUT_GAIN_EXPONENT_PTR
      &OneValue,                            // INPUT_GAIN_MANTISSA_PTR
      0,                                    // PEQ_CONFIG_FIELD
      $music_example.EQENA,                 // ENABLE_BIT_MASK_FIELD 
      $music_example.EQSEL;                 // COEFF_SELECT_FIELD      

   .VAR/DM2 right_peq_dm2[$music_example.peq.STRUC_SIZE] = 
      0,                                    // PTR_INPUT_DATA_BUFF_FIELD
      0,                                    // INPUT_CIRCBUFF_SIZE_FIELD
      0,                                    // PTR_OUTPUT_DATA_BUFF_FIELD
      0,                                    // OUTPUT_CIRCBUFF_SIZE_FIELD
      &right_peq_delaybuf_dm2,              // PTR_DELAY_LINE_FIELD
      &peq_coeffs1,                         // PTR_COEFS_BUFF_FIELD
      1,                                    // NUM_STAGES_FIELD
      0,                                    // DELAY_BUF_SIZE 
      0,                                    // COEFF_BUF_SIZE
      $music_example.NUM_SAMPLES_PER_FRAME, // BLOCK_SIZE_FIELD
      &eq_scale_buf1,                       // PTR_SCALE_BUFF_FIELD
      &ZeroValue,                           // INPUT_GAIN_EXPONENT_PTR
      &OneValue,                            // INPUT_GAIN_MANTISSA_PTR
      0,                                    // PEQ_CONFIG_FIELD
      $music_example.EQENA,                 // ENABLE_BIT_MASK_FIELD 
      $music_example.EQSEL;                 // COEFF_SELECT_FIELD

    .VAR/DM2 peq_bank_select[12] =
       &peq_coeffs1,
       &eq_scale_buf1,
       &numstages_eq1,
       &peq_coeffs2,
       &eq_scale_buf2,
       &numstages_eq2,
       &peq_coeffs3,
       &eq_scale_buf3,
       &numstages_eq3,
       &peq_coeffs4,
       &eq_scale_buf4,
       &numstages_eq4;	   
.endif       
.if uses_STEREO_ENHANCEMENT
   // ** Algorithm coefficients buffer
   // $coef_1_le  0.31; // should be 1.31, the value here is minused 1
   // $coef_2_le  -0.44;
   // $coef_1_ri  0.31;
   // $coef_2_ri  -0.44;
   // $coef_3_le  0.74;
   // $coef_4_le  -0.38;
   // $coef_3_ri  0.68;
   // $coef_4_ri  -0.34;
   // Inside buffer, they must be placed in the order that used by the algorithm
   // From left to right, coef 4 to 1   
   .VAR/DM1 $stereo_3d_enhancement.coeff_buf[8] = -0.38, 0.74, -0.44, 0.31, -0.34, 0.68, -0.44, 0.31;   
   
   .VAR/DMCIRC $stereo_3d_enhancement.delay1[$stereo_3d_enhancement.DELAY_BUFFER_SIZE];
   .VAR/DMCIRC $stereo_3d_enhancement.delay2[$stereo_3d_enhancement.DELAY_BUFFER_SIZE];
   

   .VAR $stereo_3d_enhancement_delay1_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($stereo_3d_enhancement.delay1),                                     // size
         &$stereo_3d_enhancement.delay1,                                            // read pointer
         &$stereo_3d_enhancement.delay1 + $stereo_3d_enhancement.REFLECTION_DELAY;  // write pointer
   .VAR $stereo_3d_enhancement_delay2_cbuffer_struc[$cbuffer.STRUC_SIZE] =
         LENGTH($stereo_3d_enhancement.delay2),                                     // size
         &$stereo_3d_enhancement.delay2,                                            // read pointer
         &$stereo_3d_enhancement.delay2 + $stereo_3d_enhancement.REFLECTION_DELAY;  // write pointer         
   .VAR/DM2 stereo_3d_obj[$stereo_3d_enhancement.STRUC_SIZE] =
      0,                                                        // INPUT_CH1_PTR_BUFFER_FIELD
      0,                                                        // INPUT_CH1_CIRCBUFF_SIZE_FIELD
      0,                                                        // INPUT_CH2_PTR_BUFFER_FIELD
      0,                                                        // INPUT_CH2_CIRCBUFF_SIZE_FIELD
      0,                                                        // OUTPUT_CH1_PTR_BUFFER_FIELD
      0,                                                        // OUTPUT_CH1_CIRCBUFF_SIZE_FIELD
      0,                                                        // OUTPUT_CH2_PTR_BUFFER_FIELD
      0,                                                        // OUTPUT_CH2_CIRCBUFF_SIZE_FIELD
      $music_example.NUM_SAMPLES_PER_FRAME,                     // NUM_SAMPLES      
      &$stereo_3d_enhancement_delay1_cbuffer_struc,             // DELAY_1_BUFFER_FIELD
      &$stereo_3d_enhancement_delay2_cbuffer_struc,             // DELAY_2_BUFFER_FIELD
      &$stereo_3d_enhancement.coeff_buf,                        // COEFF_STRUC_FIELD
      $stereo_3d_enhancement.REFLECTION_DELAY,                  // REFLECTION_DELAY_SAMPLES_FIELD
      0,                                                        // MIX (currently not used)
      0,                                                        // SE_CONFIG
      $stereo_3d_enhancement.SEENA;                             // ENABLE_BIT_MASK_FIELD
.endif

.if uses_CMPD
    .VAR/DM $cmpd_gain;
    .VAR cmpd100_obj_44kHz[$cmpd100.STRUC_SIZE] =
     64,                                                // OFFSET_CONTROL_WORD                0
     $music_example.CMPDENA,                            // OFFSET_ENABLE_BIT_MASK             1
     0,                                                 // OFFSET_INPUT_CH1_PTR               2
     0,                                                 // OFFSET_INPUT_CH1_LEN               3
     0,                                                 // OFFSET_INPUT_CH2_PTR               4
     0,                                                 // OFFSET_INPUT_CH2_LEN               5
     0,                                                 // OFFSET_OUTPUT_CH1_PTR              6
     0,                                                 // OFFSET_OUTPUT_CH1_LEN              7
     0,                                                 // OFFSET_OUTPUT_CH2_PTR              8
     0,                                                 // OFFSET_OUTPUT_CH2_LEN              9
     0x080000,                                          // OFFSET_MAKEUP_GAIN q4.19           10
     &$cmpd_gain,                                       // OFFSET_GAIN_PTR q4.19              11
     $music_example.NUM_SAMPLES_PER_FRAME,              // NUM_SAMPLES                        12
     0x800000,                                          // OFFSET_NEG_ONE q.23                13
     0.0625,                                            // OFFSET_POW2_NEG4 q.23              14
     0xF9B037,                                          // OFFSET_EXPAND_THRESHOLD q.23       15
     0xFA0541,                                          // OFFSET_LINEAR_THRESHOLD q.23       16
     0xFE56CB,                                          // OFFSET_COMPRESS_THRESHOLD q.23     17
     0xFF8070,                                          // OFFSET_LIMIT_THRESHOLD q.23        18
     0x100000,                                          // OFFSET_INV_EXPAND_RATIO q4.19      19
     0x080000,                                          // OFFSET_INV_LINEAR_RATIO q4.19      20
     0x015555,                                          // OFFSET_INV_COMPRESS_RATIO q4.19    21
     0x00CCCD,                                          // OFFSET_INV_LIMIT_RATIO q4.19       22
     0,                                                 // OFFSET_EXPAND_CONSTANT q4.19       23
     0,                                                 // OFFSET_LINEAR_CONSTANT q4.19       24
     0,                                                 // OFFSET_COMPRESS_CONSTANT q4.19     25
     4328332,                                           // OFFSET_EXPAND_ATTACK_TIME          26
     200465,                                            // OFFSET_EXPAND_DECAY_TIME           27
     4328332,                                           // OFFSET_LINEAR_ATTACK_TIME          28
     60649,                                             // OFFSET_LINEAR_DECAY_TIME           29
     6423342,                                           // OFFSET_COMPRESS_ATTACK_TIME        30
     40482,                                             // OFFSET_COMPRESS_DECAY_TIME         31
     8165755,                                           // OFFSET_LIMIT_ATTACK_TIME           32
     30380;                                             // OFFSET_LIMIT_DECAY_TIME            33
     
.VAR cmpd100_obj_48kHz[$cmpd100.STRUC_SIZE] =
     64,                                                // OFFSET_CONTROL_WORD                0
     $music_example.CMPDENA,                            // OFFSET_ENABLE_BIT_MASK             1
     0,                                                 // OFFSET_INPUT_CH1_PTR               2
     0,                                                 // OFFSET_INPUT_CH1_LEN               3
     0,                                                 // OFFSET_INPUT_CH2_PTR               4
     0,                                                 // OFFSET_INPUT_CH2_LEN               5
     0,                                                 // OFFSET_OUTPUT_CH1_PTR              6
     0,                                                 // OFFSET_OUTPUT_CH1_LEN              7
     0,                                                 // OFFSET_OUTPUT_CH2_PTR              8
     0,                                                 // OFFSET_OUTPUT_CH2_LEN              9
     0x080000,                                          // OFFSET_MAKEUP_GAIN q4.19           10
     &$cmpd_gain,                                       // OFFSET_GAIN_PTR q4.19              11
     $music_example.NUM_SAMPLES_PER_FRAME,              // NUM_SAMPLES                        12
     0x800000,                                          // OFFSET_NEG_ONE q.23                13
     0.0625,                                            // OFFSET_POW2_NEG4 q.23              14
     0xF9B037,                                          // OFFSET_EXPAND_THRESHOLD q.23       15
     0xFA0541,                                          // OFFSET_LINEAR_THRESHOLD q.23       16
     0xFE56CB,                                          // OFFSET_COMPRESS_THRESHOLD q.23     17
     0xFF8070,                                          // OFFSET_LIMIT_THRESHOLD q.23        18
     0x100000,                                          // OFFSET_INV_EXPAND_RATIO q4.19      19
     0x080000,                                          // OFFSET_INV_LINEAR_RATIO q4.19      20
     0x015555,                                          // OFFSET_INV_COMPRESS_RATIO q4.19    21
     0x00CCCD,                                          // OFFSET_INV_LIMIT_RATIO q4.19       22
     0,                                                 // OFFSET_EXPAND_CONSTANT q4.19       23
     0,                                                 // OFFSET_LINEAR_CONSTANT q4.19       24
     0,                                                 // OFFSET_COMPRESS_CONSTANT q4.19     25
     4081753,                                           // OFFSET_EXPAND_ATTACK_TIME          26
     184358,                                            // OFFSET_EXPAND_DECAY_TIME           27
     4081753,                                           // OFFSET_LINEAR_ATTACK_TIME          28
     55738,                                             // OFFSET_LINEAR_DECAY_TIME           29
     6177395,                                           // OFFSET_COMPRESS_ATTACK_TIME        30
     37200,                                             // OFFSET_COMPRESS_DECAY_TIME         31
     8089353,                                           // OFFSET_LIMIT_ATTACK_TIME           32
     27915;                                             // OFFSET_LIMIT_DECAY_TIME            33
.endif

   .VAR dithertype = 0;

   .VAR pcmin_l_pk_dtct[] = 
      0,                                    // PTR_INPUT_BUFFER_FIELD
      0,                                    // INPUT_BUFF_SIZE_FIELD 
      $music_example.NUM_SAMPLES_PER_FRAME, // NUM_SAMPLES_FEILD
      &$music_example.PeakPcmInL;           // PEAK_LEVEL_PTR

   .VAR pcmin_r_pk_dtct[] = 
      0,                                    // PTR_INPUT_BUFFER_FIELD
      0,                                    // INPUT_BUFF_SIZE_FIELD 
      $music_example.NUM_SAMPLES_PER_FRAME, // NUM_SAMPLES_FEILD
      &$music_example.PeakPcmInR;           // PEAK_LEVEL_PTR

   .VAR dac_l_pk_dtct[] = 
      0,                                    // PTR_INPUT_BUFFER_FIELD
      0,                                    // INPUT_BUFF_SIZE_FIELD 
      $music_example.NUM_SAMPLES_PER_FRAME, // NUM_SAMPLES_FEILD
      &$music_example.PeakDacL;             // PEAK_LEVEL_PTR

   .VAR dac_r_pk_dtct[] = 
      0,                                    // PTR_INPUT_BUFFER_FIELD
      0,                                    // INPUT_BUFF_SIZE_FIELD 
      $music_example.NUM_SAMPLES_PER_FRAME, // NUM_SAMPLES_FEILD
      &$music_example.PeakDacR;             // PEAK_LEVEL_PTR

 // Statistics from Modules sent via SPI
   // ------------------------------------------------------------------------
   .VAR StatisticsPtrs[$music_example.STATUS.BLOCK_SIZE] =
      &$music_example.CurMode,
      &$music_example.SysControl,
      &$music_example.PeakMipsFunc,
      &$music_example.PeakMipsDecoder,      
      &$music_example.PeakPcmInL,
      &$music_example.PeakPcmInR,
      &$music_example.PeakDacL,
      &$music_example.PeakDacR,
      &$music_example.CurDacL,
      &$music_example.CurDacR,
.if uses_PEQ
      &left_peq_dm2 + $music_example.peq.PEQ_CONFIG_FIELD,
.else
      &ZeroValue,
.endif
      &$M.MUSIC_EXAMPLE_MODULES_STAMP.CompConfig,      
      &$codec_type,
      .ifdef SELECTED_CODEC_SBC
      &$sbc.sampling_freq,
      &$sbc.channel_mode,
      &$sbc.bitpool,
      &$sbc.nrof_blocks,
      &$sbc.nrof_channels,
      &$sbc.nrof_subbands,
      &$sbc.allocation_method;
      .endif
      .ifdef SELECTED_CODEC_MP3
      &$mp3dec.sampling_freq,
      &$mp3dec.mode,
      &$mp3dec.framelength,
      &$mp3dec.bitrate,
      &$mp3dec.frame_version,
      &$mp3dec.frame_layer,
      &$M.system_config.data.ZeroValue;
      .endif
      .ifdef SELECTED_CODEC_FASTSTREAM
      &$music_example.dec_sampling_freq,
      &$music_example.dec_bitpool,
      &$voice_enabled,
      &$sbcenc.setting_sampling_freq,
      &$sbcenc.setting_bitpool,
      &$music_example.PeakMipsEncoder,
      &$M.system_config.data.ZeroValue; 
     .endif

// -----------------------------------------------------------------------------

// STREAM MAPS
//
// A stream object is created for each stream: IN_L, IN_R, DAC_L, and DAC_R.
// These objects are used to populate processing module data objects (such as
// aux_mix_dm1 and pass_thru_obj) with input pointers and output pointers so
// that processing modules (such as $tone_mix and $stream_copy.pass_thru) know 
// where to get and write their data.
//
// Entries would be added to these objects if more processing modules were to be 
// added to the system.

// left input stream map
// The pass_thru_obj uses the left in stream to populate its input field
   .VAR stream_map_left_in[] =
    &pass_thru_obj + $stream_copy.INPUT_CH1_PTR_BUFFER_FIELD,
    &pass_thru_obj + $stream_copy.INPUT_CH1_CIRCBUFF_SIZE_FIELD,
.if uses_STEREO_ENHANCEMENT
    &stereo_3d_obj + $stereo_3d_enhancement.INPUT_CH1_PTR_BUFFER_FIELD,
    &stereo_3d_obj + $stereo_3d_enhancement.INPUT_CH1_CIRCBUFF_SIZE_FIELD,
.endif
    &mix_dm1 + $music_example.mix.INPUT_CH1_PTR_BUFFER_FIELD,
    &mix_dm1 + $music_example.mix.INPUT_CH1_CIRCBUFF_SIZE_FIELD,
    &pcmin_l_pk_dtct + $M.audio_proc.peak_monitor.PTR_INPUT_BUFFER_FIELD,
    &pcmin_l_pk_dtct + $M.audio_proc.peak_monitor.INPUT_BUFF_SIZE_FIELD;    

// right input stream map
// The pass_thru_obj uses the right in stream to populate its input field
   .VAR stream_map_right_in[] =
    &pass_thru_obj + $stream_copy.INPUT_CH2_PTR_BUFFER_FIELD,
    &pass_thru_obj + $stream_copy.INPUT_CH2_CIRCBUFF_SIZE_FIELD,
.if uses_STEREO_ENHANCEMENT    
    &stereo_3d_obj + $stereo_3d_enhancement.INPUT_CH2_PTR_BUFFER_FIELD,
    &stereo_3d_obj + $stereo_3d_enhancement.INPUT_CH2_CIRCBUFF_SIZE_FIELD,
.endif
    &mix_dm1 + $music_example.mix.INPUT_CH2_PTR_BUFFER_FIELD,
    &mix_dm1 + $music_example.mix.INPUT_CH2_CIRCBUFF_SIZE_FIELD,
    &pcmin_r_pk_dtct + $M.audio_proc.peak_monitor.PTR_INPUT_BUFFER_FIELD,
    &pcmin_r_pk_dtct + $M.audio_proc.peak_monitor.INPUT_BUFF_SIZE_FIELD;

// left output stream map
// The pass_thru_obj uses the left out stream to populate its output field
   .VAR stream_map_left_out[] =
.if uses_PEQ   
    &left_peq_dm2 + $audio_proc.peq.OUTPUT_ADDR_FIELD,
    &left_peq_dm2 + $audio_proc.peq.OUTPUT_SIZE_FIELD,
    &left_peq_dm2 + $audio_proc.peq.INPUT_ADDR_FIELD,
    &left_peq_dm2 + $audio_proc.peq.INPUT_SIZE_FIELD,
.endif    
    &pass_thru_obj + $stream_copy.OUTPUT_CH1_PTR_BUFFER_FIELD,
    &pass_thru_obj + $stream_copy.OUTPUT_CH1_CIRCBUFF_SIZE_FIELD,
.if uses_STEREO_ENHANCEMENT    
    &stereo_3d_obj + $stereo_3d_enhancement.OUTPUT_CH1_PTR_BUFFER_FIELD,
    &stereo_3d_obj + $stereo_3d_enhancement.OUTPUT_CH1_CIRCBUFF_SIZE_FIELD,
.endif
.if uses_CMPD
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_INPUT_CH1_PTR,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_INPUT_CH1_LEN,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_OUTPUT_CH1_PTR,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_OUTPUT_CH1_LEN,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_INPUT_CH1_PTR,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_INPUT_CH1_LEN,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_OUTPUT_CH1_PTR,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_OUTPUT_CH1_LEN,
.endif
    &mix_dm1 + $music_example.mix.OUTPUT_PTR_BUFFER_FIELD,
    &mix_dm1 + $music_example.mix.OUTPUT_CIRCBUFF_SIZE_FIELD,    
    &dac_l_pk_dtct + $M.audio_proc.peak_monitor.PTR_INPUT_BUFFER_FIELD,
    &dac_l_pk_dtct + $M.audio_proc.peak_monitor.INPUT_BUFF_SIZE_FIELD;

// right output stream map
// The pass_thru_obj uses the right out stream to populate its output field
   .VAR stream_map_right_out[] =
.if uses_PEQ   
    &right_peq_dm2 + $audio_proc.peq.OUTPUT_ADDR_FIELD,
    &right_peq_dm2 + $audio_proc.peq.OUTPUT_SIZE_FIELD,
    &right_peq_dm2 + $audio_proc.peq.INPUT_ADDR_FIELD,
    &right_peq_dm2 + $audio_proc.peq.INPUT_SIZE_FIELD,
.endif    
    &pass_thru_obj + $stream_copy.OUTPUT_CH2_PTR_BUFFER_FIELD,
    &pass_thru_obj + $stream_copy.OUTPUT_CH2_CIRCBUFF_SIZE_FIELD,
.if uses_STEREO_ENHANCEMENT    
    &stereo_3d_obj + $stereo_3d_enhancement.OUTPUT_CH2_PTR_BUFFER_FIELD,
    &stereo_3d_obj + $stereo_3d_enhancement.OUTPUT_CH2_CIRCBUFF_SIZE_FIELD,
.endif
.if uses_CMPD
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_INPUT_CH2_PTR,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_INPUT_CH2_LEN,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_OUTPUT_CH2_PTR,
    &cmpd100_obj_44kHz + $cmpd100.OFFSET_OUTPUT_CH2_LEN,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_INPUT_CH2_PTR,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_INPUT_CH2_LEN,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_OUTPUT_CH2_PTR,
    &cmpd100_obj_48kHz + $cmpd100.OFFSET_OUTPUT_CH2_LEN, 
.endif    
    &dac_r_pk_dtct + $M.audio_proc.peak_monitor.PTR_INPUT_BUFFER_FIELD,
    &dac_r_pk_dtct + $M.audio_proc.peak_monitor.INPUT_BUFF_SIZE_FIELD;

// ----------------------------------------------------------------------------
// STREAM SETUP TABLE
// This table is used with $frame.distribute_streams and $frame.update_streams.

// $frame.distribute_streams populates processing module data objects with
// current address into the cbuffers so modules know where to get and write
// their data.
//
// $frame.update_streams updates the cbuffer read and write addresses into the
// the cbuffers after modules have finished their processing so that the
// pointers are in the correct positions for the next time the processing
// modules are called.

.VAR stream_setup_table[] =
//IN LEFT
    &stream_map_left_in, LENGTH(stream_map_left_in)/2,
    $cbuffer.get_read_address_and_size,  $cbuffer.set_read_address,
    $music_example.NUM_SAMPLES_PER_FRAME,
//IN RIGHT
    &stream_map_right_in, LENGTH(stream_map_right_in)/2,
    $cbuffer.get_read_address_and_size,  $cbuffer.set_read_address,
    $music_example.NUM_SAMPLES_PER_FRAME,
//OUT LEFT
    &stream_map_left_out, LENGTH(stream_map_left_out)/2,
    $cbuffer.get_write_address_and_size, $cbuffer.set_write_address,
    $music_example.NUM_SAMPLES_PER_FRAME,
//OUT RIGHT
    &stream_map_right_out, LENGTH(stream_map_right_out)/2,
    $cbuffer.get_write_address_and_size, $cbuffer.set_write_address,
    $music_example.NUM_SAMPLES_PER_FRAME,    
    0;

// -----------------------------------------------------------------------------
// REINITIALIZATION FUNCTION TABLE
// Reinitialization functions and corresponding data objects can be placed
// in this table.  Functions in this table all called every time a frame of data
// is ready to be processed and the reinitialization flag is set.  
// This table must be null terminated.
   .VAR reinitialize_table[] =
    // Function                          r7                   r8
.if uses_PEQ    
    $music_example.peq.initialize,      &left_peq_dm2,       &peq_bank_select,
    $music_example.peq.initialize,      &right_peq_dm2,      &peq_bank_select,
.endif    
.if uses_STEREO_ENHANCEMENT    
    $stereo_3d_enhancement.initialize,  0,                   &stereo_3d_obj,
.endif
.if uses_CMPD
    $music_example.cmpd100.initialize,  &cmpd100_obj_44kHz,  &cmpd100_obj_48kHz,
.endif
    0;

// -----------------------------------------------------------------------------

// MODE FUNCTION TABLE
// This table contains all of the modes in this system.  The VM plugin sends a
// message that contains a mode value, which is used as a pointer into this
// table.  As shipped, this file only contains one
// mode, which corresponds to pass_thru operation.  Developers can expand this
// table if they add additional processing modes to the system.  This table must
// be null terminated.
// 
// Every time a frame of data is ready to process, the functions from the
// corresponding mode table are called.
   .VAR mode_table[] =
    &StandBy_proc_funcs,
    &pass_thru_proc_funcs,   
    &full_proc_funcs,
    &mono_proc_funcs,
    &mono_pass_thru_proc_funcs,
    // more entries can be added here
    0;

// ----------------------------------------------------------------------------
// MODE TABLES (aka FUNCTION TABLES)
// Modes are defined as tables that contain a list of functions with
// corresponding
// data objects.  The functions are called in the order that they appear.
//
// $frame.distribute_stream should always be first as it tells the processing
// modules where to get and write data.
//
// $frame.update_streams should always be last as it advances cbuffer pointers
// to the correct positions after the processing modules have read and written
// data.
//
// The processing modules are called by the function $frame.run_function_table,
// which is defined in the frame_sync library.
//
// Processing modules must be written to take input from r7 and r8.  A zero
// should be used if the module does not require input.
//
// Mode tables must be null terminated.
//
// Additional modes can be created by adding new tables.
   .VAR pass_thru_proc_funcs[] =
    // Function                                 r7                   r8
       $frame.distribute_streams,               &stream_setup_table, &$cbuffer_strucs,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_l_pk_dtct,    0,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_r_pk_dtct,    0,
       $stream_copy,                            &pass_thru_obj,      0,
       $M.audio_proc.peak_monitor.Process.func, &dac_l_pk_dtct,      0,
       $M.audio_proc.peak_monitor.Process.func, &dac_r_pk_dtct,      0,
       $frame.update_streams,                   &stream_setup_table, &$cbuffer_strucs,
       0;
       

   .VAR full_proc_funcs[] =
    // Function                                 r7                   r8
       $frame.distribute_streams,               &stream_setup_table, &$cbuffer_strucs,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_l_pk_dtct,    0,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_r_pk_dtct,    0,
.if uses_STEREO_ENHANCEMENT        
       $stereo_3d_enhancement,                  0,                   &stereo_3d_obj,
.else
       $stream_copy,                            &pass_thru_obj,      0,
.endif
.if uses_PEQ
       $music_example.peq.process,              &left_peq_dm2,       0,
       $music_example.peq.process,              &right_peq_dm2,      0,
.endif
.if uses_CMPD
       $music_example.cmpd100.analysis,         &cmpd100_obj_44kHz,  &cmpd100_obj_48kHz,
       $music_example.cmpd100.applygain,        &cmpd100_obj_44kHz,  &cmpd100_obj_48kHz,
.endif
       $M.audio_proc.peak_monitor.Process.func, &dac_l_pk_dtct,      0,
       $M.audio_proc.peak_monitor.Process.func, &dac_r_pk_dtct,      0,
       $frame.update_streams,                   &stream_setup_table, &$cbuffer_strucs,
       0;

   .VAR StandBy_proc_funcs[] =
    // Function                                 r7                   r8
       $frame.distribute_streams,               &stream_setup_table, &$cbuffer_strucs,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_l_pk_dtct,    0,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_r_pk_dtct,    0,
       $stream_copy,                            &pass_thru_obj,      0,
       $M.audio_proc.peak_monitor.Process.func, &dac_l_pk_dtct,      0,
       $M.audio_proc.peak_monitor.Process.func, &dac_r_pk_dtct,      0,
       $frame.update_streams,                   &stream_setup_table, &$cbuffer_strucs,
       0;

   .VAR mono_proc_funcs[] =
    // Function                                 r7                   r8
       $frame.distribute_streams,               &stream_setup_table, &$cbuffer_strucs,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_l_pk_dtct,    0,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_r_pk_dtct,    0,       
       $music_example.mix.process,              &mix_dm1,            0,
.if uses_PEQ       
       $music_example.peq.process,              &left_peq_dm2,       0,
.endif
.if uses_CMPD
       $music_example.cmpd100.analysis,         &cmpd100_obj_44kHz,  &cmpd100_obj_48kHz,
       $music_example.cmpd100.applygain,        &cmpd100_obj_44kHz,  &cmpd100_obj_48kHz,
.endif
       $M.audio_proc.peak_monitor.Process.func, &dac_l_pk_dtct,      0,
       $frame.update_streams,                   &stream_setup_table, &$cbuffer_strucs,
       0;
       
   .VAR mono_pass_thru_proc_funcs[] =
    // Function                                 r7                   r8
       $frame.distribute_streams,               &stream_setup_table, &$cbuffer_strucs,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_l_pk_dtct,    0,
       $M.audio_proc.peak_monitor.Process.func, &pcmin_r_pk_dtct,    0,       
       $music_example.mix.process,              &mix_dm1,            0,
       $M.audio_proc.peak_monitor.Process.func, &dac_l_pk_dtct,     0,
       $frame.update_streams,                   &stream_setup_table, &$cbuffer_strucs,
       0;

.ENDMODULE;


