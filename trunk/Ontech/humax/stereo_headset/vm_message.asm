// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc 2008             http://www.csr.com
// %%version
//
// $Revision$ $Date$
// *****************************************************************************

.include "music_example.h"
.include "stack.h"
.include "pskey.h"
.include "message.h"

// VM Message Handlers
.MODULE $M.music_example_message;
   .DATASEGMENT DM;
   .VAR set_mode_message_struc[$message.STRUC_SIZE];
   .VAR volume_message_struc[$message.STRUC_SIZE];
   .VAR set_param_message_struc[$message.STRUC_SIZE];    
   .VAR ping_message_struc[$message.STRUC_SIZE];
   .VAR get_param_message_struc[$message.STRUC_SIZE];
   .VAR load_params_message_struc[$message.STRUC_SIZE];
   .VAR ps_key_struc[$pskey.STRUC_SIZE]; 
.ENDMODULE;

// *****************************************************************************
// DESCRIPTION: $MsgMusicSetMode
//       handle mode change
//  r1 = mode
//  r2 = eq curve state
//       00 = 0 = Enabled, User Curve #1
//       01 = 1 = Bypassed
//       10 = 2 = Enabled, User Curve #2
//  r3 = TBD
// *****************************************************************************
.MODULE $M.music_example_message.SetMode;

      .CODESEGMENT   PM;

func:

   r5 = M[&$M.system_config.data.CurParams + $music_example.PARAMETERS.OFFSET_CONFIG];
   // ensure that only desired bits of config word are compared against
   r6 = r5 AND $music_example.EQCONFIG_MASK1;
   
   r2 = r2 LSHIFT $music_example.EQCONFIG_SHIFT;
   // XOR the incoming bits with current config bits
   // to determine if they changed
   r6 = r6 XOR r2;
   if Z jump dont_update_config;
      // zero out the appropriate bits   
      r7 = r5 AND $music_example.EQCONFIG_MASK2;
      // set the appropriate bits      
      r7 = r7 OR r2;
      M[&$M.system_config.data.CurParams + $music_example.PARAMETERS.OFFSET_CONFIG] = r7;
dont_update_config:

   // ensure mode is valid    
   r3 = $music_example.SYSMODE.LAST_MODE;
   Null = r3 - r1;   
   if NEG r1 = r3;                  
   r3 = $music_example.SYSMODE.STANDBY;
   Null = r3 - r1;
   if POS r1 = r3;
   // save mode
   r3 = M[$music_example.sys_mode];
   M[$music_example.sys_mode] = r1;
   // Re-Init only on mode change      
   r3 = r3 - r1;  
   r1 = M[$music_example.reinit];
   r1 = r1 OR r3;
   // check to see if config word changed
   r1 = r1 OR r6;
   M[$music_example.reinit] = r1;
   rts; 
.ENDMODULE;

// *****************************************************************************
// DESCRIPTION: $MsgMusicSetParameter
//       handle set parameter
//  r1 = Parameter ID
//  r2 = Parameter MSW
//  r3 = Parameter LSW
//  r4 = Status (NZ=done)
// *****************************************************************************
.MODULE $M.music_example_message.SetParam;

   .CODESEGMENT PM;

func:
   Null = r1;
   if NEG rts;
   Null = r1 - $music_example.PARAMETERS.STRUCT_SIZE;
   if POS rts;
      // Save Parameter 
      r3 = r3 AND 0xFFFF;
      r2 = r2 LSHIFT 16;
      r2 = r2 OR r3;
      M[$M.system_config.data.CurParams + r1] = r2;
      // Check status
      Null = Null + r4;
      if NZ jump $music_example_reinitialize;
      // Set Mode to standby
      r8 = $music_example.SYSMODE.STANDBY;
      M[$music_example.sys_mode] = r8;
      rts;
.ENDMODULE;

// *****************************************************************************
// DESCRIPTION: Read Parameter
//       handle get parameter 
// INPUTS:  
//    r1 = Parameter ID
// Response
//    P0 = requested ID
//    P1 = returned ID, 0 if request was invalid
//    P2 = MSW
//    P3 = LSW
// *****************************************************************************
.MODULE $M.music_example_message.GetParam;

   .CODESEGMENT PM;

func:
   // Validate Request
   // P0
   r3 = r1;          
   // P1
   r4 = r1;          
   if NEG r4 = Null;
   Null = r1 - $music_example.PARAMETERS.STRUCT_SIZE;
   if POS r4 = Null;
   r6 = M[$M.system_config.data.CurParams + r4];
   // MSW   : P2
   r5 = r6 LSHIFT -16;     
   // LSW   : P3
   r6 = r6 AND 0xFFFF;     
   // push rLink onto stack
   $push_rLink_macro;
   // Send GET RESPONSE Message
   r2 = $music_example.VMMSG.GETPARAM_RESP;
   call $message.send_short;
   // pop rLink from stack
   jump $pop_rLink_and_rts;
   rts;
.ENDMODULE;

.if 0
// *****************************************************************************
// DESCRIPTION: $MsgCvcPing
//       Ping from VM - Verify Operation
// *****************************************************************************
.MODULE $M.music_example_message.Ping;

   .CODESEGMENT   PM;

func:
   // push rLink onto stack
   $push_rLink_macro;
   // Send PING Message
   r2 = $M.CVC.VMMSG.PINGRESP;
   r3 = M[$M.CVC_HEADSET.FrameCounter];
   call $message.send_short;
   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;
.endif
// *****************************************************************************
// DESCRIPTION: $MsgMusicExampleSetVolume
//       handle call state mode
//  r1 = N/A
//  r2 = N/A      
//  r3 = DAC L gain
//  r4 = DAC R gain
// *****************************************************************************
.MODULE $M.music_example_message.Volume;

   .CODESEGMENT   PM;

func:
   
   // Limit Gains
   r5 = 0xf;
   r3 = r3 AND r5;   
   M[$music_example.CurDacL] = r3; 
   r4 = r4 AND r5;   
   M[$music_example.CurDacR] = r4; 

   // push rLink onto stack
   $push_rLink_macro;
   
   r2 = $music_example.VMMSG.CODEC;
   call $message.send_short;

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// DESCRIPTION: $LoadParams
// r1 = PsKey Address containing Music Example parameters  
// *****************************************************************************
.MODULE $M.music_example.LoadParams;

   .CODESEGMENT PM;    
   
func:
   $push_rLink_macro;
   // Set Mode to standby
   r8 = $music_example.SYSMODE.STANDBY;
   M[$music_example.sys_mode] = r8;
       
   // Copy default parameters into current parameters
   call $M.music_example.load_default_params.func;  
   // Save for SPI Status
//   M[$M.CVC_HEADSET.Last_PsKey] = r1;  
   r2 = r1;          
TestPsKey:
   if Z jump done;
      // r2 = key;
      //  &$friendly_name_pskey_struc;
      r1 = &$M.music_example_message.ps_key_struc;        
      // &$DEVICE_NAME_pskey_handler;
      r3 = &$M.music_example.PsKeyReadHandler.func;  
      call $pskey.read_key;
      jump $pop_rLink_and_rts; 
done:
   // copy codec config word to current config word
   r0 = M[&$M.system_config.data.CurParams + $music_example.CODEC_CONFIG];
   M[&$M.system_config.data.CurParams + $music_example.PARAMETERS.OFFSET_CONFIG] = r0;
   call $M.music_example.ReInit.func;
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// DESCRIPTION: $PsKeyReadHandler
//  INPUTS:
//    Standard (short) message mode:
//    r1 = Key ID
//    r2 = Buffer Length; $pskey.FAILED_READ_LENGTH on failure
//    r3 = Payload.  Key ID Plus data
// *****************************************************************************
.MODULE $M.music_example.PsKeyReadHandler;

   .CODESEGMENT PM;

func:
   $push_rLink_macro;
   
   // error checking - check if read failed
   // if so, DSP default values will be used instead of PsKey values.
   Null = r2 - $pskey.FAILED_READ_LENGTH;
   if Z jump $M.music_example.LoadParams.done;
   // Adjust for Key Value in payload?
   I0 = r3 + 1;       
   r10 = r2 - 1;
   // Clear sign bits
   // I2=copy of address  
   I2 = I0;             
   // r3=mask to clear sign extension
   r3 = 0x00ffff;       
   do loop1;
      r0 = M[I2,0];
      r0 = r0 AND r3;
      M[I2,1] = r0;
loop1:

   // error checking - make sure first value is the Pskey address.
   // if not, DSP default values will be used instead of PsKey values.
   r0 = M[I0,1];
   Null = r1 - r0;
   if NZ jump $M.music_example.LoadParams.done;
   
   // get next Pskey address
   // r5 = address of next PsKey      
   r5 = M[I0,1];              
   r0 = M[I0,1];        
   // r4 = NumParams (last parameter + 1)
   r4 = r0 AND 0xff;          
   // r0 = firstParam (zero-based index into
   //      paramBlock)
   r0 = r0 LSHIFT -8;         
                              
   // initial mask value                     
   r8 = Null;  

start_loop:
      r8 = r8 LSHIFT -1;
      if NZ jump withinGroup;
      // group
      r3 = M[I0,1];              
      // mask value
      r8 = 0x8000;               
      // used for odd variable
      r7 = Null;                 
withinGroup:
      Null = r3 AND r8;
      if Z jump dontOverwriteCurrentValue;
         // overwrite current parameter
         r7 = r7 XOR 0xffffff;  
         if Z jump SomeWhere;  
         // MSB for next two parameters
         r2 = M[I0,1];              
         // MSB for param1
         r6 = r2 LSHIFT -8;         
         jump SomeWhereElse;
SomeWhere:
         // MSB for param2
         r6 = r2 AND 0xff;          
SomeWhereElse:
         // LSW
         r1 = M[I0,1];              
         r6 = r6 LSHIFT 16;
         // Combine MSW and LSW
         r1 = r1 OR r6;             
         M[$M.system_config.data.CurParams + r0] = r1;
dontOverwriteCurrentValue:   
      r0 = r0 + 1;
      Null = r0 - r4;
   if NEG jump start_loop;
   // PS Key Being requested
   r2 = r5;                   
   jump $M.music_example.LoadParams.TestPsKey;

.ENDMODULE;
