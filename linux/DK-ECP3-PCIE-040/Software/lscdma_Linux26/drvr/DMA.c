/** @file DMA.c */

/*========================================================================*/
/*========================================================================*/
/*
 *           LINUX MEMORY PAGES AND DMA SETUP ROUTINES
 */
/*========================================================================*/
/*========================================================================*/
/*========================================================================*/

#include "lscdma.h"

/**
 * Enable the write channel and start the DMA.
 *
 * NOTE: this function should be called after obtaining the hdwAccess spin_lock
 * using spin_lock_irqsave() to ensure the ISR is not running and about to modify
 * the interrupt controller or SGDMA registers.
 * 
 * @param pBrd specific hardware board to access and setup
 * @param pChan DMA channel 
 */
void StartDmaWrite(pcie_board_t *pBrd,
		   DMAChannel_t *pChan)
{

	if (pChan->sgList == NULL)
	{
		printk(KERN_WARNING "lscdma: StartDmaWrite NULL sgList!  Can't proceed.\n");
		return;
	}
	

	SGDMA_EnableChan(pBrd, DMA_WR_CHAN, pChan->startBD);

	SGDMA_ConfigWrite(pBrd, 
			  DMA_WR_CHAN, 
			  pChan->sgLen,
			  pChan->sgList, 
			  pBrd->SGDMA.WriteBurstSize);



	// The ISR is locked-out by the spin lock to ensure we can
	// enable and modify interrupt controller registers and start
	// the SGDMA IP.

	
	SGDMA_StartWriteChan(pBrd); 

}


/**
 * Enable the read channel and start the DMA.
 * 
 * NOTE: this function should be called after obtaining the hdwAccess spin_lock
 * using spin_lock_irqsave() to ensure the ISR is not running and about to modify
 * the interrupt controller or SGDMA registers.
 * 
 * @param pBrd specific hardware board to access and setup
 * @param pChan DMA channel 
 */
void StartDmaRead(pcie_board_t *pBrd,
		   DMAChannel_t *pChan)
{

	if (pChan->sgList == NULL)
	{
		printk(KERN_WARNING "lscdma: StartDmaRead NULL sgList!  Can't proceed.\n");
		return;
	}
	

	SGDMA_EnableChan(pBrd, DMA_RD_CHAN, pChan->startBD);


	SGDMA_ConfigRead(pBrd, 
			  DMA_RD_CHAN, 
			  pChan->sgLen,
			  pChan->sgList, 
			  pBrd->SGDMA.ReadBurstSize);

	// The ISR is locked-out by the spin lock to ensure we can
	// enable and modify interrupt controller registers and start
	// the SGDMA IP.

	SGDMA_StartReadChan(pBrd); 

}



/**
 * Initialize the data elements that control the SGDMA operation.
 * 
 * use info from User's vritual buffer - length, 
 *
 * this assumes that the transfer can be done with one mapping.
 * It does not try to handle splitting the entire xfer among multiple
 * allocations of SG lists and maps.
 * 
 * Code takes advantage of the fact that the biggest move is 1MB and
 * that can fit within the 256 BD's.
 * If you need to move more than 1 MB (i.e. need more than 256 BD's) 
 * then this code needs to be changed to handle it.

 * @param pChan specific DMA channel of a board/device that is being setup
 * for the transfer, read or write channel.
 * @parma direction specifies reading or writing, use PCI_DMA_TODEVICE or
 * PCI_DMA_FRPOMDEVICE
 */
int initDMAChan(pcie_board_t *pBrd,
		DMAChannel_t *pChan,
		unsigned long userAddr,
		size_t len,
		int direction)
{
	int i, ret, dir;

	unsigned long firstPg, lastPg;
	unsigned long firstPgOffset;

	if (DrvrDebug)
		printk(KERN_INFO "lscdma: initDMA  userAddr=%lx len=%d\n", userAddr, (u32)len);


	if (!pBrd->SGDMA.DmaOk)
		return(-EINVAL);   // invalid, SGDMA not supported or broken, See initboard

	if (pBrd->IRQ == -1)
		return(-EINVAL);   // no ISR setup so don't go any further or we'll get stuck!


	if (direction == PCI_DMA_FROMDEVICE)
		dir = 1;  // a user read
	else if (direction == PCI_DMA_TODEVICE)
		dir = 0;  // a user write
	else
		return(-EINVAL);   // invalid direction parameter
	

	//-------------------------------------------------------------------------
	// STEP 1
	// Get information about the user's buffer in virtual memory space
	//-------------------------------------------------------------------------
	if (DrvrDebug)
		printk(KERN_INFO "lscdma: Create SGlist from user buffer\n");

	firstPg = (userAddr & PAGE_MASK) >> PAGE_SHIFT;
	lastPg = ((userAddr + len - 1) & PAGE_MASK) >> PAGE_SHIFT;
	firstPgOffset = userAddr & ~PAGE_MASK;
	pChan->numPages = lastPg - firstPg + 1;

	// Initialize channel control parameters
	pChan->totalXferSize = len;   // Want to move this much
	pChan->thisXferSize = len;    // we do all of it at once
	pChan->xferedSoFar = 0;
	pChan->elapsedTime = 0;
	pChan->bufBaseVirtualAddr = (void *)userAddr;
	pChan->isDmaAddr64 = FALSE;   // don't support 64 bit right now
	pChan->direction = direction;  // PCI_DMA_TODEVICE or PCI_DMA_FROMDEVICE

	// signal waiting for DMA to complete, will be set to true by ISR
	pChan->doneDMA = FALSE;
	pChan->cancelDMA = FALSE;

	if (firstPgOffset != 0)
		printk(KERN_WARNING "lscdma: WARNING! buffer not page aligned! SGDMA may not cope well"
				"with partial page transfers!\n");	
	
	if (DrvrDebug)
		printk(KERN_INFO "FirstPg=%lx offset=%lx LastPg=%lx nPgs=%d\n", firstPg,
									firstPgOffset,
								     lastPg,
									pChan->numPages);

    	//-------------------------------------------------------------------------
	// STEP 2: Get mapping of user's pages that represent their buffer
	// This will be used to generate the scatter list.
	//-------------------------------------------------------------------------

	// Allocate memory to hold the page mapping of the User's pages
	// Need an array of NumPages, and each entry will point to a page in the process's MM
	pChan->pageList = kmalloc(pChan->numPages * sizeof(struct page *), GFP_KERNEL);
	if (pChan->pageList == NULL) 
	{
		printk(KERN_ERR "lscdma: Can't allocate page list!\n");
		return(-ENOMEM);
	}
		
	// Get the list of the pages that make up the user's complete buffer in virtual
	// address space world.
	down_read(&current->mm->mmap_sem);
	ret = get_user_pages(current,  // this process is the one to map
				current->mm,
				userAddr,   // starting at this address
				pChan->numPages,   // for this many pages
				dir, // 0 = user write, 1 = user read
				1, // force access to user space
				pChan->pageList,  // fill this list with reference to user's pages
				NULL);	   // don't care about VMA information
	up_read(&current->mm->mmap_sem);
	
	
	if (ret != pChan->numPages)
	{
		printk(KERN_ERR "lscdma: Did not get mapping to all user pages!  Aborting!\n");
		ret = -ENOMEM;
		goto FREE_MEM;

	}
 
// TODO remove this debugging stuff later
	if (DrvrDebug)
	{
		printk(KERN_INFO "lscdma: get_user_pages = %d\n", ret);
		for (i = 0; i < pChan->numPages; i++)
		{
			printk(KERN_INFO "\tpage[%d]: index=%lx\n", i, pChan->pageList[i]->index);
		}
	}	


	//-------------------------------------------------------------------------
	// STEP 3: Create the PCI Scatter/Gather List
	//-------------------------------------------------------------------------
	
	// Allocate storage for the scatter list
	pChan->sgList = kmalloc(sizeof(struct scatterlist) * pChan->numPages, GFP_KERNEL);
	if (pChan->sgList == NULL)
	{
		printk(KERN_ERR "lscdma: can't get memory for Scatter/Gather list!\n");
		ret = -ENOMEM;
		goto FREE_PAGES;
	}

	memset(pChan->sgList, 0, pChan->numPages * sizeof(struct scatterlist));


	// fill in the information from the pages data
	pChan->sgList[0].page = pChan->pageList[0];
	pChan->sgList[0].offset = firstPgOffset;
	pChan->sgList[0].length = PAGE_SIZE - firstPgOffset;
	for (i = 1; i < pChan->numPages; i++)
	{
		if (pChan->pageList[i] == NULL)
			goto FREE_SGMEM;
		pChan->sgList[i].page = pChan->pageList[i];
		pChan->sgList[i].offset = 0;
		pChan->sgList[i].length = PAGE_SIZE;
	}

	// Call API to create the Scatter/Gather List
	pChan->sgLen = pci_map_sg(pBrd->pPciDev,    // the PCI device (for bus addr info)
				pChan->sgList,      // the place to store the list
				pChan->numPages,    // how many initial pages are in it
				pChan->direction);  // data flow direction

	if (pChan->sgLen == 0)
	{
		printk(KERN_ERR "pci_dma_sg failed!\n");
		ret = -ENOMEM;
		goto FREE_SGMEM;
	}
	


// TODO remove this debugging stuff later
	if (DrvrDebug)
	{
		struct scatterlist *sg = pChan->sgList;
		printk(KERN_INFO "sgLen = %d\n", pChan->sgLen);
		for (i = 0; i < pChan->sgLen; i++, sg++)
		{
			printk(KERN_INFO "sg[%d]: busAddr=%x  len=%d\n", i, 
							(u32)sg_dma_address(sg),
							sg_dma_len(sg));
		}
	}

	return(0);   // Normal flow. Everthing went OK



	//-------------------------------------------------------------------------
	// The following clean-up after an error condition
	//-------------------------------------------------------------------------

FREE_SGMEM:
	if (pChan->sgList != NULL)
		kfree(pChan->sgList);
	

	//-------------------------------------------------------------------------
	// Free the locked-down pages
	//-------------------------------------------------------------------------
FREE_PAGES:
	if (DrvrDebug)
		printk(KERN_INFO "Free pages\n");
	// Mark pages as dirty, since they've been written to by DMA
	for (i = 0; i < pChan->numPages; i++)
	{
		if (!PageReserved(pChan->pageList[i]))
			SetPageDirty(pChan->pageList[i]);
	}


	// Now free up pages from being locked down in memory
	for (i = 0; i < pChan->numPages; i++)
	{
		page_cache_release(pChan->pageList[i]);
	}

FREE_MEM:
	//-------------------------------------------------------------------------
	// Free the array used to hold the list of pages
	//-------------------------------------------------------------------------

	if (pChan->pageList != NULL)
		kfree(pChan->pageList);

	return(ret);
}





/**
 * Release the data elements used in the SGDMA operation.
 * 
 * @param pBrd pointer to the specific device
 * @param pChan specific DMA channel of a board/device that is being setup
 * for the transfer, read or write channel.
 */
int releaseDMAChan(pcie_board_t *pBrd,
		DMAChannel_t *pChan)
{
	int i;
	unsigned long userAddr;
	userAddr = (unsigned long)pChan->bufBaseVirtualAddr;


	if (DrvrDebug)
		printk(KERN_INFO "lscdma: releaseDMAChan  userAddr=%lx\n", userAddr);

	//-------------------------------------------------------------------------
	// Free the Scatter/Gather list and invalidate caches affected by
	// the transfer
	//-------------------------------------------------------------------------

	if (DrvrDebug)
		printk(KERN_INFO "unmap and free SG List\n");

	pci_unmap_sg(pBrd->pPciDev, 
			pChan->sgList,
			pChan->sgLen,
			pChan->direction);


	if (pChan->sgList != NULL)
		kfree(pChan->sgList);
	

	//-------------------------------------------------------------------------
	// Free the locked-down pages
	//-------------------------------------------------------------------------
	if (DrvrDebug)
		printk(KERN_INFO "Free pages\n");

	if (pChan->direction == PCI_DMA_FROMDEVICE)
	{
		// Mark pages as dirty, since they've been written to by DMA
		for (i = 0; i < pChan->numPages; i++)
		{
			if (!PageReserved(pChan->pageList[i]))
				SetPageDirty(pChan->pageList[i]);
		}
	}


	// Now free up pages from being locked down in memory
	for (i = 0; i < pChan->numPages; i++)
	{
		page_cache_release(pChan->pageList[i]);
	}


	//-------------------------------------------------------------------------
	// Free the array used to hold the list of pages
	//-------------------------------------------------------------------------

	if (pChan->pageList != NULL)
		kfree(pChan->pageList);

	
	return(OK);
}




