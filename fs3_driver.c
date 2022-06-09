////////////////////////////////////////////////////////////////////////////////
//
//  File           : fs3_driver.c
//  Description    : This is the implementation of the standardized IO functions
//                   for used to access the FS3 storage system.
//
//   Author        : *** Devon Burke ***
//   Last Modified : *** 11/21/2021 ***
//

// Includes
#include <string.h>
#include "cmpsc311_log.h"
#include "fs3_controller.h"
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Project Includes
#include "fs3_driver.h"
#include "fs3_cache.h"
#include "fs3_network.h"

//
// Defines
#define SECTOR_INDEX_NUMBER(x) ((int)(x / FS3_SECTOR_SIZE))
typedef uint64_t FS3CmdBlk;

//
// Static Global Variables
int mountStatus = 0;
// Mount flag that is used to define whether it is mounted or not
FS3CmdBlk y = 0;

struct state
{
	int size;
	char name[128];
	int position;
	int handle;
	int FileIsOpen;
};
// struct that holds initialized variables associated to the file
struct state newFiles[1024];
// giving my struct an identifier to access objects inside struct

int64_t arr2[64][1024];
// Global 2D array

//
// Implementation

// Constructing the commandblock -Shifting values come from readme (Op is not 8, but 4)
FS3CmdBlk construct_fs3_cmdblock(uint8_t op, int16_t sec, int_fast32_t trk, uint8_t ret)
{
	uint64_t opNew = op;
	uint64_t secNew = sec;
	uint64_t trkNew = trk;
	uint64_t retNew = ret;
	FS3CmdBlk x = 0;

	x = x | (opNew << 60);

	x = x | (secNew << 44);

	x = x | (trkNew << 12);

	x = x | (retNew << 4);

	return x;
}
// create an FS3 array opcode from the variable fields

// This deconstructs the command block
int deconstruct_fs3_cmdblock(FS3CmdBlk cmdblock, uint8_t *op, int16_t *sec, int32_t *trk, uint8_t *ret)
{

	uint64_t opDec = 255;
	uint64_t secDec = 65535;
	uint64_t trkDec = 4294967295;
	uint64_t retDec = 255;

	*op = (cmdblock >> 60) & opDec;

	*sec = (cmdblock >> 44) & secDec;

	*trk = (cmdblock >> 12) & trkDec;

	*ret = (cmdblock >> 4) & retDec;

	return (0);
}
// extract register state from bus values

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_mount_disk
// Description : FS3 interface, mount/initialize filesystem
//
// Inputs : none
// Outputs : 0 if successful, -1 if failure

int32_t fs3_mount_disk(void)
{

	uint8_t op = 0; // Mount Op code = 0 --> Follow cmdBlk rules in readme
	int16_t sec = 0;
	int32_t trk = 0;
	uint8_t ret = 0;
	FS3CmdBlk x;
	int i;
	int j;
	for (i = 0; i < 1024; i++)
	{
		newFiles[i].size = 0; // Initializing all struct objects to a set value
		newFiles[i].position = 0;
		newFiles[i].handle = -1;
		newFiles[i].FileIsOpen = 0;
	}

	for (i = 0; i < 64; i++)
	{ // Sets 2D equal to -1 which will be used for read/write functions
		for (j = 0; j < 1024; j++)
		{
			arr2[i][j] = -1;
		}
	}

	x = construct_fs3_cmdblock(op, sec, trk, ret);
	x = network_fs3_syscall(x, &y, NULL);
	deconstruct_fs3_cmdblock(x, &op, &sec, &trk, &ret);
	if (ret == 1) // If ret returns a value of 1 the program failed
	{
		return -1;
	}
	else
	{
		mountStatus = 1;
		return 0;
	}
	return -1;
}
// Given the variable fields, check if mounted and in not mount (when ret == 0). Clean file position/size.

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_unmount_disk
// Description : FS3 interface, unmount the disk, close all files
//
// Inputs : none
// Outputs : 0 if successful, -1 if failure

int32_t fs3_unmount_disk(void)
{
	uint8_t op = 4; // Unmount Op = 4 --> Follow cmdblk rules in readme
	int16_t sec = 0;
	int32_t trk = 0;
	uint8_t ret = 0;
	FS3CmdBlk x;
	int i;
	for (i = 0; i < 1024; i++)
	{
		newFiles[i].size = 0; // Initializing all objects in struct to original values (deleting everything after unmount is called)
		newFiles[i].position = 0;
		newFiles[i].handle = -1;
		newFiles[i].FileIsOpen = 0;
	}

	x = construct_fs3_cmdblock(op, sec, trk, ret);
	x = network_fs3_syscall(x, &y, NULL);
	deconstruct_fs3_cmdblock(x, &op, &sec, &trk, &ret);

	if (ret == 1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_open
// Description : This function opens the file and returns a file handle
//
// Inputs : path - filename of the file to open
// Outputs : file handle if successful, -1 if failure

int16_t fs3_open(char *path)
{
	int i;
	for (i = 0; i < 1024; i++)
	{
		if (strcmp(path, newFiles[i].name) == 0) // Check if file exists by checking if path is same as file name
		{

			if (newFiles[i].FileIsOpen == 1) // Check if its already open, FileIsOpen
			{
				return -1; // Return -1
			}
			else
			{
				newFiles[i].FileIsOpen = 1; // Open file
				return newFiles[i].handle;	// Return the handle according the rubric
			}
		}
	}
	for (i = 0; i < 1024; i++)
	{
		if (newFiles[i].handle == -1)
		{
			newFiles[i].handle = i;	  // Set handle to set value i
			newFiles[i].position = 0; // Set file position/size to 0 since it is a new file
			newFiles[i].size = 0;
			newFiles[i].FileIsOpen = 1;
			strcpy(newFiles[i].name, path); // Open file, copy path name to the new file (will return file handle)
			return newFiles[i].handle;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_close
// Description : This function closes the file
//
// Inputs : fd - the file descriptor
// Outputs : 0 if successful, -1 if failure

int16_t fs3_close(int16_t fd)
{
	int i;
	for (i = 0; i < 1024; i++)
	{
		if (fd == newFiles[i].handle) // Check if handle is equal to file handle
		{
			newFiles[i].position = 0;	// When closing set position to 0
			newFiles[i].FileIsOpen = 0; // FileIsopen to 0;
			return 0;					// Return 0 if successful
		}
		else
		{
			return -1; // Return -1 if failure
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_read
// Description : Reads "count" bytes from the file handle "fh" into the
// buffer "buf"
//
// Inputs : fd - filename of the file to read from
// buf - pointer to buffer to read into
// count - number of bytes to read
// Outputs : bytes read if successful, -1 if failure

int32_t fs3_read(int16_t fd, void *buf, int32_t count)
{
	uint8_t op = 2; // Read Op = 2
	uint8_t opT = 1;
	int16_t sec = -1; // Set others to 0 since its 1 file
	int32_t trk = -1;
	uint8_t ret = 0;
	FS3CmdBlk x;
	FS3SectorIndex secInd;
	int sec_pos = 0;
	int count2 = 0;
	char buf2[1024]; // Character array that will be used to later for memcpy
	int i;
	int j;
	int bool = 0;
	int size;
	int count3 = count;

	while (count > 0)
	{
		bool = 0;
		sec_pos = newFiles[fd].position % 1024; // While loop that loops through find/setting track and sec to perform read function on
		secInd = newFiles[fd].position / 1024;
		count2 = 0;
		for (i = 0; i < 64; i++)
		{
			for (j = 0; j < 1024; j++)
			{
				if (arr2[i][j] == fd)
				{
					if (count2 == secInd)
					{
						trk = i;
						sec = j;
						bool = 1; // Flag thats used for breaking outer for loop
						break;
					}
					else
					{
						count2++; // Count2 increments until it equals sector index to run function
					}
				}
			}
			if (bool == 1)
			{
				break;
			}
		}
		if (trk == -1 && sec == -1)
		{
			break; // If track and sec remain -1 break loop
		}

		if (mountStatus == 0) // Check if its mount, if not return -1
		{
			return -1;
		}
		if (newFiles[fd].FileIsOpen == 0) // Check if the file is open, if not return -1
		{
			return -1;
		}
		if (ret == 1) // If ret returns a value of 1 the program failed
		{
			return -1;
		}

		if (1024 - sec_pos < count) // Function that finds size of write
		{
			size = 1024 - sec_pos;
		}
		else
		{
			size = count;
		}
		char *newerBuf = fs3_get_cache(trk, sec);
		if (newerBuf == NULL)
		{
			x = construct_fs3_cmdblock(opT, sec, trk, ret); // TSeek operations
			x = network_fs3_syscall(x, &y, NULL);
			deconstruct_fs3_cmdblock(x, &opT, &sec, &trk, &ret); // Three system calls in order to complete the folowing

			x = construct_fs3_cmdblock(op, sec, trk, ret); // Do system call to update fields
			x = network_fs3_syscall(x, &y, buf2);
			deconstruct_fs3_cmdblock(x, &op, &sec, &trk, &ret);
			memcpy(&((char *)buf)[count3 - count], &buf2[sec_pos], size); // Memcpy data in buf2 (at specific position) to buf
			fs3_put_cache(trk, sec, buf2);
		}
		else
		{
			memcpy(&((char *)buf)[count3 - count], &newerBuf[sec_pos], size); // If fs3_get_cache is not null use the buf return as newBuf
		}

		newFiles[fd].position += size; // Setting position to value of count
		count -= size;
		trk = -1; // Set trk and sec = -1 to allow process to repeat of finding new sectors/tracks to perform function
		sec = -1;
	}
	return (count3 - count); // Return number of bytes written
}

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_write
// Description : Writes "count" bytes to the file handle "fh" from the
// buffer "buf"
//
// Inputs : fd - filename of the file to write to
// buf - pointer to buffer to write from
// count - number of bytes to write
// Outputs : bytes written if successful, -1 if failure

int32_t fs3_write(int16_t fd, void *buf, int32_t count)
{
	uint8_t op = 3;	 // Op code for Write
	uint8_t opT = 1; // op code for Tseek
	uint8_t opR = 2; // Op code for Read
	int16_t sec = -1;
	FS3SectorIndex secInd; // sector index
	int sec_pos = 0;
	int count2 = 0;
	int32_t trk = -1;
	uint8_t ret = 0;
	char buf2[1024]; // Local buffer
	FS3CmdBlk x;
	int i;
	int j;
	int size;
	int bool = 0;
	int count3 = count;

	while (count > 0)
	{
		bool = 0;
		sec_pos = newFiles[fd].position % 1024;
		secInd = newFiles[fd].position / 1024; // While loop that loops through finding/setting track and sec to perform function on
		count2 = 0;
		for (i = 0; i < 64; i++)
		{
			for (j = 0; j < 1024; j++)
			{
				if (arr2[i][j] == fd)
				{
					if (count2 == secInd)
					{
						trk = i;
						sec = j;
						bool = 1;
						break;
					}
					else
					{
						count2++; // Count2 increments until it equals sector index to run function
					}
				}
			}
			if (bool == 1) // Flag that says if bool == 1 then function must of broke out of nested for loop so break this for loop as well
			{
				break;
			}
		}
		if (trk == -1 && sec == -1) // If track and sec remain -1 loop through and check if 2d array = -1 then set trk and sec to their respective values
		{
			for (i = 0; i < 64; i++)
			{
				for (j = 0; j < 1024; j++)
				{
					if (arr2[i][j] == -1)
					{
						trk = i;
						sec = j;
						arr2[i][j] = fd;
						bool = 1;
						break;
					}
				}
				if (bool == 1)
				{
					break;
				}
			}
		}
		if (mountStatus == 0) // Check if its mount, if not return -1
		{
			return -1;
		}
		if (newFiles[fd].FileIsOpen == 0) // Check if the file is open, if not return -1
		{
			return -1;
		}
		if (ret == 1) // If ret returns a value of 1 the program failed
		{
			return -1;
		}

		if (1024 - sec_pos < count) // Function that finds size of write
		{
			size = 1024 - sec_pos;
		}
		else
		{
			size = count;
		}
		char *newerBuf = fs3_get_cache(trk, sec);
		if (newerBuf == NULL)
		{
			x = construct_fs3_cmdblock(opT, sec, trk, ret); // TSeek operations
			x = network_fs3_syscall(x, &y, NULL);
			deconstruct_fs3_cmdblock(x, &opT, &sec, &trk, &ret); // Three system calls in order to complete the folowing

			x = construct_fs3_cmdblock(opR, sec, trk, ret); // Read system call
			x = network_fs3_syscall(x, &y, buf2);
			deconstruct_fs3_cmdblock(x, &opR, &sec, &trk, &ret);

			memcpy(&buf2[sec_pos], &((char *)buf)[count3 - count], size); // Write operations
			x = construct_fs3_cmdblock(op, sec, trk, ret);
			x = network_fs3_syscall(x, &y, buf2);
			deconstruct_fs3_cmdblock(x, &op, &sec, &trk, &ret);
			fs3_put_cache(trk, sec, buf2);
		}
		else
		{
			memcpy(&newerBuf[sec_pos], &((char *)buf)[count3 - count], size);
			x = construct_fs3_cmdblock(op, sec, trk, ret);
			x = network_fs3_syscall(x, &y, newerBuf);
			deconstruct_fs3_cmdblock(x, &op, &sec, &trk, &ret);
			fs3_put_cache(trk, sec, newerBuf); // If fs3_get_cache is not null use the buf return as newBuf
		}

		newFiles[fd].position += size; // Setting position to value of count
		count -= size;				   // Decrementing count to how many bytes are left over
		if (newFiles[fd].position > newFiles[fd].size)
		{
			newFiles[fd].size = newFiles[fd].position; // If the write goes beyond, the size should increase --> updating size to value of file position
		}
		trk = -1; // Set trk and sec = -1 to allow process to repeat of finding new sectors/tracks to perform function
		sec = -1;
	}
	return (count3 - count); // Return number of bytes written
}

////////////////////////////////////////////////////////////////////////////////
//
// Function : fs3_seek
// Description : Seek to specific point in the file
//
// Inputs : fd - filename of the file to write to
// loc - offfset of file in relation to beginning of file
// Outputs : 0 if successful, -1 if failure

int32_t fs3_seek(int16_t fd, uint32_t loc)
{
	if (mountStatus == 0)
	{ // Check if it is mount, if not return error
		return -1;
	}

	if (newFiles[fd].FileIsOpen == 0)
	{ // Check if file is open, if not set handle to 0 and return error
		newFiles[fd].handle = 0;
		return -1;
	}

	if (fd != newFiles[fd].handle)
	{ // If handle does not equal fd, return error
		return -1;
	}

	if (loc <= newFiles[fd].size && loc >= 0)
	{ // If the location is within the size of the file 0 or larger, set the file position (current position) to that location.
		newFiles[fd].position = loc;
		return 0;
	}

	return -1; // Return error if handle is bad, file is closed, and loc is beyond end of file
}