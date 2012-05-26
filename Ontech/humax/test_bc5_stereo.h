#ifndef _TEST_HEADSET_H_
#define _TEST_HEADSET_H_

#include <message.h>
#include "headset_states.h"

/* Register the main task  */
void test_init(void);

#define HS_MESSAGE_BASE   0x2000

/**************************************************
   VM2HOST
 **************************************************/
typedef enum {
    HS_HFP_STATE = HS_MESSAGE_BASE,
	HS_A2DP_STATE,
	HS_AVRCP_STATE
} vm2host_bc5_stereo;

typedef struct {
    uint16 state;
} HS_HFP_STATE_T;

typedef struct {
    uint16 state;
} HS_A2DP_STATE_T;

typedef struct {
    uint16 state;
} HS_AVRCP_STATE_T;

void vm2host_send_hfp_state(headsetHfpState state);
void vm2host_send_a2dp_state(headsetA2dpState state);
void vm2host_send_avrcp_state(headsetAvrcpState state);

/**************************************************
   HOST2VM
 **************************************************/
typedef enum {
    HEADSET_EVENT_MSG = HS_MESSAGE_BASE + 0x80
} host2vm_bc5_stereo;

typedef struct {
    uint16 event;
} HEADSET_EVENT_MSG_T;

typedef struct {
    uint16 length;
    uint16 bcspType;
    uint16 funcId;

    union {
        HEADSET_EVENT_MSG_T headset_event_msg;
    } headset_from_host_msg;
} HEADSET_FROM_HOST_MSG_T;

/* HS host messages handler */
void handle_msg_from_host(Task task, MessageId id, Message message);

#endif
