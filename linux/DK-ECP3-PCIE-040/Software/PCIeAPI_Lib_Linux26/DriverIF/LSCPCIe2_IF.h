/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file LSCPCIe2_IF.h */
 
#ifndef LATTICE_SEMI_LSCPCIE2_IF_H
#define LATTICE_SEMI_LSCPCIE2_IF_H


#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

#include "dllDef.h"

// The driver
#include "lscpcie2/Ioctl.h"
#include "PCIe_IF.h"

namespace LatticeSemi_PCIe
{


#define SYS_MEM_BUF_SIZE 16384  // 16kB buffer
#define DMA_MEM_BUF_SIZE 16384  // 16kB buffer

#ifndef SYS_MEM_BUF_SIZE 
#define SYS_MEM_BUF_SIZE 16384  // 16kB buffer
#endif

#ifndef SYS_DMA_BUF_SIZE 
#define SYS_DMA_BUF_SIZE 16384  // 16kB buffer
#endif

#define WB(a) (a & 0x0fffffff)   // convert the system (PC) resource addr to the board's local Address


/**
 * Interface to the Lattice Windows WDM lscpcie2 device driver.
* The lscpcie2 driver provides the following features:
* <ul>
* <li> basic read/write access (through PCIe_IF)
* <li> INTx or MSI interrupt support
* <li> Common Buffer DMA memory allocation, 16kB w/ read/write access from user space
* <li> Extra driver information
* </ul>
 */
class DLLIMPORT LSCPCIe2_IF : public LatticeSemi_PCIe::PCIe_IF
{
public:


    LSCPCIe2_IF	(const char *pBoardID, const char *pDemoID, uint32_t devNum = 1 );
    ~LSCPCIe2_IF();


	bool getPciDriverExtraInfo(const ExtraResourceInfo_t **pExtra);
	bool getPCIExtraInfoStr(string &outs);

	bool writeSysDmaBuf(uint32_t *pData, size_t len);
	bool readSysDmaBuf(uint32_t *pData, size_t len);

protected:
	ExtraResourceInfo_t PCIExtraInfo;

private:
	uint32_t drvrID;
	uint32_t devNum;
};

} //END_NAMESPACE

#endif

