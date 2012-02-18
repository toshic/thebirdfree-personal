/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_extract_headers.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - Extract App. Specific Parameters.

*/

#include <vm.h>
#include <print.h>
#include <pbap_common.h>
#include <goep.h>
#include <string.h>
#include <ctype.h>

#include <goep.h>
#include <goep_apphdrs.h>
#include "pbaps.h"
#include "pbaps_private.h"

#ifdef DEBUG_PRINT_ENABLED
static void printParamString(const uint8* str, uint16 len)
{
	uint16 c;
	
	PRINT(("\""));
	for (c=0; c<len; c++)
	{
		if (str[c]>=' ')
		{
			PRINT(("%c", str[c]));
		}
		else
		{
			PRINT(("_"));
		}
	}
	PRINT(("\""));
	PRINT(("\n"));
}
#else
	#define printParamString(x,y)
#endif /* DEBUG_PRINT_ENABLED */


uint16 pbapsExtractvCardEntry(const uint8 *src)
{
	uint16 entry = 0;
	uint16 val = 0;
	uint8 ch;
	
	ch = toupper(src[0]);
	while (((src[0] >= '0') && (src[0] <= '9')) || ((src[0] >= 'A') && (src[0] <= 'F')))
	{
		ch = toupper(src[0]);
		if ((ch>= '0') && (ch <= '9'))
			val = ch - '0';
		else if ((ch >= 'A') && (ch <= 'F'))
			val = 10 + (ch - 'A');
		else /* Invalid character */
			return 0;
		entry = (entry * 16) + val;
		src+=2; /* Step to next unicode char */
	}
	
	return entry;
}


void pbapsExtractvCardListingParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg)
{
	const uint8 *src = SourceMap(msg->src);
	const uint8 *s;
	void *ws;
	bool fin;
	
	/* Set parameter default values */
	pbap_order_values order = pbap_param_order_def;
	uint8 *srchVal = NULL;
	uint16 size_srchVal = 0;
	pbap_search_values srchAttr = pbap_param_srch_attr_def;
	uint16 maxList = pbap_param_max_list_def;
	uint16 listStart = pbap_param_strt_offset_def;
	
	pbap_phone_book pbook = pbap_b_unknown;	
	
	uint16 param;
	uint8 size_param;
	pbap_goep_parameters paramID;
	
	pbaps_lib_status status = pbaps_success;
	
	PRINT(("pbapsExtractvCardListingParameters\n"));
	
	if ((msg->nameLength != 0) && (msg->nameOffset!=0))
	{ /* Packet contains a folder name */
		s = &src[msg->nameOffset];
		
		pbook = PbapcoGetBookIDFromName(s, msg->nameLength);
		if (pbook == pbap_b_unknown)
		{ /* Supplied folder is invalid - send error to application */
			status = pbaps_pull_vcl_invalid_folder;
		}
	}
	
	ws = Goep_apphdr_GetCreateWS(msg->src, msg->headerOffset, msg->headerLength);
	/* Can't fail since Goep_apphdr_GetCreateWS uses PanicUnlessMalloc */
	
	do
	{
		fin = Goep_apphdr_GetParameter(ws, (uint8*)&paramID, &param, &size_param);
		s = &src[param];
		switch (paramID)
		{
		case pbap_param_order:
			Goep_apphdr_GetUint8(order, s);
			PRINT(("    Order : %d\n",order));
			break;
		case pbap_param_srch_val:
			srchVal = malloc(size_param * sizeof(uint8));
			if (srchVal)
			{
				memmove(srchVal, s, size_param);
				size_srchVal = size_param;
					
				PRINT(("    Search Value : "));
				printParamString(srchVal, size_param);
			}
			else
			{ /* Can't allocate search value.  Ignore it and do the best we can. */
				srchVal = NULL;
				size_srchVal = 0;
				/* Inform application of problem */
				status = pbaps_pull_vcl_no_search_memory;
			}
			break;
		case pbap_param_srch_attr:
			Goep_apphdr_GetUint8(srchAttr, s);
			PRINT(("    Search Attr : %d\n",srchAttr));
			break;
		case pbap_param_max_list:
			Goep_apphdr_GetUint16(maxList, s);
			PRINT(("    Max List : 0x%x\n",maxList));
			break;
		case pbap_param_strt_offset:
			Goep_apphdr_GetUint16(listStart, s);
			PRINT(("    List Start : 0x%x\n",listStart));
			break;
		default:
			/* Don't support this parameter, ignore it. */
			break;
		}
	}
	while (!fin);
			
	Goep_apphdr_GetDestroyWS(ws);
	
	pbapsMsgSendGetvCardListStartInd(state, status, order, srchVal, size_srchVal,
										pbook, srchAttr, maxList, listStart);

}

void pbapsExtractvCardEntryParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg)
{
	const uint8 *src = SourceMap(msg->src);
	const uint8 *s;
	void *ws;
	bool fin;
	
	/* Set parameter default values */
	pbap_format_values format = pbap_param_format_def;
	uint32 filter_lo = 0, filter_hi = 0;
	
	uint16 entry = 0;
	
	uint16 param;
	uint8 size_param;
	pbap_goep_parameters paramID;
	
	PRINT(("pbapsExtractvCardEntryParameters\n"));
	
	/* Extract name from the stream */	
	entry = pbapsExtractvCardEntry(&src[msg->nameOffset+1]); /* Allow for Unicode */
		
	PRINT(("     Entry : %u 0x%x\n", entry, entry));
	
	ws = Goep_apphdr_GetCreateWS(msg->src, msg->headerOffset, msg->headerLength);
	/* Can't fail since Goep_apphdr_GetCreateWS uses PanicUnlessMalloc */
	
	do
	{
		fin = Goep_apphdr_GetParameter(ws, (uint8*)&paramID, &param, &size_param);
		s = &src[param];
		switch (paramID)
		{
		case pbap_param_format:
			Goep_apphdr_GetUint8(format, s);
			PRINT(("    Format : %d\n",format));
			break;
		case pbap_param_filter:
 			Goep_apphdr_GetUint64(filter_lo, filter_hi, s);
  			PRINT(("     Filter low : 0x%x %x\n",(uint16)(filter_lo>>16), (uint16)(filter_lo & 0xFFFF)));
  			PRINT(("    Filter High : 0x%x %x\n",(uint16)(filter_hi>>16), (uint16)(filter_hi & 0xFFFF)));
			break;
		default:
			/* Don't support this parameter, ignore it. */
			break;
		}
	}
	while (!fin);
			
	Goep_apphdr_GetDestroyWS(ws);
	
	pbapsMsgSendGetvCardEntryStartInd(state, entry, format, filter_lo, filter_hi);
}


bool pbapsExtractPhonebookParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg)
{
	const uint8 *src = SourceMap(msg->src);
	const uint8 *s;
	void *ws;
	bool fin;
	
	/* Set parameter default values */
	uint16 maxList = pbap_param_max_list_def;
	uint16 listStart = pbap_param_strt_offset_def;
	pbap_format_values format = pbap_param_format_def;
	uint32 filter_lo = 0, filter_hi = 0;
	pbap_phone_repository repository = pbap_local;
	pbap_phone_book phonebook = pbap_b_unknown;
	
	uint16 param;
	uint8 size_param;
	pbap_goep_parameters paramID;
	
	PRINT(("pbapsExtractPhonebookParameters\n"));
	
	/* Extract repository and phonebook from the stream */
	if (msg->nameLength > 0)
	{
		uint16 pos = msg->nameLength-3;
		uint16 pb_s, pb_e;
		uint16 rep_e = 1;
		
		s = &src[msg->nameOffset];
		
		PRINT(("    Phonebook Name : "));
		printParamString(&src[msg->nameOffset], msg->nameLength);
		
		/* Find end of phonebook name */
		while (s[pos] != '.')
			pos -= 2;
		pb_e = pos - 1;
		
		/* Find start of phonebook name */
		while (s[pos] != '/')
			pos -= 2;
		pb_s = pos + 1;
		
		phonebook = PbapcoGetBookIDFromName(&s[pb_s], (pb_e - pb_s));
		
		/* Determine existance of repository */
		while (s[rep_e] != '/')
			rep_e += 2;
		
		if (rep_e != pos)
		{ /* Repository present */
				rep_e -= 2;
				repository = PbapcoGetRepositoryIDFromName(&s[0], rep_e);
		}
	
		if ((repository == pbap_r_unknown) || (phonebook == pbap_b_unknown) || (phonebook == pbap_telecom))
		{
			return FALSE;
		}
		/* Check repository is one available on this server */
		switch (repository)
		{
		case pbap_current:
		case pbap_local:
			if ((state->repos & PBAP_REP_LOCAL) != PBAP_REP_LOCAL)
				return FALSE;
			break;
		case pbap_sim1:
			if ((state->repos & PBAP_REP_SIM1) != PBAP_REP_SIM1)
				return FALSE;
			break;
		default:
			return FALSE;
		}
	}
	
	ws = Goep_apphdr_GetCreateWS(msg->src, msg->headerOffset, msg->headerLength);
	/* Can't fail since Goep_apphdr_GetCreateWS uses PanicUnlessMalloc */
	
	do
	{
		fin = Goep_apphdr_GetParameter(ws, (uint8*)&paramID, &param, &size_param);
		s = &src[param];
		switch (paramID)
		{
		case pbap_param_format:
			Goep_apphdr_GetUint8(format, s);
			PRINT(("    Format : %d\n",format));
			break;
		case pbap_param_filter:
  			Goep_apphdr_GetUint64(filter_lo, filter_hi, s);
  			PRINT(("     Filter low : 0x%x %x\n",(uint16)(filter_lo>>16), (uint16)(filter_lo & 0xFFFF)));
  			PRINT(("    Filter High : 0x%x %x\n",(uint16)(filter_hi>>16), (uint16)(filter_hi & 0xFFFF)));
			break;
		case pbap_param_max_list:
			Goep_apphdr_GetUint16(maxList, s);
			PRINT(("    Max List : 0x%x\n",maxList));
			break;
		case pbap_param_strt_offset:
			Goep_apphdr_GetUint16(listStart, s);
			PRINT(("    List Start : 0x%x\n",listStart));
			break;
		default:
			/* Don't support this parameter, ignore it. */
			break;
		}
	}
	while (!fin);
			
	Goep_apphdr_GetDestroyWS(ws);
	
	pbapsMsgSendGetPhonebookStartInd(state, repository, phonebook, format, filter_lo, filter_hi, maxList, listStart);
	
	return TRUE;
}

