SET BUILD_ENV=D:\Stereo-Headset-SDK-2009.R2

%BUILD_ENV%\tools\bin\pauseonerror.exe %BUILD_ENV%\tools\bin\make.exe -R BLUELAB=%BUILD_ENV%\tools -R LIBRARY_VERSION=HumaxLib %1 %2 install doxygen
