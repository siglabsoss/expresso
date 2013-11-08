/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file DebugPrint.h
 * This collection of debug routines sends formatted message strings
 * to the Windows system debug message queue using API OutputDebugString.
 * The functions are defined as macros so when building for RELEASE to
 * macros are empty - no code impact.  When not building for release (normal
 * development work) debugging is enabled and the macros call different
 * functions that send the string to the debug Q.
 * <p>
 * Use the Windows utility DebugView.exe to view the system log.
 * DebugView is available for download from Sysinternals - www.sysinternals.com
 * The debug version of the driver also logs info to the same place so the
 * user space APIs can be correlated to driver operation.
 */

 
#ifndef LATTICE_SEMI_DEBUGPRINT_H
#define LATTICE_SEMI_DEBUGPRINT_H


#include <syslog.h>
#include <stdarg.h>

#define MAX_MSG_SIZE 256

#include "dllDef.h"


extern void DLLIMPORT DebugPrint(char *fmtStr, ...);

#ifdef RELEASE

#define DEBUGPRINT(a)
#define DEBUGSTR(a)
#define DEBUGSTRNL(a)
#define TRACE()
#define ENTER()
#define LEAVE()

#else

// Use this macro like this: DEBUGPRINT(("BAD cnt=%d\n",n));  
#define DEBUGPRINT(a) DebugPrint a 

#define DEBUGSTR(a) syslog(LOG_INFO, a) // print a string (new line not added)
#define DEBUGSTRNL(a) syslog(LOG_INFO, a)  // print string, (same in linux)
#define TRACE() DebugPrint("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__)
#define ENTER() DebugPrint("ENTER: %s\n", __FUNCTION__)
#define LEAVE() DebugPrint("LEAVE: %s\n", __FUNCTION__)

#endif

// Log an error message and print to stdout
#define ERRORSTR(a) syslog(LOG_ERR, a); cout<<a

#endif

