/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file PCIe_IF.h */
 
#ifndef LATTICE_SEMI_PCIE_IF_H
#define LATTICE_SEMI_PCIE_IF_H


#include <unistd.h>
#include <sys/types.h>

#include "dllDef.h"

#include "RegisterAccess.h"

// The driver interface definitions
#include "lscpcie/Ioctl.h"


/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{


#ifndef MAX_PCI_BARS
#define MAX_PCI_BARS 7
#endif

/**
 * Macros for specifying the address in a specific BAR address space.
 * to write to address 0x10 in BAR1 of the device use: BAR1(0x10)
 */
#define BAR0(a) (0x00000000 | a)
#define BAR1(a) (0x10000000 | a)
#define BAR2(a) (0x20000000 | a)
#define BAR3(a) (0x30000000 | a)
#define BAR4(a) (0x40000000 | a)
#define BAR5(a) (0x50000000 | a)
#define BAREXP(a) (0x60000000 | a)



#define OFFSET2BAR(o) (o>>28)
#define OFFSET2ADDR(o) (o & 0x0fffffff)
#define WB(a) (a & 0x0fffffff)

#define BARADDR2BAR(o) (o>>28)
#define BARADDR2OFFSET(o) (o & 0x0fffffff)
#define MAKE_BARADDR(n, o) ((n<<28) | (o & 0x0fffffff))

#define MAX_DEV_FILENAME_SIZE 1024   // maximum chars in a device driver filename
#define MAX_FUNCTION_NAME_SIZE 64   // maximum chars in a driver function string

// TODO - Remove these Windows based defines
#define MAX_DEV_VEND_STR 18     // example fomat is: "ven_1204&dev_5303"
#define MAX_SUBSYS_STR 16 // exmaple format is: "subsys_30301204"
#define MAX_DEV_ID_STR  (MAX_DEV_VEND_STR + MAX_SUBSYS_STR + 4)


#define MAX_BOARD_ID_SIZE 32
#define MAX_DEMO_ID_SIZE 32


/* Define all driver/Board types */
#define LSCPCIE_SC   0x53031204                    // PCI Device ID for the SC board
#define LSCPCIE_EC   0xe2351204                    // PCI Device ID for the ECP2M board

#define MAX_RW_BLOCK_SIZE 4096   // Maximum number of bytes per block transfer operation

typedef struct
{
	uint16_t vendorID;   // 00
	uint16_t deviceID;   // 02
	uint16_t command;    // 04
	uint16_t status;     // 06
	uint8_t  revID;      // 08
	uint8_t  classCode[3];      // 09
	uint8_t  cacheLineSize;      // 0c
	uint8_t  latencyTimer;      // 0d
	uint8_t  headerType;      // 0e
	uint8_t  BIST;      // 0f
	uint32_t BAR0;      // 10
	uint32_t BAR1;      // 14
	uint32_t BAR2;      // 18
	uint32_t BAR3;      // 1c
	uint32_t BAR4;      // 20
	uint32_t BAR5;      // 24
	uint32_t cardBusPtr;      // 28
	uint16_t subsystemVendorID;   // 2c 
	uint16_t subsystemID;         // 2e
	uint32_t expROM_BAR;      // 30
	uint8_t  capabilitiesPtr;      // 34
	uint8_t  reserved[7];
	uint8_t  interruptLine;      // 3c
	uint8_t  interruptPin;      // 3d
	uint8_t  minGrant;      // 3e
	uint8_t  maxLatency;      // 3f
} PCICfgRegs_t;

/**
 * The PCIe_IF class provides the actual basic read/write access to hardware registers.
 * This class implements the only the common methods used by all PCIe driver interfaces.
 * More advanced driver interfaces add functionality based on what they can expect the
 * Windows kernel mode driver to support.  This class gauruntees that the following
 * methods are available:
 * <ul>
 * <li>read8/16/32()
 * <li>write8/16/32()
 * <li>block read versions of the above
 * <li>getPCIConfigRegs()
 * <li>getPCIDriverInfo()
 * <li>getPCIResourcesStr()
 * <li>getDriverVersionStr()
 * <li>getDriverResourcesStr()
 * <li>getPCICapabiltiesStr()
 * </ul>
 *
 * @implementation
 * This class extends the base class to provide the exact methods
 * to access the hardware registers in a LSC FPGA on PCIEpress card.
 * The hardware (FPGA) is accessed using specific DeviceIoControl()
 * calls to the corresponding Windows kernel-mode device driver.
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
 * Addressing convention used throughout: 32 bit addr = [BAR(31:28)][Address(27:0)]
 *
 * <p> The specific driver and board to open are identified by:
 * <ul>
 *<li> GUID = Driver ID = the specific Windows driver to open such as lscpcie2.sys or lscdma.sys
 * <li> Vendor/Device = Board ID = specify the specific board to attach to.  Each type of 
 *     Lattice eval board will have a unique PCI Device ID.
 * <li> SubSystem = Demo ID = specify which IP to connect to.  Each demo IP design has a
 *    specific ID stored in the PCI SubSystem Vendor/Device registers.
 * <li> Instance = which occurrence of the board/demo to attach to if multiple, identical
 *       boards are in the system.
 * </ul>
 * 
 */
class DLLIMPORT PCIe_IF : public LatticeSemi_PCIe::RegisterAccess
{
public:

	// Board Identifiers  (PCI DEV_VEND pair)
	static const char *PCIE_SC_BRD;      //  Generic SC board ID used in older demo designs
	static const char *PCIE_ECP2M_BRD;      // Generic ECP2M board ID used in older demo designs
	static const char *PCIE_ECP2M35_BRD;      // Generic ECP2M board ID used in older demo designs
	static const char *PCIE_ECP2M50_BRD;      // Generic ECP2M board ID used in older demo designs

	static const char *PCIE_ECP3_BRD;      // Generic ECP3 board ID used in demo designs

	// These are more a list of what boards were made, but aren't used to identify demos or FPGAs
	static const char *SC80_PCIE_x8_BRD_GL028;    // SC80, 1152 pkg, x8, SMPTE BNC connectors, 16 SegLED
	static const char *ECP2M35_PCIE_x4_BRD_GL029; // ECP2M35, 672 pkg, SMPTE BNC, 16 SegLED
	static const char *ECP2M50_PCIE_x4_BRD_GL029; // ECP2M50, 672 pkg, SMPTE BNC, 16 SegLED
	static const char *SC25_PCIE_x1_BRD_GL030;    // SC25, 900 pkg, x1 connector, RJ45+SFP+SATA, no 16 Seg
	static const char *ECP2M35_PCIE_x1_BRD_GL031; // ECP2M35, 672 pkg, x1 connector, SFP+SATA
	static const char *ECP2M35_PCIE_x1_BRD_GL034; // ECP2M35 video board, x1 connector, doudle height
	static const char *SC80_PCIE_x4_BRD_GL035;    // SC80(v3), 1152 pkg, x4 connector,  16 SegLED

	// Demo Identifiers  (PCI SUBSYS pair)
	static const char *PCIE_DEMO_SC;    // original PCIe control plane demo for SC80 & 25 boards
	static const char *PCIE_DEMO_EC;    //  original PCIe control plane demo for ECP2M35 boards
	static const char *BASIC_DEMO;      //  original PCIe Basic demo for all boards
	static const char *SFIF_DEMO;       // PCIeThruput demo ID
	static const char *PCIEDMA_DEMO;    // PCIe SGDMA demo ID - SGDMA to/from user space memory
	
	static const GUID *pLSCSIM_DRIVER;
	static const GUID *pLSCPCIE_DRIVER;
	static const GUID *pLSCPCIE2_DRIVER;
	static const GUID *pLSCDMA_DRIVER;


	
	PCIe_IF(const GUID *pGUID, 
		    const char *pBoardID,
			const char *pDemoID,
			uint32_t devNum = 1, 
			const char *pFriendlyName = NULL, 
			const char *pFunctionName = NULL);

    ~PCIe_IF();

	HANDLE getDriverHandle(void);

    bool getPCIConfigRegs(uint8_t *pCfg);
    bool getPCIDriverInfo(const PCIResourceInfo_t **pInfo);

    bool getPCIResourcesStr(string &outs);
    bool getDriverVersionStr(string &outs);
    bool getDriverResourcesStr(string &outs);
    bool getPCICapabiltiesStr(string &outs);

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

protected:
	HANDLE	hDev;
	int OpenDriver(HANDLE &h,
		  	const GUID *pGUID, 
		  	const char *pBoardID,
		  	const char *pDemoID,
		  	uint32_t instance = 1, 
		  	const char *pFriendlyName = NULL, 
		  	const char *pFunctionName = NULL);

	int MmapDriver(void);

	PCIResourceInfo_t PCIinfo;

	volatile void *pBAR[MAX_PCI_BARS];   /**< pointer to mmap'ed BAR into user space */
	size_t         sizeBAR[MAX_PCI_BARS]; /**< size of BAR mmap for releasing */

private:

};

/**
 * Exception class for objects in the Lattice PCIe driver module.
 * This type of exception is thrown if an occurs deep in the
 * driver code and execution can not continue.  The top-level
 * code can determine the exception cause by displaying the
 * char string returned by the what() method.
 */
class DLLIMPORT PCIe_IF_Error : public exception 
{ 
   public:
       PCIe_IF_Error(char *s) {msg = s;}
       virtual const char *what(void) const throw()
       {
           return(msg);
       }

   private:
       char *msg;
};

} //END_NAMESPACE

#endif

