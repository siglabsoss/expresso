/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/*
 * Template for a simulated Lattice FPGA Demo IP design in software.
 * This class simulates access to hardware IP devices.
 * All access are done to memory-resident objects within this program.
 * No actual bus transfers are done.
 * Devices are not created in this file, but created and added in the
 * layer above.
 */

using namespace std;

#include <iostream>
#include <string>
#include <exception>


#include "DebugPrint.h"
#include "SimDemoIP.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a simulated instance of the SGDMA IP in the FPGA.
 * Everything is just arrays in user space in this program.
 * No real hardware is accessed and no driver is used.
 * @param freeOnClose set to true if you want the device objects freed when destructor called
 * flase if you wnat to do it yourself back in the app
 */
SimDemoIP::SimDemoIP(bool freeOnClose)
{

	ENTER();

	// Initialize list to empty
	for (int i = 0; i < MAX_HDW_DEVICES; i++)
	{
		this->DevList[i] = NULL;
	}
	numHdwDevs = 0;

	FreeDevicesOnClose = freeOnClose; 
}


/**
 * Delete an instance of a register access object.
 */
SimDemoIP::~SimDemoIP()
{
	// Delete all the simulated hardware devices that were created

	int i;

	ENTER();
	
	if (FreeDevicesOnClose)
	{
	    for (i = 0; i < this->numHdwDevs; i++)
	    {
		    if (this->DevList[i] != NULL)
		    {
			    delete this->DevList[i];
		    }
	    }
	}

}



/**
 * Add a new deive to the list.
 * When a SimDemoIP object is first created there are no devices associated
 * with it.  The device (i.e. GPIO, Memory, SFIF) need to be created by the
 * app (which knows there address locations) and then added to the Demo
 * simulation using this method.
 * @param pDev pointer to the device to add
 * @return true if added, false if list full
 */
bool SimDemoIP::addDev(LatticeSemi_PCIe::SimHdw *pDev)
{
	ENTER();

	if (numHdwDevs >= MAX_HDW_DEVICES)
	    return(false);

	DevList[numHdwDevs++] = pDev;

	return(true);
}



/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*    READ/WRITE METHODS */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/**
 * Read 8 bits from an LSC FPGA register via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @param val location of storage for data read
 * @return true if read byte OK, false if driver reports error
 */
bool SimDemoIP::read8(uint32_t offset, uint8_t &val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	val = pDev->read8(offset);


	return(true);
}
    

/**
 * Write 8 bits to an LSC hardware register via PCIe bus.
 * @param offset BAR + address of device register to write to
 * @param val value to write into the register
 * @return true if byte written, false if driver had error
 */
bool SimDemoIP::write8(uint32_t offset, uint8_t val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write8(offset, val);

	return(true);
}



/**
 * Read 16 bits from an FPGA register via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @param val location of storage for data read
 * @return true; false if address not multiple of 2
 */
bool SimDemoIP::read16(uint32_t offset, uint16_t &val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	val = pDev->read16(offset);

	return(true);
}


/**
 * Write 16 bits to an SC hardware register via PCIe bus.
 * @param offset address of device register to write to
 * @param val value to write into the register
 * @return true; error in writing will cause hardware exception
 */
bool SimDemoIP::write16(uint32_t offset, uint16_t val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write16(offset, val);

	return(true);
}



/**
 * Read 32 bits from an SC hardware register via PCIe bus.
 * This is done with 2 16 bit reads because the SC900 only has a 16 bit wide
 * data bus and will not allow accesses larger than 16 bits.
 * @param offset address of device register to read from
 * @param val location of storage for data read
 * @return true; false if address not multiple of 4
 * @note error in reading will cause hardware exception
 */
bool SimDemoIP::read32(uint32_t offset, uint32_t &val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	val = pDev->read32(offset);

	return(true);
}
    


/**
 * Write 32 bits to an SC hardware register via PCIe bus.
 * @param offset address of device register to write to
 * @param val value to write into the register
 * @return true; false if address not multiple of 4
 * @note error in reading will cause hardware exception
 */
bool SimDemoIP::write32(uint32_t offset, uint32_t val)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write32(offset, val);

	return(true);
}


/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/

/**
 * Read a block of 8 bit registers from FPGA hardware via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @param val location of storage for data read
 * @param len number of bytes to read
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error if driver fails
 */
bool SimDemoIP::read8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->read8(offset, val, len);

	return(true);
}
    

/**
 * Write a block of 8 bit registers into FPGA hardware via PCIe bus.
 * @param offset address of device register to write to
 * @param val location of bytes to write
 * @param len number of bytes to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error in writing will cause hardware exception
 * @note block size is not limitted
 */
bool SimDemoIP::write8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write8(offset, val, len);

	return(true);
}



/**
 * Read a block of 16 bit registers from SC hardware via PCIe bus.
 * @param offset address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 16 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 * @note error in reading will cause hardware exception
 */
bool SimDemoIP::read16(uint32_t offset, uint16_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();


	if (offset & 0x01)
		return(false);  /* error!  must be multiple of 2 */

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->read16(offset, val, len);

	return(true);
}


/**
 * Write a block of 16 bit registers into FPGA hardware via PCIe bus.
 * @param offset BAR + address of device registers to write to
 * @param val location of 16 bit words to write
 * @param len number of 16 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 * @note block size is  not limitted
 */
bool SimDemoIP::write16(uint32_t offset, uint16_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	if (offset & 0x01)
		return(false);  /* error!  must be multiple of 2 */

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write16(offset, val, len);

	return(true);
}



/**
 * Read a block of 32 bit registers from FPGA hardware via PCIe bus.
 * @param offset BAR + address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 32 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4
 */
bool SimDemoIP::read32(uint32_t offset, uint32_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	if (offset & 0x03)
		return(false);  /* error!  must be multiple of 4 */

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->read32(offset, val, len);

	return(true);
}
    


/**
 * Write a block of 32 bit registers into SC hardware via PCIe bus.
 * @param offset address of device registers to write to
 * @param val location of 32 bit words to write into SC
 * @param len number of 32 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4
 * @note block size is not limitted, user buffer used directly
 */
bool SimDemoIP::write32(uint32_t offset, uint32_t *val, size_t len, bool incAddr)
{
	LatticeSemi_PCIe::SimHdw *pDev;

	ENTER();

	if (offset & 0x03)
		return(false);  /* error!  must be multiple of 4 */

	pDev = this->findDev(offset);
	if (pDev == NULL)
		return(false);   // Bad address

	pDev->write32(offset, val, len);


	return(true);
}



/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*                 P R I V A T E   M E T H O D S                              */
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/

/**
 * Search the list of devices and see who matches this address.
 * Address needs to fall in the device's range.
 * @param addr the "bus" address 
 * @return pointer to the device or NULL if no address match
 */
LatticeSemi_PCIe::SimHdw * SimDemoIP::findDev(uint32_t addr)
{
	int i;

	for (i = 0; i < this->numHdwDevs; i++)
	{
		if (this->DevList[i]->addrMatch(addr))
			return (this->DevList[i]);
	}

	return(NULL);
}

