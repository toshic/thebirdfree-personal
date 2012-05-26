#include <app/message/system_message.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include "headset_private.h"
#include "test_bc5_stereo.h"
#include "test_utils.h"

static TaskData testTask;

/* Register the test task  */
void test_init(void) {
    testTask.handler = handle_msg_from_host;
    if (MessageHostCommsTask(&testTask)) {Panic();}
}

/**************************************************
   VM2HOST
 **************************************************/

void vm2host_send_hfp_state(headsetHfpState state) {
	HS_HFP_STATE_T message;
    message.state = state;
    test_send_message(HS_HFP_STATE, (Message)&message, sizeof(HS_HFP_STATE_T), 0, NULL);
}

void vm2host_send_a2dp_state(headsetA2dpState state) {
	HS_A2DP_STATE_T message;
    message.state = state;
    test_send_message(HS_A2DP_STATE, (Message)&message, sizeof(HS_A2DP_STATE_T), 0, NULL);
}

void vm2host_send_avrcp_state(headsetAvrcpState state) {
	HS_AVRCP_STATE_T message;
    message.state = state;
    test_send_message(HS_AVRCP_STATE, (Message)&message, sizeof(HS_AVRCP_STATE_T), 0, NULL);
}

/**************************************************
   HOST2VM
 **************************************************/

/* HS host messages handler */
void handle_msg_from_host(Task task, MessageId id, Message message) {
    HEADSET_FROM_HOST_MSG_T *tmsg = (HEADSET_FROM_HOST_MSG_T *)message;

    switch (tmsg->funcId) {
        case HEADSET_EVENT_MSG:
            MessageSend(
                &theHeadset.task,
                tmsg->headset_from_host_msg.headset_event_msg.event,
                NULL
            );
            break;
    }
}
