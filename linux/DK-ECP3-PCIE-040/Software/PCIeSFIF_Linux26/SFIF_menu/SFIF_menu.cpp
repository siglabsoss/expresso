/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file SFIF_menu.cpp */
/**
 * 
 * \page SFIF_page PCIe SFIF Menu Program
 *
 * SFIF_menu is a console app in <B>SFIF_menu.cpp</B> that allows various
 * TLPs types to be
 * sent from the PCIe Eval board to the Root Complex.  The SFIF IP
 * core is used to test the PCIe link.  The SFIF IP reads TLPs from a
 * Tx FIFO.  These TLPs are loaded by software (this program) based on
 * user input.  Various types of TLPs can be sent (MRd, MWr).  The
 * following diagram illustrates the components of the SFIF IP module.
 * The main points are the Control registers that allow loading the 
 * Transmit FIFO and reading the Receive FIFO and controlling operation,
 * and the counter registers that are used for tracking the activity
 * and computing the thruput.  The SFIF is accessed and controlled 
 * via the Wishbone slave interface.  The SFIF operates in 2 basic modes:
 * (1) cycle mode (2) thruput mode.  In cycle mode the SFIF plays out the
 * TX FIFO the programmed number of cycles.  In thruput mode the 
 * SFIF just keep playing out the TX FIFO over and over until its stopped.
 * Counters are 32 bits, and will wrap (based on the 125 MHz PCIe clk)
 * in 30 seconds, so don't run longer than 30 seconds or the results will
 * be unusable.
 * 
 * <p>
 * \verbatim
                   -----+--------- Wishbone Bus
	   SFIF         |
	  --------------|----------------
	  |      |     WBs     |        |
	  |      -------+-------        |
	  |             |               |
	  |       ------+-------        |
	  |       |  Ctrl &    |        | 
	  |    +--|  Cntrs     +---+    | 
	  |    |  --------------   |    |
	  | ---+----           ----+--- |
	  | |  TX  |           |  Rx  | |
	  | | FIFO |           | FIFO | |
	  | --------           -------- |
	  -----|------------------/|\----
               |                   |
              \|/                  |
             MWr/MRd              CplD  
                   PCIe IP core
 \endverbatim
 *
 * <p>
 * The SFIF hardware is controlled through the SFIF class that is part of the PCIeAPI_Lib.
 * The SFIF class provides methods to load TLPs into the TX FIFO, start the SFIF and read
 * back the resulting counters.  This program does not directly access the SFIF registers.
 * See the SFIF class for details on internal operation.  Some registers that are used for
 * computing resulting transfer thruputs are:
 *
 * <p>
 * <B>SFIF Counter Registers</B>
 * <ul>
 * <li> ElapsedTime - total clks between while SFIF is enabled
 * <li> TxTLPCnt - number of TLPs sent from the TX FIFO while the SFIF is enabled
 * <li> RxTLPCnt - number of CplDs recvd, counts can occur well after SFIF is enabled
 * <li> WrWaitTime - number of clk cycles when a MWr TLP needed to be sent but no credits available
 * <li> LastCplDTime - time stamp (clk cycles since SFIF started) of the last CplD recvd, used for MRd thruput calculations
 * <li> RdWaitTime - number of clk cycles when a MRd TLP needed to be sent but no credits available
 * </ul>
 *
 *
 * 
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


using namespace std;


#include "PCIeAPI.h"         // the APIs
#include "LSCPCIe2_IF.h"     // the driver interface
#include "GPIO.h"            // the GPIO IP module
#include "SFIF.h"            // the SFIF IP module
#include "MiscUtils.h"
#include "MemFormat.h"
#include "../MemMap.h"       // IP module base addresses



//////////////////////////////////////////////
// Required to use Lattice PCIe methods
//////////////////////////////////////////////
using namespace LatticeSemi_PCIe;


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*               GLOBAL VARIABLES                                           */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

LSCPCIe2_IF *pDrvr;
SFIF        *pSFIF;
GPIO        *pGPIO;

// Used for verifying access to driver Common DMA Buffer 
uint32_t WriteDmaBuf[FIFO_SIZE/4];
uint32_t ReadDmaBuf[FIFO_SIZE/4];

/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*            FUNCTION PROTOTYPES                                           */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

void    writeTlpMenu(void);
void    readTlpMenu(void);
void    thruputTests(void);
void    changeConfigSettings(void);
void    GPIOTest(void);
void    showSFIFCounters(SFIF::SFIFCntrs_t &sfifCnts);

/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

//////////////////////////////////////////////
//             Main Entry
//////////////////////////////////////////////

/**
 * SFIF Menu Console App Entry Point.
 * Open access to an eval board that has the PCIe Thruput demo IP
 * loaded into the FPGA.  Then create instances of the SFIF and GPIO
 * devices to control the demo hardware.
 * Then display menu options to let the user exercise various TLP 
 * transfers and compute the resulting throughput.
 * <p>
 * Requires the standard environment variables to be defined to identify
 * which eval board to connect to and operate on.
 */
int main(int argc, char **argv, char **env)
{
	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
	uint32_t boardNum;
	string infoStr;
	uint32_t rd32_val;

	uint32_t ui;
	int i;
	char line[80];


	cout<<"\n\n========================================================\n";
	cout<<"           Lattice PCIe SFIF Menu\n";
	cout<<"                 ver2.0.0\n";
	cout<<"========================================================\n";


	cout<<"\n\n======================================\n";
	cout<<"    Instantiate the PCIeAPI DLL class\n";
	cout<<    "======================================\n";
	PCIeAPI theDLL;
	cout<<"Dll version info: "<<theDLL.getVersionStr()<<endl;

	// Setup so lots of diag output occurs from tests
	theDLL.setRunTimeCtrl(PCIeAPI::VERBOSE);

	// Get the environment variables needed to open the desired board
	// Pre-load with defaults
	strcpy(boardName, "ven_1204&dev_5303");
	strcpy(demoName, "subsys_30301204");
	if (!GetPCIeEnvVars(boardName, demoName, boardNum))
		cout<<"\nEnvVars not found.  Using defaults.\n";

	cout<<"boardName = "<<boardName<<endl;
	cout<<"boardNum = "<<boardNum<<endl;
	cout<<"demoName = "<<demoName<<endl;


	try
	{
		cout<<"\n\n======================================\n";
		cout<<    "       Driver Interface Info\n";
		cout<<    "======================================\n";
		cout<<"Opening LSCPCIe2_IF...\n";
		pDrvr = new LSCPCIe2_IF(boardName, 
								demoName, 
								boardNum);

		cout<<"OK.\n";

		pDrvr->getDriverVersionStr(infoStr);
		cout<<"Version: "<<infoStr<<endl;
		pDrvr->getDriverResourcesStr(infoStr);
		cout<<"Resources: "<<infoStr<<endl;
		pDrvr->getPCICapabiltiesStr(infoStr);
		cout<<"Link Info: "<<infoStr<<endl;


		//==================================================
		//  lscpcie2 Driver Info Test
		//==================================================
		cout<<"\n\n";
		cout<<"==========================================\n";
		cout<<"  Step #1: get PCIe resources\n";
		cout<<"==========================================\n";

		const PCIResourceInfo_t *pInfo;
		pDrvr->getPCIDriverInfo(&pInfo);
		cout<<"lscpcie Driver Info:\n";
		cout<<"numBARs = "<<pInfo->numBARs<<endl;
		for (ui = 0; ui < MAX_PCI_BARS; ui++)
		{
			if (ui >= pInfo->numBARs)
				cout<<"***";  // not initialized
			cout<<"\tBAR"<<ui<<":";
			cout<<"  nbar="<<pInfo->BAR[ui].nBAR;
			cout<<"  type="<<(int)pInfo->BAR[ui].type;
			cout<<"  size="<<pInfo->BAR[ui].size;
			cout<<"  Addr="<<hex<<pInfo->BAR[ui].physStartAddr<<dec;
			cout<<"  mapped="<<pInfo->BAR[ui].memMapped;
			cout<<"  flags="<<pInfo->BAR[ui].flags<<endl;
		}
		cout<<"hasInterrupt="<<pInfo->hasInterrupt<<endl;
		cout<<"intrVector="<<pInfo->intrVector<<endl;


		pDrvr->getPCIExtraInfoStr(infoStr);
		cout<<"\n\nDevice Driver Info:\n"<<infoStr<<endl;

		const ExtraResourceInfo_t *pExtra;
		pDrvr->getPciDriverExtraInfo(&pExtra);

		if (pExtra->DmaBufSize < FIFO_SIZE)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"            ERROR\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nDMA Common Buffer smaller than SFIF TX FIFO\n";
			cout<<"Exiting to avoid possible buffer overflows.\n";
			exit(-1);
		}

		if (pExtra->DmaPhyAddrLo % 4096)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"WARNING!  DMA Common Buffer Base is not 4kB aligned.\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nNo support to handle crossing 4kB address boundary\n";
			cout<<"Continue? (y/n): ";
			cin>>line;
			if (line[0] != 'y')
				exit(-1);
		}

		if (pExtra->DmaPhyAddrLo % 128)
		{
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"WARNING!  Base is not 128 aligned.\n";
			cout<<"Writes need to account for crossing 4k boundary\n";
			cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			cout<<"\nNo support to handle crossing 4kB address boundary\n";
			cout<<"Continue? (y/n): ";
			cin>>line;
			if (line[0] != 'y')
				exit(-1);
		}


		//==================================================
		//  lscpcie2 Driver DMA Buf access test
		//==================================================
		cout<<"\n\n";
		cout<<"=======================================================\n";
		cout<<"  Step #2: Test Read/Write DMA Sys buf from User Space\n";
		cout<<"=======================================================\n";

		cout<<"Testing accessing DMA Sys Buf from User space"<<endl;
		cout<<"Filling WriteBuf with 0xa5\n";
		for (i = 0; i < 1024; i++)
			WriteDmaBuf[i] = (i<<16) | i;
		cout<<"Calling driver to write DMA common buf\n";
		pDrvr->writeSysDmaBuf(WriteDmaBuf, 4096);

		cout<<"Clearing ReadBuf\n";
		memset(ReadDmaBuf, 0x00, 4096);
		cout<<"Calling driver to read DMA common buf\n";
		pDrvr->readSysDmaBuf(ReadDmaBuf, 4096);
		for (i = 0; i < 1024; i++)
		{
			if (ReadDmaBuf[i] != WriteDmaBuf[i])
				cout<<"ERROR! DMA Read != Write"<<endl;
		}
		cout<<"PASS\n";

		//==================================================
		// Access GPIO
		//==================================================
		cout<<"\n\n";
		cout<<"=======================================================\n";
		cout<<"  Step #3: Create GPIO device object and test\n";
		cout<<"=======================================================\n";


		pGPIO = new GPIO("GPIO",		 // a unique name for the IP module instance
						 memGPIO(0),	// its base address
						 pDrvr);		// driver interface to use for register access

		GPIOTest();


		//==================================================
		// Access SFIF 
		//==================================================
		cout<<"\n\n";
		cout<<"=======================================================\n";
		cout<<"  Step #4: Create SFIF device object and test\n";
		cout<<"=======================================================\n";

		pSFIF = new SFIF("SFIF",		 // a unique name for the IP module instance
						 memSFIF(0),	// its base address
						 pDrvr);		// driver interface to use for register access

		// Display all the SFIF register values for visual check
		pSFIF->showRegs();

		cout<<"\nWrite ICG_Count = 0xbeef\n";
		pSFIF->write32(0x08, 0xbeef);
		cout<<"Read TX_ICG: SFIF[8] = ";
		rd32_val = pSFIF->read32(0x08);
		cout<<hex<<rd32_val<<dec<<endl;
		if (rd32_val != 0xbeef)
		{
			cout<<"ERROR! Did not read back what was written!\n";
		}
		else
		{
			cout<<"PASS\n";
		}


		bool done = false;
		while (!done)
		{
			cout<<"\n\n";
			cout<<"==================================================\n";
			cout<<"            SFIF Menu\n";
			cout<<"==================================================\n";
			cout<<"R = (Read) MRd TLP Menu\n";
			cout<<"W = (Write) MWr TLP Menu\n";
			cout<<"T = Thruput Tests\n";
			cout<<"G = GPIO Module Tests\n";
			cout<<"S = SFIF Register Status\n";
			cout<<"P = PCIe Link Settings\n"; 
			cout<<"C = Change TLP Configuration Settings\n"; 
			cout<<"D = Display contents of DMA Common buf\n";
			cout<<"x = Exit\n";
			cout<<"-> ";
			cin>>line;

			switch (line[0])
			{
				case 'r':
				case 'R':
					readTlpMenu();
					break;

				case 'w':
				case 'W':
					writeTlpMenu();
					break;

				case 'g':
				case 'G':
					GPIOTest();
					break;

				case 't':
				case 'T':
					thruputTests();
					break;

				case 'd':
				case 'D':
					cout<<"\nSystem Common Buffer Contents Display";
					pDrvr->readSysDmaBuf(ReadDmaBuf, FIFO_SIZE);
					cout<<hex;
					for (i = 0; i < FIFO_SIZE/4; i++)
					{
						if ((i % 4) == 0)
							printf("\n%04X:  ", i*4);
						printf("%08X  ", ReadDmaBuf[i]);
					}
					cout<<dec;
					break;

				case 'c':
				case 'C':
					changeConfigSettings();
					break;

				case 'p':
				case 'P':
					//showPCIeSettings(pWBM);
					pDrvr->getPCIResourcesStr(infoStr);
					cout<<infoStr<<endl;
					pDrvr->getPCICapabiltiesStr(infoStr);
					cout<<infoStr<<endl;
					pDrvr->getPCIExtraInfoStr(infoStr);
					cout<<infoStr<<endl;
					break;

				case 's':
				case 'S':
					cout<<"\n\n";
					cout<<"============================\n";
					cout<<"  Read SFIF Registers\n";
					cout<<"============================\n";
					pSFIF->showRegs();

					cout<<"\nRoot Complex Initial Credits:"<<endl;
					cout<<"PD_CA (Wr): "<<((pGPIO->read32(0x24))>>16)<<endl;
					cout<<"NPH_CA(Rd): "<<((pGPIO->read32(0x24)) & 0xffff)<<endl;
					break;

				case 'x':
				case 'X':
					done = true;
					break;
			}
		}

	}
	catch (std::exception &e)
	{
		cout<<"\n!!!ERROR!!! Testing Halted with errors: "<<e.what()<<endl;
	}

	cout<<"\nEnd."<<endl;

	delete pSFIF;
	delete pGPIO;
	delete pDrvr;
}




/**
 * Run some test to verify access to the GPIO module.
 * This proves that PCIe control plane type access is working.
 */
void GPIOTest(void)
{
	int i;
	uint32_t wr_val, rd_val;
	char line[80];

	cout<<"\n\n";
	cout<<"==================================================\n";
	cout<<"        Test Basic GPIO IP Module Access\n";
	cout<<"==================================================\n";
	cout<<"\nLook For GPIO ID: ";   
	cout<<"Read32: GPIO[0] = ";
	uint32_t rd32_val = pGPIO->getID();
	cout<<hex<<rd32_val<<dec<<endl;
	if (rd32_val != 0x12043010)
	{
		cout<<"ERROR! GPIO module ID does not match 0x12043010!\n";
		cout<<"Aborting further GPIO tests."<<endl;
		cout<<"Continue? (y/n): ";
		cin>>line;
		if (line[0] != 'y')
			return;
	}

	cout<<"ScratchPad Wr/Rd/Verify: ";
	wr_val = rand();
	pGPIO->setScratchPad(wr_val);
	rd_val = pGPIO->getScratchPad();
	if (wr_val == rd_val)
		cout<<"PASS"<<endl;
	else
		cout<<"FAIL!!!"<<endl;


	// Light LED segments to visually show access is working
	cout<<"\nLED Segment Test: ";   
	for (i = 0; i < 16; i++)
	{
		pGPIO->setLED16Display((uint16_t)(1<<i));
		Sleep(100);
	}
}



/**
 * Change global configuration parameters.
 * Parameters that are used in the TLP headers.
 */
void changeConfigSettings(void)
{
	uint32_t RlxOrdBit = 1;
	uint32_t SnoopBit = 0;
	uint32_t PoisonedBit = 0;
	uint32_t TrafficClassBits = 0;
	uint32_t temp;
	char str[16];

	fflush(stdin);
	cout<<"\n==================================================\n";
	cout<<"       TLP Configuration Settings\n";
	cout<<"       Settings apply to all TLPs\n";
	cout<<"==================================================\n";
	cout<<"Relaxed Ordering(1/0=default): "<<RlxOrdBit<<"   ";
	str[0] = '\0';
	fgets(str, 8, stdin);
	if ((str[0] == '0') || (str[0] == '1'))
	{
		RlxOrdBit = str[0] - '0';  // 1 or 0
	}

	cout<<"No Cache Snoop(1/0=default): "<<SnoopBit<<"   ";
	str[0] = '\0';
	fgets(str, 8, stdin);
	if ((str[0] == '0') || (str[0] == '1'))
	{
		SnoopBit = str[0] - '0';  // 1 or 0
	}

	cout<<"Poisoned Payload(1/0=default): "<<PoisonedBit<<"   ";
	str[0] = '\0';
	fgets(str, 8, stdin);
	if ((str[0] == '0') || (str[0] == '1'))
	{
		PoisonedBit = str[0] - '0';	 // 1 or 0
	}

	cout<<"Traffic Class(7-0/0=default): "<<TrafficClassBits<<"   ";
	str[0] = '\0';
	fgets(str, 8, stdin);
	if (str[0] > ' ')
	{
		temp = atoi(str);
		if (temp <= 7)
			TrafficClassBits = temp;
	}

	pSFIF->cfgTLP(RlxOrdBit,  SnoopBit,  PoisonedBit,  TrafficClassBits );
}



/**
 * Display the SFIF TLP counter registers.
 * Usually called after a run to see what has transpired - number of TLPs
 * sent, number CplD recvd, elapsed time, etc.
 * @param sfifCnts the SFIF structure defining the various register fields
 */
void    showSFIFCounters(SFIF::SFIFCntrs_t &sfifCnts)
{
	cout<<"ELAPSED_CNT: SFIF[0x14] = "<<dec<<sfifCnts.ElapsedTime <<endl;
	cout<<"TX_TLP_CNT: SFIF[0x18] = "<<sfifCnts.TxTLPCnt<<endl;
	cout<<"RX_TLP_CNT: SFIF[0x1C] = "<<sfifCnts.RxTLPCnt<<endl;
	cout<<"CREDIT_WR_PAUSE_CNT: SFIF[0x20] = "<<sfifCnts.WrWaitTime<<endl;
	cout<<"LAST_CPLD_TS: SFIF[0x24] = "<<sfifCnts.LastCplDTime<<endl;
	cout<<"CREDIT_RD_PAUSE_CNT: SFIF[0x28] = "<<sfifCnts.RdWaitTime<<endl;
}


/**
 * Generate MWr TLPs and load into TX FIFO and send to PC.
 * The user can select the payload size and the number of packets to send
 * per cycle.  The cycle parameter allows the TX FIFO contents to be re-played
 * out a number of times.  The ICG (Inter Cycle Gap) is used to insert a 
 * delay of that many 125 MHz clk periods between cycles.
 *
 */
void    writeTlpMenu(void)
{
	uint32_t i;
	uint32_t cycles;
	uint32_t icg;
	uint32_t n, nPkts;
	uint32_t payloadSize;
	uint32_t numDWs;
	uint32_t wrAddr;
	double   pauseRatio;
	double thruput;
	SFIF::SFIFCntrs_t sfifCnts;
	char line[80];


	while (1)
	{
		wrAddr = 0;	   // Starting point in Sys Common buffer (offset)
		cout<<"\n\n";
		cout<<"==================================================\n";
		cout<<"  Write a TLP(s) into system memory\n";
		cout<<"==================================================\n";
		cout<<"Payload Size(4,16,32,64,128): ";
		cin>>payloadSize;
		switch (payloadSize)
		{
			case 4:
			case 16:
			case 32:
			case 64:
			case 128:
				numDWs = payloadSize / 4;
				break;
			default:
				payloadSize = 4;
				numDWs = 1;
		}

		cout<<"Tx pkts: ";
		cin>>nPkts;
		cout<<"Tx cycles: ";
		cin>>cycles;
		cout<<"ICG: ";
		cin>>icg;

		if (cycles < 1)
			cycles = 1;
		else if (cycles > 0xffff)
			cycles = 0xffff;


		if (((numDWs + 4) * 4 * nPkts) > FIFO_SIZE)
		{
			nPkts = FIFO_SIZE / ((numDWs + 4) * 4);
			cout<<"Too large for TX FIFO.  Set to: "<<nPkts<<endl;
		}


		// Clear DMA common buffer done by SFIF class


		pSFIF->setupSFIF(WRITE_CYC_MODE,  // runMode
						 MWR_TLPS,		  // trafficMode
						 cycles, 
						 icg, 
						 0,				// rdTLPSize
						 payloadSize,  // wrTLPSize
						 0,				// numRdTLPs
						 nPkts);   // numWrTLPs


		pSFIF->startSFIF(false);  // not looping, just run the set many cycles

		Sleep(1000);  // Wait 1 second while SFIF runs

		pSFIF->stopSFIF();


		pSFIF->getCntrs(sfifCnts);
		showSFIFCounters(sfifCnts);


		pauseRatio = (double)sfifCnts.WrWaitTime / (double)sfifCnts.ElapsedTime;
		cout<<"Run-time waiting for MWr credits: "<<(pauseRatio * 100.0)<<"%\n";

		if (sfifCnts.ElapsedTime == 0xffffffff)
		{
			cout<<"ERROR!  Counters saturated.  Can't compute thruput.\n";
		}
		else
		{
			thruput = ((payloadSize * sfifCnts.TxTLPCnt) / (8E-9 * sfifCnts.ElapsedTime)) / 1E6;

			cout<<"Write: "<<(payloadSize * sfifCnts.TxTLPCnt)<<" bytes   Throughput: "<<thruput<<" MB/s\n";
		}

		cout<<"Clearing ReadBuf\n";
		memset(ReadDmaBuf, 0x00, sizeof(ReadDmaBuf));
		cout<<"Calling driver to read out "<<numDWs * nPkts<<" words\n";
		pDrvr->readSysDmaBuf(ReadDmaBuf, payloadSize * nPkts);

		cout<<"Verifying contents transfered to User Space...\n";
		if (pSFIF->verifyMWrXfer(ReadDmaBuf, payloadSize * nPkts))
			cout<<"PASSED!"<<endl;


		cout<<"\nDisplay Rcvd data(y/n)? ";
		cin>>line;
		if (line[0] == 'y')
		{
			n = numDWs * nPkts;
			if (n > sizeof(ReadDmaBuf) / 4)
				n =  sizeof(ReadDmaBuf) / 4;
			cout<<hex;
			for (i = 0; i < n; i++)
			{
				if ((i % 4) == 0)
					printf("\n%04X:  ", i*4);
				printf("%08X  ", ReadDmaBuf[i]);
			}
			cout<<dec;
		}

		cout<<"\n\nAgain(y/n)? ";
		cin>>line;
		if (line[0] != 'y')
			break;
	}

}




/**
 * Generate MRd TLPs and load into TX FIFO and send to PC.
 * The user can select the payload size and the number of packets to send
 * per cycle.  The cycle parameter allows the TX FIFO contents to be re-played
 * out a number of times.  The ICG (Inter Cycle Gap) is used to insert a 
 * delay of that many 125 MHz clk periods between cycles.
 * The received Completions with Data are captured in the RX FIFO and
 * processed to verify all read requests were satisfied.
 *
 */
void    readTlpMenu(void)
{
	uint32_t cycles;
	uint32_t icg;
	uint32_t nPkts;
	uint32_t readReqSize;
	uint32_t numDWs;
	uint64_t totalBytes;
	char line[80];
	double   pauseRatio;
	double   thruput;
	string outs;

	SFIF::SFIFCntrs_t sfifCnts;


	while (1)
	{

		cout<<"\n\n";
		cout<<"==================================================\n";
		cout<<"  Read a TLP(s) from system memory\n";
		cout<<"==================================================\n";
		cout<<"Read Request Size(4,16,32,64,128,256,512): ";
		cin>>readReqSize;
		switch (readReqSize)
		{
			case 4:
			case 16:
			case 32:
			case 64:
			case 128:
			case 256:
			case 512:
				numDWs = readReqSize / 4;
				break;
			default:
				readReqSize = 4;
				numDWs = 1;
		}

		cout<<"Number MRd TLPs to send: ";
		cin>>nPkts;
		if (nPkts > 32)
		{
			cout<<"NOTE: Max 32 outstanding TLPs."<<endl;
			nPkts = 32;
		}
		cout<<"# Cycles of MRd pkts: ";
		cin>>cycles;
		if (cycles >= 0x10000)
		{
			cout<<"NOTE: Max 64k cycles."<<endl;
			cycles = 0xffff;
		}
		else if (cycles < 1)
			cycles = 1;

		cout<<"ICG: ";
		cin>>icg;

		// Put pattern to read in DMA common buffer done by SFIF class

		pSFIF->setupSFIF(READ_CYC_MODE,	 // runMode
						 MRD_TLPS,		  // trafficMode
						 cycles, 
						 icg, 
						 readReqSize,  // rdTLPSize
						 0,			   // wrTLPSize
						 nPkts,				// numRdTLPs
						 0);   // numWrTLPs


		pSFIF->startSFIF(false);  // not looping, just run the set many cycles

		Sleep(1000);

		pSFIF->stopSFIF();


		pSFIF->getCntrs(sfifCnts);
		showSFIFCounters(sfifCnts);


		//pauseRatio = (double)sfifCnts.RdWaitTime / (double)sfifCnts.ElapsedTime;
		pauseRatio = (double)sfifCnts.RdWaitTime / (double)sfifCnts.LastCplDTime;
		cout<<"Run-time waiting for MRd credits: "<<(pauseRatio * 100.0)<<"%\n";

		if (pSFIF->getSFIFParseRxFIFO(outs))
		{
			cout<<outs;
		}
		else
		{
			cout<<"\n\nThruput estimate based on read request size, Rx TLP count and Last CplD time\n";
			if (readReqSize < RCB)
				totalBytes = (uint64_t)readReqSize * (uint64_t)sfifCnts.RxTLPCnt;
			else
				totalBytes = (uint64_t)RCB * (uint64_t)sfifCnts.RxTLPCnt;
			thruput = (totalBytes / (8E-9 * sfifCnts.LastCplDTime)) / 1E6;

			cout<<"Recvd: "<<dec<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.LastCplDTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";
		}


		cout<<"\n\nAgain(y/n)? ";
		cin>>line;
		if (line[0] != 'y')
			break;
	}

}




/**
 * Throughput Tests.
 * Run a write 
 * Run a Read
 * Run a Read & Write
 * Run a Read & Write and system access
 */
void    thruputTests(void)
{

	uint32_t i;
	uint32_t cycles;
	uint32_t icg;
	uint32_t nPkts;
	uint32_t pktRdRatio, pktWrRatio;
	uint32_t numDWs, rdNumDWs, wrNumDWs;
	uint32_t payloadSize;
	uint32_t readReqSize;
	uint32_t msecRunTime;
	uint8_t mode;
	uint64_t totalBytes;
	double thruput;
	double   pauseRatio;
	SFIF::SFIFCntrs_t sfifCnts;
	char line[80];



	cout<<"\n==================================================\n";
	cout<<"          Thruput Tests\n";
	cout<<"==================================================\n";
	cout<<"Test Duration (msec): ";
	cin>>msecRunTime;
	if (msecRunTime > 30000)
	{
		cout<<"WARNING! Duration limitted to 30 secs or counters saturate.\n";
		msecRunTime = 30000;
	}
	else if (msecRunTime <= 0)
	{
		msecRunTime = 1;
	}

	cout<<"Mode (RWA=8|RW=4|Rd=2|Wr=1): ";
	cin>>line;
	mode = atoi(line);

	if ((mode < 1) || (mode > 15))
	{
		cout<<"Invalid selection.\n";
		return;
	}

	if (!(mode & 1))
		goto READTEST;

	///////////////////////////////////////////////////////////////////
	//                    WRITE TEST
	///////////////////////////////////////////////////////////////////
	cout<<"\n\n";
	cout<<"==================================================\n";
	cout<<"             Write Rate Testing\n";
	cout<<"==================================================\n";
	cout<<"Continuous bursts of (100) 128 byte MWr TLPs\n";
	payloadSize = 128;
	numDWs = payloadSize / 4;
	nPkts = 100;   // 12800 bytes of addressing
	icg = 1;
	cycles = 1;	 // actually not used cause loop forever, but load with something sane


	cout<<"Triggering RUN...looping...\n";

	pSFIF->setupSFIF(THRUPUT_MODE,	// runMode
					 MWR_TLPS,		  // trafficMode
					 cycles, 
					 icg, 
					 0,				// rdTLPSize
					 payloadSize,  // wrTLPSize
					 0,				// numRdTLPs
					 nPkts);   // numWrTLPs


	pSFIF->startSFIF(true);	 // loop forever

	Sleep(msecRunTime);

	pSFIF->stopSFIF();


	pSFIF->getCntrs(sfifCnts);
	showSFIFCounters(sfifCnts);


	pauseRatio = (double)(sfifCnts.WrWaitTime) / (double)(sfifCnts.ElapsedTime);
	cout<<"Run-time waiting for MWr credits: "<<(pauseRatio * 100.0)<<"%\n";


	if ((sfifCnts.ElapsedTime == 0xffffffff) || (sfifCnts.TxTLPCnt == 0xffffffff))
	{
		cout<<"ERROR!  Counters saturated.  Can't compute thruput.\n";
	}
	else
	{
		totalBytes = (uint64_t)payloadSize * (uint64_t)sfifCnts.TxTLPCnt;
		thruput = ((totalBytes) / (8E-9 * sfifCnts.ElapsedTime)) / 1E6;

		cout<<"Wrote: "<<(totalBytes)<<" bytes  in "<<(8E-9 * sfifCnts.ElapsedTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";
	}


	READTEST:
	if (!(mode & 2))
		goto READWRITE_TEST;

	///////////////////////////////////////////////////////////////////
	//                    READ TEST
	///////////////////////////////////////////////////////////////////
	cout<<"\n\n";
	cout<<"==================================================\n";
	cout<<"             Read Rate Testing\n";
	cout<<"==================================================\n";
	cout<<"Continuous burst of 32 Read Requests of 512 bytes\n";
	readReqSize = 512;
	nPkts = 32;	 // need to limit so that we keep reads requests within the system DMA buffer
	icg = 1;   // we could set this to some value to simulate processing time for returned pkts
			   // or just blast it and hope flow control keeps everything in check
	cycles = 1;	 // actually not used cause loop forever, but load with something sane

	cout<<"Triggering RUN...looping...\n";

	pSFIF->setupSFIF(THRUPUT_MODE,	  // runMode
					 MRD_TLPS,		  // trafficMode
					 cycles, 
					 icg, 
					 readReqSize,	  // rdTLPSize
					 0,				  // wrTLPSize
					 nPkts,			  // numRdTLPs
					 0);			  // numWrTLPs


	pSFIF->startSFIF(true);	 // loop forever

	Sleep(msecRunTime);

	pSFIF->stopSFIF();


	pSFIF->getCntrs(sfifCnts);
	showSFIFCounters(sfifCnts);


	pauseRatio = (double)(sfifCnts.RdWaitTime) / (double)(sfifCnts.LastCplDTime);
	cout<<"Run-time waiting for MRd credits: "<<(pauseRatio * 100.0)<<"%\n";


	if ((sfifCnts.ElapsedTime == 0xffffffff) || (sfifCnts.RxTLPCnt == 0xffffffff) || (sfifCnts.LastCplDTime == 0xffffffff))
	{
		cout<<"ERROR!  Counters saturated.  Can't compute thruput.\n";
	}
	else if (sfifCnts.LastCplDTime == 0xffffffff)
	{
		cout<<"ERROR!  No timestamp for CplDs!  Can't compute thruput.\n";
	}
	else
	{
		// assume all CplD have come in and ReadReqSize = 8 * RCB
		totalBytes = (uint64_t)RCB * (uint64_t)sfifCnts.RxTLPCnt;
		thruput = ((totalBytes) / (8E-9 * sfifCnts.LastCplDTime)) / 1E6;

		cout<<"Recvd: "<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.LastCplDTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";
	}


	READWRITE_TEST:

	if (!(mode & 4))
		goto READWRITEACCESS_TEST;


	///////////////////////////////////////////////////////////////////
	//                  READ/WRITE TEST
	///////////////////////////////////////////////////////////////////
	cout<<"\n\n";
	cout<<"==================================================\n";
	cout<<"             Read+Write Rate Testing\n";
	cout<<"==================================================\n";
	cout<<"Read Request Size: 512 bytes\n";
	readReqSize = 512;
	rdNumDWs = readReqSize / 4;
	cout<<"Write TLPs Size: 128 bytes\n";
	payloadSize = 128;
	wrNumDWs = payloadSize / 4;

	cout<<"nPkts (1-96): ";
	cin>>nPkts;
	cout<<"MRd ratio (1/n): ";
	cin>>pktRdRatio;
	cout<<"MWr ratio (1/n): ";
	cin>>pktWrRatio;
	cout<<"ICG: ";
	cin>>icg;
	cout<<"Cycles: ";
	cin>>cycles;

	if (pktRdRatio == 0)
		pktRdRatio = 1;
	if (pktWrRatio == 0)
		pktWrRatio = 1;

	cout<<"Load MRd & MWr TLPs into TX_FIFO\n";
	if (cycles == 0)
	{
		pSFIF->setupSFIF(THRUPUT_MODE,	  // runMode
						 MRD_MWR_TLPS,		  // trafficMode 
						 cycles, 
						 icg, 
						 readReqSize,	  // rdTLPSize
						 payloadSize,				// wrTLPSize
						 nPkts,			  // numRdTLPs
						 nPkts,				// numWrTLPs
						 pktRdRatio,
						 pktWrRatio);
		pSFIF->startSFIF(true);	 // loop forever
	}
	else
	{
		pSFIF->setupSFIF(WRITE_CYC_MODE,	// runMode
						 MRD_MWR_TLPS,		  // trafficMode
						 cycles, 
						 icg, 
						 readReqSize,	  // rdTLPSize
						 payloadSize,				// wrTLPSize
						 nPkts,			  // numRdTLPs
						 nPkts,				// numWrTLPs
						 pktRdRatio,
						 pktWrRatio);
		pSFIF->startSFIF(false);  // run cycles

	}

	Sleep(msecRunTime);

	pSFIF->stopSFIF();


	pSFIF->getCntrs(sfifCnts);
	showSFIFCounters(sfifCnts);




	if (cycles == 0)
	{

		if ((sfifCnts.ElapsedTime == 0xffffffff) || (sfifCnts.TxTLPCnt == 0xffffffff) || (sfifCnts.RxTLPCnt == 0xffffffff))
		{
			cout<<"ERROR!  Counters saturated.  Can't compute thruput.\n";
		}
		else
		{
			pauseRatio = (double)(sfifCnts.WrWaitTime) / (double)(sfifCnts.ElapsedTime);
			cout<<"Run-time waiting for MWr credits: "<<(pauseRatio * 100.0)<<"%\n";

			pauseRatio = (double)(sfifCnts.RdWaitTime) / (double)(sfifCnts.LastCplDTime);
			cout<<"Run-time waiting for MRd credits: "<<(pauseRatio * 100.0)<<"%\n";

			cout<<"READ Thruput: ";
			// assume all CplD have come in and ReadReqSize = 8 * RCB 
			totalBytes = (uint64_t)RCB * (uint64_t)sfifCnts.RxTLPCnt;
			thruput = ((totalBytes) / (8E-9 * sfifCnts.LastCplDTime)) / 1E6;

			cout<<"\n   Recvd: "<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.LastCplDTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";

			// Factor in the number of MRd TLPs sent and remove this from the total Tx TLP count
			cout<<"WRITE Thruput: ";
			totalBytes = (uint64_t)payloadSize * ((uint64_t)sfifCnts.TxTLPCnt * pktRdRatio / (pktRdRatio + pktWrRatio));
			thruput = ((totalBytes) / (8E-9 * sfifCnts.ElapsedTime)) / 1E6;

			cout<<"\n   Wrote: "<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.ElapsedTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";
		}
	}
	else
	{
		cout<<"\nSurvived the Rd+Wr test."<<endl;
	}


	READWRITEACCESS_TEST:
	if (!(mode & 8))
		return;

	///////////////////////////////////////////////////////////////////
	//                  READ/WRITE/ACCESS TEST
	///////////////////////////////////////////////////////////////////
	cout<<"\n\n";
	cout<<"==================================================\n";
	cout<<"       Read+Write+GPIO Access Rate Testing\n";
	cout<<"==================================================\n";
	cout<<"Read Request Size: 512 bytes\n";
	readReqSize = 512;
	rdNumDWs = readReqSize / 4;
	cout<<"Write TLPs Size: 128 bytes\n";
	payloadSize = 128;
	wrNumDWs = payloadSize / 4;
	nPkts = 16;
	icg = 1;
	cycles = 1;	 // actually not used cause loop forever, but load with something sane
	pktRdRatio = 4;
	pktWrRatio = 1;

	cout<<"Load MRd & MWr TLPs into TX_FIFO\n";
	pSFIF->setupSFIF(THRUPUT_MODE,	  // runMode
					 MRD_MWR_TLPS,	  // trafficMode - both reads and writes
					 cycles, 
					 icg, 
					 readReqSize,	  // rdTLPSize
					 payloadSize,	  // wrTLPSize
					 nPkts,		  // numRdTLPs
					 nPkts,			  // numWrTLPs
					 pktRdRatio,
					 pktWrRatio);

	pSFIF->startSFIF(true);	 // loop forever




	i = msecRunTime / 10;	// fudge factor to get it to run for N msec

	int led = 1;
	int err = 0;
	while (i)
	{
		// do a burst of 1000 GPIO accesses to PCIe board
		for (uint32_t jj = 0; jj < 100; jj++)
		{
			// write Scratch Pad
			pGPIO->setScratchPad(jj);
			// Read GPIO ID register and verify
			if (pGPIO->getID() != 0x53030100)
				++err;
			// read scratch pad and verify
			if (pGPIO->getScratchPad() != jj)
				++err;
		}

		if ((i % 10) == 0)	// every 100msec show some activity
		{
			pGPIO->setLED16Display((uint16_t)led);
			led = (led<<1) & 0xffff;
			if (led == 0)
				led = 1;
		}

		Sleep(1);
		--i;
	}

	pSFIF->stopSFIF();


	pSFIF->getCntrs(sfifCnts);
	showSFIFCounters(sfifCnts);


	if ((sfifCnts.ElapsedTime == 0xffffffff) || (sfifCnts.TxTLPCnt == 0xffffffff) || (sfifCnts.RxTLPCnt == 0xffffffff))
	{
		cout<<"ERROR!  Counters saturated.  Can't compute thruput.\n";
	}
	else
	{
		pauseRatio = (double)(sfifCnts.WrWaitTime) / (double)(sfifCnts.ElapsedTime);
		cout<<"Run-time waiting for MWr credits: "<<(pauseRatio * 100.0)<<"%\n";

		pauseRatio = (double)(sfifCnts.RdWaitTime) / (double)(sfifCnts.LastCplDTime);
		cout<<"Run-time waiting for MRd credits: "<<(pauseRatio * 100.0)<<"%\n";

		cout<<"READ Thruput: ";
		// assume all CplD have come in and ReadReqSize = 8 * RCB 
		totalBytes = (uint64_t)RCB * (uint64_t)sfifCnts.RxTLPCnt;
		thruput = ((totalBytes) / (8E-9 * sfifCnts.LastCplDTime)) / 1E6;

		cout<<"\n   Recvd: "<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.LastCplDTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";

		// Factor in the number of MRd TLPs sent and remove this from the total Tx TLP count
		cout<<"WRITE Thruput: ";
		totalBytes = (uint64_t)payloadSize * ((uint64_t)sfifCnts.TxTLPCnt * pktRdRatio / (pktRdRatio + pktWrRatio));
		thruput = ((totalBytes) / (8E-9 * sfifCnts.ElapsedTime)) / 1E6;

		cout<<"\n   Wrote: "<<totalBytes<<" bytes  in "<<(8E-9 * sfifCnts.ElapsedTime)<<" secs   Throughput: "<<thruput<<" MB/s\n";
	}

}


