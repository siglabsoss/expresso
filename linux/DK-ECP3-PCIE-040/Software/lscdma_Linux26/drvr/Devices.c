/** @file Devices.c */

/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/
/*
 *     HARDWARE REGISTER ACCESS ROUTINES
 */
/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/

#include "lscdma.h"

/**
 * Read a 8 bit hardware register and return the word.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * The goal is speed here.
 * 
 * @param pBrd pointer to the Board to get hardware BAR address
 * @param offset in board's mapped control BAR to read from
 * @return 8 bit value read from the hardware register
 */
inline UCHAR rdReg8(pcie_board_t *pBrd, ULONG offset)
{
	return(readb(pBrd->ctrlBARaddr + offset));
}




/**
 * Read a 16 bit hardware register and return the word.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * The goal is speed here.
 * 
 * @param pBrd pointer to the Board to get hardware BAR address
 * @param offset in board's mapped control BAR to read from
 * @return 16 bit value read from the hardware register
 */
inline USHORT rdReg16(pcie_board_t *pBrd, ULONG offset)
{
	return(readw(pBrd->ctrlBARaddr + offset));
}




/**
 * Read a 32 bit hardware register and return the word.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * The goal is speed here.
 * 
 * @param pBrd pointer to the Board to get hardware BAR address
 * @param offset in board's mapped control BAR to read from
 * @return 32 bit value read from the hardware register
 */
inline ULONG rdReg32(pcie_board_t *pBrd, ULONG offset)
{
	return(readl(pBrd->ctrlBARaddr + offset));
}




/**
 * Write a 8 bit value to a hardware register.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * 
 * @param pBrd pointer to Board to get hardware BAR address
 * @param offset in the board's mapped control BAR to write to
 * @param val the 8 bit value to write
 */
inline void wrReg8(pcie_board_t *pBrd, ULONG offset, UCHAR val)
{
	writeb(val, pBrd->ctrlBARaddr + offset);
}



/**
 * Write a 16 bit value to a hardware register.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * 
 * @param pBrd pointer to Board to get hardware BAR address
 * @param offset in the board's mapped control BAR to write to
 * @param val the 16 bit value to write
 */
inline void wrReg16(pcie_board_t *pBrd, ULONG offset, USHORT val)
{
	writew(val, pBrd->ctrlBARaddr + offset);
}

/**
 * Write a 32 bit value to a hardware register.
 * @note No parameter checking is done.  The caller is responsible for ensuring
 * the hardware is still mapped (i.e. device state != stopped or removed) and
 * valid.
 * 
 * @param pBrd pointer to Board to get hardware BAR address
 * @param offset in the board's mapped control BAR to write to
 * @param val the long value to write
 */
inline void wrReg32(pcie_board_t *pBrd, ULONG offset, ULONG val)
{
	writel(val, pBrd->ctrlBARaddr + offset);
}




/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
/*
 *           Utilities
 */
/*========================================================================*/
/*========================================================================*/
/*========================================================================*/

/**
 * Read the 256 bytes of PCI Cfg0 register space from the device hardware.
 * The register data is saved in the Device Extension PCICfgRegs array.
 *
 */
int ReadPCIConfigRegs(pcie_board_t *pBrd)
{
	int status;
	int i;

	for (i = 0; i < 0x100;  i = i + 4)
	{
		status = pci_read_config_dword(pBrd->pPciDev, i, &(pBrd->PCICfgRegs[i/4]));
		if (status)
		{
			printk(KERN_ERR "lscdma: calling pci_read_config_dword(%d)\n", i);
			return(status);
		}
	}
	

	if (DrvrDebug)
	{
		printk("PCICfgRegs 00: %x %x %x %x\n", pBrd->PCICfgRegs[0], pBrd->PCICfgRegs[1], 
							pBrd->PCICfgRegs[2], pBrd->PCICfgRegs[3]);
		printk("PCICfgRegs 10: %x %x %x %x\n", pBrd->PCICfgRegs[4], pBrd->PCICfgRegs[5], 
							pBrd->PCICfgRegs[6], pBrd->PCICfgRegs[7]);
		printk("PCICfgRegs 20: %x %x %x %x\n", pBrd->PCICfgRegs[8], pBrd->PCICfgRegs[9], 
							pBrd->PCICfgRegs[10], pBrd->PCICfgRegs[11]);
		printk("PCICfgRegs 30: %x %x %x %x\n", pBrd->PCICfgRegs[12], pBrd->PCICfgRegs[13], 
							pBrd->PCICfgRegs[14], pBrd->PCICfgRegs[15]);

	}

	return(status);
}



/**
 * Process the pre-loaded PCI Config Space 0 registers and extract
 * the PCIe link capabilities from it.  The link info is set into
 * the PCIeMaxPayloadSize, PCIeMaxReadReqSize, PCIeLinkWidth and
 * PCIeRCBSize varaibles in the device extension.
 * @note This function requires the PciCfg buf to have been loaded
 * by a call to ReadPCIConfigRegs(), which is normally done at the
 * very beginning of StartDevice.
 * @return true if PCIE Cpabailties structure was found and parsed
 * false if errors in parsing.
 */
int ParsePCIeLinkCap(pcie_board_t *pBrd)
{

	int i, id, next, index;
	u8 *buf;
	u32 *pPCI;
	u8 *p8;
	u16 *p16;
	u32 *p32;
	bool found = false;


	pPCI = pBrd->PCICfgRegs;
	buf = (u8 *)pBrd->PCICfgRegs;

	if (ReadPCIConfigRegs(pBrd) != 0)
	{
		printk(KERN_ERR "lscdma: Error Reading Config Regs!\n");
		return(-1);
	}
		

	// Now parse the capabilities structures.  The first structure in the list
	// is pointed to by the Capabilities Ptr at location 0x34.  If this is 0
	// or the capabilities bit in the status is 0 then there are none.
	if (((pPCI[STAT_CMD_REG] & 0x00100000) == 0) || (pPCI[CAP_PTR_REG] == 0))
	{
		printk(KERN_ERR "lscdma: No PCIE Capabilities Structure!\n");
		return(-1);
	}

	i = 0;
	next = (int)pPCI[CAP_PTR_REG] & 0x000000ff;
	while ((next >= 0x40) && (i < 16))
	{
		++i;  // loop counter to prevent circular loop
		index = next;
		id = buf[next];
		p8 = (u8 *)&buf[next];
		p16 = (u16 *)&buf[next];
		p32 = (u32 *)&buf[next];
		next = (int)buf[next + 1];
		switch(id)
		{
			case 1:  // Power Management
				printk("lscdma: Power Management Capability Structure @ %x\n", index);
				break;

			case 2:  // AGP Capability
				printk("lscdma: AGP Capability Structure @ %x\n", index);
				break;

			case 3:  // VPD (Vital Product Data) Capability
				printk("lscdma: VPD Capability Structure @ %x\n", index);
				break;

			case 4:  // Slot ID Capability
				printk("lscdma: Slot ID Capability Structure @ %x\n", index);
				break;

			case 5:  // MSI
				printk("lscdma: MSI Capability Structure @ %x\n", index);
				break;

			case 6:  // CompactPCI Hot Swap
				printk("lscdma: CompactPCI Capability Structure @ %x\n", index);
				break;

			case 7:  // PCI-X
				printk("lscdma: PCI-X Capability Structure @ %x\n", index);
				break;

			case 8:  // AMD
				printk("lscdma: AMD Capability Structure @ %x\n", index);
				break;

			case 9:  // Vendor Specific
				printk("lscdma: Vendor Specific Capability Structure @ %x\n", index);
				break;

			case 0x0a:  // Debug Port
				printk("lscdma: Debug Port Capability Structure @ %x\n", index);
				break;

			case 0x0b:  // CompactPCI central resource control
				printk("lscdma: CompactPCI resource Capability Structure @ %x\n", index);
				break;

			case 0x0c:  // PCI Hot Plug
				printk("lscdma: PCI Hot Plug Capability Structure @ %x\n", index);
				break;

			case 0x10: // PCI Express
				printk("lscdma: PCI Express Capability Structure @ %x\n", index);
				pBrd->PCIeMaxReadReqSize = (128<<((p16[4] & 0x7000)>>12));
				pBrd->PCIeMaxPayloadSize = (128<<((p16[4] & 0x0e)>>5));

				if (p16[8] & 0x0008)
					pBrd->PCIeRCBSize = 128;
				else
					pBrd->PCIeRCBSize = 64;

				 pBrd->PCIeLinkWidth = ((p16[9] & 0x03f0)>>4);

				 printk("lscdma: MaxPayloadSize = %d\n", pBrd->PCIeMaxPayloadSize);
				 printk("lscdma: MaxReadReqSize = %d\n", pBrd->PCIeMaxReadReqSize);
				 printk("lscdma: RCBSize = %d\n", pBrd->PCIeRCBSize);
				 printk("lscdma: LinkWidth = x%d\n", pBrd->PCIeLinkWidth);

				// Slot Registers and Root Registers not implemented by our EndPoint core
				found = true;

				break;


			default:
				return( -1);
				break;

		}

	}


	if (found)
		return(0);
	else
		return( -1);

}



/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/
/*
 *     GPIO IP BLOCK ROUTINES
 *
 * GPIO block has a down counter that can interrupt, so make sure its off.
 * The interrupt controller is also in the GPIO block so disable all interrupt sources
 * during initialization.  The individual devices unmask their interrupt during
 * normal operation, so default state is masked during startup.
 */
/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/

/**
 * Initialize the GPIO IP module.
 * Disable counter circuits and DMA triggers.
 * @param pBrd pointer to device extension to get hardware BAR address
 */
int GPIO_Setup(pcie_board_t *pBrd)
{
	//--------------------------------------------------------------------------
	// Verify that the hardware we're accessing has the correct IP blocks and 
	// versions.  If the ID registers don't match what we expect then abort
	// becasue we don't want to take the chance of enabling interrupts but not
	// being able to control them!  or trying to program SGDMA or other registers
	// and not have the right memory map!
	//--------------------------------------------------------------------------
	if (rdReg32(pBrd, GPIO(GPIO_ID_REG)) != GPIO_ID_VALID)
	 {
		 printk(KERN_ERR "ERROR! GPIO IP not found!\n");
		 return (-ENODEV);
	 }


	wrReg32(pBrd, GPIO(GPIO_CNTRCTRL), 0);	 // disable and reset counter
	wrReg32(pBrd, GPIO(GPIO_DMAREQ), 0x00000000);	 // Disable till SGDMA ready
	//wrReg16(pBrd, GPIO(GPIO_LED16SEG), (USHORT)0xff00);	 // show * on LEDs during init

	return(OK);
}


/**
 * Initialize Interrupt Controller IP module.
 * Disable all interrupts globally and mask all sources.
 * @param pBrd pointer to device extension to get hardware BAR address
 */
int IntrCtrl_Setup(pcie_board_t *pBrd)
{
	//--------------------------------------------------------------------------
	// Verify that the hardware we're accessing has the correct IP blocks and 
	// versions.  If the ID registers don't match what we expect then abort
	// becasue we don't want to take the chance of enabling interrupts but not
	// being able to control them!  or trying to program SGDMA or other registers
	// and not have the right memory map!
	//--------------------------------------------------------------------------
	if (rdReg32(pBrd, GPIO(INTRCTL_ID_REG)) != INTRCTL_ID_VALID)
	{
		printk(KERN_ERR "ERROR! INTRCTRL IP not found!\n");
		return (-ENODEV);
	}

	wrReg32(pBrd, GPIO(INTRCTL_ENABLE), 0);	 // mask all device interrupts
	wrReg32(pBrd, GPIO(INTRCTL_CTRL), 0);	 // disable output pin

	return(OK);
}



/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/
/*
 *     SCATTER GATHER DMA IP ROUTINES
 *
 * For the demo hardware this driver uses, SGDMA channel 0 is wired for writing to
 * the PCIe core.  Channels 1-4 are wired for sending read requests and then reading
 * from the PCIe core.  These are the only channels supported by the hardware and 
 * their directions are fixed.
 */
/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/

/**
 * Configure the global registers in the SGDMA for this specific hardware
 * design.  Chan0 = write, Chan1-4 = reads
 *
 * @param pBrd pointer to device extension to get hardware BAR address
 */
int SGDMA_Init(pcie_board_t *pBrd)
{
	ULONG x;

	// Verify ID
	x = rdReg32(pBrd, SGDMA(SGDMA_IPID_REG));
	if ((x & 0xffff0000) != 0x12040000)
	{	
		printk(KERN_ERR "lscdma: Invalid SGDMA ID: %lx\n", x);
		return(-ENODEV);
	}

	// Verify IP version cause registers are different between revs.
	x = rdReg32(pBrd, SGDMA(SGDMA_IPVER_REG));
	x = x>>16;  // version is in upper 16 bits [major:8][minor:8]
	pBrd->SGDMA.ipVer = x;

	if ((x == 0x0200) || (x == 0x0201) || (x == 0x0202))
	{
		printk(KERN_ERR "lscdma: Invalid SGDMA IP ver: %lx\n", x);
		return(-ENODEV);
	}
	printk(KERN_INFO "lscdma: SGDMA IP ver: %lx\n", x);

	// Maybe check number of BDs and Channels


	wrReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG), 0xffff0000);	 // disable and reset all channels and mask REQs
	wrReg32(pBrd, SGDMA(SGDMA_GSTATUS_REG), 0x00000000);	 // Disable SGDMA till ready



	wrReg32(pBrd, SGDMA(CHAN_CTRL(0)), 0x00000040);	  // Grp 1, BD base 0, no err mask
	wrReg32(pBrd, SGDMA(CHAN_PBOFF(0)), 0x00000000);	  //  Pkt Buf offset = 0 (1st block)

	wrReg32(pBrd, SGDMA(CHAN_CTRL(1)), 0x00000000);	  // Grp 0, BD base 0, no err mask
	wrReg32(pBrd, SGDMA(CHAN_PBOFF(1)), 0x00000000);	  //  Pkt Buf offset = 0 (1st block)



	wrReg32(pBrd, SGDMA(SGDMA_GARBITER_REG), 0xfffc148f);  // Global Arbiter Control - enable 0 & 1
	wrReg32(pBrd, SGDMA(SGDMA_GEVENT_REG), 0xfffc0000);	  // Global Event - enable ch0 & 1
	wrReg32(pBrd, SGDMA(SGDMA_GSTATUS_REG), 0xe0000000);	  // enable buses and DMA engine

	return(OK);
}

/**
 * Disable the SGDMA core.
 * @param pBrd pointer to device extension to get hardware BAR address
 */
void SGDMA_Disable(pcie_board_t *pBrd)
{


	wrReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG), 0xffff0000); // disable and reset all channels and mask REQs
	wrReg32(pBrd, SGDMA(SGDMA_GSTATUS_REG), 0x00000000);	 // Disable SGDMA and bus accesses
}



/**
 * Configure the global registers in the SGDMA for a specific channel.
 * Preserve other bit settings.  Enable channel after its setup.
 *
 * @param pBrd pointer to device extension to get hardware BAR address
 * @param chan the DMA channel to configure
 * @param startBD the buffer descriptor this channel starts at
 */
void SGDMA_EnableChan(pcie_board_t *pBrd, ULONG chan, ULONG startBD)
{
	ULONG x;

	x = rdReg32(pBrd, SGDMA(CHAN_CTRL(chan)));
	if (pBrd->SGDMA.ipVer < 0x0200)
		x = x | (startBD<<8);
	else
		x = x | (startBD<<16);   // newer versions of IP
	wrReg32(pBrd, SGDMA(CHAN_CTRL(chan)), x);	  // set the BD base (preserve other bits)
	wrReg32(pBrd, SGDMA(CHAN_PBOFF(chan)), 0x00000000);	  //  Pkt Buf not used

	x = rdReg32(pBrd, SGDMA(SGDMA_GARBITER_REG));         // Enable channel in arbiter (if used)
	x = (x & (~(0x00010000<<chan))) | 0x0000148f;  // unmask channel and set default weights
	wrReg32(pBrd, SGDMA(SGDMA_GARBITER_REG), x);  // set Global Arbiter Control

	x = rdReg32(pBrd, SGDMA(SGDMA_GEVENT_REG));    
	x = (x & (~(0x00010000<<chan)));  // Unmask events from this channel
	wrReg32(pBrd, SGDMA(SGDMA_GEVENT_REG), x);  // set Event register

	x = rdReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG));
	x = (x & (~(0x00010000<<chan))) | (1<<chan);
	wrReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG), x);	  // enable and unmask the channel

	// Make sure the SGDMA is enabled (sometimes diagnostic programs doing peeks and pokes
	// turn of the SGDMA global registers on exit, which could "hang" the driver)
	wrReg32(pBrd, SGDMA(SGDMA_GSTATUS_REG), 0xe0000000);	  // enable buses and DMA engine
}

/**
 * Clear bits in the global registers in the SGDMA to disable a specific channel.
 * Preserve other bit settings.
 *
 * @param pBrd pointer to device extension to get hardware BAR address
 * @param chan the DMA channel to configure
 */
void SGDMA_DisableChan(pcie_board_t *pBrd, ULONG chan)
{
	ULONG x;

	x = rdReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG));
	x = (x | (0x00010000<<chan) ) & ~(1<<chan);      // mask and disable
	wrReg32(pBrd, SGDMA(SGDMA_GCONTROL_REG), x);	  //  the channel

	x = rdReg32(pBrd, SGDMA(SGDMA_GEVENT_REG));    
	x = (x | (0x00010000<<chan));                  // Mask events from this channel
	wrReg32(pBrd, SGDMA(SGDMA_GEVENT_REG), x);  // set Event register

	x = rdReg32(pBrd, SGDMA(SGDMA_GARBITER_REG));  
	x = (x | (0x00010000<<chan));                    // Mask channel from being scheduled
	wrReg32(pBrd, SGDMA(SGDMA_GARBITER_REG), x);  // set Global Arbiter Control
}


/**
 * Configure the Channel to do a write to PC using a Scatter Gather list.
 * This setups up the buffer descriptors.  SGDMA_EnableChan() should be 
 * called before this to setup the channel for transfer.  Finally call
 * SGDMA_StartWriteChan() to start the transfer.
 * 
 * @note The channel must have a valid start BD programed into its config
 * register before calling this function.  This function uses that register
 * value to locate the BDs to configure.
 *
 * @param pBrd pointer to device extension to get hardware BAR address
 * @param chan the SGDMA channel number to setup for MWr to PC
 * @param nBDs number of buffer descriptors this channel can use, 256 max
 * @param pSGlist the PC mapping of virtual memory pages, destination of MWr
 * @param burstSize the max MWr TLP payload size, normally 128 bytes
 */
void SGDMA_ConfigWrite(pcie_board_t *pBrd,
			ULONG chan,
			ULONG nBDs,
			struct scatterlist *pSGlist,
			ULONG burstSize)
{
	ULONG i;
	ULONG dstAddr;
	ULONG srcAddr;
	ULONG bd;
	ULONG len;
	ULONG srcAddrMode;
	ULONG srcDataSize;
	struct scatterlist *sg;


	sg = pSGlist;

	// Get channels base buffer descriptor
	bd = rdReg32(pBrd, SGDMA(CHAN_CTRL(chan)));
	if (pBrd->SGDMA.ipVer < 0x0200)
		bd = ((bd>>8) & 0xff);  // get base BD number
	else
		bd = ((bd>>16) & 0xffff);  // get base BD number


	srcAddr = pBrd->SGDMA.Chan[chan].DevMem.baseAddr;
	srcAddrMode = pBrd->SGDMA.Chan[chan].DevMem.addrMode;
	srcDataSize = pBrd->SGDMA.Chan[chan].DevMem.dataWidth;

	for (i = 0; i < nBDs; i++)
	{
		// assumption that length <= PAGE_SIZE <= 4096
		len = sg_dma_len(sg);
		dstAddr = (u32)sg_dma_address(sg);

		if (i == (nBDs - 1))
			wrReg32(pBrd, SGDMA(BD_CFG0(bd)), (DST_MEM | DST_SIZE(DATA_64BIT) | DST_BUS(BUS_B) |  \
        					  SRC_ADDR_MODE(srcAddrMode) | SRC_SIZE(srcDataSize) | SRC_BUS(BUS_A) | EOL));
		else
			wrReg32(pBrd, SGDMA(BD_CFG0(bd)), (DST_MEM | DST_SIZE(DATA_64BIT) | DST_BUS(BUS_B) |  \
							  SRC_ADDR_MODE(srcAddrMode) | SRC_SIZE(srcDataSize) | SRC_BUS(BUS_A)));
		if (len < burstSize)
			wrReg32(pBrd, SGDMA(BD_CFG1(bd)), BURST_SIZE(len) | XFER_SIZE(len));
		else
			wrReg32(pBrd, SGDMA(BD_CFG1(bd)), BURST_SIZE(burstSize) | XFER_SIZE(len));
		
		wrReg32(pBrd, SGDMA(BD_SRC(bd)), WB(srcAddr));	 //  source = EBR_64 address
		wrReg32(pBrd, SGDMA(BD_DST(bd)), dstAddr);	//  destination = PC page address

		if (srcAddrMode != ADDR_MODE_FIFO)
			srcAddr = srcAddr + len;  // where to start from next time

		++bd;	 // next BD
		++sg;    // next entry in ScatterGather list
	}

}



/**
 * Configure the Channel to do a read from the PC using a Scatter Gather list.
 * This setups up the buffer descriptors.  SGDMA_EnableChan() should be 
 * called before this to setup the channel for transfer.  Finally call
 * SGDMA_StartReadChan() to start the transfer.
 * 
 * @note The channel must have a valid start BD programed into its config
 * register before calling this function.  This function uses that register
 * value to locate the BDs to configure.
 *
 * @param pBrd pointer to device extension to get hardware BAR address
 * @param chan the SGDMA channel number to setup for MRd to PC
 * @param nBDs number of buffer descriptors this channel can use, 256 max
 * @param pSGlist the PC mapping of virtual memory pages, destination of MWr
 * @param burstSize the max MWr TLP payload size, normally 128 bytes
 */
void SGDMA_ConfigRead(pcie_board_t *pBrd,
			ULONG chan,
			ULONG nBDs,
			struct scatterlist *pSGlist,
			ULONG burstSize)
{
	ULONG i;
	ULONG dstAddr;
	ULONG srcAddr;
	ULONG bd;
	ULONG len;
	ULONG dstAddrMode;
	ULONG dstDataSize;
	struct scatterlist *sg;

	sg = pSGlist;

	// Get channels base buffer descriptor
	bd = rdReg32(pBrd, SGDMA(CHAN_CTRL(chan)));
	if (pBrd->SGDMA.ipVer < 0x0200)
		bd = ((bd>>8) & 0xff);  // get base BD number
	else
		bd = ((bd>>16) & 0xffff);  // get base BD number


	dstAddr = pBrd->SGDMA.Chan[chan].DevMem.baseAddr;
	dstAddrMode = pBrd->SGDMA.Chan[chan].DevMem.addrMode;
	dstDataSize = pBrd->SGDMA.Chan[chan].DevMem.dataWidth;

	for (i = 0; i < nBDs; i++)
	{
		// assumption that length <= PAGE_SIZE <= 4096
		len = sg_dma_len(sg);
		srcAddr = (u32)sg_dma_address(sg);

		if (i == (nBDs - 1))
			wrReg32(pBrd, SGDMA(BD_CFG0(bd)), (DST_ADDR_MODE(dstAddrMode) | DST_SIZE(dstDataSize) | DST_BUS(BUS_A) |  \
						  SRC_MEM | SRC_SIZE(DATA_64BIT) | SRC_BUS(BUS_B) | 0xf0 | EOL));
		else
			wrReg32(pBrd, SGDMA(BD_CFG0(bd)), (DST_ADDR_MODE(dstAddrMode) | DST_SIZE(dstDataSize) | DST_BUS(BUS_A) |  \
						  SRC_MEM | SRC_SIZE(DATA_64BIT) | SRC_BUS(BUS_B) | 0xf0));

		if (len < burstSize)
			wrReg32(pBrd, SGDMA(BD_CFG1(bd)), BURST_SIZE(len) | XFER_SIZE(len));
		else
			wrReg32(pBrd, SGDMA(BD_CFG1(bd)), BURST_SIZE(burstSize) | XFER_SIZE(len));
		
		wrReg32(pBrd, SGDMA(BD_SRC(bd)), srcAddr);	//  source = PC page address
		wrReg32(pBrd, SGDMA(BD_DST(bd)), WB(dstAddr));	 //  destination = EBR_64 address

		if (dstAddrMode != ADDR_MODE_FIFO)
			dstAddr = dstAddr + len;  // where to start from next time

		++bd;	 // next BD
		++sg;    // next Scatter Gather entry
	}

}




/**
 * Enable SGDMA Write interrupts and trigger the transfer.
 * This is a critical section of code, and is held by the HdwAccess spinlock
 * so only one thread at a time can modify the Interrupt controller
 * enable register.
 */
bool SGDMA_StartWriteChan(pcie_board_t *pBrd)
{
	 ULONG reg;

	reg = rdReg32(pBrd, GPIO(INTRCTL_ENABLE));
	// Enable the interrupt bit in the GPIO Interrupt Controller
	wrReg32(pBrd,  GPIO(INTRCTL_ENABLE),  (reg | INTRCTL_INTR_WR_CHAN));

	wrReg32(pBrd, GPIO(GPIO_DMAREQ), 0x0001);	// trigger DMA chan0

	return(TRUE);
}


/**
 * Enable SGDMA Read interrupts and trigger the transfer.
 * This is a critical section of code, and is held by the hdwAccess spinlock
 * so only one thread at a time can modify the Interrupt controller
 * enable register.
 */
bool SGDMA_StartReadChan(pcie_board_t *pBrd)
{
	 ULONG reg = rdReg32(pBrd, GPIO(INTRCTL_ENABLE));

	 // Enable the interrupt bit in the GPIO Interrupt Controller
	wrReg32(pBrd, GPIO(INTRCTL_ENABLE),  (reg | INTRCTL_INTR_RD_CHAN));
	wrReg32(pBrd, GPIO(GPIO_DMAREQ), 0x0002);	// trigger DMA chan1

	return(TRUE);
}




