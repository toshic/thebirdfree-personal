/*
    Warning - this file was autogenerated by genparse
    DO NOT EDIT - any changes will be lost
*/

#ifndef AT_CMD_H
#define AT_CMD_H

#include <message_.h>

#ifdef __XAP__
#include <source.h>
#endif

void handleUnrecognisedCmd(const uint8 *data, uint16 length, Task task);

#ifdef __XAP__
uint16 parseUart(Source uart, Task task);
#endif


typedef enum
{
	EVT_SLC_CONNECT_IND,
	EVT_SLC_CONNECT_CFM,
	EVT_SLC_DISCONNECT_IND,
	EVT_AUDIO_CONNECT_IND,
	EVT_AUDIO_CONNECT_CFM,
	EVT_AUDIO_DISCONNECT_IND,
	EVT_CALL_STATE_INCOMING,
	EVT_CALL_STATE_OUTGOING,
	EVT_CALL_STATE_ACTIVE,
	EVT_CALL_STATE_IDLE,
	EVT_A2DP_SIGNAL_CONNECT_IND,
	EVT_A2DP_SIGNAL_CONNECT_CFM,
	EVT_A2DP_SIGNAL_DISCONNECT_IND,
	EVT_A2DP_OPEN_IND,
	EVT_A2DP_OPEN_CFM,
	EVT_A2DP_START_IND,
	EVT_A2DP_START_CFM,
	EVT_A2DP_SUSPEND_IND,
	EVT_A2DP_SUSPEND_CFM,
	EVT_AVRCP_SIGNAL_CONNECT_IND,
	EVT_AVRCP_SIGNAL_CONNECT_CFM,
	EVT_AVRCP_SIGNAL_DISCONNECT_IND,
	EVT_AVRCP_CMD_PLAY,
	EVT_AVRCP_CMD_PAUSE,
	EVT_AVRCP_CMD_STOP,
	EVT_AVRCP_CMD_SKIP_FWD,
	EVT_AVRCP_CMD_SKIP_RWD,
	EVT_AVRCP_CMD_FWD,
	EVT_AVRCP_CMD_RWD,
	EVT_PBAP_CONNECT_IND,
	EVT_PBAP_CONNECT_CFM,
	EVT_PBAP_DISCONNECT_IND,
	EVT_PBAP_SET_PHONEBOOK_REQ,
	EVT_PBAP_PULL_PHONEBOOK_REQ,
	EVT_PBAP_PULL_PHONEBOOK_START,
	EVT_PBAP_PULL_PHONEBOOK_COMPLETE,
	EVT_SMS_READY
}evt_string_id;


struct sequence
{
  const uint8 *data;
  uint16 length;
};

struct set_volume_microphone
{
  uint16 volume;
};
struct set_volume_speaker
{
  uint16 volume;
};
struct virtual_incoming_call
{
  struct sequence callerid;
};
struct sms_new_message_ind
{
  struct sequence sender;
  struct sequence text;
};
struct a2dp_signal_connect_req
{
  struct sequence bdaddr;
};
struct slc_connect_req
{
  struct sequence bdaddr;
};
struct set_phonebook_index
{
  uint16 index;
};
struct write_pin
{
  struct sequence pin;
};
void set_volume_microphone(Task , const struct set_volume_microphone *);
void set_volume_speaker(Task , const struct set_volume_speaker *);
void audio_connect_req(Task );
void audio_disconnect_req(Task );
void virtual_incoming_call(Task , const struct virtual_incoming_call *);
void sms_new_message_ind(Task , const struct sms_new_message_ind *);
void a2dp_signal_connect_req_to_ags(Task );
void a2dp_signal_connect_req(Task , const struct a2dp_signal_connect_req *);
void a2dp_signal_disconnect_req(Task );
void a2dp_start_req(Task );
void master_reset(Task );
void a2dp_suspend_req(Task );
void read_local_bdaddr(Task );
void read_remote_name(Task );
void read_remote_rssi(Task );
void slc_connect_req(Task , const struct slc_connect_req *);
void slc_disconnect_req(Task );
void set_phonebook_index(Task , const struct set_phonebook_index *);
void write_pin(Task , const struct write_pin *);

void SendEvent(evt_string_id id,uint16 status);
void SendData(uint8* data,uint16 length);
void SendHex(uint8* data,uint16 length);
#endif
