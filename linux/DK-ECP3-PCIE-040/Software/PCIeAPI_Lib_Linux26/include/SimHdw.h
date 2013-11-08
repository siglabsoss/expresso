/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

 
#ifndef LATTICE_SEMI_SIMHDW_H
#define LATTICE_SEMI_SIMHDW_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <string>
#include <exception>


/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{


/** 
 * Defintion of a Simulator
 */

class DLLIMPORT SimHdw
{
public:
	SimHdw(const char *nameStr, uint32_t base, uint32_t mask, uint32_t size);
	virtual ~SimHdw();
	const char *getName(void);

	bool	addrMatch(uint32_t addr);

	uint8_t  read8(uint32_t busAddr);
	void     write8(uint32_t busAddr, uint8_t val);
	uint16_t read16(uint32_t busAddr);
	void     write16(uint32_t busAddr, uint16_t val);
	uint32_t read32(uint32_t busAddr);
	void 	 write32(uint32_t busAddr, uint32_t val);

	/* Block Access Methods */
	void read8(uint32_t busAddr, uint8_t *val, size_t len, bool incAddr=true);
	void write8(uint32_t busAddr, uint8_t *val, size_t len, bool incAddr=true);
	void read16(uint32_t busAddr, uint16_t *val, size_t len, bool incAddr=true);
	void write16(uint32_t busAddr, uint16_t *val, size_t len, bool incAddr=true);
	void read32(uint32_t busAddr, uint32_t *val, size_t len, bool incAddr=true);
	void write32(uint32_t busAddr, uint32_t *val, size_t len, bool incAddr=true);

protected:
	uint32_t    baseAddress;    // bus address of start of device's address window
	uint32_t    addrLimit;      // address of last register in window, maybe less than mask
	uint32_t    addrMask;       // mask valid bus address bits.  used to detect device chip select
	uint32_t    localMask;      // convert bus address to just local relative addressing
	uint32_t	size;           // number of registers
	string	    name;

	uint32_t	*memArray;
	uint8_t		*pMem8;
	uint16_t	*pMem16;
	uint32_t	*pMem32;

	uint8_t		intNum;
};
/**
 * Simulated Hardware Exception.
 * Thrown by class methods in the simulated hardware module when un-recoverable
 * hardware errors or system API failures have occurred and execution
 * can not continue safely.
 */
class DLLIMPORT SimHdwError : public exception 
{ 
   public:
       SimHdwError(char *s) {msg = s;}
       virtual const char *what(void) const throw()
       {
           return(msg);
       }

   private:
       char *msg;
};

} //END_NAMESPACE

#endif
