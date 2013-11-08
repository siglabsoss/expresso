/*  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Ioctl.h
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

 
#include <asm/ioctl.h>

#include "sysDefs.h"

#ifndef MAX_PCI_BARS
#define MAX_PCI_BARS 7    // do not count/use the expansion ROM
#endif

#define MAX_DRIVER_NAME_LEN 128
#define MAX_DRIVER_VERSION_LEN 128


typedef char DriverVerStr_t[MAX_DRIVER_NAME_LEN];


/**
 * Info structure for the characteristics of a device BAR mapped into the driver's
 * memory space.  Info is used by the driver to know how to access device memory
 * locations within this BAR.
 */
typedef struct
{
	ULONG nBAR;          /**< which BAR this info belongs to (0-5) */
	ULONG physStartAddr; /**< PCI Bus address BAR starts at */
	ULONG size;          /**< how big, in bytes, the windows into the device memory is */
	bool memMapped;      /**< true if driver has mapped this bus address into user space for simple pointer access */
	USHORT flags;        /**< memory type flags (Windows specifics) */
	UCHAR type;          /**< memory or IO (see Windows) */

} PCI_BAR_t;


/**
 * Device Drvier specific information to return to user space.
 * This includes the BAR resources allocated to the device, 
 * interrupts, etc.
 * NOTE: to keep backwards compatibility with older applications
 * that were built against v1.0.0.x drivers, do no add any new
 * fields to this structure or the driver will crash during
 * copying info back to user pages because the size will be different
 * and a fault will occur.  Add new fields to the extra info struct.
 */
typedef struct
{
	// Device Memory Access info
	ULONG numBARs;      /**< Number of active BARs device has been assigned */
	PCI_BAR_t BAR[MAX_PCI_BARS];  /**< structure containing info for all possible BARs */
	UCHAR PCICfgReg[256];  /**< raw PCI config register values */

	// Device Interrupt info
	bool hasInterrupt;   /**< True if device asked for and is assigned an interrupt */
	ULONG intrVector;    /**< The interrupt vector number assigne dto the device */


} PCIResourceInfo_t;



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


/** This IO_CTL is used to get PCI driver name and version
 * back to user space applications.
 */
#define IOCTL_LSCPCIE_GET_VERSION_INFO _IOR(LSCPCIE_MAGIC, 0, DriverVerStr_t)


/** This IO_CTL is used to get PCI and driver information from the
 * driver back to user space applications.
 */
#define IOCTL_LSCPCIE_GET_RESOURCES    _IOR(LSCPCIE_MAGIC, 1, PCIResourceInfo_t)

/**
 * This IO_CTL is used to set the BAR number that is mapped in with the MMAP command.
 */
#define IOCTL_LSCPCIE_SET_BAR     _IOW(LSCPCIE_MAGIC, 2, int)


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
