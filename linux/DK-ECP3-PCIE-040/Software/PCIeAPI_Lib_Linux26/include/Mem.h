/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Mem.h */

 
#ifndef LATTICE_SEMI_PCIE_MEM_H
#define LATTICE_SEMI_PCIE_MEM_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <time.h>
#include <exception>

#include "dllDef.h"
#include "Device.h"

/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{

/**
 * Specific definition of a Lattice memory device IP module.
 *
 * This class inherits hardware access from the generic Device class.
 * The main purpose of this class is to provide utility methods that
 * operate on a linear memory array in hardware to do such operations
 * as clearing memory, filling with a pattern, reading a block of
 * memory, etc.  All of which are primarily used in testing routines
 * to verify memory access and memory contents after a transfer.
 */
class DLLIMPORT Mem : public LatticeSemi_PCIe::Device
{
public:
	Mem(const char *nameStr, 
	    size_t size, 
	    uint32_t addr, 
	    LatticeSemi_PCIe::RegisterAccess *pRA);
	~Mem();

	bool clear(void);
  	bool loadFromFile(char *fName);
	bool saveToFile(char *fName);
	bool test(size_t len=0);
	bool fill(uint8_t val);
	bool fill(uint8_t val, uint32_t offset, size_t len);
	bool fillPattern(uint32_t val);
	bool fillPattern(uint32_t val, uint32_t offset, size_t flen);
	bool get(uint32_t offset, size_t num, void *buf);
	bool set(uint32_t offset, size_t num, void *buf);
	size_t getLen(void);
	void setAccessWidth(uint8_t dataSize);

private:
	void	*arg;
	size_t len;
	uint8_t accessWidth;

};

} //END_NAMESPACE

#endif
