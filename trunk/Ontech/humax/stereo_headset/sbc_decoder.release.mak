###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: sbc_decoder
# Configuration: Release
# Generated: �� 5 26 22:26:49 2012
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=sbc_decoder
OUTDIR=D:/Stereo-Headset-SDK-2009.R2/apps/Humax
DEFS=-DAUDIO_CBUFFER_SIZE=320 -DCODEC_CBUFFER_SIZE=4096 -DGOOD_WORKING_BUFFER_LEVEL=0.65 -DSELECTED_CODEC_SBC -DHARDWARE_RATE_MATCHx 
HARDWARE=kalimba
BOOTSTRAP=1
LIBS=core cbops sbc codec frame_sync audio_proc spi_comm math 
ASMS=\
      codec_decoder.asm\
      stream_copy.asm\
      music_example_config.asm\
      music_example_system.asm\
      spi_message.asm\
      vm_message.asm\
      sr_adjustment.asm
DEBUGTRANSPORT=SPITRANS=LPT SPIPORT=1

# Project-specific options
comfortnoisegain=0
debugtransport=[SPI|LPT1|No]
warpfilter=1
warpratehighcoefficient=0.01
warpratelowcoefficient=0.001
warpratemaxramp=0.005
warpratetransitionlevelwords=100
zerodataratebuffering=200000
zerodataratestopping=150000

-include sbc_decoder.mak
include $(BLUELAB)/Makefile.dsp
