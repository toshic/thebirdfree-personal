SET BUILD_ENV=D:\Audio-Adaptor-SDK-2009.R1

%BUILD_ENV%/tools/bin/make -R BLUELAB=%BUILD_ENV%/tools -f makefile.mak build

@if errorlevel == 1 goto end
%BUILD_ENV%/tools/bin/BlueFlashCmd -TRANS "SPITRANS=LPT SPIPORT=1" merge

:end