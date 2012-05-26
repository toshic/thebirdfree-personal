// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc %%copyright(2009) http://www.csr.com
// %%version
//
// $Revision$  $Date$
// *****************************************************************************

.ifndef music_example_LIB_H
.define music_example_LIB_H

//  MUSIC_EXAMPLE version number
.define  MUSIC_EXAMPLE_VERSION                              0x001D

// Algorithm IDs
.CONST $MUSIC_EXAMPLE_SYSID                                 0xE002;

// SPI Message IDs
.CONST $M.music_example.SPIMSG.STATUS                       0x1007;
.CONST $M.music_example.SPIMSG.PARAMS                       0x1008;
.CONST $M.music_example.SPIMSG.REINIT                       0x1009;
.CONST $M.music_example.SPIMSG.VERSION                      0x100A;
.CONST $M.music_example.SPIMSG.CONTROL                      0x100B;

// VM Message IDs
.CONST $music_example.VMMSG.READY                           0x1000;
.CONST $music_example.VMMSG.SETMODE                         0x1001;
.CONST $music_example.VMMSG.VOLUME                          0x1002;
.CONST $music_example.VMMSG.SETPARAM                        0x1004;
.CONST $music_example.VMMSG.CODEC                           0x1006;
.CONST $music_example.VMMSG.PING                            0x1008;
.CONST $music_example.VMMSG.PINGRESP                        0x1009;
.CONST $music_example.VMMSG.SECPASSED                       0x100c;
.CONST $music_example.VMMSG.SETSCOTYPE                      0x100d;
.CONST $music_example.VMMSG.GETPARAM                        0x1010;
.CONST $music_example.VMMSG.GETPARAM_RESP                   0x1011;
.CONST $music_example.VMMSG.LOADPARAMS                      0x1012;

.CONST $music_example.REINITIALIZE                          1;

// System Modes
.CONST $music_example.SYSMODE.STANDBY                       0;
.CONST $music_example.SYSMODE.PASSTHRU                      1;
.CONST $music_example.SYSMODE.FULLPROC                      2;
.CONST $music_example.SYSMODE.MONO                          3;
.CONST $music_example.SYSMODE.MONO_PASSTHRU                 4;

.CONST $music_example.SYSMODE.LAST_MODE                   $music_example.SYSMODE.MONO_PASSTHRU;

// Sys Control Flags
// DAC Over-ride
.CONST $M.music_example.CONTROL.DAC_OVERRIDE                0x8000;
// Mode Over-ride
.CONST $M.music_example.CONTROL.MODE_OVERRIDE               0x2000;

.CONST $stereo_3d_enhancement.SEENA                         0x0040;
.CONST $music_example.EQENA                                 0x0100;
.CONST $music_example.EQSEL                                 0x0200;
.CONST $music_example.CMPDENA                               0x0020;

// these bit mask fields should be compliments
.CONST $music_example.EQCONFIG_MASK1                        0x000300;
.CONST $music_example.EQCONFIG_MASK2                        0xfffcff;

.CONST $music_example.EQCONFIG_SHIFT                        8;

// Data block size
// TMR_PERIOD_AUDIO_COPY and NUM_SAMPLES_PER_FRAME will affect the audio quality. Need to ensure
// that both TMR_PERIOD_AUDIO_COPY and NUM_SAMPLE_PER_FRAME are set appropriately to provide the
// DAC port with enough data for each timer interrupt.
//
// Therefore, the minimum value for NUM_SAMPLES_PER_FRAME is ceiling((TMR_PERIOD_AUDIO_COPY/1000000) * SAMPLE_RATE)
// where TMR_PERIOD_AUDIO_COPY is given in microseconds and SAMPLE_RATE is given in hertz.
.CONST $music_example.NUM_SAMPLES_PER_FRAME                 160;

// Overloaded PEQ data object definition
.CONST $music_example.peq.INPUT_ADDR_FIELD                  0;
.CONST $music_example.peq.INPUT_SIZE_FIELD                  1;
.CONST $music_example.peq.OUTPUT_ADDR_FIELD                 2;
.CONST $music_example.peq.OUTPUT_SIZE_FIELD                 3;
.CONST $music_example.peq.DELAYLINE_ADDR_FIELD              4;
.CONST $music_example.peq.COEFS_ADDR_FIELD                  5;
.CONST $music_example.peq.NUM_STAGES_FIELD                  6;
.CONST $music_example.peq.DELAYLINE_SIZE_FIELD              7;
.CONST $music_example.peq.COEFS_SIZE_FIELD                  8;
.CONST $music_example.peq.BLOCK_SIZE_FIELD                  9;
.CONST $music_example.peq.SCALING_ADDR_FIELD                10;
.CONST $music_example.peq.GAIN_EXPONENT_ADDR_FIELD          11;
.CONST $music_example.peq.GAIN_MANTISA_ADDR_FIELD           12;
.CONST $music_example.peq.PEQ_CONFIG_FIELD                  13;
.CONST $music_example.peq.ENABLE_BIT_MASK_FIELD             14;
.CONST $music_example.peq.COEFF_SELECT_FIELD                15;
.CONST $music_example.peq.STRUC_SIZE                        16;

// PEQ Bank Select Definitions
.CONST $music_example.peq.BS_COEFFS_PTR_FIELD               0;
.CONST $music_example.peq.BS_SCALE_PTR_FIELD                1;
.CONST $music_example.peq.BS_NUMSTAGES_FIELD                2;
.CONST $music_example.peq.BS_STRUC_SIZE                     3;

.CONST $music_example.peq.2X_BS_STRUC_SIZE                  6;

// music_example.mix data object definitions
.CONST $music_example.mix.INPUT_CH1_PTR_BUFFER_FIELD        0;
.CONST $music_example.mix.INPUT_CH1_CIRCBUFF_SIZE_FIELD     1;
.CONST $music_example.mix.INPUT_CH2_PTR_BUFFER_FIELD        2;
.CONST $music_example.mix.INPUT_CH2_CIRCBUFF_SIZE_FIELD     3;
.CONST $music_example.mix.OUTPUT_PTR_BUFFER_FIELD           4;
.CONST $music_example.mix.OUTPUT_CIRCBUFF_SIZE_FIELD        5;
.CONST $music_example.mix.INPUT_CH1_GAIN_FIELD              6;
.CONST $music_example.mix.INPUT_CH2_GAIN_FIELD              7;
.CONST $music_example.mix.NUM_SAMPLES                       8;
.CONST $music_example.mix.STRUC_SIZE                        9;


// SPI Status Block
.CONST $music_example.STATUS.CUR_MODE_OFFSET                0;
.CONST $music_example.STATUS.SYSCONTROL_OFFSET              1;
.CONST $music_example.STATUS.FUNC_MIPS_OFFSET               2;
.CONST $music_example.STATUS.DECODER_MIPS_OFFSET            3;
.CONST $music_example.STATUS.PEAK_PCMINL_OFFSET             4;
.CONST $music_example.STATUS.PEAK_PCMINR_OFFSET             5;
.CONST $music_example.STATUS.PEAK_DACL_OFFSET               6;
.CONST $music_example.STATUS.PEAK_DACR_OFFSET               7;
.CONST $music_example.STATUS.CUR_DACL_OFFSET                8;
.CONST $music_example.STATUS.CUR_DACR_OFFSET                9;
.CONST $music_example.STATUS.EQ_TIER_OFFSET                 10;
.CONST $music_example.STATUS.CONFIG_FLAG_OFFSET             11;
.CONST $music_example.STATUS.CODEC_TYPE_OFFSET              12;
.CONST $music_example.STATUS.CODEC_FS_OFFSET                13;
.CONST $music_example.STATUS.CODEC_CHANNEL_MODE             14;
.CONST $music_example.STATUS.CODEC_STAT1                    15;
.CONST $music_example.STATUS.CODEC_STAT2                    16;
.CONST $music_example.STATUS.CODEC_STAT3                    17;
.CONST $music_example.STATUS.CODEC_STAT4                    18;
.CONST $music_example.STATUS.CODEC_STAT5                    19;
.CONST $music_example.STATUS.BLOCK_SIZE                     20;

// Parameter Block
// Bitwise flag for enabling and disabling features
.CONST $music_example.PARAMETERS.OFFSET_CONFIG              0;

.CONST $music_example.PARAMETERS.OFFSET_PEQ1_NUMSTAGES      1;

// PEQ Preset #1 Coefficients
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B2      2;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B1      3;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_B0      4;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_A2      5;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE1_A1      6;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B2      7;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B1      8;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_B0      9;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_A2      10;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE2_A1      11;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B2      12;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B1      13;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_B0      14;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_A2      15;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE3_A1      16;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B2      17;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B1      18;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_B0      19;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_A2      20;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE4_A1      21;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B2      22;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B1      23;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_B0      24;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_A2      25;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_STAGE5_A1      26;

// PEQ Preset #1 Stage scale factors
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_SCALE1         27;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_SCALE2         28;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_SCALE3         29;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_SCALE4         30;
.CONST $music_example.PARAMETERS.OFFSET_PEQ1_SCALE5         31;

.CONST $music_example.PARAMETERS.OFFSET_PEQ2_NUMSTAGES      32;

// PEQ Preset #2 Coefficients
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B2      33;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B1      34;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_B0      35;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_A2      36;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE1_A1      37;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B2      38;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B1      39;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_B0      40;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_A2      41;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE2_A1      42;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B2      43;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B1      44;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_B0      45;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_A2      46;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE3_A1      47;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B2      48;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B1      49;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_B0      50;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_A2      51;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE4_A1      52;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B2      53;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B1      54;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_B0      55;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_A2      56;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_STAGE5_A1      57;

// PEQ Preset #2 Stage scale factors
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_SCALE1         58;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_SCALE2         59;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_SCALE3         60;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_SCALE4         61;
.CONST $music_example.PARAMETERS.OFFSET_PEQ2_SCALE5         62;

.CONST $music_example.PARAMETERS.OFFSET_PEQ3_NUMSTAGES      63;

// PEQ Preset #3 Coefficients
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B2      64;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B1      65;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_B0      66;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_A2      67;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE1_A1      68;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B2      69;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B1      70;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_B0      71;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_A2      72;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE2_A1      73;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B2      74;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B1      75;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_B0      76;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_A2      77;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE3_A1      78;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B2      79;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B1      80;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_B0      81;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_A2      82;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE4_A1      83;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B2      84;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B1      85;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_B0      86;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_A2      87;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_STAGE5_A1      88;

// PEQ Preset #3 Stage scale factors
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_SCALE1         89;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_SCALE2         90;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_SCALE3         91;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_SCALE4         92;
.CONST $music_example.PARAMETERS.OFFSET_PEQ3_SCALE5         93;

.CONST $music_example.PARAMETERS.OFFSET_PEQ4_NUMSTAGES      94;

// PEQ Preset #4 Coefficients
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B2      95;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B1      96;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_B0      97;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_A2      98;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE1_A1      99;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B2      100;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B1      101;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_B0      102;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_A2      103;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE2_A1      104;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B2      105;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B1      106;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_B0      107;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_A2      108;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE3_A1      109;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B2      110;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B1      111;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_B0      112;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_A2      113;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE4_A1      114;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B2      115;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B1      116;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_B0      117;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_A2      118;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_STAGE5_A1      119;

// PEQ Preset #4 Stage scale factors
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_SCALE1         120;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_SCALE2         121;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_SCALE3         122;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_SCALE4         123;
.CONST $music_example.PARAMETERS.OFFSET_PEQ4_SCALE5         124;

// DAC Gain Settings
.CONST $music_example.PARAMETERS.OFFSET_DAC_GAIN_L          125;
.CONST $music_example.PARAMETERS.OFFSET_DAC_GAIN_R          126;

// Stereo 3d Enhancement Settings
.CONST $music_example.PARAMETERS.OFFSET_REFLECTION_DELAY    127;
.CONST $music_example.PARAMETERS.OFFSET_SE_MIX              128;

// Compander Settings
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_THRESHOLD1   129;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_THRESHOLD1   130;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_THRESHOLD1 131;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_THRESHOLD1    132;
.CONST $music_example.PARAMETERS.OFFSET_INV_EXPAND_RATIO1   133;
.CONST $music_example.PARAMETERS.OFFSET_INV_LINEAR_RATIO1   134;
.CONST $music_example.PARAMETERS.OFFSET_INV_COMPRESS_RATIO1 135;
.CONST $music_example.PARAMETERS.OFFSET_INV_LIMIT_RATIO1    136;
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_ATTACK_TC1   137;
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_DECAY_TC1    138;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_ATTACK_TC1   139;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_DECAY_TC1    140;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_ATTACK_TC1 141;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_DECAY_TC1  142;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_ATTACK_TC1    143;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_DECAY_TC1     144;
.CONST $music_example.PARAMETERS.OFFSET_MAKEUP_GAIN1        145;
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_THRESHOLD2   146;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_THRESHOLD2   147;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_THRESHOLD2 148;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_THRESHOLD2    149;
.CONST $music_example.PARAMETERS.OFFSET_INV_EXPAND_RATIO2   150;
.CONST $music_example.PARAMETERS.OFFSET_INV_LINEAR_RATIO2   151;
.CONST $music_example.PARAMETERS.OFFSET_INV_COMPRESS_RATIO2 152;
.CONST $music_example.PARAMETERS.OFFSET_INV_LIMIT_RATIO2    153;
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_ATTACK_TC2   154;
.CONST $music_example.PARAMETERS.OFFSET_EXPAND_DECAY_TC2    155;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_ATTACK_TC2   156;
.CONST $music_example.PARAMETERS.OFFSET_LINEAR_DECAY_TC2    157;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_ATTACK_TC2 158;
.CONST $music_example.PARAMETERS.OFFSET_COMPRESS_DECAY_TC2  159;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_ATTACK_TC2    160;
.CONST $music_example.PARAMETERS.OFFSET_LIMIT_DECAY_TC2     161;
.CONST $music_example.PARAMETERS.OFFSET_MAKEUP_GAIN2        162;

// Post Mastering Block Settings
.CONST $music_example.PARAMETERS.OFFSET_DITHER_NOISE_SHAPE  163;

// DSP application parameter Block
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_0          164;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_1          165;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_2          166;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_3          167;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_4          168;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_5          169;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_6          170;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_7          171;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_8          172;
.CONST $music_example.PARAMETERS.OFFSET_DSP_USER_9          173;

// codec config words, one word per codec, up to 5 codecs supported
.CONST $music_example.CODEC1_CONFIG                         174;
.CONST $music_example.CODEC2_CONFIG                         175;
.CONST $music_example.CODEC3_CONFIG                         176;
.CONST $music_example.CODEC4_CONFIG                         177;
.CONST $music_example.CODEC5_CONFIG                         178;


.CONST $music_example.PARAMETERS.STRUCT_SIZE                179;

.ifdef SELECTED_CODEC_SBC
   .CONST $music_example.CODEC_CONFIG            $music_example.CODEC1_CONFIG;
.endif

.ifdef SELECTED_CODEC_MP3
   .CONST $music_example.CODEC_CONFIG            $music_example.CODEC2_CONFIG;
.endif

.ifdef SELECTED_CODEC_FASTSTREAM
   .CONST $music_example.CODEC_CONFIG            $music_example.CODEC3_CONFIG;
.endif

.endif

