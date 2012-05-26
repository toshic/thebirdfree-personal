// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc 2007             http://www.csr.com
// %%version
//
// $Revision$ $Date$
// *****************************************************************************

.include "music_example.h"
.include "spi_comm_library.h"
.include "stack.h"


// SPI Message Handlers  
.MODULE $M.music_example_spi;  
   .DATASEGMENT DM;
   .VAR status_message_struc[$spi_comm.STRUC_SIZE];
   .VAR version_message_struc[$spi_comm.STRUC_SIZE];
   .VAR reinit_message_struc[$spi_comm.STRUC_SIZE];
   .VAR parameter_message_struc[$spi_comm.STRUC_SIZE];
   .VAR control_message_struc[$spi_comm.STRUC_SIZE];  
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    message handler
//
// DESCRIPTION:
//    User registered message handler
//
// INPUTS:
//    r0 - command ID
//   r1 - Command/Response data pointer
//   r2 - command data length 
// OUTPUTS:
//    r8 - response data length
//
// *****************************************************************************
.MODULE $M.music_example.GetParams;

   .CODESEGMENT PM;

func:
   r3 = &$M.system_config.data.CurParams;
   r4 = $music_example.PARAMETERS.STRUCT_SIZE;
   r5 = &$M.system_config.data.DefaultParameters;
   M[r1] = r3;
   M[r1 + 1] = r4;
   M[r1 + 2] = r5;
   r8 = 3;
   rts; 
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    message handler
//
// DESCRIPTION:
//    User registered message handler
//
// INPUTS:
//    r0 - command ID
//   r1 - Command/Response data pointer
//   r2 - command data length 
// OUTPUTS:
//    r8 - response data length
//
// *****************************************************************************
.MODULE $M.music_example.GetVersion;

   .CODESEGMENT PM;

func:
   r3 = $MUSIC_EXAMPLE_SYSID;
   r5 = M[$music_example.Version];
   r6 = M[$current_dac_sampling_frequency];
   // Make sure the order of arguments match the 
   // SPI messaging document.
   M[r1] = r3;                   
   M[r1 + 1] = r5;
   M[r1 + 2] = r6;

   r8 = 3;
   rts; 
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    message handler
//
// DESCRIPTION:
//    User registered message handler
//
// INPUTS:
//    r0 - command ID
//   r1 - Command/Response data pointer
//   r2 - command data length 
// OUTPUTS:
//    r8 - response data length
//
// *****************************************************************************

.MODULE $M.music_example.ReInit;

   .CODESEGMENT PM;

func:
   r8 = 1;
   M[$music_example.reinit] = r8;
   r8 = 0;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    message handler
//
// DESCRIPTION:
//    User registered message handler
//
// INPUTS:
//    r0 - command ID
//   r1 - Command/Response data pointer
//   r2 - command data length 
// OUTPUTS:
//    r8 - response data length
//
// *****************************************************************************

.MODULE $M.music_example.GetControl;

   .CODESEGMENT PM;

func:
   // push rLink onto stack
   $push_rLink_macro;
      
   // SysControl
   // OvrDACgain
   // OvrMode   
   I4 = &$music_example.SpiSysControl;
   I0 = r1;
   r10 = LENGTH($music_example.SpiSysControl);
   do lp_copy_control;
      r0 = M[I0,1];
      M[I4,1] = r0;
lp_copy_control:

// Zero out right output cbuffer only if MONO mode is selected in Music Manager
   r4 = 1;
   M[$audio_out_tone_upsample_stereo_mix.param + $cbops.auto_resample_mix.IO_RIGHT_INDEX_FIELD] = r4;
   r0 = M[$music_example.OvrMode];
   Null = r0 - $music_example.SYSMODE.MONO;
   if NEG jump done_zero_right_cbuffer_output;  
      // no tone mixing to the right channel
      M[$audio_out_tone_upsample_stereo_mix.param + $cbops.auto_resample_mix.IO_RIGHT_INDEX_FIELD] = -r4;
      r3 = Null;
      
      // Zero out DC estimate on right channel dc_remove operator when switching into mono mode
      // in order to prevent the transient DC estimate from being written into the $dac_out_right
      // cbuffer. The transient DC estimate causes a buzz since the $dac_out_right cbuffer is
      // no longer updated but the data in this cbuffer is still fed to the DAC port.
      M[&$M.main.audio_out_dc_remove_op_right.param + $cbops.dc_remove.DC_ESTIMATE_FIELD] = r3;      

      r10 = LENGTH($dac_out_right);
      I0 = &$dac_out_right;
      do clear_right_cbuffer_output;
        M[I0,1] = r3;
      clear_right_cbuffer_output:
   done_zero_right_cbuffer_output:

   r0 = M[$music_example.SysControl];
   NULL = r0 AND $M.music_example.CONTROL.DAC_OVERRIDE;
   if Z jump dontupdateDAC;
   r2 = M[$music_example.OvrDACgain];
   if NEG jump dontupdateDAC;
   r3 = r2 AND 0xf;
   M[$music_example.CurDacL] = r3;
   r2 = r2 LSHIFT -8;
   r4 = r2 AND 0xf;
   M[$music_example.CurDacR] = r4;
   
   r2 = $music_example.VMMSG.CODEC;
   call $message.send_short;   
dontupdateDAC:   
   r8 = 0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    message handler
//
// DESCRIPTION:
//    User registered message handler
//
// INPUTS:
//    r0 - command ID
//   r1 - Command/Response data pointer
//   r2 - command data length 
// OUTPUTS:
//    r8 - response data length
//
// *****************************************************************************
.MODULE $M.music_example.GetStatus;

   .CODESEGMENT PM;

func:   
   // Pointer to Payload
   I1 = r1; 
   // Copy Status
   r10 = $music_example.STATUS.BLOCK_SIZE;
   // Payload Size
   r8 = r10;  
   I4 = &$M.system_config.data.StatisticsPtrs;  
   r1 = M[I4,1];
   do lp_copy_status;   
      // Dereference Pointer
      r1 = M[r1];                  
      // Copy, Next POinter
      M[I1,1] = r1, r1 = M[I4,1];    
      // Bug in BC3MM in index register feed forward     
      nop;  
lp_copy_status:
   // Clear Peak Statistics
   r10 = LENGTH($music_example.Statistics);
   I4 = &$music_example.Statistics;
   r2 = r2 XOR r2;
   do loop_clr_statistics;
      M[I4,1] = r2;
loop_clr_statistics:
   rts;
.ENDMODULE;
   
