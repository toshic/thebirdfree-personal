/*************************************************************************
  Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
  Part of Audio-Adaptor-SDK 2009.R1

	FILE : 
				battery.c

	CONTAINS:
				The Battery Monitor Library for BlueLab3

	DESCRIPTION:
				Allows readings of the test pins and supply voltage to be taken
				either once, or at regular intervals.
				The readings are returned to a specified Task.

**************************************************************************/



/* INCLUDES */
#include "battery.h"
#include "battery_private.h"

#include <stdlib.h>                         /*For malloc and NULL*/

#include <adc.h>                            /*For Adc functions*/
#include <app/message/system_message.h>	    /*For Message types*/
#include <message.h>                        /*For Message functions*/



/* FUNCTION IMPLEMENTATIONS */

/*
  This function sends a retry message to the task passed with the source passed
  Used to reattempt readings from the Adc.
*/
static void repeatReading(Task task, vm_adc_source_type source)
{
	vm_adc_source_type* reading_to_repeat = malloc(sizeof(vm_adc_source_type));
	*reading_to_repeat = source;
	MessageSend(task, BATTERY_INTERNAL_RETRY_MESSAGE, reading_to_repeat);
}


/*
  This function is used to return mV readings to the task passed.
  Readings will not exceed uint16 range, but we don't do pMalloc blocks
  of 1 word and many existing apps rely on a uint32 return value.
*/
static void sendReading(Task task, uint16 reading)
{
    uint32* mV = malloc(sizeof(uint32));
    *mV = reading;
    MessageSend(task, BATTERY_READING_MESSAGE, mV);
}

/*
  If any readings need to be adjusted (for example due to an
  external divider network) the adjustments should be made here.
*/
static uint16 getReading(MessageAdcResult * readingMessage)
{
    if(readingMessage->adc_source == VM_ADC_SRC_VDD_BAT)
    {
        /* Use the scaled reading in the message rather than the unscaled reading */
        return readingMessage->scaled_reading;
    }
    else
        return readingMessage->reading;
}

/*
   Map the battery source to read from to an ADC source.
   Does very little when the sources numbers line up.
*/
static vm_adc_source_type batSrcToAdcSrc(battery_reading_source bat_src)
{
    vm_adc_source_type adc_src = 0;
    
    switch(bat_src)
    {
        case AIO0:
            if(AIO0 == VM_ADC_SRC_AIO0)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_AIO0;
        break;
        case AIO1:
            if(AIO1 == VM_ADC_SRC_AIO1)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_AIO1;
        break;
        case VDD:
            if(VDD == VM_ADC_SRC_VREF)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_VREF;
        break;
        case AIO2:
            if(AIO2 == VM_ADC_SRC_AIO2)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_AIO2;
        break;
        case AIO3:
            if(AIO3 == VM_ADC_SRC_AIO3)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_AIO3;
        break;
        case BATTERY_INTERNAL:
            if(BATTERY_INTERNAL == VM_ADC_SRC_VDD_BAT)
                adc_src = (vm_adc_source_type) bat_src;
            else
                adc_src = VM_ADC_SRC_VDD_BAT;
        break;
    }
    
    return adc_src;
}


/*
  This function is sent:
   MESSAGE_ADC_RESULT messages from the firmware.
   BATTERY_INTERNAL_TIMED_MESSAGE messages from itself.	
   BATTERY_INTERNAL_RETRY_MESSAGE messages when an attempted to read the adc failed.
*/
static void batteryHandler(Task task, MessageId id, Message message)
{
    /* Used to access to the BatteryState */
    BatteryState* current_battery_state = (BatteryState*)task;
										
    switch(id)
    {		
        case BATTERY_INTERNAL_TIMED_MESSAGE:
        {
            /* We are resampling Vref to get Vdd */
            if(!AdcRequest(task, VM_ADC_SRC_VREF))
                repeatReading(task, VM_ADC_SRC_VREF);
        }		
        break;
		
        case BATTERY_INTERNAL_RETRY_MESSAGE:
        {
            vm_adc_source_type* repeat_src = (vm_adc_source_type*)message;
			
            if(!AdcRequest(task, *repeat_src))
                repeatReading(task, *repeat_src);
        }	
        break;

        case MESSAGE_ADC_RESULT:
        {
            /* Cast payload to something useful */
            MessageAdcResult* result = (MessageAdcResult*)message;
            
            /* Was the clients original request a reading from Vdd */
            if(current_battery_state->source == VDD)
            {
                /* Yes it was, check that we have just received a reading of Vref from
                   the adc which we can use to calculate Vdd */
                if(result->adc_source == VM_ADC_SRC_VREF)
                {
                    if(current_battery_state->period == 0)
                    {
                        /*
                            They want a single reading and we have what they requested, 
                            so convert it to mV and give it to them
                            Reading = (Vref/Vdd)*255  So Vdd = (Vref/reading)*255 = (1250/reading)*255
                        */
                        uint16 mV = ((uint32)318750)/getReading(result); /* 1250*255 = 318750 */
                        sendReading(current_battery_state->client, mV);						
					}
                    else
                    {
                        /* They want an averaged reading, so store this one */
                        current_battery_state->mv_running_total += getReading(result);
                        current_battery_state->readings_taken ++;
                        
                        /* If we have reached the number of readings we wish to average over */
                        if(current_battery_state->readings_taken == AVERAGE_OVER)
                        {
                            /* Get averaged reading and send to client.
                               Reset count and running total */
                            uint16 mV = ((uint32)318750)/(current_battery_state->mv_running_total / AVERAGE_OVER);
                            sendReading(current_battery_state->client, mV);
                            
                            current_battery_state->readings_taken = 0;
                            current_battery_state->mv_running_total = 0;
                        }
                        
                        /* Now send a timed message to self to read Vref again */
                        MessageSendLater(&current_battery_state->task, BATTERY_INTERNAL_TIMED_MESSAGE, 0, current_battery_state->period);
                    }
                }
            }
            else
            {
                /* Client requested something other than Vdd, what do we have */
                if(result->adc_source == VM_ADC_SRC_VREF)
                {
                    /*
                        We have Vref, so use this to calculate Vdd
                        Vdd = (Vref / reading)*255.  Vref is always 1.25v or 1250mV
                    */
                    current_battery_state->vdd_reading = ((uint32)318750)/getReading(result);
                    
                    /*
                        Now get the clients original request from the Adc 
                        This code depend on the battery_reading_source type and the
                        vm_adc_source_type defining AIO0, AIO1, VDD/VREF, AIO2 in 
                        the same order.
                    */
                    if(!AdcRequest(task, batSrcToAdcSrc(current_battery_state->source)))
                        repeatReading(task, batSrcToAdcSrc(current_battery_state->source));			

                }
                else
                {
                    /*
                       We have a reading from the source the client requested and
                       a reading of Vdd. If the client requested a single reading, then we
                       can return a reading from the source in mV. If the client requested an
                       averaged reading sent periodically, then we add this to our running total
                    */
                    uint16 cur_reading = ((uint32)current_battery_state->vdd_reading*(getReading(result)))/255;
				
                    if(current_battery_state->period ==0)
                    {
                        /* The client only wants a single reading */
                        sendReading(current_battery_state->client, cur_reading);												
                    }
                    else
					{
                        /* The client wants an averaged reading */
                        current_battery_state->mv_running_total += cur_reading;
                        current_battery_state->readings_taken ++;
                        
                        /* If we have reached the number of readings we wish to average over */
                        if(current_battery_state->readings_taken == AVERAGE_OVER)
                        {
                            /* Get average reading and send to client.
                               Reset count and running total */	
                            sendReading(current_battery_state->client, current_battery_state->mv_running_total/AVERAGE_OVER);					
                            
                            current_battery_state->readings_taken = 0;
                            current_battery_state->mv_running_total = 0;
                        }

                        /*
                            Send a timed message to ourself which will trigger another read of Vref when 
                            received. The user expects an averaged reading every "period", so we want a 
                            new reading every period/AVERAGE_OVER
                        */
                        MessageSendLater(&current_battery_state->task, BATTERY_INTERNAL_TIMED_MESSAGE, 0, current_battery_state->period);
                    }
                }
            }
        }
        break;
    }
}


/*
    This is called by the client. They will have created a BatteryState struct.
    This struct has it's first data element as a TaskData item. This is a bit
    like sub classing. As long as the first element in the state is a function pointer
    we can always deliver the message to the correct place.
*/
void BatteryInit(BatteryState* state, Task client, battery_reading_source source, uint16 period)
{
    /* Set handler for messages from Adc */
    state->task.handler = batteryHandler;

    /* Set task we send messages back to (readings in mV) */
    state->client = client;

    /* Set source to read from */
    state->source = source;

    /* Set how frequently we make readings - zero means do only one reading */
    state->period = period/AVERAGE_OVER;
	
    state->readings_taken = 0;
    state->mv_running_total = 0;

    /* Now make the first request - task to deliver msg to and src to read */
    if(!AdcRequest(&state->task, VM_ADC_SRC_VREF))
        repeatReading(&state->task, VM_ADC_SRC_VREF);
}





