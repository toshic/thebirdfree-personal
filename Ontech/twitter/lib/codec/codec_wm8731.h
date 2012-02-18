/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731.h
    
DESCRIPTION
	Header file for the Wolfson WM8731 codec library. Used to set up and
	configure the device.
*/

#ifndef _CODEC_WM8731_H_
#define _CODEC_WM8731_H_


#include "codec.h"

#include <csrtypes.h>


/* vol parameter of wm8731Set[Left|Right]LineIn */
#define WM8731_LINEIN_0DB 23

/* vol parameter of wm8731Set[Left|Right]HeadphoneOut */
#define WM8731_HEADPHONE_0DB 121

/* sideatt parameter of wm8731SetAnaloguePath */
#define WM8731_SIDEATT_MINUS15DB 3
#define WM8731_SIDEATT_MINUS12DB 2
#define WM8731_SIDEATT_MINUS9DB  1
#define WM8731_SIDEATT_MINUS6DB  0

/* deemp parameter of wm8731SetDigitalPath */
#define WM8731_DEEMP_48K     3
#define WM8731_DEEMP_44K1    2
#define WM8731_DEEMP_32K     1
#define WM8731_DEEMP_DISABLE 0

/* format parameter of wm8731SetDigitalFormat */
#define WM8731_FORMAT_DSP   3
#define WM8731_FORMAT_I2S   2
#define WM8731_FORMAT_LEFT  1
#define WM8731_FORMAT_RIGHT 0

/* iwl parameter of wm8731SetDigitalFormat */
#define WM8731_IWL_32 3
#define WM8731_IWL_24 2
#define WM8731_IWL_20 1
#define WM8731_IWL_16 0

/* mode parameter of wm8731SetSamplingControl */
#define WM8731_MODE_USB    1
#define WM8731_MODE_NORMAL 0


void wm8731Init(wolfson_init_params *pio);
void wm8731SetLeftLineIn(wolfson_init_params *pio, uint16 vol, uint16 mute, uint16 inboth);
void wm8731SetRightLineIn(wolfson_init_params *pio, uint16 vol, uint16 mute, uint16 inboth);
void wm8731SetLeftHeadphoneOut(wolfson_init_params *pio, uint16 vol, uint16 lzcen, uint16 hpboth);
void wm8731SetRightHeadphoneOut(wolfson_init_params *pio, uint16 vol, uint16 rzcen, uint16 hpboth);
void wm8731SetAnaloguePath(wolfson_init_params *pio, uint16 micboost, uint16 mutemic, uint16 insel, uint16 bypass,
						   uint16 dacsel, uint16 sidetone, uint16 sideatt);
void wm8731SetDigitalPath(wolfson_init_params *pio, uint16 adchpd, uint16 deemp, uint16 dacmu, uint16 hpor);
void wm8731SetPowerDown(wolfson_init_params *pio, uint16 lineinpd, uint16 micpd, uint16 adcpd, uint16 dacpd,
						uint16 outpd, uint16 oscpd, uint16 clkoutpd, uint16 poweroff);
void wm8731SetDigitalFormat(wolfson_init_params *pio, uint16 format, uint16 iwl, uint16 lrp, uint16 lrswap,
							uint16 ms, uint16 bclkinv);
void wm8731SetSamplingControl(wolfson_init_params *pio, uint16 mode, uint16 bosr, uint16 sr, uint16 clkidiv2, uint16 clkodiv2);
void wm8731SetActive(wolfson_init_params *pio, uint16 active);
void wm8731Reset(wolfson_init_params *pio);


#endif /* _CODEC_WM8731_H_ */
