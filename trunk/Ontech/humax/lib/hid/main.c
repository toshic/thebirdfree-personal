/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of BlueLab 4.1.2-Release */
#include "hid.h"
#include "hid_keyboard_service_record.h"

#include <connection.h>
#include <message.h>
#include <vm.h>
#include <stream.h>
#include <bdaddr.h>
#include <stdio.h>
#include <source.h>
#include <string.h>
#include <assert.h>
#include <panic.h>
#include <stdlib.h>


#define MAIN_DEBUG if (1) printf


typedef struct 
{
	TaskData			task;
	HID_LIB *			hidLib;
	bdaddr				bd_addr;
    
    Source              hid_source;
    Sink                hid_sink;
    Source              uart_source;
}taskState;

/* The app task */
taskState app;

static const hid_config app_config = 
{
    sizeof(hid_keyboard_service_record),
    hid_keyboard_service_record
};

/* Lower power table  */
static const lp_power_table app_power_table[]=
{
	/* mode,    	min_interval, max_interval, attempt, timeout, duration */
	{lp_active,		0,	    	  0,			0,		 0,		5},	/* Active mode for 5 seconds */
	{lp_sniff,		32,	      	  200,			1,		 8,	    0}	/* Enter sniff mode (20-40mS)*/
};

static const hid_connection_config app_connection_config = 
{
    2, app_power_table, /* Power table */
    11250,              /* Latency (11.25ms) */
};


/* Task 1 handler function */
static void app_handler(Task task, MessageId id, Message message)
{
	switch(id)
    {
		case CL_INIT_CFM:
		{
            CL_INIT_CFM_T *prim = (CL_INIT_CFM_T *)message;
			MAIN_DEBUG("CL_INIT_CFM\n");
            
            /* Connection Library initialisation was a success, initialise the HID library */
    		if (prim->status == success)
                HidInit(&app.task, &app_config);
		}
		break;

		case HID_INIT_CFM:
		{
			HID_INIT_CFM_T *prim = (HID_INIT_CFM_T *)message;
			MAIN_DEBUG("HID_INIT_CFM\n");
            
			/* Check for hid_init_success. What do we do if it failed? */
			if (prim->status == hid_init_success)
			{
                /* Record HID instance */
                if (!app.hidLib)
                {
        			MAIN_DEBUG("Init completed!\n");
				    app.hidLib = prim->hid_lib;				
                    
#if 0
					/* Configure HID and UART streams */
    				StreamConfigure(VM_STREAM_HID_BUTTONS, 0);
	       			StreamConfigure(VM_STREAM_HID_WHEEL, 0);
        			StreamConfigure(VM_STREAM_HID_SENSOR, SENSOR_UART);
        			StreamConfigure(VM_STREAM_HID_SAMPLE_RATE, 0);
        			app.hid_source = StreamHidSource();
    				StreamConfigure(VM_STREAM_UART_CONFIG, VM_STREAM_UART_LATENCY);
	   			    app.uart_source = StreamUartSource();
		      		app.hid_sink = StreamHidSink();
    				(void)StreamConnect(app.uart_source, app.hid_sink);
#endif
                    
                    /* Turn off security */
        			ConnectionSmRegisterIncomingService(protocol_l2cap, 0, secl_none);
                    ConnectionSmSetSecurityMode(task, sec_mode1_non_secure, hci_enc_mode_off);
                    
                    /* Write class of device */
				    ConnectionWriteClassOfDevice(HID_MAJOR_DEVICE_CLASS | HID_MINOR_KEYBOARD);
                    
                    /* Set device to inquiry scan mode, waiting for discovery */
	       			ConnectionWriteInquiryscanActivity(0x400, 0x200);
			     	ConnectionSmSetSdpSecurityIn(TRUE);
                    
                    /* Make this device discoverable (inquiry scan), and connectable (page scan) */
				    ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);
                }
			}
		}
		break;
        
        case HID_CONNECT_IND:
        {
            HID_CONNECT_IND_T *prim = (HID_CONNECT_IND_T *)message;
            MAIN_DEBUG("HID_CONNECT_IND\n");
            
#if 0
            /* Turn off any inquiry scan */
			ConnectionWriteScanEnable(0);
#endif
            
            /* Accept connection */
            HidConnectResponse(prim->hid, TRUE, &app_connection_config);
        }
        break;
        
        case HID_CONNECT_CFM:
        {
            MAIN_DEBUG("HID_CONNECT_CFM\n");
            
            /* TODO: Do something with interrupt sink */
        }
        break;

        case HID_DISCONNECT_IND:
        {
            MAIN_DEBUG("HID_DISCONNECT_IND\n");
        }
        break;
        
        case HID_GET_IDLE_IND:
        {
            HID_GET_IDLE_IND_T *prim = (HID_GET_IDLE_IND_T *)message;
            MAIN_DEBUG("HID_GET_IDLE_IND\n");
            HidGetIdleResponse(prim->hid, hid_success, 1);
        }
        break;

        case HID_SET_IDLE_IND:
        {
            HID_SET_IDLE_IND_T *prim = (HID_SET_IDLE_IND_T *)message;
            MAIN_DEBUG("HID_SET_IDLE_IND, idle=%d\n", prim->idle_rate);
            HidSetIdleResponse(prim->hid, hid_success);
        }
        break;

        case HID_GET_PROTOCOL_IND:
        {
            HID_GET_PROTOCOL_IND_T *prim = (HID_GET_PROTOCOL_IND_T *)message;
            MAIN_DEBUG("HID_GET_PROTOCOL_IND\n");
            HidGetProtocolResponse(prim->hid, hid_success, hid_protocol_report);
        }
        break;

        case HID_SET_PROTOCOL_IND:
        {
            HID_SET_PROTOCOL_IND_T *prim = (HID_SET_PROTOCOL_IND_T *)message;
            MAIN_DEBUG("HID_SET_PROTOCOL_IND, protocol=%d\n", prim->protocol);
            HidSetProtocolResponse(prim->hid, hid_success);
        }
        break;
        
        case HID_GET_REPORT_IND:
        {
            HID_GET_REPORT_IND_T *prim = (HID_GET_REPORT_IND_T *)message;
            uint8 *data = (uint8 *)malloc(prim->report_length);
            uint16 index;
            for (index = 0; index < prim->report_length; index++)
            	data[index] = index & 0xFF;
            MAIN_DEBUG("HID_GET_REPORT_IND, type=%d, length=%d\n", prim->report_type, prim->report_length);
            HidGetReportResponse(prim->hid, hid_success, prim->report_type, prim->report_length, data);
            free(data);
        }
        break;
        
        case HID_SET_REPORT_IND:
        {
            HID_SET_REPORT_IND_T *prim = (HID_SET_REPORT_IND_T *)message;
            uint16 index;
            MAIN_DEBUG("HID_SET_REPORT_IND, type=%d, length=%d, data=", prim->report_type, prim->report_length);
            for (index = 0; index < prim->report_length; index++)
            	MAIN_DEBUG("%x ", prim->report_data[index]);
            MAIN_DEBUG("\n");
            HidSetReportResponse(prim->hid, hid_success);
        }
        break;

        case HID_DATA_IND:
        {
            HID_DATA_IND_T *prim = (HID_DATA_IND_T *)message;
            uint16 index;
            MAIN_DEBUG("HID_DATA_IND, length=%d, data=", prim->data_length);
            for (index = 0; index < prim->data_length; index++)
            	MAIN_DEBUG("%x ", prim->data[index]);
            MAIN_DEBUG("\n");
        }
        break;

        case CL_SM_PIN_CODE_IND:
		{
			CL_SM_PIN_CODE_IND_T *prim = (CL_SM_PIN_CODE_IND_T *)message;
            MAIN_DEBUG("CL_SM_PIN_CODE_IND\n");
            
            ConnectionSmPinCodeResponse(&prim->bd_addr, 4, (unsigned char *)"1234"); 
		}
		break;
		
		case CL_SM_AUTHORISE_IND:
        {
            CL_SM_AUTHORISE_IND_T *prim = (CL_SM_AUTHORISE_IND_T *)message;
            MAIN_DEBUG("CL_SM_AUTHORISE_IND\n");
            
            /* For now just authorise but we should check the protocol etc */
            ConnectionSmAuthoriseResponse(&prim->bd_addr, prim->protocol_id, prim->channel, prim->incoming, 1);
        }
        break;
        
#if 0
		case CL_SM_AUTHENTICATE_CFM:
        {
            CL_SM_AUTHENTICATE_CFM_T *prim = (CL_SM_AUTHENTICATE_CFM_T *)message;
            MAIN_DEBUG("CL_SM_AUTHENTICATE_CFM\n");
            
    		if (prim->status == authSuccess)
            {
                MAIN_DEBUG("Pairing success, now set the trust level\n");
                ConnectionSmSetTrustLevel(&prim->bd_addr, TRUE);
            }
            else if (prim->status == authFail)
            {
                MAIN_DEBUG("Pairing failed\n");
            }
            else
            {
                MAIN_DEBUG("Pairing timeout\n");
            }
        }
		break;
#endif
        
		default:
		{
			/* An unexpected message has arrived - must handle it */
			MAIN_DEBUG("main app - message type not yet handled 0x%x\n", id);
		}
		break;
    }
}
        

int main(void)
{
	/* Make sure Uart has been successfully initialised before running */
	if (StreamUartSource())
	{
        /* Set up task 1 handler */
	    app.task.handler = app_handler;

        /* Init the Connection Manager */
      	ConnectionInit(&app.task);

	    /* Start the message scheduler loop */
     	MessageLoop();
    }
    else
    {
        MAIN_DEBUG("Failed to initialise UART\n");
    }   
	
    return 0;
}
