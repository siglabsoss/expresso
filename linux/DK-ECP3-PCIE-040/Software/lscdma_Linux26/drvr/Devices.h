/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Devices.h
 * Defines and Macros for accessing hardware devices on the eval board.
 * The base addresses and their BAR's are specified.
 * 
 * Accessing hardware uses the addressing convention [BAR:31..28][Offset:27..0]
 * See BAR0() def below.
 */

 
#ifndef DEVICES_H
#define DEVICES_H


#ifndef MAX_PCI_BARS
#define MAX_PCI_BARS 7
#endif

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
//   --- NOTICE --- NOTICE --- NOTICE
// This is the requested size for the common buffer
// DMA block.  The kernel may not give us this much
// memory, so always use pBrd->CBDMA.DmaBufSize
// for length checks.  It has the actual size.
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#define SYS_MEM_BUF_SIZE (64 * 1024)  /**< 64kB buffer for requested common buffer DMA */
#define DMA_MEM_BUF_SIZE (64 * 1024)  /**< 64kB buffer for requested common buffer DMA */

#ifndef SYS_DMA_BUF_SIZE 
#define SYS_DMA_BUF_SIZE (64 * 1024)  /**< 64kB buffer for requested common buffer DMA */
#endif

#define SGDMA_MAX_SIZE (1024 * 1024)  /**< 1 MB is maximum amount SGDMA will move */


/**
 * These defines set the max number of Buffer
 * Descriptors that are available for the Read
 * channel and the Write channel.  This greatly
 * effects the number of pages that can be sent
 * in one DMA operation.
 * Max is 256 (WRs + RDs)
 */
#define MAX_DMA_WR_BDS 256  /**< max for ImgFIFO 1MB=256 * 4K */
#define MAX_DMA_RD_BDS 16   /**< max for EBR; 64KB = 16 * 4K */



typedef ULONG BARAddr_t;	 /**< Definition of the Driver Address type using BAR+Offest */

/**
 * Macros for specifying the address in a specific BAR address space.
 * To write to address 0x10 in BAR1 of the device use: BAR1(0x10)
 */
#define BAR0(a) (0x00000000 | a)
#define BAR1(a) (0x10000000 | a)
#define BAR2(a) (0x20000000 | a)
#define BAR3(a) (0x30000000 | a)
#define BAR4(a) (0x40000000 | a)
#define BAR5(a) (0x50000000 | a)
#define BAREXP(a) (0x60000000 | a)

#define OFFSET2BAR(o) (o>>28)            /**< extract BAR from a Driver Device Address */
#define OFFSET2ADDR(o) (o & 0x0fffffff)  /**< extract device bases offset from a Driver Device Address */

#define BARADDR2BAR(o) (o>>28)	 /**< extract BAR from the full Driver Address */ 
#define BARADDR2OFFSET(o) (o & 0x0fffffff)	 /**< extract device base offset from the full Driver Address */ 
#define MAKE_BARADDR(n, o) ((n<<28) | (o & 0x0fffffff))  /**< create Drive Device Address from BAR and base offset */

#define MAX_RW_BLOCK_SIZE 4096   /**< Maximum number of bytes per block transfer operation */


/*=====================================================================*/
/* NOTE:
 * For the Linux version of the driver, we are only supporting a
 * single BAR for the control plane access to the IP.  In the case
 * of the DMA demo, BAR0 is that such BAR.  It is registered and
 * mapped based on the demo type.
 * The BAR macros really aren't needed then, since BAR0() equates to
 * nothing added to the device base address.  We're keeping the 
 * macros and usage though, just in case we want to add multiple
 * BAR support in the future.
 */
/*=====================================================================*/

#define GPIO(i)     ((BAR0(0x0000)) + i)  /**< address of GPIO IP module */
#define SGDMA(i)    ((BAR0(0x2000)) + i)  /**< address of SGDMA IP module */
#define IMG_FIFO(i) ((BAR0(0x4000)) + i)  /**< address of ColorBar IP module */
#define EBR_64(i)  ((BAR0(0x10000)) + i)  /**< Address of EBR for image move storage */
#define EBR64_SIZE (64 * 1024)            /**< size of EBR storage for Image Move */
#define IMG_FIFO_FRAME_SIZE (1024 * 1024) /**< size of ColorBar image frame */

#define WB(a) (a & 0x0fffffff)   /**< convert the system (PC) resource addr to the board's local Address */


#include "SGDMA.h"
#include "GPIO.h"

#endif
