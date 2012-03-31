/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "Timers.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "uart.h"
#include "lmi_flash.h"

/* Demo app includes. */
#include "partest.h"

/* lwip library */
#include "lwiplib.h"

/* http client header */
#include "ff.h"
#include "httpc.h"
#include "Rtc.h"
#include "lcd_terminal.h"

// periodic timer
static xTimerHandle xPeriodicTimer = NULL;
static xSemaphoreHandle xSemaphoreTimer = NULL;

static void StartNetwork(void)
{
    /* Create the lwIP task if running on a processor that includes a MAC and
    PHY. */
    if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) )
    {
        unsigned long ulUser0, ulUser1;
        unsigned char pucMACArray[8];
    
        //
        // Enable and Reset the Ethernet Controller.
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
        SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);
    
        //
        // Enable Port F for Ethernet LEDs.
        //  LED0        Bit 3   Output
        //  LED1        Bit 2   Output
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    
        //
        // Configure the hardware MAC address for Ethernet Controller
        // filtering of incoming packets.
        //
        // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
        // non-volatile USER0 and USER1 registers.  These registers can be read
        // using the FlashUserGet function, as illustrated below.
        //
        FlashUserGet(&ulUser0, &ulUser1);
    
        //
        // Convert the 24/24 split MAC address from NV ram into a 32/16 split
        // MAC address needed to program the hardware registers, then program
        // the MAC address into the Ethernet Controller registers.
        //
        pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
        pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
        pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
        pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
        pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
        pucMACArray[5] = ((ulUser1 >> 16) & 0xff);
    
        //
        // Initialze the lwIP library, using DHCP.
        //
        lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
    }
}

unsigned long checkFreeMem(void)
{
    unsigned long memsize = 0;
    char *ptr = NULL;

	vTaskSuspendAll();
    do{
        memsize+=1;
        ptr = malloc(memsize);
        if(ptr)
            free(ptr);
    }while(ptr);
	xTaskResumeAll();
    
    return memsize;
}


#if configUSE_TIMERS
static void TimerCallback( xTimerHandle pxExpiredTimer )
{
	xSemaphoreGive(xSemaphoreTimer);
}
static void http_test(void)
{
	time_t timer;
	static int count;
	unsigned long tick;

//	  printf("[%d]http ^9%d`\n",count++,http_get("192.168.100.20",80,"/ap.html"));
	tick = xTaskGetTickCount();
	printf(">>");
	printf("[%d]naver ^9%d ^f%ld`\n",count,http_get("www.naver.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
	tick = xTaskGetTickCount();
//	  printf("[%d]google ^9%d ^f%ld`\n",count++,http_get("www.google.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
	printf("freemem = %ld\n",checkFreeMem());
	printf("stack = ^b%d`\n",uxTaskGetStackHighWaterMark(NULL));
	timer=RtcGetTime();
	printf("^f%s`",asctime(localtime(&timer)) + 11);

}
#else
static void httpTimerCallback(void *pv)
{
    time_t timer;
    static int count;
    printf("[%d]naver ^9%d\n",count,http_get("www.naver.com",80,"/",NULL,NULL));
//    printf("[%d]google ^9%d\n",count++,http_get("www.google.com",80,"/",NULL,NULL));
    printf("freemem = %ld\n",checkFreeMem());
    timer=RtcGetTime();
    printf("^f%s`",asctime(localtime(&timer)) + 11);

    sys_timeout(2000, httpTimerCallback, NULL);
}
#endif

int report_measure(){
    char *rpt;
    char node_string[100];
    time_t timer;
    int i, ret;
    static int count, success;
    

	struct {
		unsigned char addr;
		int temp;
		int humidity;
		int co2;
		int retry;
		int sound;
	}sensor[10] = {
		{0xA0, 19 * 100, 41 * 100, 3600, 0, 0},
		{0xA1, 20 * 100, 42 * 100, 3600, 0, 0},
		{0xA2, 21 * 100, 43 * 100, 3600, 0, 0},
		{0xA3, 22 * 100, 44 * 100, 3600, 0, 0},
		{0xA4, 23 * 100, 45 * 100, 3600, 0, 0},
		{0xA5, 24 * 100, 46 * 100, 3600, 0, 0},
		{0xA6, 25 * 100, 47 * 100, 3600, 0, 0},
		{0xA7, 26 * 100, 48 * 100, 3600, 0, 0},
		{0xA8, 27 * 100, 49 * 100, 3600, 0, 0},
		{0xA9, 28 * 100, 50 * 100, 3600, 0, 0}
	};
			
	
	
    rpt = mem_malloc(1024);
    sprintf(rpt,"/sensor/logging?id=%lu",RtcGetTime());
    for(i=0;i<10;i++){
        sprintf(node_string,"||00000000000000%02x|%d.%02d,%d.%02d,%d,%d,%d", sensor[i].addr,
				sensor[i].temp/100,sensor[i].temp%100,
				sensor[i].humidity/100,sensor[i].humidity%100,
				sensor[i].co2,sensor[i].retry,sensor[i].sound);
        strcat(rpt,node_string);
    }
    count++;
    if( (ret = http_get("pentascan.dyndns.org",2222,rpt,NULL,NULL)) == 200)
        success++;
    mem_free(rpt);

    fprintf(stderr,"^a<%d/%d>`result ^f[%d]`\n",success,count,ret);
    timer=RtcGetTime();
    fprintf(stderr,"^f%s`",asctime(localtime(&timer)) + 11);
    return 0;
}


//*****************************************************************************
//
// Defines the size of the buffers that hold the path, or temporary
// data from the SD card.  There are two buffers allocated of this size.
// The buffer size must be large enough to hold the longest expected
// full path name, including the file name, and a trailing null character.
//
//*****************************************************************************
#define PATH_BUF_SIZE   80

//*****************************************************************************
//
// Defines the size of the buffer that holds the command line.
//
//*****************************************************************************
#define CMD_BUF_SIZE    64

//*****************************************************************************
//
// This buffer holds the full path to the current working directory.
// Initially it is root ("/").
//
//*****************************************************************************
static char g_cCwdBuf[PATH_BUF_SIZE] = "/";

//*****************************************************************************
//
// A temporary data buffer used when manipulating file paths, or reading data
// from the SD card.
//
//*****************************************************************************
static char g_cTmpBuf[PATH_BUF_SIZE];

//*****************************************************************************
//
// The buffer that holds the command line.
//
//*****************************************************************************
static char g_cCmdBuf[CMD_BUF_SIZE];

//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;

//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current
// directory and enumerates through the contents, and prints a line for
// each item it finds.  It shows details such as file attributes, time and
// date, and the file size, along with the name.  It shows a summary of
// file sizes at the end along with free space.
//
//*****************************************************************************
int Cmd_ls(char *path)
{
    unsigned long ulTotalSize;
    unsigned long ulFileCount;
    unsigned long ulDirCount;
    unsigned int uIdx;
    FRESULT fresult;
    FATFS *pFatFs;



    //
    // Copy the current working path into a temporary buffer so
    // it can be manipulated.
    //
    strcpy(g_cTmpBuf, g_cCwdBuf);

    //
    // If the first character is /, then this is a fully specified
    // path, and it should just be used as-is.
    //
    if(path == 0){
		//printf("current directory\n");
    }
    else if(path[0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(path) + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then
        // copy it into the temporary buffer so it can be checked.
        //
        else
        {
            strncpy(g_cTmpBuf, path, sizeof(g_cTmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level
    // on the CWD.
    //
    else if(!strcmp(path, ".."))
    {
        //
        // Get the index to the last character in the current path.
        //
        uIdx = strlen(g_cTmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/)
        // is found, or until we bump up to the start of the path.
        //
        while((g_cTmpBuf[uIdx] != '/') && (uIdx > 1))
        {
            //
            // Back up one character.
            //
            uIdx--;
        }

        //
        // Now we are either at the lowest level separator in the
        // current path, or at the beginning of the string (root).
        // So set the new end of string here, effectively removing
        // that last part of the path.
        //
        g_cTmpBuf[uIdx] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current
    // directory, and it needs to be appended to the current path.
    //
    else
    {
        //
        // Test to make sure that when the new additional path is
        // added on to the current path, there is room in the buffer
        // for the full new path.  It needs to include a new separator,
        // and a trailing null character.
        //
        if(strlen(g_cTmpBuf) + strlen(path) + 1 + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // The new path is okay, so add the separator and then append
        // the new directory to the path.
        //
        else
        {
            //
            // If not already at the root level, then append a /
            //
            if(strcmp(g_cTmpBuf, "/"))
            {
                strcat(g_cTmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(g_cTmpBuf, path);
        }
    }    

	//
    // Open the current directory for access.
    //
    fresult = f_opendir(&g_sDirObject, g_cTmpBuf);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        printf("open error\n");
        return(fresult);
    }

    ulTotalSize = 0;
    ulFileCount = 0;
    ulDirCount = 0;

    //
    // Give an extra blank line before the listing.
    //
    printf("\n");

    //
    // Enter loop to enumerate through all directory entries.
    //
    for(;;)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        //
        // If the attribue is directory, then increment the directory count.
        //
        if(g_sFileInfo.fattrib & AM_DIR)
        {
            ulDirCount++;
        }

        //
        // Otherwise, it is a file.  Increment the file count, and
        // add in the file size to the total.
        //
        else
        {
            ulFileCount++;
            ulTotalSize += g_sFileInfo.fsize;
        }

        //
        // Print the entry information on a single line with formatting
        // to show the attributes, date, time, size, and name.
        //
        printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                    (g_sFileInfo.fattrib & AM_DIR) ? 'D' : '-',
                    (g_sFileInfo.fattrib & AM_RDO) ? 'R' : '-',
                    (g_sFileInfo.fattrib & AM_HID) ? 'H' : '-',
                    (g_sFileInfo.fattrib & AM_SYS) ? 'S' : '-',
                    (g_sFileInfo.fattrib & AM_ARC) ? 'A' : '-',
                    (g_sFileInfo.fdate >> 9) + 1980,
                    (g_sFileInfo.fdate >> 5) & 15,
                     g_sFileInfo.fdate & 31,
                    (g_sFileInfo.ftime >> 11),
                    (g_sFileInfo.ftime >> 5) & 63,
                     g_sFileInfo.fsize,
                     g_sFileInfo.fname);
    }   // endfor

    //
    // Print summary lines showing the file, dir, and size totals.
    //
    printf("\n%4u File(s),%10u bytes total\n%4u Dir(s)",
                ulFileCount, ulTotalSize, ulDirCount);

    //
    // Get the free space.
    //
    fresult = f_getfree("/", &ulTotalSize, &pFatFs);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    //
    // Display the amount of free space that was calculated.
    //
    printf(", %10uK bytes free\n", ulTotalSize * pFatFs->csize / 2);

    //
    // Made it to here, return with no errors.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "cd" command.  It takes an argument
// that specifes the directory to make the current working directory.
// Path separators must use a forward slash "/".  The argument to cd
// can be one of the following:
// * root ("/")
// * a fully specified path ("/my/path/to/mydir")
// * a single directory name that is in the current directory ("mydir")
// * parent directory ("..")
//
// It does not understand relative paths, so dont try something like this:
// ("../my/new/path")
//
// Once the new directory is specified, it attempts to open the directory
// to make sure it exists.  If the new path is opened successfully, then
// the current working directory (cwd) is changed to the new path.
//
//*****************************************************************************
int
Cmd_cd(char *path)
{
    unsigned int uIdx;
    FRESULT fresult;

    //
    // Copy the current working path into a temporary buffer so
    // it can be manipulated.
    //
    strcpy(g_cTmpBuf, g_cCwdBuf);

    //
    // If the first character is /, then this is a fully specified
    // path, and it should just be used as-is.
    //
    if(path[0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(path) + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then
        // copy it into the temporary buffer so it can be checked.
        //
        else
        {
            strncpy(g_cTmpBuf, path, sizeof(g_cTmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level
    // on the CWD.
    //
    else if(!strcmp(path, ".."))
    {
        //
        // Get the index to the last character in the current path.
        //
        uIdx = strlen(g_cTmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/)
        // is found, or until we bump up to the start of the path.
        //
        while((g_cTmpBuf[uIdx] != '/') && (uIdx > 1))
        {
            //
            // Back up one character.
            //
            uIdx--;
        }

        //
        // Now we are either at the lowest level separator in the
        // current path, or at the beginning of the string (root).
        // So set the new end of string here, effectively removing
        // that last part of the path.
        //
        g_cTmpBuf[uIdx] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current
    // directory, and it needs to be appended to the current path.
    //
    else
    {
        //
        // Test to make sure that when the new additional path is
        // added on to the current path, there is room in the buffer
        // for the full new path.  It needs to include a new separator,
        // and a trailing null character.
        //
        if(strlen(g_cTmpBuf) + strlen(path) + 1 + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // The new path is okay, so add the separator and then append
        // the new directory to the path.
        //
        else
        {
            //
            // If not already at the root level, then append a /
            //
            if(strcmp(g_cTmpBuf, "/"))
            {
                strcat(g_cTmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(g_cTmpBuf, path);
        }
    }

    //
    // At this point, a candidate new directory path is in chTmpBuf.
    // Try to open it to make sure it is valid.
    //
    fresult = f_opendir(&g_sDirObject, g_cTmpBuf);

    //
    // If it cant be opened, then it is a bad path.  Inform
    // user and return.
    //
    if(fresult != FR_OK)
    {
        printf("cd: %s\n", g_cTmpBuf);
        return(fresult);
    }

    //
    // Otherwise, it is a valid new path, so copy it into the CWD.
    //
    else
    {
        strncpy(g_cCwdBuf, g_cTmpBuf, sizeof(g_cCwdBuf));
    }

    //
    // Return success.
    //
    return(0);
}

int
Cmd_mkdir(char *path)
{
    unsigned int uIdx;
    FRESULT fresult;

    //
    // Copy the current working path into a temporary buffer so
    // it can be manipulated.
    //
    strcpy(g_cTmpBuf, g_cCwdBuf);

    //
    // If the first character is /, then this is a fully specified
    // path, and it should just be used as-is.
    //
    if(path[0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(path) + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // If the new path name (in argv[1])  is not too long, then
        // copy it into the temporary buffer so it can be checked.
        //
        else
        {
            strncpy(g_cTmpBuf, path, sizeof(g_cTmpBuf));
        }
    }

    //
    // If the argument is .. then attempt to remove the lowest level
    // on the CWD.
    //
    else if(!strcmp(path, ".."))
    {
        //
        // Get the index to the last character in the current path.
        //
        uIdx = strlen(g_cTmpBuf) - 1;

        //
        // Back up from the end of the path name until a separator (/)
        // is found, or until we bump up to the start of the path.
        //
        while((g_cTmpBuf[uIdx] != '/') && (uIdx > 1))
        {
            //
            // Back up one character.
            //
            uIdx--;
        }

        //
        // Now we are either at the lowest level separator in the
        // current path, or at the beginning of the string (root).
        // So set the new end of string here, effectively removing
        // that last part of the path.
        //
        g_cTmpBuf[uIdx] = 0;
    }

    //
    // Otherwise this is just a normal path name from the current
    // directory, and it needs to be appended to the current path.
    //
    else
    {
        //
        // Test to make sure that when the new additional path is
        // added on to the current path, there is room in the buffer
        // for the full new path.  It needs to include a new separator,
        // and a trailing null character.
        //
        if(strlen(g_cTmpBuf) + strlen(path) + 1 + 1 > sizeof(g_cCwdBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        //
        // The new path is okay, so add the separator and then append
        // the new directory to the path.
        //
        else
        {
            //
            // If not already at the root level, then append a /
            //
            if(strcmp(g_cTmpBuf, "/"))
            {
                strcat(g_cTmpBuf, "/");
            }

            //
            // Append the new directory to the path.
            //
            strcat(g_cTmpBuf, path);
        }
    }

    //
    // At this point, a candidate new directory path is in chTmpBuf.
    // Try to open it to make sure it is valid.
    //
    fresult = f_mkdir(g_cTmpBuf);

    //
    // If it cant be opened, then it is a bad path.  Inform
    // user and return.
    //
    if(fresult != FR_OK)
    {
        printf("mkdir: %s fail\n", g_cTmpBuf);
        return(fresult);
    }

    //
    // Return success.
    //
    return(0);
}


//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of
// a file and prints it to the console.  This should only be used on
// text files.  If it is used on a binary file, then a bunch of garbage
// is likely to printed on the console.
//
//*****************************************************************************
int
Cmd_cat(char *file)
{
    FRESULT fresult;
    unsigned int usBytesRead;

    //
    // First, check to make sure that the current path (CWD), plus
    // the file name, plus a separator and trailing null, will all
    // fit in the temporary buffer that will be used to hold the
    // file name.  The file name must be fully specified, with path,
    // to FatFs.
    //
    if(strlen(g_cCwdBuf) + strlen(file) + 1 + 1 > sizeof(g_cTmpBuf))
    {
        printf("Resulting path name is too long\n");
        return(0);
    }

    //
    // Copy the current path to the temporary buffer so it can be manipulated.
    //
    strcpy(g_cTmpBuf, g_cCwdBuf);

    //
    // If not already at the root level, then append a separator.
    //
    if(strcmp("/", g_cCwdBuf))
    {
        strcat(g_cTmpBuf, "/");
    }

    //
    // Now finally, append the file name to result in a fully specified file.
    //
    strcat(g_cTmpBuf, file);

    //
    // Open the file for reading.
    //
    fresult = f_open(&g_sFileObject, g_cTmpBuf, FA_READ);

    //
    // If there was some problem opening the file, then return
    // an error.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    //
    // Enter a loop to repeatedly read data from the file and display it,
    // until the end of the file is reached.
    //
    do
    {
        //
        // Read a block of data from the file.  Read as much as can fit
        // in the temporary buffer, including a space for the trailing null.
        //
        fresult = f_read(&g_sFileObject, g_cTmpBuf, sizeof(g_cTmpBuf) - 1,
                         &usBytesRead);

        //
        // If there was an error reading, then print a newline and
        // return the error to the user.
        //
        if(fresult != FR_OK)
        {
            printf("\n");
            f_close(&g_sFileObject);
            return(fresult);
        }

        //
        // Null terminate the last block that was read to make it a
        // null terminated string that can be used with printf.
        //
        g_cTmpBuf[usBytesRead] = 0;

        //
        // Print the last chunk of the file that was received.
        //
        printf("%s", g_cTmpBuf);

    //
    // Continue reading until less than the full number of bytes are
    // read.  That means the end of the buffer was reached.
    //
    }
    while(usBytesRead == sizeof(g_cTmpBuf) - 1);

    f_close(&g_sFileObject);
    //
    // Return success.
    //
    return(0);
}

//*****************************************************************************
//
// This function implements the "cat" command.  It reads the contents of
// a file and prints it to the console.  This should only be used on
// text files.  If it is used on a binary file, then a bunch of garbage
// is likely to printed on the console.
//
//*****************************************************************************
int
Cmd_log(char *argv)
{
    FRESULT fresult;
    unsigned int usBytesWritten;
	char *file,*string;
	char ts_string[22];
	unsigned long timestamp = get_fattime();
    time_t tm = RtcGetTime();
    struct tm * timeinfo;

    timeinfo = localtime(&tm);

	sprintf(ts_string,"%04d/%02d/%02d %02d:%02d:%02d ",timeinfo->tm_year + 1900, timeinfo->tm_mon+ 1 , timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	

	file = strtok(argv," \t\r\n");
	if(file== NULL){
		printf("filename not given\n");
		return 0;
	}
	string = strtok(NULL,"\r");
	if(string== NULL){
		printf("string not given\n");
		return 0 ;
	}
	
    strcpy(g_cTmpBuf, g_cCwdBuf);
    if(file[0] == '/')
        strncpy(g_cTmpBuf, file, sizeof(g_cTmpBuf));
    else
    {
        if(strlen(g_cCwdBuf) + strlen(file) + 1 + 1 > sizeof(g_cTmpBuf))
        {
            printf("Resulting path name is too long\n");
            return(0);
        }

        if(strcmp(g_cTmpBuf, "/"))
        {
            strcat(g_cTmpBuf, "/");
        }
        strcat(g_cTmpBuf, file);
    }

    //
    // Open the file for reading.
    //
    fresult = f_open(&g_sFileObject, g_cTmpBuf, FA_WRITE | FA_OPEN_ALWAYS);

    //
    // If there was some problem opening the file, then return
    // an error.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    // seek to end of file
    fresult = f_lseek(&g_sFileObject, f_size(&g_sFileObject));

    //
    // If there was some problem opening the file, then return
    // an error.
    //
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    fresult = f_write(&g_sFileObject, ts_string, strlen(ts_string),
                     &usBytesWritten);
    if(fresult != FR_OK || strlen(ts_string) != usBytesWritten)
    {
        f_close(&g_sFileObject);
        return(fresult);
    }
	

    fresult = f_write(&g_sFileObject, string, strlen(string),
                     &usBytesWritten);
    if(fresult != FR_OK || strlen(string) != usBytesWritten)
    {
        f_close(&g_sFileObject);
        return(fresult);
    }

    fresult = f_write(&g_sFileObject, "\n", 1,
                     &usBytesWritten);
    if(fresult != FR_OK || usBytesWritten != 1)
    {
        f_close(&g_sFileObject);
        return(fresult);
    }

    f_close(&g_sFileObject);
    //
    // Return success.
    //
    return(0);
}

int
Cmd_lcd(char *argv)
{
    if(argv && *argv)
        fprintf(stderr,"%s\n",argv);
    return(0);
}


int Cmd_free(char *argv)
{
#ifdef MEM_USE_TRACE
	char free_mem[80];
	show_free(free_mem);
	printf(free_mem);	
#else
	unsigned long free = checkFreeMem();
	printf("Free mem = %d\n",free);
#endif
    return 0;
}

int Cmd_date(char *argv)
{
	time_t timer;
	timer=RtcGetTime();
	printf("%s",asctime(localtime(&timer)));
	return 0;
}

static void print_http(unsigned long size,char *content,void *pv)
{
	unsigned long i;
	for(i=0;i<size;i++)
		putchar(content[i]);
}

static void file_http(unsigned long size,char *content,void *pv)
{
    FRESULT fresult;
    unsigned int usBytesWritten;

    fresult = f_write(&g_sFileObject, content, size,
		&usBytesWritten);
}


int Cmd_wget(char *argv)
{
    char *url,*file;
    FRESULT fresult;

    url = strtok(argv," \t\r\n");
    if(url == NULL)
        return 0;

    printf("url = %s\n",url);

    file = strtok(NULL," \r\n");
    if(file == NULL)
    	http_req(url,print_http,NULL);
    else{
        printf("file = %s\n",file);
        
        strcpy(g_cTmpBuf, g_cCwdBuf);
        if(file[0] == '/')
            strncpy(g_cTmpBuf, file, sizeof(g_cTmpBuf));
        else
        {
            if(strcmp(g_cTmpBuf, "/"))
            {
                strcat(g_cTmpBuf, "/");
            }
            strcat(g_cTmpBuf, file);
        }

    	fresult = f_open(&g_sFileObject, g_cTmpBuf, FA_WRITE | FA_CREATE_ALWAYS);
    	if(fresult != FR_OK){
    	    printf("file open error\n");
    	    return 0;
    	}
    	http_req(url,file_http,NULL);
    	f_close(&g_sFileObject);
    }

    return 0;
}

int Cmd_task(char*argv)
{
    char *task_status = mem_malloc(200);

	if(task_status == NULL){
		printf("task_status malloc fail\n");
		return 0;
	}
	vTaskList(task_status);
	printf("name\t\tstatus\tpri\tstack\ttcb");
	printf(task_status);
	mem_free(task_status);
	return 0;
}

int Cmd_top(char*argv)
{
#if configGENERATE_RUN_TIME_STATS
    char *top_string = mem_malloc(200);

	if(top_string == NULL){
		printf("top_string malloc fail\n");
		return 0;
	}

    vTaskGetRunTimeStats(top_string);
	printf(top_string);
	mem_free(top_string);
#endif    
    return 0;
}

int Cmd_reboot(char *argv)
{
    return 0;
}

int Cmd_ifconfig(char *argv)
{
    return 0;
}

int Cmd_help(char *argv);

typedef int (*cmd_func)(char *argv);

typedef struct {
	const char *cmd;
	const char *desc;
	cmd_func cb;
}command_table;

command_table CMD_TABLE[] = 
{
	"ls","list files",Cmd_ls,
	"cd","change directory",Cmd_cd,
	"mkdir","make directory",Cmd_mkdir,
	"cat","show file content",Cmd_cat,
	"free","show free memory",Cmd_free,
	"date","show current time",Cmd_date,
	"wget","get URL",Cmd_wget,
	"task","show task status",Cmd_task,
	"top","show cpu usage",Cmd_top,
	"log","write log",Cmd_log,
	"lcd","print message to lcd",Cmd_lcd,
	"reboot","reboot system",Cmd_reboot,
	"ifconfig","show network configuration",Cmd_ifconfig,
	"help","show this message",Cmd_help
};
	
int Cmd_help(char *argv)
{
	int i;
    for(i=0;i<sizeof(CMD_TABLE)/sizeof(command_table);i++){
        printf("%s\t: %s\n",CMD_TABLE[i].cmd,CMD_TABLE[i].desc);
    }   
	return 0;
}

void parse_cmd(char *cmd)
{
	int i;
	char *ptr;
	printf("\n");
	if(cmd && *cmd){
    	ptr = strtok(cmd," \t\n\r");
		if(ptr){
	    	for(i=0;i<sizeof(CMD_TABLE)/sizeof(command_table);i++){
	    		if(!strcmp(CMD_TABLE[i].cmd,ptr)){
	    			CMD_TABLE[i].cb(strtok(NULL,"\r"));
	    			break;
	    		}
	    	}	
            if(i ==sizeof(CMD_TABLE)/sizeof(command_table))
                printf("Unknown command\n");
		}
    }
	printf("%s>",g_cCwdBuf);
}

#define CMD_BUFFER_LEN	100

void vMainTask( void *pvParameters )
{
    int c;
	char *cmd_buffer = mem_malloc(CMD_BUFFER_LEN);
	int cmd_index = 0;
    FRESULT fresult;

	if(cmd_buffer == NULL){
		printf("cmd_buffer malloc fail\n");
		return;
	}

	// Semaphore cannot be used before a call to xSemaphoreCreateCounting().
	// The max value to which the semaphore can count should be 10, and the
	// initial value assigned to the count should be 0.
	xSemaphoreTimer = xSemaphoreCreateCounting( 10, 0 );

	if( xSemaphoreTimer == NULL )
	{
		printf("#### Fail to create semaphore\n");
		return;
	}

    StartNetwork();
    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
    {
        printf("f_mount error: %d\n", fresult);
        return;
    }
    

#if configUSE_TIMERS
	xPeriodicTimer = xTimerCreate(	( const signed char * ) "http timer",/* Text name to facilitate debugging.  The kernel does not use this itself. */
									( 60 * configTICK_RATE_HZ ),			/* The period for the timer. */
									pdTRUE,								/* Don't auto-reload - hence a one shot timer. */
									( void * ) 0,							/* The timer identifier.  In this case this is not used as the timer has its own callback. */
									TimerCallback );				/* The callback to be called when the timer expires. */
#endif									


    fprintf(stderr,"Pentascan AP\n");
    xTimerStart(xPeriodicTimer,0);
    
	for( ;; )
	{
		/* wait 10msec */
		if(xSemaphoreTake( xSemaphoreTimer, configTICK_RATE_HZ / 100 ) == pdTRUE){
			report_measure();
		}

		if((c = getchar()) != -1){
		    putchar(c);
			if(c == '\r'){
				cmd_buffer[cmd_index++] = (char)c;
				cmd_buffer[cmd_index] = '\0';
				cmd_index = 0;
				parse_cmd(cmd_buffer);
			}else if(c == '\b'){
			    if(cmd_index > 0)
			        cmd_index--;
			}else if(cmd_index < CMD_BUFFER_LEN - 1){
				cmd_buffer[cmd_index++] = c;
			}else{
				printf("cmd buffer overflow\n");
			}
		}
	}
}

