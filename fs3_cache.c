////////////////////////////////////////////////////////////////////////////////
//
//  File           : fs3_cache.c
//  Description    : This is the implementation of the cache for the
//                   FS3 filesystem interface.
//
//  Author         : Patrick McDaniel
//  Last Modified  : Sun 17 Oct 2021 09:36:52 AM EDT
//

// Includes
#include "cmpsc311_log.h"

// Project Includes
#include "fs3_cache.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//
// Support Macros/Data

// cache struct

typedef struct
{
    char buf[1024];
    int trkFind;
    int secFind;
    int lastAcc;
} cache;
// struct that holds initialized variables associated to the file

// global variables that are modifiable
cache *cacheStruct;
int hit;
int miss;
int cacheIns;
int clk;
int maxCache;
//
// Implementation

////////////////////////////////////////////////////////////////////////////////
//
// Function     : fs3_init_cache
// Description  : Initialize the cache with a fixed number of cache lines
//
// Inputs       : cachelines - the number of cache lines to include in cache
// Outputs      : 0 if successful, -1 if failure

int fs3_init_cache(uint16_t cachelines)
{
    int i;
    maxCache = cachelines;

    cacheStruct = malloc(sizeof(cache) * cachelines); // Allocate memory for cache
    if (cacheStruct == NULL)                          // If cache NULL set to failure
    {
        return -1;
    }

    for (i = 0; i < (0x8); i++) // Walk array marking cachelines unused.
    {
        cacheStruct[i].trkFind = -1;
        cacheStruct[i].secFind = -1;
        cacheStruct[i].lastAcc = 0;
    }
    return 0; // If run correctly, return success
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : fs3_close_cache
// Description  : Close the cache, freeing any buffers held in it
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int fs3_close_cache(void)
{
    if (cacheStruct != NULL)
    {
        free(cacheStruct);  // When closing, free all memory from cache
        cacheStruct = NULL; // After freeing, set cacheStruct to NULL since it is still being pointed to
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : fs3_put_cache
// Description  : Put an element in the cache
//
// Inputs       : trk - the track number of the sector to put in cache
//                sct - the sector number of the sector to put in cache
// Outputs      : 0 if inserted, -1 if not inserted

int fs3_put_cache(FS3TrackIndex trk, FS3SectorIndex sct, void *buf)
{
    int i;
    int current;
    int memInd;
    for (i = 0; i < maxCache; i++)
    {
        if (cacheStruct[i].secFind == sct && cacheStruct[i].trkFind == trk) // Case where you find a sector and track and you put it in the cache
        {
            memcpy(cacheStruct[i].buf, buf, 1024);
            cacheStruct[i].lastAcc = clk;
            clk++;
            cacheIns++; // Updating cache Inserts in every case for metrics
            return 0;
        }
    }
    for (i = 0; i < maxCache; i++)
    {
        if (cacheStruct[i].secFind == -1 && cacheStruct[i].trkFind == -1) // Case where you don't find a sector and a track and put mem in cache
        {
            memcpy(cacheStruct[i].buf, buf, 1024);
            cacheStruct[i].trkFind = trk;
            cacheStruct[i].secFind = sct;
            cacheStruct[i].lastAcc = clk;
            clk++;
            cacheIns++; // Updating cache Inserts in every case for metrics
            return 0;
        }
    }
    current = cacheStruct[0].lastAcc; // LRU case, where you walk through the array searching for the least used access time and replacing that mem with the new one
    for (i = 0; i < maxCache; i++)
    {
        if (cacheStruct[i].lastAcc < current)
        {
            current = cacheStruct[i].lastAcc;
            memInd = i; // Saving that new minimum memory index
        }
    }
    memcpy(cacheStruct[memInd].buf, buf, 1024); // Memcopies and setting that mem to its specific track and sec values
    cacheStruct[memInd].trkFind = trk;
    cacheStruct[memInd].secFind = sct;
    cacheStruct[memInd].lastAcc = clk;
    clk++;
    cacheIns++; // Updating cache Inserts in every case for metrics
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : fs3_get_cache
// Description  : Get an element from the cache (
//
// Inputs       : trk - the track number of the sector to find
//                sct - the sector number of the sector to find
// Outputs      : returns NULL if not found or failed, pointer to buffer if found

void *fs3_get_cache(FS3TrackIndex trk, FS3SectorIndex sct)
{
    return NULL;
    int i;
    for (i = 0; i < maxCache; i++)
    {
        if (cacheStruct[i].secFind == sct && cacheStruct[i].trkFind == trk) // Walk through array and see if track and sec are in the cache
        {
            cacheStruct[i].lastAcc = clk;
            clk++;
            hit++;                     // Update my hits for metrics
            return cacheStruct[i].buf; // If track and sec found, then return the buffer and continue function in driver.c
        }
    }
    miss++; // Update my misses for metrics
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : fs3_log_cache_metrics
// Description  : Log the metrics for the cache
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int fs3_log_cache_metrics(void)
{
    int cacheGet = miss + hit;                                  // Update my cache gets everytime my missess and hits update
    float cacheHitRatio = ((float)hit / (float)cacheIns) * 100; // Update my cache hit ratio everytime by hits and cache inserts update

    printf("** FS3 cache Metrics **\n");
    printf("Cache inserts    [    %d]\n", cacheIns); // Print statements that form metrics for end of program reports
    printf("Cache gets       [    %d]\n", cacheGet);
    printf("Cache hits       [    %d]\n", hit);
    printf("Cache misses     [    %d]\n", miss);
    printf("Cache hit ratio  [%%%.2f]\n", cacheHitRatio);
    return 0;
}
