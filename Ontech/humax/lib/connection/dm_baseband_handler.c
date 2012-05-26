/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_baseband_handler.c        

DESCRIPTION
    This is the baseband management entity and is used to route responses
	back to the task that initiated the request.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_baseband_handler.h"

#include <vm.h>


/****************************************************************************
NAME	
    connectionHandleReadClassOfDeviceRequest

DESCRIPTION
    Handles the internal message that initiates the read class of device

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T *req)
{
	/* Issue the request to BlueStack */
	MAKE_PRIM_C(DM_HCI_READ_CLASS_OF_DEVICE);
	VmSendDmPrim(prim);

	/* Not currently processing one of these so set the lock */
	state->stateInfoLock = req->theAppTask;
    state->sink = 0;
}


/****************************************************************************
NAME	
    connectionHandleReadClassOfDeviceComplete

DESCRIPTION
    Handles the read class of device complete message from BlueStack

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceComplete(connectionReadInfoState *state, const DM_HCI_READ_CLASS_OF_DEVICE_COMPLETE_T *cfm)
{
    if (state->stateInfoLock)
    {
        /* Send result to task that originated the request */
        MAKE_CL_MESSAGE(CL_DM_CLASS_OF_DEVICE_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        message->dev_class = cfm->dev_class;
        MessageSend(state->stateInfoLock, CL_DM_CLASS_OF_DEVICE_CFM, message);
        
        /* Reset the resource lock */
        state->stateInfoLock = 0;
    }
}


/****************************************************************************
NAME	
    connectionHandleWritePageScanActivityRequset

DESCRIPTION
    Handles the internal message requesting a page scan activity write

RETURNS
    void
*/
void connectionHandleWritePageScanActivityRequset(const CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_PAGESCAN_ACTIVITY);
	prim->pagescan_interval = req->ps_interval;
	prim->pagescan_window = req->ps_window;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteInquiryScanActivityRequset

DESCRIPTION
    Handles the internal message requesting a inquiry scan activity write

RETURNS
    void
*/
void connectionHandleWriteInquiryScanActivityRequest(const CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_INQUIRYSCAN_ACTIVITY);
	prim->inqscan_interval = req->is_interval;
	prim->inqscan_window = req->is_window;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteScanEnableRequset

DESCRIPTION
    Handles the internal message requesting a write scan enable

RETURNS
    void
*/
void connectionHandleWriteScanEnableRequest(const CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_SCAN_ENABLE);
	prim->scan_enable = connectionConvertHciScanEnable(req->mode);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCodRequset

DESCRIPTION
    Handles the internal message requesting a write class of device

RETURNS
    void
*/
void connectionHandleWriteCodRequset(const CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T *req)
{
	MAKE_PRIM_C(DM_HCI_WRITE_CLASS_OF_DEVICE);
	prim->dev_class = req->class_of_device;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCachedPageModeRequest

DESCRIPTION
    Handles the internal message requesting a to store the page mode for a given device

RETURNS
    void
*/
void connectionHandleWriteCachedPageModeRequest(const CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T *req)
{
	MAKE_PRIM_T(DM_WRITE_CACHED_PAGE_MODE_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
    prim->page_scan_mode = connectionConvertPageScanMode(req->ps_mode);
    prim->page_scan_rep_mode = connectionConvertPageScanRepMode(req->ps_rep_mode);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteCachedClkOffsetRequest

DESCRIPTION
    Handles the internal message requesting a to store the clock offset for 
	a given device

RETURNS
    void
*/
void connectionHandleWriteCachedClkOffsetRequest(const CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T *req)
{
	/* Send the request to BlueStack */
	MAKE_PRIM_T(DM_WRITE_CACHED_CLOCK_OFFSET_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
	prim->clock_offset = req->clock_offset;
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleClearParamCacheRequest

DESCRIPTION
    Handles the internal message requesting to clear the parameter cache for 
	a given device

RETURNS
    void
*/
void connectionHandleClearParamCacheRequest(const CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T *req)
{
	/* Send the request to BlueStack */
	MAKE_PRIM_T(DM_CLEAR_PARAM_CACHE_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
	VmSendDmPrim(prim);
}


/****************************************************************************
NAME	
    connectionHandleWriteFlushTimeoutRequest

DESCRIPTION
    Set the flush timeout for a particular ACL.

RETURNS
    void
*/
void connectionHandleWriteFlushTimeoutRequest(const CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T *req)
{
	bdaddr addr;

	/* 
		Convert the sink to the address of the underlying ACL and 
		only issue this if we have an ACL  
	*/
	if (SinkGetBdAddr(req->sink, &addr))
	{
		/* Send the request to BlueStack */
		MAKE_PRIM_C(DM_HCI_WRITE_AUTO_FLUSH_TIMEOUT);
		connectionConvertBdaddr_t(&prim->bd_addr, &addr);
		prim->timeout = req->flush_timeout;
		VmSendDmPrim(prim);
	}
}


/****************************************************************************/
void connectionHandleChangeLocalName(const CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T *req)
{
	uint16 i;
	MAKE_PRIM_C(DM_HCI_CHANGE_LOCAL_NAME); 
	prim->common.length = sizeof(DM_HCI_CHANGE_LOCAL_NAME_T) + req->length_name;

	if (req->length_name)
		prim->name_part[0] = VmGetHandleFromPointer(req->name);
	else
		prim->name_part[0] = 0;

	/* Set all the other ptrs in the message to null */
	for (i = 1; i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS; i++)
		prim->name_part[i] = 0;

	VmSendDmPrim(prim);
}
