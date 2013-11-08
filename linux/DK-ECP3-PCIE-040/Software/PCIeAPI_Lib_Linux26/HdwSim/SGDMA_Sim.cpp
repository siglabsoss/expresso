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
#include "SGDMA_Sim.h"


using namespace LatticeSemi_PCIe;

/**
 * Create a simulated SGDMA IP device.
 * SG-DMA IP has a max window of 8kB.  The actual usable addresses are the first 0x1400.
 * Pass these parameters to the base class constuctor.
 * @param nameStr pointer to the char string naming this device
 * @param addr hardware address of the device (may not be needed)
 */
SGDMA_Sim::SGDMA_Sim( const char *nameStr, uint32_t base) : SimHdw(nameStr, base, 0xffffe000, 0x1400)
{
	ENTER();

	DEBUGPRINT(("SGDMA device: %s created.", nameStr));

	this->memArray[0] = 0x1204dddd;  // IPID = Vendor ID + made up IP ID
	this->memArray[1] = 0x0101f303;  // IPVER = major+ minor + 16 chans + 4 sub-chans + cap
	this->memArray[2] = 0xffff0000;  // GCONTROL = mask all DMA req, all channels disabled
	this->memArray[3] = 0x00000000;  // GSTATUS = 
	this->memArray[4] = 0xffff0000;  // GEVENT = event masks initialize to masked (all 1's)
	this->memArray[5] = 0xffff0000;  // GERROR =  error masks initialize to masked (all 1's)
	this->memArray[6] = 0x00000000;  // GARBITER = 
	this->memArray[7] = 0x00000000;  // GAUX = 

}


/**
 * Delete a hardware device.
 */
SGDMA_Sim::~SGDMA_Sim()
{
	ENTER();
}


