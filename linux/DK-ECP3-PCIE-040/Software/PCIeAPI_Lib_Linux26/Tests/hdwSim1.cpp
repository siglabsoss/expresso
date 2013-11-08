/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/*
 * Lattice API open the library test
 *
 * This file tests "opening" the DLL and classes in it.
 * Specifically simulated stuff for testing when hdw is not
 * available.
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>

using namespace std;


#include "PCIeAPI.h"
#include "PCIe_IF.h"
#include "GPIO.h"
#include "Mem.h"

#include "SimHdw.h"
#include "GPIO_Sim.h"
#include "Mem_Sim.h"
#include "SimDemoIP.h"
#include "PCIe_Sim.h"

using namespace LatticeSemi_PCIe;


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/**
 * MAIN entry point.
 */   
int main(int argc, char **argv, char **env)
{
	uint8_t buf[256];


	cout<<"\n\n========================================================\n";
	cout<<"           Lattice PCIeAPI Hdw Sim Testing\n";
	cout<<"========================================================\n";


	cout<<"Instantiate the DLL top level class\n";
	PCIeAPI theDLL;
	cout<<"Dll version info: "<<theDLL.getVersionStr()<<endl;

	try
	{
		cout<<"\n\n";
		cout<<"==========================================\n";
		cout<<"\nCreating Hdw Sim Objects object"<<endl;
		cout<<"==========================================\n";

		// Step 1.
		// Instantiate the hardware simulation block into which we'll add the simulated
		// hardware devices.  This needs to be done to "create" the simulated hardware
		// we'll access.
		cout<<"Create the Hardware Sim block\n";
		SimDemoIP *pDemoHdw = new SimDemoIP(true);
			  

		// Step 2.
		// Create the simulated hardware devices that exist in the FPGA hardware for
		// a specific, real hardware demo
		cout<<"Create GPIO_Sim hardware device\n";
		GPIO_Sim *pGpioDev = new GPIO_Sim("GPIOsim", BAR0(0));
		pDemoHdw->addDev(pGpioDev);

		cout<<"Create BAR0 Mem1 hardware device\n";
		Mem_Sim *pMemDev = new Mem_Sim("Mem1", BAR0(0x10000), 0xffff0000, 0x10000);  // 64kB
		pDemoHdw->addDev(pMemDev);

		cout<<"Create BAR1 Mem2 hardware device\n";
		Mem_Sim *pMemDev2 = new Mem_Sim("Mem2", BAR1(0x10000), 0xffff0000, 0x10000);  // 64kB
		pDemoHdw->addDev(pMemDev2);

		// Step 3.
		// Create the simulated driver interface to access the simulated hardware
		// and pass it the list of hardware devices
		cout<<"Create the simulated Driver Interface\n";
		PCIe_Sim *pSimIF = new PCIe_Sim(pDemoHdw);   // use default PCI cfg reg settings


		// Step 4.
		// create the Device Access Objects that have methods to control the hardware
		// objects
		GPIO *pGpio = new GPIO("GPIO", BAR0(0x100), pSimIF);
		cout<<"Calling GPIO getID: "<<hex<<pGpio->getID()<<endl;
		cout<<"Calling GPIO getScratchPad(): "<<hex<<pGpio->getScratchPad()<<endl;
		cout<<"Calling GPIO setScratchPad(0x12345678)\n";
		pGpio->setScratchPad(0x12345678);
		cout<<"Calling GPIO getScratchPad(): "<<hex<<pGpio->getScratchPad()<<endl;

		pGpio->setLED16Display('A');

		for (int i = 0; i <= 0x10; i = i + 4)
			cout<<"read32(GPIO["<<i<<"]) = "<<pSimIF->read32(BAR0(0 + i))<<endl;


		LatticeSemi_PCIe::Mem *pEBR0 = new Mem("EBR0", 0x10000, BAR0(0x10000), pSimIF);
		pEBR0->clear();
		pEBR0->get(0, 256, buf);



		delete pGpio;
		delete pSimIF; 

		delete pDemoHdw;    // all individual device objects deleted in here too

	}
	catch (std::exception &e)
	{
		cout<<"\nError openning driver interface: "<<e.what()<<endl;
	}

	cout<<"\nEnd."<<endl;

}



