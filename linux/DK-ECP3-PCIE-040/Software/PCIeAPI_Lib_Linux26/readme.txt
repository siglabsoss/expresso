TARGET PLATFORM: Linux
==============================

This directory holds the software to build the Lattice PCIe API library.
The end results is a shared library (.so), 
a static library (if desired instead) and set
of include files.  The user is expected to link their top application
with this library to invoke all the PCIe driver routines.
The API library provides routines to do the following:


This is not a driver.  The Linux kernel mode device drivers are located
in separate driver directories such as lscpcie, lscpcie2, lscdma.
The kernel mode drivers are standalone drivers and do not rely on any files
in this project.

Like-wise this project is a user space library, that does not use any files
found in the driver projects.  The drvier interface files do have copies of
the driver names, board device IDs and demo IP IDs so that they can open the
correct board.  These values are copies.  Manual maintenance must keep the
values synchronized with the drivers.



DevObjs/ - classes to interface to Lattice IP modules instantiated in the
	FPGA fabric.
	
DriverIF - classes to open and interface to the Lattice Kernel Driver that
	binds to a particular evaluation board.
	
HdwSim/ - collection of classes to simulate accessing real hardware.  Used
	primarily when developing and testing code without hardware.  
	May not be 100% accurate simulation.

Utils/ - software utilities used by various modules, such as DEBUGPRINT

Includes/ - copy of all "public" include files that will be included by a user's
	application code.


NOTES ON BUILDING
===================


The LSC_PCIEAPI_DIR environment variable must be defined and have its value
 set to the location of the top of the PCIeAPI library directory tree.
run setup.sh

Build with the following commands:
make allclean
make depends
make

make debug - builds dll library with debug symbols and extra debug logging
make release - builds the dll library with optimization and no debug code included
make static - builds the static link library with optimization and no debug code included
make staticD - builds the static link library with debug symbols and extra debug logging

#define VERBOSE and use with make debug to also print log info to stdout


