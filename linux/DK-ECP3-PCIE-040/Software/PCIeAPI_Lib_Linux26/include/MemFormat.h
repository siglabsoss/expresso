/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file MemFormat.h
 * The MemFormat Class provides a utility for accessing a Device class
 * and issuing a text command string to perform read /writes to device memory.
 * This is most useful for interactive menu settings where the user wants
 * to dump a device's memory or write a set of data to memory.
 * Formatting of a block of memory can also be done using the formatXXX()
 * methods.  The data is formatted into a text string with line feeds
 * and addresses.
 */
 
#ifndef LATTICE_SEMI_MEMFORMAT_H
#define LATTICE_SEMI_MEMFORMAT_H


#include <unistd.h>
#include <sys/types.h>
#include <string>

#include "dllDef.h"
#include "Device.h"

/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{



/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/*
 *    MEMORY Read/Write Utility
 */
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
/**
 * The Memory access and formatting class */
class DLLIMPORT MemFormat
{
public:

    MemFormat(Device *pDev, size_t maxBuf=256);
    ~MemFormat();
    bool doCmd(char *cmdStr, string &s);
    
	static bool formatBlockOfBytes(string &fs, uint32_t startAddr, uint32_t len, uint8_t *data);
	static bool formatBlockOfShorts(string &fs, uint32_t startAddr, uint32_t len, uint16_t *data);
	static bool formatBlockOfWords(string &fs, uint32_t startAddr, uint32_t len, uint32_t *data);



private:
	/* Constants and defines */
    uint32_t rd_size;
	uint32_t rd_addr;
	uint32_t rd_count;

    uint32_t wr_size;
	uint32_t wr_addr;
	uint32_t wr_count;
    
	uint8_t  *buf8;
	uint16_t *buf16;
	uint32_t *buf32;
	size_t MaxBlockSize;

    Device *pMem;   // Class that defines how to read/write device memory
        
    char* strip(char *s);         
	char* skipWhiteSpace(char *s);
	char* skipWord(char *s);
	int parseHexList(char *str, int skip, uint32_t *buf);
};




}  // end namespace

#endif
