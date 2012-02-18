/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_apphdrs.c
    
DESCRIPTION
 Helper library for GOEP Library clients to assist in adding and retrieving
 		application specific headers.

*/

#include <vm.h>
#include <sink.h>
#include <source.h>
#include <string.h>
#include <panic.h>
#include <memory.h>

#include "goep_apphdrs.h"


typedef struct
{
	Source src;
	uint16 end;
	uint16 length;
	uint16 current;
} apphdr_workspace;

/*!
	@brief Add a uint8 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddUint8(Sink snk, uint8 paramID, uint8 param)
{
	uint8* s=SinkMap(snk);
	uint16 o=SinkClaim(snk, 2+1); /* 'header' + parameter */
	
	if (o==0xffff)
		return 0; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = paramID & 0xFF;
	s[1] = 1;
	s[2] = param & 0xFF;
	
	return 3;
}

/*!
	@brief Add a uint16 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddUint16(Sink snk, uint8 paramID, uint16 param)
{
	uint8* s=SinkMap(snk);
	uint16 o=SinkClaim(snk, 2+2); /* 'header' + parameter */
	
	if (o==0xffff)
		return 0; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = paramID & 0xFF;
	s[1] = 2;
	s[2] = (param>>8) & 0xFF;
	s[3] = param & 0xFF;
	
	return 4;
}

/*!
	@brief Add a uint32 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddUint32(Sink snk, uint8 paramID, uint32 param)
{
	uint8* s=SinkMap(snk);
	uint16 o=SinkClaim(snk, 2+4); /* 'header' + parameter */
	
	if (o==0xffff)
		return 0; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = paramID & 0xFF;
	s[1] = 4;
	s[2] = (param>>24) & 0xFF;
	s[3] = (param>>16) & 0xFF;
	s[4] = (param>>8) & 0xFF;
	s[5] = param & 0xFF;
	
	return 6;
}

/*!
	@brief Add a uint64 parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param_lo Low 32 bits (0..31) of the actual parameter to add
	@param param_hi High 32 bits (31..63) of the actual parameter to add
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddUint64(Sink snk, uint8 paramID, uint32 param_lo, uint32 param_hi)
{
	uint8* s=SinkMap(snk);
	uint16 o=SinkClaim(snk, 2+8); /* 'header' + parameter */
	
	if (o==0xffff)
		return 0; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = paramID & 0xFF;
	s[1] = 8;
	s[2] = (param_hi>>24) & 0xFF;
	s[3] = (param_hi>>16) & 0xFF;
	s[4] = (param_hi>>8) & 0xFF;
	s[5] = param_hi & 0xFF;
	s[6] = (param_lo>>24) & 0xFF;
	s[7] = (param_lo>>16) & 0xFF;
	s[8] = (param_lo>>8) & 0xFF;
	s[9] = param_lo & 0xFF;
	
	return 10;
}

/*!
	@brief Add a buffer parameter to the GOEP Packet
	@param snk Sink to add the parameter to
	@param paramID Identifier of the parameter
	@param param Actual parameter to add
	@param length Length of the buffer
	
	Returns length used to add the paramter. Zero (0) is returned on error.
	On return, the offset points to the next offset that can accept a parameter
	
*/
uint16 Goep_apphdr_AddBuffer(Sink snk, uint8 paramID, const uint8 *param, uint8 size_param)
{
	uint8* s=SinkMap(snk);
	uint16 o=SinkClaim(snk, 2+size_param); /* 'header' + parameter */
	
	if (o==0xffff)
		return 0; /* Error */
	
	s+=o;
	/* Packet type */
	s[0] = paramID & 0xFF;
	s[1] = size_param;
	
    if (size_param>0)
		memmove(&s[2], param, size_param);
	
	return size_param + 2;
}


/* Functions to Retrieve Application Specific Parameters */

/*!
	@brief Create a workspace to get Application Specific Parameters
	@param src Source containting the parameters
	@param offset Offset into the source for the parameter header
	@param length Length of the parameter header
	
	Returns void pointer to be used for follow up calls to this library.  NULL
	on error.
*/
void *Goep_apphdr_GetCreateWS(Source src, uint16 offset, uint16 length)
{
	apphdr_workspace *ws = NULL;
	
	/* Create workspace */
	ws = (apphdr_workspace *)PanicUnlessMalloc(sizeof(apphdr_workspace));
	
	/* Initialise structure */
	ws->src = src;
	ws->end = offset + length;
	ws->length = length;
	ws->current = offset;

	return (void *)ws;
}

/*!
	@brief Get the next paramter from the GOEP header.
	@param workspace Pointer returned from Goep_apphdr_GetCreateWS.
	@param paramID ID of the next parameter
	@param param Offset to the parameter in the source
	@param size_param Size of the next parameter

	Returns TRUE if there are no more paramters, and FALSE if there are.
*/
bool Goep_apphdr_GetParameter(void *workspace, uint8 *paramID, uint16 *param, uint8 *size_param)
{
	apphdr_workspace *ws = (apphdr_workspace *)workspace;
	bool ret = FALSE;
	const uint8* s=SourceMap(ws->src); /* Should be near free if the source is already mapped in */

	s+=ws->current;
	*paramID = s[0] & 0xff;
	*size_param = s[1] & 0xff;
	*param = ws->current + 2;
	ws->current += *size_param;
	ws->current += 2; /* Allow for the size of the preamble to the paramter */
	
	if (ws->current >= ws->end)
		ret = TRUE;
	
	return ret;
}

/*!
	@brief Destroy the workspace for Application Specific Parameter retrieval.
	@param workspace Pointer returned from Goep_apphdr_GetCreateWS.
*/
void Goep_apphdr_GetDestroyWS(void *workspace)
{
	free(workspace);
}
