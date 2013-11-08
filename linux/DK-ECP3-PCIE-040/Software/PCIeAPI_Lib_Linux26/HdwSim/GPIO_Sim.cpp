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
#include "GPIO_Sim.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a simulated GPIO hardware device.
 * Fixed address size of 64 bytes, so mask can be set here.  Not programmable.
 * @param nameStr pointer to the char string naming this device
 * @param addr hardware address of the device (may not be needed)
 */
GPIO_Sim::GPIO_Sim(const char *nameStr, uint32_t base) : SimHdw(nameStr, base, 0xffffffc0, 0x40)
{

	ENTER();

	DEBUGPRINT(("GPIO device: %s created.\n", nameStr));

	this->memArray[0] = 0x53030100;  // Demo ID register
	this->memArray[1] = 0;   // scratch pad
	this->memArray[2] = 0xaa000000;  // set DIP switch to AA for test
	this->memArray[4] = 0x44332211;  // set current count for fun
	this->memArray[5] = 0x7fffffff;  // set reload for fun
}


/**
 * Delete a hardware device.
 */
GPIO_Sim::~GPIO_Sim()
{
	LEAVE();
}


