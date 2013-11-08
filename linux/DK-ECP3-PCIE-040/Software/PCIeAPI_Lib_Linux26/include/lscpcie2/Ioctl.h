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
 
#ifndef LATTICE_SEMI_PCIE2_IOCTL_H
#define LATTICE_SEMI_PCIE2_IOCTL_H


#include "lscpcie/Ioctl.h"


/**
 * Additional Device Driver specific information to return to user space.
 * This includes the DMA resources allocated to the device, interrupts, etc.
 * This is for the newer PCIeBasic and PCIeSFIF demos and applications.
 */
typedef struct
{
        // Instance and device location info
        ULONG devID;     /**< board number of specific device */

        ULONG  busNum;         /**< PCI bus number board located on */
        USHORT deviceNum;      /**< PCI device number assigned to board */
        USHORT functionNum;    /**< our function number, which is always 0 (no multi-function */
        ULONG   UINumber;      /**< motherboard slot number (not always implemented/valid) */

        // Device DMA Common buffer memory info
        bool hasDmaBuf;        /**< true if DMA buffer has been allocated by driver */
        ULONG DmaBufSize;      /**< size in bytes of said buffer */
        bool DmaAddr64;        /**< true if the address mode is 64 bit (almost always 32 bit) */
        ULONG DmaPhyAddrHi;    /**< Upper 32 bits of 64 bit address for 64 bit mode */
        ULONG DmaPhyAddrLo;    /**< DMA bus address to be programmed into device */


        char DriverName[MAX_DRIVER_NAME_LEN];   /**< version and name compiled into driver */


} ExtraResourceInfo_t;



/**
 * LSCCPIE2 IOCTL Operations.
 * use these defines when performing an ioctl operation to a device.
 * Controls 0,1,2 are defined in lscpcie/Ioctl.h
 */

#define LSCPCIE_MAGIC 'L'


/** This IO_CTL is used to get additional information from the
 * driver back to user space applications.  This info is used by the SFIF.
 */
#define IOCTL_LSCPCIE2_GET_EXTRA_INFO     _IOR(LSCPCIE_MAGIC, 3, ExtraResourceInfo_t)


/**
 * Controls to support future user space handling of interrupts.
 * Not implemented yet.
 */
#define IOCTL_LSCPCIE2_WAIT_FOR_INTERRUPT       _IO(LSCPCIE_MAGIC, 4)
#define IOCTL_LSCPCIE2_DISABLE_INTERRUPT        _IO(LSCPCIE_MAGIC, 5)


#define IOCTL_LSCPCIE2_MAX_NR 5   // ^^^^^^must match last entry above^^^^^^^


#endif
