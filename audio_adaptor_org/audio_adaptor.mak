######################################################################################################
##### AUDIO ADAPTOR
#####
##### Copy DSP images depending on MP3 encoder configuration
######################################################################################################

ifneq (, $(findstring -DINCLUDE_MP3_ENCODER_PLUGIN,$(DEFS)))

# ensure we always go and get the latest from the DSP app 
# 
.PHONY : image/mp3_encoder/mp3_encoder.kap \
	image/mp3_encoder/000 \
	image/mp3_encoder/001 \
	image/sbc_encoder/sbc_encoder.kap  \
	image/sbc_encoder/000
#
# the sbc_encoder algorithms come from the dsp apps included as part of the project
image/sbc_encoder/sbc_encoder.kap :
	$(mkdir) image/sbc_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\sbc_encoder.kap $@
image/sbc_encoder/000 :
	$(mkdir) image/sbc_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\000 $@
#
# the mp3_encoder algorithms come from the dsp apps included as part of the project
image/mp3_encoder/mp3_encoder.kap :
	$(mkdir) image/mp3_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\mp3_encoder.kap $@
#
image/mp3_encoder/000 :
	$(mkdir) image/mp3_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\000 $@
#
image/mp3_encoder/001 :
	$(mkdir) image/mp3_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\001 $@
#
image.fs : image/mp3_encoder/mp3_encoder.kap \
	image/mp3_encoder/000 \
	image/mp3_encoder/001 \
	image/sbc_encoder/sbc_encoder.kap  \
	image/sbc_encoder/000

else

.PHONY : image/sbc_encoder/sbc_encoder.kap  \
	image/sbc_encoder/000  

# the sbc_encoder algorithms come from the dsp apps included as part of the project
image/sbc_encoder/sbc_encoder.kap : 
	$(mkdir) image/sbc_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\sbc_encoder.kap $@
image/sbc_encoder/000 : 
	$(mkdir) image/sbc_encoder
	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\000 $@	
	
image.fs : image/sbc_encoder/sbc_encoder.kap  \
	image/sbc_encoder/000  \
	| remove_mp3
	
remove_mp3 : 
	$(del) image/mp3_encoder/mp3_encoder.kap  $(del) image/mp3_encoder/000 $(del) image/mp3_encoder/001

endif


keys ::
	-$(pscli) $(SPI) -m $(OUTPUT)_base.psr
    

ifneq (,$(findstring -DDEV_PC_1645_USB,$(DEFS)))
TRANSPORT=usb_vm
keys ::
	-$(pscli) $(SPI) -m $(OUTPUT)_usb.psr
endif

ifneq (,$(findstring -DDEV_PC_1645_ANALOGUE,$(DEFS)))
TRANSPORT=none
keys ::
	-$(pscli) $(SPI) -m $(OUTPUT)_analogue.psr
endif  

ifneq (,$(findstring -DRED_PC_141,$(DEFS)))
TRANSPORT=usb_vm
keys ::
	-$(pscli) $(SPI) -m $(OUTPUT)_usb.psr
endif

ifneq (,$(findstring -DRED_PC_142,$(DEFS)))
TRANSPORT=none
keys ::
	-$(pscli) $(SPI) -m $(OUTPUT)_analogue.psr
endif

