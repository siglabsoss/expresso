/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file PCIeAPI.cpp
 * C++ DLL Interface.
 * This file is run first when the DLL is loaded by the application program.
 * This file also is repsonsible for closing down the driver and objects when
 * the DLL is exited - application closes or errors out.
 */
#include <iostream>
#include <stdlib.h>
#include <syslog.h>
#include <stdint.h>

#include "dllDef.h"
#include "bld_num.h"
#include "PCIeAPI.h"

#include "DebugPrint.h"

using namespace std;
using namespace LatticeSemi_PCIe;


//extern "C"	void OutputDebugString(const char *);


// Global Variable Initialization
uint32_t	PCIeAPI::RunTimeCtrl = PCIeAPI::NONE;


/**
 * Construct an interface to the PCIeAPI DLL libarary.
 * Version info and run-time control is accessable through an object of
 * this class.
 * @note This class should be instantiated when using the DLL so that the system
 * logging can be initialized.  The API uses macros DEBUGSTR() to print
 * run-time info to the system log via syslog() calls.  First the logger
 * needs to be opened, and that is done in this constructor (can closed
 * in the destructor).
 */
PCIeAPI::PCIeAPI()
{
	// Have to open access to the system log
	openlog("PCIeAPI", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_USER);
	
	DEBUGSTR("PCIeAPI: constructor\n");
}

/** Set a specific functional mode during execution (like verbose printing to stdout) */
void  PCIeAPI::setRunTimeCtrl(RunTimeCtrl_t a)
{
    RunTimeCtrl = RunTimeCtrl | a;
}

/** Clear (disable) a specific run-time option during execution */
void  PCIeAPI::clrRunTimeCtrl(RunTimeCtrl_t a) 
{
    RunTimeCtrl = RunTimeCtrl & ~a;
}


/**
 * Close a PCIeAPI DLL library.
 */
PCIeAPI::~PCIeAPI()
{
	DEBUGSTR("PCIeAPI: destructor\n");

	closelog();
}


/**
 * Return the build version and comments associated with the library.
 * Returns a pointer to a C string.
 */
const char *PCIeAPI::getVersionStr(void)
{
	return(PCIEAPI_LIB_VER);
}


