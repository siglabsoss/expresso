/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file MemFormat.cpp */

using namespace std;

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "MemFormat.h"

using namespace LatticeSemi_PCIe;


/**
 * Create a utility that performs byte/short/long read/writes to memory.
 * Formatting for output is done correctly.
 * @param pDev device class used for actual register reads and writes
 * @param maxBuf maximum number of bytes that can be read at one time
 */
MemFormat::MemFormat(Device *pDev, size_t maxBuf)
{

    rd_size = 1;
	rd_addr = 0;
	rd_count = 1;

    wr_size = 1;
	wr_addr = 0;
	wr_count = 1;
	
	pMem = pDev;

	MaxBlockSize = maxBuf;  // Allocate this much buffer space
              
	buf8 = new uint8_t[MaxBlockSize];
	buf16 = new uint16_t[MaxBlockSize / 2];
	buf32 = new uint32_t[MaxBlockSize / 4];
}


/**
 * Delete the Memory Access object, which frees allocated formatting buffers.
 */
MemFormat::~MemFormat()
{
     delete buf8;         
     delete buf16;         
     delete buf32;         
}


/**
 * Parse a text command and perform the memory access/display operation.
 * The command can specify reading memory or writing memory.
 * The command has the syntax:
 * r(b|s|l) <address> (<count>)
 * w(b|s|l) <address> data0 (data1 ...)
 * 
 * The text command allows calling from a menu-like program or passing the command
 * string from a GUI written in a different language (VisualBasic, Java, Python, etc).
 * Results of executing the command are returned in a string (C++ object).
 * The command is not sent to stdout.  This allows the command results to be
 * handled as needed by the caller (print to screen, send to a GUI, etc).
 * 
 * @param cmdStr the null terminated text string containing the command to execute
 * @param results the C++ string object to return the results of the command in
 */
bool MemFormat::doCmd(char *cmdStr, string &results)
{
     char cmd[80];
     int i, n, nVals;
     uint32_t addr, count;
     uint8_t val8;
     uint16_t val16;
     uint32_t val32;
     
     // Remove any white space at beginning
     cmdStr = strip(cmdStr);
     
	try
	{

	     // parse command
	     switch (cmdStr[0])
	     {
				case 'r':	// read:  r [addr [count]]
				case 'R':
					n = sscanf(cmdStr, "%s %x %d", cmd, &addr, &count);

					if (cmd[0] != 'r')
					   return(false);
			if (cmd[1] == 'b')
			   rd_size = 1;
			else if (cmd[1] == 's')
			   rd_size = 2;
			else if (cmd[1] == 'l')
			   rd_size = 4;
			   
					if (count * rd_size > MaxBlockSize)
					   count = MaxBlockSize/ rd_size;


					if (n == 1)
					{
						rd_addr = rd_addr + rd_count;
					}
					else if (n == 2)
					{
						rd_addr = addr;
					}
					else if (n == 3)
					{
						rd_addr = addr;
						rd_count = count;
					}
					// perform operation based on size
					switch (rd_size)
					{
			       case 1:
				    if (rd_count == 1)
					buf8[0] = pMem->read8(rd_addr);
				    else
					pMem->read8(rd_addr, buf8, rd_count);
				    formatBlockOfBytes(results, rd_addr, rd_count, buf8);
				    break;

			       case 2:
				    if (rd_count == 1)
					buf16[0] = pMem->read16(rd_addr);
				    else
					pMem->read16(rd_addr, buf16, rd_count);
				    formatBlockOfShorts(results, rd_addr, rd_count, buf16);
				    break;
				    
			       case 4:
				    if (rd_count == 1)
					buf32[0] = pMem->read32(rd_addr);
				    else
					pMem->read32(rd_addr, buf32, rd_count);
				    formatBlockOfWords(results, rd_addr, rd_count, buf32);
				    break;
			}
				    break;

				case 'w':  // write:  w <address> [data_1]...[data_n]
				case 'W':
					n = sscanf(cmdStr, "%s %x %x", cmd, &addr, &val32);
					if (n == 2)
					   return(false);   // nothing to do
					if (cmd[0] != 'w')
					   return(false);
			if (cmd[1] == 'b')
			   wr_size = 1;
			else if (cmd[1] == 's')
			   wr_size = 2;
			else if (cmd[1] == 'l')
			   wr_size = 4;
					if (n == 1)
					   return(true);   // changed write mode, but nothing to write

			// Parse the list of hex values
			nVals = parseHexList(cmdStr, 2, buf32);
			if (nVals == 0)
			   return(false);  // Something went wrong, nothing to write
			wr_addr = addr;
			   
					// perform operation based on size
					switch (wr_size)
					{
			       case 1:
				    results = "wr8";
				    if (nVals == 1)
				    {
					      val8 = (uint8_t)buf32[0];
					      pMem->write8(wr_addr, val8);
				    }
				    else
				    {
					for (i = 0; i < nVals; i++)
					    buf8[i] = (uint8_t)buf32[i];         
					pMem->write8(wr_addr, buf8, nVals);
				    }    
				    break;

			       case 2:
				    results = "wr16";
				    if (nVals == 1)
				    {
					      val16 = (uint16_t)buf32[0];
					      pMem->write16(wr_addr, val16);
				    }
				    else
				    {
					for (i = 0; i < nVals; i++)
					    buf16[i] = (uint16_t)buf32[i];         
					pMem->write16(wr_addr, buf16, nVals);
				    }    
				    break;
				    
			       case 4:
				    results = "wr32";
				    if (nVals == 1)
				    {
					 pMem->write32(wr_addr, buf32[0]);
				    }
				    else
				    {
					pMem->write32(wr_addr, buf32, nVals);
				    }    
				    break;
			}
				
					break;
	    
	       default:
		       return(false);
	    }
	}
	catch (std::exception &e)
	{
		cout<<"\nMemFormat or Access Error: "<<e.what()<<endl;
		return(false);
	}
    
    return(true);
}




/**
 * Format a buffer of 8 bit values (bytes) into a displayable text string.
 *
 *  0000:        0x55 0x66 0x77
 *  0010: 0xa0  0xb0  0xc0
 * 
 * @param ostr the caller supplied string to return the formatted bytes in
 * @param startAddr the starting address of the data, used in address field of format
 * @param len the number of bytes to format
 * @param data the array of bytes to process
 * @return true if successful, false if error encountered
 */
bool MemFormat::formatBlockOfBytes(string &ostr, uint32_t startAddr, uint32_t len, uint8_t *data)
{
	uint32_t i, j;
	uint32_t addr;
	char *blank;
	char buf[256];

	if (len == 1)
	{
		sprintf(buf, "%08x: %02x\n", startAddr, *data);
		ostr.append(buf);
		return(true);
	}

	if (len > 8)
	{
		ostr.append("\n");	/* can't fit all on one line so start its own block */
		blank = "    ";
	}
	else
	{
		blank = "";

	}

	addr = (startAddr & 0xfffffff0);
	len = len + (startAddr - addr);

	j = 0;


	for (i = 0; i < len; i++)
	{
		if ((i % 16) == 0)
		{
			if (i > 0)
				sprintf(buf, "\n%08x:", addr);
			else
				sprintf(buf, "%08x:", startAddr);
			ostr.append(buf);
		}
		else if ((i % 8) == 0)
		{
			ostr.append("  ");	// put a little space between 8 cols and 8 cols
		}

		if (addr < startAddr)
			ostr.append(blank);
		else
		{
			sprintf(buf, "  %02x", data[j]);
			ostr.append(buf);
			++j;
		}

		++addr;
	}

	ostr.append("\n");

	return(true);
}



/**
 * Format a buffer of 16 bit values (shorts) into a displayable text string.
 *
 *  0000:        0x5566 0x7788
 *  0010: 0xa0b0 0xc0d0
 * 
 * @param ostr the caller supplied string to return the formatted shorts in
 * @param startAddr the starting address of the data, used in address field of format
 * @param len the number of shorts to format
 * @param data the array of shorts to process
 * @return true if successful, false if error encountered
 */
bool MemFormat::formatBlockOfShorts(string &ostr, uint32_t startAddr, uint32_t len, uint16_t *data)
{
	uint32_t i, j;
	uint32_t addr;
	char *blank;
	char buf[256];

	if (len == 1)
	{
		sprintf(buf, "%08x: %04x\n", startAddr, *data);
		ostr.append(buf);
		return(true);
	}


	if (len > 4)
	{
		ostr.append("\n");	/* can't fit all on one line so start its own block */
		blank = "      ";
	}
	else
	{
		blank = "";
	}

	addr = (startAddr & 0xfffffff0);
	len = len + (startAddr - addr) / 2;

	j = 0;

	for (i = 0; i < len; i++)
	{
		if ((i % 8) == 0)
		{
			if (i > 0)
				sprintf(buf, "\n%08x:", addr);
			else
				sprintf(buf, "%08x:", startAddr);
			ostr.append(buf);
		}
		else if ((i % 4) == 0)
		{
			ostr.append("  ");	// divide output into 2 cols
		}


		if (addr < startAddr)
			ostr.append(blank);
		else
		{
			sprintf(buf, "  %04x", data[j]);
			ostr.append(buf);
			++j;
		}

		addr = addr + 2;
	}

	ostr.append("\n");

	return(true);
}


/**
 * Format a buffer of 32 bit values (words) into a displayable text string.
 *
 *  0000:            0x55667788
 *  0010: 0xa0b0c0d0 0xaabbccdd 0x11223344
 * 
 * @param ostr the caller supplied string to return the formatted words in
 * @param startAddr the starting address of the data, used in address field of format
 * @param len the number of words to format
 * @param data the array of words to process
 * @return true if successful, false if error encountered
 */
bool MemFormat::formatBlockOfWords(string &ostr, uint32_t startAddr, uint32_t len, uint32_t *data)
{
	uint32_t i, j;
	uint32_t addr;
	char *blank;
	char buf[256];

	if (len == 1)
	{
		sprintf(buf, "%08x: %08x\n", startAddr, *data);
		ostr.append(buf);
		return(true);
	}

	if (len > 2)
	{
		ostr.append("\n");	/* can't fit all on one line so start its own block */
		blank = "          ";
	}
	else
	{
		blank = "";
	}

	addr = (startAddr & 0xfffffff0);
	len = len + (startAddr - addr) / 4;

	j = 0;
	for (i = 0; i < len; i++)
	{
		if ((i % 4) == 0)
		{
			if (i > 0)
				sprintf(buf, "\n%08x:", addr);
			else
				sprintf(buf, "%08x:", startAddr);
			ostr.append(buf);
		}
		else if ((i % 2) == 0)
		{
			ostr.append("  ");	// divide output into 2 cols
		}

		if (addr < startAddr)
		{
			ostr.append(blank);
		}
		else
		{
			sprintf(buf, "  %08x", data[j]);
			ostr.append(buf);
			++j;
		}

		addr = addr + 4;
	}

	ostr.append("\n");

	return(true);
}


/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/
//                            PRIVATE METHODS
/*===================================================================================*/
/*===================================================================================*/
/*===================================================================================*/

/**
 * Strip whitespace and control chars from beginning of string, returning first
 * printable character.
 */
char* MemFormat::strip(char *s)
{
	while (((*s <= 0x21) || ((unsigned char)*s > 0x7f)) && (*s != '\0'))
		++s;
	return(s);
}


/**
 * Skip over whitespace and return pointer to the first printable character.
 */
char * MemFormat::skipWhiteSpace(char *s)
{
    while(*s <= 0x20)
    {
        if (*s == '\0')
            return(NULL);
        else
            ++s;  // next char
    }

    return(s);
}


/**
 * Skip over a word and return pointer to the first whitespace character.
 */
char * MemFormat::skipWord(char *s)
{
    while(*s > 0x20)
    {
        if (*s == '\0')
            return(NULL);
        else
            ++s;  // next char
    }

    return(s);
}


/**
 * Parse a string of white-space separated hex values, returning the
 * numeric values in the array buf[].
 * @param str the character string of values: "aa 55 FFFF 001"
 * @param skip the number of words to skip in the beginning
 * @param buf storage for the converted integer values
 * @return the number of integers converted
 */
int MemFormat::parseHexList(char *str, int skip, uint32_t *buf)
{
    char *s;
    size_t cnt;

    s = str;

   while (skip)
   {
        s = skipWhiteSpace(s);
        if (s == NULL)
            return(-1);
        s = skipWord(s);
        if (s == NULL)
            return(-1);
        --skip;
   }

    s = skipWhiteSpace(s);

    cnt = 0;
    while (s != NULL)
    {
        if (sscanf(s, "%x", &buf[cnt]) == 1)
            ++cnt;
        s = skipWord(s);
        if (s == NULL)
            break;
        s = skipWhiteSpace(s);
        
        if (cnt == MaxBlockSize)
           break;   // Reached maximum storage space, ignore rest
    }

    return(cnt);
}
