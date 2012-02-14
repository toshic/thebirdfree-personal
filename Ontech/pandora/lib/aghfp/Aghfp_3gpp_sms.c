#include "aghfp_private.h"
#include "aghfp_parse.h"
#include "aghfp_send_data.h"
#include "aghfp_ok.h"

#include <stdio.h>
#include <string.h>

#ifdef AGHFP_DEBUG_3GPP
#define DEBUG_PRINT_ENABLED
#define AGHFP_3GPP_DEBUG(x) { printf x; }
#else
#define AGHFP_3GPP_DEBUG(x)
#endif

bool wait_body;

void aghfpHandleListMessageText(Task task, const struct aghfpHandleListMessageText *cmgl)
{   
    uint16 i;
    AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CMGL="));
	for(i=0;i<cmgl->stat.length;i++)
		AGHFP_3GPP_DEBUG(("%c",cmgl->stat.data[i]));
	AGHFP_3GPP_DEBUG(("\n"));
	aghfpSendAtCmd(aghfp,"+CMGL: 1,\"REC UNREAD\",\"01012345678\",\"Mr. Smith\",\"11/12/24,17:45:03+09\"\r\nMessage.");
	aghfpSendOk(aghfp);
}

void aghfpHandleListMessage(Task task, const struct aghfpHandleListMessage *cmgl)
{
    AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CMGL=%d\n",cmgl->stat));
	aghfpSendAtCmd(aghfp,"+CMGL: 1,\"REC UNREAD\",\"01012345678\",\"Mr. Smith\",\"11/12/24,17:45:03+09\"\r\nMessage.");
	aghfpSendOk(aghfp);
}

void aghfpHandleReadMessage(Task task, const struct aghfpHandleReadMessage *cmgr)
{
	AGHFP *aghfp = (AGHFP *) task;

    MAKE_AGHFP_MESSAGE(AGHFP_READ_MESSAGE_IND);
	message->index = cmgr->index;
	message->aghfp = aghfp;
    MessageSend(aghfp->client_task, AGHFP_READ_MESSAGE_IND, message);

	AGHFP_3GPP_DEBUG(("AT+CMGR=%d\n",cmgr->index));
}

void aghfpHandleSelectPhonebookMemoryStorage(Task task, const struct aghfpHandleSelectPhonebookMemoryStorage *cpbs)
{
	uint16 i;
	AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CPBS="));
	for(i=0;i<cpbs->storage.length;i++)
		AGHFP_3GPP_DEBUG(("%c",cpbs->storage.data[i]));
	AGHFP_3GPP_DEBUG(("\n"));
	aghfpSendOk(aghfp);
}

void aghfpHandleNewMessageIndicationQuery(Task task)
{
    AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CNMI=?\n"));
	aghfpSendAtCmd(aghfp,"+CNMI: (0-3),(0-3),(0-3),(0,1),(0,1)");
    aghfpSendOk(aghfp);
}

void aghfpHandleNewMessageIndication(Task task, const struct aghfpHandleNewMessageIndication *cnmi)
{
	AGHFP *aghfp = (AGHFP *) task;

	MAKE_AGHFP_MESSAGE(AGHFP_NEW_MESSAGE_IND);
	message->mode = cnmi->mode;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_NEW_MESSAGE_IND, message);

	AGHFP_3GPP_DEBUG(("AT+CNMI=%d,%d,%d,%d,%d\n",cnmi->mode,cnmi->mt,cnmi->bm,cnmi->ds,cnmi->bfr));
	aghfpSendOk(aghfp);
}

void aghfpHandlePreferedMessageStorageQuery(Task task)
{
	AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CPMS=?\n"));
	aghfpSendAtCmd(aghfp,"+CPMS: (\"ME\")");
	aghfpSendOk(aghfp);
}

void aghfpHandlePreferedMessageStorageStatus(Task task)
{
    AGHFP *aghfp = (AGHFP *) task;
    AGHFP_3GPP_DEBUG(("AT+CPMS?\n"));
    aghfpSendAtCmd(aghfp,"+CPMS: \"ME\",0,100");
    aghfpSendOk(aghfp);
}

void aghfpHandlePreferedMessageStorage(Task task, const struct aghfpHandlePreferedMessageStorage *cpms)
{
	uint16 i;
	AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CPMS="));
	for(i=0;i<cpms->storage.length;i++)
		AGHFP_3GPP_DEBUG(("%c",cpms->storage.data[i]));
	AGHFP_3GPP_DEBUG(("\n"));
/*	aghfpSendAtCmd(aghfp,"+CPMS: \"ME\",0,100,\"ME\",0,100,\"ME\",0,100");*/
    aghfpSendAtCmd(aghfp,"+CPMS: 0,100");
	aghfpSendOk(aghfp);
}

void aghfpHandleMessageFormatQuery(Task task)
{
	AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CMGF=?\n"));
	aghfpSendAtCmd(aghfp,"+CMGF: (1)");
	aghfpSendOk(aghfp);
}

void aghfpHandleMessageFormat(Task task, const struct aghfpHandleMessageFormat *cmgf)
{
	AGHFP *aghfp = (AGHFP *) task;
	AGHFP_3GPP_DEBUG(("AT+CMGF=%d\n",cmgf->format));
	aghfpSendOk(aghfp);
}

void aghfpHandleSendMessage( Task task, const struct aghfpHandleSendMessage *cmgs)
{
	AGHFP *aghfp = (AGHFP *) task;
	uint16 i;

	MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_SEND_MESSAGE_IND, cmgs->sender.length);

	AGHFP_3GPP_DEBUG(("AT+CMGS="));
	for(i=0;i<cmgs->sender.length;i++)
		AGHFP_3GPP_DEBUG(("%c",cmgs->sender.data[i]));
	AGHFP_3GPP_DEBUG(("\n"));
	
	if (!message)
    {
    	/* Tell the HF that we couldn't handle its request */
        aghfpSendError(aghfp);
    }
    else
    {
        message->aghfp = aghfp;
	    message->size_number = cmgs->sender.length;
		memcpy(message->number, cmgs->sender.data, cmgs->sender.length * sizeof(uint8));

		MessageSend(aghfp->client_task, AGHFP_SEND_MESSAGE_IND, message);
    }
	
}

/* public function */
void AghfpSendNewMessageIndex(AGHFP *aghfp)
{
	char buf[4];

    aghfpAtCmdBegin(aghfp);
    aghfpAtCmdString(aghfp, "+CMTI: ");
    aghfpAtCmdString(aghfp, "\"ME\",");
	sprintf(buf, "%d", aghfp->sms_index++);
	aghfpAtCmdString(aghfp, buf);
    aghfpAtCmdEnd(aghfp);
}

void AghfpSendMessageBody(AGHFP *aghfp, char *number, char *message)
{
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CMGR: ");
	aghfpAtCmdString(aghfp, "\"REC UNREAD\",");
	
    aghfpAtCmdString(aghfp, "\"");
	aghfpAtCmdString(aghfp, number );
    aghfpAtCmdString(aghfp, "\",\"\",");

	aghfpAtCmdString(aghfp, "\"10/12/24,17:38:15+09\"\r\n");
	aghfpAtCmdString(aghfp, message);
	aghfpAtCmdEnd(aghfp);

	aghfpSendOk(aghfp);
}

void AghfpSendMessageAck(AGHFP *aghfp)
{
	char buf[4];
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CMGS: ");
	sprintf(buf, "%d", aghfp->sms_index++);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);
	aghfpSendOk(aghfp);
}
