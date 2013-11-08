AccessLayer for Windows

Access to the Lattice Eval Board is via the PCI-Express bus on the PC and
the PCI-E IP in the Lattice FPGA.  The lscpcie device driver provides
the interface between the user-space/Windows Kernel/hardware.

lscpcie is shared with the PCIeDemo.  It is only included here in this
project to pull in the needed header files.  Modifications to the driver
will appear in the PCIeDemo.  It is recommended that changes not be made
to the driver.  If they are they must be compatible with PCIeDemo.
