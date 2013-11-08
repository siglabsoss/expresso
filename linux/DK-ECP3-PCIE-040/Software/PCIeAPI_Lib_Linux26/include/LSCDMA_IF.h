/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file LSCDMA_IF.h */
 
#ifndef LATTICE_SEMI_LSCDMA_IF_H
#define LATTICE_SEMI_LSCDMA_IF_H

#include <stdint.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <linux/types.h>

#include "dllDef.h"

// The driver and Hardware Details
#include "lscdma/Ioctl.h"
#include "PCIe_IF.h"

namespace LatticeSemi_PCIe
{



/**
 * Interface to the Lattice Windows WDM lscdma device driver.
* The lscdma driver provides the following features:
* <ul>
* <li> basic register read/write access (through PCIe_IF)
* <li> INTx or MSI interrupt support (ISR)
* <li> SGDMA IP core configuration, 1 RD chan, 1 WR chan.
* <li> Common Buffer DMA memory allocation, 64kB w/ read/write access from user space
* <li> Extra driver information
* </ul>
 */
class DLLIMPORT LSCDMA_IF : public LatticeSemi_PCIe::PCIe_IF
{
public:


    LSCDMA_IF(const char *pBoardID, const char *chan, uint32_t devNum = 1 );
    ~LSCDMA_IF();


	bool getDriverDMAInfo(const DMAResourceInfo_t **pExtra);
	bool getDriverDMAInfoStr(string &outs);

	size_t getSysDmaBufSize(void);
	bool writeSysDmaBuf(uint32_t *pData, size_t len=0);
	bool readSysDmaBuf(uint32_t *pData, size_t len=0);

	bool ReadFromDevice(void *pBuf, size_t len);
	bool WriteToDevice(void *pBuf, size_t len);

protected:
	DMAResourceInfo_t DMAInfo;

private:
	uint32_t drvrID;
	uint32_t devNum;
};

} //END_NAMESPACE

#endif

