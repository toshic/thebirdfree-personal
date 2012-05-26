#include <stdio.h>
#include <test.h>
#include <panic.h>
#include <vm.h>
#include <string.h>
#include "WritePSKey.h"

void WritePsKey(uint16 pskey, uint16 value)
{
	WritePsKeys(pskey,&value,1);
}

void WritePsKeys(uint16 pskey, uint16 *value,uint16 len)
{
    PDU_ *test = PanicUnlessMalloc(sizeof(PDU_)+ len - 1 );
    
    test->type = 0x0002;
    test->pdulen = sizeof(PDU_) + len - 1;
    test->varid = 0x7003;
    test->status = 0;
    test->data = pskey;
    test->pad[0] = len;
    test->pad[1] = 0;
	memcpy(&test->pad[2],value,len);
    
    TestBccmd_(test,test->pdulen);
}


