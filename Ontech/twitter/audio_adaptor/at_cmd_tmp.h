/*
    Warning - this file was autogenerated by genparse
    DO NOT EDIT - any changes will be lost
*/

#ifndef AT_CMD_TMP_H
#define AT_CMD_TMP_H

#include <message_.h>

#ifdef __XAP__
#include <source.h>
#endif

const uint8 *parseData(const uint8 *s, const uint8 *e, Task task);
void handleUnrecognised(const uint8 *data, uint16 length, Task task);

#ifdef __XAP__
uint16 parseSource(Source rfcDataIncoming, Task task);
#endif

struct sequence
{
  const uint8 *data;
  uint16 length;
};

struct inband_ring_enable
{
  uint16 enable;
};
struct ssp_enable
{
  uint16 enable;
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
struct current_call_enable
{
  uint16 enable;
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
struct set_scan_mode
{
  uint16 mode;
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
struct write_local_name
{
  struct sequence name;
};
void enter_dut_mode(Task );
void inband_ring_enable(Task , const struct inband_ring_enable *);
void inband_ring_query(Task );
void ssp_enable(Task , const struct ssp_enable *);
void ssp_query(Task );
void set_volume_microphone(Task , const struct set_volume_microphone *);
void set_volume_speaker(Task , const struct set_volume_speaker *);
void audio_connect_req(Task );
void audio_disconnect_req(Task );
void virtual_incoming_call(Task , const struct virtual_incoming_call *);
void current_call_enable(Task , const struct current_call_enable *);
void currnet_call_query(Task );
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
void set_scan_mode(Task , const struct set_scan_mode *);
void slc_connect_req(Task , const struct slc_connect_req *);
void slc_disconnect_req(Task );
void set_phonebook_index(Task , const struct set_phonebook_index *);
void write_pin(Task , const struct write_pin *);
void vin_request(Task );
void write_local_name(Task , const struct write_local_name *);
void query_status(Task );

#endif
