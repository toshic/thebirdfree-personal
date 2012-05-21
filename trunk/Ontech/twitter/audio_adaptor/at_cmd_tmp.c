/*
    Warning - this file was autogenerated by genparse
    DO NOT EDIT - any changes will be lost
*/

#include "at_cmd_tmp.h"

#include <ctype.h>
#include <panic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util.h> /* hash and compare */

#if (defined TEST_HARNESS || defined DISPLAY_AT_CMDS)
#include <stdio.h>
#endif

typedef const uint8 *ptr;

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
uint16 parseSource(Source rfcDataIn, Task task)
{
  ptr s = SourceMap(rfcDataIn);
  ptr e = s + SourceSize(rfcDataIn);
  ptr p = parseData(s, e, task);
  if(p != s)
  {
    SourceDrop(rfcDataIn, (uint16) (p - s));
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
  { 'A', 43 },
  { 'O', 44 },
  { 'S', 45 },
  { 'B', 46 },
  { 'I', 47 },
  { 'A', 48 },
  { 'M', 49 },
  { 'S', 50 },
  { 'N', 51 },
  { 'N', 52 },
  { 'N', 53 },
  { 'C', 54 },
  { 'C', 55 },
  { 'I', 56 },
  { '\t', 34 },
  { ' ', 34 },
  { ':', -1 },
  { '=', -1 },
  { 'N', 57 },
  { 'C', 58 },
  { 'Y', 59 },
  { 'T', 60 },
  { 'D', 61 },
  { 'D', 62 },
  { 'M', 63 },
  { 'S', 64 },
  { 'N', 65 },
  { 'N', 66 },
  { 'C', 67 },
  { 'N', 68 },
  { 'N', 69 },
  { 'T', 70 },
  { '\t', 49 },
  { ' ', 49 },
  { ':', -2 },
  { '=', -2 },
  { '\t', 50 },
  { ' ', 50 },
  { ':', -3 },
  { '=', -3 },
  { '\t', 51 },
  { ' ', 51 },
  { ':', 71 },
  { '=', 71 },
  { 'M', 72 },
  { '\t', 53 },
  { '\n', -4 },
  { '\r', -4 },
  { ' ', 53 },
  { '\t', 54 },
  { '\n', -5 },
  { '\r', -5 },
  { ' ', 54 },
  { '\t', 55 },
  { ' ', 55 },
  { ':', -6 },
  { '=', -6 },
  { '\t', 56 },
  { ' ', 56 },
  { ':', -7 },
  { '=', -7 },
  { '\t', 57 },
  { '\n', -8 },
  { '\r', -8 },
  { ' ', 57 },
  { ':', -9 },
  { '=', -9 },
  { '\t', 58 },
  { '\n', -10 },
  { '\r', -10 },
  { ' ', 58 },
  { '\t', 59 },
  { '\n', -11 },
  { '\r', -11 },
  { ' ', 59 },
  { '\t', 60 },
  { '\n', -12 },
  { '\r', -12 },
  { ' ', 60 },
  { '\t', 61 },
  { '\n', -13 },
  { '\r', -13 },
  { ' ', 61 },
  { '\t', 62 },
  { '\n', -14 },
  { '\r', -14 },
  { ' ', 62 },
  { '\t', 63 },
  { '\n', -15 },
  { '\r', -15 },
  { ' ', 63 },
  { '\t', 64 },
  { '\n', -16 },
  { '\r', -16 },
  { ' ', 64 },
  { '\t', 65 },
  { ' ', 65 },
  { ':', -17 },
  { '=', -17 },
  { '\t', 66 },
  { ' ', 66 },
  { ':', -18 },
  { '=', -18 },
  { '\t', 67 },
  { '\n', -19 },
  { '\r', -19 },
  { ' ', 67 },
  { '\t', 68 },
  { ' ', 68 },
  { ':', -20 },
  { '=', -20 },
  { '\t', 69 },
  { ' ', 69 },
  { ':', -21 },
  { '=', -21 },
  { '\t', 70 },
  { ' ', 70 },
  { ':', 73 },
  { '=', 73 },
  { '\t', 71 },
  { '\n', -22 },
  { '\r', -22 },
  { ' ', 71 },
  { '?', 74 },
  { '\t', 72 },
  { ' ', 72 },
  { ':', -23 },
  { '=', -23 },
  { '\t', 73 },
  { '\n', -24 },
  { '\r', -24 },
  { ' ', 73 },
  { '?', 75 },
  { '\t', 76 },
  { '\n', -22 },
  { '\r', -22 },
  { ' ', 76 },
  { '?', 74 },
  { '\t', 77 },
  { '\n', -24 },
  { '\r', -24 },
  { ' ', 77 },
  { '?', 75 },
  { '\t', 76 },
  { '\n', -22 },
  { '\r', -22 },
  { ' ', 76 },
  { '\t', 77 },
  { '\n', -24 },
  { '\r', -24 },
  { ' ', 77 },
};

static const Arc *const states[79] = {
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
  &arcs[50],
  &arcs[51],
  &arcs[53],
  &arcs[54],
  &arcs[56],
  &arcs[57],
  &arcs[58],
  &arcs[59],
  &arcs[60],
  &arcs[61],
  &arcs[62],
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
  &arcs[79],
  &arcs[80],
  &arcs[84],
  &arcs[88],
  &arcs[92],
  &arcs[93],
  &arcs[97],
  &arcs[101],
  &arcs[105],
  &arcs[109],
  &arcs[115],
  &arcs[119],
  &arcs[123],
  &arcs[127],
  &arcs[131],
  &arcs[135],
  &arcs[139],
  &arcs[143],
  &arcs[147],
  &arcs[151],
  &arcs[155],
  &arcs[159],
  &arcs[163],
  &arcs[167],
  &arcs[172],
  &arcs[176],
  &arcs[181],
  &arcs[186],
  &arcs[191],
  &arcs[195],
  &arcs[199],
};

static uint16 matchLiteral(ptr s, ptr e, Task task)
{ s=s; e=e; task=task; return 0; }

ptr parseData(ptr s, ptr e, Task task)
{
  ptr p;

#ifdef DISPLAY_AT_CMDS
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
        struct set_scan_mode set_scan_mode;
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
          if(match1(skip1(UtilGetNumber(skip1(t, e), e, &uu->set_scan_mode.mode), e), e))
          {
#ifndef TEST_HARNESS
            set_scan_mode(task, &uu->set_scan_mode);
#endif
#ifdef TEST_HARNESS
            printf("Called set_scan_mode");
            printf(" mode=%d", uu->set_scan_mode.mode);
            putchar('\n');
#endif
            continue;
          }
          break;
        case 18:
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
        case 19:
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
        case 20:
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
        case 21:
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
        case 22:
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
        case 23:
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
        case 24:
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
      handleUnrecognised(s, (uint16) (p-s), task);
#endif
#ifdef TEST_HARNESS
      printf("Called handleUnrecognised\n");
#endif
    }
  }

  return s;
}

/*
read_local_bdaddr
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar R
   MatchChar D
   MatchChar B
   MatchChar D
   Skip " \t"
   Match "\r\n"


read_remote_rssi
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar R
   MatchChar D
   MatchChar S
   MatchChar S
   Skip " \t"
   Match "\r\n"


write_pin
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar P
   MatchChar I
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetString pin
   Skip " \t"
   Match "\r\n"


read_remote_name
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar R
   MatchChar D
   MatchChar N
   MatchChar M
   Skip " \t"
   Match "\r\n"


write_local_name
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar W
   MatchChar R
   MatchChar N
   MatchChar M
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetWildString name
   Skip " \t"
   Match "\r\n"


set_phonebook_index
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar P
   MatchChar B
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetNumber index
   Skip " \t"
   Match "\r\n"


slc_connect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar C
   MatchChar O
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetString bdaddr
   Skip " \t"
   Match "\r\n"


slc_disconnect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar D
   MatchChar S
   MatchChar C
   Skip " \t"
   Match "\r\n"


audio_connect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar A
   MatchChar C
   MatchChar O
   MatchChar N
   Skip " \t"
   Match "\r\n"


audio_disconnect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar A
   MatchChar D
   MatchChar S
   MatchChar C
   Skip " \t"
   Match "\r\n"


virtual_incoming_call
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar C
   MatchChar I
   MatchChar N
   MatchChar C
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetString callerid
   Skip " \t"
   Match "\r\n"


a2dp_signal_connect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar C
   MatchChar O
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetString bdaddr
   Skip " \t"
   Match "\r\n"


a2dp_signal_connect_req_to_ags
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar C
   MatchChar O
   MatchChar N
   Skip " \t"
   Match "\r\n"


a2dp_signal_disconnect_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar D
   MatchChar S
   MatchChar C
   Skip " \t"
   Match "\r\n"


a2dp_start_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar P
   MatchChar L
   MatchChar Y
   Skip " \t"
   Match "\r\n"


a2dp_suspend_req
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar S
   MatchChar P
   MatchChar D
   Skip " \t"
   Match "\r\n"


sms_new_message_ind
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar C
   MatchChar M
   MatchChar T
   MatchChar I
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetString sender
   Skip " \t"
   SkipOnce ",;"
   Skip " \t"
   GetWildString text
   Skip " \t"
   Match "\r\n"


set_volume_speaker
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar V
   MatchChar G
   MatchChar S
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetNumber volume
   Skip " \t"
   Match "\r\n"


set_volume_microphone
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar V
   MatchChar G
   MatchChar M
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetNumber volume
   Skip " \t"
   Match "\r\n"


master_reset
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar M
   MatchChar R
   MatchChar S
   MatchChar T
   Skip " \t"
   Match "\r\n"


vin_request
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar V
   MatchChar I
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   Skip "?"
   Skip " \t"
   Match "\r\n"


inband_ring_enable
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar I
   MatchChar B
   MatchChar R
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetNumber enable
   Skip " \t"
   Match "\r\n"


query_status
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar T
   MatchChar A
   MatchChar T
   Skip " \t"
   Match "=:"
   Skip " \t"
   Skip "?"
   Skip " \t"
   Match "\r\n"


set_scan_mode
   Skip " \t"
   MatchChar A
   MatchChar T
   Skip " \t"
   MatchChar +
   Skip " \t"
   MatchChar S
   MatchChar C
   MatchChar A
   MatchChar N
   Skip " \t"
   Match "=:"
   Skip " \t"
   GetNumber mode
   Skip " \t"
   Match "\r\n"


*/
