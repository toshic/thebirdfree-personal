/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    dm_inquiry_handler.c        

DESCRIPTION
    This file contains the implementation of the inquiry management
    entity. This is responsible for ensuring that only one inquiry
    request at a time is issued to Bluestack. It also manages the 
    inquiry process by filtering on class of device and initiating
    remote name requests (if requested by the client).

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_inquiry_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>

/****************************************************************************
NAME	
    inquirySendResult

DESCRIPTION
    Send a CL_DM_INQUIRE_RESULT message to the client

RETURNS
    void
*/
static void inquirySendResult(Task theAppTask, inquiry_status status, const bdaddr *addr, uint32 dev_class, uint16 clock_offset, page_scan_rep_mode_t ps_rep_mode, page_scan_mode_t ps_mode, int16 rssi, uint8 size_eir_data, uint8* eir_data)
{
    if (theAppTask)
    {	
        /* Create an inquiry result message and populate it with the supplied args */
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_INQUIRE_RESULT,size_eir_data);
        message->status = status;
        
        /* Check the address has been set */
        if (addr)
            message->bd_addr = *addr;
        else
            BdaddrSetZero(&message->bd_addr);
        
        message->dev_class = dev_class;
        message->clock_offset = clock_offset;
        message->page_scan_rep_mode = connectionConvertPageScanRepMode_t(ps_rep_mode);
        message->page_scan_mode = connectionConvertPageScanMode_t(ps_mode);
        message->rssi = rssi;
		message->size_eir_data = size_eir_data;
		if(size_eir_data)
		{
			memmove(message->eir_data, eir_data, size_eir_data);
		}
		else
		{
			message->eir_data[0] = 0;
		}
        MessageSend(theAppTask, CL_DM_INQUIRE_RESULT, message);
    }
}


/****************************************************************************
NAME	
    inquiryParseEir

DESCRIPTION
    Take the EIR data structure received from bluestack and concat the data 
	into a single block to send to the application

RETURNS
    A pointer to a single block containing the EIR data and the size of the 
	eir data
*/
static uint8* inquiryParseEir(uint8* size_eir_data, uint8 *inquiry_data[HCI_EIR_DATA_PACKET_PTRS])
{
	uint8 i, j;
	uint8* eir_data = NULL;
	uint8* eir_data_part = NULL;
	uint8* p;
	
	/* Work out the number of EIR data parts */
	for(i=0 ; inquiry_data[i] != NULL ; i++)
	{}
	
	if(i>0)
	{
		/* Allocate memory for the EIR data */
		*size_eir_data = ((i-1) * HCI_EIR_DATA_BYTES_PER_PTR);
		eir_data = PanicUnlessMalloc(*size_eir_data);
	
		/* Point to first data part */
		p = eir_data;
	
		/* Copy all data except for last data part */
		for(j=0;j<(i-1);j++)
		{
			/* Get data part j and copy to p */
			eir_data_part = VmGetPointerFromHandle(inquiry_data[j]);
			memmove(p,eir_data_part,HCI_EIR_DATA_BYTES_PER_PTR);
			free(eir_data_part);
		
			/* Point to next data part */
			p += HCI_EIR_DATA_BYTES_PER_PTR;
		}
	
		/* Get the last data part */
		eir_data_part = VmGetPointerFromHandle(inquiry_data[i-1]);
		
		/* Work out how much actual data there is in it */
		for(j=(HCI_EIR_DATA_BYTES_PER_PTR-1); eir_data_part[j] == 0; j--)
		{}
		j++;
	
		/* Realloc eir data to fit in the last bit */
		eir_data = realloc(eir_data, *size_eir_data + j + 1);
	
		/* Point p to the new section (data may have moved during realloc) */
		p = eir_data + *size_eir_data;    
		*size_eir_data += (j+1);
	
		/* Copy in the data */
		memmove(p,eir_data_part,j);
		free(eir_data_part);
	
		/* Terminate data */
		p+=j;
		*p = 0;
	}
	else
	{
		*size_eir_data = 0;
		eir_data = NULL;
	}
	
	return eir_data;
}


/****************************************************************************
  NAME	
      connectionStartInquiry
  
  DESCRIPTION
      This function actually kicks off an inquiry message to BlueStack.
  
  RETURNS
      void
*/
void connectionHandleInquiryStart(connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_REQ_T *inquiry_req)
{
	/* Store application task in lock for use later */
	state->inquiryLock = inquiry_req->theAppTask;

	/* 
		If the class of device field is set, set an event filter so we 
		only receive inquiry results with devices with this class of 
		device. By default we filter in all devices so if class of 
		device mask set to zero we don't have to set the event filter.
	*/
	if (inquiry_req->class_of_device)
	{
		MAKE_PRIM_C(DM_HCI_SET_EVENT_FILTER);
		prim->filter_type = INQUIRY_RESULT_FILTER;
		prim->filter_condition_type = CLASS_OF_DEVICE_RESPONDED;
		prim->condition.class_mask.class_of_device = inquiry_req->class_of_device;

        /* 
			We don't care what the bits other than the specific class 
			of device are set to so set them to zero 
		*/
		prim->condition.class_mask.mask = inquiry_req->class_of_device;
		VmSendDmPrim(prim);
	}

    {
		/* Start an inquiry with the supplied parameters */
		MAKE_PRIM_C(DM_HCI_INQUIRY);
		prim->lap = inquiry_req->inquiry_lap;
		prim->inquiry_length = inquiry_req->timeout;
		prim->num_responses = inquiry_req->max_responses;
		VmSendDmPrim(prim);
	}
}


/****************************************************************************
NAME	
    connectionHandleInquiryResult

DESCRIPTION
    This function handles inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResult(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_T *inq_result)
{
	uint16 array;
	uint16 index;
	uint16 results_left = inq_result->num_responses;

	/* Iterate through the array of inquiry result ptrs */
	for (array = 0; array < (inq_result->num_responses+HCI_MAX_INQ_RESULT_PER_PTR-1)/HCI_MAX_INQ_RESULT_PER_PTR; array++)
	{
		uint16 res_this_block = HCI_MAX_INQ_RESULT_PER_PTR;
		HCI_INQ_RESULT_T *resarray = (HCI_INQ_RESULT_T *) VmGetPointerFromHandle(inq_result->result[array]);
		
		if (results_left < res_this_block)
			res_this_block = results_left;
		
		for (index = 0; index< res_this_block; index++)
		{
			/* Send an inquiry result message to the client */
			bdaddr	addr;
			connectionConvertBdaddr(&addr, &((resarray+index)->bd_addr));
			inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
				(resarray+index)->dev_class, (resarray+index)->clock_offset,
				(resarray+index)->page_scan_rep_mode, (resarray+index)->page_scan_mode,
				CL_RSSI_UNKNOWN, 0, NULL);
		}
		
		/* Free the data */
		free(resarray);
	}
}


/****************************************************************************
NAME	
    connectionHandleInquiryResultWithRssi

DESCRIPTION
    This function handles inquiry results with RSSI as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResultWithRssi(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_WITH_RSSI_T *inq_result)
{
	uint16 array;
	uint16 index;
	uint16 results_left = inq_result->num_responses;

	/* Iterate through the array of inquiry result ptrs */
	for (array = 0; array < (inq_result->num_responses+HCI_MAX_INQ_RESULT_PER_PTR-1)/HCI_MAX_INQ_RESULT_PER_PTR; array++)
	{
		uint16 res_this_block = HCI_MAX_INQ_RESULT_PER_PTR;
		HCI_INQ_RESULT_WITH_RSSI_T *resarray = (HCI_INQ_RESULT_WITH_RSSI_T *) VmGetPointerFromHandle(inq_result->result[array]);
		
		if (results_left < res_this_block)
			res_this_block = results_left;
		
		for (index = 0; index< res_this_block; index++)
		{
			/* Send an inquiry result message to the client */
			bdaddr	addr;
			connectionConvertBdaddr(&addr, &((resarray+index)->bd_addr));
			inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
				(resarray+index)->dev_class, (resarray+index)->clock_offset,
				(resarray+index)->page_scan_rep_mode, 0, (uint16)(resarray+index)->rssi,
				0, NULL);
		}
		
		/* Free the data */
		free(resarray);
	}
}


/****************************************************************************
NAME	
    connectionHandleExtendedInquiryResult

DESCRIPTION
    This function handles extended inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleExtendedInquiryResult(const connectionInquiryState *state, const DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T *ind)
{
	uint8 size_eir_data = 0;
	
	uint8* eir_data = inquiryParseEir(&size_eir_data, (uint8**)ind->eir_data_part);
		
	bdaddr	addr;
	connectionConvertBdaddr(&addr, &ind->result.bd_addr);
	
	/* Send an inquiry result message to the client */
	inquirySendResult(state->inquiryLock, inquiry_status_result, &addr, 
					 ind->result.dev_class, ind->result.clock_offset,
					 ind->result.page_scan_rep_mode, 0, (int16)ind->result.rssi,
					 size_eir_data, eir_data);
	
	/* Free memory */
	if(size_eir_data)
		free(eir_data);
}


/****************************************************************************
NAME	
    connectionHandleInquiryComplete

DESCRIPTION
    This function handles an inquiry complete indication fromBlueStack.

RETURNS
    void
*/
void connectionHandleInquiryComplete(connectionInquiryState *state)
{
	/* Tell the client the inquiry has completed */
    inquirySendResult(state->inquiryLock, inquiry_status_ready, 0, (uint32) 0, 0, 0, 0, CL_RSSI_UNKNOWN, 0, NULL);

	/* Reset the lock */
	state->inquiryLock = 0;

	{
		/* Reset the event filter so its not longer filtering on cod */
		MAKE_PRIM_C(DM_HCI_SET_EVENT_FILTER);
		prim->filter_type = INQUIRY_RESULT_FILTER;
		prim->filter_condition_type = NEW_DEVICE_RESPONDED;
		VmSendDmPrim(prim);
	}
}


/****************************************************************************
NAME	
    connectionHandleInquiryCancel

DESCRIPTION
    This function cancels an ongoing inquiry

RETURNS
    void
*/
void connectionHandleInquiryCancel(const connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T *cancel_req)
{
	/* Check if we have an inquiry in progress */
	if (state->inquiryLock)
	{
		if (state->inquiryLock == cancel_req->theAppTask)
		{
			/* We have an inquiry so issue the cancel */
			MAKE_PRIM_C(DM_HCI_INQUIRY_CANCEL);
			VmSendDmPrim(prim);
		}
	}
	else
	{	
		/* Send an inquiry complete to the app telling it we're ready */
		inquirySendResult(cancel_req->theAppTask, inquiry_status_ready, 0, (uint32) 0, 0, 0, 0, CL_RSSI_UNKNOWN, 0, NULL);
	}	
}

/****************************************************************************
NAME	
    connectionHandleReadRemoteName

DESCRIPTION
    This function will initiate a read of the remote name of the specified device

RETURNS
    void
*/
void connectionHandleReadRemoteName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T *req)
{
	/* Issue request to read the remote name */
	MAKE_PRIM_C(DM_HCI_REMOTE_NAME_REQUEST);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
	VmSendDmPrim(prim);

	/* Store application task in lock for use later */
	state->nameLock = req->theAppTask;
}


/****************************************************************************
NAME	
    remoteNameComplete

DESCRIPTION
    Send a CL_DM_REMOTE_NAME_COMPLETE message to the client

RETURNS
    void
*/
static void remoteNameComplete(Task theAppTask, const bdaddr *addr, hci_status status, char* name, uint16 length)
{
    if (theAppTask)
    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_REMOTE_NAME_COMPLETE, length);
        message->status = status;
        
        /* Check the address has been set */
        if (addr)
            message->bd_addr = *addr;
        else
            BdaddrSetZero(&message->bd_addr);
        
        message->size_remote_name = length;
        
        /* Populate remote name fields */
        if (length)
        {
            memmove(message->remote_name, name, length);	
            free(name);
        }	
        else
            message->remote_name[0] = 0;	
        
        /* Send onto Client */
        MessageSend(theAppTask, CL_DM_REMOTE_NAME_COMPLETE, message);
    }
    else
    {
        if (length)
            free(name);
    }
}

/****************************************************************************
NAME	
    connectionHandleRemoteNameComplete

DESCRIPTION
    Remote name result

RETURNS
    void
*/
void connectionHandleRemoteNameComplete(connectionInquiryState *state, const DM_HCI_REMOTE_NAME_COMPLETE_T* prim)
{
	bdaddr	addr;
	connectionConvertBdaddr(&addr, &prim->bd_addr);
	
	/* Providing the read was a success and we have a vaid name */
	if (!prim->status && prim->name_part[0])
	{
		uint16	length;
		uint8	i;

		/* Only handle the first segment */
		char*	name = VmGetPointerFromHandle(prim->name_part[0]);

		/* Find the length of the string in the first segment, limiting to 	MAX_NAME_LENGTH bytes */
		for(length = 0; length < MAX_NAME_LENGTH; length++)
		{
			if (name[length] == '\0')
				break;
		}
		
		name[length] = '\0';
		
		/* 	Free any other segments */
		for(i = 1;i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS;i++)
		{
			if (prim->name_part[i] != NULL)
				free(VmGetPointerFromHandle(prim->name_part[i]));
		}
		
		/* Remote name read, send Client message to notify them of the result */
		remoteNameComplete(state->nameLock, &addr, connectionConvertHciStatus(prim->status), name, length);
	}
	else
	{
		/* Read failed, send Client message to notify them of the result */
		remoteNameComplete(state->nameLock, &addr, connectionConvertHciStatus(prim->status), NULL, 0);
	}
	
	/* Reset resource lock */
	state->nameLock = 0;
}

/****************************************************************************
NAME	
    connectionHandleReadLocalName

DESCRIPTION
    This function will initiate a read of the local name of the device

RETURNS
    void
*/
void connectionHandleReadLocalName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T *req)
{	
	/* Issue request to read the remote name */
	MAKE_PRIM_C(DM_HCI_READ_LOCAL_NAME);
	VmSendDmPrim(prim);
	
	/* Store application task in lock for use later */
	state->nameLock = req->theAppTask;
}

/****************************************************************************
NAME	
    localNameComplete

DESCRIPTION
    Send a CL_DM_LOCAL_NAME_COMPLETE message to the client

RETURNS
    void
*/
static void localNameComplete(Task theAppTask, hci_status status, char* name, uint16 length)
{
    if (theAppTask)
    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_DM_LOCAL_NAME_COMPLETE, length);
        message->status = status;        
        message->size_local_name = length;
        
        /* Populate local name fields */
        if (length)
        {
            memmove(message->local_name, name, length);	
            free(name);
        }	
        else
            message->local_name[0] = '\0';	
        
        /* Send onto Client */
        MessageSend(theAppTask, CL_DM_LOCAL_NAME_COMPLETE, message);
    }
    else
    {
        if (length)
            free(name);
    }
}

/****************************************************************************
NAME	
    connectionHandleLocalNameComplete

DESCRIPTION
    Local name result

RETURNS
    void
*/
void connectionHandleLocalNameComplete(connectionInquiryState *state, const DM_HCI_READ_LOCAL_NAME_COMPLETE_T* prim)
{
	/* Providing the read was a success and we have a vaid name */
	if (!prim->status && prim->name_part[0])
	{
		uint16	length;
		uint8	i;

		/* Only handle the first segment */
		char*	name = VmGetPointerFromHandle(prim->name_part[0]);

		/* Find the length of the string in the first segment, limiting to 	MAX_NAME_LENGTH bytes */
		for(length = 0; length < MAX_NAME_LENGTH; length++)
		{
			if (name[length] == '\0')
				break;
		}
		
		name[length] = '\0';
		
		/* 	Free any other segments */
		for(i = 1;i < HCI_LOCAL_NAME_BYTE_PACKET_PTRS;i++)
		{
			if (prim->name_part[i] != NULL)
				free(VmGetPointerFromHandle(prim->name_part[i]));
		}
		
		/* Remote name read, send Client message to notify them of the result */
		localNameComplete(state->nameLock, connectionConvertHciStatus(prim->status), name, length);
	}
	else
	{
		/* Read failed, send Client message to notify them of the result */
		localNameComplete(state->nameLock, connectionConvertHciStatus(prim->status), NULL, 0);
	}
	
	/* Reset resource lock */
	state->nameLock = 0;
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryTx

DESCRIPTION
    This function will initiate a write of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleWriteInquiryTx(connectionReadInfoState* infoState, connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T *req)
{
	/* Check command supported by firmware */
	if (infoState->version != bluetooth_unknown)
	{
		MAKE_PRIM_C(DM_HCI_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL_REQ);
		prim->tx_power = req->tx_power;
		VmSendDmPrim(prim);
	}
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryTx

DESCRIPTION
    This function will initiate a read of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleReadInquiryTx(connectionReadInfoState* infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T *req)
{
	/* Check command supported by firmware */
	if (infoState->version != bluetooth_unknown)
	{
		/* Issue request to read the inquiry tx */
		MAKE_PRIM_C(DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_REQ);
		VmSendDmPrim(prim);

		/* Store application task in lock for use later */
		state->inquiryLock = req->theAppTask;		
	}
	else
	{
		/* Tell the app this is unsupported */
		MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_TX_CFM);
		message->status = hci_error_unsupported_feature;
		message->tx_power = 0;
		MessageSend(req->theAppTask, CL_DM_READ_INQUIRY_TX_CFM, message);
	}
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryTx

DESCRIPTION
    This function handles a read inquiry tx result

RETURNS
    void
*/
void connectionHandleReadInquiryTxComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_COMPLETE_T *cfm)
{
	if (state->inquiryLock)
	{
		MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_TX_CFM);
		message->status = connectionConvertHciStatus(cfm->status);
		message->tx_power = cfm->tx_power;
		MessageSend(state->inquiryLock, CL_DM_READ_INQUIRY_TX_CFM, message);
	}

	/* Clear resource lock */	
	state->inquiryLock = 0;
}

/****************************************************************************
NAME	
    connectionHandleWriteIacLapRequest

DESCRIPTION
    Write IAC

RETURNS
    void
*/
void connectionHandleWriteIacLapRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T *req)
{
	uint16 index;
	uint24_t *ptr;
	MAKE_PRIM_C(DM_HCI_WRITE_CURRENT_IAC_LAP);

	/* Store application task in lock for use later */
	state->inquiryLock = req->theAppTask;

	/* Store number of IACs */
	prim->num_current_iac = req->num_iac;
	
	/* Zero the entries */
	memset(prim->iac_lap, 0, sizeof(uint24_t *) * HCI_IAC_LAP_PTRS);

	/* Allocate memory block */
	prim->iac_lap[0] = (uint24_t *)malloc(sizeof(uint24_t) * prim->num_current_iac);
	ptr = prim->iac_lap[0];

	/* Copy IACs */
	for (index = 0; index < req->num_iac; index++)
		ptr[index] = req->iac[index];
	
	/* Vm friendly */
	prim->iac_lap[0] = VmGetHandleFromPointer(prim->iac_lap[0]);

	/* Send request */
	VmSendDmPrim(prim);
}

/****************************************************************************
NAME	
    connectionHandleWriteIacLapComplete

DESCRIPTION
    Write IAC confirmation

RETURNS
    void
*/
void connectionHandleWriteIacLapComplete(connectionInquiryState *state, const DM_HCI_WRITE_CURRENT_IAC_LAP_COMPLETE_T *prim)
{
	if (state->inquiryLock)
	{
	    MAKE_CL_MESSAGE(CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM);
	    message->status = (!prim->status) ? success : fail;       
			message->status = fail;        
		MessageSend(state->inquiryLock, CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM, message);
	}
	
	/* Reset resource lock */
	state->inquiryLock = 0;
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryModeRequest

DESCRIPTION
    Write inquiry mode

RETURNS
    void
*/
void connectionHandleWriteInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T *req)
{
    /* Issue inquiry mode change with the supplied parameter */
	MAKE_PRIM_C(DM_HCI_WRITE_INQUIRY_MODE);
	prim->mode = connectionConvertInquiryMode_t(req->mode);
	VmSendDmPrim(prim);

	/* Store application task in lock for use later */
	state->inquiryLock = req->theAppTask;
}

/****************************************************************************
NAME	
    connectionHandleWriteInquiryModeComplete

DESCRIPTION
    Write inquiry mode result

RETURNS
    void
*/
void connectionHandleWriteInquiryModeComplete(connectionInquiryState *state, const DM_HCI_WRITE_INQUIRY_MODE_COMPLETE_T *prim)
{
    MAKE_CL_MESSAGE(CL_DM_WRITE_INQUIRY_MODE_CFM);
    message->status = (!prim->status) ? success : fail;       
    MessageSend(state->inquiryLock, CL_DM_WRITE_INQUIRY_MODE_CFM, message);

    /* Clear lock */
    state->inquiryLock = 0;
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryModeRequest

DESCRIPTION
    Read inquiry mode

RETURNS
    void
*/
void connectionHandleReadInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T *req)
{
	/* Issue inquiry mode read */
	MAKE_PRIM_C(DM_HCI_READ_INQUIRY_MODE);
	VmSendDmPrim(prim);

	/* Store application task in lock for use later */
	state->inquiryLock = req->theAppTask;
}

/****************************************************************************
NAME	
    connectionHandleReadInquiryModeComplete

DESCRIPTION
    Read inquiry mode result

RETURNS
    void
*/
void connectionHandleReadInquiryModeComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_MODE_COMPLETE_T *prim)
{
    MAKE_CL_MESSAGE(CL_DM_READ_INQUIRY_MODE_CFM);
    message->status = (!prim->status) ? success : fail;
    message->mode = connectionConvertInquiryMode(prim->mode);        
    MessageSend(state->inquiryLock, CL_DM_READ_INQUIRY_MODE_CFM, message);
    
    /* Clear lock */
    state->inquiryLock = 0;
}


/****************************************************************************
NAME
    connectionHandleWriteEirDataRequest

DESCRIPTION
    Handles request for Writing the Extended Inquiry Data. 

RETURNS
    void
*/
void connectionHandleWriteEirDataRequest(connectionReadInfoState *infoState, const CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T *req)
{
    uint8 i;
    uint8 *p;
    uint8 octets_copied = 0;
    uint8 remainder;
    uint8 eir_data_length;

	if (infoState->version == bluetooth2_1)
	{
    	MAKE_PRIM_C(DM_HCI_WRITE_EXTENDED_INQUIRY_RESPONSE_DATA);
    	prim->fec_required = req->fec_required;

   		eir_data_length = (req->size_eir_data <= HCI_EIR_DATA_LENGTH)? req->size_eir_data : HCI_EIR_DATA_LENGTH;  

    	for (i=0; i<(eir_data_length / HCI_EIR_DATA_BYTES_PER_PTR); i++)
    	{
			p = PanicUnlessMalloc(HCI_EIR_DATA_BYTES_PER_PTR);
			memmove(p, req->eir_data + octets_copied, HCI_EIR_DATA_BYTES_PER_PTR);
        	octets_copied += HCI_EIR_DATA_BYTES_PER_PTR;
			prim->eir_data_part[i] = VmGetHandleFromPointer(p);
    	}

    	remainder = eir_data_length % HCI_EIR_DATA_BYTES_PER_PTR;
    	if (remainder)
    	{
        	p = PanicUnlessMalloc(HCI_EIR_DATA_BYTES_PER_PTR);
        	memmove(p, req->eir_data+octets_copied, remainder);
        	memset(p + remainder, 0, HCI_EIR_DATA_BYTES_PER_PTR - remainder);
			prim->eir_data_part[i] = VmGetHandleFromPointer(p);
        	i++;
    	}
    	for (; i < HCI_EIR_DATA_PACKET_PTRS; i++)
    	{
        	prim->eir_data_part[i] = NULL;
    	}

    	VmSendDmPrim(prim);
	}
}

/****************************************************************************
NAME
    connectionHandleReadEirDataRequest

DESCRIPTION
    Handles request for Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataRequest(connectionReadInfoState *infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_EIR_DATA_REQ_T *req)
{
	if (infoState->version == bluetooth2_1)
	{
		MAKE_PRIM_C(DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA);
		VmSendDmPrim(prim);

		/* Store application task in lock for use later */
		state->inquiryLock = req->task;
	}
	else
	{
		/* Not supported, tell the app */
		MAKE_CL_MESSAGE(CL_DM_READ_EIR_DATA_CFM);
		message->status = hci_error_unsupported_feature;
		message->fec_required = FALSE;
		message->size_eir_data = 0;
		message->eir_data[0] = 0;
    	MessageSend(state->inquiryLock, CL_DM_READ_EIR_DATA_CFM, message);
	}
}


/****************************************************************************
NAME
    connectionHandleReadEirDataComplete

DESCRIPTION
    Handles result from Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataComplete(connectionInquiryState *state, const DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_COMPLETE_T *cfm)
{
	if (state->inquiryLock)
	{
		uint8 size_eir_data = 0;
		uint8* eir_data = inquiryParseEir(&size_eir_data, (uint8**)cfm->eir_data_part);
		
		MAKE_CL_MESSAGE_WITH_LEN(CL_DM_READ_EIR_DATA_CFM, size_eir_data);
		message->status = connectionConvertHciStatus(cfm->status);
		message->fec_required = cfm->fec_required;
		message->size_eir_data = size_eir_data;
		memmove(message->eir_data, eir_data, size_eir_data);
    	MessageSend(state->inquiryLock, CL_DM_READ_EIR_DATA_CFM, message);
		
		free(eir_data);
	}
	
	/* Clear lock */
	state->inquiryLock = 0;
}
