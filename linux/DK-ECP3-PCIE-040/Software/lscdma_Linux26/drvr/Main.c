/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file Main.c
 * 
 * Generic PCI/PCI-Express Device Driver for Lattice Eval Boards.
 *
 * NOTE: code has been targeted for RedHat WorkStation 4.0 update 4
 *  kernel 2.6.9-42.ELsmp #1 SMP Wed Jul 12 23:27:17 EDT 2006 i686 i686 i386 GNU/Linux
 *
 *
 * A Linux kernel device driver, for Lattice PCIe Eval boards on the PCIe bus,
 * that maps the
 * device's PCI address windows (its BAR0-n) into shared memory that is
 * accessible by a user-space driver that implements the real control of
 * the device.
 * 
 * The intent is to map each active BAR to a corresponding minor device
 * so that the user space driver can open that minor device and mmap it
 * to get access to the registers.
 *
 * The BAR register definitions are Demo/application specific.  The driver
 * does not make any assumptions about the number of BARs that exist or
 * their size or use.  These are policies of the demo.  The driver just
 * makes them available to user space.
 *
 * The driver places no policies on the use of the device.  It simply allows
 * direct access to the PCI memory space occupied by the device.  Any number
 * of processes can open the device.  It is up to the higher level application
 * space driver to coordinate multiple accesses. The policy is basically the
 * same as a flat memory space embedded system.
 *
 * The ioctl system call can be used to control interrupts or other global
 * settings of the device.
 *
 * BUILDING:
 * 
 * Compile as regular user (no need to be root)
 * The product is a kernel module: lscdma.ko
 *
 *
 * INSTALLING:
 *
 * Need to be root to install a module.
 * 
 * Use the shell scripts insdrvr and rmdrvr to install and remove
 * the driver. 
 * The scripts may perform udev operations to make the devices known to the /dev
 * file system.
 *
 * Manual:
 * install with system call: /sbin/insmod lscpice.ko
 * remove with system call: /sbin/rmmod lscdma.ko
 * check status of module: cat /proc/modules
 * cat /proc/devices
 *
 * The printk() messages can be seen by running the command dmesg.
 *
 * The Major device number is dynamically assigned.  This info can
 * be found in /proc/devices.
 *
 
 * Diagnostic information can be seen with: cat /proc/driver/lscdma
 *
 *
 * The minor number refers to the specific device.
 * Previous incarnations used the minor number to encode the board and BAR to
 * access.  This has been abandoned, and the minor now referes to the specific
 * device controlled by this driver (i.e. the eval board).  Once open() the
 * user has access to all BARs and board resources through the same file
 * descriptor.  The user space code knows how many BARs are active via ioctl
 * calls to return the PCI resource info.
 *
 *
 * Diagnostic information can be seen with: cat /proc/driver/lscdma
 *
 * The standard read/write system operations are not implemented because the
 * user has direct access to the device registers via a standard pointer.
 *
 * This driver implements the 2.6 kernel method of sysfs and probing to register
 * the driver, discover devices and make them available to user space programs.
 * A major player is creating a specific Class lscdma which 
 *
 * register it with the PCI subsystem to probe for the eval board(s)
 * register it as a character device (cdev) so it can get a major number and minor numbers
 * create a special sysfs Class and add each discovered device under the class
 * udev processes the /sys/class/ tree to populate 
 *
 *
 * BASED ON:
 * Original lscpcie Linux driver which did things the 2.4 kernel way
 *
 */

#include "lscdma.h"

#ifndef CONFIG_PCI
	#error No PCI Bus Support in kernel!
#endif

#define USE_PROC  /* For debugging */

// Comment out this define if your kernel has no MSI API's 
#define MSI    /* attempt to use MSI interrupt support */




MODULE_AUTHOR("Lattice Semiconductor");
MODULE_DESCRIPTION("LSC_PCIe DMA Device Driver");

/* License this so no annoying messages when loading module */
MODULE_LICENSE("Dual BSD/GPL");

MODULE_ALIAS("lscdma");




/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*
 *            DRIVER GLOBAL VARIABLES
 */
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/*-------------------------------------------------*/


/**
 * The driver's global database of all boards and run-time information.
 */
static struct LSCDMA lscdma;



int DrvrDebug = 0;


static const char Version[] = "lscdma v1.1.0 - ECP3 and SGDMA v2.3 support";  /**< version string for display */


static const char *BoardName[4] = {"??", "SC", "ECP2M", "ECP3"};
static const char *DemoName[3] = {"??", "DMA"};



/**
 * List of boards we will attempt to find and associate with the driver.
 */
static struct pci_device_id lscdma_pci_id_tbl[] = 
{
	{ 0x1204, 0x5303, 0x1204, 0x3040, },   // SC DMA
	{ 0x1204, 0xe250, 0x1204, 0x3040, },   // ECP2M DMA
	{ 0x1204, 0xec30, 0x1204, 0x3040, },   // ECP3 DMA
	{ }			/* Terminating entry */
};

MODULE_DEVICE_TABLE(pci, lscdma_pci_id_tbl);


// Wait Q's to park user's read/write requests while DMAing
DECLARE_WAIT_QUEUE_HEAD(lscdma_ReadQ);
DECLARE_WAIT_QUEUE_HEAD(lscdma_WriteQ);



/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
/*
 *            PROC DEBUG STUFF
 */
/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
#ifdef USE_PROC /* don't waste space if unused */

/**
 * Procedure to format and display data into the /proc filesystem when 
 * a user cats the /proc/driver/lscpie2 file.
 */
int lscdma_read_procmem(char *buf, char **start, off_t offset,
						 int count, int *eof, void *data)
{
	int i, n;
	int len = 0;
	// int limit = count - 80; /* Don't print more than this */
	pci_dev_bar_t *p;  

	*start = buf + offset;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: /proc entry created\n");

	/* Put any messages in here that will be displayed by cat /proc/driver/.. */
	len += sprintf(buf+len, "\nLSC PCIE Device Driver Info\n");
	len += sprintf(buf+len, "\nNumBoards: %d  Major#: %d\n", lscdma.numBoards, MAJOR(lscdma.drvrDevNum));

	for (n = 0; n < NUM_BOARDS; n++)
	{

		if (lscdma.Board[n].ID != 0)
		{

			len += sprintf(buf+len, "Board:%d = %x  Demo=%x IRQ=%d\n", lscdma.Board[n].instanceNum, 
									lscdma.Board[n].ID,
									lscdma.Board[n].demoID,
									lscdma.Board[n].IRQ);

			for (i = 0; i < NUM_BARS; i++)
			{
				p = &lscdma.Board[n].Dev_BARs[i];
				len += sprintf(buf+len, "BAR[%d]  pci_addr=%p  kvm_addr=%p\n"
							   "          type=%d  dataSize=%d  len=%ld\n"
							   "          start=%lx  end=%lx  flags=%lx\n",
							   i, 
							   p->pci_addr,
							   p->kvm_addr,
							   p->memType,
							   p->dataSize,
							   p->len,
							   p->pci_start,
							   p->pci_end,
							   p->pci_flags);
			}
		}
	}

	if (len < offset + count)
		*eof = 1;	/* Mark that this is a complete buffer (the End of File) */

	/* Not sure about all this, but it works */
	len = len - offset;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;


	return(len);
}


#endif /* USE_PROC */



/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/
/*
 *     HARDWARE BOARD DISCOVERY AND SETUP
 */
/*====================================================================================*/
/*====================================================================================*/
/*====================================================================================*/


/**
 * Initialize the board's resources.
 * This is called when probe() has found a matching PCI device (via the PCI subsystem
 * probing for boards on behalf of the driver).  The board resources are mapped in
 * and its setup to be accessed.
 */
static pcie_board_t* initBoard(struct pci_dev *PCI_Dev_Cfg, void * devID)
{
	int i;
	unsigned char irq;
	pcie_board_t *pBrd;
	pci_dev_bar_t *pBAR;
	pci_dev_bar_t *p;
	u16 SubSystem;
	u16 VendorID;
	u16 DeviceID;

	/****************************************************/
	/* Device info passed in from the PCI controller via probe() */
	/****************************************************/

// TODO
// Add writing an 'E' to the LEDs to show an error if initialization fails
// Problem is we don't have BARs setup till end of this function :-(


	if (DrvrDebug)
		printk(KERN_INFO "lscdma: init EvalBoard\n");

	if (lscdma.numBoards >= NUM_BOARDS)
	{
		printk(KERN_WARNING "lscdma: init: Too many boards! Increase NUM_BOARDS!\n");
		return(NULL);
	}


	/* Next available board structure in data base */
	pBrd = &lscdma.Board[lscdma.numBoards];

	// Initialize new board structure to all 0's
	memset(pBrd, 0, sizeof(pcie_board_t));


	if (pci_read_config_word(PCI_Dev_Cfg, PCI_VENDOR_ID, &VendorID))
	{
		printk(KERN_ERR "lscdma: init EvalBoard cfg access failed!\n");
		return(NULL);
	}
	if (VendorID != 0x1204)
	{
		printk(KERN_ERR "lscdma: init EvalBoard not Lattice ID!\n");
		return(NULL);
	}

	if (pci_read_config_word(PCI_Dev_Cfg, PCI_DEVICE_ID, &DeviceID))
	{
		printk(KERN_ERR "lscdma: init EvalBoard cfg access failed!\n");
		return(NULL);
	}

	if (pci_read_config_word(PCI_Dev_Cfg, PCI_SUBSYSTEM_ID, &SubSystem))
	{
		printk(KERN_ERR "lscdma: init EvalBoard cfg access failed!\n");
		return(NULL);
	}


	// Start initializing data for new board
	pBrd->ID = DeviceID;
	pBrd->demoID = SubSystem;
	pBrd->pPciDev = PCI_Dev_Cfg;
	pBrd->majorNum = MAJOR(lscdma.drvrDevNum);
	pBrd->minorNum = MINOR(lscdma.drvrDevNum) + lscdma.numBoards;
	atomic_set(&(pBrd->OpenToken), 1);   // initialize "open" token to available 
 	spin_lock_init(&pBrd->hdwAccess);   // initialize spin-lock for register access protection

	// Figure out if board is SC or ECP2M or ECP3, if demo is DMA or its an unknown board
	if ((DeviceID == 0x5303) && (SubSystem == 0x3040))
	{
		++lscdma.numSC_SFIF;
		pBrd->instanceNum  = lscdma.numSC_SFIF;
		pBrd->boardType = SC_BOARD;
		pBrd->demoType = DMA_DEMO;
		pBrd->ctrlBAR = 0;
	}
	else if ((DeviceID == 0xe250) && (SubSystem == 0x3040))
	{
		++lscdma.numECP2M_SFIF;
		pBrd->instanceNum  = lscdma.numECP2M_SFIF;
		pBrd->boardType = ECP2M_BOARD;
		pBrd->demoType = DMA_DEMO;
		pBrd->ctrlBAR = 0;
	}
	else if ((DeviceID == 0xec30) && (SubSystem == 0x3040))
	{
		++lscdma.numECP3_SFIF;
		pBrd->instanceNum  = lscdma.numECP3_SFIF;
		pBrd->boardType = ECP3_BOARD;
		pBrd->demoType = DMA_DEMO;
		pBrd->ctrlBAR = 0;
	}
	else
	{
		printk(KERN_ERR "lscdma: init ERROR! unknown board: %x %x\n", DeviceID, SubSystem);
		pBrd->instanceNum  = 0;
		pBrd->boardType = 0;
		pBrd->demoType = 0;
		return(NULL);
	}

	// For now, all demos use only one BAR and that BAR is for control plane and is also what will
	// be mmap'ed into user space for the driver interface to access.
	pBrd->mmapBAR = pBrd->ctrlBAR;


	//=============== Interrupt handling stuff ========================
	if (pci_read_config_byte(PCI_Dev_Cfg, PCI_INTERRUPT_LINE, &irq))
		pBrd->IRQ = -1;  // no interrupt
	else
		pBrd->IRQ = irq;

	if (DrvrDebug)
	{
		printk(KERN_INFO "lscdma: init brdID: %x  demoID: %x\n", DeviceID, SubSystem);
		printk(KERN_INFO "lscdma: init Board[] =%d\n", lscdma.numBoards);
		printk(KERN_INFO "lscdma: init IRQ=%d\n", irq);
	}


	//================ DMA Common Buffer (Consistent) Allocation ====================
	// First see if platform supports 32 bit DMA address cycles (like what won't!)
	if (pci_set_dma_mask(PCI_Dev_Cfg, DMA_32BIT_MASK))
	{
		printk(KERN_WARNING "lscdma: init DMA not supported!\n");
		pBrd->CBDMA.hasDMA = FALSE;
	}
	else
	{	
		pBrd->CBDMA.hasDMA = TRUE;
		pBrd->CBDMA.dmaBufSize = DMA_BUFFER_SIZE;
		pBrd->CBDMA.dmaCPUAddr = pci_alloc_consistent(PCI_Dev_Cfg, pBrd->CBDMA.dmaBufSize, &pBrd->CBDMA.dmaPCIBusAddr);
		if (pBrd->CBDMA.dmaCPUAddr == NULL)
		{
			printk(KERN_WARNING "lscdma: init DMA alloc failed! No DMA buffer.\n");
			pBrd->CBDMA.hasDMA = FALSE;
		}
	}


	/* Get info on all the PCI BAR registers */
	pBrd->numBars = 0;  // initialize
	for (i = 0; i < NUM_BARS; i++)
	{
		p = &(pBrd->Dev_BARs[i]);
		p->pci_start = pci_resource_start(PCI_Dev_Cfg, i);
		p->pci_end   = pci_resource_end(PCI_Dev_Cfg, i);
		p->len       = pci_resource_len(PCI_Dev_Cfg, i);
		p->pci_flags = pci_resource_flags(PCI_Dev_Cfg, i);

		if ((p->pci_start > 0) && (p->pci_end > 0))
		{
			++(pBrd->numBars);
			p->bar = i;
			p->pci_addr = (void *)p->pci_start;
			p->memType = p->pci_flags;   /* IORESOURCE Definitions: (see ioport.h)
						      * 0x0100 = IO
						      * 0x0200 = memory
						      * 0x0400 = IRQ
						      * 0x0800 = DMA
						      * 0x1000 = PREFETCHable
						      * 0x2000 = READONLY
						      * 0x4000 = cacheable
						      * 0x8000 = rangelength ???
						      */
			/*============================================================*
			*                                                             *
			* Windows DDK definitions CM_PARTIAL_RESOURCE_DESCRIPTOR.Type *
			*                                                             *
			* #define CmResourceTypeNull                0                 *
			* #define CmResourceTypePort                1                 *
			* #define CmResourceTypeInterrupt           2                 *
			* #define CmResourceTypeMemory              3                 *
			* #define CmResourceTypeDma                 4                 *
			* #define CmResourceTypeDeviceSpecific      5                 *
			* #define CmResourceTypeBusNumber           6                 *
			* #define CmResourceTypeMaximum             7                 *
			* #define CmResourceTypeNonArbitrated     128                 *
			* #define CmResourceTypeConfigData        128                 *
			* #define CmResourceTypeDevicePrivate     129                 *
			* #define CmResourceTypePcCardConfig      130                 *
			* #define CmResourceTypeMfCardConfig      131                 *
			*============================================================*/
			if (DrvrDebug)
			{
				printk(KERN_INFO "lscdma: init BAR=%d\n", i);
				printk(KERN_INFO "lscdma: init start=%lx\n", p->pci_start);
				printk(KERN_INFO "lscdma: init end=%lx\n", p->pci_end);
				printk(KERN_INFO "lscdma: init len=0x%lx\n", p->len);
				printk(KERN_INFO "lscdma: init flags=0x%lx\n", p->pci_flags);
			}
		}
	}


	// Map the BAR into kernel space so the driver can access registers.
	// The driver can not directly read/write the PCI physical bus address returned
	// by pci_resource_start().  In our current implementation the driver really
	// doesn't access the device registers, so this is not used.  It could be used
	// if the driver took a more active role in managing the devices on the board.

	// Map the default BAR into the driver's address space for access to LED registers,
	// masking off interrupts, and any other direct hardware controlled by the driver.
	// Note that the BAR may be different per demo.  Basic uses BAR1, SFIF & SGDMA use BAR0
	pBAR = &(pBrd->Dev_BARs[pBrd->ctrlBAR]);
	if (pBAR->pci_start)
	{
		pBrd->ctrlBARaddr = ioremap(pBAR->pci_start,   // PCI bus start address
					    pBAR->len);    // BAR size
		pBAR->kvm_addr = pBrd->ctrlBARaddr;  // for historic reasons

		if (pBrd->ctrlBARaddr)
		{
			wrReg32(pBrd, GPIO(GPIO_LED16SEG), 0x80f3); // display an 'E' for error (erased if all goes well)
		}
		else 
		{
			printk(KERN_ERR "lscdma: init ERROR with ioremap\n");
			return(NULL);
		}

	}
	else
	{
		printk(KERN_ERR "lscdma: init ERROR ctrlBAR %d not avail!\n", pBrd->ctrlBAR);
		return(NULL);
	}



	// Get information about the PCIe link characteristics
	ParsePCIeLinkCap(pBrd);

	// Initialize the burst sizes for the DMA channels on this board
	pBrd->SGDMA.WriteBurstSize = pBrd->PCIeMaxPayloadSize; 
	pBrd->SGDMA.ReadBurstSize = pBrd->PCIeMaxReadReqSize; 


	//--------------------------------------------------------------------------
	// Verify that the hardware we're accessing has the correct IP blocks and 
	// versions.  If the ID registers don't match what we expect then abort
	// becasue we don't want to take the chance of enabling interrupts but not
	// being able to control them!  or trying to program SGDMA or other registers
	// and not have the right memory map!
	//--------------------------------------------------------------------------

	if (GPIO_Setup(pBrd) != OK)
		 return(NULL);

	if (IntrCtrl_Setup(pBrd) != OK)
		 return(NULL);

	if (SGDMA_Init(pBrd) != OK)
		 return(NULL);

	pBrd->SGDMA.DmaOk = TRUE;

	++lscdma.numBoards;

	return(pBrd);  // pointer to board found and initialized

}



/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
/*
 *            DRIVER FILE OPERATIONS (OPEN, CLOSE, MMAP, IOCTL)
 */
/*========================================================================*/
/*========================================================================*/
/*========================================================================*/


/**
 * Open a device.
 * A device (Lattice PCIe Eval Board) can only be open by one user at a time.
 * The main reason is to simplify the control logic.  If multiple programs
 * could open the same device, there would be contention for resources (there
 * is only one SGDMA IP core with only 1 read channel and 1 write channel).
 * Multiple users would have to wait, for their turn to use the IP by placing
 * the requests into a Q, and processing them serialy.  But why even bother.
 * The usage model here is to only run 1 demo at a time anyway, so only one
 * application will ever need to open the device at a time.
 *
 * The minor number is the index into the Board[] list.
 * It specifies exactly what board and is correlated to the device node filename.
 * Only valid board devices that have been enumerated by probe() and initialized
 * are in the list, are in /sys/class/lscdma/ and should appear in /dev/lscdma/
 *
 * The interrupts are connected in this routine.  This follows the recommended
 * flow for modules, where interrupt resources aren't claimed until a user actually
 * opens and needs them, and are released when the device is closed, so they can
 * be available for some other device.
 *
 * Note that the PCI device has already been enabled in probe() and init() so it
 * doesn't need to be done again.
 */
int lscdma_open(struct inode *inode, struct file *filp)
{
	u32 brdNum;
	u32 funcNum;
	pcie_board_t *pBrd; 
	int result;
	int status;


	/* Extract the board number from the minor number */
 	brdNum = LSCDMA_MINOR_TO_BOARD(iminor(inode));
 	funcNum = LSCDMA_MINOR_TO_FUNCTION(iminor(inode));

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: open(): board#=%d  func#=%d\n", brdNum, funcNum);

	/*
	 * FYI: If you want to get the filename of the device node that was opened
	 * in user space to get here, use the following pointers:
	 *	filp->f_dentry->d_name.name
	 *	filp->f_dentry->d_name.len
	 */


	/* Validate (paranoid) */
	if (brdNum >= lscdma.numBoards)
	{
    		printk(KERN_ERR "lscdma: brd# %d No such board!\n", brdNum);
		return(-ENODEV);
	}

	// This is what the user wants to access
	pBrd = &lscdma.Board[brdNum];


	if (pBrd->ID == 0)
	{
    		printk(KERN_ERR "lscdma: brd# %d note configured correctly!\n", brdNum);
		return(-ENODEV);  // Board[] entry not configured correctly
	}


	// Take the token to Open the device.
	// This decrements the counter, that starts at 1, and returns true
	// if the count is now 0, effectively taking a semaphore.
	// Anyone else will get a false, because the count will go to -1
	// in which case we know the device is already open, so put the
	// count back to 0 and exit with busy error.
	if (!atomic_dec_and_test(&pBrd->OpenToken))
	{
    		printk(KERN_ERR "lscdma: brd# %d already open!\n", brdNum);
		atomic_inc(&pBrd->OpenToken);  // restore to prev value
		return(-EBUSY);
	}


	/* Provide direct access to the board's resources in future system calls */
	filp->private_data = pBrd;
	pBrd->function = funcNum;


	// we may want to do this to "power-up" a previously "closed" board
	if (pci_enable_device(pBrd->pPciDev) != 0)
	{
		atomic_inc(&(pBrd->OpenToken));   // restore to prev value
    		printk(KERN_ERR "lscdma: brd# %d can't enable pci device!\n", brdNum);
		return(-ENODEV);  // can't enable it so abort, the device is dead
	}
 

	// Enable this board to write MSI and DMA into PC system memory
	pci_set_master(pBrd->pPciDev);


	// First thing is to disable all interrupt sources so nothing can happen during
	// connecting the ISR
	IsrDisableInterrupts(pBrd);

	SGDMA_Init(pBrd);   // clear out any previous SGDMA setup
	
        pBrd->InterruptCounter = 0;  // reset for new round of interrupts
	pBrd->SGDMA.ReadChanOpen = false;   // no channels are open
	pBrd->SGDMA.WriteChanOpen = false;   // no channels are open

	pBrd->msi = false;  // default to no MSI, if we have support then its set to true



#ifdef MSI 

	// Connect up interrupts
	// Setup the interrupt service routine for this board
	if (pci_enable_msi(pBrd->pPciDev) == 0) 
	{
		pBrd->IRQ = pBrd->pPciDev->irq;

		if (DrvrDebug)
			printk(KERN_INFO "lscdma: Attach MSI interrupt\n");
			
		result = request_irq(pBrd->pPciDev->irq,  // the IRQ assigned to us

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
				(irq_handler_t)lscdma_ISR,       // the ISR routine to invoke
#else
				        lscdma_ISR,       // the ISR routine to invoke
#endif
					0,                // flags - none needed for MSI
					"lscdmaMSI",      // a name to show in /proc/interrupts
					pBrd);            // arg to pass to our ISR, our board that interrutps
						          //    must pass exact same value to free_irq()
		if (result)
		{
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: can't get MSI IRQ %d\n", pBrd->IRQ);
			pBrd->IRQ = -1;
		}
		else
		{
			pBrd->msi = true;
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: MSI IRQ=%d\n", pBrd->IRQ);
		}

	}
#else
	if (0)
	{
	   // Not using MSI, force to INTx below
	}
#endif
	else   // Use INTx legacy interrupts
	{
		if (DrvrDebug)
			printk(KERN_INFO "lscdma: Attach INTx interrupt\n");

		result = request_irq(pBrd->pPciDev->irq,  // the IRQ assigned to us
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
				(irq_handler_t)lscdma_ISR,       // the ISR routine to invoke
				0,   // FLAGS is deprecated
#else
					lscdma_ISR,       // the ISR routine to invoke
					SA_SHIRQ,         // flags - PCI INTx are shared interrupts
#endif
					"lscdmaINTx",     // a name to show in /proc/interrupts
					pBrd);            // arg to pass to our ISR, our board that interrutps
						          //    must pass exact same value to free_irq()
		if (result)
		{
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: can't get INTx IRQ %d\n", pBrd->IRQ);
			pBrd->IRQ = -1;
		}
		else
		{
			pBrd->IRQ = pBrd->pPciDev->irq;
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: INTx IRQ=%d\n", pBrd->IRQ);
		}
	}

	// configure the Tasklet that will handle all the real interrupt processing after the
	// ISR does the intial ackowledge and dispatches the real work 
	tasklet_init(&(pBrd->isrTasklet),      // initialize this tasklet
			lscdma_isr_tasklet,     // with this function to run in thread
			(unsigned long)pBrd);   // and call it with the handle to this board



	init_waitqueue_head(&pBrd->SGDMA.ReadWaitQ);
	init_waitqueue_head(&pBrd->SGDMA.WriteWaitQ);


	// Determine file that was opened (based on minor #)
	//  0 = just allow plain regsiter access to any BAR
	//  1 = "ColorBars" - use DMA chan0 for writes to PC only
	//  2 = "ImgMem" - use DMA chan1 for read from PC, DMA chan0 to write back to PC

	status = OK;
	switch (iminor(inode) % MINORS_PER_BOARD)
	{
		case 0:   
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: Open for Register Access\n");
			break;


		case 1:
			// ColorBars = the pixel color generating FIFO.  You can only read from
			// the device and send to PC (its a FIFO) so need the write channels.
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: Open for ColorBars\n");

			if (!pBrd->SGDMA.WriteChanOpen)
			{
				pBrd->SGDMA.WriteChanOpen = true;
				pBrd->SGDMA.ReadChanOpen = false;

				// Initialize the device memory characteristics this channel will
				// be accessing.  Used to program actual SGDMA BDs.
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.addrMode = ADDR_MODE_FIFO;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.baseAddr = IMG_FIFO(0);
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.dataWidth = DATA_64BIT;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.maxAddrRange = IMG_FIFO_FRAME_SIZE;

				// Configure the SGDMA channel(s) needed by this device
				pBrd->SGDMA.Chan[DMA_WR_CHAN].numBDs = IMG_FIFO_FRAME_SIZE / PAGE_SIZE;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].startBD = 0;

				sema_init(&pBrd->SGDMA.Chan[DMA_WR_CHAN].dmaMutex, 1);  // initialize for single access
			}
			else
			{
				status = -EBUSY;
			}
			break;

		case 2:
			// ImgMem = the Image Memory device (EBR + pixel filter).  You can read and write to
			// the device (its linear memory) so need both DMA channels.
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: Open for ImageMove\n");

			if (!pBrd->SGDMA.ReadChanOpen && !pBrd->SGDMA.WriteChanOpen )
			{
				pBrd->SGDMA.WriteChanOpen = true;
				pBrd->SGDMA.ReadChanOpen = true;

				// Initialize the device memory characteristics the channels will
				// be accessing.  Used to program actual SGDMA BDs.
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.addrMode = ADDR_MODE_MEM;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.baseAddr = EBR_64(0);
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.dataWidth = DATA_64BIT;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].DevMem.maxAddrRange = EBR64_SIZE;
				pBrd->SGDMA.Chan[DMA_RD_CHAN].DevMem.addrMode = ADDR_MODE_MEM;
				pBrd->SGDMA.Chan[DMA_RD_CHAN].DevMem.baseAddr = EBR_64(0);
				pBrd->SGDMA.Chan[DMA_RD_CHAN].DevMem.dataWidth = DATA_64BIT;
				pBrd->SGDMA.Chan[DMA_RD_CHAN].DevMem.maxAddrRange = EBR64_SIZE;


				// Configure the SGDMA channel(s) needed by this device
				pBrd->SGDMA.Chan[DMA_WR_CHAN].numBDs = EBR64_SIZE / PAGE_SIZE;
				pBrd->SGDMA.Chan[DMA_WR_CHAN].startBD = 0;
				pBrd->SGDMA.Chan[DMA_RD_CHAN].numBDs = EBR64_SIZE / PAGE_SIZE;
				pBrd->SGDMA.Chan[DMA_RD_CHAN].startBD = EBR64_SIZE / PAGE_SIZE;  // put after writes

				sema_init(&pBrd->SGDMA.Chan[DMA_RD_CHAN].dmaMutex, 1);  // initialize for single access
				sema_init(&pBrd->SGDMA.Chan[DMA_WR_CHAN].dmaMutex, 1);  // initialize for single access
			}
			else
			{
				status = -EBUSY;
			}
			break;

		default:
			status = -ENODEV;
			break;

	}

	if (status != OK)
	{
		printk(KERN_ERR "lscdma: ERROR Opening = %d\n", status);
		atomic_inc(&(pBrd->OpenToken));   // restore to prev value cause we didn't open it
	}
	else
	{
		// Write an 'O' to the LEDs to signal its opened
		wrReg32(pBrd, GPIO(GPIO_LED16SEG), 0x00ff);   // display an 'O' 

		// Enable interrupts
		if (pBrd->IRQ != -1)
			IsrEnableInterrupts(pBrd);
	}


	return(status);
}


/**
 * Close.
 * The complement to open().
 */
int lscdma_release(struct inode *inode, struct file *filp)
{
	struct PCIE_Board *pBrd = filp->private_data;

	u32 mnr = iminor(inode);

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: close() - closing board=%d  function=%x\n", 
				LSCDMA_MINOR_TO_BOARD(mnr), LSCDMA_MINOR_TO_FUNCTION(mnr));



// TODO - Can this really happen?  Can we get here without having had the device close?
//	Is this possible in Linux?
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// I want to make sure that there is no DMA still occurring because this
	// brd is going away and catastrophe will result if the DMA is writing to
	// this user's memory pages and using the brd data structures.
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (pBrd->SGDMA.Reading)
	{
		// cancel the DMA
		pBrd->SGDMA.Chan[DMA_RD_CHAN].cancelDMA = TRUE;

		SGDMA_DisableChan(pBrd, DMA_RD_CHAN);   // cancel this cause we don't want DMA writing anymore
		udelay(10);   // make sure SGDMA is done before continuing and releasing memory
	}
	
	if (pBrd->SGDMA.Writing)
	{
		// cancel the DMA
		pBrd->SGDMA.Chan[DMA_WR_CHAN].cancelDMA = TRUE;

		SGDMA_DisableChan(pBrd, DMA_WR_CHAN);   // cancel this cause we don't want DMA writing anymore
		udelay(10);   // make sure SGDMA is done before continuing and releasing memory
	}



	// Disable and release the interrupts
	IsrDisableInterrupts(pBrd);

	if (pBrd->IRQ != -1)
	{
		if (DrvrDebug)
			printk(KERN_INFO "lscdma: close() - free_irq %d\n", pBrd->IRQ);
		free_irq(pBrd->IRQ, pBrd);  // call first

#ifdef MSI 
		if (pBrd->msi)
			pci_disable_msi(pBrd->pPciDev);  // call second if MSI was used
#endif
	}

    	pBrd->IRQ = -1;
	pBrd->msi = false;

	pBrd->SGDMA.Reading = false;
	pBrd->SGDMA.Writing = false;
	pBrd->SGDMA.ReadChanOpen = false;
	pBrd->SGDMA.WriteChanOpen = false;

	// Write a 'C' to the LEDs to signal its closed
	wrReg32(pBrd, GPIO(GPIO_LED16SEG), 0x00f3);   // display a 'C' 

	pci_disable_device(pBrd->pPciDev);  // we may want to do this to "power-down" the board

	// Give back the token and let someone else open the board
	atomic_inc(&(pBrd->OpenToken));

	return(0);
}



/**
 * ioctl.
 * Allow simple access to generic PCI control type things like enabling
 * device interrupts and such.
 * IOCTL works on a board object as a whole, not a BAR.
 */
int lscdma_ioctl(struct inode *inode, 
		  struct file *filp,
		  unsigned int cmd,
		  unsigned long arg)
{
	int i;
	int status = OK;
	int mnr = iminor(inode);
	pcie_board_t *pBrd = NULL; 
	PCIResourceInfo_t *pInfo;
	DMAResourceInfo_t *pDmaInfo;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: ioctl(minor=%d cmd=%d arg=%lx size=%d)\n", 
						mnr, _IOC_NR(cmd), arg, _IOC_SIZE(cmd));

	if (_IOC_TYPE(cmd) != LSCPCIE_MAGIC)
		return(-EINVAL);
	if (_IOC_NR(cmd) > IOCTL_LSCDMA_MAX_NR)
		return(-EINVAL);

	pBrd = filp->private_data;

	switch (cmd)
	{

		case IOCTL_LSCPCIE_GET_VERSION_INFO:
			// first make sure the pointer passed in arg is still valid user page
			if (!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
			{
				status = -EFAULT;
				break;  // abort
			}

			pInfo = kmalloc(sizeof(MAX_DRIVER_VERSION_LEN ), GFP_KERNEL);
			if (pInfo == NULL)
			{
				status = -EFAULT;
				break;  // abort
			}


			strncpy((void *)arg, Version, MAX_DRIVER_VERSION_LEN - 1);
			kfree(pInfo);  // release kernel temp buffer

			break;

		case IOCTL_LSCPCIE_SET_BAR:
			// The argument passed in is the direct BAR number (0-5) to use for mmap
			pBrd->mmapBAR = arg;
			break;


		case IOCTL_LSCPCIE_GET_RESOURCES:
			// first make sure the pointer passed in arg is still valid user page
			if (!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
			{
				status = -EFAULT;
				break;  // abort
			}

			pInfo = kmalloc(sizeof(PCIResourceInfo_t), GFP_KERNEL);
			if (pInfo == NULL)
			{
				status = -EFAULT;
				break;  // abort
			}

			if (pBrd->IRQ > 0)
			    pInfo->hasInterrupt = TRUE;
			else
			    pInfo->hasInterrupt = FALSE;
			pInfo->intrVector = pBrd->IRQ;
			pInfo->numBARs = pBrd->numBars;
			for (i = 0; i < MAX_PCI_BARS; i++)
			{
			    pInfo->BAR[i].nBAR = pBrd->Dev_BARs[i].bar;
			    pInfo->BAR[i].physStartAddr = (ULONG)pBrd->Dev_BARs[i].pci_addr;
			    pInfo->BAR[i].size = pBrd->Dev_BARs[i].len;
			    pInfo->BAR[i].memMapped = (pBrd->Dev_BARs[i].kvm_addr) ? 1 : 0;
			    pInfo->BAR[i].flags = (USHORT)(pBrd->Dev_BARs[i].pci_flags);
			    pInfo->BAR[i].type = (UCHAR)((pBrd->Dev_BARs[i].memType)>>8);  // get the bits that show IO or mem
			}
			for (i = 0; i < 0x100; ++i)
			    pci_read_config_byte(pBrd->pPciDev, i, &(pInfo->PCICfgReg[i]));

			if (copy_to_user((void *)arg, (void *)pInfo, sizeof(PCIResourceInfo_t)) != 0)
				status = -EFAULT; // Not all bytes were copied so this is an error
			kfree(pInfo);  // release kernel temp buffer

			break;


		case IOCTL_LSCDMA_GET_DMA_INFO:
			// first make sure the pointer passed in arg is still valid user page
			if (!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))
			{
				status = -EFAULT;
				break;  // abort
			}

			pDmaInfo = kmalloc(sizeof(DMAResourceInfo_t), GFP_KERNEL);
			if (pDmaInfo == NULL)
			{
				status = -EFAULT;
				break;  // abort
			}

			pDmaInfo->devID = pBrd->minorNum;     // board number of specific device

			pDmaInfo->busNum = pBrd->pPciDev->bus->number;  // PCI bus number board located on
			pDmaInfo->deviceNum = PCI_SLOT(pBrd->pPciDev->devfn);     // PCI device number assigned to board
			pDmaInfo->functionNum = PCI_FUNC(pBrd->pPciDev->devfn);   // our function number
			pDmaInfo->UINumber = pBrd->minorNum;      // slot number (not implemented) 

			// Device DMA Common buffer memory info
			pDmaInfo->hasDmaBuf = pBrd->CBDMA.hasDMA;        // true if DMA buffer has been allocated by driver 
			pDmaInfo->DmaBufSize = pBrd->CBDMA.dmaBufSize;   // size in bytes of said buffer 
			pDmaInfo->DmaAddr64 = 0;      // driver only asks for 32 bit, SGDMA only supports 32 bit 
			pDmaInfo->DmaPhyAddrHi = 0;    // not used, only 32 bit
			pDmaInfo->DmaPhyAddrLo = pBrd->CBDMA.dmaPCIBusAddr;    // DMA bus address to be programmed into device 

			// Info obtained from the PCIeCap Cfg Reg of the Link
			pDmaInfo->MaxPayloadSize = pBrd->PCIeMaxPayloadSize;
			pDmaInfo->MaxReadReqSize = pBrd->PCIeMaxReadReqSize;
			pDmaInfo->RCBSize = pBrd->PCIeRCBSize;
			pDmaInfo->LinkWidth = pBrd->PCIeLinkWidth;

			if (copy_to_user((void *)arg, (void *)pDmaInfo, sizeof(DMAResourceInfo_t)) != 0)
				status = -EFAULT; // Not all bytes were copied so this is an error
			kfree(pDmaInfo);  // release kernel temp buffer

			break;


		case IOCTL_LSCDMA_READ_SYSDMABUF:
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: ioctl read_sysdma\n");

			if (!access_ok(VERIFY_WRITE, (void *)arg, pBrd->CBDMA.dmaBufSize))
			{
				status = -EFAULT;
				break;  // abort
			}

			if (!pBrd->CBDMA.hasDMA)
			{
				status = -EINVAL;   // invalid, no DMA buffer allocated
				break;
			}

			// Send the whole buffer over to the DriverIF API and let user space decide if
			// they only want a small chunk of it.
			if (copy_to_user((void *)arg, pBrd->CBDMA.dmaCPUAddr, pBrd->CBDMA.dmaBufSize) != 0)
				return(-EFAULT);
			break;


		case IOCTL_LSCDMA_WRITE_SYSDMABUF:
			if (DrvrDebug)
				printk(KERN_INFO "lscdma: ioctl write_sysdma\n");

			if (!access_ok(VERIFY_READ, (void *)arg, pBrd->CBDMA.dmaBufSize))
			{
				status = -EFAULT;
				break;  // abort
			}

			if (!pBrd->CBDMA.hasDMA)
			{
				status = -EINVAL;   // invalid, no DMA buffer allocated
				break;
			}

			// Read the whole buffer from user space DriverIF API. 
			// Maybe only small chunk of it is modified, but its user's call.
			if (copy_from_user(pBrd->CBDMA.dmaCPUAddr, (void *)arg,  pBrd->CBDMA.dmaBufSize) != 0)
				return(-EFAULT);
			break;

		case IOCTL_LSCDMA_GET_DMA_TIMERS:
			printk(KERN_ERR "lscdma: ioctl unsupported IOCTL_LSCDMA_GET_DMA_TIMERS\n");
			status = -EINVAL;
			break;

		case IOCTL_LSCDMA_SET_BURST_SIZES:
			printk(KERN_ERR "lscdma: ioctl unsupported IOCTL_LSCDMA_SET_BURST_SIZES\n");
			status = -EINVAL;
			break;

		default:
			status = -EINVAL;   // invalid IOCTL argument
	}

	return(status);
}


/**
 * mmap.
 * This is the most important driver method.  This maps the device's PCI
 * address space (based on the select mmap BAR number) into the user's
 * address space, allowing direct memory access with standard pointers.
 */
int lscdma_mmap(struct file *filp,
				 struct vm_area_struct *vma)
{
	int num;
	int sysErr;
	pcie_board_t *pBrd = filp->private_data;
	pci_dev_bar_t *pBAR;
	unsigned long phys_start;	 /* starting address to map */
	unsigned long mapSize;			/* requested size to map */
	unsigned long offset;		 /* how far into window to start map */

	// Map the BAR of the board, specified by mmapBAR (normally the default one that the
	// demo supports - normally only one valid BAR in our demos)
	pBAR = &(pBrd->Dev_BARs[pBrd->mmapBAR]);

	mapSize = vma->vm_end - vma->vm_start;
	offset = vma->vm_pgoff << PAGE_SHIFT;

	num = pBAR->bar;  // this is a check to make sure we really initialized the BAR and structures

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: mmap Board=%d  BAR=%d\n", pBrd->minorNum, num);

	if (num == -1)
		return(-ENOMEM);   /* BAR not activated, no memory */

	if (mapSize > pBAR->len)
		return(-EINVAL);  /* asked for too much memory. */

	/* Calculate the starting address, based on the offset passed by user */
	phys_start = (unsigned long)(pBAR->pci_addr) + offset;

	if (DrvrDebug)
	{
		printk(KERN_INFO "lscdma: remap_page_range(0x%lx, 0x%x, %d, ...)\n",
		   vma->vm_start, (uint32_t)phys_start, (uint32_t)mapSize);
	}

	/* Make sure the memory is treated as uncached, non-swap device memory */
	vma->vm_flags = vma->vm_flags | VM_LOCKED | VM_IO | VM_RESERVED;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10))
	/* Do the page mapping the new 2.6.10+ way */
	sysErr = remap_pfn_range(vma,
				  (unsigned long)vma->vm_start,
				  (phys_start>>PAGE_SHIFT),
				  mapSize,
				  vma->vm_page_prot);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,8))
	/* Do the page mapping the intermediate way */
	sysErr = remap_page_range(vma,
				  (unsigned long)vma->vm_start,
				  phys_start,
				  mapSize,
				  vma->vm_page_prot);
#else
	#error Unsupported kernel version!!!!
#endif


	if (sysErr < 0)
	{
		printk(KERN_ERR "lscdma: remap_page_range() failed!\n");
		return(-EAGAIN);
	}

	return(0);
}




/**
 * read.
 * Read from system CommonBuffer DMA memory into users buffer.
 * User passes length (in bytes) like reading from a file.
 */
ssize_t lscdma_read(struct file *filp,
		 	char __user *userBuf,
			size_t len,
			loff_t *offp)
{
	int ret;
	pcie_board_t *pBrd = filp->private_data;
	unsigned long irqFlag;

	ret = -1;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: read len=%d  userAddr=%p\n", (u32)len, userBuf);

	if (pBrd->function == MEM_ACCESS_NUM)
		return(-ENODEV);  // read only allowed for DMA channels


	// Try to take the Mutex Semaphore
	// Only one DMA operation can proceed at a time so this blocks the caller if there
	// is something already transferring.
	if (down_interruptible(&pBrd->SGDMA.Chan[DMA_WR_CHAN].dmaMutex))
		return(-ERESTARTSYS);


	// This only operates on the Write channel and no one else can be using it now
	// (locked by semaphore) so no need to grab spin_lock
	ret = initDMAChan(pBrd,
			&pBrd->SGDMA.Chan[DMA_WR_CHAN],  // write channel
			(unsigned long)userBuf,
			len,
			PCI_DMA_FROMDEVICE);

	if (ret != OK)
	{
		up(&pBrd->SGDMA.Chan[DMA_WR_CHAN].dmaMutex);
		return(ret);   // error occurred setting up the DMA operation into user space
	}

	// Initialize DMA transfer state variables
	pBrd->SGDMA.Reading = TRUE;
	pBrd->SGDMA.Chan[DMA_WR_CHAN].doneDMA = FALSE;
	pBrd->SGDMA.Chan[DMA_WR_CHAN].cancelDMA = FALSE;
	pBrd->SGDMA.Chan[DMA_WR_CHAN].xferStatus = WAITING;

	// Start the SGDMA
	spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
	{
		StartDmaWrite(pBrd, &pBrd->SGDMA.Chan[DMA_WR_CHAN]);
	}
	spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);

	// Go to sleep waiting for it to complete (or get a signal)

	ret = wait_event_interruptible(pBrd->SGDMA.ReadWaitQ, pBrd->SGDMA.Chan[DMA_WR_CHAN].doneDMA);
	
	if ((ret != OK) || (pBrd->SGDMA.Chan[DMA_WR_CHAN].doneDMA != TRUE))
	{
		// cancel the DMA
		pBrd->SGDMA.Chan[DMA_WR_CHAN].cancelDMA = TRUE;

		SGDMA_DisableChan(pBrd, DMA_WR_CHAN);   // cancel this cause we don't want DMA writing anymore
		udelay(10);   // wait for SGDMA done before continuing and releasing memory

		ret = -ERESTARTSYS;   // user process exited or Ctrl-C, or crash or something
	}


	// Free all resources used for the transfer
	releaseDMAChan(pBrd, &pBrd->SGDMA.Chan[DMA_WR_CHAN]);

	pBrd->SGDMA.Reading = FALSE;

	up(&pBrd->SGDMA.Chan[DMA_WR_CHAN].dmaMutex);

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: read completed.  ret=%d  status=%d  done=%d cancel=%d\n", 
							ret, 
							pBrd->SGDMA.Chan[DMA_WR_CHAN].xferStatus,
							pBrd->SGDMA.Chan[DMA_WR_CHAN].doneDMA,
							pBrd->SGDMA.Chan[DMA_WR_CHAN].cancelDMA);

	if (ret == OK)
		return(len);   // this much was transfered
	else
		return(ret);   // an error occurred
}


/**
 * write.
 * Write from users buffer into system CommonBuffer DMA memory.
 * User passes length (in bytes) like writing to a file.
 */
ssize_t lscdma_write(struct file *filp,
		 	const char __user *userBuf,
			size_t len,
			loff_t *offp)
{
	int ret;
	pcie_board_t *pBrd = filp->private_data;
	unsigned long irqFlag;

	ret = -1;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: write len=%d  userAddr=%p\n", (u32)len, userBuf);

	if (pBrd->function !=  IMAGE_MOVE_NUM)
		return(-ENODEV);  // user write only allowed to ImageMove device channel

	// Try to take the Mutex Semaphore
	// Only one DMA operation can proceed at a time so this blocks the caller if there
	// is something already transferring.
	if (down_interruptible(&pBrd->SGDMA.Chan[DMA_RD_CHAN].dmaMutex))
		return(-ERESTARTSYS);


	ret = initDMAChan(pBrd,
			&pBrd->SGDMA.Chan[DMA_RD_CHAN],  // read channel in the SGDMA
			(unsigned long)userBuf,
			len,
			PCI_DMA_TODEVICE);

	if (ret != OK)
	{
		up(&pBrd->SGDMA.Chan[DMA_RD_CHAN].dmaMutex);
		return(ret);   // error occurred setting up the DMA operation into user space
	}

	pBrd->SGDMA.Writing = TRUE;
	pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA = FALSE;
	pBrd->SGDMA.Chan[DMA_RD_CHAN].cancelDMA = FALSE;
	pBrd->SGDMA.Chan[DMA_RD_CHAN].xferStatus = WAITING;

	// Start the SGDMA
	spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
	{
		StartDmaRead(pBrd, &pBrd->SGDMA.Chan[DMA_RD_CHAN]);
	}
	spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);

	// Go to sleep waiting for it to complete (or get a signal)

	ret = wait_event_interruptible(pBrd->SGDMA.WriteWaitQ, pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA);
	
	if ((ret != OK) || (pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA != TRUE))
	{
		printk(KERN_WARNING "Errored out of wait_event ret=%d  done=%d\n", 
						ret, pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA);
		// cancel the DMA
		pBrd->SGDMA.Chan[DMA_RD_CHAN].cancelDMA = TRUE;

		SGDMA_DisableChan(pBrd, DMA_RD_CHAN);   // cancel this cause we don't want DMA writing anymore
		udelay(10);   // make sure SGDMA is done before continuing and releasing memory

		ret = -ERESTARTSYS;   // user process exited or Ctrl-C, or crash or something
	}


	// Free all resources used for the transfer
	releaseDMAChan(pBrd, &pBrd->SGDMA.Chan[DMA_RD_CHAN]);

	pBrd->SGDMA.Writing = FALSE;

	up(&pBrd->SGDMA.Chan[DMA_RD_CHAN].dmaMutex);

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: write completed.  ret=%d  status=%d  done=%d cancel=%d\n", 
							ret, 
							pBrd->SGDMA.Chan[DMA_RD_CHAN].xferStatus,
							pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA,
							pBrd->SGDMA.Chan[DMA_RD_CHAN].cancelDMA);
	if (ret == OK)
		return(len);   // this much was transfered
	else
		return(ret);   // an error occurred

}






/*==================================================================*/
/*==================================================================*/
/*==================================================================*/
/*
 *              M O D U L E   F U N C T I O N S
 */
/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

/*
 * The file operations table for the device.
 * read/write/seek, etc. are not implemented because device access
 * is memory mapped based.
 */
static struct file_operations drvr_fops =
{
	owner:   THIS_MODULE,
	open:    lscdma_open,
	release: lscdma_release,
	ioctl:   lscdma_ioctl,
	mmap:    lscdma_mmap,
	read:	 lscdma_read,
	write:	 lscdma_write,
};


/*------------------------------------------------------------------*/



/**
 * Called by the PCI subsystem when it has probed the PCI buses and has
 * found a device that matches the criteria registered in the pci table.
 * For each board found, the type and demo are determined in the initBoard
 * routine.  All resources are allocated.  A new device is added to the
 * /sys/class/lscdma/ tree with the name created by the:
 * <board><demo><instance> information.
 */
static int __devinit lscdma_probe(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{
	static char devNameStr[12] = "lscdma__";
	pcie_board_t *brd;
	int err;

	devNameStr[7] = '0' + lscdma.numBoards;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: pci probe for: %s  pdev=%p  ent=%p\n", 
					devNameStr, pdev, ent);

	/*
	 * Enable the bus-master bit values.
	 * Some PCI BIOSes fail to set the master-enable bit.
	 * Some demos support being an initiator, so need bus master ability.
	 */
	err = pci_request_regions(pdev, devNameStr);
	if (err)
		return err;

	pci_set_master(pdev);

	err = pci_enable_device(pdev);
	if (err)
		return err;

	/*
	 * Call to perform board specific initialization and figure out
	 * which BARs are active, interrupt vectors, register ISR, what board
	 * it is (SC or ECP2M or ECP3), what demo (SGMDA) and what instance
	 * number (is it the 2nd time we've seen a SC DMA?)
	 * Returns pointer to the Board structure after all info filled in.
	 */
	brd = initBoard(pdev, (void *)ent);

	if (brd == NULL)
	{
		printk(KERN_ERR "lscdma: Error initializing Eval Board\n");
		// Clean up any resources we acquired along the way
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		
		return(-1);
	}
		


	// Initialize the CharDev entry for this new found eval board device
	brd->charDev.owner = THIS_MODULE;
	kobject_set_name(&(brd->charDev.kobj), "lscdma");

	cdev_init(&(brd->charDev), &drvr_fops);

//?????
// Does cdev_add initialize reference count in the kobj?
//?????

	/* Create the minor numbers here and register the device as a character device.
	 * A number of minor devices can be associated with this particular board.
	 * The hope/idea is that we give the starting minor number and the number of them
	 * and all those devices will be associated to this one particular device.
	 */
	if (cdev_add(&(brd->charDev), MKDEV(brd->majorNum,brd->minorNum), MINORS_PER_BOARD))
	{
		printk(KERN_ERR "lscdma: Error adding char device\n");
		kobject_put(&(brd->charDev.kobj));
		return(-1);	
	}


	/* This creates a new entry in the /sys/class/lscdma/ tree that represents this
	 * new device in user space.  An entry in /dev will be created based on the name
	 * given in the last argument.  udev is responsible for mapping sysfs Classes to
	 * device nodes, and is done outside this kernel driver.
	 *
	 * The name is constructed from the board type, demo type and board instance.
	 * Examples: "sc_dma_1", "ecp2m_dma_2"
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	class_device_create(lscdma.sysClass,
				NULL,
				MKDEV(brd->majorNum,brd->minorNum),
				 &(pdev->dev),   // this is of type struct device, the PCI device?
				"%s_%s_%d", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);
	printk(KERN_INFO "lscdma: added device: %s_%s_%d\n", BoardName[brd->boardType], 

								DemoName[brd->demoType], 
								brd->instanceNum);
	class_device_create(lscdma.sysClass,
				NULL,
				MKDEV(brd->majorNum,brd->minorNum + COLOR_BARS_NUM),
				 &(pdev->dev),   // this is of type struct device, the PCI device?
				"%s_%s_%d_CB", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);
	printk(KERN_INFO "lscdma: added device: %s_%s_%d_CB\n", BoardName[brd->boardType], 

								DemoName[brd->demoType], 
								brd->instanceNum);
	class_device_create(lscdma.sysClass,
				NULL,
				MKDEV(brd->majorNum,brd->minorNum + IMAGE_MOVE_NUM),
				 &(pdev->dev),   // this is of type struct device, the PCI device?
				"%s_%s_%d_IM", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);
	printk(KERN_INFO "lscdma: added device: %s_%s_%d_IM\n", BoardName[brd->boardType], 

								DemoName[brd->demoType], 
								brd->instanceNum);

#else  // earlier 2.6 kernel using class_simple_


	class_simple_device_add(lscdma.sysClass,
				MKDEV(brd->majorNum,brd->minorNum),
				NULL,   // this is of type struct device, but who?????
				"%s_%s_%d", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);

	printk(KERN_INFO "lscdma: added device: %s_%s_%d\n", BoardName[brd->boardType], 
								DemoName[brd->demoType], 
								brd->instanceNum);

	
	class_simple_device_add(lscdma.sysClass,
				MKDEV(brd->majorNum, brd->minorNum + COLOR_BARS_NUM),
				NULL,   // this is of type struct device, but who?????
				"%s_%s_%d_CB", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);
	printk(KERN_INFO "lscdma: added device: %s_%s_%d_CB\n", BoardName[brd->boardType], 
								DemoName[brd->demoType], 
								brd->instanceNum);

	class_simple_device_add(lscdma.sysClass,
				MKDEV(brd->majorNum, brd->minorNum + IMAGE_MOVE_NUM),
				NULL,   // this is of type struct device, but who?????
				"%s_%s_%d_IM", BoardName[brd->boardType], DemoName[brd->demoType], brd->instanceNum);
	printk(KERN_INFO "lscdma: added device: %s_%s_%d_IM\n", BoardName[brd->boardType], 
								DemoName[brd->demoType], 
								brd->instanceNum);
#endif

	/* Store a pointer to the Board structure with this PCI device instance for easy access
	 * to board info later on.
	 */
	pci_set_drvdata(pdev, brd);



	// Write an 'I' to the LEDs at end of initialization
	wrReg32(brd, GPIO(GPIO_LED16SEG), 0x2233);   // display an 'I' 

	return 0;
}



/**
 * Undo all resource allocations that happened in probe() during device discovery
 * and initilization.  Majore steps are:
 * 1.) release PCI resources
 * 2.) release minor numbers
 * 3.) delete the character device associated with the Major/Minor
 * 4.) remove the entry from the sys/class/lscdma/ tree
 */ 
static void __devexit lscdma_remove(struct pci_dev *pdev)
{
	pcie_board_t *pBrd = pci_get_drvdata(pdev);

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: pci remove for device: pdev=%p board=%p\n", pdev, pBrd);


	// Write an 'R' to the LEDs when device is removed
	if (pBrd->ctrlBARaddr)
		wrReg32(pBrd, GPIO(GPIO_LED16SEG), 0x98c7);   // display an 'R' 

	// Release Common Buffer DMA Buffer
	if (pBrd->CBDMA.hasDMA)
	{
		pci_free_consistent(pdev, pBrd->CBDMA.dmaBufSize, pBrd->CBDMA.dmaCPUAddr, pBrd->CBDMA.dmaPCIBusAddr);
	}



	// Shut off interrupt sources - not implemented in Basic or SFIF
	SGDMA_Disable(pBrd);
	IsrDisableInterrupts(pBrd);

	// Free our internal access to the control BAR address space
	if (pBrd->ctrlBARaddr)
		iounmap(pBrd->ctrlBARaddr);

	// No more access after this call
	pci_release_regions(pdev);

	// Unbind the minor numbers of this device
	// using the MAJOR_NUM + board_num + Minor Range of this board
	cdev_del(&(pBrd->charDev));

	unregister_chrdev_region(MKDEV(pBrd->majorNum, pBrd->minorNum), MINORS_PER_BOARD);


	// Remove the device entry in the /sys/class/lscdma/ tree
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	class_device_destroy(lscdma.sysClass, MKDEV(pBrd->majorNum, pBrd->minorNum));
	class_device_destroy(lscdma.sysClass, MKDEV(pBrd->majorNum, pBrd->minorNum + COLOR_BARS_NUM));
	class_device_destroy(lscdma.sysClass, MKDEV(pBrd->majorNum, pBrd->minorNum + IMAGE_MOVE_NUM));

#else
	class_simple_device_remove(MKDEV(pBrd->majorNum, pBrd->minorNum));
	class_simple_device_remove(MKDEV(pBrd->majorNum, pBrd->minorNum + COLOR_BARS_NUM));
	class_simple_device_remove(MKDEV(pBrd->majorNum, pBrd->minorNum + IMAGE_MOVE_NUM));
#endif

}


/*-------------------------------------------------------------------------*/
/*             DRIVER INSTALL/REMOVE POINTS                                */
/*-------------------------------------------------------------------------*/

/*
 *	Variables that can be overriden from module command line
 */
static int	debug = 0;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "lscdma enable debugging (0-1)");


/**
 * Main structure required for registering a driver with the PCI core.
 * name must be unique across all registered PCI drivers, and shows up in
 * /sys/bus/pci/drivers/
 * id_table points to the table of Vendor,Device,SubSystem matches
 * probe is the function to call when enumerating PCI buses to match driver to device
 * remove is the function called when PCI is shutting down and devices/drivers are 
 * being removed.
 */
static struct pci_driver lscdma_driver = {
	.name = "lscdma",
	.id_table = lscdma_pci_id_tbl,
	.probe = lscdma_probe,
	.remove = __devexit_p(lscdma_remove),

/*
	.save_state - Save a device's state before its suspended
	.suspend - put device into low power state
	.resume - wake device from low power state
	.enable_wake - enable device to generate wake events from low power state
*/
};


/*-------------------------------------------------------------------------*/

/**
 * Initialize the driver.
 * called by init_module() when module dynamically loaded by insmod
 */
static int __init lscdma_init(void)
{
	int result;
	int i, n;
	int err;
	//pci_dev_bar_t *p;
	//pcie_board_t *pB;

	printk(KERN_INFO "lscdma: _init()   debug=%d\n", debug);
	printk(KERN_INFO "lscdma: Version=%s\n", Version);
	DrvrDebug = debug;

	/* Initialize the driver database to nothing found, no BARs, no devices */
	memset(&lscdma, 0, sizeof(lscdma));
	for (n = 0; n < NUM_BOARDS; n++)
		for (i = 0; i < NUM_BARS; i++)
			lscdma.Board[n].Dev_BARs[i].bar = -1;

	/*
	 * Register device driver as a character device and get a dynamic Major number
         * and reserve enough minor numbers for the maximum amount of boards * BARs
         * we'd expect to find in a system.
	 */
	result = alloc_chrdev_region(&lscdma.drvrDevNum,   // return allocated Device Num here
				      0, 	    // first minor number
				      MAX_MINORS,   
                                      "lscdma");

	if (result < 0)
	{
		printk(KERN_WARNING "lscdma: can't get major/minor numbers!\n");
		return(result);
	}


	if (DrvrDebug)
		printk(KERN_INFO "lscdma: Major=%d  num boards=%d\n", MAJOR(lscdma.drvrDevNum), lscdma.numBoards );

	
	if (DrvrDebug)
		printk(KERN_INFO "lscdma: cdev_init()\n");



	/* Create the new sysfs Class entry that will hold the tree of detected Lattice PCIe Eval
	 * board devices.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	lscdma.sysClass = class_create(THIS_MODULE, "lscdma");
#else
	lscdma.sysClass = class_simple_create(THIS_MODULE, "lscdma");
#endif
	if (IS_ERR(lscdma.sysClass))
	{
		printk(KERN_ERR "lscdma: Error creating simple class interface\n");
		return(-1);	
	}



	if (DrvrDebug)
		printk(KERN_INFO "lscdma: registering driver with PCI\n");


	/* Register our PCI components and functions with the Kernel PCI core.
	 * Returns negative number for error, and 0 if success.  It does not always
	 * return the number of devices found and bound to the driver because of hot
	 * plug - they could be bound later.
	 */
	err = pci_register_driver(&lscdma_driver);

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: pci_register_driver()=%d\n", err);

	if (err < 0)
		return(err);


#ifdef USE_PROC /* only when available */
	create_proc_read_entry("driver/lscdma", 0, 0, lscdma_read_procmem, NULL);
#endif


	return(0); /* succeed */

}


/**
 * Driver clean-up.
 * Called when module is unloaded by kernel or rmmod
 */
static void __exit lscdma_exit(void)
{
	int i;

	printk(KERN_INFO "lscdma: _exit()\n");


	pci_unregister_driver(&lscdma_driver);

	for (i = 0; i < NUM_BOARDS; i++)
	{
		if (lscdma.Board[i].ID != 0)
		{
			/* Do the cleanup for each active board */
			printk(KERN_INFO "lscdma: Cleaning up board: %d\n", i);

			// Disable and release IRQ if still active
		}
	}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	class_destroy(lscdma.sysClass);
#else
	class_simple_destroy(lscdma.sysClass);
#endif


	// Free every minor number and major number we reserved in init
	unregister_chrdev_region(lscdma.drvrDevNum, MAX_MINORS);


#ifdef USE_PROC
	remove_proc_entry("driver/lscdma", NULL);
#endif

	return;
}


/*
 * Kernel Dynamic Loadable Module Interface APIs
 */

module_init(lscdma_init);
module_exit(lscdma_exit);



