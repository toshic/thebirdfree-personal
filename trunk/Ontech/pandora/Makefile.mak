###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: audio_adaptor
# Configuration: Release
# Generated: �� 12 18 14:40:33 2010
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=audio_adaptor
OUTDIR=D:/CSR_SDK/Audio-Adaptor-SDK-2009.R1/apps/audio_adaptor
HARDWARE_INDEX=4
DEFS=-DUSER_CONFIGURE_CODEC -DDEV_PC_1645_ANALOGUE -DxENABLE_DEBUG -DNO_CHARGER_TRAPS -DKAL_MSG 

DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0
EXECUTION_MODE=native
STACKSIZE=0
TRANSPORT=raw
FIRMWARE=unified
HARDWARE=kalimba
FLASHSIZE=8192
BUILD_MERGE=merge
PANIC_ON_PANIC=1
FIRMWAREIMAGE=
LIBRARY_VERSION=

LIBS=-la2dp -laghfp -laudio -lavrcp -lcodec -lbattery -lbdaddr -lconnection -lcsr_tone_plugin -lcsr_sbc_encoder_plugin -lregion -lservice -lcsr_faststream_source_plugin -lcsr_sco_loopback_plugin_debug -lcsr_mp3_encoder_plugin -lgoep -lgoep_apphdrs -lmd5 -lpbaps -lpbap_common -lsdp_parse -lcsr_cvsd_usb_no_dsp_plugin -lcsr_common_no_dsp_plugin -lspp
#LIBS+= -lsyncs
INPUTS=\
      audio_adaptor.mak\
      leds.led\
      audio_adaptor_base.psr\
      audioAdaptor_buttons.button\
      audio_adaptor_analogue.psr\
      audio_adaptor_usb.psr\
      main.c\
      audioAdaptor_a2dp_stream_control.c\
      audioAdaptor_a2dp_msg_handler.c\
      audioAdaptor_aghfp_msg_handler.c\
      audioAdaptor_aghfp_call.c\
      audioAdaptor_aghfp_slc.c\
      audioAdaptor_avrcp_msg_handler.c\
      audioAdaptor_scan.c\
      audioAdaptor_sys_handler.c\
      audioAdaptor_cl_msg_handler.c\
      audioAdaptor_event_handler.c\
      audioAdaptor_streammanager.c\
      audioAdaptor_profile_slc.c\
      audioAdaptor_init.c\
      audioAdaptor_a2dp_slc.c\
      audioAdaptor_avrcp_slc.c\
      audioAdaptor_statemanager.c\
      audioAdaptor_codec_msg_handler.c\
      audioAdaptor_powermanager.c\
      leds.c\
      audioAdaptor_buttons.c\
      audioAdaptor_led.c\
      audioAdaptor_dev_instance.c\
      uart.c\
      at_cmd.c\
      buffer.c\
      folder.c\
      handle_pbap.c\
      handle_system.c\
      pb_access.c\
      state.c\
      vcard_gen.c\
	  SppServer.c\
      audioAdaptor_a2dp.h\
      audioAdaptor_a2dp_stream_control.h\
      audioAdaptor_a2dp_msg_handler.h\
      audioAdaptor_aghfp_msg_handler.h\
      audioAdaptor_aghfp_call.h\
      audioAdaptor_aghfp_slc.h\
      audioAdaptor_avrcp_msg_handler.h\
      audioAdaptor_scan.h\
      audioAdaptor_sys_handler.h\
      audioAdaptor_private.h\
      audioAdaptor_cl_msg_handler.h\
      audioAdaptor_events.h\
      audioAdaptor_event_handler.h\
      audioAdaptor_streammanager.h\
      audioAdaptor_profile_slc.h\
      audioAdaptor_init.h\
      audioAdaptor_a2dp_slc.h\
      audioAdaptor_avrcp_slc.h\
      audioAdaptor_statemanager.h\
      audioAdaptor_states.h\
      audioAdaptor_codec_msg_handler.h\
      audioAdaptor_powermanager.h\
      leds.h\
      audioAdaptor_buttons.h\
      audioAdaptor_led.h\
      audioAdaptor_dev_instance.h\
      uart.h\
      at_cmd.h
# Project-specific options
type=1

-include audio_adaptor.mak
include $(BLUELAB)/Makefile.vm
