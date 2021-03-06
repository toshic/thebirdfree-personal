/*
    This file was autogenerated by buttonparse
*/

#ifndef _AUDIOADAPTOR_BUTTONS_H
#define _AUDIOADAPTOR_BUTTONS_H

#include <message.h>

/* messages sent to the client */
enum
{
	BUTTON_DEVICE_DISCOVER_REQ = 1000 /* base value */,
	BUTTONS_CLEAR_PDL_REQ,
	BUTTON_CONNECT_SECOND_DEVICE_REQ,
	PIO_RAW
};

typedef enum
{
	sMFB,
	Unknown
} InternalState;

typedef struct
{
	uint16 pio;
} PIO_RAW_T;

typedef struct
{
	InternalState store_held;
	InternalState double_press;
	uint16 pio_raw_bits;
	uint16 pskey_wakeup;
	uint16 store_bits;
	uint16 store_count;
	uint16 timed_id;
} PioStoredState;

typedef struct
{
	TaskData task;
	Task client;
	PioStoredState pio_states;
} PioState;

void pioInit(PioState *state, Task client);

#endif
