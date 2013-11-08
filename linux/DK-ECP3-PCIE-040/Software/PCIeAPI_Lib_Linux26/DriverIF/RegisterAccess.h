/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file RegisterAccess.h */
 
#ifndef LATTICE_SEMI_REGISTERACCESS_H
#define LATTICE_SEMI_REGISTERACCESS_H

#define POSIX_C_SOURCE 199506

#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <exception>

#include "dllDef.h"

/*
 * Lattice Semiconductor Corp. namespace
 */
namespace LatticeSemi_PCIe
{


/**
 * Hardware AccessLayer template.
 * This base class provides a template for a driver interfaces to implement the exact methods
 * needed to access the hardware registers.  The main purpose of this class is to provide
 * a common descriptor (class, interface) used by all "real" classes to describe the methods
 * used for reading and writting. Basically everyone uses this as the description of how
 * to read and write registers and polymorphism takes care of who and how its done.
 * 
 * @note This class is an ADT.  This class must be derived from and
 * all virtual functions must be implemented with the actual methods needed to
 * read/write registers in a Lattice FPGA.  The implementor is responsible for
 * any endianess conversion that must take place.  Also, any driver
 * initialization should be done in the constructor, and freed in the
 * destructor.
 */
class DLLIMPORT RegisterAccess
{
public:

    RegisterAccess(void *drvrID) {pDrvrID = drvrID;}
    virtual ~RegisterAccess() {/* Nothing to do */ ;}


    /*=====================================================*/
    /*         REGISTER ACCESS METHODS                     */
    /*=====================================================*/
    virtual uint8_t read8(uint32_t addr) = 0;
    virtual void write8(uint32_t addr, uint8_t val) = 0;
    virtual uint16_t read16(uint32_t addr) = 0;
    virtual void write16(uint32_t addr, uint16_t val) = 0;
    virtual uint32_t read32(uint32_t addr) = 0;
    virtual void write32(uint32_t addr, uint32_t val) = 0;

	/* Block Access Methods */
    virtual bool read8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true) = 0;
    virtual bool write8(uint32_t addr, uint8_t *val, size_t len, bool incAddr=true) = 0;
    virtual bool read16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true) = 0;
    virtual bool write16(uint32_t addr, uint16_t *val, size_t len, bool incAddr=true) = 0;
    virtual bool read32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true) = 0;
    virtual bool write32(uint32_t addr, uint32_t *val, size_t len, bool incAddr=true) = 0;


private:
    void *pDrvrID;


};

} //END_NAMESPACE

#endif
