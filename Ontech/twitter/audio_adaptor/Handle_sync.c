#include "audioAdaptor_private.h"
#include "Handle_sync.h"
#include <syncs.h>
/* Interface Functions */

/*
  Initialise the Sync System
*/
void initSync(void)
{
	DEBUG(("####initSync\n"));
	/* Initialise the Sync library */
	SyncsInit(&the_app->task, 1, 0x06);
}

static void handleSYNCInitCfm(SYNCS_INIT_CFM_T *pCfm)
{
	DEBUG(("SYNCS_INIT_CFM_T %d\n",pCfm->status));
}
static void handleSYNCConnectInd(SYNCS_CONNECT_IND_T *pInd)
{
	DEBUG(("SYNCS_CONNECT_IND_T\n"));
	SyncsConnectResponse(pInd->syncs, TRUE, 255);
}
static void handleSYNCConnectCfm(SYNCS_CONNECT_CFM_T *pCfm)
{
	DEBUG(("SYNCS_CONNECT_CFM_T\n"));
}
static void handleSYNCDisconnectInd(SYNCS_DISCONNECT_IND_T *pInd)
{
	DEBUG(("SYNCS_DISCONNECT_IND_T\n"));
}

void handleSyncMessages(MessageId pId, Message pMessage)
{
	switch (pId)
	{
	case SYNCS_INIT_CFM:
		handleSYNCInitCfm((SYNCS_INIT_CFM_T *)pMessage);
		break;
	case SYNCS_CONNECT_IND:
		handleSYNCConnectInd((SYNCS_CONNECT_IND_T *)pMessage);
		break;
	case SYNCS_CONNECT_CFM:
		handleSYNCConnectCfm((SYNCS_CONNECT_CFM_T *)pMessage);
		break;
	case SYNCS_DISCONNECT_IND:
		handleSYNCDisconnectInd((SYNCS_DISCONNECT_IND_T *)pMessage);
		break;
		
	default:
		DEBUG(("Unhandled SYNC Message\n"));
		break;
	}
}

