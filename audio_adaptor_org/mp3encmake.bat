@echo off
REM rewrite the app makefile to support the mp3 encoder

ren audio_adaptor.mak audio_adaptor.mak_no_mp3

echo # ensure we always go and get the latest from the DSP app >> audio_adaptor.mak
echo # >> audio_adaptor.mak
echo .PHONY : image/mp3_encoder/mp3_encoder.kap \>> audio_adaptor.mak
echo 	image/mp3_encoder/000 \>> audio_adaptor.mak
echo 	image/mp3_encoder/001 \>> audio_adaptor.mak
echo 	image/sbc_encoder/sbc_encoder.kap  \>> audio_adaptor.mak
echo 	image/sbc_encoder/000>> audio_adaptor.mak
echo #>> audio_adaptor.mak
echo # the sbc_encoder algorithms come from the dsp apps included as part of the project>> audio_adaptor.mak
echo image/sbc_encoder/sbc_encoder.kap :>> audio_adaptor.mak
echo 	$(mkdir) image/sbc_encoder>> audio_adaptor.mak
echo 	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\sbc_encoder.kap $@>> audio_adaptor.mak
echo image/sbc_encoder/000 :>> audio_adaptor.mak
echo 	$(mkdir) image/sbc_encoder>> audio_adaptor.mak
echo 	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\sbc_encoder\000 $@>> audio_adaptor.mak
echo #>> audio_adaptor.mak
echo # the mp3_encoder algorithms come from the dsp apps included as part of the project>> audio_adaptor.mak
echo image/mp3_encoder/mp3_encoder.kap :>> audio_adaptor.mak
echo 	$(mkdir) image/mp3_encoder>> audio_adaptor.mak
echo 	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\mp3_encoder.kap $@>> audio_adaptor.mak
echo #>> audio_adaptor.mak
echo image/mp3_encoder/000 :>> audio_adaptor.mak
echo 	$(mkdir) image/mp3_encoder>> audio_adaptor.mak
echo 	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\000 $@>> audio_adaptor.mak
echo #>> audio_adaptor.mak
echo image/mp3_encoder/001 :>> audio_adaptor.mak
echo 	$(mkdir) image/mp3_encoder>> audio_adaptor.mak
echo 	$(copyfile) ..\..\kalimba\apps\audio_adaptor\image\mp3_encoder\001 $@>> audio_adaptor.mak
echo #>> audio_adaptor.mak
echo image.fs : image/mp3_encoder/mp3_encoder.kap \>> audio_adaptor.mak
echo 	image/mp3_encoder/000 \>> audio_adaptor.mak
echo 	image/mp3_encoder/001 \>> audio_adaptor.mak
echo 	image/sbc_encoder/sbc_encoder.kap  \>> audio_adaptor.mak
echo 	image/sbc_encoder/000>> audio_adaptor.mak
