/*
 *  COPYRIGHT (c) 2007 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/**
 * Memory Map of all hardware devices in the PCIEDMA IP
 * The hardware layout is:
 * <verbatim>
 *
 *   |--------|   |---------|  |----------|
 *   | GPIO   |   | IMG_FIFO|  | EBR_64   |
 *   | 32bit  |   | 64 bit  |  |  64 bit  |
 *   |________|   |_________|  |__________|
 *      /\            /\            /\
 *      ||            ||            ||
 * |----------------------------------------|
 * |     Wishbone Bus  64 bit               |
 * |----------------------------------------|
 *  /\                    |         /\
 *  ||                    |         || 64 bit data bus
 *  ||                    |     |----------|
 *  ||                    |     | WBM_A    |
 *  ||                    +---->|  SGDMA   |
 *  ||                          |_WBM_B____|
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
#define memSGDMA(i)    ((BAR0(0x2000)) + i)
#define memIMG_FIFO(i) ((BAR0(0x4000)) + i)
#define memEBR_64(i)   ((BAR0(0x10000)) + i)

#define EBR64_SIZE (64 * 1024)


#endif
