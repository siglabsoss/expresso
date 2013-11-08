/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Device.h */
 
#ifndef LATTICE_SEMI_DEVICE_H
#define LATTICE_SEMI_DEVICE_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

#include <string>
#include <exception>


#include "dllDef.h"

#include "RegisterAccess.h"


/*
 * Lattice Semiconductor Corp. namespace for LatticeSC device.
 */
namespace LatticeSemi_PCIe
{


/** 
 * Defintion of a generic Platform Device.
 * This is a base class that describes the basic properties that all hardware
 * devices have.  Devices can be read and written and may have a name.
 * This class is inherited by a specific device class and tailored
 * to the specific hardware/IP device. In particular the new device class will
 * set the range of addresses, the base address, the name (for debug purposes).
 * <p>
 * The most important feature of this base class is the hardware access method
 * through the RegisterAccess class.  This is typically a previously instantiated
 * DriverInterface object that provides the true read/write access methods to the
 * hardware. The Device read/write methods use the assigned RegisterAccess class to actually 
 * do the register access.  Checks are made to ensure the accesses remain within the bounds
 * of the address range specified during device creation.  Checks are also done in the
 * RegsiterAccess to ensure access are aligned.  Any error results in throwing an exception.
 * Exceptions are used to notify an access failure (param check or driver error).
 * Higher level code should use a try{ read8(); } catch(std::exception &e){cout<<e.what();}
 * to catch an exception and display and handle the problem.
 * <p>
 * Derived classes of specific IP device drivers may over-ride any access method that
 * is not supported and throw an exception.  An example would be IP that is only 32 bit
 * accessable and does not allow read/write 8/16.  
 */

class DLLIMPORT Device
{
public:
	Device(LatticeSemi_PCIe::RegisterAccess *pReg,
		   const char *nameStr,
		   uint32_t addr,
		   uint32_t range);
	virtual ~Device();
	const char *getName(void);

	/* Pass the register read/write operation to the AccessLayer object, but add in the device's
	 * specific base address first.
	 */
		uint8_t  read8(uint32_t offset);
		void     write8(uint32_t offset, uint8_t val);
		uint16_t read16(uint32_t offset);
		void     write16(uint32_t offset, uint16_t val);
		uint32_t read32(uint32_t offset);
		void 	 write32(uint32_t offset, uint32_t val);

		/* Block Access Methods */
		void read8(uint32_t offset, uint8_t *val, size_t len, bool incAddr=true);
		void write8(uint32_t offset, uint8_t *val, size_t len, bool incAddr=true);
		void read16(uint32_t offset, uint16_t *val, size_t len, bool incAddr=true);
		void write16(uint32_t offset, uint16_t *val, size_t len, bool incAddr=true);
		void read32(uint32_t offset, uint32_t *val, size_t len, bool incAddr=true);
		void write32(uint32_t offset, uint32_t *val, size_t len, bool incAddr=true);

		uint32_t getRange(void);


protected:
	LatticeSemi_PCIe::RegisterAccess 	*pReg;
	uint32_t       			baseAddress;
	uint32_t       			addrLimit;
	uint32_t       			addrRange;
	string	       			name;

};




/**
 * Exception class for objects of Lattice PCIe Device type.
 * This type of exception is thrown if an occurs deep in the hardware access
 * code and execution can not continue.  The top-level
 * code can determine the exception cause by displaying the
 * char string returned by the what() method.
 */
class DLLIMPORT DeviceError : public exception 
{ 
   public:
       DeviceError(char *s) {msg = s;}
       virtual const char *what(void) const throw()
       {
           return(msg);
       }

   private:
       char *msg;
};

} // END_NAMESPACE

#endif
