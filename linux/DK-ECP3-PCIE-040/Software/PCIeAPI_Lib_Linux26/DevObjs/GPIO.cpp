/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file GPIO.cpp */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "PCIeAPI.h"
#include "DebugPrint.h"
#include "GPIO.h"

using namespace LatticeSemi_PCIe;


/**
 * private register defines of the GPIO
 */
#define ID_REG          0x00
#define SCRATCH_REG     0x04
#define LED16_REG       0x08
#define DIP_SW_REG      0x0b
#define CNTR_CTRL_REG   0x0c
#define CNTR_VAL_REG    0x10
#define CNTR_RELOAD_REG 0x14

#define MAX_ADDRESS     0x110    // Includes interrupt controller registers too

/**
 * Construct access to a Lattice Demo GPIO IP module.
 * Register access to the IP in the FPGA device is through the 
 * Driver Interface object instantiated 
 * to access the real hardware and do reads/writes.
 * 
 * The base class Device is initialized to the base address and register size.
 *
 * @param nameStr the name of the FPGA device for unique naming
 * @param baseAddr the physical bus address the device registers start at
 * @param pRA pointer to a RegisterAccess object for accessing the hardware
 */
GPIO::GPIO(const char *nameStr, 
	   uint32_t baseAddr, 
	   RegisterAccess *pRA) : LatticeSemi_PCIe::Device(pRA, nameStr, baseAddr, MAX_ADDRESS)
{
	ENTER();
	/* Base Class Device has setup the name, address, eventLog already */


	LEAVE();
}

/**
 * Destroy the GPIO IP module. 
 * The device should no longer be accessed using any methods.  All references
 * to the device will now be invalid.
 */

GPIO::~GPIO()
{
	ENTER();
	
}



/**
 * Read the 32bit Demo IP ID register.
 * @return 32bit Demo ID value
 * @throws exception if register read fails
 */
uint32_t GPIO::getID(void)
{
	ENTER();
	return(this->read32(ID_REG));
}

/**
 * Write a 32bit value to the scratch pad register.
 * @param val the value to write into the scratch pad
 * @throws exception if hardware access fails
 */
void GPIO::setScratchPad(uint32_t val)
{
	ENTER();
	this->write32(SCRATCH_REG, val);
}

/**
 * Read the 32bit value in the scratch pad register.
 * @param val storage for the scratch pad value
 * @return true if successful; false if error
 */
uint32_t GPIO::getScratchPad(void)
{
	ENTER();
	return(this->read32(SCRATCH_REG));

}

/**
 * Read the 8bit value produced by the DIP switch positions.
 * A 1 = switch in up position, 0=down
 * bit d0=Sw1:4, bit d7=Sw2:1
 * 
 * @return the 8 bit DIP switch register value
 */
uint8_t GPIO::getDIPsw(void)
{
	ENTER();
	return(this->read8(DIP_SW_REG));
}

/**
 * Read the 32bit Counter register value.
 * The counter is a 32bit down-counter, which starts at the reload value
 * and counts down to 0.  It can restart if the mode is set to reload.
 * 
 * @return the 32 bit counter value
 */
uint32_t GPIO::getCounter(void)
{

	ENTER();

	return(this->read32(CNTR_VAL_REG));
}

/**
 * Write the 32bit Counter Reload value.
 * This is the value that gets loaded into the down-counter when it reaches 0.
 * @param val reload value.
 * 
 * @return true if successful; false if error
 */
void GPIO::setCounterReload(uint32_t val)
{
	ENTER();

	this->write32(CNTR_RELOAD_REG, val);
}

/**
 * Start or stop the 32 bit down counter in the GPIO IP module.
 * The counter starts counting down from the value set by
 * setCounterReload().
 * @param run true to start down counter, false to stop it.
 * 
 */
void GPIO::runCounter(bool run)
{
	ENTER();
	if (run)
		this->write8(CNTR_CTRL_REG, 0x03);
	else
		this->write8(CNTR_CTRL_REG, 0x00);

}



/**
 * Test the LED segments by first lighting each segment, then
 * turning them off.  The display the characters "LATTICE*" 
 * in order ending with '*'.
 * If PCIeAPI runtime=verbose then print the actions to stdout.
 */
void GPIO::LED16DisplayTest(void)
{
	int i;

	ENTER();
	cout<<"=== 16 Segment LED Test ===\n";

	if (RUNTIMECTRL(VERBOSE))
		cout<<"Segment Test:";

	// Clear Segments
	setLED16Display((uint16_t)0x0000);

	for (i = 0; i < 16; i++)
	{
		setLED16Display((uint16_t)i, true);
		if (RUNTIMECTRL(VERBOSE))
			cout<<".."<<i;
		Sleep(300);  // msec
	}
	for (i = 15; i >=0 ; i--)
	{
		setLED16Display((uint16_t)i, false);
		if (RUNTIMECTRL(VERBOSE))
			cout<<".."<<i;
		Sleep(300);
	}
	if (RUNTIMECTRL(VERBOSE))
		cout<<endl;
	setLED16Display('L');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"L";
	Sleep(300);
	setLED16Display('A');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"A";
	Sleep(300);
	setLED16Display('T');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"T";
	Sleep(300);
	setLED16Display('T');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"T";
	Sleep(300);
	setLED16Display('I');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"I";
	Sleep(300);
	setLED16Display('C');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"C";
	Sleep(300);
	setLED16Display('E');
	if (RUNTIMECTRL(VERBOSE))
		cout<<"E"<<endl;

	Sleep(300);
	setLED16Display('*');

	LEAVE();

}
 
/**
 * Light the segments of the 16 segment display.
 * Each bit in val controls an LED segment.  A 1 turns is on.  A 0 turns it off.
 * Layout on the board: bit d15 = segU, d0=segA (see board wiring diagram)
 * @param val the 8 bit value to control all 8 LEDs at once
 * 
 */
void GPIO::setLED16Display(uint16_t val)
{
	ENTER();
	this->write16(LED16_REG, val);
}

/**
 * Light a single LED segment.
 * The LED segment specified by num is either turned on or off.  The other LEDs
 * are not effected.
 * @param num LED segment number: segA = 0, segU = 15
 * @param on true to turn on, false to turn off
 */
void GPIO::setLED16Display(uint8_t num, bool on)
{
	uint16_t t;

	ENTER();
	t = this->read16(LED16_REG);

	if (on)
		t = t | (1<<num);
	else
		t = t & ~(1<<num);

	this->write16(LED16_REG, t);
}

/**
 * Display an ASCII character on the display.
 * The proper segments are lit to display the ASCII character.
 * @param c character to display, visible range is: 0x21-0x7f
 */
void GPIO::setLED16Display(char c)
{
	uint16_t bits;

	ENTER();

	if ((uint8_t)c < 0x20)
		c = 0x20;
	else if ((uint8_t)c > 0x7f)
		c = 0x20;

	// Look up the bit settings in a character map.
	bits = GPIO::CharBitMap[(uint8_t)(c - 0x20)];

	// write that map to the LED control registers to display the character
	this->write16(LED16_REG, bits);
}



/**
 * Character Bit Map for a 16 Segment LED.
 * Segment A is wired is lsb (d0) and Segment U is msb (d15).
 * The characters are from Maxim's MAX6954 Display Driver App note.
 * Only 0x21 - 0x7f are printable; all other chars are blank.
 */
uint16_t GPIO::CharBitMap[0x60] =
{
	0x0000,	  // c <= 0x20
	0x000c,	  // 0x21 = !
	0x0280,	  // 0x22 = "
	0xffff,	  // 0x23 = #
	0xaabb,	  // 0x24 = $
	0xee99,	  // 0x25 = %
	0x663a,	  // 0x26 = &
	0x0400,	  // 0x27 = '
	0x1400,	  // 0x28 = (
	0x4100,	  // 0x29 = )
	0xff00,	  // 0x2a = *
	0xaa00,	  // 0x2b = +
	0x4000,	  // 0x2c = ,
	0x8800,	  // 0x2d = -
	0xa060,	  // 0x2e = .
	0x4400,	  // 0x2f = /

	0x44ff,	  // 0x30 = 0
	0x000c,
	0x8877,
	0x083f,
	0x888c,
	0x90b3,
	0x88fb,
	0x000f,
	0x88ff,
	0x88bf,

	0x2200,	  // 0x3a = :
	0x4200,	  // 0x3b = ;
	0x1400,	  // 0x3c = <
	0x8830,	  // 0x3d = =
	0x4100,	  // 0x3e = >
	0x2806,	  // 0x3f = ?

	0x0af7,	  // 0x40 = @
	0x88cf,	  // 0x41 = A
	0x2a3f,
	0x00f3,
	0x223f,
	0x88f3,
	0x80c3,
	0x08fb,
	0x88cc,
	0x2200,
	0x007c,
	0x94c0,
	0x00f0,
	0x05cc,
	0x11cc,
	0x00ff,
	0x88c7,
	0x10ff,
	0x98c7,
	0x88bb,
	0x2203,
	0x00fc,
	0x44c0,
	0x50cc,
	0x5500,
	0x2500,
	0x4433,	  // Z

	0x2212,	  // 0x5b = [
	0x1100,	  // 0x5c = '\' 
	0x2221,	  // 0x5d = ]
	0x4406,	  // 0x5e = ^
	0x0030,	  // 0x5f = _

	0x0100,	  // 0x60 = `
	0x88cf,	  // 0x61 = a (A)
	0x2a3f,
	0x00f3,
	0x223f,
	0x88f3,
	0x80c3,
	0x08fb,
	0x88cc,
	0x2200,
	0x007c,
	0x94c0,
	0x00f0,
	0x05cc,
	0x11cc,
	0x00ff,
	0x88c7,
	0x10ff,
	0x98c7,
	0x88bb,
	0x2203,
	0x00fc,
	0x44c0,
	0x50cc,
	0x5500,
	0x2500,
	0x4433,	  // z (Z)

	0xa212,	  // 0x7b = {
	0x2200,	  // 0x7c = |
	0x2a21,	  // 0x7d = }
	0x8844,	  // 0x7e = ~
	0x28ff	  // 0x7f = 
};


