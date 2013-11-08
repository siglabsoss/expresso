/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Device.cpp */

using namespace std;

#include "Device.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a base hardware device.
 * Assign values to base class attributes such as its name, base address and range.
 * This class is typically inheritted by a specific hardware device that provides the
 * parameteres in its constructor, such as an IP module that has 64 bytes of registers.
 *
 * @param pReg  pointer to the specific RegisterAccess object used by the driver interface
 * @param nameStr pointer to the char string naming this device, such as "GPIO", "EBR"
 * @param addr base address the device registers begin at
 * @param range size of valid address window (in bytes). Used to check read/write accesses
 */
Device::Device(RegisterAccess *pReg,
		   const char *nameStr, 
		   uint32_t addr, 
		   uint32_t range) 
{
	this->pReg = pReg;
	this->name = nameStr;
	this->baseAddress = addr;
	this->addrLimit = range - 1;
	this->addrRange = range;


}


/**
 * Delete a device.
 * No real operations done at this base class level.
 */
Device::~Device()
{
}


/**
 * Return the name of the device.
 * @return C char string of the name
 */
const char *Device::getName(void)
{
	return(name.c_str());
}


/**
 * Return the address length of the device.
 * For example if you created a memory device, this parameter
 * would hold the overall size of the memory (16KB) that was
 * passed in the constructor.
 * @return address range set in constructor
 */
uint32_t Device::getRange(void) 
{ 
	return(addrRange);
}



/*===========================================================*/
/*===========================================================*/
/*           Single Access Methods                            */
/*===========================================================*/
/*===========================================================*/

/**
 * Invoke Driver Interface RegisterAccess method to read an 8 bit word from
 * an offset within this device's address space.
 * @param offset register address in this device
 * @return returns the 8 bit value read from offset
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess.
 */
uint8_t Device::read8(uint32_t offset)
{
	uint8_t val;
	 if (offset > addrLimit)
		 throw(DeviceError("Device::read8() range error!"));
	 val = pReg->read8(baseAddress + offset);

	 return(val);
}


/**
 * Invoke Driver Interface RegisterAccess method to write an 8 bit word to
 * an offset within this device's address space.
 * @param offset register address in this device
 * @param val value to write into the register
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write8(uint32_t offset, uint8_t val)
{
	if (offset > addrLimit) 
		throw(DeviceError("Device::write8() range error!"));
	pReg->write8(baseAddress + offset, val);
}


/**
 * Invoke Driver Interface RegisterAccess method to read a 16 bit word from
 * an offset within this device's address space.
 * @param offset register address in this device
 * @return returns the 16 bit value read from offset
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
uint16_t Device::read16(uint32_t offset)
{
	uint16_t val;
	if (offset > addrLimit)
		throw(DeviceError("Device::read16() range error!"));
	val = pReg->read16(baseAddress + offset);
	return(val);
}


/**
 * Invoke Driver Interface RegisterAccess method to write a 16 bit word to
 * an offset within this device's address space.
 * @param offset register address in this device
 * @param val value to write into the register
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write16(uint32_t offset, uint16_t val)
{
	if (offset > addrLimit) 
		throw(DeviceError("Device::write16() range error!"));
	pReg->write16(baseAddress + offset, val);
}


/**
 * Invoke Driver Interface RegisterAccess method to read a 32 bit word from
 * an offset within this device's address space.
 * @param offset register address in this device
 * @return returns the 8 bit value read from offset
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
uint32_t Device::read32(uint32_t offset) 
{
	uint32_t val; 
	if (offset > addrLimit) 
		throw(DeviceError("Device::read32() range error!"));
	val = pReg->read32(baseAddress + offset);
	return(val);
}


/**
 * Invoke Driver Interface RegisterAccess method to write a 32 bit word to
 * an offset within this device's address space.
 * @param offset register address in this device
 * @param val value to write into the register
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write32(uint32_t offset, uint32_t val)
{ 
	if (offset > addrLimit) 
		throw(DeviceError("Device::write32() range error!"));
	pReg->write32(baseAddress + offset, val);
}



/*===========================================================*/
/*===========================================================*/
/*           Block Access Methods                            */
/*===========================================================*/
/*===========================================================*/

/**
 * Invoke Driver Interface RegisterAccess method to read a block of 8 bit words from
 * an offset within this device's address space.
 * @param offset memory address to start reading from
 * @param val pointer to where to store bytes read from device
 * @param len number of bytes to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::read8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	 if (offset + len > addrRange) 
		 throw(DeviceError("Device::block_read8() range error!"));
	 if (!(pReg->read8(baseAddress + offset, val, len, incAddr)))
		 throw(DeviceError("Device::block_read8() RegisterAccess error!"));
}


/**
 * Invoke Driver Interface RegisterAccess method to write a block of 8 bit words to
 * an offset within this device's address space.
 * @param offset memory address to start writing at
 * @param val pointer to the bytes to be stored in the device
 * @param len number of bytes to write
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	if (offset + len > addrRange) 
		throw(DeviceError("Device::block_write8() range error!"));
	if (!(pReg->write8(baseAddress + offset, val, len, incAddr)))
		throw(DeviceError("Device::block_write8() RegisterAccess error!"));
}


/**
 * Invoke Driver Interface RegisterAccess method to read a block of 16 bit words from
 * an offset within this device's address space.
 * @param offset memory address to start reading from
 * @param val pointer to where to store data read from the device
 * @param len number of 16 bit words to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::read16(uint32_t offset, uint16_t *val, size_t len, bool incAddr) 
{
	if (offset + len > addrRange) 
		throw(DeviceError("Device::block_read16() range error!"));
	if (!(pReg->read16(baseAddress + offset, val, len, incAddr)))
		throw(DeviceError("Device::block_read16() RegisterAccess error!"));
}


/**
 * Invoke Driver Interface RegisterAccess method to write a block of 16 bit words to
 * an offset within this device's address space.
 * @param offset memory address to start writing at
 * @param val pointer to the data to be stored in the device
 * @param len number of words to write (not bytes)
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write16(uint32_t offset, uint16_t *val, size_t len, bool incAddr)
{
	if (offset + len > addrRange) 
		throw(DeviceError("Device::block_write16() range error!"));
	if (!(pReg->write16(baseAddress + offset, val, len, incAddr)))
		throw(DeviceError("Device::block_write16() RegisterAccess error!"));
}


/**
 * Invoke Driver Interface RegisterAccess method to read a block of 32 bit words from
 * an offset within this device's address space.
 * @param offset memory address to start reading from
 * @param val pointer to where to store data read from the device
 * @param len number of 32 bit words to read
 * @param incAddr optional param, (default)true=read noraml RAM, false=read FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::read32(uint32_t offset, uint32_t *val, size_t len, bool incAddr)
{ 
	if (offset + len > addrRange) 
		throw(DeviceError("Device::block_read32() range error!"));
	if (!(pReg->read32(baseAddress + offset, val, len, incAddr)))
		throw(DeviceError("Device::block_read32() RegisterAccess error!"));
}


/**
 * Invoke Driver Interface RegisterAccess method to write a block of 32 bit words to
 * an offset within this device's address space.
 * @param offset memory address to start writing at
 * @param val pointer to the data to be stored in the device
 * @param len number of words to write (not bytes)
 * @param incAddr optional param, (default)true=write noraml RAM, false=write FIFO
 * @exception throws DeviceError if address out of range or error
 * in RegisterAccess device driver.
 */
void Device::write32(uint32_t offset, uint32_t *val, size_t len, bool incAddr) 
{ 
	if (offset + len > addrRange) 
		throw(DeviceError("Device::block_write32() range error!"));
	if (!(pReg->write32(baseAddress + offset, val, len, incAddr)))
		throw(DeviceError("Device::block_write32() RegisterAccess error!"));
}


