/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file SFIF.h */
 
#ifndef LATTICE_SEMI_PCIE_SFIF_H
#define LATTICE_SEMI_PCIE_SFIF_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <time.h>
#include <exception>

#include "dllDef.h"

#include "Device.h"
#include "RegisterAccess.h"
#include "LSCPCIe2_IF.h"

/*
 * Lattice Semiconductor Corp. namespace.
 */
namespace LatticeSemi_PCIe
{
 
// Run Modes for the SFIF
#define THRUPUT_MODE    0   
#define READ_CYC_MODE   1
#define WRITE_CYC_MODE  2    

#define MRD_TLPS  1    
#define MWR_TLPS  2    
#define MRD_MWR_TLPS  3    
#define MRD_MWR_CTRL_TLPS 4    


// all PCs we've seen have these same settings
#define MAX_TLP_SIZE 128   
#define MAX_READ_SIZE 512
#define RCB 64
#define RCB_DW (RCB / 4)
    
#define MAX_ADDRESS     0x40   // register address space module takes up
#define FIFO_SIZE (16 * 1024)   // size of Tx and Rx FIFOs in SFIF

/* Register Definitions */
#define SFIF_CTRL       0x00 // Global Control - Controls operation of SFIF
#define SFIF_TXTLP_CTRL 0x04 // Tx TLP Control	- Tx TLP control of loading TLP into FIFO
#define SFIF_TXICG_CTRL 0x08 // Tx ICG Control - Tx Inter-Cycle TLP Gap control
#define SFIF_TXTLP_DATA 0x0c // Tx TLP Data - DW of TLP data
#define SFIF_RXTLP_DATA 0x10 // Rx TLP Data - DW of Completion TLP data
#define SFIF_ELAPSE_CNT	0x14 // Elapsed Count - Number of clock cycles in current run
#define SFIF_TXTLP_CNT  0x18 // Tx TLP Count - Number of Tx TLPs sent in current run
#define SFIF_RXTLP_CNT  0x1c // Rx TLP Count - Number of CplDs received in current run
#define SFIF_WRWAIT_CNT 0x20 // Wr Wait Time  - Number of clock cycles waiting for Posted Credits in current run
#define SFIF_LAST_CPLD  0x24 // Last CplD Time - Timestamp for last CplD
#define SFIF_RDWAIT_CNT 0x28 // Rd Wait Time  - Number of clocks cycles waiting for Non-Posted Credits or Tags in current run





/**
 * Specific definition of a Lattice SFIF IP module used in demos.
 *
 * This class inherits from the generic Device class to provide the actual read/write
 * methods.  The class needs a RegisterAccess object (the Driver Interface) to pass
 * to the Device class for the hardware access.
 * <p>
 * The SFIF module contains the following registers:
 * <ul>
 * <li> 0x00 Global Control - Controls operation of SFIF
 * <li> 0x04 Tx TLP Control	- Tx TLP control of loading TLP into FIFO
 * <li> 0x08 Tx IPG Control - Tx Inter TLP Gap control
 * <li> 0x0c Tx TLP Data - DW of TLP data
 * <li> 0x10 Rx TLP Data - DW of Completion TLP data
 * <li> 0x14 Elapsed Count - Number of clock cycles in current run
 * <li> 0x18 Tx TLP Count - Number of Tx TLPs sent in current run
 * <li> 0x1c Rx TLP Count - Number of CplDs received in current run
 * <li> 0x20 Wr Wait Time  - Number of clock cycles waiting for Posted Credits in current run
 * <li> 0x24 Last CplD Time - Timestamp for last CplD
 * <li> 0x28 Rd Wait Time  - Number of clocks cycles waiting for Non-Posted Credits or Tags in current run
 *</ul>
 * 
 */
class DLLIMPORT SFIF : public LatticeSemi_PCIe::Device
{
public:
	typedef struct 
	{
	    uint32_t ElapsedTime;
	    uint32_t TxTLPCnt;
	    uint32_t RxTLPCnt;
	    uint32_t WrWaitTime;
	    uint32_t LastCplDTime;
	    uint32_t RdWaitTime;
	} SFIFCntrs_t;

	    
	SFIF(const char *nameStr, 
		 uint32_t addr, 
		 LatticeSemi_PCIe::RegisterAccess *pRA); 

	virtual ~SFIF();
 
	void showRegs(void);
	bool getSFIFParseRxFIFO(string &outs);
	int  getSFIFRegs(long long *elements, int len);
	void  getCntrs(SFIFCntrs_t &cnts);

	uint32_t  getRxFIFO(uint32_t *pBuf, size_t len = 0);

	bool cfgTLP(uint32_t rlxOrd,
				uint32_t snoop,
				uint32_t trafficClass,
				uint32_t poisoned);

	bool setupSFIF(uint32_t runMode, 
			uint32_t trafficMode, 
			uint32_t cycles, 
			uint32_t ICG, 
			uint32_t rdTLPSize, 
			uint32_t wrTLPSize, 
			uint32_t numRdTLPs, 
			uint32_t numWrTLPs,
		    uint32_t pktRdRatio = 4,
	        uint32_t pktWrRatio = 1);


	bool verifyMWrXfer(uint32_t *pUserBuf, size_t len = 0);

	bool startSFIF(bool loop);
	bool stopSFIF();
	void disableSFIF();


private:
	typedef struct _RxBin
	{
		uint32_t buf[MAX_READ_SIZE/4];
		uint32_t index;
		uint32_t size;
	} RxBin_t;
		  
	LSCPCIe2_IF *pDrvr;
	void	*arg;
	bool    loadMRdTLP(uint32_t numDWs, uint32_t rdAddr, uint32_t tag);
	bool    loadMWrTLP(uint32_t numDWs, uint32_t *payload, uint32_t wrAddr);

    uint32_t DmaPhysAddrLo;
	uint32_t DmaCommonBufAddr;
	uint32_t DmaCommonBufSize;

	uint32_t RequesterID;
	uint32_t ReadReqSize;
	uint32_t TotalBytesReq;
	bool     RxFifoValid;
	

	uint32_t ReadDmaBuf[FIFO_SIZE/4];
	uint32_t WriteDmaBuf[FIFO_SIZE/4];
	uint32_t RxFIFO[FIFO_SIZE/4];

	RxBin_t RxCpl[32];

	bool Aligned4k, Aligned128;

	uint32_t RlxOrdBit;
	uint32_t SnoopBit;
	uint32_t TrafficClassBits;
	uint32_t PoisonedBit;

	uint8_t RunTimes;

	uint32_t TxSaveBuf[FIFO_SIZE/4];
	uint32_t *TxSavePtr;
	uint32_t TxSaveLen;
};

/**
 * Exception class for objects in the Lattice PCIe SFIF API module.
 * This type of exception is thrown if an occurs deep in the
 * API code and execution can not continue.  The top-level
 * code can determine the exception cause by displaying the
 * char string returned by the what() method.
 */
class SFIF_Error : public exception 
{ 
   public:
       SFIF_Error(char *s) {msg = s;}
       virtual const char *what(void) const throw()
       {
           return(msg);
       }

   private:
       char *msg;
};

} //END_NAMESPACE

#endif
