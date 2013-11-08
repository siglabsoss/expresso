/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/*
 * Lattice API DebugPrint Utility Unit Tests
 *
 * This file tests the DebugPrint routine and macros.
 */


#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>

using namespace std;

#include "DebugPrint.h"
#include "PCIeAPI.h"


using namespace LatticeSemi_PCIe;


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/**
 * MAIN entry point.
 */   
int main(int argc, char **argv, char **env)
{


	cout<<"\n\n========================================================\n";
	cout<<"           Lattice LSC_API DebugPrint Tests\n";
	cout<<"========================================================\n";

	cout<<"This test uses the DEBUGPRINT utility to log messages into the \n";
	cout<<"system log to verify log operations and string formatting.\n";
	cout<<"The messages will go to /var/log/messages on RedHat systems.\n";
	cout<<"Try with the RELEASE version and there should be no output"<<endl;


	cout<<"\n1. DEBUGSTR(message)"<<endl;
	DEBUGSTR(("A message\n"));
	DEBUGSTR(("B message.."));
	DEBUGSTRNL(("end line"));

	cout<<"\n2. DEBUGPRINT((message))"<<endl;
	DEBUGPRINT(("A message\n"));
	DEBUGPRINT(("With a number- n=%d\n", 11223344));
	DEBUGPRINT(("With a hex number- n=%x\n", 0xaabbccdd));
	DEBUGPRINT(("With a char + str- %c + %s\n", 'A', " str"));
	DEBUGPRINT(("With a float- %f\n", 3.14159));


	cout<<"\n3. TRACE"<<endl;
	TRACE();


	cout<<"\n4. ENTER"<<endl;
	ENTER();

	cout<<"\n5. LEAVE"<<endl;
	LEAVE();


	cout<<"\nEnd."<<endl;

}



