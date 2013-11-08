/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file LSCDMA_IF.cpp */

#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>


using namespace std;

#include "PCIeAPI.h"
#include "LSCDMA_IF.h"

using namespace LatticeSemi_PCIe;



/**
 * Open an interface to the lscdma device driver.
 * The caller must supply the board ID, the DMA channel and the
 * instance number (which board 1st, 2nd, 3rd, etc) to open.
 * 
 * 
 * @param pBoardID string containing PCI DEV_VEND ID of board to attach to
 * @param chan string containing the channel to open, such as "img_mem", NULL = just register access
 * @param devNum instance number of board to attach to (1st, 2nd, ...)
 */
LSCDMA_IF::LSCDMA_IF(const char *pBoardID, const char *chan, uint32_t devNum) :

		LatticeSemi_PCIe::PCIe_IF(PCIe_IF::pLSCDMA_DRIVER,
						   pBoardID, 
						  PCIe_IF::PCIEDMA_DEMO,
						   devNum,
						   NULL,
						   chan)
{
	const DMAResourceInfo_t *pDummy;

	this->devNum = devNum;
	this->drvrID = drvrID;


// TODO
	// We need to create a semaphore to provide exclusive access to the Common Buffer
	// so multiple threads can't step on the buffer during transfers.
	// There's only one common buffer.

	// Call to preload the private class variable with the DMA info from the driver
	// at the start.  The dummy variable is not used.
	getDriverDMAInfo(&pDummy);


}


/**
 * Delete an instance of an SGDMA interface object.
 */
LSCDMA_IF::~LSCDMA_IF()
{
	cout<<"LSCDMA_IF: driver closed."<<endl;
}



/**
 * Return the size of the kernel buffer allocated for Common Buffer DMA
 * type transfers.  
 * The value returned is from the driver info obtained by an IOCTL call during
 * class creation.
 * @return the size is in bytes
 */
size_t LSCDMA_IF::getSysDmaBufSize(void)
{
	return(this->DMAInfo.DmaBufSize);
}


/**
 * Return the extra device driver information structure.
 * This includes the DMA memory buffer info, PCI bus/dev/func address.
 * @param pInfo user's pointer that will point to the internal driver structure
 * @return true (info obtained in constructor so always present)
 * @note Do not modify the contents of the structure!  It is read only.
 */
bool LSCDMA_IF::getDriverDMAInfo(const DMAResourceInfo_t **pInfo)
{

	if (ioctl(hDev, IOCTL_LSCDMA_GET_DMA_INFO, &DMAInfo) != 0)
		throw(PCIe_IF_Error("LSCDMA_IF: getDriverDMAInfo ioctl Failed!"));


	*pInfo = &DMAInfo;   // return pointer to the structure

	return(true);
}

/**
 * Write data for user space into the system DMA buffer for access by 
 * the device.
 * @param pBuf pointer to user's data
 * @param len the number of bytes (not DWs)
 * @return 
 */
bool LSCDMA_IF::writeSysDmaBuf(uint32_t *pBuf, size_t len) 
{
	uint32_t *tempBuf;

	if (!DMAInfo.hasDmaBuf)
		return(false);

	if (len > DMAInfo.DmaBufSize)
		return(false);
	if (len == 0)
		len = DMAInfo.DmaBufSize;

	tempBuf = (uint32_t *)malloc(DMAInfo.DmaBufSize);
	if (tempBuf == NULL)
		return(false);

	memset(tempBuf, 0, DMAInfo.DmaBufSize);
	memcpy(tempBuf, pBuf, len);

// TODO : take the semaphore
	if (ioctl(hDev, IOCTL_LSCDMA_WRITE_SYSDMABUF, tempBuf) != 0)
	{
// TODO : release the semaphore before aborting
		free(tempBuf);
		throw(PCIe_IF_Error("LSCDMA_IF: IOCTL_LSCDMA_WRITE_SYSDMABUF failed!"));
	}


// TODO : release the semaphore


	free(tempBuf);
	return(true);
}


/**
 * Read data from the system DMA buffer into the user space.
 * @param pBuf pointer to where to store device data in user space
 * @param len the number of bytes to read (not DWs)
 * @return 
 */
bool LSCDMA_IF::readSysDmaBuf(uint32_t *pBuf, size_t len) 
{
	uint32_t *tempBuf;

	if (!DMAInfo.hasDmaBuf)
		return(false);

	if (len > DMAInfo.DmaBufSize)
		return(false);
	if (len == 0)
		len = DMAInfo.DmaBufSize;   // but pBuf needs to be the right length!!!

	tempBuf = (uint32_t *)malloc(DMAInfo.DmaBufSize);
	if (tempBuf == NULL)
		return(false);

	memset(tempBuf, 0, DMAInfo.DmaBufSize);

// TODO : take the semaphore
	if (ioctl(hDev, IOCTL_LSCDMA_READ_SYSDMABUF, tempBuf) != 0)
	{
// TODO : release the semaphore before aborting
		free(tempBuf);
		throw(PCIe_IF_Error("LSCDMA_IF: IOCTL_LSCDMA_READ_SYSDMABUF failed!"));
	}


	memcpy(pBuf, tempBuf, len);
	free(tempBuf);

// TODO : release the semaphore


	return(true);
}


/**
 * Read a number of bytes from the SGDMA channel (device). 
 * Uses the OS API ReadFile() to invoke the driver and cause the
 * necessary parameters to be passed to the driver.  This call blocks
 * until the driver has moved all requested data.  It operates like a
 * normal file read operation.
 * 
 * @param pBuf where to store data DMA'ed from device
 * @param len number of bytes to transfer in DMA operation
 *
 */
bool LSCDMA_IF::ReadFromDevice(void *pBuf, size_t len)
{
	ssize_t retLen;

	retLen = read(this->hDev, 
			pBuf, 
			len);

	if ((size_t)retLen != len)
		return(false);
	else
		return(true);
}



/**
 * Write a number of bytes to the SGDMA channel (device). 
 * Uses the OS API WriteFile() to invoke the driver and cause the
 * necessary parameters to be passed to the driver.  This call blocks
 * until the driver has moved all requested data.  It operates like a
 * normal file write operation.
 * 
 * @param pBuf where to get the data to DMA to the device
 * @param len number of bytes to transfer in DMA operation
 
 */
bool LSCDMA_IF::WriteToDevice(void *pBuf, size_t len)
{
	ssize_t retLen;

 	retLen = write(this->hDev,
			  pBuf,
			  len);

	if ((size_t)retLen != len)
		return(false);
	else
		return(true);

	return(true);
}


/**
 * Return a string containing additional information (beyond that provided
 * by PCIe_IF) about the driver and device, such as the device bus, function
 * and device numbers, the DMA buffer addresses, etc.
 * @see DMAResourceInfo_t 
 * @return true if successful; false if error
 */
bool LSCDMA_IF::getDriverDMAInfoStr(string &outs)
{


	const DMAResourceInfo_t *pInfo;
	getDriverDMAInfo(&pInfo);

	std::ostringstream oss;

	oss<<"\nlscdma Driver DMA Info:\n";
	oss<<"DevID="<<pInfo->devID<<endl;
	oss<<"PCI Bus#="<<pInfo->busNum<<endl;
	oss<<"PCI Dev#="<<pInfo->deviceNum<<endl;
	oss<<"PCI Func#="<<pInfo->functionNum<<endl;
	oss<<"UINumber="<<pInfo->UINumber<<endl;
	oss<<"hasDMA="<<pInfo->hasDmaBuf<<endl;
	oss<<"DmaBufSize="<<pInfo->DmaBufSize<<endl;
	oss<<"DmaAddr64="<<pInfo->DmaAddr64<<endl;
	oss<<"DmaPhyAddrHi="<<hex<<pInfo->DmaPhyAddrHi<<endl;
	oss<<"DmaPhyAddrLo="<<pInfo->DmaPhyAddrLo<<dec<<endl;


	if (pInfo->DmaPhyAddrLo % 4096)
	{
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		oss<<"WARNING!  Base is not 4kB aligned.\n";
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	}

	if (pInfo->DmaPhyAddrLo % 128)
	{
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		oss<<"WARNING!  Base is not 128 aligned.\n";
		oss<<"Writes need to account for crossing 4k boundary\n";
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	}

	outs = oss.str();

	return(true);
}


