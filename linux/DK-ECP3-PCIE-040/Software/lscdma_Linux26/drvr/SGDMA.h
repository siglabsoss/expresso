/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file SGDMA.h
 * Defines and Macros for accessing the SGDMA IP module.
 * The registers are defined and the various bits within each
 * register.  This file mimics the SGDMA class found in the PCIeAPI_Lib
 * DLL, but is used in kernel driver land, not user land.   
 *
 * <p>
 * The SGDMA contains the following registers:
 * <ul>
 * <li> Global configuration and control registers
 * <li> Channel setup registers
 * <li> Buffer Descriptor registers
 * </ul>
 * 
 * Macros are provided to access each of these categories and make
 * configuring channels and buffer descriptors easier.
 */

#ifndef SGDMA_H
#define SGDMA_H


#define SGDMA_ID_VALID    0x12040000

//----------------------------------------
//             DMA IP Defines
//----------------------------------------

// SGDMA IP Registers
#define SGDMA_IPID_REG       0x00
#define SGDMA_IPVER_REG      0x04
#define SGDMA_GCONTROL_REG   0x08
#define SGDMA_GSTATUS_REG    0x0c
#define SGDMA_GEVENT_REG     0x10
#define SGDMA_GERROR_REG     0x14
#define SGDMA_GARBITER_REG   0x18
#define SGDMA_GAUX_REG       0x1c
#define SGDMA_CHAN_CTRL_BASE 0x200
#define SGDMA_BD_BASE        0x400


#define BUS_A   0
#define BUS_B   1
#define PB      2

// Use with SRC_BUS() and DST_BUS() macros  to define BUS size
#define DATA_64BIT   3
#define DATA_32BIT   2
#define DATA_16BIT   1
#define DATA_8BIT    0
  
#define ADDR_MODE_FIFO 0
#define ADDR_MODE_MEM  1
#define ADDR_MODE_LOOP 2

#define EOL       1
#define NEXT      0
#define SPLIT     2
#define BUSLOCK   4
#define AUTORTY   8

#define SRC_BUS(a) (a<<8)
#define SRC_SIZE(a) (a<<10) // 0=8 bit, 1=16 bit, 2=32 bit, 3=64 bit
#define SRC_ADDR_MODE(a) (a<<13)  // 0=FIFO, 1=MEM, 2=LOOP
#define SRC_MEM (1<<13)
#define SRC_FIFO (0<<13)

#define DST_BUS(a) (a<<16)
#define DST_SIZE(a) (a<<18) // 0=8 bit, 1=16 bit, 2=32 bit, 3=64 bit
#define DST_ADDR_MODE(a) (a<<21)  // 0=FIFO, 1=MEM, 2=LOOP
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


#endif
