Lattice PCIe Development Kit Demonstrations for Linux

This directory contains the executable demo applications and device drivers
for the Lattice PCIe Development Kit.  It is targetted for the 2.6 version
of the Linux kernel.



INSTALLING
==========
1.) Install demos and drivers
	- run the install script: ./install.sh
	- you will be prompted for the root password
	- if an eval board is installed, it should display an "I" on the LEDs

2.) Rebuild the drivers
	- if errors occur during install, the drivers may not match the kernel
	- change to the Software/ directory and run "make drivers"
	- repeat step 1

3.) Verify install
	- run "/sbin/lsmod | grep lsc" and it should show lscpcie2  and lscdma


RUNNING
=======
Icons will be placed on the desktop.
Click to run a demo.  
All demos are setup to default to running on the first found ECP2M board.



