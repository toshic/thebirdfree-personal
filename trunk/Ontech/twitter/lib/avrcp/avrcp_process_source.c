/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_process_source.c        

DESCRIPTION
	This file contains code to drop the Source data when the application has finished with it.

NOTES

*/
/*lint -e655 */


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"


void AvrcpSourceProcessed(AVRCP *avrcp)
{
	avrcpSourceProcessed(avrcp);
}
