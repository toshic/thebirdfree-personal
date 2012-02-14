/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    aghfp_wbs.h
    
DESCRIPTION
	Interface for the Wide Band Speech Service for the AGHFP library.

*/

/*!
@file aghfp_wbs.h
@brief Interface for the Wide Band Speech Service for the AGHFP library..

*/

/*****************************************************************************

IMPORTANT INFORMATION REGARDING THE MAINTENANCE OF THIS FILE

The contents of this file shadows that of wbs.h in the WBS speech module. If 
that file is changed, this file must also be changed accordingly.

The .h and .c versions of this file exist to provide an interface for WB-Speech 
for this library. It was desired to be able to build a WB-Speech and 
non-WB-Speech variant of the library with the non-WB-Speech not being dependant 
upon the existence on a WBS library. One option to achieve this would be to 
place compile switches throughout the library code for the WBS handling. This 
options did not appear to fit in with how the library was currently written. A 
second option would be to use a compile switch which would implement a stub for 
the WBS library within the this library for the non-WB-Speech version. This is 
what the .h and .c versions of this file aim to achieve. If WBS is defined, 
wbs.h is included and the WBS library is linked in to complete the interface. 
If WBS is not defined, the .h file implements the defines, enums, structures and 
prototypes which form the WBS-Speech interface and the .c provides the stubs 
for the actual function calls.

This is bad from a code maintenance point of view since this file needs to be 
maintained in line with wbs.h and wbs.c, but the desire to have a non-WB-Speech 
variant not dependant upon the WBS library and to have no WBS compile switches 
scattered around the code necessitated such an approach.

The way foward would be to implement a 'common' module, included by all 
libraries, which can house things which are common to the libraries and can be
used to implement stubs when features are not required.

******************************************************************************/

#ifndef AGHFP_WBS_H
#define AGHFP_WBS_H

#ifdef WBS

#include <wbs.h>

#else

#include <bdaddr_.h>
#include <message.h>


/*!
	@brief WB-Speech codecs.
	
	Currently, AMR-WB is unsupported
*/
typedef enum
{
	/*! CVSD Codec. CVSD support is mandatory. */
	wbs_codec_cvsd,
	/*! SBC Codec. SBC support is mandatory. */
	wbs_codec_sbc,
	/*! AMR-WB Codec.  Currently unsupported */
	wbs_codec_amr_wb,
	/*! EVRC-WB Codec.  Currently unsupported */
	wbs_codec_evrc_wb,
	
	wbs_codec_eol
} wbs_codecs;

/*!
	@brief WB-Speech codec bit masks.
	
	Currently, AMR-WB is unsupported
*/
typedef enum
{
	/*! SBC Codec. SBC support is mandatory. */
	wbs_codec_mask_cvsd = (1 << wbs_codec_cvsd),
	/*! SBC Codec. SBC support is mandatory. */
	wbs_codec_mask_sbc = (1 << wbs_codec_sbc),
	/*! AMR-WB Codec.  Currently unsupported */
	wbs_codec_mask_amr_wb = (1 << wbs_codec_amr_wb),
	/*! AMR-WB Codec.  Currently unsupported */
	wbs_codec_mask_evrc_wb = (1 << wbs_codec_evrc_wb),
	
	wbs_codecs_mask_eol
} wbs_codec_mask;

/*!
	@brief WB-Speech status codes.
*/
typedef enum
{
	/*! Operation succeeded */
	wbs_status_success,
	/*! Operation failed */
	wbs_status_failure,
	
	wbs_result_eol
} wbs_status_code;

/*!
	@brief Structure defining user definable information regarding the HF's codecs.

*/
typedef struct
{
	uint8		num_codecs;
	uint16		codecs;
	uint16		codec_uuids[1];
} codecs_info;

/*
   @brief Upstream Messages for the WB-Speech Library   
*/
#define WBS_MESSAGE_BASE	0x6e00

/* UUIDs for WB-Speech. */
#define WBS_SERV_CLASS		(0x1306)
#define WBS_SERV_CLASS_HI 	((WBS_SERV_CLASS >> 8) & 0xff)
#define WBS_SERV_CLASS_LO 	(WBS_SERV_CLASS & 0xff)

#define WBS_CODEC_CVSD 	 	(0x1234)
#define WBS_CODEC_CVSD_HI 	((WBS_CODEC_CVSD >> 8) & 0xff)
#define WBS_CODEC_CVSD_LO 	(WBS_CODEC_CVSD & 0xff)

#define WBS_CODEC_SBC 		(0x5678)
#define WBS_CODEC_SBC_HI 	((WBS_CODEC_SBC >> 8) & 0xff)
#define WBS_CODEC_SBC_LO 	(WBS_CODEC_SBC & 0xff)

#define WBS_CODEC_AMR 		(0xabcd)
#define WBS_CODEC_AMR_HI 	((WBS_CODEC_AMR >> 8) & 0xff)
#define WBS_CODEC_AMR_LO 	(WBS_CODEC_AMR & 0xff)

#define WBS_CODEC_EVRC 	 	(0xef12)
#define WBS_CODEC_EVRC_HI 	((WBS_CODEC_EVRC >> 8) & 0xff)
#define WBS_CODEC_EVRC_LO 	(WBS_CODEC_EVRC & 0xff)

/*
	Return the codec mask for the supported codecs in this build.
*/
uint16 WbsIncludedCodecsMask(void);

/*
	Return the UUID16 given a codec mask.
*/
uint16 WbsCodecToUuid16(codecs_info *codecs_info_in, uint16 codec);

/*
	Return the codec mask given a UUID16.
*/
uint16 WbsUuid16ToCodec(codecs_info *codecs_info_in, uint16 codecUUID16);

#endif /* WBS */

#endif /* AGHFP_WBS_H */

