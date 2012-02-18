/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_apphdrs.h
    
DESCRIPTION
 Helper library for GOEP Library clients to assist in adding and retrieving
 		application specific headers.

*/
/*!
@file goep_apphdrs.h
@brief Helper library for GOEP Library clients to assist in adding and retrieving
	 		application specific parameters.



Library Dependecies : None
*/

#ifndef GOEP_APPHDRS_H_
#define GOEP_APPHDRS_H_

#include <sink.h>
#include <source.h>

/* Functions to Add Application Specific Parameters */

/*! @name Parameter Addition Functions */
/*! \{ */
/*!
	@brief Add a uint8 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
*/
uint16 Goep_apphdr_AddUint8(Sink snk, uint8 paramID, uint8 param);

/*!
	@brief Add a uint16 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
*/
uint16 Goep_apphdr_AddUint16(Sink snk, uint8 paramID, uint16 param);

/*!
	@brief Add a uint32 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
*/
uint16 Goep_apphdr_AddUint32(Sink snk, uint8 paramID, uint32 param);

/*!
	@brief Add a uint64 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param_lo Low 32 bits (0..31) of the actual parameter to add
	@param param_hi High 32 bits (31..63) of the actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddUint64(Sink snk, uint8 paramID, uint32 param_lo, uint32 param_hi);

/*!
	@brief Add a buffer parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	@param length Length of the buffer
	
	Returns length used to add the paramter. Zero (0) is returned on error.
*/
uint16 Goep_apphdr_AddBuffer(Sink snk, uint8 paramID, const uint8 *param, uint8 size_param);
/*! \} */


/*! @name Parameter Extraction Functions
	@brief Functions to Retrieve Application Specific Parameters.
	
	To retrieve the application specific parameters from the GOEP header,
	first call Goep_apphdr_GetCreateWS to create a workspace to track the
	extraction process.  Then iteratively call Goep_apphdr_GetParameter to find
	the next parameter in the header.
	The macros Goep_apphdr_GetUintXX can be used to extract unsigned integer values.
	When all the parameters have been extracted, call Goep_apphdr_GetDestroyWS to clean
	up the resources being used.
	
	Algorithm :-
<PRE>
	ws = Goep_apphdr_GetCreateWS(src, offset, length);
	if (!ws)
	{
		-- Perform error handling --
	}
	else
	{
		bool fin;
		do
		{
			fin = Goep_apphdr_GetParameter(ws, &paramID, &param, &size_param);
			-- Process parameter as required --
		}
		while (!fin)
		Goep_apphdr_GetDestroyWS(ws);
	}
</PRE>
*/
/*! \{ */
/*!
	@brief Create a workspace to get Application Specific Parameters
	@param src Source containting the parameters
	@param offset Offset into the source for the parameter header
	@param length Length of the parameter header
	
	Returns void pointer to be used for follow up calls to this library.  NULL
	on error.
*/
void *Goep_apphdr_GetCreateWS(Source src, uint16 offset, uint16 length);

/*!
	@brief Get the next paramter from the GOEP header.
	@param workspace Pointer returned from Goep_apphdr_GetCreateWS.
	@param paramID ID of the next parameter
	@param param Offset to the parameter in the source
	@param size_param Size of the next parameter

	Returns TRUE if there are no more paramters, and FALSE if there are.
*/
bool Goep_apphdr_GetParameter(void *workspace, uint8 *paramID, uint16 *param, uint8 *size_param);

/*!
	@brief Destroy the workspace for Application Specific Parameter retrieval.
	@param workspace Pointer returned from Goep_apphdr_GetCreateWS.
*/
void Goep_apphdr_GetDestroyWS(void *workspace);
/*! \} */

/*! @name Parameter Extraction Macros
	@brief Macros to extact unsigned integer parameters from the GOEP Header.
	
	@param result Result of the extraction.
	@param param Uint8 pointer to the parameter to extract.
*/
/*! \{ */
/*!
	@brief Extract a Uint8 value.
*/
#define Goep_apphdr_GetUint8(result, param)  {result = param[0];}
/*!
	@brief Extract a Uint16 value.
*/
#define Goep_apphdr_GetUint16(result, param) {result = param[0]<<8 | param[1];}
/*!
	@brief Extract a Uint32 value.
*/
#define Goep_apphdr_GetUint32(result, param) {result = (uint32)(param[0])<<24 | (uint32)(param[1])<<16 | (uint32)(param[2])<<8 | (uint32)(param[3]);}
/*!
	@brief Extract a Uint64 value.
*/
#define Goep_apphdr_GetUint64(res_lo, res_hi, param) {res_hi = (uint32)(param[0])<<24 | (uint32)(param[1])<<16 | (uint32)(param[2])<<8 | (uint32)(param[3]); \
													  res_lo = (uint32)(param[4])<<24 | (uint32)(param[5])<<16 | (uint32)(param[6])<<8 | (uint32)(param[7]);}
/*! \} */
		
		
#endif /* GOEP_APPHDRS_H_ */

