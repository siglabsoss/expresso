/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/*
 * Hardware Access to the Lattice FPGA via PCI Express.
 * This class inherits from RegisterAccess and implements all the
 * required methods using Win32 device driver IOCTL to read/write
 * registers in an FPGA on a PCIexpress card.
 */

 
#ifndef LATTICE_SEMI_PCIE_SIM_DEMO_IP_H
#define LATTICE_SEMI_PCIE_SIM_DEMO_IP_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>


#include "SimHdw.h"

/*
 * Lattice Semiconductor Corp. namespace for Lattice PCIE demo.
 */
namespace LatticeSemi_PCIe
{


#define MAX_HDW_DEVICES 16


/**
 * Provides the actual read/write access to hardware registers.
 * This class implements the methods.
 *
 * @implementation
 * This class extends the base class to provide the exact methods
 * to access the hardware registers in a LSC FPGA on PCIEpress card.
 * The hardware (FPGA) is accessed through the device driver in the
 * lscpcie subdirectory.
 *
 * <p> A PCI Express device is accessed through the PCI bus address space
 * mapped into a processor's memory map, so it is basically a PCI device
 * as far as the software is concerned.  A PCI device can have 6 windows
 * of addresses into its onboard memory.  These are known as BARs.  Each
 * BAR can start at an arbitrary address in the processor's map.  Therefore
 * to read/write to registers in different BARs of the device, the correct
 * base address needs to be specified.  Since the base address is completely
 * arbitrary (setup by the OS at boot), the software will pass the BAR it
 * wants to access in the upper nibble of the addr parameter to the read/write
 * methods.  The access layer driver code will then pass that BAR to the device
 * driver code so it can generate the proper CPU bus address to reach the BAR
 * of the device on the PCI bus.
 * 
 * <p> The BAR is included in the address so the signature of the class does
 * not change from the RegisterAccess class because some useful utilities 
 * could be used for bit access, and they rely on a RegisterAccess type interface.
 * So adding another parameter for BAR number is not done.
 */
class DLLIMPORT SimDemoIP
{
public:


    SimDemoIP(bool freeOnClose = true);
    ~SimDemoIP();

    bool addDev(LatticeSemi_PCIe::SimHdw *pDev);

    bool read8(uint32_t offset, uint8_t &val);
    bool write8(uint32_t addr, uint8_t val);
    bool read16(uint32_t addr, uint16_t &val);
    bool write16(uint32_t addr, uint16_t val);
    bool read32(uint32_t addr, uint32_t &val);
    bool write32(uint32_t addr, uint32_t val);

	/* Block Access Methods */
    bool read8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true);
    bool write8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true);
    bool read16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true);
    bool write16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true);
    bool read32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true);
    bool write32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true);





private:

	LatticeSemi_PCIe::SimHdw *findDev(uint32_t addr);


	LatticeSemi_PCIe::SimHdw *DevList[MAX_HDW_DEVICES];
	int numHdwDevs;
	bool FreeDevicesOnClose;
};


} //END_NAMESPACE

#endif

