/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */


using namespace std;
#include <iostream>
#include <string>

#include "DebugPrint.h"
#include "Mem_Sim.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a simulated hardware device.
 * Assign values to base class attributes.
 * @param nameStr pointer to the char string naming this device
 * @param base start hardware address of the device (may not be needed)
 * @param mask valid address bits to decode
 * @param range size of valid address window (in bytes). Used to check read/write accesses
 */
Mem_Sim::Mem_Sim( const char *nameStr, uint32_t base, uint32_t mask, uint32_t size) : SimHdw(nameStr, base, mask, size)
{
	ENTER();

	DEBUGPRINT(("Memory device: %s created. Base: 0x%x  Size: 0x%x\n", nameStr, base, size));
}


/**
 * Delete a hardware device.
 */
Mem_Sim::~Mem_Sim()
{
	ENTER();
}


