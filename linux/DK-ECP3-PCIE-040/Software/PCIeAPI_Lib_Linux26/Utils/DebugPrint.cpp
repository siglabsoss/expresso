/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file DebugPrint.cpp */

using namespace std;

#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "DebugPrint.h"

/**
 * Format a string, using printf rules, and submit the string
 * to the Windows debug queue.
 * @param fmtStr printf-like format string
 * @param ... variable list of params to place in the formatted string
 */
void DebugPrint(char *fmtStr, ...)
{
#ifndef RELEASE
	va_list args;
	char    str[MAX_MSG_SIZE];

	// Format the args into a string
	va_start(args, fmtStr);
	vsnprintf(str, MAX_MSG_SIZE, fmtStr, args);
	va_end(args);

	// Send to Linux system log
	syslog(LOG_INFO, str);


#ifdef DEBUG
#ifdef VERBOSE
	cout<<str<<endl;  // also print to the screen
#endif
#endif

#endif

}
