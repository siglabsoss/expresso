/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file dllmain.cpp */
/**
 * PCIe Demo C++ Shared Library.
 *
 * This provides the framework for a loadable shared library that is loaded
 * by the Java GUI and creates the platform (see Cpp_Jni.cpp).  Once created, the
 * JNI interface functions then invoke the methods of the platform.
 * This file is primarily the .so library interface for initialization and
 * shutdown.
 */


#include <iostream>
#include <stdlib.h>

#include "DebugPrint.h"

using namespace std;

/* See Cpp_Jni.cpp */
extern bool createBasicDemo(void);
extern bool deleteBasicDemo(void);

 

/*
 * Standard GNU shared library entry interface
 * called before the main() in the program that loaded us
 */
void __attribute__ ((constructor)) init_lib(void)
{
	DEBUGSTR("Cpp_Jni DllMain(): constructor");


	if (!createBasicDemo())
	{
		cout<<"ERROR! Can't create Basic Demo Java interface!"<<endl;
		exit(-1);
	}
}


/*
 * Standard GNU shared library exit interface
 * called when main() program (that loaded us) exits.
 */
void __attribute__ ((destructor)) close_lib(void)
{
	DEBUGSTR("Cpp_Jni DllMain(): destructor");

	if (!deleteBasicDemo())
	{
		cout<<"ERROR! Can't delete Basic Demo Java interface!"<<endl;
		exit(-1);
	}

}


