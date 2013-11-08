/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/**  @file PCIe_Menu.cpp */ 
/**
 * \page Menu_page PCIe Basic Demo Menu Program Description
 *
 * The PCIe Basic Demo Menu Program (see PCIe_menu.cpp) illustrates
 * basic read/write operations over
 * the PCIe bus to registers in the demo IP of the Lattice FPGA.
 * This provides the basis for a Control Plane type application over
 * the PCIe bus.
 * The demo also displays the PCI registers and PCIe Capabilities 
 * structures that are in the Core and configured by the Root Complex.
 * <p>
 * The Menu provides a command line interface to inter-actively
 * exercise the GPIO registers and EBR memory.
 * <p>
 * The flow of execution shows the following:
 * <ol>
 * <li> Instantiate the lscpcie2 driver interface to open access to the
 * device driver.
 * <li> Use the driver methods to display info about the driver and the
 * PCIe config registers.
 * <li> Create a GPIO object that then provides all the methods for 
 * exercising the LEDs, counter logic and reading the DIP switches.
 * <li> Create an EBR object that then provides all the methods for
 * testing reading/writing access to the EBR memory.
 * <li> Create a BAR object that then provides read/write access to any
 * memory location; used for peeking and poking; diagnostics.
 * <li> Launch the menu for inter-active selection of tests to run.
 * <li> When done, clean-up and return.
 * </ol>
 *
 * The menu provides the same functionality as the GUI, but may be easier to
 * follow from the view point of a software developer looking to see how
 * to open the driver and use the classes and methods provided in the PCIeAPI
 * library to access IP in the FPGA.
 *
 * <p>
 * The specific Evaluation Board to open and access is provided via environment
 * variables.  The following environment variables are typically set via a config
 * batch file or with the PCIeScan utility prior to running this application:
 * <ul>
 * <li><B>LSC_PCIE_BOARD</B> = PCI Vendor and Device string of the board to open
 * <li><B>LSC_PCIE_IP_ID</B> = "subsys_53031204"  subsystem ID = demo IP ID
 * <li><B>LSC_PCIE_INSTANCE</B> = instance of board to open, typically 1, the first
 * </ul>
 *
 * 
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "PCIeAPI.h"         // the APIs
#include "LSCPCIe2_IF.h"     // the driver interface
#include "GPIO.h"            // the GPIO IP module
#include "Mem.h"            // the General Memory IP module
#include "MiscUtils.h"
#include "MemFormat.h"
#include "../MemMap.h"       // IP module base addresses



//////////////////////////////////////////////
// Required to use Lattice PCIe methods
//////////////////////////////////////////////
using namespace LatticeSemi_PCIe;


typedef uint32_t Addr_t;
typedef uint8_t  Data_t;



/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*               GLOBAL VARIABLES                                           */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

LSCPCIe2_IF *pDrvr;
Mem        *pEBR;
Mem        *pBAR0;
Mem        *pBAR1;
GPIO        *pGPIO;


/* Function Prototypes */
void menu(void);
static char* strip(char *s);
static char getCmd(char *line);
void    showHelp(void);





//////////////////////////////////////////////
//             Main Entry
//////////////////////////////////////////////

/**
 * PCIe Basic Demo Entry Point.
 * Create the driver interface object, display driver info,
 * create the IP device objects (GPIO and EBR) and then
 * launch the menu().
 * Environment variables must be set to specify the board
 * to open.
 */
int main(int argc, char *argv[])
{
	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
	uint32_t boardNum;
	string infoStr;
	uint32_t ui;



	cout<<"\n\n========================================================\n";
	cout<<"           Lattice PCIe Basic Demo Menu\n";
	cout<<"                 ver2.0.1\n";
	cout<<"========================================================\n";


	cout<<"\n\n======================================\n";
	cout<<"    Instantiate the PCIeAPI DLL class\n";
	cout<<    "======================================\n";
	PCIeAPI theDLL;
	cout<<"Dll version info: "<<theDLL.getVersionStr()<<endl;

	// Setup so lots of diag output occurs from tests
	theDLL.setRunTimeCtrl(PCIeAPI::VERBOSE);

	// Get the environment variables needed to open the desired board
	// Pre-load with defaults of an ECP2M SolBrd and the basic demo IP bitstream
	strcpy(boardName, "ven_1204&dev_e250");
	strcpy(demoName, "subsys_30101204");
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
		//  lscpcie2 Driver Info
		//==================================================
		cout<<"\n\n";
		cout<<"==========================================\n";
		cout<<"  PCIe Driver & Link Resources\n";
		cout<<"==========================================\n";

		const PCIResourceInfo_t *pInfo;
		pDrvr->getPCIDriverInfo(&pInfo);
		cout<<"lscpcie2 Driver Info:\n";
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

		//==================================================
		// Access GPIO
		//==================================================
		cout<<"\n\n";
		cout<<"=======================================================\n";
		cout<<"  Create access to GPIO IP module\n";
		cout<<"=======================================================\n";


		pGPIO = new GPIO("GPIO",		 // a unique name for the IP module instance
				 GPIOreg(0),	// its base address
				 pDrvr);		// driver interface to use for register access

		cout<<"\n\n======================================\n";
		cout<<    "  Create access to the 16KB EBR\n";
		cout<<    "======================================\n";

		pEBR = new Mem("EBR16", EBR_SIZE, EBRreg(0), pDrvr); 

		// This gives us access to all the memory locations within BAR0
		// Its for diagnostic purposes
		pBAR0 = new Mem("BAR0", BAR0_SIZE, BAR0reg(0), pDrvr); 

		// This gives us access to all the memory locations within BAR1
		// Its for diagnostic purposes
		pBAR1 = new Mem("BAR1", BAR1_SIZE, BAR1reg(0), pDrvr); 

		// Do the tests
		menu();

	}
	catch (std::exception &e)
	{
		cout<<"RUNTIME ERROR!!! "<<e.what()<<endl;
	}

	cout<<"Done."<<endl;

	delete pEBR;
	delete pBAR0;
	delete pBAR1;
	delete pGPIO;
	delete pDrvr;
	return(0);
}




/**
 * Strip whitespace and control chars from beginning of string, returning first
 * printable character.
 */
static char* strip(char *s)
{
	while (((*s <= 0x21) || ((unsigned char)*s > 0x7f)) && (*s != '\0'))
		++s;
	return(s);
}


/**
 * Return a character to be used as a menu choice.
 */
static char getCmd(char *line)
{
	char choice;

	cin.getline(line, 80);
	choice = *(strip(line));
	return(choice);
}




/**
 * PCIe Basic Demo user interactive Menu.
 * Provides the following operations:
 *
 * <ul>
 * <li>c - PCI Config Registers
 * <li>e - PCIe Extended Capabilities Registers
 * <li>l - LED test
 * <li>d - DIP switch setting
 * <li>m[cf] - EBR memory test
 * <li>i <file> - Input file to EBR memory
 * <li>o <file> - Output EBR memory to file
 * <li>r[bsl] [<addr> [count]]
 * <li>w[bsl] [<addr> <data_1>...<data_n>]
 * <li>v - version and driver info
 * <li>t[1-5] [num] - run a debug test num times
 * <li>q,x - exit
 * </ul>
 */
void menu(void)
{
	int     argCnt;
	bool    done;
	char    line[256];
	char    lastCmd[256];
	uint8_t buf[256];
	uint8_t d;
	size_t	i;
	string  s;
	char	fName[128];
	size_t  n;
	char    c;
	int	barNum = 0;


	MemFormat BAR0Access(pBAR0, BAR0_SIZE);   // max range to read
	MemFormat BAR1Access(pBAR1, BAR1_SIZE);   // max range to read

	showHelp();

	done = false;
	while (!done)
	{
		// show the prompt
		cout<<"\n=> ";
		argCnt = 0;

		s.clear();

		switch (getCmd(line))
		{
			case 'r':	// read:  r [addr [count]]
			case 'R':
                               if (barNum == 0)
                               {
                                   if (BAR0Access.doCmd(line, s))
                                           cout<<s<<endl;
                               }
                               else if (barNum == 1)
                               {
                                   if (BAR1Access.doCmd(line, s))
                                           cout<<s<<endl;
                               }
				break;

			case 'w':  // write:  w <address> [data_1]...[data_n]
			case 'W':
                                if (barNum == 0)
                                {
                                    BAR0Access.doCmd(line, s);
                                }
                                else if (barNum == 1)
                                {
                                    BAR1Access.doCmd(line, s);
                                }
				break;


                        case 'b':  // BAR selection
                        case 'B':
                                if (((line[0] == 'B') || (line[0] == 'b')) && (line[1] == '1'))
                                    barNum = 1;
                                else if (((line[0] == 'B') || (line[0] == 'b')) && (line[1] == '0'))
                                    barNum = 0;  // default
                                cout<<"Accessing BAR"<<barNum<<endl;
                                break;
 


			case 'c':  // Show PCI Config registers
			case 'C':
				cout<<"PCI Config Registers (00-3f):\n";
				pDrvr->getPCIConfigRegs(buf);
				MemFormat::formatBlockOfBytes(s, 0x00, 0x40, buf);
				cout<<s<<endl;
				s.clear();  // clear the string
				pDrvr->getPCIResourcesStr(s);
				cout<<s<<endl;

				cout<<"\n\nPCI Capabilities Registers (40-ff):\n";
				s.clear();  // clear the string
				MemFormat::formatBlockOfBytes(s, 0x40, 0xc0, &buf[0x40]);
				cout<<s<<endl;
				s.clear();  // clear the string
				pDrvr->getPCICapabiltiesStr(s);
				cout<<s<<endl;
				break;

			case 'e':	// Show PCI Express registers
			case 'E':
				cout<<"PCI Express Extended Capabilities Registers (100-fff)\n";
				cout<<"Not accessable yet.\n";
				break;

			case 'm':  // Memory test
			case 'M':
				if ((line[0] == 'm') && (line[1] == 'c'))
				{
				    if (pEBR->clear())
					cout<<"EBR Memory cleared to 0x00"<<endl;
				    else
					cout<<"ERROR! in Memory clear!"<<endl;
				    break;
				}
				else if ((line[0] == 'm') && (line[1] == 'f'))
				{
					n = sscanf(&line[3], "%x", &i);
					if (n == 1)
					{
					    if (pEBR->fill((uint8_t)i))
						cout<<"EBR Memory filled with 0x"<<hex<<i<<endl;
					    else
						cout<<"ERROR!  Memory Fill failed!"<<endl;
					}
					break;
				}
				else
				{
				    if (pEBR->test())
					cout<<"PASS"<<endl;
				    else
					cout<<"FAILED"<<endl;
				}
				break;


			case 'i':  // Input file into EBR Memory
			case 'I':
				i = sscanf(line,  "%c %s", &c, fName);
				if (i != 2)
					break;
				if (pEBR->loadFromFile(fName))
				    cout<<"OK"<<endl;
				else
				    cout<<"FAILED"<<endl;
				break;

			case 'o':  // write EBR memory to a File
			case 'O':
				i = sscanf(line,  "%c %s", &c, fName);
				if (i != 2)
					break;
				if (pEBR->saveToFile(fName))
				    cout<<"OK"<<endl;
				else
				    cout<<"FAILED"<<endl;
				break;


			case 'l':  // LED test - 
			case 'L':
				pGPIO->LED16DisplayTest();
				break;


			case 'd':  // DIP Switch Test 
			case 'D':
				cout<<"=== DIP Switch Test ===\n";
				cout<<"Reading switch setting 10 times:\n";
				for (i = 0; i < 10; i++)
				{
					d = pGPIO->getDIPsw();
					cout<<"["<<dec<<i<<"] read: 0x"<<hex<<(unsigned int)d<<endl;
					Sleep(1000);
				}
				break;

			case 'n':  // Interrupt Test
			case 'N':
				cout<<"PCI Express Interrupt Test\n";
				cout<<"NOT IMPLEMENTED YET\n";
				break;

			case 'v':  // Version and Driver info
			case 'V':
				cout<<"PCI Driver Version:\n";
				if (pDrvr->getDriverVersionStr(s))
					cout<<s<<endl;
				cout<<"PCI Driver Information:\n";
				s.clear();
				if (pDrvr->getDriverResourcesStr(s))
					cout<<s<<endl;
				break;

			case 't':
			case 'T':
				if (line[1] == '1')
				{
					uint32_t id_reg;
					int t1_times;
					int tt = 1;
					cout<<"T1 - read Demo ID test"<<endl;
					i = sscanf(&line[2],  "%d", &t1_times);
					if (i != 1)
						t1_times = 100000;

					do
					{
						cout<<dec<<tt<<endl;
						++tt;
						id_reg = pGPIO->getID();
						if (t1_times > 0)
							--t1_times;
					} while((id_reg == 0x53030100) && (t1_times != 0));
					if (t1_times)
						cout<<"FAILURE: read 0x"<<hex<<id_reg<<" not 0x53030100"<<endl;
				}
				else if (line[1] == '2')
				{
					uint32_t wr_val = 0x11223344;
					int t2_times;
					int tt = 1;
					cout<<"T2 - write test"<<endl;
					i = sscanf(&line[2],  "%d", &t2_times);
					if (i != 1)
						t2_times = 100000;

					do
					{
						pGPIO->setScratchPad(wr_val);
						cout<<dec<<tt<<": wr="<<hex<<wr_val<<endl;
						++tt;
						if (t2_times > 0)
							--t2_times;
					} while (t2_times != 0);
				}
				else if (line[1] == '3')
				{
					uint32_t wr_val;
					uint32_t rd_val;
					int t3_times;
					int tt = 1;
					cout<<"T3 - write/read test"<<endl;
					i = sscanf(&line[2],  "%d", &t3_times);
					if (i != 1)
						t3_times = 100000;

					do
					{
						wr_val = ((rand())<<16) | rand();
						pGPIO->setScratchPad(wr_val);
						cout<<dec<<tt<<": wr="<<hex<<wr_val<<endl;
						++tt;
						rd_val = pGPIO->getScratchPad();
						if (t3_times > 0)
							--t3_times;
					} while ((t3_times != 0) && (rd_val == wr_val));

					if (rd_val != wr_val)
						cout<<"FAILURE: read 0x"<<hex<<rd_val<<"   wrote 0x"<<hex<<wr_val<<endl;

				}
				else if (line[1] == '4')
				{
					uint32_t wr_val;
					uint32_t rd_val;
					int t4_times;
					int tt = 0;
					int addr;
					cout<<"T4 - write32/read32 EBR test"<<endl;
					i = sscanf(&line[2],  "%d", &t4_times);
					if (i != 1)
						t4_times = 100000;

					do
					{
						addr = (tt & 0x3ffc);
						wr_val = ((rand())<<16) | rand();
						pEBR->write32(addr, wr_val);
					  	rd_val = pEBR->read32(addr);
						cout<<dec<<tt<<": addr= "<<hex<<addr<<"  wr="<<hex<<wr_val<<"  rd="<<hex<<rd_val<<endl;
						if (t4_times > 0)
							--t4_times;
						++tt;

					} while ((t4_times != 0) && (rd_val == wr_val));

					if (rd_val != wr_val)
						cout<<"FAILURE @ 0x"<<hex<<addr<<"  read 0x"<<hex<<rd_val<<"   wrote 0x"<<hex<<wr_val<<endl;

				}
				else if (line[1] == '5')
				{
					uint8_t wr_val;
					uint8_t rd_val;
					int t5_times;
					int tt = 0;
					int addr;
					cout<<"T5 - write8/read8 EBR test"<<endl;
					i = sscanf(&line[2],  "%d", &t5_times);
					if (i != 1)
						t5_times = 100000;

					do
					{
						addr = (tt & 0x3fff);
						wr_val = (rand() & 0xff);
						pEBR->write8(addr, wr_val);
						rd_val =  pEBR->read8(addr);
						cout<<dec<<tt<<": addr= "<<hex<<addr<<"  wr="<<hex<<(int)wr_val<<"  rd="<<hex<<(int)rd_val<<endl;
						if (t5_times > 0)
							--t5_times;
						++tt;

					} while ((t5_times != 0) && (rd_val == wr_val));

					if (rd_val != wr_val)
						cout<<"FAILURE @ 0x"<<hex<<addr<<"  read 0x"<<hex<<(int)rd_val<<"   wrote 0x"<<hex<<(int)wr_val<<endl;

				}
				break;
	

			case 'h':  // Help
			case 'H':
			case '?':
				showHelp();
				break;

			case 'q':
			case 'Q':
			case 'X':
			case 'x':
				done = true;
				break;

			default:
				showHelp();
		}

		strcpy(lastCmd, line);
	}

}


/**
 * Show menu commands and the help.
 */
void    showHelp(void)
{
	cout<<"\n\n======= PCIe Basic Demo Menu ==========\n";
	cout<<" c - PCI Config Registers\n";
	cout<<" e - PCIe Extended Capabilities Registers\n";
	cout<<" l - LED test\n";
	cout<<" d - DIP switch setting\n";
	cout<<" m[cf] - EBR memory test\n";
	cout<<" i <file> - Input file to EBR memory\n";
	cout<<" o <file> - Output EBR memory to file\n";
	cout<<" r[bsl] [<addr> [count]]\n";
	cout<<" w[bsl] [<addr> <data_1>...<data_n>]\n";
	cout<<" b[0|1] - set BAR space access\n";
	cout<<" v - version and driver info\n";
	cout<<" t[1-5] [num] - run a debug test num times\n";
	cout<<" q,x - exit\n\n";
}


