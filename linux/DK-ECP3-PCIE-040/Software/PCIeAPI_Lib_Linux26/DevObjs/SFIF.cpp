/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file SFIF.cpp */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
using namespace std;

#include "PCIeAPI.h"
#include "DebugPrint.h"
#include "SFIF.h"
#include "MemFormat.h"

using namespace LatticeSemi_PCIe;


/**
 * private register defines of the SFIF
 */
 
#define TIMESTAMP   0
#define TIMESTAMP2  1
#define HEADER0     2
#define HEADER1     3
#define HEADER2     4
#define DATA        5

// TLP Type Definitions
#define MRD 1
#define MWR 0

#define SWAP(A) ((A>>24) | ((A>>8) & 0x0000ff00) | ((A<<8) & 0x00ff0000) | (A<<24))



const char *CplStatusStr[8] = {"SC", "UR", "CRS", "???", "CA", "???", "???", "???"};



/**
 * Construct access to a Lattice Demo SFIF IP module.
 * Register access to the IP in the FPGA device is through the 
 * Driver Interface object instantiated 
 * to access the real hardware and do reads/writes.
 * 
 * The base class Device is initialized to the base address and register size.
 *
 * @param nameStr the name of the FPGA device for unique naming
 * @param baseAddr the physical bus address the device registers start at
 * @param pRA pointer to the LSCPCIe2 driver (which is a RegisterAccess object) for accessing the hardware
 * and for obtaining info about Common Buffer memory addresses, sizes and clearing & checking memory
 * after thruput tests.
 */
SFIF::SFIF(const char *nameStr, 
	   uint32_t baseAddr, 
	   RegisterAccess *pRA) : LatticeSemi_PCIe::Device(pRA, nameStr, baseAddr, MAX_ADDRESS)
{
	ENTER();
	/* Base Class Device has setup the name, address, and range already */

	RunTimes = 0;	 // counter for loading uniques patterns into the transfer buffers
    
	// Default settings
	RlxOrdBit = 1;
	SnoopBit = 0;
	PoisonedBit = 0;
    TrafficClassBits = 0;


	pDrvr = (LSCPCIe2_IF *)pRA;  // has to be a driver of type LSCPCIe2 that was passed in
	Aligned4k = true;
	Aligned128 = true;

	const ExtraResourceInfo_t *pExtra;
	pDrvr->getPciDriverExtraInfo(&pExtra);

		if (pExtra->DmaBufSize < FIFO_SIZE)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"            ERROR\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nDMA Common Buffer smaller than SFIF TX FIFO\n";
			cout<<"Exiting to avoid possible buffer overflows.\n";
			ERRORSTR("DMA Common Buffer < 16KB!\n");
			throw SFIF_Error("SFIF: DMA Common Buffer < 16KB");
		}

		if (pExtra->DmaPhyAddrLo % 4096)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"WARNING!  DMA Common Buffer Base is not 4KB aligned.\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nNo support to handle crossing 4KB address boundary\n";
			Aligned4k = false;
			ERRORSTR("DMA Common Buffer not 4KB aligned!\n");
			throw SFIF_Error("SFIF: DMA Common Buffer not 4KB aligned");
		}

		if (pExtra->DmaPhyAddrLo % 128)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"WARNING!  Base is not 128 aligned.\n";
			cout<<"Writes need to account for crossing 4k boundary\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nNo support to handle crossing 4kB address boundary\n";
			Aligned128 = false;
			ERRORSTR("DMA Common Buffer not 128 aligned!\n");
			throw SFIF_Error("SFIF: DMA Common Buffer not 128 aligned");
		}
		// Build the PCIe TLP requester ID out of PCI Bus/Dev/Func
		RequesterID = ((pExtra->busNum)<<24) | ((pExtra->deviceNum)<<19) | ((pExtra->functionNum)<<16);
		DmaCommonBufAddr = pExtra->DmaPhyAddrLo;
		DmaCommonBufSize = FIFO_SIZE;

	LEAVE();
}

/**
 * Destroy the SFIF IP module. 
 * The device should no longer be accessed using any methods.  All references
 * to the device will now be invalid.
 */

SFIF::~SFIF()
{
	ENTER();
	
}



/**
 * Print the values of the SFIF registers to STDOUT.
 * Use to get a quick dump of the current state of all SFIF registers.
 */
void SFIF::showRegs(void)
{
		cout<<"============================\n";
		cout<<"     SFIF Registers\n";
		cout<<"============================\n";
		cout<<"CONTROL: SFIF[0x00] = "<<hex<<read32(0)<<endl;
		cout<<"TX_TLP_CTRL: SFIF[0x04] = "<<read32(4)<<endl;
		cout<<"TX_ICG: SFIF[0x08] = "<<read32(8)<<endl;
		cout<<"TX_DATA: SFIF[0x0c] = "<<read32(0x0c)<<endl;
		cout<<"RX_DATA: SFIF[0x10] = "<<read32(0x10)<<endl;
		cout<<"ELAPSED_CNT: SFIF[0x14] = "<<read32(0x14)<<endl;
		cout<<"TX_TLP_CNT: SFIF[0x18] = "<<read32(0x18)<<endl;
		cout<<"RX_TLP_CNT: SFIF[0x1C] = "<<read32(0x1c)<<endl;
		cout<<"CREDIT_WR_PAUSE_CNT: SFIF[0x20] = "<<read32(0x20)<<endl;
		cout<<"LAST_CPLD_TS: SFIF[0x24] = "<<read32(0x24)<<endl;
		cout<<"CREDIT_RD_PAUSE_CNT: SFIF[0x28] = "<<read32(0x28)<<endl;

}


/**
 * Start the SFIF operating by setting the Run bit.
 * All other bits are left alone.  Its assumed they have been properly
 * set up in the setupSFIF() routine.
 * @param loop if true then enable continuous looping, else play Tx FIFO once and stop
 * @return true if successfuly started
 */
bool SFIF::startSFIF(bool loop)
{
	ENTER();

	// Get the current operating mode
	uint32_t mode = read32(SFIF_CTRL);

	if (loop)
		mode = mode | 0x0c;	 // turn on the loop & run bits
	else
		mode = mode | 0x04;	 // turn on the run bit

	write32(SFIF_CTRL, mode);	// take out of reset and start running again

	return(true);
}





/**
 * Stop the SFIF operating by clearing the Run bit.
 * All other bits are left alone so the SFIF can be re-started
 * by setting the run bit.
 * The call waits 1 msec to ensure the SFIF is stopped (a cycle completes)
 * before returning to ensure all counters have settled.
 * @return true if successfuly stopped
 */
bool SFIF::stopSFIF()
{
	ENTER();

	uint32_t mode;
	mode = read32(SFIF_CTRL);
	mode = mode & ~0x0c;  // shut off the run bit and the loop bit
	write32(SFIF_CTRL, mode);

	Sleep(1);  // wait to make sure its stopped

	++RunTimes;	 // counter for loading uniques patterns into the transfer buffers

	return(true);
}


/**
 * Disable the SFIF by clearing all bits in control register.
 * Called when exiting to ensure SFIF is no longer running and
 * DMAing into memory.
 */
void SFIF::disableSFIF()
{
	ENTER();

	write32(SFIF_CTRL, 0);	// disable everything

}


/**
 * Set specific global parameters for the all TLP headers that are
 * generated.  See the PCIe spec for details on each field.
 * @param rlxOrd the Relaxed Order bit setting
 * @param snoop the Cache Snoop bit setting
 * @param trafficClass the trafficClass bit setting
 * @param poisoned the Poisoned bit setting
 * @return true if bits are legal values and set
 */
bool SFIF::cfgTLP(uint32_t rlxOrd,
					uint32_t snoop,
					uint32_t trafficClass,
					uint32_t poisoned)
{
	if (rlxOrd)
		RlxOrdBit = 1;
	else
		RlxOrdBit = 0;
	if (snoop)
		SnoopBit = 1;
	else
		SnoopBit = 0;
    if (poisoned)
		PoisonedBit = 1;
	else
		PoisonedBit = 0;
    TrafficClassBits = trafficClass & 0x07;

	return(true);
}

/**
 * Return the various counter values in the SFIF in 64 bit integer format.
 * This is primarily used by the GUI (Java) for computing thruputs and
 * Java doesn't support 32bit unsigned, so values need to be 64 bit.
 * @return number of elements written into the array
 */
int  SFIF::getSFIFRegs(long long *elements, int len)
{
	ENTER();
	elements[0] = read32(SFIF_ELAPSE_CNT);	 // ElapsedTime
	elements[1] = read32(SFIF_TXTLP_CNT);	 // TxTLPCnt
	elements[2] = read32(SFIF_RXTLP_CNT);	 // RxTLPCnt
	elements[3] = read32(SFIF_WRWAIT_CNT);	 // WrWaitTime
	elements[4] = read32(SFIF_LAST_CPLD);	 // LastCplDTime
	elements[5] = read32(SFIF_RDWAIT_CNT);	 // RdWaitTime
	return(6);
}


/**
 * Return the various counter register values in the SFIF in a structure.
 * This doesn't read all the reigsters, just the ones useful for computing
 * throughput and time spent waiting credits.
 * @param cnts the user supplied structure to return the register values in
 */
void  SFIF::getCntrs(SFIFCntrs_t &cnts)
{
	ENTER();
	cnts.ElapsedTime = read32(SFIF_ELAPSE_CNT);	 // ElapsedTime
	cnts.TxTLPCnt = read32(SFIF_TXTLP_CNT);	 // TxTLPCnt
	cnts.RxTLPCnt = read32(SFIF_RXTLP_CNT);	 // RxTLPCnt
	cnts.WrWaitTime = read32(SFIF_WRWAIT_CNT);	 // WrWaitTime
	cnts.LastCplDTime = read32(SFIF_LAST_CPLD);	 // LastCplDTime
	cnts.RdWaitTime = read32(SFIF_RDWAIT_CNT);	 // RdWaitTime
}


/**
 * Return the contents of the SFIF Rx FIFO.
 * Read the RX FIFO register until the FIFO is empty.  The max size is 16 KB dictated
 * by the current IP design.  Less than 16KB may be read, even 0 bytes if the FIFO is
 * already empty.  The raw DWORD contents are returned.  No processing is done on the
 * contents.  The format of the data contents:
 * <br>
 * [time_stamp:1DW][time_stamp:1DW][TLP_header:3DW][TLP_payload:nDW]...
 * @param pBuf where to store the data read.  Must be atleast FIFO_SIZE.
 * @param len number of bytes to read (must be on word boundaries)
 * @return number of bytes read from the FIFO.
 */
uint32_t  SFIF::getRxFIFO(uint32_t *pBuf, size_t len)
{
	uint32_t n, i;

	ENTER();

	if ((len > FIFO_SIZE) || (len == 0))
		len = FIFO_SIZE;

	n = 0;	// number of words read from RX FIFO
	for (i = 0; i < len/4; i++)
	{
		if (read32(SFIF_CTRL) & 0x20)
			break;	// Rx FIFO is empty now - no more data

		pBuf[i] =  read32(SFIF_RXTLP_DATA);
		++n;

	}


	return(n * 4);   // number of bytes read, even though its always DWORDs

}


/**
 * Compare a local saved copy of the transmit FIFO payload to received values.
 * The SFIF LoadMWr saves the payload contents in TxSaveBuf[].
 * Only the first cycle is saved (i.e. only a cycle is loaded into the Tx FIFO)
 * Notes on Cycles: The TLPs sent are only relavant as a single cycle.  Repeating
 * Cycles will write to the same memory addresses that the first one did, so the
 * transfer results of subsequent cycles are indistinguishable from the first
 * cycle. So only need to check one cycles worth.
 * @param pUserBuf pointer to callers data that is the end result of the transfer
 * from the SFIF into PC system memory.
 * @param len number of bytes to compare.  If 0, then use what SFIF recorded as
 * xfer size.
 * 
 * @return true if matches
 */
bool  SFIF::verifyMWrXfer(uint32_t *pUserBuf, size_t len)
{
	uint32_t i;
	uint32_t errs = 0;

	ENTER();

	if ((len > FIFO_SIZE) || (len == 0))
		len = this->TxSaveLen;

	len = len / 4;
	for (i = 0; i < len; i++)
	{
		if (pUserBuf[i] != this->TxSaveBuf[i])
		{
			cout<<"ERROR!  DMA MWr xfer error @ addr: "<<i;
			cout<<"\t"<<hex<<TxSaveBuf[i]<<" != ";
			cout<<pUserBuf[i]<<dec<<endl;
			++errs;
		}
	}

	
	if (errs)
		return(false);
	else
		return(true);

}



/**
 * Examine the contents of the SFIF RX FIFO and parse into recvs CplD TLPs that correspond to 
 * the MRd requests.
 * The TLP contents are formatted and returned in a string for display.  If the FIFO has overflowed
 * or there are more than one set of data
 * @param outs user supplied string to return parsed results into.
 * @return true if successfully parsed; false if contents not parseable (more than one set, overflow, etc)
 */
bool SFIF::getSFIFParseRxFIFO(string &outs)
{
	uint32_t i, j;
	uint32_t n, tag;
	uint32_t len;
	uint32_t numDWs;
	uint32_t nDwRcvd, rx_time, ts;
	int state;
	uint32_t data;
	double   thruput;
	bool status = true;


	// initialize to avoid compiler warnings
	tag = 0;
	len = 0;
	numDWs = 0;
	rx_time = 0;
	ts = 0;
	std::ostringstream oss;

	ENTER();

	n = 0;	// number of words read from RX FIFO
	for (i = 0; i < FIFO_SIZE/4; i++)
	{
		if (read32(SFIF_CTRL) & 0x20)
			break;	// Rx FIFO is empty now - no more data

		RxFIFO[i] =  read32(SFIF_RXTLP_DATA);
		++n;

	}


	// if the RX FIFO contains all the CplD's then parse and use the time stamps
	// otherwise return the raw contents and false.  
	if (this->RxFifoValid)
	{
		// Clear the RX Bins for processing split transactions out of FIFO
		memset(RxCpl, 0, sizeof(RxCpl));

		oss<<"Parsing Received CplDs\n";

		j = 0;
		nDwRcvd = 0;
		state = TIMESTAMP;
		oss<<hex;
		for (i = 1; i < n; i++)
		{
			switch (state)
			{
				case TIMESTAMP:
					if (RxFIFO[i] > 0)
					{
						ts = RxFIFO[i];
						state = TIMESTAMP2;
					}
					break;

				case TIMESTAMP2:
					if (ts == RxFIFO[i])
					{
						oss<<"\n\nTLP["<<j<<"] Time = 0x"<<ts<<endl;
						state = HEADER0;
					}
					else if (RxFIFO[i] == 0)
					{
						state = TIMESTAMP;
					}
					else
					{
						state = TIMESTAMP2;
						ts = RxFIFO[i];
					}
					break;

				case HEADER0:
					if ((RxFIFO[i] & 0x4a000000) == 0x4a000000)
					{
						numDWs = RxFIFO[i] & 0x3ff;
						state = HEADER1;
						oss<<"\tLen=0x"<<numDWs<<" DWs\n";
						++j;
					}
					else
					{
						oss<<"PARSE ERROR!  Expected CplD HDR[0]\n";
						state = TIMESTAMP;
					}
					break;

				case HEADER1:
					if ((RxFIFO[i] & 0xffff0000) != 0)
						oss<<"WARNING: expected CplID=0/0/0\n";
					len = (RxFIFO[i] & 0x0fff);
					oss<<"\tByteCount=0x"<<len;
					if (RxFIFO[i] & 0x1000)
						oss<<"  BCM=1";
					oss<<"   Status: "<<((RxFIFO[i]>>12) & 0x07)<<" ("<<CplStatusStr[(RxFIFO[i]>>12) & 0x07]<<")\n";
					state = HEADER2;
					break;

				case HEADER2:
					if ((RxFIFO[i] & 0xffff0000) != RequesterID)
						oss<<"ERROR: Not Our Cpl! Expected req_ID="<<RequesterID<<endl;
					tag = ((RxFIFO[i] & 0xff00)>>8) & 0x1f;	  // force 5 bit tag
					oss<<"\ttag="<<tag;
					oss<<"   lowAddr="<<(RxFIFO[i] & 0x7f)<<endl;
					state = DATA;
					rx_time = ts;
					break;

				case DATA:
					if (len == this->ReadReqSize)
					{
						// First packet of what could be multiples to fulfill entire read request
						RxCpl[tag].index = 0;
						RxCpl[tag].size = this->ReadReqSize;
					}
					else if (len > this->ReadReqSize)
					{
						oss<<"PARSE ERROR!  Going back to find TIMESTAMP.\n";
						state = TIMESTAMP;
						break;
					}

					data = SWAP(RxFIFO[i]);
					oss<<"\tDATA["<<RxCpl[tag].index<<"]: "<<hex<<data<<endl;
					RxCpl[tag].buf[RxCpl[tag].index] = data;
					++RxCpl[tag].index;
					++nDwRcvd;	// total data DWords read from the RX FIFO (for thruput computation)
					RxCpl[tag].size = RxCpl[tag].size - 4;	 // processed one more DW of payload
					--numDWs;  // processed 1 more DW from this TLP
					--len;
					if ((numDWs == 0) || (RxCpl[tag].size == 0))
						state = TIMESTAMP;
					break;
			}

		}

		if (this->TotalBytesReq != nDwRcvd * 4)
			oss<<"\n\nERROR! asked for "<<dec<<this->TotalBytesReq<<" bytes, but got "<<(nDwRcvd * 4)<<endl;


		// The recvd data (read completions) can now be compared to the transmit pattern in the System DMA buffer
		// If more than 1 cycle was specified, the Rx FIFO has multiple copies of the recvd data, but this parser
		// will only keep the last cycle - each cycle overwrites previous cycle's data.
		// No checking to verify that each read cycle was received correctly.
		//
		oss<<"\nVerifying Received Data\n";
		n = 0;
		bool err = false;
		for (i = 0; i < 32; i++)
		{
			if (RxCpl[i].index > 0)
			{
				for (j = 0; j < RxCpl[i].index; j++)
				{
					if (RxCpl[i].buf[j] != WriteDmaBuf[n])
					{
						err = true;
						break;
					}
					++n;
				}
				if (!err)
					oss<<"ReadReq["<<i<<"]: OK\n";
				else
					oss<<"ReadReq["<<i<<"]: ERROR!  Read Data != WriteDma @ "<<i<<endl;
			}
		}
		thruput = ((nDwRcvd * 4) / (8E-9 * rx_time)) / 1E6;

		oss<<"\nRead: "<<dec<<(nDwRcvd * 4)<<" bytes   ElpsTime: "<<rx_time<<" clks   Thruput: "<<thruput<<" MB/s\n";

		outs = oss.str();
	}
	else
	{

		// Just display the contents since its probably not going to match anything in the PC buffer
		// cause its looped so many times.
		outs.append("Raw Data Display (CplDs overflowed FIFO)\n");
		MemFormat::formatBlockOfWords(outs, 0x0000, n, RxFIFO);

		status = false;   // Let user know that parsing did not happen. Just the raw contents are returned.
	}

	LEAVE();

	return(status);
}



/**
 * Setup the SFIF to operate in a certain mode.
 * Operating mode is controlled by runMode and trafficMode parameters.
 * <p>
 * Operating mode can be either THRUPUT_MODE, READ_CYC_MODE or WRITE_CYC_MODE.
 * THRUPUT_MODE runs continuously.  Cycle mode only runs for the given number
 * of cycles.
 * <p>
 * trafficMode defines the types of TLPs.  There are 4 types that can be created
 * and placed in the TX FIFO.
 * <ul>
 * <li> MRD_TLPS - MRd requests with payload Pattern = <pkt#[31:24]><tag[24:16]><word#[15:8][7:8]>
 * <li> MWR_TLPS _ MWr with a pre-defined payload pattern=<pkt#[31:24]><RunTimes[23:16]><word#[15:0]>
 * <li> MRD_MWR_TLPS - Read and Write TLPs based on ratio
 * <li> MRD_MWR_CTRL_TLPS - same as above (Ctrl not implemented yet)
 * </ul>
 * <p>
 * 
 *
 * This function does not start the SFIF.  That must be done with a call to startSFIF().
 * The operations performed are:
 * <ol>
 * <li> reset the controls and FIFOs
 * <li> clear Sys Common Buf, fill with pattern if RD mode
 * <li> Setup ICG and other simple params
 * <li> determine Xfer type
 * <li> load the Tx FIFO with specifc traffic pattern
 * </ol>
 * 
 * @param runMode  continuous or cycle mode
 * @param trafficMode  RDs, WRs or both
 * @param cycles number of times to replay TX FIFO
 * @param ICG  number of 125MHz clks to wait between cycles, before resending TX FIFO
 * @param rdTLPSize  read request size in bytes
 * @param wrTLPSize  payload size in bytes
 * @param numRdTLPs  number of Rd TLPs per cycle to place in TX FIFO
 * @param numWrTLPs  number of Wr TLPs per cycle to place in TX FIFO
 */
bool SFIF::setupSFIF(uint32_t runMode, 
				     uint32_t trafficMode,
					 uint32_t cycles, 
			         uint32_t ICG, 
			         uint32_t rdTLPSize, 
			         uint32_t wrTLPSize, 
			         uint32_t numRdTLPs, 
			         uint32_t numWrTLPs,
			         uint32_t pktRdRatio,
			         uint32_t pktWrRatio)
{
	uint32_t i;
	uint32_t n, nPkts;
	uint32_t payloadSize, readReqSize, totalBytesReq;
	uint32_t rdNumDWs, wrNumDWs;
	uint32_t pattern;
	uint32_t wrAddr, rdAddr;
	uint32_t payload[MAX_TLP_SIZE / 4];
	uint32_t tag;
//	uint32_t pktRdRatio = 4;
//	uint32_t pktWrRatio = 1;

	ENTER();

	if (cycles < 1)
		cycles = 1;
	else if (cycles > 0xffff)
		cycles = 0xffff;


	DEBUGSTR("Reset FIFOs\n");
	write32(SFIF_CTRL, 0x41);  // turn on enable
	write32(SFIF_CTRL, 0x43);  // assert RESET
	if (runMode == THRUPUT_MODE)
		write32(SFIF_CTRL, (cycles<<16) | 0x49);  // de-assert RESET, filter RX pkts, loop forever, leave enable on
	else
		write32(SFIF_CTRL, (cycles<<16) | 0x41);  // de-assert RESET, leave enable on and filter RX pkts

	write32(SFIF_TXICG_CTRL, ICG);  //  clocks between TLPs


	readReqSize = 0;
	totalBytesReq = 0;



	if (trafficMode == MWR_TLPS)
	{
		DEBUGSTR("Load: MWr\n");
		nPkts = numWrTLPs;
		wrAddr = 0;
		wrNumDWs = wrTLPSize / 4;
		payloadSize = wrTLPSize;

		DEBUGSTR("Clear DMA common buffer\n");
		memset(WriteDmaBuf, 0, sizeof(WriteDmaBuf));
		pDrvr->writeSysDmaBuf(WriteDmaBuf, sizeof(WriteDmaBuf));

		this->TxSaveLen = 0;
		this->TxSavePtr = TxSaveBuf;

		DEBUGSTR("Load TX FIFO data\n"); 
		// pattern=<pkt#[31:24]><RunTimes[23:16]><word#[15:0]>
		for (n = 0; n < nPkts; n++)
		{

			pattern = ((n+1)<<24) | (RunTimes<<16);
			for (i = 0; i < wrNumDWs; ++i)
				payload[i] = pattern | (i<<8) | i;

			loadMWrTLP(wrNumDWs, payload, wrAddr);

			// Keep a copy of what was loaded into the Tx FIFO for comparison later
			if (this->TxSaveLen < sizeof(this->TxSaveBuf))
			{
				memcpy(this->TxSavePtr, payload, wrNumDWs * 4);
				this->TxSavePtr = this->TxSavePtr + wrNumDWs;
				this->TxSaveLen = this->TxSaveLen + wrNumDWs * 4;
			}

			wrAddr = wrAddr + payloadSize;
		}
	}
	else if (trafficMode == MRD_TLPS)
	{
		DEBUGSTR("Load: MRd\n");
		nPkts = numRdTLPs;
		rdAddr = 0;
		rdNumDWs = rdTLPSize / 4;
		readReqSize = rdTLPSize;

		totalBytesReq = readReqSize * nPkts * cycles;

		// Fill Common DMA buffer with pattern to read for verification in SFIF RX FIFO
		// Pattern = <pkt#[31:24]><tag[24:16]><word#[15:8][7:8]>
		for (n = 0; n < nPkts; n++)
		{
			pattern = ((n+1)<<24);
			for (i = 0; i < rdNumDWs; i++)
				WriteDmaBuf[n * rdNumDWs + i] = pattern | ((n%32)<<16) | (i<<8) | i;
		}

		DEBUGPRINT(("Fill DMA common buffer with %d bytes\n", totalBytesReq));
		pDrvr->writeSysDmaBuf(WriteDmaBuf, readReqSize * nPkts);

		DEBUGSTR("Load TX_FIFO with READ TLPs\n");
		tag = 0;
		for (n = 0; n < nPkts; n++)
		{
			loadMRdTLP(rdNumDWs, rdAddr, tag);

			tag = ((tag + 1) & 0x1f);
			rdAddr = rdAddr + readReqSize;
		}

	}
	else if ((trafficMode == MRD_MWR_TLPS) || (trafficMode == MRD_MWR_CTRL_TLPS))
	{
		DEBUGSTR("Load: MWr+MRd\n");
		// nPkts is assumed to be 4 until we resolve whatever causes some system hang-ups
		nPkts = numWrTLPs;
		rdNumDWs = rdTLPSize / 4;
		wrNumDWs = wrTLPSize / 4;
		readReqSize = rdTLPSize;
		rdAddr = FIFO_SIZE / 2;	  // read from middle of buffer
		wrAddr = 0;	 // write to beginning of buffer
		tag = 1;  // tag 0 is reserved for writes so don't use it 

		this->TxSaveLen = 0;
		this->TxSavePtr = TxSaveBuf;

		DEBUGSTR("Clear DMA common buffer\n");
		memset(WriteDmaBuf, 0, sizeof(WriteDmaBuf));
		// Pattern = <pkt#[31:24]><tag[24:16]><word#[15:8][7:8]>
		uint32_t nRdReq = nPkts / pktRdRatio;
		for (n = 0; n < nRdReq; n++)
		{
			pattern = ((n+1)<<24);
			for (i = 0; i < rdNumDWs; i++)
				WriteDmaBuf[rdAddr/4 + (n * rdNumDWs + i)] = pattern | ((n%32)<<16) | (i<<8) | i;
		}

		DEBUGSTR("Fill DMA common buffer with read pattern.\n");
		pDrvr->writeSysDmaBuf(WriteDmaBuf, rdAddr + rdNumDWs*4*nRdReq);


		for (n = 0; n < nPkts; n++)
		{
			// Load the read request
			if (numRdTLPs && (n % pktRdRatio) == 0)
			{
				DEBUGPRINT(("-->MRdTLP: tag=%d\n", tag));
				loadMRdTLP(rdNumDWs, rdAddr, tag);

				++tag;
				if (tag >= 32)
					tag = 1;   // can't use tag 0 cause its used for writes
				rdAddr = rdAddr + readReqSize;
				if (rdAddr >= FIFO_SIZE)
					rdAddr = FIFO_SIZE / 2;	  // wrap back to middle of memory buffer
			}

			if (numWrTLPs && (n % pktWrRatio) == 0)
			{
				DEBUGSTR("-->MWrTLP\n");
				pattern = ((n+1)<<24) | (wrNumDWs<<16);
				for (i = 0; i < wrNumDWs; ++i)
					payload[i] = pattern | (i<<8) | i;

				loadMWrTLP(wrNumDWs, payload, wrAddr);

				// Keep a copy of what was loaded into the Tx FIFO for comparison later
				if (this->TxSaveLen < sizeof(this->TxSaveBuf))
				{
					memcpy(this->TxSavePtr, payload, wrNumDWs * 4);
					this->TxSavePtr = this->TxSavePtr + wrNumDWs;
					this->TxSaveLen = this->TxSaveLen + wrNumDWs * 4;
				}

				wrAddr = wrAddr + wrTLPSize;
				if (wrAddr >= (FIFO_SIZE / 2))
					wrAddr = 0;	  // wrap back to start of memory buffer
			}
		}

	}
	else
		return(false);	// illegal traffic type


	ReadReqSize = readReqSize;
	TotalBytesReq = totalBytesReq;

	if (trafficMode == MRD_TLPS)
	{
	    if ((ReadReqSize > 0) && (TotalBytesReq < (FIFO_SIZE - 2048)))	 // overhead of CplDs & time cnts
		    RxFifoValid = true;
	    else
		    RxFifoValid = false;
	}
	else
	{
		RxFifoValid = false;  // Don't parse SFIF Rx FIFO for MWr or MRd+MWr thruput tests
	}

	LEAVE();

	return(true);
}


/**
 * Load a MWr TLP with appropriate header and payload.
 * Create the 3DW header of the TLP.
 * Write the payload into the TX FIFO
 *
 * @param numDWs number of DWs of payload
 * @param payload pointer to uint32_t words to include into the TLP payload
 * @param wrAddr address into System DMA buffer to start writing at (offset)
 *
 * @return true if successful; false if error such as address or 
 */
bool    SFIF::loadMWrTLP(uint32_t numDWs, uint32_t *payload, uint32_t wrAddr)
{
	uint32_t credits;
	uint32_t i;

	ENTER();
	// check address range of writes
	if ((wrAddr + (numDWs * 4) - 1) >= DmaCommonBufSize)
	{
		ERRORSTR("\nERROR!!! loadMWrTLP addr range error!");
		throw SFIF_Error("loadMWrTLP addr range error!");
		return(false);
	}

	// turn wrAddr into the actual physical memory address
	wrAddr = wrAddr + DmaCommonBufAddr;

	// 1 credit = 4 DWs
	credits = numDWs / 4;
	if (numDWs % 4)
		++credits;

	// Write credit control word twice to clock into credit FIFO
	write32(SFIF_TXTLP_CTRL, 0x8010 | (credits<<5));	 // Ctrl word, Posted credits: hdr & data
	write32(SFIF_TXTLP_DATA, 0);		// dummy writes to clock in control and credits
	write32(SFIF_TXTLP_CTRL, 0x8010 | (credits<<5));	 // Ctrl word, Posted credits: hdr & data
	write32(SFIF_TXTLP_DATA, 0);

	write32(SFIF_TXTLP_CTRL, 0x0001);	 // data word, start
	write32(SFIF_TXTLP_DATA, 0x40000000	  // HDR0: MemWr, 3DW Hdr w/ payload
				  |  (TrafficClassBits<<20)
				  |  (PoisonedBit<<14)
				  |  (RlxOrdBit<<13)
				  |  (SnoopBit<<12)
				  |  numDWs);  // payload size in DWs
	if (numDWs == 1)
		write32(SFIF_TXTLP_DATA, RequesterID | 0x0000000f);	  // HDR1: tag=0, 1st_BE = full, no last_BE
	else
		write32(SFIF_TXTLP_DATA, RequesterID | 0x000000ff);	  // HDR1: tag=0, 1st_BE, last_BE


	if (numDWs == 1)
	{
		write32(SFIF_TXTLP_CTRL, 0x0002);	 // data word, end, full 64 bit
		write32(SFIF_TXTLP_DATA, (wrAddr & 0xfffffffc));	 // HDR[2]=addr,  2 lsb's must be 0
		write32(SFIF_TXTLP_CTRL, 0x0002);	 // data word, end
		write32(SFIF_TXTLP_DATA, SWAP(payload[0]));	 // first DW
	}
	else
	{
		write32(SFIF_TXTLP_CTRL, 0x0000);	 // data word
		write32(SFIF_TXTLP_DATA, (wrAddr & 0xfffffffc));	 // HDR[2]=addr,  2 lsb's must be 0

		write32(SFIF_TXTLP_DATA, SWAP(payload[0]));	 // first DW

		for (i = 1; i < numDWs - 1; ++i)
		{
			write32(SFIF_TXTLP_CTRL, 0x0000);	 // data word
			write32(SFIF_TXTLP_DATA, SWAP(payload[i]));
		}

		write32(SFIF_TXTLP_CTRL, 0x000a);	 // last data word, end, DWEN
		write32(SFIF_TXTLP_DATA, SWAP(payload[numDWs - 1]));
		write32(SFIF_TXTLP_CTRL, 0x000a);	 // data word, end, DWEN
		write32(SFIF_TXTLP_DATA, 0);	// junk word to fill out 64 bit FIFO word
	}

	LEAVE();

	return(true);
}


/**
 * Load a MRd TLP with appropriate header into the TX FIFO.
 * Create the 3DW header for the TLP.
 *
 * @param numDWs number of DWs of payload to ask for in the read
 * @param rdAddr address into System DMA buffer to start reading at (offset)
 * @param the tag to add to the read request.  Each outstanding request needs a 
 * unique ID.
 *
 * @return true if successful; false if error such as address out of range
 */
bool    SFIF::loadMRdTLP(uint32_t numDWs, uint32_t rdAddr, uint32_t tag)
{
	uint32_t cplds;

	ENTER();
	// check address range of read request
	if ((rdAddr + ((numDWs * 4) - 1)) >= DmaCommonBufSize)
	{
		ERRORSTR("\nERROR!!! loadMRdTLP addr range error!");
		throw SFIF_Error("loadMRdTLP addr range error!");
		return(false);
	}

	// Turn rdAddr into the actual physical memory address
	rdAddr = rdAddr + DmaCommonBufAddr;

	if ((numDWs % RCB_DW) == 0)
		cplds = numDWs / RCB_DW;
	else
		cplds = numDWs / RCB_DW + 1;   // expect one extra back with partial data
	if (cplds > 15)
	{
		ERRORSTR("RANGE ERROR!!! Number of expected CplDs > 15!  Won't fit in SFIF field!\n");
		throw SFIF_Error("loadMRdTLP: CplDs RANGE ERROR");
	}

	// Write credit control word twice to clock into credit FIFO
	write32(SFIF_TXTLP_CTRL, (tag<<21) | (cplds<<17) | (MRD<<16) | 0x8200); // Ctrl word, 1 credit NonPosted Read Header
	write32(SFIF_TXTLP_DATA, 0);		// dummy writes to clock in control and credits
	write32(SFIF_TXTLP_CTRL, (tag<<21) | (cplds<<17) | (MRD<<16) | 0x8200); // Ctrl word, 1 credit NonPosted Read Header
	write32(SFIF_TXTLP_DATA, 0);

	write32(SFIF_TXTLP_CTRL, 0x0001);	 // data word, start
	write32(SFIF_TXTLP_DATA, 0x00000000	  // HDR0: MRd, 3DW Hdr w/ payload
				  |  (TrafficClassBits<<20)
				  |  (PoisonedBit<<14)
				  |  (RlxOrdBit<<13)
				  |  (SnoopBit<<12)
				  |  numDWs);  // read request size in DWs
	if (numDWs == 1)
		write32(SFIF_TXTLP_DATA, RequesterID | 0x0000000f | (tag<<8));	// HDR1: tag, 1st_BE = full, no last_BE
	else
		write32(SFIF_TXTLP_DATA, RequesterID | 0x000000ff | (tag<<8));	// HDR1: tag, 1st_BE, last_BE

	write32(SFIF_TXTLP_CTRL, 0x000a);	 // data word, end, dwen
	write32(SFIF_TXTLP_DATA, (rdAddr & 0xfffffffc));	 // HDR[2]=addr,  2 lsb's must be 0
	write32(SFIF_TXTLP_CTRL, 0x000a);	 // data word, end, dwen
	write32(SFIF_TXTLP_DATA, 0);	 // junk word to fill out 64 bit FIFO word

	LEAVE();

	return(true);

}



