/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file DMATest.cpp */
/**
 * \page DMATest_page PCIe DMA IP Tests
 *
 *  The DMATest demo (see DMATest.cpp) contains a series of diagnostic
 *  tests on the IP design associated with
 *  the DMA demo.  The tests verify access to the IP and that the
 *  IP is functioning as intended.  Tests are declared FAIL if the
 *  return value does not match the expected value.  A macro CHECK
 *  is used to test for the correct values.
 * <p>
 * This file can also serve as a reference design for using the various
 * PCIe API library classes and methods.  It plainly illustrates simple
 * tasks such as:
 * <ul>
 * <li> Opening the DLL
 * <li> Opening a driver interface
 * <li> Creating device objects (GPIO, SGDMA, EBR, etc.)
 * <li> Using read/write methods on device objects to access hardware
 * <li> Transfering data via DMA between the Eval Board and driver memory
 * <li> Interrupts
 * <li> Using utility functions such as memory allocation, formatting
 * </ul>
 */


#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

#include "PCIeAPI.h"
#include "MiscUtils.h"
#include "LSCDMA_IF.h"
#include "GPIO.h"
#include "SGDMA.h"
#include "Mem.h"
#include "MemFormat.h"
#include "../MemMap.h"   // definition of all IP devices in this design

#define IMG_FRAME_SIZE (1024*1024) 

#define CHECK(x,y) if (x != y) {printf("Comparison Error Detected @ line: %d\n", __LINE__); exit(-1);}


//////////////////////////////////////////////
// Required to use Lattice PCIe methods
//////////////////////////////////////////////
using namespace LatticeSemi_PCIe;





//////////////////////////////////////////////
//             Main Entry
//////////////////////////////////////////////

/**
 * DMA IP Tests.
 * Perform a set of canned functions to verify driver access and
 * correct IP operation.  Does not perform complex DMA moves, but
 * just tests the basics.
 */
int main(int argc, char *argv[])
{
 	LSCDMA_IF *pDrvr;
 	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
 	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
 	uint32_t boardNum;
 	string infoStr;
	uint32_t rd32_val;
    

    cout<<"\n=======================================\n";
    cout<<  "|   Lattice PCIe SGDMA Demo IP Test   |\n";
    cout<<  "|             ver 1.0                 |\n";
    cout<<  "| Verify access to eval board, IP and |\n";
    cout<<  "| correct system operation.           | \n";
    cout<<  "=======================================\n";


	cout<<"\n\n========================================================\n";
	cout<<"           Lattice LSC_API Openning the DLL Tests\n";
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
    strcpy(boardName, "ECP3");
    strcpy(demoName, "DMA");
	boardNum = 1;
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
       cout<<"Opening LSCDMA_IF...\n";
		pDrvr = new LSCDMA_IF(boardName, 
                              NULL,   // no specific channel, just register access
				              boardNum);

		cout<<"OK.\n";
  
        pDrvr->getDriverVersionStr(infoStr);
        cout<<"Version: "<<infoStr<<endl;
        pDrvr->getDriverResourcesStr(infoStr);
        cout<<"Resources: "<<infoStr<<endl;
        pDrvr->getPCICapabiltiesStr(infoStr);
        cout<<"Link Info: "<<infoStr<<endl;
    

	const DMAResourceInfo_t *pLinkInfo;
	if (pDrvr->getDriverDMAInfo(&pLinkInfo) == false)
	{
	    cout<<"ERROR accessing Driver Link Information in lscdma driver.\n";
	    exit(-1);
	}

	
	cout<<"\n\n----------------------------------\n";
	cout<<"   DMA Info Structures\n";
	cout<<"----------------------------------\n";


	pDrvr->getDriverDMAInfoStr(infoStr);
        cout<<infoStr<<endl;
	ULONG MaxPayloadSize;  /* PCIe max payload size for MWr into PC memory */
	ULONG MaxReadReqSize;  /* PCIe max read size for MRd from PC memory */
	ULONG RCBSize;         /* Root Complex Read Completion size (normally 64 bytes) */
	ULONG LinkWidth;       /* PCIe link width: x1, x4 */


	cout<<" Link Width = "<<dec<<pLinkInfo->LinkWidth<<endl;
	cout<<" MaxPayloadSize = "<<dec<<pLinkInfo->MaxPayloadSize<<endl;
	cout<<" MaxReadReqSize= "<<dec<<pLinkInfo->MaxReadReqSize<<endl;
	cout<<" RCB Size = "<<dec<<pLinkInfo->RCBSize<<endl;
        
        cout<<"\n\n======================================\n";
        cout<<    "       GPIO ACCESS TESTS\n";
        cout<<    "======================================\n";
		cout<<"Creating GPIO device"<<endl;
   	    GPIO *pGpio = new GPIO("GPIO", memGPIO(0), pDrvr); 

    
        rd32_val = pGpio->getID();
        cout<<"Read GPIO ID register: 0x"<<hex<<rd32_val<<endl;
	CHECK(0x12043010, rd32_val);
        cout<<"Write/Read/Verify GPIO ScratchPad:\n";
        cout<<"GPIO ScratchPadReg = 0x"<<hex<<pGpio->getScratchPad()<<endl;
        cout<<"Set to 0x44332211...";
        pGpio->setScratchPad(0x44332211);
        cout<<"GPIO ScratchPadReg = 0x"<<hex<<pGpio->getScratchPad()<<endl;
        if (pGpio->getScratchPad() != 0x44332211)
	{
           cout<<"FAIL!!!"<<endl;
	   exit(-1);
	}
        else        
	{
           cout<<"PASS"<<endl;
	}

        cout<<"LED test...\n";
        pGpio->LED16DisplayTest();
    
    


        cout<<"\n\n======================================\n";
        cout<<    "       INTERRUPT CONTROLLER TESTS\n";
        cout<<    "======================================\n";
	rd32_val = pGpio->read32(INTRCTL_ID_REG);
	printf("IntrCtrl  ID: %08x\n", rd32_val);
	CHECK(0x12043050, rd32_val);

	// Record Interrupt Controller state to later restore
	uint32_t save1 = pGpio->read32(INTRCTL_CTRL);
	uint32_t save2 = pGpio->read32(INTRCTL_ENABLE);

	printf("\nClear interrupt control, disable everything:\n");
	pGpio->write32(INTRCTL_CTRL, 0);
	pGpio->write32(INTRCTL_ENABLE, 0);
	printf("\tctrl = %08x\n", pGpio->read32(INTRCTL_CTRL));
	printf("\tstatus = %08x\n", pGpio->read32(INTRCTL_STATUS));
	printf("\tenable = %08x\n", pGpio->read32(INTRCTL_ENABLE));

	printf("\nLooping interrupts (watch 16seg LEDs change)...\n");
	pGpio->write32(INTRCTL_CTRL, INTRCTL_TEST_MODE | INTRCTL_OUTPUT_EN);
	pGpio->write32(INTRCTL_ENABLE, INTRCTL_TEST1_EN);
	for (int i = 0; i < 100; i++)
	{
		pGpio->write32(INTRCTL_CTRL, INTRCTL_INTR_TEST1 | INTRCTL_TEST_MODE | INTRCTL_OUTPUT_EN);
		Sleep(50);
		printf(".");
	}
	pGpio->write32(INTRCTL_CTRL, 0);
	pGpio->write32(INTRCTL_ENABLE, 0);

	// Restore previous interrupt controller state
	pGpio->write32(INTRCTL_CTRL, save1);
	pGpio->write32(INTRCTL_ENABLE, save2);


        cout<<"\n\n======================================\n";
        cout<<    "       SGDMA ACCESS TESTS\n";
        cout<<    "======================================\n";
	cout<<"Creating SGDMA device"<<endl;
   	SGDMA *pSgdma = new SGDMA("SGDMA", memSGDMA(0), pDrvr); 
    
        cout<<"Read SGDMA ID register: 0x"<<hex<<(pSgdma->getID())<<endl;
        

	pSgdma->showGlobalRegs();

	pSgdma->showChannelRegs();

        cout<<"\n\n=======================================================\n";
	cout<<"Testing Buffer Descriptor Registers: ";
	if (pSgdma->verifyBufDescRegs())
	{
	    cout<<"PASS"<<endl;
	}
	else
	{
	    cout<<"FAIL"<<endl;
	    exit(-1);
	}
        cout<<"=======================================================\n";

	pSgdma->showBufDescRegs();
   


        cout<<"\n\n======================================\n";
        cout<<    "       ColorBar IP ACCESS TESTS\n";
	cout<<    " First colors should be 0x000000ff\n";
        cout<<    "======================================\n";
	pGpio->write32(0x30, 1);  // set reset
	pGpio->write32(0x30, 0);  // clear reset to run
	for (int i = 0; i < 16 ; i++)
	{
	    rd32_val = pDrvr->read32(memIMG_FIFO(0));
	    printf("%d: %08x\n", i, rd32_val);
	    CHECK(0x000000ff, rd32_val);
	}
	


        cout<<"\n\n======================================\n";
        cout<<    "       EBR ACCESS TESTS\n";
        cout<<    "======================================\n";
	cout<<"Creating EBR Memory device object"<<endl;
	uint32_t *buf =	(uint32_t *)malloc(pSgdma->getSizeDrvrCB());
	string outs;
	
	if (buf == NULL)
	{
            cout<<"ERROR allocating buffer memory\n";
            ShowLastError();
            exit(-1);
    }

   	Mem *pEbrMem = new Mem("EBR64", EBR64_SIZE, memEBR_64(0), pDrvr); 

	// Let's try testing 1024 first and build up
	pGpio->write32(0x28, 0);
	if (pEbrMem->test(1024))
	{
	    cout<<"PASS"<<endl;
	}
	else
	{
	    cout<<"FAIL"<<endl;
	    exit(-1);
	}

     // Display memory using utility format function
	pEbrMem->get(0, 256, buf);
	MemFormat::formatBlockOfWords(outs, 0, 64, buf);
	cout<<outs;

        cout<<"\n\n======================================\n";
        cout<<    "       EBR_IMG ACCESS TESTS\n";
        cout<<    "======================================\n";

	cout<<"\n\nEBR=0xaaaaaaaa  XOR mask = 0\n";
	pGpio->write32(0x28, 0);
	memset(buf, 0xaa, 256);
	pEbrMem->set(0, 256, buf);
	memset(buf, 0x00, 256);
	pEbrMem->get(0, 256, buf);
	for (int i = 0; i < 16; i++)
	{
	    printf("%d: %x\n", i, buf[i]);
	    CHECK(buf[i], 0xaaaaaaaa);
	}

	cout<<"\n\nEBR=0xaaaaaaaa  XOR mask = ffffffff\n";
	pGpio->write32(0x28, 0xffffffff);
	memset(buf, 0xaa, 256);
	pEbrMem->set(0, 256, buf);
	memset(buf, 0x00, 256);
	pEbrMem->get(0, 256, buf);
	for (int i = 0; i < 16; i++)
	{
	    printf("%d: %x\n", i, buf[i]);
	    CHECK(buf[i], 0x55555555);
	}

	cout<<"\n\nEBR=0xaaaaaaaa  XOR mask = f0f0f0f0\n";
	pGpio->write32(0x28, 0xf0f0f0f0);
	memset(buf, 0xaa, 256);
	pEbrMem->set(0, 256, buf);
	memset(buf, 0x00, 256);
	pEbrMem->get(0, 256, buf);
	for (int i = 0; i < 16; i++)
	{
	    printf("%d: %x\n", i, buf[i]);
	    CHECK(buf[i], 0x5a5a5a5a);
	}
	

        cout<<"\n\n======================================\n";
        cout<<    "       DMA TRANSFER TESTS\n";
        cout<<    "======================================\n";
        cout<<"\n-------------- Verify Driver Common Buffer Access -----------------\n";
        cout<<"getSizeDrvrCB = "<<dec<<pSgdma->getSizeDrvrCB()<<" bytes\n";
        if (pSgdma->testDrvrCB())
	{
           cout<<"PASS\n";
	}
        else
	{
            cout<<"FAIL!!!\n";
	    exit(-1);
	}
        
        cout<<"--------------- DMA Read Test -----------------\n";            
        cout<<"\nLoad pattern in Driver Common Buffer\n";
        pSgdma->fillPatternDrvrCB(0x12340000);
	pGpio->write32(0x28, 0x00000000);  // clear bit mask pattern
        
        cout<<"Configure SGDMA Chan1 and Transfer data to EBR_64\n";
        pSgdma->ReadFromCB(1,     // channel
                          4096, // len
                          WB(memEBR_64(0)),  // destination Wishbone bus address in IP
                          1,           // linear memory mode
                          DATA_64BIT,  // size of source data
                          1,           // use 1 BD
                          1);          // start at BD #1
        
	// Display some diagnostics
	pSgdma->showGlobalRegs();
	pSgdma->showChannelRegs();

        cout<<"Verify pattern in EBR memory...\n";
	memset(buf, 0, 4096);  // clear to ensure new values are loaded
        pEbrMem->get(0, 4096, buf);
#if 0
        for (int i = 0; i < 4096/4; i++)
        {
	    CHECK(buf[i], (0x12340000 | i));
        }
#endif
        
	outs.clear();
	MemFormat::formatBlockOfWords(outs, 0, 64, buf);
	cout<<outs;



        cout<<"\n--------------- DMA Write Test -----------------\n";            
        cout<<"\nLoad pattern in EBR\n";
	pEbrMem->fill(0x55, 0, 4096);

	// Set the XOR Mask
	pGpio->write32(0x28, 0xffff0000);  // load bit invert pattern
        
        cout<<"Clear system memory\n";
        pSgdma->clearDrvrCB();
        
        cout<<"Configure SGDMA Chan0 and Transfer data to PC\n";
        pSgdma->WriteToCB(0,     // channel
                          4096, // len
                          WB(memEBR_64(0)),  // source Wishbone bus address in IP
                          1,           // linear memory mode
                          DATA_64BIT,  // size of source data
                          1,           // use 1 BD
                          0);          // start at BD #0
        
	// Display some diagnostics
	pSgdma->showGlobalRegs();
	pSgdma->showChannelRegs();

        cout<<"Verify pattern in memory...\n";
	memset(buf, 0, 4096);  // clear to ensure new values are loaded
        pSgdma->getDrvrCB(buf, 4096);
#if 1
        for (int i = 0; i < 4096/4; i++)
        {
	    CHECK(buf[i], 0xaaaa5555);
        }
#endif 
	pSgdma->showDrvrCB(256);



        cout<<"\n\n======================================\n";
	cout<<" User Space Memory Allocation Test\n";
        cout<<"======================================\n";
        cout<<"Allocate 1MB user buf memory...\n";
        
	uint32_t *pBigBuf = (uint32_t *)AllocAlignedBuffer(IMG_FRAME_SIZE);
	uint32_t *pBB;
	if (pBigBuf == NULL)
	{
	    cout<<"!!! ERROR !!! couldn't allocate 1MB\n";
	    ShowLastError();
	}
	else
	{
	    FreeAlignedBuffer(pBigBuf, (IMG_FRAME_SIZE));
	}


        cout<<"\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout<<    "   ALL TESTS COMPLETED \n";
        cout<<    "!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

        
        free(buf);
	delete pGpio;
	delete pSgdma;
	delete pEbrMem;
        delete pDrvr;
    }
    catch (std::exception &e)
    {
	cout<<"\nError during testing: "<<e.what()<<endl;
	ShowLastError();
	return(-1);

    }
	
	

    return EXIT_SUCCESS;
}
