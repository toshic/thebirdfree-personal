/*
    Warning - this file was autogenerated by genparse
    DO NOT EDIT - any changes will be lost
*/

#include "at_cmd.h"
#include "audioAdaptor_private.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_aghfp_slc.h"
#include "uart.h"
#include "SppServer.h"
#include "WritePSKey.h"

#include <ctype.h>
#include <codec.h>
#include <panic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util.h> /* hash and compare */

#if (defined TEST_HARNESS || defined DISPLAY_UART_CMDS)
#include <stdio.h>
#endif

typedef const uint8 *ptr;

static ptr parseData(ptr s, ptr e, Task task);

static __inline__ char my_toupper(char c)
{ return 'a' <= c && c <= 'z' ? c +'A'-'a' : c; }

static ptr skip1(ptr s, ptr e)
{
  if(s)
    while(s != e && (*s == ' ' || *s == '\t'))
      ++s;
  return s;
}

static ptr match1(ptr s, ptr e)
{ return s && s != e && (*s == '\r' || *s == '\n') ? s+1 : 0; }

#ifdef TEST_HARNESS
static void printString(const char *name, const struct sequence *s)
{
  uint16 i;
  printf(" %s='", name);
  for(i = 0; i < s->length; ++i) putchar(s->data[i]);
  printf("'");
}
#endif

static __inline__ ptr skipQuote(ptr p, ptr e)
{ return p && p != e && *p == '"' ? p+1 : p; }

static const int isStringTable[] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,1,0,0,0,0,0,0,1,1,0,1,0,1,
    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0
};

static __inline__ int isString(char c)
{
    return (c & ~0x7F) == 0 && isStringTable[(int)c];
}

static ptr getString(ptr p, ptr e, struct sequence *res)
{
  p = skipQuote(p, e);
  if(p)
  {
    ptr s = p;
    while(p != e && isString(*p)) ++p;
    if(p != s)
    {
        res->data = s;
        res->length = p - s;
        return skipQuote(p, e);
    }
    else
    {
        p = skipQuote(p, e);
        if (*p==',' || *p == '\r')
        {
            res->data = 0;
            res->length = 0;
            return p;
        }
    }
  }
  res->data = 0;
  res->length = 0;
  return 0;
}

static ptr skipOnce1(ptr s, ptr e)
{
  if(s)
  {
    if(s != e && (*s == ',' || *s == ';'))
      ++s;
  }
  return s;
}

static ptr getWildString(ptr p, ptr e, struct sequence *res)
{
  if(p)
  {
    ptr s = p;
    p = (uint8*)UtilFind(0xFFFF, '\r', (const uint16*)p, 0, 1, (uint16) (e - p));
    if (!p) p = (uint8*)UtilFind(0xFFFF, '\n', (const uint16*)p, 0, 1, e - p);
    if (!p) p = e;
    res->data = s;
    res->length = p - s;
    return p;
  }
  res->data = 0;
  res->length = 0;
  return 0;
}

static ptr findEndOfPacket(ptr s, ptr e)
{
  /*
     Returns
     0   if the buffer holds an incomplete packet
     s+1 if the buffer holds an invalid packet
     end of the first packet otherwise
  */
  if(s == e) return 0;

  if(*s == '\r')
  {
    /* expecting <cr> <lf> ... <cr> <lf> */
    if(e-s >= 4)
    {
      if(s[1] == '\n' && s[2] != '\r')
      {
        ptr p = s+2;
        if(*p != '\r')
        {
#ifndef TEST_HARNESS
        p = (const uint8*)UtilFind(0xFFFF, '\r', (const uint16*)p, 0, 1, (uint16)(e-p));
#endif
#ifdef TEST_HARNESS
		   while(p != e && *p != '\r') p++;
#endif
        return p == 0 || p + 1 == e ? 0 /* no terminator yet */
             : p[1] == '\n' ? p+2 /* valid */
             : s+1 ; /* invalid terminator */
         }
         else
             return s+1;
      }
      else
      {
        return s+1;
      }
    }
    else
    {
      /* Can't tell yet */
      return 0;
    }
  }
  else
  {
    /* expecting ... <cr> */
    ptr p = s;
    while(p != e && (*p == ' ' || *p == '\n' || *p == '\0' || *p == '\t')) ++p;
    if(p != e && *p == '\r') return s+1;
    while(p != e && *p != '\r') ++p;
    return p == e ? 0 : p+1;
  }
}

#ifndef TEST_HARNESS
#ifdef __XAP__
uint16 parseUart(Source uart, Task task)
{
  ptr s = SourceMap(uart);
  ptr e = s + SourceSize(uart);
  ptr p = parseData(s, e, task);
  if(p != s)
  {
    SourceDrop(uart, (uint16) (p - s));
    return 1;
  }
  else
  {
    return 0;
  }
}
#endif
#endif

typedef struct {
  char c;
  int to;
} Arc;

static const Arc arcs[] = {
  { '\t', 0 },
  { ' ', 0 },
  { 'A', 1 },
  { 'T', 2 },
  { '\t', 2 },
  { ' ', 2 },
  { '+', 3 },
  { '\t', 3 },
  { ' ', 3 },
  { 'A', 4 },
  { 'C', 5 },
  { 'I', 6 },
  { 'M', 7 },
  { 'R', 8 },
  { 'S', 9 },
  { 'V', 10 },
  { 'W', 11 },
  { 'C', 12 },
  { 'D', 13 },
  { 'I', 14 },
  { 'M', 15 },
  { 'B', 16 },
  { 'C', 17 },
  { 'D', 18 },
  { 'P', 19 },
  { 'R', 20 },
  { 'S', 21 },
  { 'D', 22 },
  { 'C', 23 },
  { 'D', 24 },
  { 'P', 25 },
  { 'T', 26 },
  { 'G', 27 },
  { 'I', 28 },
  { 'R', 29 },
  { 'O', 30 },
  { 'S', 31 },
  { 'N', 32 },
  { 'T', 33 },
  { 'R', 34 },
  { 'O', 35 },
  { 'S', 36 },
  { 'L', 37 },
  { 'S', 38 },
  { 'P', 39 },
  { 'B', 40 },
  { 'N', 41 },
  { 'S', 42 },
  { 'O', 43 },
  { 'S', 44 },
  { 'B', 45 },
  { 'I', 46 },
  { 'A', 47 },
  { 'M', 48 },
  { 'S', 49 },
  { 'N', 50 },
  { 'N', 51 },
  { 'N', 52 },
  { 'C', 53 },
  { 'C', 54 },
  { 'I', 55 },
  { '\t', 34 },
  { ' ', 34 },
  { ':', -1 },
  { '=', -1 },
  { 'N', 56 },
  { 'C', 57 },
  { 'Y', 58 },
  { 'T', 59 },
  { 'D', 60 },
  { 'D', 61 },
  { 'M', 62 },
  { 'S', 63 },
  { 'N', 64 },
  { 'C', 65 },
  { 'N', 66 },
  { 'N', 67 },
  { 'T', 68 },
  { '\t', 48 },
  { ' ', 48 },
  { ':', -2 },
  { '=', -2 },
  { '\t', 49 },
  { ' ', 49 },
  { ':', -3 },
  { '=', -3 },
  { '\t', 50 },
  { ' ', 50 },
  { ':', 69 },
  { '=', 69 },
  { 'M', 70 },
  { '\t', 52 },
  { '\n', -4 },
  { '\r', -4 },
  { ' ', 52 },
  { '\t', 53 },
  { '\n', -5 },
  { '\r', -5 },
  { ' ', 53 },
  { '\t', 54 },
  { ' ', 54 },
  { ':', -6 },
  { '=', -6 },
  { '\t', 55 },
  { ' ', 55 },
  { ':', -7 },
  { '=', -7 },
  { '\t', 56 },
  { '\n', -8 },
  { '\r', -8 },
  { ' ', 56 },
  { ':', -9 },
  { '=', -9 },
  { '\t', 57 },
  { '\n', -10 },
  { '\r', -10 },
  { ' ', 57 },
  { '\t', 58 },
  { '\n', -11 },
  { '\r', -11 },
  { ' ', 58 },
  { '\t', 59 },
  { '\n', -12 },
  { '\r', -12 },
  { ' ', 59 },
  { '\t', 60 },
  { '\n', -13 },
  { '\r', -13 },
  { ' ', 60 },
  { '\t', 61 },
  { '\n', -14 },
  { '\r', -14 },
  { ' ', 61 },
  { '\t', 62 },
  { '\n', -15 },
  { '\r', -15 },
  { ' ', 62 },
  { '\t', 63 },
  { '\n', -16 },
  { '\r', -16 },
  { ' ', 63 },
  { '\t', 64 },
  { ' ', 64 },
  { ':', -17 },
  { '=', -17 },
  { '\t', 65 },
  { '\n', -18 },
  { '\r', -18 },
  { ' ', 65 },
  { '\t', 66 },
  { ' ', 66 },
  { ':', -19 },
  { '=', -19 },
  { '\t', 67 },
  { ' ', 67 },
  { ':', -20 },
  { '=', -20 },
  { '\t', 68 },
  { ' ', 68 },
  { ':', 71 },
  { '=', 71 },
  { '\t', 69 },
  { '\n', -21 },
  { '\r', -21 },
  { ' ', 69 },
  { '?', 72 },
  { '\t', 70 },
  { ' ', 70 },
  { ':', -22 },
  { '=', -22 },
  { '\t', 71 },
  { '\n', -23 },
  { '\r', -23 },
  { ' ', 71 },
  { '?', 73 },
  { '\t', 74 },
  { '\n', -21 },
  { '\r', -21 },
  { ' ', 74 },
  { '?', 72 },
  { '\t', 75 },
  { '\n', -23 },
  { '\r', -23 },
  { ' ', 75 },
  { '?', 73 },
  { '\t', 74 },
  { '\n', -21 },
  { '\r', -21 },
  { ' ', 74 },
  { '\t', 75 },
  { '\n', -23 },
  { '\r', -23 },
  { ' ', 75 },
};

static const Arc *const states[77] = {
  &arcs[0],
  &arcs[3],
  &arcs[4],
  &arcs[7],
  &arcs[17],
  &arcs[19],
  &arcs[21],
  &arcs[22],
  &arcs[27],
  &arcs[28],
  &arcs[32],
  &arcs[34],
  &arcs[35],
  &arcs[36],
  &arcs[37],
  &arcs[38],
  &arcs[39],
  &arcs[40],
  &arcs[41],
  &arcs[42],
  &arcs[43],
  &arcs[44],
  &arcs[45],
  &arcs[48],
  &arcs[49],
  &arcs[50],
  &arcs[52],
  &arcs[53],
  &arcs[55],
  &arcs[56],
  &arcs[57],
  &arcs[58],
  &arcs[59],
  &arcs[60],
  &arcs[61],
  &arcs[65],
  &arcs[66],
  &arcs[67],
  &arcs[68],
  &arcs[69],
  &arcs[70],
  &arcs[71],
  &arcs[72],
  &arcs[73],
  &arcs[74],
  &arcs[75],
  &arcs[76],
  &arcs[77],
  &arcs[78],
  &arcs[82],
  &arcs[86],
  &arcs[90],
  &arcs[91],
  &arcs[95],
  &arcs[99],
  &arcs[103],
  &arcs[107],
  &arcs[113],
  &arcs[117],
  &arcs[121],
  &arcs[125],
  &arcs[129],
  &arcs[133],
  &arcs[137],
  &arcs[141],
  &arcs[145],
  &arcs[149],
  &arcs[153],
  &arcs[157],
  &arcs[161],
  &arcs[166],
  &arcs[170],
  &arcs[175],
  &arcs[180],
  &arcs[185],
  &arcs[189],
  &arcs[193],
};

static uint16 matchLiteral(ptr s, ptr e, Task task)
{ s=s; e=e; task=task; return 0; }

static ptr parseData(ptr s, ptr e, Task task)
{
  ptr p;

#ifdef DISPLAY_UART_CMDS
  {
  	ptr c = s;
    printf("\nreceived: ");
  	while (c != e)
  	{
		if (*c == '\r')			printf("\\r");
		else if (*c == '\n') 	printf("\\n");
		else 					putchar(*c);

		c++;
  	}
  }
#endif

#ifdef TEST_HARNESS
  task = task;
#endif
  for(; (p = findEndOfPacket(s, e)) != 0; s = p)
  {
    if(p == s+1)
    {
      /* Silently discard one character; no packets are that short */
      continue;
    }
    else if(matchLiteral(s, p, task))
    {
      continue;
    }
    else
    {
      union {
        struct inband_ring_enable inband_ring_enable;
        struct set_volume_microphone set_volume_microphone;
        struct set_volume_speaker set_volume_speaker;
        struct virtual_incoming_call virtual_incoming_call;
        struct sms_new_message_ind sms_new_message_ind;
        struct a2dp_signal_connect_req a2dp_signal_connect_req;
        struct slc_connect_req slc_connect_req;
        struct set_phonebook_index set_phonebook_index;
        struct write_pin write_pin;
        struct write_local_name write_local_name;
      } u, *uu = &u;
      int state = 0;
      ptr t = s;
      while(t != e && state >= 0)
      {
        char m = my_toupper((char) *t);
        const Arc *a = states[state];
        const Arc *const last_a = states[state+1];
#ifndef TEST_HARNESS
        a = (const Arc *) (void *) UtilFind(0xFFFF, (uint16) m, (const uint16 *) (void *) &a[0].c, 0, sizeof(Arc), (uint16) (last_a - a));
#endif
#ifdef TEST_HARNESS
        while(a != last_a && a->c != m) a++;
#endif
        /*lint -e{801} suppress goto is deprecated */
        if(!a) goto unrecognised;
        state = a->to;
        ++t;
      }
      switch(-state)
      {
        case 1:
          if(match1(skip1(UtilGetNumber(skip1(t, e), e, &uu->inband_ring_enable.enable), e), e))
          {
#ifndef TEST_HARNESS
            inband_ring_enable(task, &uu->inband_ring_enable);
#endif
#ifdef TEST_HARNESS
            printf("Called inband_ring_enable");
            printf(" enable=%d", uu->inband_ring_enable.enable);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 2:
          if(match1(skip1(UtilGetNumber(skip1(t, e), e, &uu->set_volume_microphone.volume), e), e))
          {
#ifndef TEST_HARNESS
            set_volume_microphone(task, &uu->set_volume_microphone);
#endif
#ifdef TEST_HARNESS
            printf("Called set_volume_microphone");
            printf(" volume=%d", uu->set_volume_microphone.volume);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 3:
          if(match1(skip1(UtilGetNumber(skip1(t, e), e, &uu->set_volume_speaker.volume), e), e))
          {
#ifndef TEST_HARNESS
            set_volume_speaker(task, &uu->set_volume_speaker);
#endif
#ifdef TEST_HARNESS
            printf("Called set_volume_speaker");
            printf(" volume=%d", uu->set_volume_speaker.volume);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 4:
          if(t)
          {
#ifndef TEST_HARNESS
            audio_connect_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called audio_connect_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 5:
          if(t)
          {
#ifndef TEST_HARNESS
            audio_disconnect_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called audio_disconnect_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 6:
          if(match1(skip1(getString(skip1(t, e), e, &uu->virtual_incoming_call.callerid), e), e))
          {
#ifndef TEST_HARNESS
            virtual_incoming_call(task, &uu->virtual_incoming_call);
#endif
#ifdef TEST_HARNESS
            printf("Called virtual_incoming_call");
            printString("callerid", &uu->virtual_incoming_call.callerid);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 7:
          if(match1(skip1(getWildString(skip1(skipOnce1(skip1(getString(skip1(t, e), e, &uu->sms_new_message_ind.sender), e), e), e), e, &uu->sms_new_message_ind.text), e), e))
          {
#ifndef TEST_HARNESS
            sms_new_message_ind(task, &uu->sms_new_message_ind);
#endif
#ifdef TEST_HARNESS
            printf("Called sms_new_message_ind");
            printString("sender", &uu->sms_new_message_ind.sender);
            printString("text", &uu->sms_new_message_ind.text);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 8:
          if(t)
          {
#ifndef TEST_HARNESS
            a2dp_signal_connect_req_to_ags(task);
#endif
#ifdef TEST_HARNESS
            printf("Called a2dp_signal_connect_req_to_ags");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 9:
          if(match1(skip1(getString(skip1(t, e), e, &uu->a2dp_signal_connect_req.bdaddr), e), e))
          {
#ifndef TEST_HARNESS
            a2dp_signal_connect_req(task, &uu->a2dp_signal_connect_req);
#endif
#ifdef TEST_HARNESS
            printf("Called a2dp_signal_connect_req");
            printString("bdaddr", &uu->a2dp_signal_connect_req.bdaddr);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 10:
          if(t)
          {
#ifndef TEST_HARNESS
            a2dp_signal_disconnect_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called a2dp_signal_disconnect_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 11:
          if(t)
          {
#ifndef TEST_HARNESS
            a2dp_start_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called a2dp_start_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 12:
          if(t)
          {
#ifndef TEST_HARNESS
            master_reset(task);
#endif
#ifdef TEST_HARNESS
            printf("Called master_reset");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 13:
          if(t)
          {
#ifndef TEST_HARNESS
            a2dp_suspend_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called a2dp_suspend_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 14:
          if(t)
          {
#ifndef TEST_HARNESS
            read_local_bdaddr(task);
#endif
#ifdef TEST_HARNESS
            printf("Called read_local_bdaddr");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 15:
          if(t)
          {
#ifndef TEST_HARNESS
            read_remote_name(task);
#endif
#ifdef TEST_HARNESS
            printf("Called read_remote_name");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 16:
          if(t)
          {
#ifndef TEST_HARNESS
            read_remote_rssi(task);
#endif
#ifdef TEST_HARNESS
            printf("Called read_remote_rssi");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 17:
          if(match1(skip1(getString(skip1(t, e), e, &uu->slc_connect_req.bdaddr), e), e))
          {
#ifndef TEST_HARNESS
            slc_connect_req(task, &uu->slc_connect_req);
#endif
#ifdef TEST_HARNESS
            printf("Called slc_connect_req");
            printString("bdaddr", &uu->slc_connect_req.bdaddr);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 18:
          if(t)
          {
#ifndef TEST_HARNESS
            slc_disconnect_req(task);
#endif
#ifdef TEST_HARNESS
            printf("Called slc_disconnect_req");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 19:
          if(match1(skip1(UtilGetNumber(skip1(t, e), e, &uu->set_phonebook_index.index), e), e))
          {
#ifndef TEST_HARNESS
            set_phonebook_index(task, &uu->set_phonebook_index);
#endif
#ifdef TEST_HARNESS
            printf("Called set_phonebook_index");
            printf(" index=%d", uu->set_phonebook_index.index);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 20:
          if(match1(skip1(getString(skip1(t, e), e, &uu->write_pin.pin), e), e))
          {
#ifndef TEST_HARNESS
            write_pin(task, &uu->write_pin);
#endif
#ifdef TEST_HARNESS
            printf("Called write_pin");
            printString("pin", &uu->write_pin.pin);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 21:
          if(t)
          {
#ifndef TEST_HARNESS
            vin_request(task);
#endif
#ifdef TEST_HARNESS
            printf("Called vin_request");
            putchar('\n');
#endif
            continue;
          }
          break;
        case 22:
          if(match1(skip1(getWildString(skip1(t, e), e, &uu->write_local_name.name), e), e))
          {
#ifndef TEST_HARNESS
            write_local_name(task, &uu->write_local_name);
#endif
#ifdef TEST_HARNESS
            printf("Called write_local_name");
            printString("name", &uu->write_local_name.name);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 23:
          if(t)
          {
#ifndef TEST_HARNESS
            query_status(task);
#endif
#ifdef TEST_HARNESS
            printf("Called query_status");
            putchar('\n');
#endif
            continue;
          }
          break;
        default:
          break;
      }
      /*
        The message does not contain a recognised AT command or response.
        Pass the data on to the application to have a go at 
      */
unrecognised:
#ifndef TEST_HARNESS
      handleUnrecognisedCmd(s, (uint16) (p-s), task);
#endif
#ifdef TEST_HARNESS
      printf("Called handleUnrecognisedCmd\n");
#endif
    }
  }

  return s;
}

static void SendOk(void)
{
    UartPrintf("\r\nOK\r\n");
}

static void SendError(void)
{
    UartPrintf("\r\nERROR\r\n");
}


/* handler functions */
void audio_connect_req(Task task)
{
	if(the_app->dev_inst[0] && the_app->dev_inst[0]->aghfp_sink)
	{
		aghfpSlcAudioOpen(the_app->dev_inst[0]);
	}
	else
		SendError();
    
}

void audio_disconnect_req(Task task)
{
	if(the_app->dev_inst[0] && the_app->dev_inst[0]->aghfp_sink)
	{
		aghfpSlcAudioClose();
	}
	else
		SendError();
    
}
void virtual_incoming_call(Task task, const struct virtual_incoming_call *req)
{
	if(the_app->dev_inst[0] && the_app->dev_inst[0]->aghfp_sink)
	{
		AghfpSetCallerIdDetails(the_app->aghfp, 129, req->callerid.length, req->callerid.data, 0,0);

		/* store incoming number */
		if(the_app->remote_number)
			free(the_app->remote_number);
		the_app->remote_number = malloc(req->callerid.length);
		if(the_app->remote_number)
		{
			the_app->size_remote_number = req->callerid.length;
			memcpy(the_app->remote_number,req->callerid.data,req->callerid.length);
		}

		MessageSend(task,APP_VOIP_CALL_INCOMING,0);
	}
	else
		SendError();
}
void a2dp_signal_connect_req_to_ags(Task task)
{
}
void a2dp_signal_connect_req(Task task, const struct a2dp_signal_connect_req *req)
{
}
void a2dp_signal_disconnect_req(Task task)
{
}
void a2dp_start_req(Task task)
{
	MessageSend(&the_app->task,APP_AUDIO_STREAMING_ACTIVE,0);

}
void a2dp_suspend_req(Task task)
{
	MessageSend(&the_app->task,APP_AUDIO_STREAMING_TIMER,0);
}
void read_local_bdaddr(Task task)
{

}

void set_phonebook_index(Task task, const struct set_phonebook_index *index)
{
	/* we will support only 3 phonebook index */
	if(index->index >= 0 && index->index <=2)
	{
		the_app->pb_index = index->index;
		SendOk();
	}
	else
		SendError();
}
void write_pin(Task task, const struct write_pin *pin)
{
	memcpy(the_app->pin,pin->pin.data,pin->pin.length);
	the_app->pin[pin->pin.length] = 0;
	SendOk();
}
void read_remote_name(Task task)
{
	if(aghfpSlcGetConnectedHF())
	{
		ConnectionReadRemoteName(&the_app->task, &the_app->connect_bdaddr);
	}
	else
	{
        SendError();
	}
}

void write_local_name(Task task, const struct write_local_name *name)
{
    unsigned int i;
    unsigned int len = name->name.length/2 + (name->name.length % 2);
    uint16 *name_ptr = (uint16*)PanicUnlessMalloc(len);

    memset(name_ptr,0,len);
    for(i=0;i<name->name.length;i++){
        if(i%2){
            name_ptr[i/2] |= (name->name.data[i])<<8;
        }else{
            name_ptr[i/2] = name->name.data[i];
        }
    }
    WritePsKeys(PSKEY_DEVICE_NAME,name_ptr,len);
    free(name_ptr);
	SendOk();
}

void read_remote_rssi(Task task)
{
	if(the_app->dev_inst[0] && the_app->dev_inst[0]->aghfp_sink)
	{
		ConnectionGetRssi(task, the_app->dev_inst[0]->aghfp_sink);
	}
	else
		SendError();
}

static bool fill_bdaddr(bdaddr *addr,const struct sequence *src)
{
    uint8 c;
    uint16 i;

	DEBUG(("addr length %d\n",src->length));
    
    if(src->length != 12)
        return FALSE;
    
    for( i = 0 ; i < src->length ; i++ )
    {
        c = (src->data)[i];
        
        if( c >= '0' && c <= '9' )
            c = c - '0';
        else if( c >= 'a' && c <= 'f' )
            c = c - 'a' + 10;
        else if( c >= 'A' && c <= 'F' )
            c = c - 'A' + 10;
        else
            return FALSE;
        
        if( i < 4 ) /* nap */
        {
            addr->nap <<= 4;
            addr->nap += c;
        }
        else if( i < 6 ) /* uap */
        {
            addr->uap <<= 4;
            addr->uap += c;
        }
        else /* lap */
        {
            addr->lap <<= 4;
            addr->lap += c;
        }
    }
    return TRUE;
}

void slc_connect_req(Task task, const struct slc_connect_req *req)
{
    DEBUG(("AT+SCON received\n"));
    memset(&the_app->connect_bdaddr,0,sizeof(bdaddr));
     
    if(fill_bdaddr(&the_app->connect_bdaddr,&req->bdaddr))
    {
    	DEBUG(("ADDR = (%x:%x:%lx)\n",the_app->connect_bdaddr.nap,the_app->connect_bdaddr.uap,the_app->connect_bdaddr.lap));
        SendOk();
        kickCommAction(CommActionConnect);
    }
    else
        SendError();
}

void slc_disconnect_req(Task task)
{
    /* disconnect spp first */
    SppDisconnectReq();
    kickCommAction(CommActionDisconnect);
}

void sms_new_message_ind(Task task, const struct sms_new_message_ind *cmti)
{
	uint16 i;
	
	if(the_app->dev_inst[0] && the_app->dev_inst[0]->aghfp_sink)
		AghfpSendNewMessageIndex( the_app->dev_inst[0]->aghfp);
	/* store last message */
	if(the_app->message_sender)
	{
		free(the_app->message_sender);
	}
	if(the_app->message_body)
	{
		free(the_app->message_body);
	}

	the_app->message_sender= PanicUnlessMalloc(cmti->sender.length + 1);
	memcpy(the_app->message_sender,cmti->sender.data,cmti->sender.length);
	the_app->message_sender[cmti->sender.length] = '\0';

	the_app->message_body = PanicUnlessMalloc(cmti->text.length);
	memcpy(the_app->message_body,cmti->text.data + 1,cmti->text.length - 1);
	the_app->message_body[cmti->text.length - 1] = '\0';

	for(i=cmti->text.length - 1 ; i > 0 ;i--)
	{
		if(the_app->message_body[i] == '\"')
		{
			the_app->message_body[i] = '\0';
			break;
		}
	}

	DEBUG_AGHFP(("New Message Arrived : \n"));
	DEBUG_AGHFP(("Sender : %s\n",the_app->message_sender));
	DEBUG_AGHFP(("Body : %s\n",the_app->message_body));
}

void set_volume_speaker(Task task, const struct set_volume_speaker *vgs)
{
	if(vgs->volume >= 0 && vgs->volume <=15)
	{
		the_app->vgs = vgs->volume;
		AudioSetVolume(the_app->vgs,the_app->codecTask);
	}
}

void set_volume_microphone(Task task, const struct set_volume_microphone *vgm)
{
	if(vgm->volume >= 0 && vgm->volume <=15)
	{
		the_app->vgm = vgm->volume;
		CodecSetInputGainNow(the_app->codecTask,the_app->vgm,left_and_right_ch);
	}
}

void master_reset(Task task)
{
    ConnectionSmDeleteAllAuthDevices ( 0 );
	Panic();
}

void vin_request(Task task)
{
    SppVinInfoPrint();
}

void inband_ring_enable(Task task, const struct inband_ring_enable *inband)
{
    WritePsKey(PSKEY_USR0,inband->enable);
	SendOk();
}

void query_status(Task task)
{
    uint16 *pv = (uint16 *)&the_app->conn_status;
	SendEventHex(EVT_CONN_STATUS,*pv);
}


void handleUnrecognisedCmd(const uint8 *data, uint16 length, Task task)
{
	DEBUG_AGHFP(("Unhandled UART command\n"));
	SendError();
}

/* send command */

static const char * const gEventString[] =
{
	"+SCI",
	"+SCF",
	"+SDI",
	"+ACI",
	"+ACF",
	"+ADI",
	"CALL_STATE_INCOMING",
	"CALL_STATE_OUTGOING",
	"CALL_STATE_ACTIVE",
	"CALL_STATE_IDLE",
	"+MCI",
	"+MCF",
	"+MDI",
	"+MOI",
	"+MOF",
	"+MSI",
	"+MSF",
	"+MPI",
	"+MPF",
	"+CCI",
	"+CCF",
	"+CDI",
	"+CPL",
	"+CPS",
	"+CST",
	"+CFF",
	"+CFR",
	"+CFW",
	"+CRW",
	"+PCI",
	"+PCF",
	"+PDI",
	"+PBR",
	"PBAP_PULL_PHONEBOOK_REQ",
	"+PBS",
	"+PBC",
	"+SMS",
	"+STS"
};

void SendEvent(evt_string_id id,uint16 status)
{
	UartPrintf("\r\n%s=%d\r\n",gEventString[id],status);
}

void SendEventHex(evt_string_id id,uint16 status)
{
	UartPrintf("\r\n%s=%x\r\n",gEventString[id],status);
}

void SendData(uint8* data,uint16 length)
{
	uint16 i;
	for(i=0;i<length;i++)
		UartPrintf("%c",data[i]);
}

void SendHex(uint8* data,uint16 length)
{
	uint16 i;
	for(i=0;i<length;i++)
	{
		if(data[i]<0x10)
			UartPrintf("0%x",data[i]);
		else
			UartPrintf("%x",data[i]);
	}
}

