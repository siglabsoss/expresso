/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Mem.cpp */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "PCIeAPI.h"
#include "DebugPrint.h"
#include "Mem.h"

using namespace LatticeSemi_PCIe;

/**
 * Construct a generic Memory device.
 * Usually used to represent an EBR (Embedded Block RAM) in the FPGA IP.
 * It could be used to interface to any memory mapped object.
 * Theis class provides helper functions for testing and loading and clearing
 * the memory contents.  The memory, by default, is accessed using 32 bit accesses.
 * If the device does not support 32 bit or other size accesses are to be tested,
 * call setAccessWidth().
 * <p>
 * Register access to the IP in the FPGA device is through the 
 * LSC_PCIe object which uses the lscpcie device driver
 * to access the real hardware and do reads/writes.
 * 
 * The base class Device is initialized to hold the standard parameters.
 *
 * @param nameStr the name of the FPGA device for unique naming
 * @param size the number of bytes in the memory to avoid bounds issues
 * @param baseAddr the physical bus address the device registers start at
 * @param pRegs pointer to a RegisterAccess object for accessing the hardware
 */
Mem::Mem(const char *nameStr, size_t size, uint32_t baseAddr, RegisterAccess *pRA) :
Device(pRA, nameStr, baseAddr, size)
{
	/* Base Class Device has setup the name, address already */

	this->len = size;  
	this->accessWidth = 32;  // default, change via call if not supported

}


/**
 * Destroy the specific FPGA device.
 * Device interrupts are disabled.
 * Register access object to the SC device is destroyed.
 * The device should no longer be accessed using any methods.  All references
 * to the device will now be invalid.
 */
Mem::~Mem()
{

}


/**
 * Return the length of the memory device, in bytes.
 * @return size of the device (as specified in the constructor).
 */
size_t Mem::getLen(void) 
{ 
	return(len); 
}

/**
 * Set the width of data accesses to the memory device. 
 * Default is 32 bit , but if the memory has restrictions and
 * can't be accessed on 32 bit boundaries then call this method
 * to change the access method: read8/16/32.
 */
void Mem::setAccessWidth(uint8_t size)
{
	accessWidth = size;
}

/**
 * Memory Clear Test.
 * Clear entire contents of memory (EBR) to 00's.
 * Full address range is cleared.
 * The memory size is known from the length parameter
 * passed when the memory object was instantiated.
 */
bool Mem::clear(void)
{
	// Allocate buffer for memory clearing
	uint8_t *wrBuf = new uint8_t[this->len];

	memset(wrBuf, 0, this->len);
	this->set(0, this->len, wrBuf);

	delete wrBuf;

	return(true);
}

/**
 * Memory Fill.
 * Test filling all memory bytes with the same value.
 * Full address range is tested with various patterns.
 * The memory size is known from the length parameter
 * passed when the memory object was instantiated.
 * 
 * @param val the byte value to write into all the memory locations.
 */
bool Mem::fill(uint8_t val)
{
	// Allocate buffer for memory clearing
	uint8_t *wrBuf = new uint8_t[this->len];

	memset(wrBuf, (int)val, this->len);
	this->set(0, this->len, wrBuf);

	delete wrBuf;

	return(true);
}


/**
 * Memory Fill.
 * Test filling a range of memory bytes with the same value.
 * The address range is specified with the offset and len.
 * If offset + len > device's memory size, false is returned.
 * 
 * @param val the byte value to write into all the memory locations.
 * @param offset the starting location in Mem device to begin filling
 * @param flen the nubmer of bytes to write
 * 
 * @return true if memory written, false if address range error
 */
bool Mem::fill(uint8_t val, uint32_t offset, size_t flen)
{
	if (flen + offset > this->len)
		return(false);

	// Allocate buffer for memory clearing
	uint8_t *wrBuf = new uint8_t[flen];

	memset(wrBuf, (int)val, flen);
	this->set(offset, flen, wrBuf);

	delete wrBuf;

	return(true);
}



/**
 * Memory Fill with a pattern.
 * Test filling all memory words with the same value.
 * The pattern is the upper 16 bits, the lower 16 bits increments
 * for each word location.
 * The memory size is known from the length parameter
 * passed when the memory object was instantiated.
 * 
 * @param val the initial pattern value to write into all the memory locations.
 */
bool Mem::fillPattern(uint32_t val)
{
	uint32_t i;
	// Allocate buffer for memory clearing
	uint32_t *wrBuf = new uint32_t[(this->len / 4)];

	for (i = 0; i < (this->len / 4); i++)
		wrBuf[i] = val | (i & 0x0000ffff);

	this->set(0, this->len, wrBuf);

	delete wrBuf;

	return(true);
}


/**
 * Memory Fill a range with a pattern.
 * Fill a pattern into range of memory bytes with the same value.
 * The address range is specified with the offset and len.
 * If offset + len > device's memory size, false is returned.
 * 
 * @param val the initial pattern value to write into the memory locations.
 * @param offset the starting location in Mem device to begin filling (byte address)
 * @param flen the nubmer of bytes to write
 * 
 * @return true if memory written, false if address range error
 */
bool Mem::fillPattern(uint32_t val, uint32_t offset, size_t flen)
{
	uint32_t i;
	if (flen + offset > this->len)
		return(false);

	// Allocate buffer for memory clearing
	uint32_t *wrBuf = new uint32_t[(this->len / 4)];

	for (i = 0; i < (flen / 4); i++)
		wrBuf[i] = val | (i & 0x0000ffff);

	this->set(offset, flen, wrBuf);

	delete wrBuf;

	return(true);
}



/**
 * Memory Test the contents of all locations.
 * Full address range is tested with various patterns.
 * @return true if all locations pass all tests; false if any error detected
 */
bool Mem::test(size_t len)
{
	size_t i;
	int errs;
	bool verbose = RUNTIMECTRL(VERBOSE);

	if (len == 0)
		len = this->len;  // default to entire memory device size

	bool done = false;

	// Allocate buffer for memory testing
	uint8_t *wrBuf = new uint8_t[len];
	uint8_t *rdBuf = new uint8_t[len];

	if (verbose)
		cout<<"=== Memory Test ===\n";

	while (!done)
	{
		errs = 0;
		if (verbose)
			cout<<"Writing all 00's...";
		memset(wrBuf, 0, len);
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}
		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort

		if (verbose)
			cout<<"Writing all FF's...";
		memset(wrBuf, 0xff, len);
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}
		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort

		if (verbose)
			cout<<"Writing all AA's...";
		memset(wrBuf, 0xaa, len);
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}
		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort

		if (verbose)
			cout<<"Writing all 55's...";
		memset(wrBuf, 0x55, len);
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}
		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort


		if (verbose)
			cout<<"Writing random pattern...";
		errs = 0;
		for (i = 0; i < len; i++)
			wrBuf[i] = (uint8_t)rand();
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}

		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort


		if (verbose)
			cout<<"Writing increment pattern...";
		errs = 0;
		for (i = 0; i < len; i++)
			wrBuf[i] = (uint8_t)i;
		this->set(0, len, wrBuf);
		if (verbose)
			cout<<"Verifying...";
		this->get(0, len, rdBuf);
		for (i = 0; i < len; i++)
		{
			if (wrBuf[i] != rdBuf[i])
				++errs;
		}

		if (verbose)
		{
			if (errs == 0)
				cout<<"PASS"<<endl;
			else
				cout<<dec<<errs<<" ERRORS!!!"<<endl;
		}
		if (errs)
			break;	// abort

		done = true;
	}

	delete wrBuf;
	delete rdBuf;

	if (errs)
		return(false);
	else
		return(true);
}





/**
 * Fill memory with contents from a file.
 * A file, up to memory length bytes, is read in and written
 * into the memory device.  Writing always starts at byte offset 0 and continues
 * for the size of the file or the maximum length of the memory, whichever is smaller.
 * 
 * @param fName string containing file name (standard POSIX file name)
 * @return true if file loaded into memory; false if errors opening, reading file
 */
bool Mem::loadFromFile(char *fName)
{
	size_t flen;
	FILE *fp;
	bool verbose = RUNTIMECTRL(VERBOSE);

	// Allocate buffer for memory clearing
	uint8_t *wrBuf = new uint8_t[this->len];

	if (verbose)
		cout<<"===  File-->EBR  ===\n";
	fp = fopen(fName, "rb");
	if (fp == NULL)
	{
		if (verbose)
			cout<<"ERROR! Can't open file: "<<fName<<endl;
		delete wrBuf;
		return(false);
	}
	flen = fread(wrBuf, 1, this->len, fp);	// load data into write buffer
	if ((flen == 0) || (flen == (size_t)EOF))
	{
		fclose(fp);
		if (verbose)
			cout<<"ERROR! Reading file: "<<fName<<endl;
		delete wrBuf;
		return(false);
	}
	this->set(0, flen, wrBuf);	  // store file data into EBR
	if (verbose)
		cout<<fName<<": "<<dec<<flen<<" bytes loaded into EBR."<<endl;
	fclose(fp);

	delete wrBuf;
	return(true);
}



/**
 * Write EBR memory contents to a file.
 * A file of EBR_SIZE bytes is written with the bytes in EBR.
 * Writing always starts from byte offset 0 and continues
 * for the size of the EBR_SIZE.
 */
bool Mem::saveToFile(char *fName)
{
	size_t n;
	FILE *fp;
	bool ret = true;
	bool verbose = RUNTIMECTRL(VERBOSE);

	// Allocate buffer for memory clearing
	uint8_t *rdBuf = new uint8_t[this->len];

	if (verbose)
		cout<<"===  EBR-->File  ===\n";
	fp = fopen(fName, "wb");
	if (fp == NULL)
	{
		if (verbose)
			cout<<"ERROR! Can't open file: "<<fName<<endl;
		return(false);
	}

	this->get(0, this->len, rdBuf);	   // get what is in EBR memory
	n = fwrite(rdBuf, 1, this->len, fp);	 // store data from read buffer
	if (n != this->len)
	{
		if (verbose)
			cout<<"ERROR! Writing file: "<<fName<<endl;
		ret = false;
	}
	else
	{
		if (verbose)
			cout<<"EBR: "<<dec<<n<<" bytes loaded into "<<fName<<endl;
	}
	fclose(fp);
	delete rdBuf;
	return(ret);
}



/**
 * Load a block of bytes into the memory.
 * Max size is determined by the length specified when creating the object.
 * @param offset byte location to start writing at
 * @param num number of bytes to write into memory from buf
 * @param buf location of user's data to write into memory
 * @return true if successful; false if number to write is greater than memory size
 */
bool Mem::set(uint32_t offset, size_t num, void *buf)
{
	if (num > this->len)
		return(false);

	switch (accessWidth)
	{
			case 8:
				this->write8(offset, (uint8_t *)buf, num);
				break;

			case 16:
				this->write16(offset, (uint16_t *)buf, num/2);
				break;

			case 32:
				this->write32(offset, (uint32_t *)buf, num/4);
				break;

			default:
				ERRORSTR("Mem::set: Invalid accessSize");
				return(false);	 // bad size
				break;
	}

	return(true);
}

/**
 * Read a block of bytes from memory into a users buffer.
 * Max size is determined by the length specified when creating the object.
 * 
 * @param offset byte location to start writing at
 * @param num number of bytes to read from memory into buf
 * @param buf location of storage for data read from device memory
 * @return true if successful; false if number to read is greater than memory size
 */
bool Mem::get(uint32_t offset, size_t num, void *buf)
{

	if (num > this->len)
		return(false);

	switch (accessWidth)
	{
			case 8:
				this->read8(offset, (uint8_t *)buf, num);
				break;

			case 16:
				this->read16(offset, (uint16_t *)buf, num/2);
				break;

			case 32:
				this->read32(offset, (uint32_t *)buf, num/4);
				break;

			default:
				ERRORSTR("Mem::get: Invalid accessSize");
				return(false);	 // bad size
				break;
	}

	return(true);
}

