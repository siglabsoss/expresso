LSCDMA Driver Tests

This directory holds stand-alone test that verify the driver (built in drvr/ directory)
is working.  These tests do not rely on any Application code or PCIeAPI library methods.
The tests use the standrad linux application build environment and just do basic operations,
such as opening the driver, getting the driver version, accessing guaranteed IP that must
be there.

The tests also exercise the IP (but do not provide reference design code.)
The main purpose is to quickly verify the driver features and IP access.


/**
 * Lattice PCIe + SGDMA IP Core Test Bench
 *
 * This file includes a series of menus and tests that exercise the SGDMAC core in a
 * test system contained in the Lattice FPGA.  The SGDMAC core and GPIO IP are  accessed
 * over the PCIe bus.  The PCIe IP core is also part of the design.
 *
 *
 *   |--------|            |----------|
 *   | GPIO   |            | EBR 64kB |
 *   | 32bit  |            |  64 bit  |
 *   |________|            |__________|
 *      /\                      /\
 *      ||                      ||
 * |----------------------------------------|
 * |     Wishbone Bus                       |
 * |----------------------------------------|
 *  /\                    |         /\
 *  ||                    |         || 64 bit data bus
 *  ||                    |     |----------|
 *  ||                    |     | WBM_A    |
 *  ||                    +---->|    DMA   |
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
 *
 */

Build using the makefile.
$> make open

Run by specifying the device node file name:
./open /dev/lscdma/ECP2M_DMA_1


