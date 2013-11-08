/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file
 * Memory Map of all hardware devices in the PCIE Basic demo IP
 * The hardware layout is:
 * <verbatim>
 *
 *  DIP Sw  16 Seg LED
 *     |     ^
 *     v     |
 *   |--------|                |----------|
 *   | GPIO   |                | EBR      |
 *   |        |                |   16KB   |
 *   | 32bit  |                |  32 bit  |
 *   |________|                |__________|
 *      /\                          /\
 *      ||                          ||
 * |----------------------------------------|
 * |     Wishbone Bus  32 bit bus           |
 * |----------------------------------------|
 *                /\                
 *                ||              
 *                ||               
 *                \/
 *           |-----------|
 *           | WBM_TLC   |
 *           |-----------|
 *                /\                
 *                ||
 *                ||
 *                \/
 *           -------------
 *           |   PCIe    |
 *           |  IP Core  |
 *           |___________|
 *                /\
 *                ||
 *              PC Chipset
 *
 * </verbatim>
 *
 * <p>
 * <B><I>IMPORTANT NOTE</I></B> The BAR size is 128kB, but the 2 slave devices
 * only respond to memory accesses within addresses 0x0000 to 0x4fff.  Accesses
 * beyond this range will hanng the Wishone bus because no slave
 * will be accessed to generate an ACK.
 */
#ifndef LATTICE_SEMI_MEMMAP_H
#define LATTICE_SEMI_MEMMAP_H



#define GPIOreg(i)     ((BAR1(0x0000)) + i)
#define EBRreg(i)     ((BAR1(0x1000)) + i)
#define BAR0reg(i)     ((BAR0(0x0000)) + i)
#define BAR1reg(i)     ((BAR1(0x0000)) + i)

#define EBR_SIZE (16 * 1024)
#define BAR0_SIZE (128 * 1024)
#define BAR1_SIZE (128 * 1024)



//-----------------------------------------
// GPIO registers and bits specific to SFIF
//-----------------------------------------

#define GPIO_IN  0x18
#define GPIO_OUT 0x1c

// Counter Control Reg Bit Defs
#define CNTR_RUN   1
#define CNTR_ZERO  2 
#define CNTR_HW_EN 4 
#define CNTR_FRZ   8

// Counter Register Addresses
#define CNTR1_CTRL 0x20
#define CNTR1_CNT  0x24
#define CNTR2_CTRL 0x28
#define CNTR2_CNT  0x2c

// Pulse Generator Control Reg Bit Defs
#define PLSGEN_RUN   1
#define PLSGEN_HW_EN 2 
#define PLSGEN_DONE  4 

// Pulse Generator Register Addresses
#define PLSGEN1_CTRL   0x40
#define PLSGEN1_CYCS   0x44
#define PLSGEN1_LOW    0x48
#define PLSGEN1_PERIOD 0x4c

#define PLSGEN2_CTRL   0x50
#define PLSGEN2_CYCS   0x54
#define PLSGEN2_LOW    0x58
#define PLSGEN2_PERIOD 0x5c



#endif
