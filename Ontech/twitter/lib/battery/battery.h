/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
/*!
@file   battery.h

@brief Take battery readings and return them in millivolts.

    Allows readings of the test pins and supply voltage to be taken, either
    once or at regular intervals.  The readings are sent to a Task as messages.
				
*/

#ifndef BATTERY_H_
#define BATTERY_H_

#include <message_.h> 

#define BATTERY_READING_MESSAGE 0x6800

/*!
  @brief The source from which we wish to take a battery reading.
*/

typedef enum 
{ 
    AIO0,
    AIO1,
    VDD,
    AIO2,
    AIO3,            /*Only available on certain BlueCore variants.*/
    BATTERY_INTERNAL /*Only available on certain BlueCore variants.*/
} battery_reading_source;

/*!
  @brief The internal state of each instance of the battery library.
*/

typedef struct
{
	/*! This task (receives the raw messages from the ADC.) */
    TaskData task;						
	/*! The client task (receives readings in mV.) */
    Task client;						
	/*! The time between requests to the ADC in millseconds */
    uint16 period;						
	/*! The source from which we wish to take voltage readings. */
    battery_reading_source source :3;	
	/*! The number of readings we have taken so far. */	
    int	readings_taken :13;				
	/*! The estimated supply voltage. */
    uint16 vdd_reading;					
	/*! The running total of all our readings so far. */
    uint16 mv_running_total;			
} BatteryState;

/*!
  @brief Initialise an instance of the battery library
    
  @param state  The state to be initialised.

  @param client The client task to which to deliver the readings.
    
  @param source The source from which the readings should be made.
 
  @param period The delay in milliseconds between the messages sent to the
  client.

  If the period passed is zero, then only one reading will be taken and sent.
*/
void BatteryInit(BatteryState* state, Task client, battery_reading_source source, uint16 period);

#endif /* BATTERY_H_ */


