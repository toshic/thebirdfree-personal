/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec.h
    
DESCRIPTION 

    Header file for the codec library.  This library implements the
    functionality to be able to configure a stereo codec for use. Can define if
    the stereo inputs or outputs of the codec needs to be used, and the sample
    rates to be used for the ADCs and DACs.
	
	The library exposes a functional downstream API and an upstream message
	based API.
*/
/*!
    @file   codec.h  

    @brief Header file for the codec library.  This library implements the
    functionality to be able to configure a stereo codec for use. Can define if
    the stereo inputs or outputs of the codec needs to be used, and the sample
    rates to be used for the ADCs and DACs.
 
    The library exposes a functional downstream API and an upstream message
    based API.
*/
#ifndef CODEC_H_
#define CODEC_H_

#include <codec_.h>
#include <message_.h>

struct __WolfsonCodecTaskData;
struct __CsrInternalCodecTaskData;

/*!
    @brief The codec structures.
*/
typedef struct __WolfsonCodecTaskData WolfsonCodecTaskData;
typedef struct __CsrInternalCodecTaskData CsrInternalCodecTaskData;

/*!
    @brief Defines used to configure outputs in codec_config_params struct. 
   
    On some codecs you can feed the input directly to the output without any
    digital signal processing.
*/
#define OUTPUT_NONE			0x0
#define OUTPUT_DAC			0x1
#define OUTPUT_MICIN		0x2
#define OUTPUT_LINEIN		0x4

/*!
    @brief The upstream codec messages 
*/
#define CODEC_MESSAGE_BASE	0x6400

typedef enum
{
	CODEC_INIT_CFM 		= 	CODEC_MESSAGE_BASE,
    
	CODEC_CONFIGURE_CFM ,
    
    CODEC_MESSAGE_TOP
} CodecMessageId;

/*! 
	@brief The codecs channel being referred to, left, right or both.
*/
typedef enum
{
	/*! The left channel of the codec.*/
	left_ch,						
	/*! The right channel of the codec.*/
	right_ch,						
	/*! The left and right channel of the codec. */
	left_and_right_ch				
} codec_channel;

/*!
    @brief The status codes returned to the client application to indicate the
    status of a requested operation.
*/
typedef enum
{
	/*! The requested codec operation was a success. */ 
	codec_success = (0),           
	/*! The requested codec operation was a failure. */ 
	codec_fail,                    
	/*! The requested operation supplied invalid sample rates for this
	  codec. */ 
	codec_invalid_sample_rates,    
	/*! The requested operation supplied an invalid configuration. */ 
	codec_invalid_configuration,   
	/*! The requested operation was not supported for this codec. */ 
	codec_not_supported            
} codec_status_code;

/*!
    @brief The codec being used. 
    
    Returned to the client application on initialisation of the codec library.
*/
typedef enum
{
	codec_none,					/*!< The type of codec is not specified. */ 
	codec_wm8731,				/*!< The codec is the Wolfson Wm8731. */ 
	codec_csr_internal			/*!< The codec is the CSR internal. */ 
} codec_type;

/*!
    @brief The possible sample rates that the ADCs and DACs can be configured
    to.
*/
typedef enum
{
	sample8kHz = (0),
			
	sample11_025kHz,
	sample16kHz,
	sample22_25kHz,
	sample24kHz,
	
	sample32kHz,
	sample44_1kHz,
	sample48kHz,
	sample88_2kHz,
	sample96kHz,
	/* Leave sampleNotUsed as the last enum item */
	sampleNotUsed
} sample_freq;

/*!
    @brief Type of input to the codec ADCs that will be used.
*/
typedef enum
{
	/*! The input to the codec is microphone. */ 
	mic_input,						
	/*! The input to the codec is line. */ 
	line_input,						
	/*! No input to the codec. */ 
	no_input        /* Leave no_input as the last enum item */
} input_type;

/*!
    @brief The configuration used to set up the codec. 
    
    Define type of inputs and outputs, and the sample rates to use for the ADCs
    and DACs.
*/
typedef struct
{
	/*! Type of input to use with ADCs. */
	input_type inputs;				
	/*! Can output one or more of DACS (0x1), mic in (0x2), line in (0x4). */
	uint16 outputs;					
	/*! The sample rate to use for the ADC. */
	sample_freq adc_sample_rate;	
	/*! The sample rate to use for the DAC. */
	sample_freq dac_sample_rate;	
} codec_config_params;

/*!
    @brief The params used to initialise the Wolfson codec.
*/
typedef struct
{
    uint16 csb;                     /*!< The csb pin. Defaults to 0x0200. */
    uint16 sdin;                    /*!< The sdin pin. Defaults to 0x0400. */
    uint16 sclk;                    /*!< The sclk pin. Defaults to 0x0800. */
} wolfson_init_params;

/*!
    @brief This message returns the result of a call to CodecInitXXX.
*/
typedef struct
{
	codec_status_code status;      /*!< The current codec status. */
	codec_type type_of_codec;      /*!< The type of codec being used. */
	uint16 inputGainRange;         /*!< The max input gain for the codec. */
	uint16 outputGainRange;        /*!< The max output gain for the codec. */
	Task codecTask;                /*!< The codec task. */
} CODEC_INIT_CFM_T;

/*!
    @brief This message returns the result of a call to CodecConfigure.
*/
typedef struct
{
	codec_status_code status;		/*!< The current codec status. */
} CODEC_CONFIGURE_CFM_T;
	

/*!
    @brief Initialise the Wolfson Codec. 
   
    @param appTask The current application task.
    @param init The initialisation parameters to setup the codec with.
    
    CODEC_INIT_CFM message will be received by the application. 
*/
void CodecInitWolfson(Task appTask, wolfson_init_params *init);

/*!
    @brief Initialise the CSR Internal Codec.
   
    @param appTask The current application task.
   
    CODEC_INIT_CFM message will be received by the application. 
*/
void CodecInitCsrInternal(Task appTask);

/*!
    @brief Configure the Codec with the supplied parameters.
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.

    @param config The configuration parameters to setup the codec with.
   
    CODEC_CONFIGURE_CFM message will be received by the application. 
*/
void CodecConfigure(Task codecTask, const codec_config_params *config);

/*!
    @brief Set Codec Input Gain, left channel. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.

    @param volume The gain level (volume) to set the input channel to.

    @param channel The channel to use.
*/
void CodecSetInputGain(Task codecTask, uint16 volume, codec_channel channel);


/*!
    @brief Update the codec input gain immediately. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.

    @param volume The gain level (volume) to set the input channel to.

    @param channel The channel to use.
*/
void CodecSetInputGainNow(Task codecTask, uint16 volume, codec_channel channel);


/*!
    @brief Set Codec Output Gain. 
 
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.

    @param volume The gain level (volume) to set the output left channel to.

    @param channel The channel to use.
*/
void CodecSetOutputGain(Task codecTask, uint16 volume, codec_channel channel);


/*!
    @brief Update the codec output gain immediately. 
 
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.

    @param volume The gain level (volume) to set the output left channel to.

    @param channel The channel to use.
*/
void CodecSetOutputGainNow(Task codecTask, uint16 volume, codec_channel channel);


/*!
    @brief Enable the codec. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.
*/
void CodecEnable(Task codecTask);

/*!
    @brief Disable the codec. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.
*/
void CodecDisable(Task codecTask);

/*!
    @brief Power down the codec. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.
*/
void CodecPowerDown(Task codecTask);


/*!
    @brief Retrieve the codec in use. 
   
    @param codecTask The codec task that was returned with the CODEC_INIT_CFM
    message.
*/
codec_type CodecGetCodecType(Task codecTask);


#endif /* CODEC_H_ */



