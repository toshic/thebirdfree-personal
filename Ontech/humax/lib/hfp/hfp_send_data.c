/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_send_data.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_send_data.h"
#include "hfp_common.h"

#include <panic.h>
#include <string.h>
#include <util.h>


/*lint -e525 -e830 */


/* Optimised string matching for AT cmds of the form AT+XXXX */
static uint16 matchCmd(HFP *hfp, uint8 *s)
{
  uint16 h = 0 ;
      
  h = UtilHash((uint16 *) s , 4 , h);

  switch(h)
  {
    case 2652:
      {
        const char *literal = "BLDN";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpBldnCmdPending;
	        return 1;
        }
      }
      break;
    case 3248:
      {
        const char *literal = "CHUP";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpChupCmdPending;
            return 1;
        }
      }
      break;
    case 5014:
      {
        const char *literal = "CCWA";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCcwaCmdPending;
			return 1;
        }
      }
      break;
    case 6463:
      {
        const char *literal = "BINP";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpBinpCmdPending;
            return 1;
        }
      }
      break;
    case 11462:
      {
        const char *literal = "NREC";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpNrecCmdPending;
            return 1;
        }
      }
      break;
    case 12037:
      {
        const char *literal = "VGS=";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpVgsCmdPending;;

			/* Insert the vol setting into the VGS command, making assumption that command */
			/* is formatted as "AT+VGS=00"                                                 */
			*(s+4) = '0' + hfp->vol_setting / 10;
			*(s+5) = '0' + hfp->vol_setting % 10;			
			
			/* Reset the vol setting we no longer have a cmd pending */
			hfp->vol_setting = 0xff;

	        return 1;
        }
      }
      break;
    case 13251:
      {
        const char *literal = "CHLD";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
            /* Make assumption that cmd is formatted exactly as:  */
            /*    "AT+CHLD=<n>\r"       where <n>=0..4     or        */
            /*    "AT+CHLD=<n><idx>\r"  where <n>=1|2 and <idx>=1..* */
            switch ( *(s+5)-'0' )
            {
                case 0:
			        hfp->at_cmd_resp_pending = hfpChldZeroCmdPending;
			        return 1;
			    case 1:
			        if ( *(s+6)!='\r' )
			        {
			            hfp->at_cmd_resp_pending = hfpChldOneIdxCmdPending;
			        }
			        else
			        {
			            hfp->at_cmd_resp_pending = hfpChldOneCmdPending;
		            }
			        return 1;
			    case 2:
			        if ( *(s+6)!='\r' )
			        {
			            hfp->at_cmd_resp_pending = hfpChldTwoIdxCmdPending;
		            }
		            else
		            {
			            hfp->at_cmd_resp_pending = hfpChldTwoCmdPending;
		            }
			        return 1;
			    case 3:
			        hfp->at_cmd_resp_pending = hfpChldThreeCmdPending;
			        return 1;
			    default:
			        break;
		    }
        }
      }
      break;
    case 14031:
      {
        const char *literal = "CLCC";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpClccCmdPending;
            return 1;
        }
      }
      break;
    case 15693:
      {
        const char *literal = "BRSF";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpBrsfCmdPending;
			return 1;
        }
      }
      break;
    case 17925:
      {
        const char *literal = "BVRA";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpBvraCmdPending;
            return 1;
        }
      }
      break;
    case 18805:
      {
        const char *literal = "CNUM";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCnumCmdPending;
            return 1;
        }
      }
      break;
    case 32444:
      {
        const char *literal = "CKPD";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCkpdCmdPending;
			return 1;
        }
      }
      break;
    case 36895:
      {
        const char *literal = "VGM=";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpVgmCmdPending;
            return 1;
        }
      }
      break;
    case 37304:
      {
        const char *literal = "BTRH";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
            /* Make assumption that cmd is formatted exactly as:  */
            /*    "AT+BTRH?"  or                                  */
            /*    "AT+BTRH=n" where n=0..2                        */
            if ( *(s+4)=='?' )
            {
			    hfp->at_cmd_resp_pending = hfpBtrhReqCmdPending;
			    return 1;
		    }
		    else
		    {
    		    switch ( *(s+5)-'0' )
    		    {
        		    case 0:
            			hfp->at_cmd_resp_pending = hfpBtrhZeroCmdPending;
                        return 1;
        		    case 1:
			            hfp->at_cmd_resp_pending = hfpBtrhOneCmdPending;
                        return 1;
        		    case 2:
            			hfp->at_cmd_resp_pending = hfpBtrhTwoCmdPending;
                        return 1;
                    default:
                        break;
    		    }
		    }
        }
      }
      break;
	case 38732:
      {
        const char *literal = "CIND";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCmdPending;;
	        return 1;
        }
      }
      break;
    case 54161:
      {
        const char *literal = "COPS";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
            /* Make assumption that cmd is formatted exactly as:  */
            /*    "AT+COPS?"  or                                  */
            /*    "AT+COPS=3,0"                                   */
            if ( *(s+4)=='?' )
            {	/* Sending information request command */
    			hfp->at_cmd_resp_pending = hfpCopsReqCmdPending;
	        }
	        else
	        {	/* Assume reporting format command being sent */
     			hfp->at_cmd_resp_pending = hfpCopsFormatCmdPending;
 			}
            return 1;
        }
      }
      break;
    case 54982:
      {
        const char *literal = "CMEE";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCmeeCmdPending;
	        return 1;
        }
      }
      break;
    case 56960:
      {
        const char *literal = "CLIP";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpClipCmdPending;
            return 1;
        }
      }
      break;
    case 57233:
      {
        const char *literal = "CMER";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCmerCmdPending;
	        return 1;
        }
      }
      break;
    case 57792:
      {
        const char *literal = "VTS=";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpVtsCmdPending;
	        return 1;
        }
      }
      break;
    case 49871:
      {
        const char *literal = "CSRS";  /* 'F=' not part of hash */
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCsrSfPending;
	        return 1;
        }
      }
      break;
    case 59434:
      {
        const char *literal = "CSRP";  /* 'WR=' not part of hash */
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCsrPwrPending;
	        return 1;
        }
      }
      break;
    case 16680:
      {
        const char *literal = "CSRB";  /* 'ATT=' not part of hash */
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCsrBatPending;
	        return 1;
        }
      }
      break;
    case 10773:
      {
        const char *literal = "CSR=";
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCsrModIndPending;
	        return 1;
        }
      }
      break;
    case 22587:
      {
        const char *literal = "CSRG";  /* 'ETSMS=' not part of hash */
        if( UtilCompare((uint16 *)literal, (uint16 *)s, 4) == 0)
        {
			hfp->at_cmd_resp_pending = hfpCsrGetSmsPending;
	        return 1;
        }
      }
      break;

	default:
		break;
  }

  return 0;
}


/* 
    getBytesToFlush

    Returns the number of bytes to flush. Iterates to the end of the next AT+ cmd 
	and returns the number of bytes counted.
*/
static uint16 getBytesToFlush(HFP *hfp, const uint8 *data_ptr, const uint8 *endptr)
{
    uint16 ix = 0;	

    /* Return zero if the buffer is empty */
    if (data_ptr == endptr)
        return 0;
	
	/* All AT cms start with AT so assume those chars are there */
	switch (*(data_ptr+2))
	{
	case '+':
		{
			/* Command has format AT+...*/
			if (!matchCmd(hfp, (uint8 *) data_ptr+3))
			{
				/* We didn't match any of the AT cmds in the hash above. */
			    hfp->at_cmd_resp_pending = hfpCmdPending;
			}
			/* If matchCmd worked the flag is set */
		}
		break;
		
	case 'D':
		{
			/* Must have an ATD cmd */
			if (*(data_ptr+3) == '>')
				/* Must be ATD> */
				hfp->at_cmd_resp_pending = hfpAtdMemoryCmdPending;
			else
				/* Cmd is just ATD */
				hfp->at_cmd_resp_pending = hfpAtdNumberCmdPending;
		}
		break;
		
	case 'A':			
		/* This must be an ATA cmd */
		hfp->at_cmd_resp_pending = hfpAtaCmdPending;
		break;
		
	default:
		{
			/* 
				Failed to match any of the AT cmds we know about. If not waiting 
				for a given AT response set flag to indicate we're just waiting 
				for an OK.
			*/
			hfp->at_cmd_resp_pending = hfpCmdPending;
		}
	}

    /* Find the next AT+ cmd*/
    while ((data_ptr+ix) != endptr)
    {
        if (hfpMatchChar(data_ptr+ix, endptr, 'A') && hfpMatchChar(data_ptr+ix+1, endptr, 'T'))
        {
            ix+=2;
            
            /* Found the start of an AT cmd, now find the end */
            while(((data_ptr+ix) != endptr) && (*(data_ptr+ix) != '\r'))
                ix++;

            return (ix+1);
        }

        /* Increment the index and carry on looking */
        ix++;
    }

    return (ix+1);
}


/****************************************************************************
NAME	
	hfpSendAtCmd

DESCRIPTION
	Send an AT command by putting it into the RFCOMM buffer.

RETURNS
	void
*/
void hfpSendAtCmd(Task profileTask, uint16 length, const char *at_cmd)
{
	uint16 sink_slack;
	HFP *hfp = (HFP *) profileTask;
    
	if ( SinkIsValid(hfp->sink) )
	{
    	/* Get the amount of available space left in the sink */
    	sink_slack = SinkSlack(hfp->sink);
    
    	/* Make sure we have enough space for this AT cmd */
        if (sink_slack >= length)
        {
            uint16 sink_offset = SinkClaim(hfp->sink, length);
            uint8 *data_out = SinkMap(hfp->sink);
    
    		/* Check we have a valid offset */
    		if (sink_offset == 0xffff)
            {
    			/* The sink offset returned is invalid */
				HFP_DEBUG(("Invalid sink offset\n"));
    		}
    
    		/* Check we've got a valid data ptr */
    		if (data_out)
            {
    			/* Copy the data into the buffer */
                memmove(data_out+sink_offset, at_cmd, length);
    
    			/* Try to send the next AT cmd in the buffer if possible */
    			hfpSendNextAtCmd(hfp, SinkClaim(hfp->sink, 0), data_out);
    		}
    		else
    		{
    			/* SinkMap returned an invalid data ptr. */
    			HFP_DEBUG(("Invalid sink ptr\n"));
    		}
    	}
    	else
        {
            /* Not enough space in the rfcomm buffer. */
    		HFP_DEBUG(("Insufficient space in sink\n"));
        }
    }
}


/****************************************************************************
NAME	
	hfpSendNextAtCmd

DESCRIPTION
	Attempt to send the next AT cmd (if any) pending in the RFCOMM buffer.

RETURNS
	void
*/
void hfpSendNextAtCmd(HFP *hfp, uint16 offset, const uint8 *data_out)
{
	if ( SinkIsValid(hfp->sink) )
	{
        /* Check we have a valid offset */
    	if (offset == 0xffff)
        {
    		/* The sink offset returned is invalid */
    		HFP_DEBUG(("Invalid sink offset\n"));
    	}
    
        /* Check we have something to send */
        if (hfp->at_cmd_resp_pending == hfpNoCmdPending && data_out)
        {
            /* Make sure we have an AT cmd to flush */
    		uint16 bytes = getBytesToFlush(hfp, data_out, data_out+offset);
    
    #ifdef BE_EVIL
            uint16 ix = 0;
            for (ix=0; ix < bytes; ix++)
            {
                (void) SinkFlush(hfp->sink, 1);
            }
    #else
    		if (bytes)
    		{
    			/* Flush them. */
    			uint16 flush_result = SinkFlush(hfp->sink, bytes);
    
    			/* Flush failed */
    			if (!flush_result)
    				HFP_DEBUG(("Sink flush failed\n")); 
    		} /*lint !e548 */
    #endif
    
    		/* If we've just sent an AT cmd reset the timeout */
    		if (hfp->at_cmd_resp_pending != hfpNoCmdPending)
    		{
    			/* Cancel any existing timeout messages */
    			(void) MessageCancelAll(&hfp->task, HFP_INTERNAL_WAIT_AT_TIMEOUT_IND);
    
    			/* Set a timeout - AT cmd has just been sent */
    			MessageSendLater(&hfp->task, HFP_INTERNAL_WAIT_AT_TIMEOUT_IND, 0, (uint32) AT_RESPONSE_TIMEOUT);
    		}
        }
    }
}


/****************************************************************************
NAME	
	hfpHandleWaitAtTimeout

DESCRIPTION
	Waiting for OK/ ERROR response to AT cmd timeout has expired. This means 
	that we have not received a response for the last AT cmd. So we don't
	get completely stuck, send out the next cmd anyway.

RETURNS
	void
*/
void hfpHandleWaitAtTimeout(HFP *hfp)
{
 	/* Reset the flag indicating we're waiting for a response */
	hfp->at_cmd_resp_pending = hfpNoCmdPending;

	/* Attempt to send the next AT cmd in the buffer */
	hfpSendNextAtCmd(hfp, SinkClaim(hfp->sink, 0), SinkMap(hfp->sink));
}

/*lint +e525 +e830 */
