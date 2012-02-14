#include "SppServer.h"
#include <string.h>
#include <panic.h>
#include <stdlib.h>

#define FRAME_SIZE		0

static Sink spp_sink;
static SPP *spp;

static const unsigned char pandora_service_record [] =
{
    0x09, 0x00, 0x01,   /* ServiceClassIDList(0x0001) */
    0x35, 0x11,          /* DataElSeq 17 bytes */
    0x1c, 0x45, 0x39, 0x94, 0xd5, 0xd5, 0x8b, 0x96, 0xf9, 0x66, 0x16, 0xb3, 0x7f, 0x58, 0x6b, 0xa2, 0xec, /* HKMC pandora UUID */
    0x09, 0x00, 0x04,   /* ProtocolDescriptorList(0x0004) */
    0x35, 0x0c,         /* DataElSeq 12 bytes */
    0x35, 0x03,         /* DataElSeq 3 bytes */
    0x19, 0x01, 0x00,   /* UUID L2CAP(0x0100) */
    0x35, 0x05,         /* DataElSeq 5 bytes */
    0x19, 0x00, 0x03,   /* UUID RFCOMM(0x0003) */
    0x08, 0x00,         /* uint8 0x00 */
    0x09, 0x00, 0x06,   /* LanguageBaseAttributeIDList(0x0006) */
    0x35, 0x09,         /* DataElSeq 9 bytes */
    0x09, 0x65, 0x6e,   /* uint16 0x656e */
    0x09, 0x00, 0x6a,   /* uint16 0x006a */
    0x09, 0x01, 0x00,   /* uint16 0x0100 */
    0x09, 0x01, 0x00,   /* ServiceName(0x0100) = "OnTech" */
    0x25, 0x06,         /* String length 6 */
    'O','n','T','e','c', 'h'
};

void sppDevInit(void)
{
    spp_init_params init;

    init.client_recipe = 0;
    init.size_service_record = sizeof(pandora_service_record);
	init.service_record = (uint8 *) pandora_service_record;
	init.no_service_record = 0;
	
    /* Initialise the spp profile lib, stating that this is device B */ 
    SppInitLazy(&the_app->task, &the_app->task, &init);
}

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
#if 1
    DEBUG((">>"));
#endif    
	while((size = SourceSize(source)) > 0 )
	{
		ptr = (uint8*)SourceMap(source);

/* need to check maximum Source size */
		for(i=0;i<size;i++){
#if 1		
		    if(i<16){
    			DEBUG(("%02x ",ptr[i]));
    		}else if(i==16){
    			DEBUG(("..."));
            }    		
#endif
    continue;

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
		        /*SppSendCmd(pkt_connect,sizeof(pkt_connect));*/
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

