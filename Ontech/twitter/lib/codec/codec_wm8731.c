/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    wm8731.c
    
DESCRIPTION
	Wolfson WM8731 codec library.
*/

#include "codec.h"
#ifndef CODEC_EXCLUDE_WOLFSON

#include <stdio.h>

#include <pio.h>
#include "codec_wm8731.h"
#include "codec_wm8731_private.h"

#include "codec_private.h"

void wm8731Init(wolfson_init_params *pio)
{
	/* Set PIO lines for output */
	PioSetDir(pio->csb | pio->sclk | pio->sdin, ~0);
	
	wm8731Reset(pio);

        /* Register R7 */
    wm8731SetDigitalFormat(pio,
        WM8731_FORMAT_I2S, /* I2S format */
        WM8731_IWL_24,  /* 24 bit */
        1,
        0,
        0,
        0);

    /* Register R6  */
    wm8731SetPowerDown(pio, 1, 1, 1, 1, 1, 1, 1, 0);

    wm8731SetActive(pio, 1);
}


/* Setup the Left Line In register */
void wm8731SetLeftLineIn(wolfson_init_params *pio, uint16 vol, uint16 mute, uint16 inboth)
{
	uint16 data;
	
	data = (vol & 0x001F) | ((mute & 0x0001) << 7) | ((inboth & 0x0001) << 8);
    
	wm8731WriteReg(pio, WM8731_LEFTLINEIN_REG, data);
}


/* Setup the Right Line In register */
void wm8731SetRightLineIn(wolfson_init_params *pio, uint16 vol, uint16 mute, uint16 inboth)
{
	uint16 data;
	
	data = (vol & 0x001F) | ((mute & 0x0001) << 7) | ((inboth & 0x0001) << 8);
    
	wm8731WriteReg(pio, WM8731_RIGHTLINEIN_REG, data);
}


/* Setup the Left Headphone Out register */
void wm8731SetLeftHeadphoneOut(wolfson_init_params *pio, uint16 vol, uint16 lzcen, uint16 hpboth)
{
	uint16 data;
	
	data = (vol & 0x007F) | ((lzcen & 0x0001) << 7) | ((hpboth & 0x0001) << 8);
    
	wm8731WriteReg(pio, WM8731_LEFTHEADPHONE_REG, data);
}


/* Setup the Right Headphone Out register */
void wm8731SetRightHeadphoneOut(wolfson_init_params *pio, uint16 vol, uint16 rzcen, uint16 hpboth)
{
	uint16 data;
	
	data = (vol & 0x007F) | ((rzcen & 0x0001) << 7) | ((hpboth & 0x0001) << 8);
    
	wm8731WriteReg(pio, WM8731_RIGHTHEADPHONE_REG, data);
}


/* Setup the Analogue Audio Path Control register */
void wm8731SetAnaloguePath(wolfson_init_params *pio, uint16 micboost, uint16 mutemic, uint16 insel, uint16 bypass,
						   uint16 dacsel, uint16 sidetone, uint16 sideatt)
{
	uint16 data;
	
	data = (micboost & 0x0001) | ((mutemic & 0x0001) << 1) | ((insel & 0x0001) << 2) |
		   ((bypass & 0x0001) << 3) | ((dacsel & 0x0001) << 4) | ((sidetone & 0x0001) << 5) |
		   ((sideatt & 0x0003) << 6);
    
	wm8731WriteReg(pio, WM8731_ANALOGUEPATH_REG, data);
}


/* Setup the Digital Audio Path Control register */
void wm8731SetDigitalPath(wolfson_init_params *pio, uint16 adchpd, uint16 deemp, uint16 dacmu, uint16 hpor)
{
	uint16 data;
	
	data = (adchpd & 0x0001) | ((deemp & 0x0003) << 1) | ((dacmu & 0x0001) << 3) | ((hpor & 0x0001) << 4);
    
	wm8731WriteReg(pio, WM8731_DIGITALPATH_REG, data);
}


/* Setup the Power Down Control register */
void wm8731SetPowerDown(wolfson_init_params *pio, uint16 lineinpd, uint16 micpd, uint16 adcpd, uint16 dacpd,
						uint16 outpd, uint16 oscpd, uint16 clkoutpd, uint16 poweroff)
{
	uint16 data;
	
	data = (lineinpd & 0x0001) | ((micpd & 0x0001) << 1) | ((adcpd & 0x0001) << 2) |
		   ((dacpd & 0x0001) << 3) | ((outpd & 0x0001) << 4) | ((oscpd & 0x0001) << 5) |
		   ((clkoutpd & 0x0001) << 6) | ((poweroff & 0x0001) << 7);
    
	wm8731WriteReg(pio, WM8731_POWERDOWN_REG, data);
}


/* Setup the Digital Audio Interface Format register */
void wm8731SetDigitalFormat(wolfson_init_params *pio, uint16 format, uint16 iwl, uint16 lrp, uint16 lrswap,
							uint16 ms, uint16 bclkinv)
{
	uint16 data;
	
	data = (format & 0x0003) | ((iwl & 0x0003) << 2) | ((lrp & 0x0001) << 4) |
		   ((lrswap & 0x0001) << 5) | ((ms & 0x0001) << 6) | ((bclkinv & 0x0001) << 7);
    
	wm8731WriteReg(pio, WM8731_DIGITALFORMAT_REG, data);
}


/* Setup the Sampling Control register */
void wm8731SetSamplingControl(wolfson_init_params *pio, uint16 mode, uint16 bosr, uint16 sr, uint16 clkidiv2, uint16 clkodiv2)
{
	uint16 data;
	
	data = (mode & 0x0001) | ((bosr & 0x0001) << 1) | ((sr & 0x000F) << 2) |
		   ((clkidiv2 & 0x0001) << 6) | ((clkodiv2 & 0x0001) << 7);
    
	wm8731WriteReg(pio, WM8731_SAMPLING_REG, data);
}


/* Activate/deactivate the digital audio interface */
void wm8731SetActive(wolfson_init_params *pio, uint16 active)
{
	wm8731WriteReg(pio, WM8731_ACTIVE_REG, active ? 1 : 0);
}


/* Issue a reset command */
void wm8731Reset(wolfson_init_params *pio)
{
	wm8731WriteReg(pio, WM8731_RESET_REG, 0);
}


void wm8731WriteReg(wolfson_init_params *pio, uint16 addr, uint16 data)
{
	uint16 spidata, i;
    
    uint16 csb = pio->csb;   
    uint16 sdin = pio->sdin;
    uint16 sclk = pio->sclk;
    
	/* Combine address (7 bits) and data (9 bits) into a 16-bit word to send over SPI */
	spidata = ((addr & 0x007F) << 9) | (data & 0x01FF);
	
	/* Starting conditions: CSB low, SCLK low, SDIN don't care */
	PioSet(csb | sclk, 0);
		
	/* Clock out the data, MSB first */
	for (i = 0; i < 16; i++)
	{
		PioSet(sdin, (spidata & 0x8000) ? sdin : 0);
		PioSet(sclk, sclk);
		PioSet(sclk, 0);
		spidata <<= 1;
	}
	
	PioSet(csb, csb);	/* Take CSB high to latch data */
}

#endif /* CODEC_EXCLUDE_WOLFSON */
