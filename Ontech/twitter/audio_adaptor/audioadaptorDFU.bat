rem Move to the directory of audio adaptor application directory,
rem Run this batch file is called as follow:
rem audioadaptorDFU.bat audio_adaptor_usb_141 c:\Audio-Adaptor-SDKRC3.1\

set v_rename=%1
set v_bluelab=%2

set v_vid=0x0a12
set v_pid=0xffff

set v_string="Audio Adaptor %v_rename% release with application and persistent store"

@echo.
@rem -------------------------------------------------------------------------------------
@echo. 
@rem -------------------------------------------------------------------------------------
@echo.
@rem - the Audio Adaptor BC5-MM dfu builds

copy audio_adaptor_base.psr+%v_rename%.psr temp.psr
%v_bluelab%\tools\dfu\dfubuild -v -pedantic -f %v_rename%.dfu -uv %v_vid% -up %v_pid% -ui %v_string% -s %v_bluelab%\firmware\vm\unified\elvis\stack_unsigned.xpv -p3 temp.psr . . -h image.fs
del temp.psr


