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

/* lwip library */
#include "lwiplib.h"
#include "console.h"
/* http client header */
#include "ff.h"
#include "httpc.h"
#include "Rtc.h"
#include "lcd_terminal.h"
#include "telnet.h"


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

int mountSd(void)
{
    FRESULT fresult;
    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
        printf("f_mount error: %d\n", fresult);
    return fresult;
}

void print_http(unsigned long size,char *content,void *pv)
{
    FILE *file = (FILE *)pv;
	unsigned long i;
	for(i=0;i<size;i++)
		fputc(content[i],file);
}

void file_http(unsigned long size,char *content,void *pv)
{
    FRESULT fresult;
    FIL *pFileObject = (FIL *)pv;
    unsigned int usBytesWritten;

    fresult = f_write(pFileObject, content, size,
		&usBytesWritten);
}


//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current
// directory and enumerates through the contents, and prints a line for
// each item it finds.  It shows details such as file attributes, time and
// date, and the file size, along with the name.  It shows a summary of
// file sizes at the end along with free space.
//
//*****************************************************************************
static int Cmd_ls(FILE *file,char *path)
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
		//fprintf(file,"current directory\n");
    }
    else if(path[0] == '/')
    {
        //
        // Make sure the new path is not bigger than the cwd buffer.
        //
        if(strlen(path) + 1 > sizeof(g_cCwdBuf))
        {
            fprintf(file,"Resulting path name is too long\n");
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
            fprintf(file,"Resulting path name is too long\n");
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
        fprintf(file,"open error\n");
        return(fresult);
    }

    ulTotalSize = 0;
    ulFileCount = 0;
    ulDirCount = 0;

    //
    // Give an extra blank line before the listing.
    //
    fprintf(file,"\n");

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
        fprintf(file,"%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
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
    fprintf(file,"\n%4u File(s),%10u bytes total\n%4u Dir(s)",
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
    fprintf(file,", %10uK bytes free\n", ulTotalSize * pFatFs->csize / 2);

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
static int Cmd_cd(FILE *file,char *path)
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
            fprintf(file,"Resulting path name is too long\n");
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
            fprintf(file,"Resulting path name is too long\n");
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
        fprintf(file,"cd: %s\n", g_cTmpBuf);
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

static int Cmd_mkdir(FILE *file,char *path)
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
            fprintf(file,"Resulting path name is too long\n");
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
            fprintf(file,"Resulting path name is too long\n");
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
        fprintf(file,"mkdir: %s fail\n", g_cTmpBuf);
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
static int Cmd_cat(FILE *file,char *filename)
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
    if(strlen(g_cCwdBuf) + strlen(filename) + 1 + 1 > sizeof(g_cTmpBuf))
    {
        fprintf(file,"Resulting path name is too long\n");
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
    strcat(g_cTmpBuf, filename);

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
            fprintf(file,"\n");
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
        fprintf(file,"%s", g_cTmpBuf);

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
static int Cmd_log(FILE *file,char *argv)
{
    FRESULT fresult;
    unsigned int usBytesWritten;
	char *filename,*string;
	char ts_string[22];
    time_t tm = RtcGetTime();
    struct tm * timeinfo;

    timeinfo = localtime(&tm);

	sprintf(ts_string,"%04d/%02d/%02d %02d:%02d:%02d ",timeinfo->tm_year + 1900, timeinfo->tm_mon+ 1 , timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	

	filename = strtok(argv," \t\r\n");
	if(filename == NULL){
		fprintf(file,"filename not given\n");
		return 0;
	}
	string = strtok(NULL,"\r");
	if(string== NULL){
		fprintf(file,"string not given\n");
		return 0 ;
	}
	
    strcpy(g_cTmpBuf, g_cCwdBuf);
    if(filename[0] == '/')
        strncpy(g_cTmpBuf, filename, sizeof(g_cTmpBuf));
    else
    {
        if(strlen(g_cCwdBuf) + strlen(filename) + 1 + 1 > sizeof(g_cTmpBuf))
        {
            fprintf(file,"Resulting path name is too long\n");
            return(0);
        }

        if(strcmp(g_cTmpBuf, "/"))
        {
            strcat(g_cTmpBuf, "/");
        }
        strcat(g_cTmpBuf, filename);
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

static int Cmd_lcd(FILE *file,char *argv)
{
    if(argv && *argv)
        fprintf(stderr,"%s\n",argv);
    return(0);
}

static unsigned long checkFreeMem(void)
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


static int Cmd_free(FILE *file,char *argv)
{
#ifdef MEM_USE_TRACE
	char free_mem[80];
	show_free(free_mem);
	fprintf(file,free_mem);	
#else
	unsigned long free = checkFreeMem();
	fprintf(file,"Free mem = %d\n",free);
#endif
    return 0;
}

static int Cmd_date(FILE *file,char *argv)
{
	time_t timer;
	timer=RtcGetTime();
	fprintf(file,"%s",asctime(localtime(&timer)));
	return 0;
}

static int Cmd_wget(FILE *file,char *argv)
{
    char *url,*filename;
    FRESULT fresult;
    unsigned long tick_before,tick_after;

    url = strtok(argv," \t\r\n");
    if(url == NULL)
        return 0;

    fprintf(file,"url = %s\n",url);

    filename = strtok(NULL," \r\n");
    if(filename == NULL)
    	http_req(url,print_http,(void*)file);
    else{
        fprintf(file,"file = %s\n",filename);
        
        strcpy(g_cTmpBuf, g_cCwdBuf);
        if(filename[0] == '/')
            strncpy(g_cTmpBuf, filename, sizeof(g_cTmpBuf));
        else
        {
            if(strcmp(g_cTmpBuf, "/"))
            {
                strcat(g_cTmpBuf, "/");
            }
            strcat(g_cTmpBuf, filename);
        }

    	fresult = f_open(&g_sFileObject, g_cTmpBuf, FA_WRITE | FA_CREATE_ALWAYS);
    	if(fresult != FR_OK){
    	    fprintf(file,"file open error\n");
    	    return 0;
    	}

        tick_before = xTaskGetTickCount();
    	http_req(url,file_http,(void*)&g_sFileObject);
    	f_close(&g_sFileObject);
        tick_after = xTaskGetTickCount();
        
    	fprintf(file,"lap %ld ticks (%ld sec)\n",tick_after - tick_before, (tick_after - tick_before) / configTICK_RATE_HZ);
    }

    return 0;
}

static int Cmd_task(FILE *file,char*argv)
{
    char *task_status = mem_malloc(200);

	if(task_status == NULL){
		fprintf(file,"task_status malloc fail\n");
		return 0;
	}
	vTaskList(task_status);
	fprintf(file,"name\t\tstatus\tpri\tstack\ttcb");
	fprintf(file,task_status);
	mem_free(task_status);
	return 0;
}

static int Cmd_top(FILE *file,char*argv)
{
#if configGENERATE_RUN_TIME_STATS
    char *top_string = mem_malloc(200);

	if(top_string == NULL){
		fprintf(file,"top_string malloc fail\n");
		return 0;
	}

    vTaskGetRunTimeStats(top_string);
	fprintf(file,top_string);
	mem_free(top_string);
#endif    
    return 0;
}

static int Cmd_reboot(FILE *file,char *argv)
{
    return 0;
}

static int Cmd_ifconfig(FILE *file,char *argv)
{
    unsigned char pucMACArray[6];
    unsigned long addr;
    
    fprintf(file,"Link is %s\n",lwIPLinkStatusGet() ? "UP":"DOWN");
    lwIPLocalMACGet(pucMACArray);
    fprintf(file,"MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n",pucMACArray[0],pucMACArray[1],
            pucMACArray[2],pucMACArray[3],pucMACArray[4],pucMACArray[5]);
    addr = lwIPLocalIPAddrGet();
    fprintf(file,"IP address = %s\n",inet_ntoa(addr));
    addr = lwIPLocalNetMaskGet();
    fprintf(file,"IP netmask = %s\n",inet_ntoa(addr));
    addr = lwIPLocalGWAddrGet();
    fprintf(file,"IP gateway = %s\n",inet_ntoa(addr));

    return 0;
}

static int Cmd_help(FILE *file,char *argv);

typedef int (*cmd_func)(FILE *file,char *argv);

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
	
static int Cmd_help(FILE *file,char *argv)
{
	int i;
    for(i=0;i<sizeof(CMD_TABLE)/sizeof(command_table);i++){
        fprintf(file,"%s\t: %s\n",CMD_TABLE[i].cmd,CMD_TABLE[i].desc);
    }   
	return 0;
}

static void parse_cmd(FILE *file,char *cmd)
{
	int i;
	char *ptr;
	fprintf(file,"\n");
	if(cmd && *cmd){
    	ptr = strtok(cmd," \t\n\r");
		if(ptr){
	    	for(i=0;i<sizeof(CMD_TABLE)/sizeof(command_table);i++){
	    		if(!strcmp(CMD_TABLE[i].cmd,ptr)){
	    			CMD_TABLE[i].cb(file,strtok(NULL,"\r"));
	    			break;
	    		}
	    	}	
            if(i ==sizeof(CMD_TABLE)/sizeof(command_table))
                fprintf(file,"Unknown command\n");
		}
    }
	fprintf(file,"%s>",g_cCwdBuf);
}

line_buffer *console_buffer_get(int length)
{
    line_buffer *lb;

    lb = (line_buffer *) mem_malloc(sizeof(line_buffer) + length);
    if(lb){
        memset(lb,0,sizeof(line_buffer) + length);
        lb->length = length;
    }
    return lb;
}

void console_parse(line_buffer *lb, char ch)
{
    if(ch == '\0')
        return;

    fputc(ch,lb->file);
        
    if(ch == '\r'){
        (lb->buffer)[lb->index++] = ch;
        (lb->buffer)[lb->index] = '\0';
        lb->index = 0;
        parse_cmd(lb->file,lb->buffer);
    }else if(ch == '\b'){
        if(lb->index > 0)
            lb->index--;
    }else if(lb->index < lb->length){
        (lb->buffer)[lb->index++] = ch;
    }else{
        fprintf(lb->file,"cmd buffer overflow\n");
    }
}

