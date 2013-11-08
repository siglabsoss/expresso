    LSCDMA
================

Linux kernel Device Driver used to perform SGDMA over PCI Express on
a Lattice PCIe eval board. The driver is built using the Linux kernel
build tools and devel package.



This directory implements the Linux kerenl device driver for accessing the Lattice
PCIe Eval Boards with the SGDMA IP core.  The main purpose is to create an interface
between the application running in user space (i.e. the demo) and the hardware
registers and memory that can only be addressed via kernel space.  Also the driver handles interrupts from the board.

1.) provide the bridge between User and Kernel space to allow applications to access
hardware

2.) connect to the board's interrupts

3.) allocate and map kernel memory for board accesses into system memory

4.) access kernel resources - timers, memory page mappings, etc.

5.) Transfer data from PC memory to eval board memory using SGDMA (and vice versa)




Files
=====


Driver Files
------------

Main.c - Main entry into the driver.  Probing the PCI bus is done here.  
These functions are called to dicover boards in the system and create and 
initialize database structures to open/track these boards.


Devices.c - the hardware specific code for the driver to manipulate the
GPIO, interrupt controller and SGDMA IP on the eval board.

DMA.c - Linux DMA functions to allocate transfer buffers and map between
user space and hardware addresses.  Uses routines in Devices.c to setup
the hardware.

ISR.c - The Linux interrupt handler to service the SGDMA when its completed
a transfer and needs to be setup for the next transfer.



Install Files
-------------
insdrvr.sh -  driver install script file. Installs with insmod command. 
rmdrvr.sh -  script to remove old drvier before installing new one


BUILDING
=========

Development:
Simple run make.  If all kernel header files are installed on the system
then it will build the driver module file.

