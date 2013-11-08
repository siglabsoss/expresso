/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file SGDMA.cpp */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "PCIeAPI.h"
#include "DebugPrint.h"
#include "SGDMA.h"
#include "MemFormat.h"

using namespace LatticeSemi_PCIe;


/**
 * private register defines of the SGDMA
 */
#define MAX_ADDRESS 0x1400
#define CHECK(x,y) if (x != y) {printf("Comparison Error Detected @ line: %d\n", __LINE__); return(false);}
/**
 * Construct access to a Lattice SGDMA IP module.
 * Register access to the IP in the FPGA device is through the 
 * Driver Interface object instantiated at the start of the program.
 * 
 * The base class Device is initialized to the base address and register size.
 *
 * @param nameStr the name of the FPGA device for unique naming
 * @param baseAddr the physical bus address the device registers start at
 * @param pRA pointer to the interface object to the lscdma device driver.
 * this clas requires access to the driver to open channels.  It also uses
 * the driver for the RegisterAccess object for accessing the hardware registers.
 */
SGDMA::SGDMA(const char *nameStr, 
	   uint32_t baseAddr, 
	   LSCDMA_IF *pRA) : LatticeSemi_PCIe::Device(pRA, nameStr, baseAddr, MAX_ADDRESS)
{
	uint32_t i;

	ENTER();

	// Save the driver for later opening handles to channels
	pDrvr = pRA;


	// Verify the ID register and throw exception if not a SGDMA 
	if ((this->getID() & 0xffff0000) != 0x12040000)
		throw(PCIe_IF_Error("SGDMA: Invalid Device ID!"));

	// Get the Version register because location of BD offset in channel register
	// changed between version 1.x and 2.x
	this->IPversion = this->getVersion();
	cout<<"SGDMA IP Version: 0x"<<hex<<this->IPversion<<dec<<endl;

	// Pass these in or read from Hardware?  Hardware should tell us.
	this->getHdwParams(numChannels, numBDs);

	// Get the DMA common buffer size, address, etc. for Common Buffer transfer modes
	pDrvr->getDriverDMAInfo(&pDMAInfo);
	if (pDMAInfo->hasDmaBuf)
	{
		gotCBDMA = true;
		CBDMABufSize = pDMAInfo->DmaBufSize;
		physCBDMAAddr = pDMAInfo->DmaPhyAddrLo;   // not supporting 64bit addr yet
		ReadDmaBuf = (uint32_t *)malloc(CBDMABufSize);
		WriteDmaBuf = (uint32_t *)malloc(CBDMABufSize);
		if (ReadDmaBuf == NULL)
			throw(PCIe_IF_Error("SGDMA: MALLOC ReadDmaBuf FAILED!"));
		if (WriteDmaBuf == NULL)
			throw(PCIe_IF_Error("SGDMA: MALLOC WriteDmaBuf FAILED!"));
	}
	else
	{
		gotCBDMA = false;
		CBDMABufSize = 0;
		physCBDMAAddr = 0;
		ReadDmaBuf = NULL;
		WriteDmaBuf = NULL;
	}

	// Clear the IP registers and disable everything
	//------------------------------------------------------------
	// Step 1: Reset all channels and disable DMA
	//------------------------------------------------------------

	write32(GCONTROL_REG, 0xffff0000);	 // disable and reset all channels and mask REQs
	write32(GSTATUS_REG, 0x00000000);	 // Disable SGDMA till ready
	write32(GEVENT_REG, 0xffff0000);	 // Disable event outputs
	write32(GERROR_REG, 0xffff0000);	 // mask error output

	//------------------------------------------------------------
	// Configure Arbiter
	//------------------------------------------------------------
	write32(GARBITER_REG, 0xffff148f);   // Global Arbiter Control

	//*************************************************************
	// For debugging and development: clear all channel regsiters
	// and all BD's so we can see if anything gets written to the
	// wrong place or gets accessed
	//*************************************************************
	for (i = 0; i < this->numChannels; i++)
	{
		write32(CHAN_CTRL(i), 0);
		write32(CHAN_STAT(i), 0);
		write32(CHAN_PBOFF(i), 0);
	}

	// Clear all BD's
	for (i = 0; i < this->numBDs; i++)
	{
		write32(BD_CFG0(i), 0);
		write32(BD_CFG1(i), 0);
		write32(BD_SRC(i), 0);
		write32(BD_DST(i), 0);
	}



	LEAVE();
}


/**
 * Destroy the SGDMA IP module. 
 * The device should no longer be accessed using any methods.  All references
 * to the device will now be invalid.
 */
SGDMA::~SGDMA()
{
	ENTER();


	// disable SGDMA functionality, especially interrupts
	write32(GCONTROL_REG, 0xffff0000);	 // disable and reset all channels and mask REQs
	write32(GSTATUS_REG, 0x00000000);	 // Disable SGDMA


	// Need to get the Common Buffer details
	free(ReadDmaBuf);
	free(WriteDmaBuf);

}






/**
 * Setup a channel to perform a Read operation and trigger it via software.
 * The SGDMA core performs the read operation and then halts.
 * A Read operation issues a series of MRd TLPs to the PC to read from the common buffer
 * driver memory.  The common buffer memory is used as the source of the transfer.
 * Any location on the Wishbone can be specified as the destination.
 * This method is primarily meant for testing DMA transfer operation and is not optimal.
 * The user must first load the Common Buffer using one of the other methods.
 * Max Read Request size is determined from the Link Capabilities.
 * Total transfer size is limitted by the Common buffer size which is usually under 64kB.
 * Number of BDs used can be programmed to demonstrate linking, but BDs can not specify under
 * 8 bytes minimum or more than 64kB maximum for a transfer.
 *
 * @note The channel must be linked to a Read adapter port to issue MRd and wait for CplD
 *
 * @param chan the channel to perform the read operation on
 * @param len the total number of bytes to transfer
 * @param destAddr slave address on wishbone bus - destination of transfer
 * @param destMode addressing mode; 0=FIFO, 1=linear, 2=loop
 * @param destSize data width; 0=8bit, 1=16bit, 2=32bit, 3=64bit
 * @param numBDs choose how many BDs to use, default=1.  Errors if greater than hardware has.
 * @param startBD choose where to start, default=0.  Errors if greater than hardware has.
 *
 * @note The channel must be setup in hardware to perform a read (MRd TLPs).
 */
bool SGDMA::ReadFromCB(uint8_t chan,
				   size_t len, 
				   uint32_t dstAddr, 
				   uint32_t dstMode, 
				   uint32_t dstSize, 
				   uint32_t nBDs, 
				   uint32_t bdStart)
{
	uint32_t i, rd32_val, err;

	uint32_t xferSize, blockSize, burstSize;
	uint32_t bd;
	uint32_t srcAddr;
	uint32_t gctrl_save;

	ENTER();

	err = 0;
	xferSize = len;

	if (chan >= this->numChannels)
	{
		ERRORSTR("Channel number invalid");
		return(false);   // ERROR! Channel not valid
	}
	   
	if (xferSize > CBDMABufSize)
	{
		ERRORSTR("DMA Len too large!");
		return(false);
	}
		
	if (xferSize < pDMAInfo->MaxReadReqSize)
		burstSize = xferSize;  // need to be equal (or xfer can be bigger)
	else
		burstSize = pDMAInfo->MaxReadReqSize;  // can't ask for more than this per TLP

	if (xferSize < 8)
	{
		ERRORSTR("DMA Len too small!");
		return(false);
	}

	if (bdStart + nBDs > this->numBDs)
	{
		ERRORSTR("BD_start + nBDs out of range!");
		return(false);
	}
 
	//------------------------------------------------------------
	// Step 1: Configure Buffer descriptors
	//------------------------------------------------------------
	printf("Setup %d buffer descriptors for read channel: %d\n", nBDs, chan);
	srcAddr = this->physCBDMAAddr;

	if (nBDs > 1)
		blockSize = xferSize / nBDs;
	else
		blockSize = xferSize;


//TODO make sure that the last transfer handles sub-burst length size if there is something
// left over that is less than burstSize i.e. sending 150 bytes = 128 + 22
// This is actually a bit more complicated because the if the burstSize changes, it needs to 
// be in a new BD.
// BD[0] = burst: 128, xfer: 128
// BD[1] = burst: 22, xfer:22
// 22 is not multiple of 64 bit, do I need to round up to 24?


	// xferSize = total amount to send (256, 1024, 32kB, etc.)
	// blockSize = per-buffer descriptor amount to send, if 4 BDs used then blockSize = xferSize / 4
	// burstSize = PCIe MaxPayload Size, can't send more than this per TLP
	// if sending 128 bytes in one BD, then xferSize = blockSize = burstSize
	bd = bdStart;
	for (i = 0; i < nBDs; i++)
	{
		printf("BD[%d]: src:%x -> dst:%x  len:%d\n", bd, srcAddr, dstAddr, blockSize);
	    if (i == (nBDs - 1))
			write32(BD_CFG0(bd), (DST_ADDR_MODE(dstMode) | DST_SIZE(dstSize)    | DST_BUS(BUS_A) |  \
						                         SRC_MEM | SRC_SIZE(DATA_64BIT) | SRC_BUS(BUS_B) | EOL));
	    else
			write32(BD_CFG0(bd), (DST_ADDR_MODE(dstMode) | DST_SIZE(dstSize)    | DST_BUS(BUS_A) |  \
						                         SRC_MEM | SRC_SIZE(DATA_64BIT) | SRC_BUS(BUS_B)));
	    write32(BD_CFG1(bd), BURST_SIZE(burstSize) | XFER_SIZE(blockSize));
	    write32(BD_SRC(bd), srcAddr);   	 //  source address = PC DMA buffer address
	    write32(BD_DST(bd), WB(dstAddr));	//  destination = wishbone slave device

	    srcAddr = srcAddr + blockSize;
		if (dstMode != ADDR_MODE_FIFO)
			dstAddr = dstAddr + blockSize;

	    ++bd;  // load next buffer descriptor
	}

	//------------------------------------------------------------
	// Step 2: Configure Channels
	//------------------------------------------------------------
	if (this->IPversion >= 0x02000000)
		write32(CHAN_CTRL(chan), (bdStart<<16));   // set BD base, no err mask
	else
		write32(CHAN_CTRL(chan), (bdStart<<8));   // set BD base, no err mask
	write32(CHAN_PBOFF(chan), 0x00000000);   //  Pkt Buf offset = 0 (1st block)


	//------------------------------------------------------------
	// Step 3:  Enable Core and the Channel
	// arbiter not really used now, but could be in future so allow for it
	//------------------------------------------------------------
	write32(GSTATUS_REG, 0xe0000000);   //  turn on the core and wishbone bus interfaces
	rd32_val = read32(GARBITER_REG);
	write32(GARBITER_REG, rd32_val & ~((1<<chan)<<16));   // unmask this channel for arbiter service
	rd32_val = read32(GCONTROL_REG);
	gctrl_save = rd32_val;
	write32(GCONTROL_REG, rd32_val | (1<<chan));   // Global Control:  enable channel & Req

	//------------------------------------------------------------
	// Step 4:  GO!!!!
	// Trigger transfer via software 
	//------------------------------------------------------------
	write32(CHAN_STAT(chan), 0x00000002);   // software trigger the transfer


	i = 0;
	do
	{
		Sleep(2);  // let it run then check if XFERCOMP bit is set 
		++i;  // timeout
	} while (((read32(CHAN_STAT(chan)) & 0x00000004) == 0) && (i < 1000));

	if (i >= 1000)
	{
		++err;
		ERRORSTR("XFER_TIMEOUT! Never Completed!");
	}

	// Check results
	if (checkChanStatus(chan) == false)
		++err;

	write32(CHAN_STAT(chan), 0x10);	// clear XFERCOMP so it can run again
	write32(GCONTROL_REG, gctrl_save);   // restore Global Control state

	if (err)
		return(false);
	else
		return(true);

}



/**
 * Setup a channel to perform a Write operation and trigger it via software.
 * The SGDMA core performs the write operation and then halts.
 * A Write operation issues a series of MWr TLPs to the PC to write into the common buffer
 * driver memory.  The common buffer memory is used as the destination of the transfer.
 * Any location on the Wishbone can be specified as the source.
 * This method is primarily meant for testing DMA transfer operation and is not optimal.
 * The user must read the Common Buffer contents using one of the other methods.
 * Max TLP size is determined from the Link Capabilities.
 * Total transfer size is limitted by the Common buffer size which is usually under 64kB.
 * Number of BDs used can be programmed to demonstrate linking, but BDs can not specify under
 * 8 bytes minimum or more than 64kB maximum for a transfer.
 *
 * @note The channel must be linked to a Write adapter port to issue MWr TLPs.
 *
 * @param chan the channel to perform the write operation; 0-15
 * @param len the total number of bytes to transfer
 * @param srcAddr slave address on wishbone bus - source of transfer
 * @param srcMode addressing mode; 0=FIFO, 1=Linear, 2=Loop
 * @param srcSize data width; 0=8bit, 1=16bit, 2=32bit, 3=64bit
 * @param numBDs choose how many BDs to use, default=1.  Errors if greater than hardware has.
 * @param startBD choose where to start, default=0.  Errors if greater than hardware has.
 *
 */
bool SGDMA::WriteToCB(uint8_t chan,
				   size_t len, 
				   uint32_t srcAddr, 
				   uint32_t srcMode, 
				   uint32_t srcSize, 
				   uint32_t nBDs, 
				   uint32_t bdStart)
{
	uint32_t i, rd32_val, err;

	uint32_t xferSize, blockSize, burstSize;
	uint32_t bd;
	uint32_t dstAddr;
	uint32_t gctrl_save;

	ENTER();

	err = 0;
	xferSize = len;

	if (chan >= this->numChannels)
	{
		ERRORSTR("Channel number invalid");
		return(false);   // ERROR! Channel not valid
	}
	   
	if (xferSize > CBDMABufSize)
	{
		ERRORSTR("DMA Len too large!");
		return(false);
	}
		
	if (xferSize < pDMAInfo->MaxPayloadSize)
		burstSize = xferSize;  // need to be equal (or xfer can be bigger)
	else
		burstSize = pDMAInfo->MaxPayloadSize;  // can't write more than this per TLP

	if (xferSize < 8)
	{
		ERRORSTR("DMA Len too small!");
		return(false);
	}

	bdStart = 0;   // if want to offset the starting BD, ie. use 128 BD's starting at 128 to 255
	if (bdStart + nBDs > this->numBDs)
	{
		ERRORSTR("BD_start + nBDs out of range!");
		return(false);
	}
 
	//------------------------------------------------------------
	// Step 1: Configure Buffer descriptors
	//------------------------------------------------------------
	printf("Setup %d buffer descriptors for write channel: %d\n", nBDs, chan);
	dstAddr = this->physCBDMAAddr;

	if (nBDs > 1)
		blockSize = xferSize / nBDs;
	else
		blockSize = xferSize;


//TODO make sure that the last transfer handles sub-burst length size if there is something
// left over that is less than burstSize i.e. sending 150 bytes = 128 + 22
// This is actually a bit more complicated because the if the burstSize changes, it needs to 
// be in a new BD.
// BD[0] = burst: 128, xfer: 128
// BD[1] = burst: 22, xfer:22
// 22 is not multiple of 64 bit, do I need to round up to 24?


	// xferSize = total amount to send (256, 1024, 32kB, etc.)
	// blockSize = per-buffer descriptor amount to send, if 4 BDs used then blockSize = xferSize / 4
	// burstSize = PCIe MaxPayload Size, can't send more than this per TLP
	// if sending 128 bytes in one BD, then xferSize = blockSize = burstSize
	bd = bdStart;
	for (i = 0; i < nBDs; i++)
	{
		printf("BD[%d]: src:%x -> dst:%x  len:%d\n", bd, srcAddr, dstAddr, blockSize);
	    if (i == (nBDs - 1))
			write32(BD_CFG0(bd), (DST_MEM | DST_SIZE(DATA_64BIT) | DST_BUS(BUS_B) |  \
						  SRC_ADDR_MODE(srcMode) | SRC_SIZE(srcSize) | SRC_BUS(BUS_A) | EOL));
	    else
			write32(BD_CFG0(bd), (DST_MEM | DST_SIZE(DATA_64BIT) | DST_BUS(BUS_B) |  \
					     SRC_ADDR_MODE(srcMode) | SRC_SIZE(srcSize) | SRC_BUS(BUS_A) ));
	    write32(BD_CFG1(bd), BURST_SIZE(burstSize) | XFER_SIZE(blockSize));
	    write32(BD_SRC(bd), WB(srcAddr));	 //  source address
	    write32(BD_DST(bd), dstAddr);	//  destination = PC DMA buffer address

		if (srcMode != ADDR_MODE_FIFO)
			srcAddr = srcAddr + blockSize;
	    dstAddr = dstAddr + blockSize;

	    ++bd;
	}

	//------------------------------------------------------------
	// Step 2: Configure Channels
	//------------------------------------------------------------
	if (this->IPversion >= 0x02000000)
		write32(CHAN_CTRL(chan), (bdStart<<16));   // set BD base, no err mask
	else
		write32(CHAN_CTRL(chan), (bdStart<<8));   // set BD base, no err mask
	write32(CHAN_PBOFF(chan), 0x00000000);   //  Pkt Buf offset = 0 (1st block)


	//------------------------------------------------------------
	// Step 3:  Enable Core and the Channel
	// arbiter not really used now, but could be in future so allow for it
	//------------------------------------------------------------
	write32(GSTATUS_REG, 0xe0000000);   //  turn on the core and wishbone bus interfaces
	rd32_val = read32(GARBITER_REG);
	write32(GARBITER_REG, rd32_val & ~((1<<chan)<<16));   // unmask this channel for arbiter service
	rd32_val = read32(GCONTROL_REG);
	gctrl_save = rd32_val;
	write32(GCONTROL_REG, rd32_val | (1<<chan));   // Global Control:  enable channel the channel
	// Note: the DMA_REQ is left masked because this is a hdw signal input.  Not needed for sw trigger.

	//------------------------------------------------------------
	// Step 4:  GO!!!!
	// Trigger transfer via software 
	//------------------------------------------------------------
	write32(CHAN_STAT(chan), 0x00000002);   // software trigger the transfer


	i = 0;
	do
	{
		Sleep(2);  // let it run then check if XFERCOMP bit is set 
		++i;  // timeout
	} while (((read32(CHAN_STAT(chan)) & 0x00000004) == 0) && (i < 1000));

	if (i >= 1000)
	{
		++err;
		ERRORSTR("XFER_TIMEOUT! Never Completed!");
	}

	// Check results
	if (checkChanStatus(chan) == false)
		++err;

	//-----------------------------------------------------
	//  Clean-up Channel, restore its original state
	//-----------------------------------------------------
	write32(CHAN_STAT(chan), 0x10);	// clear XFERCOMP so it can run again
	write32(GCONTROL_REG, gctrl_save);   // restore Global Control state

	if (err)
		return(false);
	else
		return(true);
}



/**
 * Enable the IP core and specified channels.
 * Enabling enables the core and the Wishbone buses.  It also enables
 * channels, which will also enable the channel in the arbiter mask.
 * Hardware request masks and event masks are left disabled.
 * @param chanMask set bit position to a 1 to enable that channel.
 * 
 * @throws exception if register read or write fails
 */
void SGDMA::enableCore(uint16_t chanMask)
{
	uint32_t mask, regVal;
	ENTER();

	mask = (~chanMask)<<16;
	regVal = read32(GARBITER_REG);
	write32(GARBITER_REG, regVal | mask);  // turn on channels for arbiter scheduling

	write32(GSTATUS_REG, 0xe0000000);     // turn on Core and Wishbone buses

	regVal = read32(GARBITER_REG);
	write32(GCONTROL_REG, regVal | chanMask);  // turn on channels (hdw req mask should be left set)
}


/**
 * Disable the IP core.
 * Turn off all channels, turn off Bus masters, turn off core, mask everything.
 * Core should not do anything anymore.
 * Use to halt core for diagnostics to ensure its not running or could be triggered
 * to run when peeking an dpoking registers.
 * @throws exception if register read or write fails
 */
void SGDMA::disableCore(void)
{
	ENTER();
	write32(GCONTROL_REG, 0xffff0000);	 // disable and reset all channels and mask REQs
	write32(GSTATUS_REG, 0x00000000);	 // Disable SGDMA till ready
	write32(GEVENT_REG, 0xffff0000);	 // Disable event outputs
	write32(GERROR_REG, 0xffff0000);	 // mask error output

}




/**
 * Read the 32bit SGDMA IP ID register.
 * @return 32bit SGDMA ID value
 * @throws exception if register read fails
 */
uint32_t SGDMA::getID(void)
{
	ENTER();

	return(read32(IPID_REG));
}



/**
 * Read the 32bit SGDMA IP Version register.
 * @return 32bit SGDMA Version value
 * @throws exception if register read fails
 */
uint32_t SGDMA::getVersion(void)
{
	ENTER();

	return(read32(IPVER_REG));
}


/**
 * Read the 32bit SGDMA IP Version Register and parse out the
 * current hardware build configuration.
 * @note The BDs are fixed at 256 until the SGDMA IP core has this info in it
 * @param numChan returns the number of channels the IP was built with
 * @param numBDs returns the number of BDs 
 * @throws exception if register read register fails
 */
void SGDMA::getHdwParams(uint8_t &numChan, uint32_t &numBDs)
{
	numChan = (uint8_t)((read32(IPVER_REG))>>12) & 0x1f;
	++numChan;  // what's in the hdw register is really the max chan#.  How many = +1
	numBDs = 256;
}


/**
 * Display all SGDMA Global Registers.
 * Use for debugging and testing to dump the SGDMA register values.
 *
 */
void	SGDMA::showGlobalRegs(void)
{
	int i;

	printf("\n\n\n=====================================================\n");
	printf("SGDMA: Global Register Contents\n");
	printf("=====================================================\n");
	// Read all the Global registers
	for (i = 0; i < 0x20; i = i + 4)
	{
		printf("SGDMA[%x] = %08x\n", i , read32(i));
	}
}

/**
 * Display all SGDMA Channel Config Registers.
 * Use for debugging and testing to dump the SGDMA register values.
 */
void	SGDMA::showChannelRegs(void)
{
	int i, j;
	int ch;

	printf("\n\n\n=====================================================\n");
	printf("SGDMA: Channel Register Contents\n");
	printf("=====================================================\n");
	ch = 0;
	for (i = 0x200; i < 0x200 + this->numChannels*0x20; i = i + 0x20)
	{
		printf("Chan[%d]\n", ch);
		for (j = 0; j < 0x18; j = j + 4)
		{
			printf("\tSGDMA[%x] = %08x\n", i+j , read32(i+j));
		}
		++ch;
	}
}

 
/**
 * Display all SGDMA Buffer Descriptor Registers.
 * Use for debugging and testing to dump the SGDMA register values.
 */
void	SGDMA::showBufDescRegs(void)
{
	uint32_t j, i;
	uint32_t bd;

	printf("\n\n\n=====================================================\n");
	printf("\n\nSGDMA: Buffer Desciptor Registers\n");
	printf("=====================================================\n");
	bd = 0;
	for (i = 0x400; i < 0x400 + this->numBDs*0x10; i = i + 0x10)
	{
			printf("BD[%d]:\n", bd);
			for (j = 0; j < 0x10; j = j + 4)
			{
				printf("\tSGDMA[%x] = %08x\n", i+j , read32(i+j));
			}

		++bd;
	}
}

 
/**
 * Verify access to all SGDMA Buffer Descriptor Registers.
 * Use for debugging and testing to validate that all registers can
 * be written and read and have the correct values.
 * @warning Do not execute while DMA transfers are in progress!
 */
bool	SGDMA::verifyBufDescRegs(void)
{
	uint32_t i;
	uint32_t bd;

	// first load unique pattern into every registers of every buf desc.
	bd = 0;
	for (i = 0x400; i < 0x400 + this->numBDs*0x10; i = i + 0x10)
	{
		write32(i, bd);
		write32((i + 4), (bd<<8) | bd);
		write32((i + 8), (bd<<16) | (bd<<8) | bd);
		write32((i + 12), (bd<<24) | (bd<<16) | (bd<<8) | bd);
		++bd;
	}

	// then verify that all registers have their programmed value
	bd = 0;
	for (i = 0x400; i < 0x400 + this->numBDs*0x10; i = i + 0x10)
	{

		CHECK((ULONG)bd, read32(i));
		CHECK((ULONG)((bd<<8) | bd), read32(i + 4));
		CHECK((ULONG)((bd<<16) | (bd<<8) | bd), read32(i + 8));
		CHECK((ULONG)((bd<<24) | (bd<<16) | (bd<<8) | bd), read32(i + 12));

		++bd;
	}

	return(true);

}

/**
 * Return the Channel's status registers in a formatted string.
 * @param outs the string to place the formatted values of the registers in
 * @param chan the SGDMA channel to operate on
 */
void SGDMA::getChanStatusStr(string &outs, uint32_t chan)
{
	uint32_t rd32_val;
	uint32_t chan_base;

	std::ostringstream oss;
	oss<<std::setbase(16);

	oss<<"CHAN["<<chan<<"]:\n";
	chan_base = 0x200 + chan * 0x20;
	rd32_val = read32(chan_base + 0x04);
	oss<<"   status: "<<hex<<rd32_val<< "=>[";
	oss<<" BD="<<dec<<(rd32_val>>24);
	if (rd32_val & 0x10000)
		cout<<" BusErr";
	if (rd32_val & 0x20000)
		cout<<" AddrErr";
	if (rd32_val & 0x40000)
		cout<<" TimeOut";
	if (rd32_val & 0x80000)
		cout<<" RtyErr";
	oss<<" STATE="<<hex<<((rd32_val>>12) & 0x0f);
	oss<<" RTY="<<dec<<((rd32_val>>7) & 0x1f);
	if (rd32_val & 0x00010)
		oss<<" ClrComp";
	if (rd32_val & 0x00008)
		oss<<" EOD";
	if (rd32_val & 0x00004)
		oss<<" XferComp";
	if (rd32_val & 0x00002)
		oss<<" Req";
	if (rd32_val & 0x00001)
		oss<<" En";
	oss<<"]"<<endl;

	rd32_val = read32(chan_base + 0x08);
	oss<<"   src_addr: "<<hex<<rd32_val<<endl;
	rd32_val = read32(chan_base + 0x0c);
	oss<<"   dst_addr: "<<hex<<rd32_val<<endl;
	rd32_val = read32(chan_base + 0x10);
	oss<<"   xfr_cnt: "<<hex<<rd32_val<<endl;

}

/**
 * Verify the Channels status registers don't show errors
 * and are done the transfer.
 * @param chan the channel number to check (0-15)
 * @return true if no errors, false if errors
 */
bool SGDMA::checkChanStatus(uint32_t chan)
{
	uint32_t rd32_val;
	uint32_t chan_base;
	int errs = 0;
	string statusStr;

	// First show the Channel's status registers
	getChanStatusStr(statusStr, chan);
	cout<<statusStr<<endl;

	chan_base = 0x200 + chan * 0x20;
	rd32_val = read32(chan_base + 0x04);
	if (rd32_val & 0xf0000)
	{
		cout<<"\nDMA FAILED: Chan has errors!";  // Errors set or not done yet!
		++errs;
	}

	if (!(rd32_val & 0x00004))
	{
		cout<<"\nDMA FAILED: Chan not done xfer.";	 // Errors set or not done yet!
		++errs;
	}

	rd32_val = read32(chan_base + 0x10);
	if ((rd32_val & 0xffff) != 0)
	{
		cout<<"\nDMA FAILED: Chan xfer_cnt not 0.";  // not done yet!
		++errs;
	}

	if (errs)
		return(false);
	else
		return(true);
}

 
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
//        SYSTEM DMA MEMORY TESTING FUNCTIONS (in kernel space)
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/


/**
 * Return the contents of the Driver's Common Buffer into the user supplied buffer.
 * The DMA Common Buffer, starting at offset 0, is read for len bytes into buf.
 * @param buf pointer to storage for bytes read from driver buffer
 * @param len number of bytes to read. Optional: if not given all read
 * @note Use getSizeDrvrCB() to ensure adequate storage has been allocated.
 * @note len should be a multiple of 4.
 * @return the number of bytes loaded into buf.
 */
size_t SGDMA::getDrvrCB(uint32_t *buf, size_t len)
{
	if ((len == 0) || (len > CBDMABufSize))
		len = CBDMABufSize;

	if (pDrvr->readSysDmaBuf(buf, len))
		return(len);
	else
		return(0);

}

/**
 * Load the contents of the Driver's Common Buffer with the user supplied buffer.
 * The DMA Common Buffer, starting at offset 0, is loaded with len bytes from buf.
 * @param buf pointer to storage for bytes read from driver buffer
 * @param len number of bytes to written. Optional: if not given, load entire buffer
 * @note Use getSizeDrvrCB() to ensure adequate storage has been allocated.
 * @note len should be a multiple of 4.
 * @return the number of bytes written into the driver buffer.
 */
size_t SGDMA::setDrvrCB(uint32_t *buf, size_t len)
{
	if ((len == 0) || (len > CBDMABufSize))
		len = CBDMABufSize;

	if (pDrvr->writeSysDmaBuf(buf, len))
		return(len);
	else
		return(0);


}


/**
 * Return the size of the Driver's Common Buffer.
 * Used so user code can allocate a buffer this size to use for get/setDrvrCB().
 * @return the size of the buffer in bytes.
 */
size_t SGDMA::getSizeDrvrCB(void)
{
	return(CBDMABufSize);
}



/**
 * Fill System DMA memory range with a specified value.
 * The global WrBuf[] is first loaded with the value, then the contents of the
 * WrBuf[] are transfered to the system buffer, using the driver ioctl call.
 * WrBuf[] is then used as the comparison in checkEBR() or checkDrvrCB().
 *
 * @param val 32 bit value written into each 32bit location
 * @param len in bytes, optional: if not given defaults to entire buf length
 */
void SGDMA::fillDrvrCB(uint32_t val, size_t len)
{
	size_t i;

	// First fill the write buffer using the 32bit values
	if (len == 0)
		len = CBDMABufSize;
	else if (len > CBDMABufSize)
		len = CBDMABufSize;

	for (i = 0; i < len/4; i++)
	{
		WriteDmaBuf[i] =  val;
	}

	// Then transfer WriteDmaBuf[] into the system common DMA buffer
	pDrvr->writeSysDmaBuf(WriteDmaBuf, len);

}


/**
 * Fill System DMA memory range with a specific pattern.
 * The global WrBuf[] is first loaded with the pattern, then the contents of the
 * WrBuf[] are transfered to the system buffer, using the driver ioctl call.
 * WrBuf[] is then used as the comparison in checkEBR() or checkSysDmaBuf().
 * The pattern is the upper 16 bits of input param pattern OR'ed an
 * incrementing value per word written.  Example: aa550000 aa550001 aa550002 ...
 *
 * @param pattern upper 16 bits will be preserved and written into each 32bit location
 * @param len in bytes, optional: if not given defaults to entire buf length
 */
void SGDMA::fillPatternDrvrCB(uint32_t pattern, size_t len)
{
	size_t i;

	// First fill the write buffer using the 32bit pattern values
	if (len == 0)
		len = CBDMABufSize;
	else if (len > CBDMABufSize)
		len = CBDMABufSize;

	for (i = 0; i < len/4; i++)
	{
		WriteDmaBuf[i] =  pattern | i;
	}

	// Then transfer WriteDmaBuf[] into the system common DMA buffer
	pDrvr->writeSysDmaBuf(WriteDmaBuf, len);

}


/**
 * Fill the system Common Buffer DMA buffer with all 0's.
 */
void SGDMA::clearDrvrCB(void)
{
	memset(WriteDmaBuf, 0x00, CBDMABufSize);
	pDrvr->writeSysDmaBuf(WriteDmaBuf, CBDMABufSize);
}


/**
 *
 * Checking the buffer compares what is in the WriteDmaBuf with what is read back
 * into the ReadDmaBuf.  The usage would be to load the WriteDmaBuf using a
 * fillSysDmaBuf() call.  Then transfer the DMA buf to the destination device, and
 * read back with 
 * @param len in bytes, optional: if not given defaults to entire buf length
 */
int SGDMA::checkDrvrCB(size_t len)
{
	size_t i;
	int errs;

	// WrBuf must still contain same pattern used in call to fillEBR() or fillSysDmaBuf()

	if ((len == 0) || (len > CBDMABufSize))
		len = CBDMABufSize;

	// Compare WrBuf[] with the values read from the DMA buffer

	pDrvr->readSysDmaBuf(ReadDmaBuf);

	errs = 0;
	for (i = 0; i < len/4; ++i)
	{
		if (WriteDmaBuf[i] != ReadDmaBuf[i])
			++errs;
	}

	if (errs)
	{
		cout<<"\tcheckSysDmaBuf: Errors!!! ReadBuf != WriteBuf"<<endl;
		//throw(PCIe_IF_Error("CHECK_SysDmaBuf FAILED COMPARISON."));
	}
	else
	{
		cout<<"\tcheckSysDmaBuf==>PASS"<<endl;
	}
	return(errs);
}



/**
 * Display the contents of the System DMA buffer.
 * @param len = number of bytes to show, Optional: if not given defaults to all
 */
void SGDMA::showDrvrCB(size_t len)
{
	string ostr;


	if ((len == 0) || (len > CBDMABufSize))
		len = CBDMABufSize;


	cout<<"\nSystem Common Buffer Contents Display";

	pDrvr->readSysDmaBuf(ReadDmaBuf);
	MemFormat::formatBlockOfWords(ostr, 0, len, ReadDmaBuf);
	cout<<ostr<<endl;

}

/**
 * Run test to verify that the Common Buffer memory in the driver can be
 * accessed and can be written with values and read back with values and
 * the contents match.
 * The entire contents are checked.
 * @return true if all test operations succeed.
 */
bool SGDMA::testDrvrCB(void)
{

	fillPatternDrvrCB(0x12340000);

	// Compare WrBuf[] with the values read from the DMA buffer
	if (checkDrvrCB() == 0)
		return(true);
	else
		return(false);

}


