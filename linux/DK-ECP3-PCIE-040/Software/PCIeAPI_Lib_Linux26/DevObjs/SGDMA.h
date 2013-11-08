/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file SGDMA.h */
 
#ifndef LATTICE_SEMI_PCIE_SGDMA_H
#define LATTICE_SEMI_PCIE_SGDMA_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <exception>

#include "dllDef.h"

#include "Device.h"
#include "RegisterAccess.h"
#include "LSCDMA_IF.h"

/*
 * Lattice Semiconductor Corp. namespace.
 */
namespace LatticeSemi_PCIe
{


#define SGDMA_ID_VALID    0x12040000

//----------------------------------------
//             DMA IP Defines
//----------------------------------------
#define BUS_A   0
#define BUS_B   1
#define PB      2

// SGDMA IP Registers
#define IPID_REG       0x00
#define IPVER_REG      0x04
#define GCONTROL_REG   0x08
#define GSTATUS_REG    0x0c
#define GEVENT_REG     0x10
#define GERROR_REG     0x14
#define GARBITER_REG   0x18
#define GAUX_REG       0x1c
#define CHAN_CTRL_BASE 0x200
#define BD_BASE        0x400


// Use with SRC_BUS() and DST_BUS() macros  to define BUS size
#define DATA_64BIT   3
#define DATA_32BIT   2
#define DATA_16BIT   1
#define DATA_8BIT    0

#define EOL       1
#define NEXT      0
#define SPLIT     2
#define LOCK      4
#define AUTORTY   8

#define ADDR_MODE_FIFO 0
#define ADDR_MODE_MEM  1
#define ADDR_MODE_LOOP 2

#define SRC_SIZE(a) (a<<10) // 0=8 bit, 1=16 bit, 2=32 bit, 3=64 bit
#define SRC_ADDR_MODE(a) (a<<13)  // 0=FIFO, 1=MEM, 2=LOOP
#define SRC_BUS(a) (a<<8)
#define SRC_MEM (1<<13)
#define SRC_FIFO (0<<13)

#define DST_SIZE(a) (a<<18) // 0=8 bit, 1=16 bit, 2=32 bit, 3=64 bit
#define DST_ADDR_MODE(a) (a<<21)  // 0=FIFO, 1=MEM, 2=LOOP
#define DST_BUS(a) (a<<16)
#define DST_MEM (1<<21)
#define DST_FIFO (0<<21)

#define CHAN_CTRL(n)   (0x200 + n * 32)   // Channel n Control Register
#define CHAN_STAT(n)   (0x204 + n * 32)   // Channel n status Register
#define CHAN_CURSRC(n) (0x208 + n * 32)   // Channel n Current Source being read
#define CHAN_CURDST(n) (0x20c + n * 32)   // Channel n Current Destination being written
#define CHAN_CURXFR(n) (0x210 + n * 32)   // Channel n Current Xfer Count still remaining
#define CHAN_PBOFF(n)  (0x214 + n * 32)   // Channel n Packet Buffer Offset

#define BD_CFG0(n) (0x400 + n * 16)   // Buffer Descriptor Config0 Register
#define BD_CFG1(n) (0x404 + n * 16)   // Buffer Descriptor Config1 Register
#define BD_SRC(n)  (0x408 + n * 16)   // Buffer Descriptor Source Address Register
#define BD_DST(n)  (0x40c + n * 16)   // Buffer Descriptor Destination Address Register

#define CHAN_STATUS_ENABLED  1
#define CHAN_STATUS_REQUEST  2
#define CHAN_STATUS_XFERCOMP 4
#define CHAN_STATUS_EOD      8
#define CHAN_STATUS_CLRCOMP  0x10
#define CHAN_STATUS_ERRORS   0x00ff0000

#define XFER_SIZE(a) (a)
#define BURST_SIZE(a) (a<<16)

#define PKT_BUF(x) (x) 
#define PKT_BUF_SIZE 4096   // this is the default SGDMAC Pkt Buf size (and what is instantiated)

/**
 * Specific definition of a Lattice Scatter Gather IP Core used in demos.
 *
 * This class provides test and debug access to the Lattice SGDMA IP Core for 
 * investigative purposes.  This class does not provide a complete, optimized 
 * driver solution.  For example, interrupts are not used (no convenient way to call-back
 * into user space).  The user is repsonsible for knowing and managing their channel
 * usage.  The methods do not know which channels are defined and which are read 
 * and which are write.  The class methods provide a general-purpose software view of
 * the SGDMA features and registers.  They do not use implementation specific features
 * such as interrupts, DMA_req lines, bus locking, packet buffer, etc. Triggering transfers is done
 * via software writing to the REQUEST bit.  Looking for completion is done by reading the XFERCOMP bit.
 * Things that are considered implementation specific (and not handled here):
 * <ul>
 * <li> use of and mapping of interrupts
 * <li> hardware device requesting transfer via DMA req port
 * <li> non-standard PCIe/SGDMA configuration
 * <li> use of packet buffer and split transfers
 * <li> whether arbiter included in IP core
 * </ul>
 * Be aware of the following:
 * <ul>
 * <li> Use only for testing SGDMA transfer scenarios and IP
 * <li> Do not use concurrent with driver initiated DMA transfers
 * <li> User must manage common buffer access - i.e. mutex
 * <li> best to only use in single threaded program - or else synchronize access
 * <li> no control of hardware or knowledge of implementation specifics
 * </ul>
 * <p>
 * This class inherits from the generic Device class to provide the actual read/write
 * methods.  The class needs a RegisterAccess object (the Driver Interface) to pass
 * to the Device class for the hardware access.
 * <p>
 * The SGDMA IP core is a standard product from Lattice Semiconductor.  The IP provides 
 * Scatter Gather DMA transfers between Wishbone bus slave devices.  The SGDMA core has
 * 2 Wishbone Master interfacest that initiate the bus transactions.  For PCIe applications,
 * one Wishbone Master interface is connected to the PCIe IP core and the other to a 
 * general purpose Wishbone bus that has the data devices for DMAing.  Please refer to the
 * SGDMA Data sheet for specific operation details.	See diagram below for compatible architecture.
 * <p>
 * The SGDMA has configuration registers for provisioning and controlling the transfers.
 * The registers are grouped as Global Configuration, Channel Configuration and Buffer
 * Descriptors.
 * <ul>
 * <li> 0x000 - Global Registers
 * <li> 0x200 - Channel Registers
 * <li> 0x400 - Buffer Descriptor Registers
 * </ul>
 *
 * <p>
 * This class expects the IP architecture to look like:
 \verbatim
 
          Memory Slave Devices
      |                 |        |
      |    Wishbone Bus |        |
   ======================================
              ^                     |
              |                     |
    ---------------------------     |
	|        WB_A             |     |
	|                         |     |
	|     SGDMA IP Core    Slv|<-----
	|                         |
	|         WB_B            |
    ---------------------------
	            |
				V
       ---------------------
	   | PCIe Adapter Core |
       ---------------------
	            |
				V
         ----------------
	     | PCIe IP Core |
         ----------------
 
 
 
\endverbatim
 *
 * <p>
 * All transfers setup by this class use the lscdma kernel-allocated Common Buffer for
 * the soruce/destination of the DMA transfer on the PC side.  The Common Buffer is used
 * because it is a contiguous block of fixed memory given to the driver.  Its physical
 * address is easily obtained and used for the programming the SGDMA channel registers.
 * The SGDMA core is not used in "scatter-gather" mode in the sense that it accesses
 * physically distributed user-space virtual memory pages that make up the user buffer.
 * Mapping virtual memory to a scatter-gather list requires many kernel API calls that
 * would become cumbersome to translate and carry back and forth across this interface,
 * plus would require re-creating these APIs as interfaces to the driver (IOCTL calls).
 * Therefore, true scatter-gather is performed entirely in the driver on dedicated 
 * functional devices.  This means that such a driver has IP architecture specific knowledge
 * and is not appropriate for a general purpose SGDMA class.  So, to be general purpose
 * this class does what is known as Common Buffer transfers.  Multiple BD's can still
 * be linked to simulate a "scatter-gather" list, but true virtual memory mapping is
 * not done here.
 *
 */
class DLLIMPORT SGDMA : public LatticeSemi_PCIe::Device
{
public:
	SGDMA(const char *nameStr, uint32_t addr, LatticeSemi_PCIe::LSCDMA_IF *pRA);
	virtual ~SGDMA();



	bool ReadFromCB(uint8_t chan,
		           size_t len, 
				   uint32_t destAddr, 
				   uint32_t destMode, 
				   uint32_t destSize, 
				   uint32_t numBDs=1,
				   uint32_t startBD=0);

	bool WriteToCB(uint8_t chan,
				   size_t len, 
				   uint32_t srcAddr, 
				   uint32_t srcMode, 
				   uint32_t srcSize, 
				   uint32_t numBDs=1,
				   uint32_t startBD=0);

	void 	enableCore(uint16_t chanMask);
	void 	disableCore(void);

	uint32_t getID(void);
	uint32_t getVersion(void);
	void 	getHdwParams(uint8_t &numChan, uint32_t &numBDs);
	void 	getChanStatusStr(string &outs, uint32_t chan);

	bool 	checkChanStatus(uint32_t chan);

	size_t	 getDrvrCB(uint32_t *buf, size_t len=0);
	size_t	 setDrvrCB(uint32_t *buf, size_t len=0);
	size_t	 getSizeDrvrCB(void);
	void 	fillDrvrCB(uint32_t val, size_t len=0);
	void 	fillPatternDrvrCB(uint32_t pattern, size_t len=0);
	void 	clearDrvrCB(void);
	int  	checkDrvrCB(size_t len=0);
	void 	showDrvrCB(size_t len=0);
	bool 	testDrvrCB(void);


	void	showGlobalRegs(void);
	void	showChannelRegs(void);
	void	showBufDescRegs(void);

	bool	verifyBufDescRegs(void);


private:
	LSCDMA_IF *pDrvr; 

	const DMAResourceInfo_t	*pDMAInfo;

	bool gotCBDMA;
	ULONG CBDMABufSize;
	ULONG physCBDMAAddr;
	uint8_t numChannels;
	uint32_t numBDs;

	uint32_t *ReadDmaBuf;
	uint32_t *WriteDmaBuf;

	uint32_t IPversion;
};

} // end namespace

#endif
