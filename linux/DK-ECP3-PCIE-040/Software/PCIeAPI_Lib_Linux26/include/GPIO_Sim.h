/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

 
#ifndef LATTICE_SEMI_GPIO_SIM_H
#define LATTICE_SEMI_GPIO_SIM_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <string>
#include <exception>

#include "SimHdw.h"

/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{


/** 
 * Defintion of a Simulated Memory
 */

class DLLIMPORT GPIO_Sim : public SimHdw
{
public:
	GPIO_Sim( const char *nameStr, uint32_t addr);
	virtual ~GPIO_Sim();


protected:

private:

};

} //END_NAMESPACE

#endif
