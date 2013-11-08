/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file GPIO.h
 * Defines and Macros for accessing the GPIO IP module.
 * The registers are defined and the various bits within each
 * register.  This file mimics the GPIO class found in the PCIeAPI_Lib
 * DLL, but is used in kernel driver land, not user land.   
 *
 * <p> The GPIO module is a Wishbone slave at a ceratin base address
 * normally BAR0 address 0.  The GPIO contains the following registers:
 * <ul>
 * <li> ID register
 * <li> Scratch Pad register
 * <li> LED controls and DIP switch inputs
 * <li> Various counters and controls for the SGDMA hdw
 * </ul>
 * In addition to the standard, basic GPIO registers (same design
 * used in all demos) the GPIO now also contains an interrupt
 * controller section with the following registers:
 * <ul>
 * <li> ID register
 * <li> Interrupt Status
 * <li> Interrupt Control
 * <li> Interrupt Enable
 * </ul>
 * 
 */

#ifndef GPIO_H
#define GPIO_H


/* Register Definitions */
#define GPIO_ID_REG       0x00
#define GPIO_SCRATCH      0x04
#define GPIO_LED16SEG     0x08
#define GPIO_DIPSW        0x0a
#define GPIO_CNTRCTRL     0x0c
#define GPIO_CNTRVAL      0x10
#define GPIO_CNTRRELOAD   0x14
#define GPIO_DMAREQ       0x18
#define GPIO_WR_CNTR      0x1c
#define GPIO_RD_CNTR      0x20

// Interrupt controller is in GPIO block
#define INTRCTL_ID_REG     0x100
#define INTRCTL_CTRL       0x104
#define INTRCTL_STATUS     0x108
#define INTRCTL_ENABLE     0x10c



// DownCounter Control Reg Bit Defs
#define CNTR_RUN     1
#define CNTR_RELOAD  2 
#define CNTR_INTR_EN 4    // enable interrupt when count reaches 0 (not implemented)


// DMA Req/Ack Reg Bit Defs
#define DMA_REQ_WR  1
#define DMA_REQ_RD0 2
#define DMA_REQ_RD1 4
#define DMA_REQ_RD2 8
#define DMA_REQ_RD3 0x10

#define DMA_ACK_WR  0x0100
#define DMA_ACK_RD0 0x0200
#define DMA_ACK_RD1 0x0400
#define DMA_ACK_RD2 0x0800
#define DMA_ACK_RD3 0x1000


// Interrupt Controller Reg Bit Defs
#define INTRCTL_OUT_ACTIVE 0x01
#define INTRCTL_TEST_MODE  0x02
#define INTRCTL_OUTPUT_EN  0x04
#define INTRCTL_INTR_TEST1 0x0100
#define INTRCTL_INTR_TEST2 0x0200
#define INTRCTL_TEST1_EN   0x0001
#define INTRCTL_TEST2_EN   0x0002
#define INTRCTL_DOWN_COUNT_EN          0x0020
#define INTRCTL_INTR_DOWN_COUNT_ZERO   0x0020

#define INTRCTL_INTR_WR_CHAN 0x0001
#define INTRCTL_INTR_RD_CHAN 0x0002


#define OLD_GPIO_ID_VALID    0x53030100
#define GPIO_ID_VALID    0x12043010
#define INTRCTL_ID_VALID 0x12043050


#endif
