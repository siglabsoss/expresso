/** @file ISR.c */

/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
/*
 *           INTERRUPT HANDLER AND TASKLET
 */
/*========================================================================*/
/*========================================================================*/
/*========================================================================*/
/**
 * The Interrupt Controller logic in the GPIO block has the following
 * functionality:
@verbatim
                                      ----------------
     IRQ[n:0]     Test2  Test1 <------| Control Reg  |
       |           |       |          ----------------
       V           V       V
   ------------------------------       ------------------
   |  Status Reg                |       | Enable Reg     |
   ------------------------------       ------------------
                   |                          |
                   V                          V
               -----------------------------------
               |      Mask and OR into 1 signal  |
               -----------------------------------
                              |
                              +-------------------------> IntrOutActive
                              |
                              V
                         -------------
                         | OutEnable |
                         -------------
                               |
                               V
                         Hdw Interrupt
   
   
   INTRCTL_ID_REG     0x100  = 0x12043050
   INTRCTL_CTRL       0x104  
   INTRCTL_STATUS     0x108
   INTRCTL_ENABLE     0x10c
@endverbatim
 *
 * The OutputEnable bit in the Control register globally controls the
 * generation of hardware interrupts.  This bit is used to turn on/off
 * interrupts from the board.  The local status of interrupt sources
 * can still be read in the Status Reg or the IntrOutctive bit in the
 * control.  This is the same state that drives the hardware interrupt.
 */


#include "lscdma.h"


/**
 * 	IsrDisableInterrupts:	Disable device interrupts
 */
bool IsrDisableInterrupts(pcie_board_t *pBrd)
{
	ULONG intCtrl;

	intCtrl = rdReg32(pBrd, GPIO(INTRCTL_CTRL));
	intCtrl = intCtrl & ~INTRCTL_OUTPUT_EN;		// turn off the interrupt output  
	wrReg32(pBrd, GPIO(INTRCTL_CTRL), intCtrl);

	pBrd->InterruptsEnabled = false;
	return(true);
}


/**
 * Enable device interrupts by setting the Global output enable
 * bit in the interrupt controller.
 */
bool IsrEnableInterrupts(pcie_board_t *pBrd)
{
	ULONG intCtrl;
	intCtrl = rdReg32(pBrd, GPIO(INTRCTL_CTRL));
	intCtrl = intCtrl | INTRCTL_OUTPUT_EN;	// turn on the interrupt output  
	wrReg32(pBrd, GPIO(INTRCTL_CTRL), intCtrl);

	pBrd->InterruptsEnabled = true;
	return(true);
}


/**
 * The ISR that is invoked by the kernel in interrupt context when the interrupt line
 * is active (as attached in to the IRQ in open()).  The only job is to verify that our
 * board is generating the interrupt and then signal the bottom-half to run in regular
 * kernel context and process the interrupt.
 */
irqreturn_t	lscdma_ISR(int irq, void *dev_id, struct pt_regs *regs)
{
	ULONG intCtrl;
	ULONG led;

	pcie_board_t *pBrd = dev_id; 
	
	// check if we interrupted
	intCtrl = rdReg32(pBrd, GPIO(INTRCTL_CTRL));

	if ((intCtrl & INTRCTL_OUT_ACTIVE) == 0)
		return(IRQ_NONE);	 // not our interrupt


	// Disable the interrupt in the standard GPIO interrupt controller IP block.
	// This prevents an infinite loop.  The Deferred Tasklet needs to figure out which
	// specific IP device on the board/FPGA is generating the interrupt(s) and 
	// service and clear all of them before re-enabling the main interrupt controller.

	
	spin_lock(&pBrd->hdwAccess);   // prevent SMP access to these registers
	{
		IsrDisableInterrupts(pBrd);

		// Increment the count of interrupts received
		++pBrd->InterruptCounter;

		// For testing - show ISR count on outer seg LEDs of 16 seg
		led = rdReg32(pBrd, GPIO(GPIO_LED16SEG));
		led = (led & 0xffffff00) | (pBrd->InterruptCounter & 0xff);
		wrReg32(pBrd, GPIO(GPIO_LED16SEG), led);
	}
	spin_unlock(&pBrd->hdwAccess);

    // schedule the board's tasklet to run and do real interrupt processing
    tasklet_schedule(&(pBrd->isrTasklet));

    return(IRQ_HANDLED);  // serviced the interrupt
}


/**
 * This is scheduled to run by the ISR and is run in the kernel context.
 * It then triggers any process(es) waiting on the ISR to have happened.
 */
void lscdma_isr_tasklet(unsigned long arg)
{
	ULONG led;
	pcie_board_t *pBrd = (pcie_board_t *)arg;
	ULONG loopCnt = 1000;
	ULONG xferSize;
	uint32_t dmaStatus;
	uint32_t reg;
	uint32_t intrStatus, intrCtrl, intrEnable;
	uint32_t service;
	unsigned long irqFlag;

	
	spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
	{
		// For testing - show count on inner ring of 16segLEDs
		led = rdReg32(pBrd, GPIO(GPIO_LED16SEG));
		led = (led & 0xffff00ff) | ((pBrd->InterruptCounter & 0xff)<<8);
		wrReg32(pBrd, GPIO(GPIO_LED16SEG), led);

		// These should all be read together so values are consistent
		intrCtrl =  rdReg32(pBrd, GPIO(INTRCTL_CTRL));
		intrStatus =  rdReg32(pBrd, GPIO(INTRCTL_STATUS));
		intrEnable =  rdReg32(pBrd, GPIO(INTRCTL_ENABLE));
	}
	spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);

	if (DrvrDebug)
		printk("DPC_ISR: Cnt=%ld  CTRL=%x  STATUS=%x\n", pBrd->InterruptCounter, intrCtrl, intrStatus);

	// While there are active interrupts and we aren't stuck
	// keep servicing the hdw devices until all interrupts are cleared
	while (loopCnt && (intrCtrl & INTRCTL_OUT_ACTIVE))
	{
		--loopCnt;

		// See who needs to be serviced (only if enabled)
		service = intrStatus & intrEnable;

		// If the test mode is active and the Test bits are creating the interrupt,
		// turn them off, but leave enabled so software can fire them again
		// Used purely for testing hdw/ISR
		if (intrCtrl & INTRCTL_TEST_MODE)
		{
			if ((intrEnable & INTRCTL_TEST1_EN) && (intrCtrl & INTRCTL_INTR_TEST1))
				intrCtrl = intrCtrl & ~INTRCTL_INTR_TEST1;	// turn off test1 interrupt
			if ((intrEnable & INTRCTL_TEST2_EN) && (intrCtrl & INTRCTL_INTR_TEST2))
				intrCtrl = intrCtrl & ~INTRCTL_INTR_TEST2;	// turn off test2 interrupt

			spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
			{
				wrReg32(pBrd, GPIO(INTRCTL_CTRL), intrCtrl);
			}
			spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);
		}
		else if (service & INTRCTL_INTR_DOWN_COUNT_ZERO)
		{
			// This is another test-case interrupt.
			// When the down counter hits 0 it creates an interrupt.  Service it here
			// by disabling the counter and releasing the task thats pending.
			// Check if the task has been canceled before doing any data transfers.

			if (DrvrDebug)
				printk("DownCount Interrupt\n");

			intrEnable =  intrEnable & ~INTRCTL_DOWN_COUNT_EN;	// turn off Down Counter interrupt
			spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
			{
				wrReg32(pBrd, GPIO(INTRCTL_ENABLE), intrEnable);

				wrReg32(pBrd, GPIO(GPIO_CNTRCTRL), 0);  // turn off the counter
			}
			spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);
//TODO
			// Release task blocked on an IOCTL call???
			// I don't even remember what the plan for this was - not implemented
		}
		else if (service & INTRCTL_INTR_WR_CHAN)
		{
			if (DrvrDebug)
				printk("SGDMA Write Chan Interrupt\n");

			if (pBrd->SGDMA.Chan[DMA_WR_CHAN].cancelDMA)
			{
				// abort all this cause read() has already cleaned up and marked the error
				pBrd->SGDMA.Chan[0].xferStatus =  ERROR;
				continue;
			}

			//----------------------------------------------------------------------------
			// This is the real processing of the SGDMA interrupts.
			// If a channel has interrupted that its done a transfer, then we need to
			// look and see if more data needs to be sent.  If so then setup a new SGlist
			// and reprogram the SGDMA channel and BD's with this data and then kick-off again.
			// If the entire transfer is complete then we need to unblock the user's task and
			// return status.
			// 
			//----------------------------------------------------------------------------

			// Read the SGDMA and make sure that the xfer was OK and that no errors have occurred for
			// this Channel.  If so then abort the entire operation, even if not all the data has been
			// moved since corruption has now occurred
			spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
			{
				dmaStatus = rdReg32(pBrd, SGDMA(CHAN_STAT(0)));
				wrReg32(pBrd, SGDMA(CHAN_STAT(0)), 0x010);  // clear Xfer Complete

				// Mask the interrupt bit in the GPIO Interrupt Controller
				// It will get enable when the WRITE/READ initiates a new xfer
				reg = rdReg32(pBrd, GPIO(INTRCTL_ENABLE));
				wrReg32(pBrd,  GPIO(INTRCTL_ENABLE),  (reg & ~INTRCTL_INTR_WR_CHAN));
			}
			spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);

			if (!(dmaStatus & CHAN_STATUS_ENABLED) || 
				!(dmaStatus & CHAN_STATUS_XFERCOMP) || 
				(dmaStatus & CHAN_STATUS_ERRORS))
			{
				// ERROR!!!!!
				printk(KERN_ERR "lscdma: SGDMA WR CHAN ERROR: %x\n", dmaStatus);

				// Abort the entire DMA transfer

				pBrd->SGDMA.Chan[0].xferStatus =  ERROR;

			}
			else
			{
				// Transfer completed OK
				pBrd->SGDMA.Chan[0].xferedSoFar = pBrd->SGDMA.Chan[0].xferedSoFar + 
									pBrd->SGDMA.Chan[0].thisXferSize;

				// record time to do this part of the write transfer
				pBrd->SGDMA.Chan[0].elapsedTime = pBrd->SGDMA.Chan[0].elapsedTime + 
									rdReg32(pBrd, GPIO_WR_CNTR);

				if (pBrd->SGDMA.Chan[0].xferedSoFar == pBrd->SGDMA.Chan[0].totalXferSize)
				{
					if (DrvrDebug)
						printk(KERN_INFO "lscdma: WrChan: Done\n");
					// We're Done!!!!!
					pBrd->SGDMA.Chan[0].xferStatus =  OK;
				}
				else if (pBrd->SGDMA.Chan[0].xferedSoFar > pBrd->SGDMA.Chan[0].totalXferSize)
				{
					printk(KERN_ERR "WrChan: ERROR! xfered too much!\n");
					// Abort the DMA transfer with an ERROR

					pBrd->SGDMA.Chan[0].xferStatus =  ERROR;
				}
				else
				{
					// More to do
					xferSize = pBrd->SGDMA.Chan[0].totalXferSize - pBrd->SGDMA.Chan[0].xferedSoFar;

					if (DrvrDebug)
						printk(KERN_INFO "WrChan: %ld more to do\n", xferSize);


//TODO
					// !!!!!!!!!!!
					// We don't handle this case right now
				}
			}



			// if all done the transfer (all data moved, no more SG Lists to handle, etc.)
			pBrd->SGDMA.Chan[DMA_WR_CHAN].doneDMA = TRUE;
			wake_up_interruptible(&(pBrd->SGDMA.ReadWaitQ));  // wake-up user waiting on read()

		}
		else if (service & INTRCTL_INTR_RD_CHAN)
		{

			if (DrvrDebug)
				printk("SGDMA Read Chan Interrupt\n");

			//----------------------------------------------------------------------------
			// This is the real processing of the SGDMA interrupts.
			// If a Read channel has interrupted that its done a transfer, then we need to
			// look and see if more data needs to be reqd.  If so then setup a new SGlist
			// and reprogram the SGDMA channel and BD's with this data and then kick-off again.
			// If the entire transfer is complete then we need to unblock the user's task and
			// return status.
			// 
			//----------------------------------------------------------------------------

			if (pBrd->SGDMA.Chan[DMA_RD_CHAN].cancelDMA)
			{
				// abort all this cause read() has already cleaned up and marked the error
				pBrd->SGDMA.Chan[1].xferStatus =  ERROR;
				continue;
			}


			// Read the SGDMA and make sure that the xfer was OK and that no errors have occurred for
			// this Channel.  If so then abort the entire operation, even if not all the data has been
			// moved since corruption has now occurred
			spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
			{
				dmaStatus = rdReg32(pBrd, SGDMA(CHAN_STAT(1)));
				wrReg32(pBrd, SGDMA(CHAN_STAT(1)), 0x010);  // clear Xfer Complete

				// Mask the interrupt bit in the GPIO Interrupt Controller
				// It will get enable when the WRITE/READ initiates a new xfer
				reg = rdReg32(pBrd, GPIO(INTRCTL_ENABLE));
				wrReg32(pBrd,  GPIO(INTRCTL_ENABLE),  (reg & ~INTRCTL_INTR_RD_CHAN));
			}
			spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);

			if (!(dmaStatus & CHAN_STATUS_ENABLED) || 
				!(dmaStatus & CHAN_STATUS_XFERCOMP) || 
				(dmaStatus & CHAN_STATUS_ERRORS))
			{
				// ERROR!!!!!
				printk(KERN_ERR "lscdma: SGDMA RD CHAN ERROR: %x\n", dmaStatus);

				// Abort the entire DMA transfer
				pBrd->SGDMA.Chan[1].xferStatus =  ERROR;

			}
			else
			{
				// Transfer completed OK
				pBrd->SGDMA.Chan[1].xferedSoFar = pBrd->SGDMA.Chan[1].xferedSoFar + 
									pBrd->SGDMA.Chan[1].thisXferSize;

				// record time to do this part of the write transfer
				pBrd->SGDMA.Chan[1].elapsedTime = pBrd->SGDMA.Chan[1].elapsedTime + 
									rdReg32(pBrd, GPIO_RD_CNTR);

				if (pBrd->SGDMA.Chan[1].xferedSoFar == pBrd->SGDMA.Chan[1].totalXferSize)
				{
					if (DrvrDebug)
						printk(KERN_INFO "lscdma: RdChan: Done\n");
					// We're Done!!!!!
					pBrd->SGDMA.Chan[1].xferStatus =  OK;
				}
				else if (pBrd->SGDMA.Chan[1].xferedSoFar > pBrd->SGDMA.Chan[1].totalXferSize)
				{
					printk(KERN_ERR "lscdma: RdChan: ERROR! xfered too much!\n");
					// Abort the DMA transfer with an ERROR
					pBrd->SGDMA.Chan[1].xferStatus =  ERROR;
				}
				else
				{
					// More to do
					xferSize = pBrd->SGDMA.Chan[1].totalXferSize - pBrd->SGDMA.Chan[1].xferedSoFar;

					if (DrvrDebug)
						printk(KERN_INFO "RdChan: %ld more to do\n", xferSize);


//TODO
					// !!!!!!!!!!!
					// We don't handle this case right now

				}
			}



			// if all done the transfer (all data moved, no more SG Lists to handle, etc.)
			pBrd->SGDMA.Chan[DMA_RD_CHAN].doneDMA = TRUE;
			wake_up_interruptible(&(pBrd->SGDMA.WriteWaitQ));  // wake up user waiting on write()

		}

		spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
		{
			// Re-read control & status regs to see if Hdw still has active interrupts
			intrCtrl =  rdReg32(pBrd, GPIO(INTRCTL_CTRL));
			intrStatus =  rdReg32(pBrd, GPIO(INTRCTL_STATUS));
			intrEnable =  rdReg32(pBrd, GPIO(INTRCTL_ENABLE));
		}
		spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);
	}

	// Turn interrupt output back on (was shutoff in ISR) unless we're stuck
	// in interrupts,  then leave off and raise some flag that interrupts are stuck
	if (loopCnt)
	{
//TODO
		// Does this need to be run with some type of protection so ISR can't be invoked
		// immediately???
		spin_lock_irqsave(&pBrd->hdwAccess, irqFlag);
		{
			IsrEnableInterrupts(pBrd);
		}
		spin_unlock_irqrestore(&pBrd->hdwAccess, irqFlag);
		
		// A Hdw interrupt can fire now and invoke the ISR again
	}
	else
	{
		printk(KERN_WARNING "INTERRUPTS ARE STUCK ON!!!!  DISABLING HDW INTRS\n");
		// leave interrupt controller output shut-off

	}


//TODO
	// Do this if some user task is blocked waiting on the interrupt
	// wake_up_interruptible(&lscdma_ISRQ);

}


