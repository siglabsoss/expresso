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

#define SIM

#include "PCIeAPI.h"
#include "PCIe_IF.h"
#include "GPIO.h"

#ifdef SIM
#include "GPIO_Sim.h"
#endif

using namespace LatticeSemi_PCIe;


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/**
 * MAIN entry point.
 */   
int main(int argc, char **argv, char **env)
{
	PCIe_IF *pDrvr;


	cout<<"\n\n========================================================\n";
	cout<<"           Lattice LSC_API Opening the DLL Tests\n";
	cout<<"========================================================\n";


	cout<<"Instantiate the DLL top level class\n";
	PCIeAPI theDLL;
	cout<<"Dll version info: "<<theDLL.getVersionStr()<<endl;

	try
	{
		cout<<"\n\n";
		cout<<"==========================================\n";
		cout<<"\nCreating PCIexpress Intf object: "<<endl;
		pDrvr = new PCIe_IF(PCIe_IF::pLSCSIM_DRIVER,
				   NULL,
				   "SIM",
				   0,
				   NULL,
				   NULL);

		cout<<"OK.\n";


		cout<<"Creating GPIO device"<<endl;
		GPIO *pGpio;

#ifdef SIM
		GPIO_Sim *pGpioSim = new GPIO_Sim("GPIOsim", BAR0(0x00)); 
		pGpio = (GPIO *)pGpioSim;
#else
		GPIO *pGpio = new GPIO("GPIO", BAR0(0x00)); 
#endif



		cout<<"Calling getID: "<<hex<<pGpio->getID()<<endl;
		cout<<"Delete Driver Interface\n";

		delete pGpio;
		delete pDrvr;
	}
	catch (std::exception &e)
	{
		cout<<"\nError openning driver interface: "<<e.what()<<endl;
	}

	cout<<"\nEnd."<<endl;

}



