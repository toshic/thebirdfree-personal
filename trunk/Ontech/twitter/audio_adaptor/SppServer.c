#include "SppServer.h"
#include <string.h>
#include <panic.h>
#include <stdlib.h>

#define FRAME_SIZE		0

static Sink spp_sink;
static SPP *spp;

uint8 vin_info[22];

typedef struct
{
	uint8 dir;
	uint16 addr;
	uint16 seq_no;
	uint16 payload_len;
	uint16 header_crc;
	uint16 data_crc;
	uint8 fragment_index;
	uint8 fragment_number;
	uint16 command;
	uint16 data[1];
}packet_frame;

static const uint16 crcTable[256] = 
{
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

static const uint8 pkt_connect[] = 
{
    0xeb,0x0b,
    0x01,
    0x00,0x00,
    0x00,0x01,
    0x00,0x07,
    0x8d,0xb8,
    0x00,0x00,0x00,
    0x4e,0xdf,0x4d,0x20,
    0xd4,0xa0
};

static const uint8 pkt_activate[] = 
{
    0x00,0x00,0x08,
    0x00,0x00,0x00,0x00,
    '0','1','1','3','4','7','5','2','4','3',
    '0','0','0','0'
};

static uint16 crcFast(uint16 *message, int nBytes)
{
    uint16 remainder = 0xffff;
    unsigned char  data;
    int i;

    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for (i = 0; i < nBytes; ++i)
    {
        data = message[i] ^ (remainder >> (16 - 8));
  		remainder = crcTable[data] ^ (remainder << 8);
    }

    /*
     * The final remainder is the CRC.
     */
    return (remainder ^ 0xffff);
}   /* crcFast() */

static uint16 crcCalc(uint16 remainder, uint8 *message,int nBytes)
{
    unsigned char  data;
    int i;

    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for (i = 0; i < nBytes; ++i)
    {
        data = message[i];

        data = data ^ (remainder >> (16 - 8));
  		remainder = crcTable[data] ^ (remainder << 8);
    }

    /*
     * The final remainder is the CRC.
     */
    return remainder;
}

static uint16 crcZero(uint16 remainder,int nBytes)
{
    unsigned char  data;
    int i;

    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for (i = 0; i < nBytes; ++i)
    {
        data = 0;

        data = data ^ (remainder >> (16 - 8));
  		remainder = crcTable[data] ^ (remainder << 8);
    }

    /*
     * The final remainder is the CRC.
     */
    return remainder;
}

uint16 send_seq;
packet_frame *recv_pkt;
#ifdef LONG_MSG    
uint16 *payload_long;
#endif
void sppDevInit(void)
{
    spp_init_params init;

    init.client_recipe = 0;
    init.size_service_record = 0;
	init.service_record = 0;
	init.no_service_record = 0;
	
    /* Initialise the spp profile lib, stating that this is device B */ 
    SppInitLazy(&the_app->task, &the_app->task, &init);

    recv_pkt = (packet_frame *) malloc(sizeof(packet_frame) + 239); /* 460 */
#ifdef LONG_MSG    
    payload_long = (uint16 *) malloc(90); /* 180 */
    if(recv_pkt == 0 || payload_long == 0)
#else
    if(recv_pkt == 0)
#endif    
        DEBUG(("malloc fail\n"));
}

typedef enum {
    frame_wait_packet,
    frame_preamble,
    frame_direction,
    frame_address,
    frame_sequence,
    frame_payload_length,
    frame_hdr_check,
    frame_fragment_flag,
    frame_fragment_index,
    frame_fragment_count,
    frame_command,
    frame_payload,
    frame_payload_check
}frame_state;

static void SppWrite(uint8 *s, unsigned int n)
{
#if 0    
    uint16 i;
    DEBUG(("<<"));
    for(i=0;i<n;i++){
        DEBUG(("%02x ",s[i]));
    }
    DEBUG(("\n"));
#endif    
	if(spp_sink)
	{
		if(SinkClaim(spp_sink, n) != 0xFFFF)
		{
			memcpy(SinkMap(spp_sink), s, n);
			SinkFlush(spp_sink, n);
		}
		else
		{
			DEBUG(("SppWrite Fail\n"));
		}
	}
}

static void SppWriteZero(unsigned int n)
{
	if(spp_sink)
	{
		if(SinkClaim(spp_sink, n) != 0xFFFF)
		{
			memset(SinkMap(spp_sink), 0, n);
			SinkFlush(spp_sink, n);
		}
		else
		{
			DEBUG(("SppWrite Fail\n"));
		}
	}
}

static void SppSendCmd(const uint8 *pkt,uint16 len)
{
    SppWrite((uint8*)pkt, len);
}

static void SppSendPacket(uint8 dir, uint8 *pkt, uint16 len, uint8 is_ack)
{
    uint8 *send_pkt = (uint8*) malloc(len+13);
    uint16 seq;
    uint16 crc;

    if(!send_pkt){
        DEBUG(("Send buffer error\n"));
        return;
    }

    if(!is_ack){
        send_seq++;
        seq=send_seq;
    }
    else{
        len=4;
        seq=recv_pkt->seq_no;
    }

    send_pkt[0] = 0xeb;
    send_pkt[1] = 0x0b;
    send_pkt[2] = dir + is_ack;
    send_pkt[3] = 0x12;
    send_pkt[4] = 0x34;
    send_pkt[5] = seq>>8;
    send_pkt[6] = seq & 0xff;
    send_pkt[7] = len>>8;
    send_pkt[8] = len & 0xff;
    crc = crcFast((uint16*)send_pkt,9);
    send_pkt[9] = crc>>8;
    send_pkt[10] = crc & 0xff;
    if(is_ack){
        send_pkt[11] = 0x00;
        send_pkt[12] = recv_pkt->command >> 8;
        send_pkt[13] = (recv_pkt->command & 0xff) + 1;
        send_pkt[14] = 0;

    }
    else{
        memcpy(send_pkt + 11,pkt,len);    
    }
    crc = crcFast((uint16*)(send_pkt + 11),len);
    send_pkt[11+len] = crc>>8;
    send_pkt[12+len] = crc & 0xff;
    SppWrite(send_pkt,len+13);
    free(send_pkt);
}

static void SppSendTwitter(uint8 index)
{
    uint8 *send_pkt = (uint8*) malloc(16);
    uint16 crc;
    uint16 len;
    uint8 crc_byte[2];

    /* twitter Entry size is 780 byte
            timestamp ? - 68byte
            username - 84byte
            userid - 64byte
            text - 564byte

            first 2byte of packet indicate total Entry count 
        */
    static const uint8 twitter_1[] =
    {
        0x00,0x01,0x2f,0x1b,0x1b,0xc4,0x31,0x34,0x36,0x35,0x31,0x31,0x34,0x34,0x31,0x34,
        0x33,0x30,0x37,0x31,0x36,0x34,0x31,0x36
    };
    /* 46 zero */
    static const uint8 twitter_2[] = "TwitterUser";
    /* 84-11 zero */
    static const uint8 twitter_3[] = "TwitterID";
    /* 64 -9 zero */
    static const uint8 twitter_4[] = "Twitter Text - ABCDEFGHIJKLMN";
    /* 425-strlen zero */

    if(!send_pkt){
        DEBUG(("Send buffer error\n"));
        return;
    }

    if(index == 0){
        send_seq++;
    }

    if(index<0x24)
        len = 0x0288;
    else
        len = 0x0103;

    send_pkt[0] = 0xeb;
    send_pkt[1] = 0x0b;
    send_pkt[2] = 0x01;
    send_pkt[3] = 0x12;
    send_pkt[4] = 0x34;
    send_pkt[5] = send_seq>>8;
    send_pkt[6] = send_seq & 0xff;
    send_pkt[7] = len>>8;
    send_pkt[8] = len & 0xff;
    crc = crcFast((uint16*)send_pkt,9);
    send_pkt[9] = crc>>8;
    send_pkt[10] = crc & 0xff;
    send_pkt[11] = 0x01;
    send_pkt[12] = index;
    send_pkt[13] = 0x25;
    send_pkt[14] = 0x11;
    send_pkt[15] = 0x02;
    SppWrite(send_pkt,16);
    crc = crcCalc(0xffff,send_pkt + 11,5);
    free(send_pkt);
    if(index == 0){
        SppWrite((uint8*)twitter_1,sizeof(twitter_1));
        crc = crcCalc(crc,(uint8*)twitter_1,sizeof(twitter_1));
        SppWriteZero(46);
        crc = crcZero(crc,46);
        
        SppWrite((uint8*)twitter_2,strlen((char*)twitter_2));
        crc = crcCalc(crc,(uint8*)twitter_2,strlen((char*)twitter_2));
        SppWriteZero(84-strlen((char*)twitter_2));
        crc = crcZero(crc,84-strlen((char*)twitter_2));
        
        SppWrite((uint8*)twitter_3,strlen((char*)twitter_3));
        crc = crcCalc(crc,(uint8*)twitter_3,strlen((char*)twitter_3));
        SppWriteZero(64-strlen((char*)twitter_3));
        crc = crcZero(crc,64-strlen((char*)twitter_3));
        
        SppWrite((uint8*)twitter_4,strlen((char*)twitter_4));
        crc = crcCalc(crc,(uint8*)twitter_4,strlen((char*)twitter_4));
        SppWriteZero(425-strlen((char*)twitter_4));
        crc = crcZero(crc,425-strlen((char*)twitter_4));
        crc ^= 0xffff;
        crc_byte[0] = crc>>8;
        crc_byte[1] = crc & 0xff;
        SppWrite(crc_byte,2);
    }
    else {
        
        crc = crcZero(crc,len-5);
        crc ^= 0xffff;
        crc_byte[0] = crc>>8;
        crc_byte[1] = crc & 0xff;
        SppWriteZero(len-5);
        SppWrite(crc_byte,2);
    }    
}

void SppVinInfoPrint(void)
{
    UartPrintf("\r\n+VIN=%s\r\n",vin_info);
}

static void ResponseSpp(void)
{
    uint8 cmd[4];
    static uint8 twitter_index;
    if(recv_pkt->dir % 2){
        SppSendPacket(recv_pkt->dir,0,4,1);
        switch(recv_pkt->command){
        case 0x0002:
            cmd[0] = 0;
            cmd[1] = 0x08;
            cmd[2] = 0;
            SppSendPacket(0x01,cmd,3,0);
            memcpy(vin_info,recv_pkt->data + 6, 21);
            vin_info[21] = 0;
            break;
        case 0x0004:
        case 0x0006:
            SppSendPacket(0x01,(uint8*)pkt_activate,sizeof(pkt_activate),0);
            break;
        case 0x000a:
            cmd[0] = 0;
            cmd[1] = 0;
            cmd[2] = 0x0c;
            cmd[3] = 0;
            SppSendPacket(recv_pkt->dir,cmd,4,0);
            break;
        case 0x0802:
            if((recv_pkt->fragment_index + 1) == recv_pkt->fragment_number){
                cmd[0] = 0;
                cmd[1] = 0x0c;
                cmd[2] = 0;
                SppSendPacket(0x01,cmd,3,0);
            }
            break;
        case 0x0c02:
            if((recv_pkt->fragment_index + 1) == recv_pkt->fragment_number){
                cmd[0] = 0;
                cmd[1] = 0x0e;
                cmd[2] = 0;
                SppSendPacket(0x01,cmd,3,0);
            }
            break;
        case 0x0e02:
            if((recv_pkt->fragment_index + 1) == recv_pkt->fragment_number)
                SppVinInfoPrint();
            break;
        case 0x1100: /* twitter request */
        case 0x1102: /* twitter update request */
            twitter_index = 0;
            UartPrintf("\r\n+TWS\r\n");
            SppSendTwitter(twitter_index);
            break;
        
        }
    }
    else{
        switch(recv_pkt->command){
        case 0x000d:
            SppSendCmd(pkt_connect,sizeof(pkt_connect));
            send_seq=1;
            break;
        case 0x1103: /* twitter ack */
            twitter_index++;
            if(twitter_index < 0x25)
                SppSendTwitter(twitter_index);
            else
                UartPrintf("\r\n+TWE\r\n");
            break;
        }
    }
}

void SppParse(Source source)
{
	uint16 size;
	uint8 *ptr;
	uint16 i,j;

	static frame_state pkt_state;
	static uint16 pkt_index;

	j=j;

	if(source != StreamSourceFromSink(spp_sink))
	    return;
#if 0
    DEBUG((">>"));
#endif    
	while((size = SourceSize(source)) > 0 )
	{
		ptr = (uint8*)SourceMap(source);

/* need to check maximum Source size */
		for(i=0;i<size;i++){
#if 0		
		    if(i<16){
    			DEBUG(("%02x ",ptr[i]));
    		}else if(i==16){
    			DEBUG(("..."));
            }    		
#endif
    		switch(pkt_state)
    		{
            case frame_wait_packet:
                if(ptr[i]==0xeb)
                    pkt_state = frame_preamble;
                else{
                    DEBUG(("?%x\n",ptr[i]));
                }
                break;
            case frame_preamble:
                if(ptr[i]==0x0b){
                    pkt_state = frame_direction;
                    recv_pkt->dir = 0;
                }
                else
                    pkt_state = frame_wait_packet;
                break;
            case frame_direction:
                recv_pkt->dir = ptr[i];
                pkt_state = frame_address;
                pkt_index = 0;
                break;
            case frame_address:
                if(pkt_index){
                    recv_pkt->addr <<= 8;
                    recv_pkt->addr += ptr[i];
                    pkt_state = frame_sequence;
                    pkt_index = 0;
                }
                else{
                    recv_pkt->addr = ptr[i];
                    pkt_index++;
                }
                break;
            case frame_sequence:
                if(pkt_index){
                    recv_pkt->seq_no<<= 8;
                    recv_pkt->seq_no += ptr[i];
                    pkt_state = frame_payload_length;
                    pkt_index = 0;
                }
                else{
                    recv_pkt->seq_no = ptr[i];
                    pkt_index++;
                }
                break;
            case frame_payload_length:
                if(pkt_index){
                    recv_pkt->payload_len<<= 8;
                    recv_pkt->payload_len += ptr[i];
                    pkt_state = frame_hdr_check;
                    pkt_index = 0;
                }
                else{
                    recv_pkt->payload_len = ptr[i];
                    pkt_index++;
                }
                break;
            case frame_hdr_check:
                if(pkt_index){
                    recv_pkt->header_crc<<= 8;
                    recv_pkt->header_crc += ptr[i];
                    pkt_state = frame_fragment_flag;
                    pkt_index = 0;
                }
                else{
                    recv_pkt->header_crc = ptr[i];
                    pkt_index++;
                }
                break;
            case frame_fragment_flag:
                if(ptr[i] == 0){
                    recv_pkt->fragment_index = 0;
                    recv_pkt->fragment_number = 0;
                    pkt_state = frame_command;
                }
                else{
                    pkt_state = frame_fragment_index;
                }
                recv_pkt->payload_len--;
                break;
            case frame_fragment_index:
                recv_pkt->fragment_index = ptr[i];
                pkt_state = frame_fragment_count;
                recv_pkt->payload_len--;
                break;
            case frame_fragment_count:
                recv_pkt->fragment_number= ptr[i];
                pkt_state = frame_command;
                recv_pkt->payload_len--;
                break;
            case frame_command:
                if(pkt_index){
                    recv_pkt->command<<= 8;
                    recv_pkt->command += ptr[i];
                    pkt_state = frame_payload;
                    pkt_index = 0;
                }
                else{
                    recv_pkt->command = ptr[i];
                    pkt_index++;
                }
                recv_pkt->payload_len--;
                break;
            case frame_payload:
                if(pkt_index < recv_pkt->payload_len){
                    if(pkt_index < 200){
                        recv_pkt->data[pkt_index] = ptr[i];
                    }
                    pkt_index++;
                }
                else{
                    recv_pkt->data_crc = ptr[i];
                    pkt_state = frame_payload_check;
                    pkt_index = 0;
                }
                break;
            case frame_payload_check:
                recv_pkt->data_crc<<= 8;
                recv_pkt->data_crc += ptr[i];
                pkt_state = frame_wait_packet;
                pkt_index = 0;
                /* print summary */
#if 0                
                DEBUG(("\nPKT RECV :\n"));
                DEBUG(("direction  : %02x\n",recv_pkt->dir));
                DEBUG(("addr       : %04x\n",recv_pkt->addr));
                DEBUG(("seq_no     : %04x\n",recv_pkt->seq_no));
                DEBUG(("payload_len: %d\n",recv_pkt->payload_len));
                DEBUG(("header_crc : %04x\n",recv_pkt->header_crc));
                DEBUG(("data_crc   : %04x\n",recv_pkt->data_crc));
                DEBUG(("fragment   : %d/%d\n",recv_pkt->fragment_index,recv_pkt->fragment_number));
                DEBUG(("command    : %04x\n",recv_pkt->command));
                DEBUG(("payload    :\n"));
                for(j=0;j<recv_pkt->payload_len && j<20;j++){
                    DEBUG(("%02x ",recv_pkt->data[j]));
                }
                if(j<recv_pkt->payload_len){
                    DEBUG(("..."));
                }
                DEBUG(("\n"));
#endif
                ResponseSpp(); 
                
                break;
            default:
                break;
    		}
		}

		SourceDrop(source,size);
	}
}

void SppDisconnectReq(void)
{
    if(spp)
        SppDisconnect(spp);
}

void handleSppMessage(MessageId id, Message message)
{
	switch(id)
	{
		case SPP_INIT_CFM:
			break;
		case SPP_CONNECT_CFM:
		{
			SPP_CONNECT_CFM_T *cfm = (SPP_CONNECT_CFM_T *) message;
		
			DEBUG(("SPP_CONNECT_CFM result = %d\n", cfm->status));

			if(cfm->status == spp_connect_success)
			{
			    spp = cfm->spp;
				spp_sink = cfm->sink;
		        MessageSinkTask(spp_sink, &the_app->task); 
		        SppSendCmd(pkt_connect,sizeof(pkt_connect));
		        send_seq = 1;
		        memset(vin_info,0,sizeof(vin_info));
			}
		}			 
			break;
		case SPP_CONNECT_IND:	
		{
			SPP_CONNECT_IND_T* ind = (SPP_CONNECT_IND_T *) message;
			SppConnectResponseLazy(ind->spp, TRUE, &ind->addr, 1, FRAME_SIZE);
		    DEBUG(("SPP CONN IND\n"));
		}
			break;
		case SPP_DISCONNECT_IND:
		    DEBUG(("SPP Disconnect\n"));
			spp_sink = 0;
			spp = 0;
			break;
		default:
			break;
	}
}

