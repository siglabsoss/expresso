/*  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file
 * Definition of the interface between the kernel driver and user space.
 * All legal IOCTL calls are defined here, and all structures passed
 * in IOCTL and other system calls are defined here.  The user space
 * application and kernel code (this driver) must agree on the exact
 * definitions of structures and parameter sizes or the driver could
 * crash.
 * <p>
 * Only things defined in this file can be accessed/used by User Space
 * code.  Anything defined in any other file of this driver is strictly
 * for kernel space use only.
 */

#ifndef LATTICE_SEMI_DMA_IOCTL_H
#define LATTICE_SEMI_DMA_IOCTL_H

#include "lscpcie/Ioctl.h"



/**
 * Additional Device Driver specific information to return to user space.
 * This includes the DMA resources allocated to the device, interrupts, etc.
 * This is supported in SGDMA type drivers and applications.
 */
typedef struct
{
	// Instance and device location info 
	ULONG devID;

	ULONG  busNum;         /**< PCI bus number the device is located on */
	USHORT deviceNum;      /**< PCI 8 bit device number on the above bus */
	USHORT functionNum;    /**< PCI Device function number (should always be 0) */
	ULONG	UINumber;      /**< Windows slot identifier (not always implemented by hdw) */

	// Device DMA Common buffer memory info
	bool hasDmaBuf;        /**< True if driver has asked for and gotten common buffer DMA memory */
	ULONG DmaBufSize;      /**< how big, in bytes, the contiguous DMA buffer is (usually < 64 KB) */
	bool DmaAddr64;        /**< true if addressing is using 64 bits */
	ULONG DmaPhyAddrHi;    /**< upper 32 bits of physical DMA buffer address (program into DMA hdw) */
	ULONG DmaPhyAddrLo;    /**< lower 32 bits of physical DMA buffer address (program into DMA hdw) */

	// PCIe Link properties needed for DMA construct of TLPs
	ULONG MaxPayloadSize;  /**< PCIe max payload size for MWr into PC memory */
	ULONG MaxReadReqSize;  /**< PCIe max read size for MRd from PC memory */
	ULONG RCBSize;         /**< Root Complex Read Completion size (normally 64 bytes) */
	ULONG LinkWidth;       /**< PCIe link width: x1, x4 */


} DMAResourceInfo_t;


/**
 * Return the performance timers used to time DMA read and write channel operations.
 * These features may not be implemented in the hardware.
 */
typedef struct
{
	__u32 Write;
	__u32 Read;
}
DMATimers_t;


/**
 * Set the DMA channel burst sizes.
 */
typedef struct
{
	__u32 Write;
	__u32 Read;
}
DMABurstSizes_t;


/**
 * IOCTL Operations.
 * Use these defines when performing an ioctl operation to a device.
 */

#define LSCPCIE_MAGIC 'L'


//============================================================================
// These IOCTL codes are for SGDMA Fucntionality
// start at 6 cause 4 and 5 used in LSCPCIe2
//============================================================================

/**
 * Read into user supplied data buffer the contents of the common DMA buffer allocated
 * by the driver.
 * note that transfer size is specified as an int but its really more like 16-64KB
 */
#define IOCTL_LSCDMA_READ_SYSDMABUF     _IOR(LSCPCIE_MAGIC, 6, int)


/**
 * Write user supplied data buffer into the common DMA buffer allocated
 * by the driver.
 * note that transfer size is specified as an int but its really more like 16-64KB
 */
#define IOCTL_LSCDMA_WRITE_SYSDMABUF     _IOW(LSCPCIE_MAGIC, 7, int) 


/**
 * Read back the timers used for computing DMA thruput, ulong[0]=write, [1]=read
 * NOT CURRENTLY IMPLEMENTED in Hdw or driver.
 */
#define IOCTL_LSCDMA_GET_DMA_TIMERS      _IOR(LSCPCIE_MAGIC, 8, DMATimers_t)


/**
 * Set the burst sizes for the DMA Write and Read, ulong[0]=write, [1]=read
 * NOT CURRENTLY IMPLEMENTED in driver.
 */
#define IOCTL_LSCDMA_SET_BURST_SIZES      _IOW(LSCPCIE_MAGIC, 9, DMABurstSizes_t) 


/** 
 * IO_CTL used to get additional information from in the
 * driver back to user space applications.  This info is SGDMA
 * driver specific.  Use for common buffer DMA address in user space.
 * See DMAResourceInfo_t.
 */
#define IOCTL_LSCDMA_GET_DMA_INFO       _IOR(LSCPCIE_MAGIC, 10, DMAResourceInfo_t)



#define IOCTL_LSCDMA_MAX_NR  10   // ^^^^^^must match last entry above^^^^^^^


#endif
