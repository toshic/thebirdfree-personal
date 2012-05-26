SET BUILD_ENV=D:\Stereo-Headset-SDK-2009.R2

%BUILD_ENV%/tools/bin/make -R BLUELAB=%BUILD_ENV%/tools -f Makefile.mak clean
@if errorlevel == 1 goto end
%BUILD_ENV%/tools/bin/make -R BLUELAB=%BUILD_ENV%/tools -f Makefile.mak build
@if errorlevel == 1 goto end
pskey_inject

:end
