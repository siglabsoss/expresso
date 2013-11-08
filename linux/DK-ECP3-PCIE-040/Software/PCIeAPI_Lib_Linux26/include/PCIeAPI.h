/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file PCIeAPI.h */

#ifndef LSC_PCIEAPI_H
#define LSC_PCIEAPI_H

#include "dllDef.h"

 
 
#define RUNTIMECTRL(a) (PCIeAPI::RunTimeCtrl & PCIeAPI::a)
 
namespace LatticeSemi_PCIe
{


/**
 * The wrapper for the Lattice PCIe API DLL library.
 * Instantiating an object of this type provides top-level access to certain basic
 * features of the PCIeAPI library such as returning the version string, setting
 * run-time controls to govern debug output.  See setRunTimeCtrl() clrRunTimeCtrl()
 * and the RunTimeCtrl_t for info on controlling run-time options.
 * @note This class does not need to be instantiated to use the driver interfaces
 * and other classes contained in the DLL.  It just provides a convient wrapper
 * around the entire collection; controlling global library type things.
 *
 * @note This class should be instantiated when using the DLL so that the system
 * logging can be initialized.  The API uses macros DEBUGSTR() to print
 * run-time info to the system log via syslog() calls.  First the logger
 * needs to be opened, and that is done in this constructor (can closed
 * in the destructor).
 */ 
class DLLIMPORT PCIeAPI
{
  public:
	  /** Bitmaps for controlling run-time behavior	of the DLL */
	  enum RunTimeCtrl_t {NONE=0,      /**< no options set */
		                  VERBOSE=1,   /**< enable run-time diag printing */
		                  DEBUG1=2, 
		                  IGNORE_VER_ERR=4, /**< continue running even if IP IDs don't match */
		                  ALL=0xffffffff  /**< select every option */
	                     };
    PCIeAPI();
    virtual ~PCIeAPI(void);

	const char *getVersionStr(void);

	/** Global control over PCIeAPI lib operations, such as verbose printing during running. */
	static uint32_t RunTimeCtrl;

	/** Set a specific fucntional mode durint execution (like verbose printing to stdout) */
	//static void setRunTimeCtrl(RunTimeCtrl_t a) {RunTimeCtrl = RunTimeCtrl | a;}
	static void setRunTimeCtrl(RunTimeCtrl_t a);

	/** Clear (disable) a specific run-time option during execution */
	//static void clrRunTimeCtrl(RunTimeCtrl_t a) {RunTimeCtrl = RunTimeCtrl & ~a;}
	static void clrRunTimeCtrl(RunTimeCtrl_t a);

  private:

};


} //END_NAMESPACE

#endif
