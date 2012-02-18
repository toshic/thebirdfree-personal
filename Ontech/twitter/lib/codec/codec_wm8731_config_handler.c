/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_config_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"

#ifndef CODEC_EXCLUDE_WOLFSON

#include "codec_private.h"
#include "codec_config_handler.h"
#include "codec_wm8731_config_handler.h"
#include "codec_wm8731.h"

#include <stdlib.h>


typedef struct
{
	int16 bosr;
	int16 sr;
} sample_rate_registers;

/* array of [adc][dac] (for use with sample_freq enum), returns corresponding 
   (bosr, sr) setting. These values are required in order to program the
   codec to use the correct sample rates. See the WM8731 spec for more info.
*/
static const sample_rate_registers lup[10][10] = { 
										{{0,3},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{1,10},{0,2},{-1,-1},{-1,-1}}, 		/* adc = 8kHz */ 
												
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}},	/* adc = 11_025kHz */
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}},	/* adc = 16kHz */
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}},	/* adc = 22_25kHz */
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}},	/* adc = 24kHz */
												
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{0,6},{-1,-1},{-1,-1},{-1,-1},{-1,-1}},	/* adc = 32kHz */ 
										{{1,9},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{1,8},{-1,-1},{-1,-1},{-1,-1}},		/* adc = 44_1kHz */ 
										{{0,1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{0,0},{-1,-1},{-1,-1}},		/* adc = 48kHz */ 
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{1,15},{-1,-1}},	/* adc = 88_2kHz */ 
										{{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{0,7}} 	/* adc = 96kHz */ 
};
			

/****************************************************************************
NAME	
	handleWm8731CodecConfigureReq

DESCRIPTION
	Handle internal codec configure request, for the Wolfson codec.
*/
void handleWm8731CodecConfigureReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_CONFIG_REQ_T *req)
{	
	uint16 micboost = 0;	/* 1 = Enable Boost 0 = Disable Boost */
	uint16 mutemic = 1;		/* 1 = Enable Mute 0 = Disable Mute */
	uint16 insel = 0;		/* 1 = Mic input to ADC 0 = Line input to ADC */
	uint16 bypass = 0;		/* 1 = Enable (use line input with output) 0 = Disable*/
	uint16 dacsel = 1;		/* 1 = Select DAC 0 = Don't select DAC */
	uint16 sidetone = 0;	/* 1 = Enable (use mic input with output) 0 = Disable */
	uint16 sideatt = WM8731_SIDEATT_MINUS6DB;	/* Side tone Attenuation	11 = -15dB 
																			10 = -12dB
																			01 = -9dB
																			00 = -6dB
												*/												  
	uint16 adchpd = 0;		/* 1 = Disable ADC High Pass Filter 0 = Enable */
	uint16 deemp = WM8731_DEEMP_DISABLE;		/* De-emphasis control by DAC	11 = 48kHz
												   								10 = 44.1kHz
																				01 = 32kHz
																				00 = Disable
												*/
	uint16 dacmu = 0;		/* 1 = DAC soft mute control Enable 0 = Disable	*/
	uint16 hpor = 0;		/* 1 = Store dc offset then High Pass Filter disabled 0 = Clear offset */
	
	codec_config_params *config = req->config;
	sample_rate_registers sample_registers;
	input_type inputs = config->inputs;
	uint16 outputs = config->outputs;
	
	bool powerDownDAC = FALSE;
	bool powerDownADC = FALSE;
	bool powerDownOutput = FALSE;
	bool powerDownMicIn = FALSE;
	bool powerDownLineIn = FALSE;
	
	if ((config->inputs < 0) || (config->inputs > no_input))
	{
		sendCodecConfigureCfm(codec->clientTask, codec_invalid_configuration);
		free(req->config);
		return;
	}
		
	/* We must set an ADC and a DAC sample rate so, if one of them is not supplied
	   we just set both to have the same sample rate, as we know this is a 
	   valid combination. */
	if ((config->adc_sample_rate < 0) || (config->adc_sample_rate >= sampleNotUsed))
		config->adc_sample_rate = config->dac_sample_rate;
	if ((config->dac_sample_rate < 0) || (config->dac_sample_rate >= sampleNotUsed))
	{
		if (config->dac_sample_rate == config->adc_sample_rate)
			config->adc_sample_rate = sample48kHz;
			
		config->dac_sample_rate = config->adc_sample_rate;
	}		
	
	sample_registers = lup[config->adc_sample_rate][config->dac_sample_rate];

	if (config->dac_sample_rate == sample48kHz)
		deemp = WM8731_DEEMP_48K;
	else if (config->dac_sample_rate == sample44_1kHz)
		deemp = WM8731_DEEMP_44K1;
	else if (config->dac_sample_rate == sample32kHz)
		deemp = WM8731_DEEMP_32K;
	else
		deemp = WM8731_DEEMP_DISABLE;
		
	free(req->config);
	
	/* Only positive numbers from the look up table are valid sample rate combinations */
	if ((sample_registers.bosr < 0) || (sample_registers.sr < 0))
	{
		sendCodecConfigureCfm(codec->clientTask, codec_invalid_sample_rates);
		return;
	}
	
	wm8731Init(codec->init_params);
	wm8731SetActive(codec->init_params, 0);
	wm8731SetDigitalFormat(codec->init_params, 
                           WM8731_FORMAT_I2S,
						   WM8731_IWL_24,	
						   0,
						   0,
						   0,
						   0);
	wm8731SetSamplingControl(codec->init_params, WM8731_MODE_USB, sample_registers.bosr, sample_registers.sr, 0 , 0);
	
	if (!(outputs & OUTPUT_DAC))
	{
		/* don't select DAC */
		dacsel = 0;
		dacmu = 1;
		powerDownDAC = TRUE;
	}
	if (inputs == mic_input)
		/* mic input to ADC */
		insel = 1;
	if (inputs != no_input)
		/* Disable mute on input to ADC */
		mutemic = 0;
	else
		powerDownADC = TRUE;
	
	if ((outputs & OUTPUT_MICIN))
		sidetone = 1;
	if ((outputs & OUTPUT_LINEIN))
		bypass = 1;
	
	if (!(outputs & OUTPUT_MICIN) && (inputs != mic_input))
		powerDownMicIn = TRUE;
	if (!(outputs & OUTPUT_LINEIN) && (inputs != line_input))
		powerDownLineIn = TRUE;
	if (!outputs)
	{
		dacmu = 1;
		powerDownOutput = TRUE;
	}
	
	wm8731SetAnaloguePath(codec->init_params, micboost, mutemic, insel, bypass, dacsel, sidetone, sideatt);
	wm8731SetDigitalPath(codec->init_params, adchpd, deemp, dacmu, hpor);
	
	/* Power down things not in use:
	   			-line input
				-mic input
				-ADC
				-DAC
				-line output
	*/
	wm8731SetPowerDown(codec->init_params, powerDownLineIn,powerDownMicIn,powerDownADC,powerDownDAC,powerDownOutput,0,0,0);
    /* 
	wm8731SetPowerDown(codec->init_params, 0,0,0,0,0,0,0,0);
	*/
	wm8731SetActive(codec->init_params, 1);

	sendCodecConfigureCfm(codec->clientTask, codec_success);

}

#endif /* CODEC_EXCLUDE_WOLFSON */
