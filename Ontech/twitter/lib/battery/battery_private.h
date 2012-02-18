/*************************************************************************
 Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
 Part of Audio-Adaptor-SDK 2009.R1

	FILE : 
				battery_private.h

	CONTAINS:
				Internal information for the Battery
                Monitor Library for BlueLab3

**************************************************************************/

#ifndef BATTERY_PRIVATE_H_
#define BATTERY_PRIVATE_H_

/* Increasing this may require an larger range for mv_running_total
   and readings_taken in the BatteryState structure*/
enum { AVERAGE_OVER = 4 }; 

/* Internal messages */
enum
{
    BATTERY_INTERNAL_TIMED_MESSAGE, 
    BATTERY_INTERNAL_RETRY_MESSAGE
};

#endif /* BATTERY_PRIVATE_H */
