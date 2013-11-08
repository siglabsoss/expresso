/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */


using namespace std;

#include "DebugPrint.h"
#include "SimHdw.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a simulated hardware device.
 * Assign values to base class attributes.
 * @param nameStr pointer to the char string naming this device
 * @param base hardware bus address of the device
 * @param mask bus address bits to decode, 1=decode, 0=ignore.  Like a BAR. must be power of 2.
 * @param size size of device's address window in bytes can be <= mask address
 */
SimHdw::SimHdw( const char *nameStr, uint32_t base, uint32_t mask, uint32_t size) 
{
	ENTER();

	this->baseAddress = base;
	this->addrLimit = size - 1;
	this->addrMask = mask;
	this->localMask = ~mask;
	this->size = size;
	this->intNum = 0;

	this->memArray = new uint32_t[size];

	this->name = nameStr;

	// Assign the same starting location to each access type.  
	this->pMem8 = (uint8_t *)this->memArray;
	this->pMem16 = (uint16_t *)this->memArray;
	this->pMem32 = (uint32_t *)this->memArray;

}


/**
 * Delete a hardware device.
 */
SimHdw::~SimHdw()
{
	ENTER();

    delete this->memArray;
}


/**
 * Return the name of the device.
 * @return C char string of the name
 */
const char *SimHdw::getName(void)
{
	ENTER();

	return(name.c_str());
}


/**
 * Return true if the address passed in falls within the range of 
 * this hardware device.
 * @param addr the suspected address 
 * @return true if its in range, false if its not for this device
 */
bool SimHdw::addrMatch(uint32_t addr)
{
	ENTER();

	if ((this->addrMask & addr) == this->baseAddress)
		return(true);
	else
		return(false);
}


/*===========================================================*/
/*===========================================================*/
/*           Single Access Methods                            */
/*===========================================================*/
/*===========================================================*/

/**
 * Perform a Platform AccessLayer method to read an 8 bit word from
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @return returns the 8 bit value read from offset
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
uint8_t SimHdw::read8(uint32_t busAddr)
{
	uint8_t val;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
	 if (offset > addrLimit)
		 throw(SimHdwError("SimHdw::read8() range error!"));
	 val = *(this->pMem8 + offset);
	 return(val);
}


/**
 * Perform a Platform AccessLayer method to write an 8 bit word to
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @param val value to write into the register
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write8(uint32_t busAddr, uint8_t val)
{
	ENTER();

	uint32_t offset = busAddr & this->localMask;
	if (offset > addrLimit) 
		throw(SimHdwError("SimHdw::write8() range error!"));
	*(this->pMem8 + offset) = val;
}


/**
 * Perform a Platform AccessLayer method to read a 16 bit word from
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @return returns the 16 bit value read from offset
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
uint16_t SimHdw::read16(uint32_t busAddr)
{
	uint16_t val;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
	if (offset > addrLimit)
		throw(SimHdwError("SimHdw::read16() range error!"));

	offset = offset / 2;
	val = *(this->pMem16 + offset);
	return(val);
}


/**
 * Invoke Platform AccessLayer method to write a 16 bit word to
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @param val value to write into the register
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write16(uint32_t busAddr, uint16_t val)
{
	ENTER();

	uint32_t offset = busAddr & this->localMask;
	if (offset > addrLimit) 
		throw(SimHdwError("SimHdw::write16() range error!"));

	offset = offset / 2;
	*(this->pMem16 + offset) = val;
}


/**
 * Invoke Platform AccessLayer method to read a 32 bit word from
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @return returns the 8 bit value read from offset
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
uint32_t SimHdw::read32(uint32_t busAddr) 
{
	uint32_t val; 
	ENTER();


	uint32_t offset = busAddr & this->localMask;
	 if (offset > addrLimit)
		 throw(SimHdwError("SimHdw::read32() range error!"));

	offset = offset / 4;
	 val = *(this->pMem32 + offset);
	 return(val);
}


/**
 * Invoke Platform AccessLayer method to write a 32 bit word to
 * an offset within this device's address space.
 * @param busAddr register address in this device
 * @param val value to write into the register
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write32(uint32_t busAddr, uint32_t val)
{ 
	ENTER();

	uint32_t offset = busAddr & this->localMask;
	if (offset > addrLimit) 
		throw(SimHdwError("SimHdw::write32() range error!"));
	offset = offset / 4;
	*(this->pMem32 + offset) = val;
}



/*===========================================================*/
/*===========================================================*/
/*           Block Access Methods                            */
/*===========================================================*/
/*===========================================================*/

/**
 * Invoke Platform AccessLayer method to read a block of 8 bit words from
 * an offset within this device's address space.
 * @param busAddr memory address to start reading from
 * @param val pointer to where to store bytes read from device
 * @param len number of bytes to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::read8(uint32_t busAddr, uint8_t *val, size_t len, bool incAddr)
{
    size_t i;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
     if (offset + len - 1 > addrLimit) 
		 throw(SimHdwError("SimHdw::block_read8() range error!"));
     for (i = 0; i < len; i++)
		 *(val + i) = *(this->pMem8 + offset + i);
}


/**
 * Invoke Platform AccessLayer method to write a block of 8 bit words to
 * an offset within this device's address space.
 * @param busAddr memory address to start writing at
 * @param val pointer to the bytes to be stored in the device
 * @param len number of bytes to write
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write8(uint32_t busAddr, uint8_t *val, size_t len, bool incAddr)
{
    size_t i;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
	if (offset + len - 1 > addrLimit) 
		throw(SimHdwError("SimHdw::block_write8() range error!"));
	for (i = 0; i < len; i++)
		*(this->pMem8 + offset + i) = *(val + i);
}


/**
 * Invoke Platform AccessLayer method to read a block of 16 bit words from
 * an offset within this device's address space.
 * @param busAddr memory address to start reading from
 * @param val pointer to where to store data read from the device
 * @param len number of 16 bit words to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::read16(uint32_t busAddr, uint16_t *val, size_t len, bool incAddr) 
{
    size_t i;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
    if (offset + len - 1 > addrLimit) 
		throw(SimHdwError("SimHdw::block_read16() range error!"));
    offset = offset / 2;
    for (i = 0; i < len; i++)
		*(val + i) = *(this->pMem16 + offset + i);
}


/**
 * Invoke Platform AccessLayer method to write a block of 16 bit words to
 * an offset within this device's address space.
 * @param busAddr memory address to start writing at
 * @param val pointer to the data to be stored in the device
 * @param len number of words to write (not bytes)
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write16(uint32_t busAddr, uint16_t *val, size_t len, bool incAddr)
{
    size_t i;
	ENTER();


	uint32_t offset = busAddr & this->localMask;
	if (offset + len - 1 > addrLimit) 
		throw(SimHdwError("SimHdw::block_write16() range error!"));
	offset = offset / 2;
	for (i = 0; i < len; i++)
		*(this->pMem16 + offset + i) = *(val + i);
}


/**
 * Invoke Platform AccessLayer method to read a block of 32 bit words from
 * an offset within this device's address space.
 * @param busAddr memory address to start reading from
 * @param val pointer to where to store data read from the device
 * @param len number of 32 bit words to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::read32(uint32_t busAddr, uint32_t *val, size_t len, bool incAddr)
{ 
    size_t i;

	ENTER();

	uint32_t offset = busAddr & this->localMask;
	if (offset + len - 1 > addrLimit) 
		throw(SimHdwError("SimHdw::block_read32() range error!"));
	offset = offset / 4;
     for (i = 0; i < len; i++)
		 *(val + i) = *(this->pMem32 + offset + i);
}


/**
 * Invoke Platform AccessLayer method to write a block of 32 bit words to
 * an offset within this device's address space.
 * @param busAddr memory address to start writing at
 * @param val pointer to the data to be stored in the device
 * @param len number of words to write (not bytes)
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws SimHdwError if address out of range or error
 * in AccessLayer device driver.
 */
void SimHdw::write32(uint32_t busAddr, uint32_t *val, size_t len, bool incAddr) 
{ 
    size_t i;

	ENTER();

	uint32_t offset = busAddr & this->localMask;
	if (offset + len - 1 > addrLimit) 
		throw(SimHdwError("SimHdw::block_write32() range error!"));
	offset = offset / 4;
	for (i = 0; i < len; i++)
		*(this->pMem32 + offset + i) = *(val + i);
}


