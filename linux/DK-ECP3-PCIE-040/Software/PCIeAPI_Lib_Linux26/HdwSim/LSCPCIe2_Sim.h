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

 
#ifndef LATTICE_SEMI_PCIE_LSCPCIE2_SIM_H
#define LATTICE_SEMI_PCIE_LSCPCIE2_SIM_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <cstring>

#include "SimDemoIP.h"
#include "LSCPCIe2_IF.h"


/*
 * Lattice Semiconductor Corp. namespace for Lattice PCIE demo.
 */
namespace LatticeSemi_PCIe
{




/**
 * Provides the Simulated PCIe Driver Interface to the simualted hardware.
 *
 * @implementation
 * This class extends the PCIe_IF class to provide the exact methods
 * to access the hardware registers in a LSC FPGA on PCIEpress card.
 *
 */
class DLLIMPORT LSCPCIe2_Sim  : public LatticeSemi_PCIe::LSCPCIe2_IF
{
public:


    LSCPCIe2_Sim(LatticeSemi_PCIe::SimDemoIP *pDemo, uint8_t *pCFG0 = NULL, const PCIResourceInfo_t *pDrvrInfo=NULL);
    ~LSCPCIe2_Sim();

    uint8_t read8(uint32_t addr);
    void write8(uint32_t addr, uint8_t val);
    uint16_t read16(uint32_t addr);
    void write16(uint32_t addr, uint16_t val);
    uint32_t read32(uint32_t addr);
    void write32(uint32_t addr, uint32_t val);

	/* Block Access Methods */
    bool read8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true);
    bool write8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true);
    bool read16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true);
    bool write16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true);
    bool read32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true);
    bool write32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true);



    bool getPCIConfigRegs(uint8_t *pCfg);

    bool getPciDriverExtraInfo(const ExtraResourceInfo_t **pExtra);
	bool getPCIExtraInfoStr(string &outs);

	bool writeSysDmaBuf(uint32_t *pData, uint32_t len);
	bool readSysDmaBuf(uint32_t *pData, uint32_t len);


private:
	void *pDrvrParams;


	uint32_t PCI_CFG0_Regs[256/4];   // emulate the cfg registers

	LatticeSemi_PCIe::SimDemoIP *pDemoIP;


	uint8_t *pSysDmaBuf;

};


} //END_NAMESPACE

#endif

