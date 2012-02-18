/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#ifndef _CODEC_WM8731_PRIVATE_H_
#define _CODEC_WM8731_PRIVATE_H_


#define WM8731_LEFTLINEIN_REG 0
#define WM8731_RIGHTLINEIN_REG 1
#define WM8731_LEFTHEADPHONE_REG 2
#define WM8731_RIGHTHEADPHONE_REG 3
#define WM8731_ANALOGUEPATH_REG 4
#define WM8731_DIGITALPATH_REG 5
#define WM8731_POWERDOWN_REG 6
#define WM8731_DIGITALFORMAT_REG 7
#define WM8731_SAMPLING_REG 8
#define WM8731_ACTIVE_REG 9
#define WM8731_RESET_REG 15


void wm8731WriteReg(wolfson_init_params *pio, uint16 addr, uint16 data);


#endif /* _CODEC_WM8731_PRIVATE_H_ */
