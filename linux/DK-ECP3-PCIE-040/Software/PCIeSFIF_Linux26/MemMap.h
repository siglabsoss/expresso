/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/**
 * Memory Map of all hardware devices in the PCIE SFIF IP
 * The hardware layout is:
 * <verbatim>
 *
 *   |--------|                |----------|
 *   | GPIO   |                | EBR      |
 *   | 32bit  |                |  64 bit  |
 *   |        |                |          |
 *   |________|                |__________|
 *      /\            /\            /\
 *      ||            ||            ||
 * |----------------------------------------|
 * |     Wishbone Bus  64 bit               |
 * |----------------------------------------|
 *  /\                    |        
 *  ||                    |       
 *  ||                    |     |----------|
 *  ||                    |     | SFIF     |
 *  ||                    +---->| Ctrl     |
 *  ||                          | FIFO's   |
 *  ||                          |_____ ____|
 *  ||                             ||   
 *  ||                             \/   64 bit data bus
 * |-----------|          |-----------------|
 * | WBM_TLC   |          | Adapter Logic   |
 * |-----------|          |-----------------|
 *           ||             ||
 *        ----------------------
 *         \__________________/
 *                /\
 *                ||
 *                \/
 *            -------------
 *            |   PCIe    |
 *            |  IP Core  |
 *            |___________|
 *                 /\
 *                 ||
 *              PC Chipset
 *
 * </verbatim>
 */
#ifndef LATTICE_SEMI_MEMMAP_H
#define LATTICE_SEMI_MEMMAP_H



#define memGPIO(i)     ((BAR0(0x0000)) + i)
#define memSFIF(i)     ((BAR0(0x1000)) + i)



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
