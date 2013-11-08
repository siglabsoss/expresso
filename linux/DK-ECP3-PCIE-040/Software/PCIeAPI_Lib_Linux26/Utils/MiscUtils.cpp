/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file MiscUtils.cpp
 * This is a collection of miscellaneous routines.  Some routines provide 
 * for 64KB aligned memory allocation.  Another displays the last system
 * error message in readable format.
 */



using namespace std;

#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>


#include "MiscUtils.h"

#include "PCIeAPI.h"

using namespace LatticeSemi_PCIe;

/**
 * Display the system error message associated with the last
 * system error code.
 * Use for when openning a file or other system call fails.
 */
void ShowLastError(void) 
{

	perror("PCIeAPI:");

#if 0
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;

	DWORD dw = GetLastError(); 

	FormatMessage(
				 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				 FORMAT_MESSAGE_FROM_SYSTEM |
				 FORMAT_MESSAGE_IGNORE_INSERTS,
				 NULL,
				 dw,
				 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				 (LPTSTR) &lpMsgBuf,
				 0, NULL );


	cout<<(LPCTSTR)lpMsgBuf<<endl;
	LocalFree(lpMsgBuf);
#endif


}



/**
 * Allocate a block of virtual memory that normally starts on a
 * 64kB boundary.  This is very useful for allocating a large buffer
 * in User Space to be used fo SG DMA operations.  Having the buffer
 * aligned to 64kB makes the Mapping use less buffer descriptors.
 * This uses the Windows API calls VirtualAlloc() and VirtualLock()
 * to allocate the memory and prevent it from being swapped.
 * VirtualLock() will fail if the amount of allocated memory is larger
 * than approx 200KB, as defined by Windows WorkingSet memory manager.
 * This default can be extended with Windows API calls.
 * The method will still allocate the buffer, but it will not be able
 * to lock it into memory and prevent swapping.
 * 
 * @param size the size of the buffer 
 * @return a pointer to the allocated block, or NULL if allocation fails.
 * @note use ShowLastError() for info regarding system failure.
 */
void    *AllocAlignedBuffer(size_t size)
{
	void *pUserSpaceBuf;


	if (posix_memalign(&pUserSpaceBuf, 0x10000, size))
	{
		ShowLastError();
		return(NULL);
	}

	return(pUserSpaceBuf);


#if 0  // Windows
	// Use VirtualAlloc
	pUserSpaceBuf = (void *)VirtualAlloc(NULL, 
					 size, 
					 MEM_COMMIT | MEM_RESERVE, 
					 PAGE_READWRITE);
	if (pUserSpaceBuf == NULL)
	{
		if (RUNTIMECTRL(VERBOSE))
		{
			cout<<"ERROR! VirtualAlloc failed.  exiting\n";
			ShowLastError();
		}
		return(NULL);
	}

	// The maximum number of pages that a process can lock is
    // equal to the number of pages in its minimum working set minus a small overhead.
	if (VirtualLock(pUserSpaceBuf, size) == FALSE)
	{
		if (RUNTIMECTRL(VERBOSE))
		{
			cout<<"WARNING: VirtualLock permission denied.\n";
			ShowLastError();
		}
		//return(NULL);
	}
#endif

}

/**
 * Unlock and release memory allocated with AllocAlignedBuffer()
 * @param pUserSpaceBuf pointer to buffer to release
 * @param size the size of the buffer 
 */
bool    FreeAlignedBuffer(void *pUserSpaceBuf, size_t size)
{

	free(pUserSpaceBuf);
	return(true);

#if 0  // Windows
	if (VirtualUnlock(pUserSpaceBuf, size) == FALSE)
	{
		if (RUNTIMECTRL(VERBOSE))
		{
			cout<<"VirtualUnlock failed.\n";
			ShowLastError();
		}
	}

	if (VirtualFree(pUserSpaceBuf, 0, MEM_RELEASE) == FALSE)
	{
		if (RUNTIMECTRL(VERBOSE))
		{
			cout<<"ERROR! VirtualFree failed.\n";
			ShowLastError();
		}
		return(false);
	}

	return(true);
#endif
}



/**
 * Return the Lattice PCIe IP board name to open.
 * This is specified by the system environment variable LSC_PCIE_BOARD.
 * The name format follows the Windows PCI Vendor/Device naming pair.  For example
 * "ven_1204&dev_5303".  The quotes maybe around the name to escape the '&' which
 * causes problems when assigning with set.  Two ways to create the environment
 * variable:
 * <ul>
 * <li>set LSC_PCIE_BOARD="ven_1204&dev5303"
 * <li> run the PCIeScan utility to locate all Lattice baords and create a
 *  config file that sets up the necessary env vars.
 * </ul>
 * @param boardName string to contain name read from envirment variable.  The string
 * size must be (LSC_PCIE_BOARD_NAME_LEN + 1) in size (to hold \0).
 * @return true if environment variable found and assigned.  False if not found.
 * User will need to use an appropriate default value.
 * @note The boardName contents are not modified if the env var is not found, so a
 * default value can be pre-loaded and will remain.
 */
bool GetPCIeEnvVarBoardName(char *boardName)
{
	char *pEnvVar;

	// Find out what board the user expects to run on
	pEnvVar = getenv("LSC_PCIE_BOARD");
	if (pEnvVar == NULL)
	{
		return(false);
	}
	else  // str is given, but it could have a " around it, if so remove "
	{
		if (*pEnvVar == '"')
			strncpy(boardName, pEnvVar + 1, LSC_PCIE_BOARD_NAME_LEN);
		else
			strncpy(boardName, pEnvVar, LSC_PCIE_BOARD_NAME_LEN);
		boardName[LSC_PCIE_BOARD_NAME_LEN] = '\0';	// ensure NULL terminated
	}

	return(true);
}


/**
 * Return the Lattice PCIe IP demo name to open.
 * This is specified by the system environment variable LSC_PCIE_INSTANCE.
 * The name format follows the Windows PCI SubSys naming.  For example
 * "subsys_30301204".  The quotes maybe around the name or not.  Quotes are
 * usually used with the board name, so this supports them too.
 * Two ways to create the environment variable:
 * <ul>
 * <li>set LSC_PCIE_IP_ID="subsys_30301204"
 * <li> run the PCIeScan utility to locate all Lattice baords and create a
 *  config file that sets up the necessary env vars.
 * </ul>
 * @param demoName string to contain name read from envirment variable.  The string
 * size must be (LSC_PCIE_DEMO_NAME_LEN + 1) in size (to hold \0).
 * @return true if environment variable found and assigned.  False if not found.
 * User will need to use an appropriate default value.
 * @note The demoName contents are not modified if the env var is not found, so a
 * default value can be pre-loaded and will remain.
 */
bool GetPCIeEnvVarDemoName(char *demoName)
{
	char *pEnvVar;

	// Find out what board the user expects to run on
	pEnvVar = getenv("LSC_PCIE_IP_ID");
	if (pEnvVar == NULL)
	{
		return(false);
	}
	else  // str is given, but it could have a " around it, if so remove "
	{
		if (*pEnvVar == '"')
			strncpy(demoName, pEnvVar + 1, LSC_PCIE_DEMO_NAME_LEN);
		else
			strncpy(demoName, pEnvVar, LSC_PCIE_DEMO_NAME_LEN);
		demoName[LSC_PCIE_DEMO_NAME_LEN] = '\0';  // ensure NULL terminated
	}

	return(true);
}

/**
 * Return the Lattice PCIe eval board instance number to open.
 * This is specified by the system environment variable LSC_PCIE_INSTANCE.
 * The environment variable must be assigned an interger value.
 * The range is typically from 1 to 4.
 * (No real maximum limit, but practically there are never more than 4 PCIe slots
 * in a system).
 * If the environment variable is not defined then the default value passed in
 * is returned.
 * @param defaultVal default board instance to use if env var not found
 * @return integer value of board instance number to use in opening a Driver Interface.
 */
uint32_t GetPCIeEnvVarBoardNum(uint32_t defaultVal)
{
	char *pEnvVar;
	uint32_t boardNum;

	// Find out what board number the user expects to run on
	pEnvVar = getenv("LSC_PCIE_INSTANCE");
	if (pEnvVar == NULL)
		boardNum = defaultVal;
	else
		boardNum = atoi(pEnvVar);

	return(boardNum);
}

/** 
 * Return the Lattice PCIe environment variables  associated with chosing the
 * specific PCIe eval board, demo IP and instance to open.
 * An application may have an option to open a variety of compatible eval boards.
 * This function would be called at the begining of an application to retrieve
 * the user selection of what specific board to connect to.
 * The eval board to connect to is based on the triplet of: (board name),(demo name),
 * (board instance).  These values are specified in 3 environment variables.
 * The environment variables used are:
 * <ul>
 * <li> LSC_PCIE_BOARD - PCI ven & dev string
 * <li> LSC_PCIE_IP_ID - PCI SubSys ID string
 * <li> LSC_PCIE_INSTANCE - integer value
 * </ul>
 * 
 * @param boardName string to contain name read from LSC_PCIE_BOARD envirment variable
 * @param demoName string to contain name read from LSC_IP_ID_BOARD envirment variable
 * @param demoName string to contain name read from LSC_PCIE_INSTANCE envirment variable
 * @return true if environment variables found and assigned.  False if not found.
 * User will then need to use an appropriate default value.
 *
 * @note The boardName and demoName contents are not modified if the env vars are not found, so a
 * default value can be pre-loaded and will remain.  boardNum will be assigned 1 for a default.
*/
bool GetPCIeEnvVars(char *boardName, char *demoName, uint32_t &boardNum)
{
	bool retVal;

	retVal = GetPCIeEnvVarBoardName(boardName);
	retVal = retVal && GetPCIeEnvVarDemoName(demoName);
	boardNum = GetPCIeEnvVarBoardNum(1);

	return(retVal);
}
