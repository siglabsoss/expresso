/** @file dllmain.cpp */
/**
 * PCIe Demo C++ Shared Library.
 *
 * This provides the framework for a loadable shared library that is loaded
 * by the Java GUI and creates the platform (see Cpp_Jni).  Once created, the
 * JNI interface functions then invoke the methods of the platform.
 * This file is primarily the .so library interface for initialization and
 * shutdown.
 */


#include <iostream>
#include <stdlib.h>

#include "DebugPrint.h"

using namespace std;

/* See Cpp_Jni.cpp */
extern bool createSFIFDemo(void);
extern bool deleteSFIFDemo(void);

 

/*
 * Standard GNU shared library entry interface
 * called before the main() in the program that loaded us
 */
void __attribute__ ((constructor)) init_lib(void)
{
	DEBUGSTR("Cpp_Jni DllMain(): constructor");


	if (!createSFIFDemo())
	{
		cout<<"ERROR! Can't create SFIF Demo Java interface!"<<endl;
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

	if (!deleteSFIFDemo())
	{
		cout<<"ERROR! Can't delete SFIF Demo Java interface!"<<endl;
		exit(-1);
	}

}


