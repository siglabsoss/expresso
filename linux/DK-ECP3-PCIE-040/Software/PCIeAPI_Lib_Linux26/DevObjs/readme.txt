TARGET PLATFORM: Lattice PCIExpress Eval Board in Win32 OS PC
==============================================================
Platform Details:
Host OS - Windows 2000 or XP
Lattice PCIe Evaluation Boards with LatticeSC or LatticeECP2M Devices


This directory holds the source code for IP device objects (drivers)

Device Objects are C++ representations ofthe IP instantiated in the
FPGA.  The Device Object provides ceratins levels of functionality.
Some do nothingmore thatn provide simple register read/write access.
Other classes may implement driver-like functionality.





 ---------------------------------------------------
 |      PC                                         |
 |                                                 | 
 |  --------------------                           |
 |  |  Application     |                           |
 |  |-------------------                           |
 |  |  API             |                           | 
 |  |-----------------------                       |
 |  |  Driver Interface    |                       |
 |=============================   --------------   |
 |  |   Windows OS        |       | Eval Board |   |
 |  -----------------------       |            |   |
 |  | Device Driver       |       | LSC_FPGA   |   |
 |  ---------+-------------       -||||---------   |
 |           |=====================++++====        |
 |                   PCIExpress Bus                |
 ---------------------------------------------------














