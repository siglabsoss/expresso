/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file GPIO.h */
 
#ifndef LATTICE_SEMI_PCIE_GPIO_H
#define LATTICE_SEMI_PCIE_GPIO_H

#define POSIX_C_SOURCE 199506

#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <time.h>
#include <exception>

#include "dllDef.h"

#include "Device.h"
#include "RegisterAccess.h"

/*
 * Lattice Semiconductor Corp. namespace.
 */
namespace LatticeSemi_PCIe
{


    

/* Register Definitions */
#define GPIO_ID_REG       0x00
#define GPIO_SCRATCH      0x04
#define GPIO_LED16SEG     0x08
#define GPIO_DIPSW        0x0a
#define GPIO_CNTRCTRL     0x0c
#define GPIO_CNTRVAL      0x10
#define GPIO_CNTRRELOAD   0x14
#define GPIO_DMAREQ       0x18
#define GPIO_WR_CNTR      0x1c
#define GPIO_RD_CNTR      0x20

// Interrupt controller is in GPIO block
#define INTRCTL_ID_REG     0x100
#define INTRCTL_CTRL       0x104
#define INTRCTL_STATUS     0x108
#define INTRCTL_ENABLE     0x10c



// DownCounter Control Reg Bit Defs
#define CNTR_RUN     1
#define CNTR_RELOAD  2 
#define CNTR_INTR_EN 4    // enable interrupt when coutn reaches 0 (not implemented)


// DMA Req/Ack Reg Bit Defs
#define DMA_REQ_WR  1
#define DMA_REQ_RD0 2
#define DMA_REQ_RD1 4
#define DMA_REQ_RD2 8
#define DMA_REQ_RD3 0x10

#define DMA_ACK_WR  0x0100
#define DMA_ACK_RD0 0x0200
#define DMA_ACK_RD1 0x0400
#define DMA_ACK_RD2 0x0800
#define DMA_ACK_RD3 0x1000


// Interrupt Controller Reg Bit Defs
#define INTRCTL_OUT_ACTIVE 0x01
#define INTRCTL_TEST_MODE  0x02
#define INTRCTL_OUTPUT_EN  0x04
#define INTRCTL_INTR_TEST1 0x0100
#define INTRCTL_INTR_TEST2 0x0200
#define INTRCTL_TEST1_EN   0x0001
#define INTRCTL_TEST2_EN   0x0002
#define INTRCTL_DOWN_COUNT_EN          0x0020
#define INTRCTL_INTR_DOWN_COUNT_ZERO   0x0020

#define INTRCTL_INTR_WR_CHAN 0x0001
#define INTRCTL_INTR_RD_CHAN 0x001e


#define OLD_GPIO_ID_VALID    0x53030100
#define GPIO_ID_VALID    0x12043010
#define INTRCTL_ID_VALID 0x12043050


/**
 * Specific definition of a Lattice General Purpose I/O block of IP used in demos.
 *
 * This class inherits from the generic Device class to provide the actual read/write
 * methods.  The class needs a RegisterAccess object (the Driver Interface) to pass
 * to the Device class for the hardware access.
 * <p>
 * The GPIO block contains the following registers:
 * <ul>
 * <li> 0x00 ID register = 0x12043010
 * <li> 0x04 Scratch pad register 
 * <li> 0x08 16 Segment LED register
 * <li> 0x0a 8 DIP switch inputs
 * <li> 0x0c counter control register
 * <li> 0x10 counter current value register
 * <li> 0x14 counter reload value register
 * <li> 0x18
 *	</ul>
 * The GPIO provides a means to verify access to hardware by reading the fixed ID register,
 * writing and reading and verifying the scratch pad, and changing the LED display to
 * provide positive real-tiome feedback.  The counter can also be used to demonstrate 
 * hardware operation independent of software access (32 bit down counter clocked at
 * 125MHz)
	
 * 
 */
class DLLIMPORT GPIO : public LatticeSemi_PCIe::Device
{
public:
	GPIO(const char *nameStr, uint32_t addr, LatticeSemi_PCIe::RegisterAccess *pRA);
	virtual ~GPIO();
 
	void LED16DisplayTest(void);
	void setLED16Display(uint16_t val);
	void setLED16Display(uint8_t num, bool on);
	void setLED16Display(char c);

	uint32_t getID(void);
	void setScratchPad(uint32_t val);
	uint32_t getScratchPad(void);
	uint8_t getDIPsw(void);

	uint32_t getCounter(void);
	void setCounterReload(uint32_t val);
	void runCounter(bool run);

private:
	void	*arg;
   	static uint16_t CharBitMap[0x60];

};

} //END_NAMESPACE

#endif
