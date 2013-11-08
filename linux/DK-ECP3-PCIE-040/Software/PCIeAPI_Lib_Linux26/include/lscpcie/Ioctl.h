/*
 *  COPYRIGHT (c) 2007 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/**
 * @file
 *
 * Define Data structures and ioctl commands to pass between the driver
 * and the user space.  The definitions serve as the interface between
 * the kernel and user.
 *
 * NOTE: This file is included in source code built for the kernel and user.
 * It must be kept compatible with both environments.
 * In particular, structure definitions can not be changed in one space without
 * recompiling the code in the other space.
 */
 
#ifndef LATTICE_SEMI_PCIE_IOCTL_H
#define LATTICE_SEMI_PCIE_IOCTL_H

#include <asm/ioctl.h>

#include "sysDefs.h"

#ifndef MAX_PCI_BARS
#define MAX_PCI_BARS 7
#endif

#define MAX_DRIVER_NAME_LEN 128
#define MAX_DRIVER_VERSION_LEN 128

/**
 * Information about a device's BAR.
 */
typedef char DriverVerStr_t[MAX_DRIVER_NAME_LEN];



/**
 * Information about a device's BAR.
 */
typedef struct
{
	ULONG nBAR;
	ULONG physStartAddr;
	ULONG size;
	bool memMapped;
	USHORT flags;
	UCHAR type;

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
	ULONG numBARs;
	PCI_BAR_t BAR[MAX_PCI_BARS];

	UCHAR PCICfgReg[256];

	// Device Interrupt Info
	bool hasInterrupt;
	ULONG intrVector;

} PCIResourceInfo_t;




/**
 * IOCTL Operations.
 * use these defines when performing an ioctl operation to a device.
 */

#define LSCPCIE_MAGIC 'L'

/** This IO_CTL is used to get PCI and driver information from the
 * driver back to user space applications.
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

/** This IO_CTL is used to get additional information from the
 * driver back to user space applications.  This info is used by the SFIF.
 */
#define IOCTL_LSCPCIE2_GET_EXTRA_INFO     _IOR(LSCPCIE_MAGIC, 3, ExtraResourceInfo_t)



#define IOCTL_LSCPCIE_MAX_NR 3   // ^^^^^^must match last entry above^^^^^^^


#endif
