Lattice Linux PCIe Demo and Driver Source Code
==============================================

This directory contains the source code for building the drivers, library
and demo applications for the PCIe demos.  These drivers and demos access
the Lattice PCIe eval board and demonstrate data movement between the PC
and the board over the PCIe bus.

DOCUMENTATION
-------------

Launch the PCIeDocIndex.html in a browser for detailed help.



SOURCE TREE
-----------

lscdma_Linux26 - kernel module device driver for performing DMA operations
	between the board and PC system memory.  The DMA demo uses this
	driver.

lscpcie2_Linux26 - kernel module device driver for basic control plane access
	to the board.  The Basic and Throuput demos use this driver.

PCIeAPI_Lib_Linux26 - shared library (DLL) that interfaces the demo applications
	to the underlying driver functionality.

PCIeBasic_Linux26 - the basic demo that show control-plane type access to the
	registers and memory in the IP in the FPGA on the eval board.

PCIeSFIF_Linux26 - the Thruput demo that uses IP in the FPGA to send TLPs over
	the PCIe link as fast as possible.  Software measures the rate and can
	vary the traffic patterns.

PCIeDMA_Linux26 - A set of applications that show moving large amounts of data
	between the eval board and system memory using DMA. 


BUILDING
--------

The Makefile builds all the drivers and demos.  
To build the drivers enter:
	make drivers

The kernel header files need to be installed (kernel-devel package)

To build the demos enter:
	make demos
The DMA demos use OpenGL, so the glut.h file needs to be in /usr/include/GL
This header file is installed by the freeglut-devel package.  It may not
be installed by default on a base Linux distribution.


64 bit systems require 64 bit versions of the libraries.
64 bit systems also require the 64 bit version of the Java JRE.



SETUP and INSTALLATION
----------------------
Use the install.sh script located in the Demonstration/ directory to
install the drivers and demos.


