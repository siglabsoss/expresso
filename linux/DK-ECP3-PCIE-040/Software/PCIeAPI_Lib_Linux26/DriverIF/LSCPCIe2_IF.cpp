/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file LSCPCIe2_IF.cpp */

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
#include "LSCPCIe2_IF.h"

using namespace LatticeSemi_PCIe;



/**
 * Open an interface to the lscpcie2 device driver.
 * The caller must supply the board ID, demo ID (such as the SFIF) and the
 * instance number (which board 1st, 2nd, 3rd, etc) to open.
 * 
 * @param pBoardID string containing PCI DEV_VEND ID of board to attach to
 * @param pDemoID string containing PCI SUBSYS ID describing particular demo IP
 * @param devNum instance number of board to attach to (1st, 2nd, ...)
 */
LSCPCIe2_IF::LSCPCIe2_IF(const char *pBoardID, const char *pDemoID, uint32_t devNum) :

		LatticeSemi_PCIe::PCIe_IF(PCIe_IF::pLSCPCIE2_DRIVER, pBoardID, pDemoID, devNum, NULL, NULL)
{

	this->devNum = devNum;
	this->drvrID = drvrID;

}


/**
 * Delete an instance of a register access object.
 */
LSCPCIe2_IF::~LSCPCIe2_IF()
{
	cout<<"LSCPCIe2_IF: driver closed."<<endl;
}

/**
 * Return the extra device driver information structure.
 * This includes the DMA memory buffer info, PCI bus/dev/func address.
 * @param pExtra user's pointer that will point to the internal driver structure
 * @return true (info obtained in constructor so always present)
 * @note Do not modify the contents of the structure!  It is read only.
 */
bool LSCPCIe2_IF::getPciDriverExtraInfo(const ExtraResourceInfo_t **pExtra)
{

	// Use driver specific IOCTL call to get extra PCI info from the driver
	if (ioctl(hDev, IOCTL_LSCPCIE2_GET_EXTRA_INFO, &PCIExtraInfo) != 0)
	{
		close(hDev);
		hDev = -1;
		throw(PCIe_IF_Error("PCIe_IF: ioctl Failed!"));
	}
	else
	{
		if (RUNTIMECTRL(VERBOSE))
			cout<<"LSCPCIe2_IF Get_Extra_Info"<<endl;

	}
	*pExtra = &PCIExtraInfo;   // return pointer to the structure

	return(true);
}

/**
 * Write data for user space into the system DMA buffer for access by 
 * the device.
 * @param pBuf pointer to user's data
 * @param len the number of bytes (not DWs)
 * @return 
 */
bool LSCPCIe2_IF::writeSysDmaBuf(uint32_t *pBuf, size_t len) 
{
	ssize_t written;

	if (len == 0)
		return(false);  // don't support writing nothing

	written = write(this->hDev,   // device handle
			(void *)pBuf,   // user buffer to write
			len);	      // number of bytes to write

	if ((size_t)written != len)
		throw(PCIe_IF_Error("LSCPCIe2_IF: WriteFile failed! invalid data returned."));

	return(true);
}


/**
 * Read data from the system DMA buffer into the user space.
 * @param pBuf pointer to where to store device data in user space
 * @param len the number of bytes to read (not DWs)
 * @return 
 */
bool LSCPCIe2_IF::readSysDmaBuf(uint32_t *pBuf, size_t len) 
{
	ssize_t nRead;

	if (len == 0)
		return(false);  // don't support reading nothing

	nRead = read(this->hDev,     // the device handle
		     (void *)pBuf,   // where to store read data
			     len);    // how much

	if ((size_t)nRead != len)
		throw(PCIe_IF_Error("LSCPCIe2_IF: ReadFile failed! invalid data returned."));

	return(true);
}


/**
 * Return a string containing additional information (beyond that provided
 * by PCIe_IF) about the driver and device, such as the device bus, function
 * and device numbers, the DMA buffer addresses, etc.
 * @see ExtraResourceInfo_t 
 * @return true if successful; false if error
 */
bool LSCPCIe2_IF::getPCIExtraInfoStr(string &outs)
{


	const ExtraResourceInfo_t *pExtra;
	getPciDriverExtraInfo(&pExtra);

	std::ostringstream oss;

	oss<<"\nDriver Extra Info:\n";
   	oss<<"Driver: "<<pExtra->DriverName<<endl;
	oss<<"DevID="<<pExtra->devID<<endl;
	oss<<"PCI Bus#="<<pExtra->busNum<<endl;
	oss<<"PCI Dev#="<<pExtra->deviceNum<<endl;
	oss<<"PCI Func#="<<pExtra->functionNum<<endl;
	oss<<"UINumber="<<pExtra->UINumber<<endl;
	oss<<"hasDMA="<<pExtra->hasDmaBuf<<endl;
	oss<<"DmaBufSize="<<pExtra->DmaBufSize<<endl;
	oss<<"DmaAddr64="<<pExtra->DmaAddr64<<endl;
	oss<<"DmaPhyAddrHi="<<hex<<pExtra->DmaPhyAddrHi<<endl;
	oss<<"DmaPhyAddrLo="<<pExtra->DmaPhyAddrLo<<dec<<endl;


	if (pExtra->DmaPhyAddrLo % 4096)
	{
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		oss<<"WARNING!  Base is not 4kB aligned.\n";
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	}

	if (pExtra->DmaPhyAddrLo % 128)
	{
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		oss<<"WARNING!  Base is not 128 aligned.\n";
		oss<<"Writes need to account for crossing 4k boundary\n";
		oss<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	}

	outs = oss.str();

	return(true);
}


