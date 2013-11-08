/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file PCIe_IF.cpp */

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

#include <errno.h>
#include <sys/mman.h>

using namespace std;


#include "sysDefs.h"
#include "PCIeAPI.h"
#include "PCIe_IF.h"
#include "DebugPrint.h"


using namespace LatticeSemi_PCIe;


/*==================================================================================================*/
/*==================================================================================================*/

/*==================================================================================================*/
/*==================================================================================================*/


//====================================================================================
//====================================================================================
//         Lattice PCIe Evaluation Board IDs  (Vendor/Device register settings)
//====================================================================================
//====================================================================================

/** Original ID for all SC boards (PCIe demo) 
 * 
 */
const char *PCIe_IF::PCIE_SC_BRD = "SC";

/** Original ID for the ECP2M boards (PCIe demo)

 */
const char *PCIe_IF::PCIE_ECP2M_BRD = "ECP2M";
const char *PCIe_IF::PCIE_ECP2M35_BRD = "ECP2M";
const char *PCIe_IF::PCIE_ECP2M50_BRD = "ECP2M";

/** ECP3 PCIe DevKit board (same style as the ECP2M PCIe board.
 */
const char *PCIe_IF::PCIE_ECP3_BRD = "ECP3";


//-------------------------------------------------------------
// TODO
// !!!!! NOT FULLY IMPLEMENTED BY HDW YET !!!!
// Place holder and reference of Board model numbers
//
// New board database.  Uniquely IDs the FPGA device and board
//-------------------------------------------------------------

/** SC80, 1152 pkg, x8, SMPTE BNC connectors, 16 SegLED */
const char *PCIe_IF::SC80_PCIE_x8_BRD_GL028 = "ven_1204&dev_11c0";
/** ECP2M35, 672 pkg, SMPTE BNC, 16 SegLED */
const char *PCIe_IF::ECP2M35_PCIE_x4_BRD_GL029 = "ven_1204&dev_11d0";
/** ECP2M50, 672 pkg, SMPTE BNC, 16 SegLED */
const char *PCIe_IF::ECP2M50_PCIE_x4_BRD_GL029 = "ven_1204&dev_11d1";
/** SC25, 900 pkg, x1 connector, RJ45+SFP+SATA, no 16 Seg */
const char *PCIe_IF::SC25_PCIE_x1_BRD_GL030 = "ven_1204&dev_11e0";
/** ECP2M35, 672 pkg, x1 connector, SFP+SATA */
const char *PCIe_IF::ECP2M35_PCIE_x1_BRD_GL031 = "ven_1204&dev_11f0";
/** ECP2M35 video board, x1 connector, doudle height */
const char *PCIe_IF::ECP2M35_PCIE_x1_BRD_GL034 = "ven_1204&dev_1220";
/** SC80(v3), 1152 pkg, x4 connector,  16 SegLED */
const char *PCIe_IF::SC80_PCIE_x4_BRD_GL035 = "ven_1204&dev_11c0";



//====================================================================================
//====================================================================================
//             Lattice PCIe Demo IDs 
// driver uses the SubSys register settings and creates the following filename
// sections
//====================================================================================
//====================================================================================
/** original PCIe control plane demo for SC80 & 25 boards.
 * 2 BARs, no interrupts: BAR0=8kB, not used; BAR1=32kB, GPIO, EBR
 */
const char *PCIe_IF::PCIE_DEMO_SC = "Basic";

/** original PCIe control plane demo for ECP2M35 boards
 * 2 BARs, no interrupts: BAR0=8kB, not used; BAR1=32kB, GPIO, EBR
 * Same as PCIE_DEMO_SC but just a different subsys ID.
 */
const char *PCIe_IF::PCIE_DEMO_EC = "Basic";


/** The PCIe Basic Demo for all boards.
 * 2 BARs, no interrupts: BAR0=8kB, not used; BAR1=32kB, GPIO, EBR
 */
const char *PCIe_IF::BASIC_DEMO = "Basic";


/** PCIeThruput Demo ID (SFIF)
 * 2 BARs: 128kB, 0,1 map to same Wishbone bus and slave devices
 * SFIF IP, GPIO + IntrCtrl, EBR
 */
const char *PCIe_IF::SFIF_DEMO = "SFIF";

/** PCIe SGDMA Demo
 *  SGDMA core, GPIO+IntrCtrl, 64kB EBR (64bit)
 */
const char *PCIe_IF::PCIEDMA_DEMO = "DMA";




//====================================================================================
//====================================================================================
//         Lattice Linux Device Driver IDs (GUID term borrowed from Windows)
// Here a driver ID is actually the name of the parent driver node in the /dev tree.
// Linux opens devices based on the filename in the /dev tree.
//====================================================================================
//====================================================================================

/** Sim PCIe driver that */
const GUID *PCIe_IF::pLSCSIM_DRIVER = "SIM";

/** Basic PCIe driver that just provides rd/wr BAR access */
const GUID *PCIe_IF::pLSCPCIE_DRIVER = "lscpcie2";

/** Basic PCIe driver (v2) that also provides interrupts ISR, common buffer allocation
 * and extra information via additional IOCTLs
 */
const GUID *PCIe_IF::pLSCPCIE2_DRIVER = "lscpcie2";

/** SGDMA driver that implements all SGDMA and interrupt control in the driver.
 * This driver is SGDMA Demo specific.
 */
const GUID *PCIe_IF::pLSCDMA_DRIVER   = "lscdma";

/*==================================================================================================*/
/*==================================================================================================*/
/*==================================================================================================*/


#define NO_SUCH_GUID     -1
#define NO_SUCH_BOARD_ID -2
#define NO_SUCH_DEMO_ID  -3
#define NO_MORE_BOARDS   -4
#define PARAM_ERROR      -5
#define OPEN_FILE_FAILED -6



/**
 * Create an interface to a Lattice PCIe Driver and eval board.
 * This requires opening the Win32 device driver that provides access to
 * the card that the FPGA is located on.  The driver needs to have been installed
 * on the PC using the "Add New Hardware" wizard.
 * @param pGUID specify the driver to open (see GUID list)
 * @param pBoardID the PCI Vendor and Device register pair indicating the board to open
 * @param pDemoID the PCI SubSystem regsiter value indicating the Demo IP to open
 * @param devNum the instance of the board (i.e. locate and open the 2nd SC80 x4 board)
 * @param pFriendlyName (NOT USED) text string naming the board, set in registry,
 * @param pFunctionName text string naming a specific driver function to open (i.e. use DMA chan #1)
 * 
 * @note If run-time VERBOSE is set then display information during opening driver and board.
 * 
 * @throws PCIe_IF_Error if the specified Driver/Board/Demo/Instance can not be found
 */
PCIe_IF::PCIe_IF(const GUID *pGUID, 
				 const char *pBoardID,
				 const char *pDemoID,
				 uint32_t devNum, 
				 const char *pFriendlyName, 
				 const char *pFunctionName): RegisterAccess((void *)pBoardID)
{
	int ret;

	hDev = -1;
	memset(pBAR, 0, sizeof(pBAR));

	if (pGUID == NULL)
	    return;   // this is being invoked by a simulation layer, do not open real hardware

	if ((strstr(pGUID, "SIM") != NULL) || (strstr(pDemoID, "SIM") != NULL))
	    return;   // this is being invoked by a simulation layer, do not open real hardware

	if (devNum == 0)
	    return;   // this is being invoked by a simulation layer, do not open real hardware

	if (RUNTIMECTRL(VERBOSE))
		cout<<"Opening BoardID: "<<pBoardID<<" DemoID: "<<pDemoID<<endl;


	/* Open the kernel driver using the driver ID, Board, Demo and instance number passed in */
	ret = OpenDriver(hDev, pGUID, pBoardID, pDemoID, devNum, pFriendlyName, pFunctionName);
	if (ret == NO_SUCH_GUID)
		throw(PCIe_IF_Error("NO_SUCH_GUID"));
	else if (ret == NO_SUCH_BOARD_ID)
		throw(PCIe_IF_Error("NO_SUCH_BOARD_ID"));
	else if (ret == NO_MORE_BOARDS)
		throw(PCIe_IF_Error("NO_MORE_BOARDS"));
	else if (ret == PARAM_ERROR)
		throw(PCIe_IF_Error("PARAM_ERROR"));
	else if (ret == OPEN_FILE_FAILED)
		throw(PCIe_IF_Error("OPEN_FILE_FAILED"));

	if (RUNTIMECTRL(VERBOSE))
		cout<<"PCIe_IF created."<<endl;


	// Use driver specific IOCTL call to get PCI info from the driver
	if (ioctl(hDev, IOCTL_LSCPCIE_GET_RESOURCES, &PCIinfo) != 0)
	{
		close(hDev);
		hDev = -1;
		throw(PCIe_IF_Error("PCIe_IF: ioctl Failed!"));
	}

	if (MmapDriver() < 0)
		throw(PCIe_IF_Error("PCIe_IF: mmap BARs Failed!"));

	if (RUNTIMECTRL(VERBOSE))
	{
		cout<<"PCIe_IF Get_RESOURCES: "<<endl;

		cout<<"hasInterrupt: "<<PCIinfo.hasInterrupt;
		cout<<"  Vect: "<<PCIinfo.intrVector;
		cout<<"  Num BARs: "<<PCIinfo.numBARs<<endl;
		for (uint32_t i = 0; i <PCIinfo.numBARs; i++)
		{
			cout<<"BAR"<<PCIinfo.BAR[i].nBAR;
			cout<<"  Addr: "<<hex<<PCIinfo.BAR[i].physStartAddr;
			cout<<"  Size: "<<dec<<PCIinfo.BAR[i].size;
			cout<<"  Mapped: "<<PCIinfo.BAR[i].memMapped<<endl;
		}
	}
}


/**
 * Delete an instance of a register access object.
 */
PCIe_IF::~PCIe_IF()
{
	int sysErr;


	/* Release the shared memory.  It won't go away but we're done with it */
	for (int i = 0; i < MAX_PCI_BARS; i++)
	{
		if (pBAR[i] != 0)
		{
			sysErr = munmap((void *)pBAR[i], sizeBAR[i]);
			if (sysErr == -1)
				perror("munmap: ");
		}
	}


	if (hDev != -1)
	{
		close(hDev);
		hDev = -1;   // don't use anymore
	}

	if (RUNTIMECTRL(VERBOSE))
		cout<<"PCIe_IF: driver closed. Bye."<<endl;
}

/**
 * Return the Windows handle to the open device driver so higher level
 * classes can use the IOCTL routines in the driver.
 * @return Windows handle to the driver (file object)
 */ 
HANDLE PCIe_IF::getDriverHandle(void) 
{
	return(hDev);
}


/**
 * Return the 256 bytes of the device's PCI configuration space registers.
 * These registers must be present on any PCI/PCIExpress device.
 * They have a standard format.
 * @param pCfg user's location to store 256 bytes
 * @return true if read byte OK, false if driver reports error
 */
bool PCIe_IF::getPCIConfigRegs(uint8_t *pCfg)
{
	PCIResourceInfo_t PCIinfo;


	if (ioctl(hDev, IOCTL_LSCPCIE_GET_RESOURCES, &PCIinfo) != 0)
		return(false);

	memcpy(pCfg, PCIinfo.PCICfgReg, sizeof(PCIinfo.PCICfgReg));

	return(true);
}


/**
 * Return the device driver information structure.
 * This includes the BAR assignments, interrupt info and the PCI cfg registers.
 * @param pInfo user's pointer that will point to the internal driver structure
 * @return true (info obtained in constructor so always present)
 * @note Do not modify the contents of the structure!  It is read only.
 */
bool PCIe_IF::getPCIDriverInfo(const PCIResourceInfo_t **pInfo)
{
	*pInfo = &PCIinfo;   // return pointer to the structure

	return(true);
}


/**
 * Return the PCI resources formatted into a displayable string.
 * Caller supplies the C++ string object and this routine parses
 * the standard PCI Cfg0 registers (0x00-0x3f) into human readable
 * format.  Mainly for diagnostic output.
 * @param outs caller supplied string to fill with formatted results
 * @return true if CFG0 registers read and parsed, false if errors
 */
bool PCIe_IF::getPCIResourcesStr(string &outs)
{
	uint8_t buf[256];
	PCICfgRegs_t *pPCI;

	// first get the Cfg 0 registers
	getPCIConfigRegs(buf);

	// Now parse and convert to a human readable string of know fields
	pPCI = (PCICfgRegs_t *)buf;

	std::ostringstream oss;
	oss<<std::setbase(16);


	oss<<"[00]Vendor ID: ";
	oss<<pPCI->vendorID;
	oss<<"\n[02]Device ID: ";
	oss<<pPCI->deviceID;
	oss<<"\n[04]Command: ";
	oss<<pPCI->command;
	oss<<" = [";
	if (pPCI->command & 0x0100)
		oss<<"IntrDis";
	// bit 9 not used in PCIexpress
	if (pPCI->command & 0x0100)
		oss<<" SERR#_en";
	// bit 7 not used in PCIexpress
	if (pPCI->command & 0x0040)
		oss<<" ParityErr";
	// bits 5..3 are not used in PCIexpress
	if (pPCI->command & 0x0004)
		oss<<" BusMstr";
	if (pPCI->command & 0x0002)
		oss<<" Mem";
	if (pPCI->command & 0x0001)
		oss<<" IO";
	oss<<"]";

	oss<<"\n[06]Status: ";
	oss<<pPCI->status;
	oss<<" = [";
	if (pPCI->status & 0x8000)
		oss<<" ParityErr";
	if (pPCI->status & 0x4000)
		oss<<" sentSysErr";
	if (pPCI->status & 0x2000)
		oss<<" recvdMstrAbort";
	if (pPCI->status & 0x1000)
		oss<<" recvdTargetAbort";
	if (pPCI->status & 0x0800)
		oss<<" sentTargetAbort";
	if (pPCI->status & 0x0100)
		oss<<" DataErr";
	// bits 7..5 are not used in PCIexpress
	if (pPCI->status & 0x0010)
		oss<<" CapList";
	if (pPCI->status & 0x0008)
		oss<<" Intr";
	oss<<"]";



	oss<<"\n[08]Rev ID: ";
	oss<<(int)pPCI->revID;
	oss<<"\n[09]Class Code: ";
	oss<<(int)pPCI->classCode[2];
	oss<<(int)pPCI->classCode[1];
	oss<<(int)pPCI->classCode[0];
	oss<<"\n[0c]Cache Line Size: ";
	oss<<(int)pPCI->cacheLineSize;
	oss<<"\n[0d]Latency Timer: ";
	oss<<(int)pPCI->latencyTimer;
	oss<<"\n[0e]Header Type: ";
	oss<<(int)pPCI->headerType;
	oss<<"\n[0f]BIST: ";
	oss<<(int)pPCI->BIST;
	oss<<"\n[10]BAR0: ";
	oss<<pPCI->BAR0;
	oss<<"\n[14]BAR1: ";
	oss<<pPCI->BAR1;
	oss<<"\n[18]BAR2: ";
	oss<<pPCI->BAR2;
	oss<<"\n[1c]BAR3: ";
	oss<<pPCI->BAR3;
	oss<<"\n[20]BAR4: ";
	oss<<pPCI->BAR4;
	oss<<"\n[24]BAR5: ";
	oss<<pPCI->BAR5;
	oss<<"\n[28]Cardbus CIS Ptr: ";
	oss<<pPCI->cardBusPtr;
	oss<<"\n[2c]Subsystem Vendor ID: ";
	oss<<pPCI->subsystemVendorID;
	oss<<"\n[2e]Subsystem ID: ";
	oss<<pPCI->subsystemID;
	oss<<"\n[30]Exp ROM: ";
	oss<<pPCI->expROM_BAR;
	oss<<"\n[34]Capabilities Ptr: ";
	oss<<(int)pPCI->capabilitiesPtr;
	oss<<"\n[3c]Interrupt Line: ";
	oss<<(int)pPCI->interruptLine;
	oss<<"\n[3d]Interrupt Pin: ";
	oss<<(int)pPCI->interruptPin;
	oss<<"\n[3e]Min Grant: ";
	oss<<(int)pPCI->minGrant;
	oss<<"\n[3f]Max Latency: ";
	oss<<(int)pPCI->maxLatency;

	outs = oss.str();

	return(true);
}

/**
 * Return the string containing the driver version and build date.
 * The drivers implement an IOCTL that returns the build number, date
 * and description of the driver.  This method gets that info from
 * the kernel driver and returns it to the caller.
 * @return true if info returned from driver or false if the driver
 * doesn't implement the IOCTL code or something else failed.
 */
bool PCIe_IF::getDriverVersionStr(string &outs)
{
	DriverVerStr_t str;
	
	if (ioctl(hDev, IOCTL_LSCPCIE_GET_VERSION_INFO, str) != 0)
	{
		outs.append("ERROR");
		return(false);
	}

	outs.append(str);

	return(true);
}


/**
 * Fill the string with info about the BARs and interrupts the
 * device and driver are using.
 * This information is obtained from the OS at driver init time.
 * @param outs the string to fill with the formatted BAR info
 * @return true if successful; false if error
 */
bool PCIe_IF::getDriverResourcesStr(string &outs)
{
	const PCIResourceInfo_t *pPCI;

	// first get the driver data
	this->getPCIDriverInfo(&pPCI);

	std::ostringstream oss;
	oss<<"Number of BARs: "<<pPCI->numBARs<<"\n";
	for (uint32_t i = 0; i < pPCI->numBARs; i++)
	{
		oss<<"BAR"<<pPCI->BAR[i].nBAR<<":";
		oss<<"  Addr: "<<hex<<pPCI->BAR[i].physStartAddr;
		oss<<"  Size: "<<dec<<pPCI->BAR[i].size;
		oss<<"  Mapped:"<<pPCI->BAR[i].memMapped<<"\n";
	}
	if (pPCI->hasInterrupt)
		oss<<"Interrupt Vector: "<<pPCI->intrVector<<"\n";
	else
		oss<<"NO INTERRUPTS\n";

	outs = oss.str();

	return(true);
}


/**
 * Fill the string with info about the PCIe Capabilities structures.
 * This information is read from the driver and the PCI config 
 * registers are parsed to see if a Capabilities list exists.
 * If so, the list is wlaked and each discovered structure is parsed
 * into readable format and saved into the return string.
 * @param outs the string to fill with the list data.
 * 
 * @return true if successful; false if error
 */
bool PCIe_IF::getPCICapabiltiesStr(string &outs)
{

	int i, id, next, index;
	uint8_t buf[256];
	PCICfgRegs_t *pPCI;
	uint8_t *p8;
	uint16_t *p16;
	uint32_t *p32;

	static const char *L0sLat[8] = {"64ns", "128ns", "256ns", "1us", "2us", "4us", ">4us"};	
	static const char *L1Lat[8] = {"1us", "2us", "4us", "8us", "16us", "32us", "64us", ">64us"};	


	// first get the Cfg 0 registers
	getPCIConfigRegs(buf);

	pPCI = (PCICfgRegs_t *)buf;

	std::ostringstream oss;
	//oss<<std::setbase(16);

	// Now parse the capabilities structures.  The first structure in the list
	// is pointed to by the Capabilities Ptr at location 0x34.  If this is 0
	// or the capabilities bit in the status is 0 then there are none.
	if (((pPCI->status & 0x10) == 0) || (pPCI->capabilitiesPtr == 0))
	{
		oss<<"No Capabilities Structures.";
		cout<<oss.str();
		return(true);
	}

	i = 0;
	next = (int)pPCI->capabilitiesPtr;
	while ((next >= 0x40) && (i < 16))
	{
		++i;  // loop counter to prevent circular loop
		index = next;
		id = buf[next];
		p8 = (uint8_t *)&buf[next];
		p16 = (uint16_t *)&buf[next];
		p32 = (uint32_t *)&buf[next];
		next = (int)buf[next + 1];
		switch(id)
		{
			case 1:  // Power Management
				oss<<"\nPower Management Capability Structure @ 0x"<<hex<<index<<"\n";

				oss<<"\tPwrCap: 0x"<<hex<<p16[1]<<dec<<" = [";
				oss<<"PME:";
				if (p16[1] & 0x8000)
					oss<<"D3c-";
				if (p16[1] & 0x4000)
					oss<<"D3h-";
				if (p16[1] & 0x2000)
					oss<<"D2-";
				if (p16[1] & 0x1000)
					oss<<"D1-";
				if (p16[1] & 0x0800)
					oss<<"D0";
				if (p16[1] & 0x0400)
					oss<<" D2_support";
				if (p16[1] & 0x0200)
					oss<<" D1_support";
				// bunch of other bits that aren't really applicable
				oss<<" ver="<<(p16[1] & 0x007);
				oss<<"]\n";

				oss<<"\tPMCSR: 0x"<<hex<<p16[2]<<dec<<" = [";
				if (p16[2] & 0x8000)
					oss<<"PME_Event";
				// Data_scale is bits 14:13
				// Data_select is bits 12:9
				if (p16[2] & 0x0100)
					oss<<" PME_en";
				oss<<" state=D"<<(p16[2] & 0x003);
				oss<<"]\n";

				oss<<"\tData: 0x"<<hex<<(int)p8[7]<<"\n";
				break;

			case 2:  // AGP Capability
				oss<<"\nAGP Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 3:  // VPD (Vital Product Data) Capability
				oss<<"\nVPD (Vital Product Data) Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 4:  // Slot ID Capability
				oss<<"\nSlot ID Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 5:  // MSI
				oss<<"\nMSI Capability Structure @ 0x"<<hex<<index<<"\n";

				oss<<"\tMsgCtrlReg: 0x"<<hex<<p16[1]<<dec<<" = [";
				if (p16[1] & 0x0080)
					oss<<"64bitAddr ";
				oss<<"numMsgs="<<(1<<((p16[1] & 0x0070)>>4))<<" ";
				oss<<"reqMsgs="<<(1<<((p16[1] & 0x000e)>>1))<<" ";
				if (p16[1] & 0x0001)
					oss<<"MSI_Enable ";
				oss<<"]\n";

				oss<<"\tMsgAddr: 0x"<<hex<<p32[2]<<p32[1]<<"\n";   // display the 64 bit address
				oss<<"\tMsgData: 0x"<<hex<<p16[6]<<"\n";
				break;

			case 6:  // CompactPCI Hot Swap
				oss<<"\nCompactPCI Hot Swap Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 7:  // PCI-X
				oss<<"\nPCI-X Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 8:  // AMD
				oss<<"\nAMD Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 9:  // Vendor Specific
				oss<<"\nVendor Specific Capability Structure @ 0x"<<hex<<index<<"\n";
				break;

			case 0x0a:  // Debug Port
				oss<<"\nDebug Port @ 0x"<<hex<<index<<"\n";
				break;

			case 0x0b:  // CompactPCI central resource control
				oss<<"\nCompactPCI central resource control @ 0x"<<hex<<index<<"\n";
				break;

			case 0x0c:  // PCI Hot Plug
				oss<<"\nPCI Hot-Plug Compatable @ 0x"<<hex<<index<<"\n";
				break;

			case 0x10: // PCI Express
				oss<<"\nPCI Express Capability Structure @ 0x"<<hex<<index<<"\n";
				oss<<"\tPCIe_Cap: 0x"<<hex<<p16[1]<<dec<<" = [";
				oss<<" IntMsg#="<<((p16[1] & 0x3e00)>>9);
				if (p16[1] & 0x0100)
					oss<<" SlotImp";
				oss<<" type="<<((p16[1] & 0x0f0)>>4);   // 0=PCIeEndPt, 1= Legacy
				oss<<" ver="<<(p16[1] & 0x00f);
				oss<<"]\n";


				oss<<"\tDev_Cap: 0x"<<hex<<p32[1]<<dec<<" = [";
				if (p32[1] & 0x4000)
					oss<<" PwrInd";
				if (p32[1] & 0x2000)
					oss<<" AttInd";
				if (p32[1] & 0x1000)
					oss<<" AttBtn";
				oss<<" L1Lat="<<L1Lat[((p32[1] & 0x0e00)>>9)];
				oss<<" L0sLat="<<L0sLat[((p32[1] & 0x01c0)>>6)];
				if (p32[1] & 0x20)
					oss<<" 8bit_tag";
				if (p32[1] & 0x18)
					oss<<" Phantom";
				oss<<" MaxTLPSize="<<(128<<(p32[1] & 0x07));
				oss<<"]\n";


				oss<<"\tDev_Ctrl: 0x"<<hex<<p16[4]<<dec<<" = [";
				oss<<" MaxRdSize="<<(128<<((p16[4] & 0x7000)>>12));
				if (p16[4] & 0x0800)
					oss<<" NoSnoop";
				if (p16[4] & 0x0400)
					oss<<" AuxPwr";
				if (p16[4] & 0x0200)
					oss<<" Phantom";
				if (p16[4] & 0x0100)
					oss<<" ExtTag";
				oss<<" MaxTLPSize="<<(128<<((p16[4] & 0x0e)>>5));
				if (p16[4] & 0x0010)
					oss<<" RlxOrd";
				if (p16[4] & 0x0008)
					oss<<" UR";
				if (p16[4] & 0x0004)
					oss<<" ERR_FATAL";
				if (p16[4] & 0x0002)
					oss<<" ERR_NONFATAL";
				if (p16[4] & 0x0001)
					oss<<" ERR_COR";
				oss<<"]\n";


				oss<<"\tDev_Stat: 0x"<<hex<<p16[5]<<dec<<" = [";
				if (p16[5] & 0x0020)
					oss<<" ReqPend";
				if (p16[5] & 0x0010)
					oss<<" AuxPwr";
				if (p16[5] & 0x0008)
					oss<<" UR";
				if (p16[5] & 0x0004)
					oss<<" ERR_FATAL";
				if (p16[5] & 0x0002)
					oss<<" ERR_NONFATAL";
				if (p16[5] & 0x0001)
					oss<<" ERR_COR";
				oss<<"]\n";


				oss<<"\tLink_Cap: 0x"<<hex<<p32[3]<<dec<<" = [";
				oss<<" Port#="<<((p32[3] & 0xff000000)>>24);
				oss<<" L1Lat="<<L1Lat[((p32[3] & 0x38000)>>15)];
				oss<<" L0sLat="<<L0sLat[((p32[3] & 0x7000)>>12)];
				if ((p32[3] & 0x0c00) == 0x0c00)
					oss<<" ASPM:L1+L0s";
				if ((p32[3] & 0x0c00) == 0x0400)
					oss<<" ASPM:L0s";
				oss<<" Width=x"<<((p32[3] & 0x03f0)>>4);
				if (p32[3] & 0x0001)
					oss<<" 2.5G";
				oss<<"]\n";


				oss<<"\tLink_Ctrl: 0x"<<hex<<p16[6]<<dec<<" = [";
				if (p16[8] & 0x0080)
					oss<<" ExtSync";
				if (p16[8] & 0x0040)
					oss<<" CommonClk";
				if (p16[8] & 0x0008)
					oss<<" RCB=128";
				else
					oss<<" RCB=64";
				oss<<" ASPM:";
				switch(p16[8] & 0x0003)
				{
					case 0: oss<<"Disabled"; break;
					case 1: oss<<"L0s"; break;
					case 2: oss<<"L1"; break;
					case 3: oss<<"L0s+L1"; break;
				}
				oss<<"]\n";


				oss<<"\tLink_Stat: 0x"<<hex<<p16[7]<<dec<<" = [";
				if (p16[9] & 0x1000)
					oss<<" SlotRefClk";
				if (p16[9] & 0x0400)
					oss<<" TRAIN_ERR";
				oss<<" Width=x"<<((p16[9] & 0x03f0)>>4);
				if (p16[9] & 0x0001)
					oss<<" 2.5G";
				oss<<"]\n";

				// Slot Registers and Root Registers not implemented by our EndPoint core

				break;


			default:
				oss<<"Unknown Capability: "<<dec<<id<<hex<<"\n";
				next = 0; // abort
				break;

		}

	}

	outs = oss.str();

	return(true);
}




/**
 * Read 8 bits from an LSC FPGA register via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @return the 8bit value read
 */
uint8_t PCIe_IF::read8(uint32_t offset) 
{
	uint8_t val;
	uint32_t nBAR;

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: read8 failed! invalid BAR."));

    	volatile uint8_t *p8 = (uint8_t *)pBAR[nBAR];

	val = *(p8 + OFFSET2ADDR(offset));

	return(val);
}
    

/**
 * Write 8 bits to an LSC hardware register via PCIe bus.
 * @param offset BAR + address of device register to write to
 * @param val value to write into the register
 */
void PCIe_IF::write8(uint32_t offset, uint8_t val)
{
	uint32_t nBAR;


	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: write8 failed! invalid BAR."));

    	volatile uint8_t *p8 = (uint8_t *)pBAR[nBAR];

	*(p8 + OFFSET2ADDR(offset)) = val;
}



/**
 * Read 16 bits from an FPGA register via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @param val location of storage for data read
 * @return uint16 value read
 */
uint16_t PCIe_IF::read16(uint32_t offset)
{
	uint16_t val;
	uint32_t nBAR;

	if (offset & 0x01)
		throw(PCIe_IF_Error("PCIe_IF: read16 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: read16 failed! invalid BAR."));

    	volatile uint16_t *p16 = (uint16_t *)pBAR[nBAR];

	val = *(p16 + (OFFSET2ADDR(offset)/sizeof(uint16_t)));

	return(val);
}


/**
 * Write 16 bits to an SC hardware register via PCIe bus.
 * @param offset address of device register to write to
 * @param val value to write into the register
 * @return true; error in writing will cause hardware exception
 */
void PCIe_IF::write16(uint32_t offset, uint16_t val)
{
	uint32_t nBAR;

	if (offset & 0x01)
		throw(PCIe_IF_Error("PCIe_IF: write16 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: write16 failed! invalid BAR."));

    	volatile uint16_t *p16 = (uint16_t *)pBAR[nBAR];

	*(p16 + (OFFSET2ADDR(offset)/sizeof(uint16_t))) = val;

}



/**
 * Read 32 bits from an SC hardware register via PCIe bus.
 * This is done with 2 16 bit reads because the SC900 only has a 16 bit wide
 * data bus and will not allow accesses larger than 16 bits.
 * @param offset address of device register to read from
 * @param val location of storage for data read
 * @return true; false if address not multiple of 4
 * @note error in reading will cause hardware exception
 */
uint32_t PCIe_IF::read32(uint32_t offset)
{
	uint32_t val;
	uint32_t nBAR;

	if (offset & 0x03)
		throw(PCIe_IF_Error("PCIe_IF: read32 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: read32 failed! invalid BAR."));

    	volatile uint32_t *p32 = (uint32_t *)pBAR[nBAR];

	val = *(p32 + (OFFSET2ADDR(offset)/sizeof(uint32_t)));

	return(val);
}
    


/**
 * Write 32 bits to an SC hardware register via PCIe bus.
 * @param offset address of device register to write to
 * @param val value to write into the register
 * @note error in reading will cause hardware exception
 */
void PCIe_IF::write32(uint32_t offset, uint32_t val)
{
	uint32_t nBAR;

	if (offset & 0x03)
		throw(PCIe_IF_Error("PCIe_IF: write32 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: write32 failed! invalid BAR."));

    	volatile uint32_t *p32 = (uint32_t *)pBAR[nBAR];

	*(p32 + (OFFSET2ADDR(offset)/sizeof(uint32_t))) = val;

}


/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/

/**
 * Read a block of 8 bit registers from FPGA hardware via PCIe bus.
 * @param offset BAR + address of device register to read from
 * @param val location of storage for data read
 * @param len number of bytes to read
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error if driver fails
 */
bool PCIe_IF::read8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	volatile uint8_t *addr;
	uint32_t nBAR;
	size_t i;

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block read8 failed! invalid BAR."));

	addr = (uint8_t *)this->pBAR[nBAR] + OFFSET2ADDR(offset);


	if (incAddr)
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
		++addr;
	    }

	}
	else  // don't increment read address
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
	    }
	}

	return(true);
}
    

/**
 * Write a block of 8 bit registers into FPGA hardware via PCIe bus.
 * @param offset address of device register to write to
 * @param val location of bytes to write
 * @param len number of bytes to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error in writing will cause hardware exception
 */
bool PCIe_IF::write8(uint32_t offset, uint8_t *val, size_t len, bool incAddr)
{
	volatile uint8_t *addr;
	uint32_t nBAR;
	size_t i;

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block read8 failed! invalid BAR."));

	addr = (uint8_t *)this->pBAR[nBAR] + OFFSET2ADDR(offset);

	if (incAddr)
	{
	    // loop to copy len bytes from  user's array to BAR addr
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
		++addr;
	    }
	}
	else  // don't increment write address
	{
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
	    }
	}

	return(true);
}



/**
 * Read a block of 16 bit registers from SC hardware via PCIe bus.
 * @param offset address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 16 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 * @note error in reading will cause hardware exception
 */
bool PCIe_IF::read16(uint32_t offset, uint16_t *val, size_t len, bool incAddr)
{
	volatile uint16_t *addr;
	uint32_t nBAR;
	size_t i;

	if (offset & 0x01)
		throw(PCIe_IF_Error("PCIe_IF: block read16 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block read16 failed! invalid BAR."));

	addr = (uint16_t *)this->pBAR[nBAR] + (OFFSET2ADDR(offset) / sizeof(uint16_t));


	if (incAddr)
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
		++addr;
	    }

	}
	else  // don't increment read address
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
	    }
	}

	return(true);
}


/**
 * Write a block of 16 bit registers into FPGA hardware via PCIe bus.
 * @param offset BAR + address of device registers to write to
 * @param val location of 16 bit words to write
 * @param len number of 16 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 */
bool PCIe_IF::write16(uint32_t offset, uint16_t *val, size_t len, bool incAddr)
{
	volatile uint16_t *addr;
	uint32_t nBAR;
	size_t i;

	if (offset & 0x01)
		throw(PCIe_IF_Error("PCIe_IF: block write16 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block write16 failed! invalid BAR."));

	addr = (uint16_t *)this->pBAR[nBAR] + (OFFSET2ADDR(offset) / sizeof(uint16_t));

	if (incAddr)
	{
	    // loop to copy len bytes from  user's array to BAR addr
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
		++addr;
	    }
	}
	else  // don't increment write address
	{
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
	    }
	}

	return(true);
}



/**
 * Read a block of 32 bit registers from FPGA hardware via PCIe bus.
 * @param offset BAR + address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 32 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4 or read failed
 */
bool PCIe_IF::read32(uint32_t offset, uint32_t *val, size_t len, bool incAddr)
{

	volatile uint32_t *addr;
	uint32_t nBAR;
	size_t i;

	if (offset & 0x03)
		throw(PCIe_IF_Error("PCIe_IF: block read32 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block read32 failed! invalid BAR."));

	addr = (uint32_t *)this->pBAR[nBAR] + (OFFSET2ADDR(offset) / sizeof(uint32_t));


	if (incAddr)
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
		++addr;
	    }

	}
	else  // don't increment read address
	{
	    // loop to copy len bytes from BAR addr to user's array
	    for (i = 0; i < len; i++)
	    {
		*val = *addr;
		++val;
	    }
	}

	return(true);
}
    


/**
 * Write a block of 32 bit registers into SC hardware via PCIe bus.
 * @param offset address of device registers to write to
 * @param val location of 32 bit words to write into SC
 * @param len number of 32 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4
 */
bool PCIe_IF::write32(uint32_t offset, uint32_t *val, size_t len, bool incAddr)
{
	volatile uint32_t *addr;
	uint32_t nBAR;
	size_t i;

	if (offset & 0x03)
		throw(PCIe_IF_Error("PCIe_IF: block write32 failed! invalid offset."));

	nBAR = OFFSET2BAR(offset);
	if (this->pBAR[nBAR] == NULL)
		throw(PCIe_IF_Error("PCIe_IF: block write32 failed! invalid BAR."));

	addr = (uint32_t *)this->pBAR[nBAR] + (OFFSET2ADDR(offset) / sizeof(uint32_t));

	if (incAddr)
	{
	    // loop to copy len bytes from  user's array to BAR addr
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
		++addr;
	    }
	}
	else  // don't increment write address
	{
	    for (i = 0; i < len; i++)
	    {
		*addr = *val;
		++val;
	    }
	}

	return(true);

}



/**
 * Open a device driver via a device interface and return the Linux file handle.
 * The device to open is specified using the /dev filename (pointed to by the GUID)
 * The instance number of the device can also be specified, such as open the 2nd board of this
 * type.  If the driver supports opening various functions, such as DMA using a
 * specific channel, then specify this function as a string in the pFunctionName
 * parameter.
 *
 * For Linux:
 * GUID points to a string constant that holds the base device node in the /dev/ tree
 * pBoardID points to the next part of the filename ("SC" or "ECP2M")
 * pDemoID points to the next part of the filename ("Basic" or "SFIF")
 * instance is a number 1, 2, etc. converted to a string to form the end of the filename
 * pFunctionName is a string that is appended to the others to open some feature of the driver
 *
 * /dev/lscpcie2/SC_Basic_1
 * /dev/lscpcie2/ECP2M_SFIF_2
 *
 * The filename is created by concatenating GUID/pBoardID_pDemoID_instance/pFunctionName
 * The file is then opened using this file name, and the returned handle will provide the
 * interface to the driver.
 *
 * the beauty of Linux and udev is that the /dev/ tree already contains all the enumerated
 * Lattice PCIe boards (as detected by the driver upon installing or the PCI bus sub-system
 * invoking the driver 
 *
 *  @param hnd reference to caller supplied HANDLE to return open driver
 *	@param pGUID pointer to the GUID identifying the Lattice PCIe device driver to open
 *	@param pBoardID pointer a string identifying the PCI Vendor_Device pair in board's CFG0 space
 *	@param pDemoID pointer a string identifying the SubSys register values used to identify a Demo
 *	@param instance which instance of that board, if multiple in system
 *  @param pFriendlyName string specifying the Window registery friendly name associated with device (NULL for now)
 *	@param pFunctionName string specifying driver specific function to open (appended to the filename at open time)
 */
int PCIe_IF::OpenDriver(HANDLE &hnd,
			const GUID *pGUID, 
			const char *pBoardID,
			const char *pDemoID,
			uint32_t instance, 
			const char *pFriendlyName, 
			const char *pFunctionName)
{
	int rv = OK;
	int fd;
	char drivername[MAX_DEV_FILENAME_SIZE];

	DEBUGPRINT(("Hello world"));

	ENTER();

	if ((pFunctionName != NULL) && (strlen(pFunctionName) >= MAX_FUNCTION_NAME_SIZE))
	{
		ERRORSTR("ERROR: FunctionName string too large\n");
		return(PARAM_ERROR);
	}


	if ((pBoardID == NULL) || (strlen(pBoardID) > MAX_BOARD_ID_SIZE))
	{
		ERRORSTR("ERROR: invalid BoardID string!\n");
		return(PARAM_ERROR);
	}

	if ((pDemoID == NULL) || (strlen(pDemoID) > MAX_DEMO_ID_SIZE))
	{
		ERRORSTR("ERROR: invalid DemoID string!\n");
		return(PARAM_ERROR);
	}


	sprintf(drivername, "/dev/%s/%s_%s_%d", pGUID,
						pBoardID,
						pDemoID,
						instance);


	if (pFunctionName != NULL)
	{
		strcat(drivername, pFunctionName);
	}

	DEBUGPRINT(("before open"));

	DEBUGPRINT(("Opening: %s", drivername));

	fd = open(drivername, O_RDWR, 0666); 
	if (fd == -1)
	{
		perror("OpenDriver: open");
		ERRORSTR("ERROR: Device file not found!\n");
		return(OPEN_FILE_FAILED);
	}

	hnd = fd;  // return the handle to the open device

	LEAVE();

	return(rv);
}


/**
 * Map the BAR address windows into a processes memory space using MMAP.
 * The number of valid BARs is found from the initial creation.
 * NOTE: This must be called after the PCI resource have been read from
 * the driver and the PCIinfo structure is populated.
 */
int PCIe_IF::MmapDriver(void)
{
	void *pmem;
	size_t len;
	int nMapped = 0;
	int nBAR;
	
	ENTER();

	// BARs have to be allocated starting at 0 and going sequentially (I believe)
	// Question: is this handling 64 bit addresses???
	for (uint32_t i = 0; i < this->PCIinfo.numBARs; i++)
	{
		nBAR = this->PCIinfo.BAR[i].nBAR;
		// First we need to select the BAR for mapping using an IOCTL call
		if (ioctl(this->hDev, IOCTL_LSCPCIE_SET_BAR, nBAR) != 0)
		{
			ERRORSTR("ERROR: MmapDriver ioctl failed!\n");
			return(-1);
		}

		len = this->PCIinfo.BAR[i].size;

		pmem = mmap(0,                      /* choose any user address */
			    len,                    /* This big */
			    PROT_READ | PROT_WRITE, /* access control */
			    MAP_SHARED,             /* access control */
			    this->hDev,             /* the device object */
			    0);                     /* the offset from beginning */

		if (pmem == MAP_FAILED)
		{
			perror("mmap: ");
			ERRORSTR("ERROR: MmapDriver mmap failed!\n");
			return(-1);
		}
		
		DEBUGPRINT(("mapped BAR%d to 0x%p", nBAR, pmem));
		this->PCIinfo.BAR[i].memMapped = true;
		this->pBAR[nBAR] = pmem;
		this->sizeBAR[nBAR] = len;   // save for unmap purposes 
		++nMapped;
	}



	return(nMapped);
}


