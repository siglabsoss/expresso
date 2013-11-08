/*
 *  COPYRIGHT (c) 2007 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file MiscUtils.h */
 
#ifndef LATTICE_SEMI_MISCUTILS_H
#define LATTICE_SEMI_MISCUTILS_H

#include "dllDef.h"

#define LSC_PCIE_BOARD_NAME_LEN 17
#define LSC_PCIE_DEMO_NAME_LEN 15

extern void DLLIMPORT ShowLastError(void);
extern void	DLLIMPORT *AllocAlignedBuffer(size_t size);
extern bool	DLLIMPORT FreeAlignedBuffer(void *pUserSpaceBuf, size_t size);
extern bool DLLIMPORT GetPCIeEnvVarBoardName(char *boardName);
extern bool DLLIMPORT GetPCIeEnvVarDemoName(char *demoName);
extern uint32_t DLLIMPORT GetPCIeEnvVarBoardNum(uint32_t defaultVal);
extern bool DLLIMPORT GetPCIeEnvVars(char *boardName, char *demoName, uint32_t &boardNum);
extern bool DLLIMPORT GetPCIeEnvVars(char *boardName, char *demoName, uint32_t &boardNum);


#endif

