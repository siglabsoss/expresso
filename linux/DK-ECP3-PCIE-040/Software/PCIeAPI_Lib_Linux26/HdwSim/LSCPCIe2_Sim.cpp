/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/*
 * Simulated Lattice FPGA access via the PCIExpress bus and PCIe IP core.
 * This class simulates AccessLayer access to hardware devices.
 * All access are done to memory-resident objects within this program.
 * No actual bus transfers are done.
 */

using namespace std;

#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>



#include "DebugPrint.h"
#include "LSCPCIe2_Sim.h"


using namespace LatticeSemi_PCIe;


static uint8_t CfgRegs[256] =
{

/*00:*/  0x04,0x12,0x03,0x53,   0x04,0x01,0x10,0x00,   0x01,0x00,0x00,0xff,   0x10,0x00,0x00,0x00,
/*10:*/  0x00,0x60,0xef,0xd7,   0x00,0x80,0xef,0xd7,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*20:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x04,0x12,0x03,0x53,
/*30:*/  0x00,0x00,0x00,0x00,   0x40,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0xff,0x00,0x00,0x00,
/*40:*/  0x10,0x68,0x01,0x00,   0x00,0x00,0x00,0x00,   0x10,0x20,0x0a,0x00,   0x11,0x0c,0x00,0x00,
/*50:*/  0x40,0x00,0x11,0x10,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*60:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x01,0x00,0x02,0x00,   0x00,0x00,0x00,0x00,

/*70:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*80:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*90:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*a0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*b0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*c0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*d0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*e0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,
/*f0:*/  0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00,

};


/**
 * Create a simulated instance of the SGDMA IP in the FPGA.
 * Everything is just arrays in user space in this program.
 * No real hardware is accessed and no driver is used.
 * @param pLog event log
 * @param drvrID  specify which device driver to use to access the board
 */
LSCPCIe2_Sim::LSCPCIe2_Sim(LatticeSemi_PCIe::SimDemoIP *pDemoIP, uint8_t *pCFG0, const PCIResourceInfo_t *pDrvrInfo) :
    LSCPCIe2_IF(NULL, "SIM", 0)
{

	ENTER();
	DEBUGSTR("Creating LSCPCIE2 Simulated Driver IF\n");

	this->pDemoIP = pDemoIP;

	if (pDrvrInfo != NULL)
	    memcpy(&PCIinfo, pDrvrInfo, sizeof(PCIinfo));
	else
	    memset(&PCIinfo, 0, sizeof(PCIinfo));

	if (pCFG0 != NULL)
	    memcpy(PCIinfo.PCICfgReg, pCFG0, sizeof(PCIinfo.PCICfgReg));
	else // Copy the canned PCI Config Space registers into the local class variable
	    memcpy(this->PCIinfo.PCICfgReg, CfgRegs, sizeof(this->PCIinfo.PCICfgReg));

	 
	pSysDmaBuf = (uint8_t *)malloc(16*1024);

	DEBUGPRINT(("hasInterrupt: %d  Vect: %d", this->PCIinfo.hasInterrupt, this->PCIinfo.intrVector));
	DEBUGPRINT(("Num BARs: %d", this->PCIinfo.numBARs));
	for (uint32_t i = 0; i <this->PCIinfo.numBARs; i++)
	{
		DEBUGPRINT(("BAR%d:  Addr: %x  Size: %d   Mapped: %d",this->PCIinfo.BAR[i].nBAR, this->PCIinfo.BAR[i].physStartAddr, this->PCIinfo.BAR[i].size, this->PCIinfo.BAR[i].memMapped));
	}

	LEAVE();
}


/**
 * Delete 
 */
LSCPCIe2_Sim::~LSCPCIe2_Sim()
{
	ENTER();
	free(pSysDmaBuf);

}




/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*    INTERRUPT SPECIFIC IMPLEMENTATIONS */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

// none right now


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*    READ/WRITE METHODS */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/**
 * Read 8 bits from an LSC FPGA register via PCIe bus.
 * @param addr BAR + address of device register to read from
 * @return value read or throw execption
 */
uint8_t LSCPCIe2_Sim::read8(uint32_t addr)
{
    uint8_t val;

	ENTER();

    if (this->pDemoIP->read8(addr, val))
	return(val);
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: read8 failed!"));
}
    

/**
 * Write 8 bits to an LSC hardware register via PCIe bus.
 * @param addr BAR + address of device register to write to
 * @param val value to write into the register
 */
void LSCPCIe2_Sim::write8(uint32_t addr, uint8_t val)
{

	ENTER();
    if (this->pDemoIP->write8(addr, val))
	return;
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: write8 failed!"));
}



/**
 * Read 16 bits from an FPGA register via PCIe bus.
 * @param addr BAR + address of device register to read from
 * @return true; false if address not multiple of 2
 */
uint16_t LSCPCIe2_Sim::read16(uint32_t addr)
{
    uint16_t val;

	ENTER();
    if (this->pDemoIP->read16(addr, val))
	return(val);
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: read16 failed!"));
}


/**
 * Write 16 bits to an SC hardware register via PCIe bus.
 * @param addr address of device register to write to
 * @param val value to write into the register
 * @return true; error in writing will cause hardware exception
 */
void LSCPCIe2_Sim::write16(uint32_t addr, uint16_t val)
{
	ENTER();

    if (this->pDemoIP->write16(addr, val))
	return;
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: write16 failed!"));
}



/**
 * Read 32 bits from an SC hardware register via PCIe bus.
 * This is done with 2 16 bit reads because the SC900 only has a 16 bit wide
 * data bus and will not allow accesses larger than 16 bits.
 * @param addr address of device register to read from
 * @return true; false if address not multiple of 4
 * @note error in reading will cause hardware exception
 */
uint32_t LSCPCIe2_Sim::read32(uint32_t addr)
{
    uint32_t val;
	ENTER();

    if (this->pDemoIP->read32(addr, val))
	return(val);
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: read32 failed!"));
}
    


/**
 * Write 32 bits to an SC hardware register via PCIe bus.
 * @param addr address of device register to write to
 * @param val value to write into the register
 * @return true; false if address not multiple of 4
 * @note error in reading will cause hardware exception
 */
void LSCPCIe2_Sim::write32(uint32_t addr, uint32_t val)
{
	ENTER();

    if (this->pDemoIP->write32(addr, val))
	return;
    else
	throw(PCIe_IF_Error("LSCPCIe2_Sim: write32 failed!"));
}


/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/
/*=================== BLOCK ACCESS METHODS ================*/

/**
 * Read a block of 8 bit registers from FPGA hardware via PCIe bus.
 * @param addr BAR + address of device register to read from
 * @param val location of storage for data read
 * @param len number of bytes to read
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error if driver fails
 */
bool LSCPCIe2_Sim::read8(uint32_t addr, uint8_t *val, size_t len, bool incAddr)
{
	ENTER();

	return(this->pDemoIP->read8(addr, val, len));
}
    

/**
 * Write a block of 8 bit registers into FPGA hardware via PCIe bus.
 * @param addr address of device register to write to
 * @param val location of bytes to write
 * @param len number of bytes to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true; error in writing will cause hardware exception
 * @note block size is not limitted
 */
bool LSCPCIe2_Sim::write8(uint32_t addr, uint8_t *val, size_t len, bool incAddr)
{
	ENTER();
	return(this->pDemoIP->write8(addr, val, len));

}



/**
 * Read a block of 16 bit registers from SC hardware via PCIe bus.
 * @param addr address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 16 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 * @note error in reading will cause hardware exception
 */
bool LSCPCIe2_Sim::read16(uint32_t addr, uint16_t *val, size_t len, bool incAddr)
{
	ENTER();

	return(this->pDemoIP->read16(addr, val, len));
}


/**
 * Write a block of 16 bit registers into FPGA hardware via PCIe bus.
 * @param addr BAR + address of device registers to write to
 * @param val location of 16 bit words to write
 * @param len number of 16 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 2
 * @note block size is  not limitted
 */
bool LSCPCIe2_Sim::write16(uint32_t addr, uint16_t *val, size_t len, bool incAddr)
{
	ENTER();

	return(this->pDemoIP->write16(addr, val, len));

}



/**
 * Read a block of 32 bit registers from FPGA hardware via PCIe bus.
 * @param addr BAR + address of device registers to read from
 * @param val location of storage for data read
 * @param len number of 32 bit words to read (not byte count)
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4
 */
bool LSCPCIe2_Sim::read32(uint32_t addr, uint32_t *val, size_t len, bool incAddr)
{
	ENTER();

	return(this->pDemoIP->read32(addr, val, len));
}
    


/**
 * Write a block of 32 bit registers into SC hardware via PCIe bus.
 * @param addr address of device registers to write to
 * @param val location of 32 bit words to write into SC
 * @param len number of 32 bit words to write
 * @param incAddr true if address increments for each operation, false to
 *        access the same address each time (FIFO access)
 * @return true;  false if address not multiple of 4
 * @note block size is not limitted, user buffer used directly
 */
bool LSCPCIe2_Sim::write32(uint32_t addr, uint32_t *val, size_t len, bool incAddr)
{
	ENTER();

	return(this->pDemoIP->write32(addr, val, len));
}



/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*                 P R I V A T E   M E T H O D S                              */
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/


/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*
 *       PCI Express Driver and Config Space Extensions to Access Layer
 */
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/

/**
 * Return the 256 bytes of the device's PCI configuration space registers.
 * These registers must be present on any PCI/PCIExpress device.
 * They have a standard format.
 * @param pCfg user's location to store 256 bytes
 * @return true if read byte OK, false if driver reports error
 */
bool LSCPCIe2_Sim::getPCIConfigRegs(uint8_t *pCfg)
{
	// Copy in the fake PCI Config Space info
	memcpy(pCfg, PCIinfo.PCICfgReg, sizeof(PCIinfo.PCICfgReg));

	return(true);
}




/**
 * Return the extra device driver information structure.
 * This includes the DMA memory buffer info, PCI bus/dev/func address.
 * @param pExtra user's pointer that will point to the internal driver structure
 * @return true (info obtained in constructor so always present)
 * @note Do not modify the contents of the structure!  It is read only.
 */
bool LSCPCIe2_Sim::getPciDriverExtraInfo(const ExtraResourceInfo_t **pExtra)
{
	*pExtra = &PCIExtraInfo;   // return pointer to the structure

	return(true);
}




bool LSCPCIe2_Sim::getPCIExtraInfoStr(string &outs)
{
 	ExtraResourceInfo_t *pExtra; 
	std::ostringstream oss;

 	pExtra = &PCIExtraInfo; 

	oss<<"\nlscpcie Driver Extra Info:\n";
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


bool LSCPCIe2_Sim::writeSysDmaBuf(uint32_t *pData, uint32_t len)
{
	memcpy(this->pSysDmaBuf, pData, len);
	return(true);

}


bool LSCPCIe2_Sim::readSysDmaBuf(uint32_t *pData, uint32_t len)
{
	memcpy(pData, this->pSysDmaBuf, len);
	return(true);

}



