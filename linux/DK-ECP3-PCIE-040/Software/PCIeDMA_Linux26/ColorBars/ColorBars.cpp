/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file ColorBars.cpp */
/**
 * \page ColorBars_page PCIe DMA ColorBar Demo
 *
 * The ColorBars demo (see ColorBars.cpp) demonstrates the Lattice 
 * PCIe IP core and the SGDMA IP core
 * operating on a PCIe Eval Board. It transfers image data from the
 * Eval Board to PC memory and software then displays it on the screen.
 * The image source is a block of IP operating as a FIFO.  The IP tracks
 * how many reads have been requested, and after 8 complete rows have been
 * read, it changes the color data provided with the next 8 rows.
 *
 * \verbatim
  
    |------------------|
    | Image Data       |
    | Generator        |
    | (ColorBars)      |
    |__________________|
             |
             V
     |-----------------|
     | SGDMA IP Core   |<-------------+
     |_________________|              |
             |                        |
             V                        |
     |-----------------|              |
     |  PCIe IP Core   |              |
     |_________________|         |----------------|
             |                   | Driver         |
             V                   |________________|
      |----------------|              ^
      | Image Buffer   |<--+          |
      |________________|   |     |----------------|
             |             |_____| Application    |
             V                   |  (this file)   |
      |----------------|         |________________|
      | OpenGL Window  |
      |________________|
  
   \endverbatim
 *
 * The image is displayed using OpenGL calls. The display rate is therefore
 * also dependent on the OpenGL library and graphics subsystem hardware.
 * Displaying an image is a quick way to illustrate that data has been moved.
 * It would not be practical to display 1MB to the screen in a text dump,
 * or save it to a file. An image provides a quick, visual way to observe a
 * large transfer of data, and it can run continuously.
 * <p>
 *
 * The image data is 1MB in size.  Each DMA Read request is 1MB in size.
 * After the hardware has transferred the pixel data, the API call returns
 * and the software displays the image.  This loop is repeated over and over.
 * The data rate (frame rate) is displayed in the window title bar.  The frame
 * rate is roughly the throughput rate in MB/sec. (each frame = 1 MB).
 * Frame rate is governed by the video refresh rate.  Most video systems will
 * not draw frames into video memory faster than the frame rate (waste of
 * operations).
 * <p>
 *
 * Running:
 * <ul>
 * <li>[Esc] - terminates program and closes the window
 * <li>[F1] - just draw blank image buffer (don't generate data) - fastest rate
 * <li>[F2] - generate color bar data via software, should be slowest data rate
 * <li>[F3] - get image data from eval board via DMA transfer
 * <li>[F4] - Draw a frame each second (slowly) so it can be viewed and the changes seen
 * <li>[Space] - pause/resume image transfer
 * </ul>
 * 
 * <p>
 * The main structure of this application is based on OpenGL and glut (GL Utilities
 * Libraries).
 * Printf's go to a console.
 * The graphics window needs to have focus to accept commands.
 *
 */


#include <cstdlib>
#include <iostream>
#include <string> 

#include <stdio.h>   // Always a good idea.
#include <string.h>   // Always a good idea.
#include <stdlib.h>   // Always a good idea.
#include <time.h>    // For our FPS stats.

#include <GL/glut.h> // GLUT support library.

using namespace std;

#include "PCIeAPI.h"
#include "DebugPrint.h"
#include "MiscUtils.h"
#include "LSCDMA_IF.h"
#include "GPIO.h"
#include "../MemMap.h"   // definition of all IP devices in this design

using namespace LatticeSemi_PCIe;



#define CB_IMAGE_FRAME_SIZE (1024 * 1024)

#define BASE_RATE 1
#define SOFTWARE_RATE 2
#define DMA_RATE 3
#define PAUSE 4


//----------------------------
// Function Declarations
//----------------------------

void cleanUpAndExit(void);
void cbDraw(void);
void cbKeyPressed( unsigned char key, int x, int y);
void cbSpecialKeyPressed( int key, int x, int y);
void cbReshape( int Width, int Height);
void setupDrawing(int Width, int Height);


//----------------------------
// Global Variables
//----------------------------

ULONG WaitTime = 0;
ULONG OpMode = DMA_RATE;
ULONG PrevMode = DMA_RATE;
ULONG PixelVal = 0x00000000;

ULONG *pImgBuf;      
ULONG LoopCnt;
ULONG ElpsCnt;
clock_t StartTime;

int Window_ID;
int Window_Width = 512;
int Window_Height = 512;


LSCDMA_IF *pDrvr;

const char* WindowTitle = "LatticeSGDMA: ColorBars";

const char *ModeName[5] = {"", "BASE_Rate", "SW_Rate", "DMA_Rate", "Paused"};

//----------------------------------------------------------
// Pixel data used for drawing image in software mode.
// Emulate the color data read from hardware.
//----------------------------------------------------------
//
// Pixel = [Alpha][Blue][Green][Red]
//
// PixelVal = 0xff000000;  // alpha on
// PixelVal = 0x00ff0000;  // blue on
// PixelVal = 0x0000ff00;  // green on
// PixelVal = 0x000000ff;  // red on

ULONG Color[64] =
{ 
	0x000000ff,	 // 0
	0x00000ef3,
	0x00001ce7,
	0x000028db,
	0x000034cf,
	0x000040c3,
	0x00004cb7,
	0x000058ab,
	0x0000649f,
	0x00007093,
	0x00007c87,
	0x0000887b,
	0x0000946f,
	0x0000a063,
	0x0000ac57,
	0x0000b84b,
	0x0000c43f,	  // 16
	0x0000d033,
	0x0000dc27,
	0x0000e81b,
	0x0000f40f,
	0x0000ff00,
	0x000ef300,
	0x001ce700,
	0x0028db00,
	0x0034cf00,
	0x0040c300,
	0x004cb700,
	0x0058ab00,
	0x00649f00,
	0x00709300,
	0x007c8700,
	0x00887b00,	  // 32
	0x00946f00,
	0x00a06300,
	0x00ac5700,
	0x00b84b00,
	0x00c43f00,
	0x00d03300,
	0x00dc2700,
	0x00e81b00,
	0x00f40f00,
	0x00ff0300,
	0x00ff0000,
	0x00f3000e,
	0x00e7001c,
	0x00db0028,
	0x00cf0034,
	0x00c30040,	  // 48
	0x00b7004c,
	0x00ab0058,
	0x009f0064,
	0x00930070,
	0x0087007c,
	0x007b0088,
	0x006f0094,
	0x006300a0,
	0x005700ac,
	0x004b00b8,
	0x003f00c4,
	0x003300d0,
	0x002700dc,
	0x001b00e8,
	0x000f00f4	 // 63
};


/**
 * Main Entry Point.
 * Creates the OpenGL window, opens the DMA driver, starts reads from the
 * ColorBar IP module and displays the pixel data in the OpenGL window.
 */
int main( int argc, char **argv)
{

	char title[256];


	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
	uint32_t boardNum;
	string infoStr;
	ULONG regVal;

	ENTER();

	//--------------------------------------------------------------------
	// Create an instance of the Lattice PCIeAPI DLL to ensure library is
	// loaded and initialized and get the and display the version info
	// for diagnostic purposes.
	//--------------------------------------------------------------------
	PCIeAPI theDLL;
	DEBUGPRINT(("Dll version info: %s\n", theDLL.getVersionStr()));


	//--------------------------------------------------------------------
	// Allocate a buffer to hold the ColorBar image buffer.  This buffer is 
	// 512 x 512 pixels.  Each pixel is 4 bytes, so get 1MB total.
	// Use the PCIeAPI function to allocate a page (or 64KB) aligned buffer.
	// This is nice because the start of the buffer will be the start of a
	// page so all BDs setup in the SGDMA will map to complete virtual pages.
	// NOTE: buffers not allocated using this have the potential to cause
	// longer transfer times because fragmented pages could result.  The
	// SGDMA has 256 BDs which is just enough to hold a SG list of 4KB pages
	// to describe an entire 1MB buffer.  If partial pages are used (exceeding
	// 256 maximum), then multiple SG list setups will be needed, increasing
	// setup and xfer time.
	//--------------------------------------------------------------------
	pImgBuf = (ULONG *)AllocAlignedBuffer(CB_IMAGE_FRAME_SIZE);

	if (pImgBuf == NULL)
	{
		ERRORSTR("ERROR! VirtualAlloc failed.  exiting\n");
		exit(-1);
	}

	LoopCnt = 0;

	//--------------------------------------------------------------------
	// Get the environment variables needed to open the desired eval board.
	// These are setup by the scan utility program or defined at the command
	// line. Pre-load with defaults in case env vars are not found.
	//--------------------------------------------------------------------
	strcpy(boardName, "ECP3");
	strcpy(demoName, "DMA");
	if (!GetPCIeEnvVars(boardName, demoName, boardNum))
		DEBUGSTR("\nEnvVars not found.  Using defaults.\n");

	DEBUGPRINT(("boardName = %s", boardName));
	DEBUGPRINT(("  boardNum = %d", boardNum));
	DEBUGPRINT(("  demoName = %s\n", demoName));



	//--------------------------------------------------------------------
	// Open the Lattice DMA driver to the board.
	// An exception will be thrown if any problems occur finding or 
	// accessing the hardware.
	//--------------------------------------------------------------------
	try
	{
		DEBUGPRINT(("Opening LSCDMA_IF...ColorBars\n"));
		pDrvr = new LSCDMA_IF(boardName, 
							  "_CB",    // function = ColorBars
							  boardNum);
	}
	catch (std::exception &e)
	{
		ERRORSTR("\nError opening driver: ");
		ERRORSTR(e.what());
		ShowLastError();
		FreeAlignedBuffer(pImgBuf, CB_IMAGE_FRAME_SIZE);
		printf("Error opening lscdma ColorBars driver.\n");
		exit(-1);
	}


	//=========================================================
	// Verify access to the driver and resources.
	//=========================================================
	const DMAResourceInfo_t *pLinkInfo;
	if (pDrvr->getDriverDMAInfo(&pLinkInfo) == false)
	{
	    printf("Error accessing driver.\n");
	    exit(-1);
	}




	//===============================================================
	// Verify the IP is accessable over the PCIe/Wishbone bus
	//===============================================================
	regVal = pDrvr->read32(memGPIO(0x00));
	if ((regVal & 0xffff0000) != 0x12040000)
	{
		ERRORSTR("\nError accessing GPIO ID register.");
		FreeAlignedBuffer(pImgBuf, CB_IMAGE_FRAME_SIZE);
		printf("Error accessing IP registers.\n");
		exit(-1);
	}


	//===============================================================
	// Reset the ColorBar Pixel Data Generator
	//===============================================================
	pDrvr->write32(memGPIO(0x30), 1);
	pDrvr->write32(memGPIO(0x30), 0);


	//===============================================================
	// Create the OpenGL Window.
	//===============================================================
	glutInit(&argc, argv);

	// Set display and drawing mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(Window_Width, Window_Height);

	// Open a window 
	sprintf(title, "%s : %s   %d FPS", WindowTitle, ModeName[OpMode], ElpsCnt);
	Window_ID = glutCreateWindow(title);

	// Register the callback function to do the drawing. 
	glutDisplayFunc(&cbDraw);

	// If there's nothing to do, draw.
	glutIdleFunc(&cbDraw);

	// It's a good idea to know when our window's resized.
	glutReshapeFunc(&cbReshape);

	// And let's get some keyboard input.
	glutKeyboardFunc(&cbKeyPressed);
	glutSpecialFunc(&cbSpecialKeyPressed);

	// OpenGL's ready to go.  Call init function to setup colors and matrixes.
	setupDrawing(Window_Width, Window_Height);

	// Print out a bit of help dialog.
	printf("\n\n%s\n", WindowTitle);
	printf("<Esc>|Q|X - terminates program and closes the window\n");
	printf("<F1> - just draw blank image buffer (don't generate data) - fastest rate.\n");
	printf("<F2> - generate color bar data via software, should be slowest data rate.\n");
	printf("<F3> - get image data from eval board via DMA transfer.\n");
	printf("<F4> - Draw a frame each second (slowly) so it can be viewed and the changes seen.\n");
	printf("<Space> - pause/resume image transfer.\n");
	printf("OpenGL window must have focus to accept input.\n");

	StartTime = clock();

	// Pass off control to OpenGL.
	// OpenGL will invoke the callbacks when needed to Draw, process KeyPress, etc.
	glutMainLoop();


	// Never get here because glutMainLoop never returns.
	return(0);   
}



/**
 * This cleans-up all instantiated objects, closes GL and
 * eventually exits the program.
 * It's everything that would normally be done at the end of main()
 * but it needs to be called explicitly from an OpenGL callback
 * once its detected that the user wants to exit.
 */
void	cleanUpAndExit(void)
{
	ENTER();    
	//-----------------------------------------------------
	// Exiting and shutting down
	//-----------------------------------------------------

	glutDestroyWindow(Window_ID);


	/* Close all LSC_PCIe objects i.e. driver interface */
	delete pDrvr;
	FreeAlignedBuffer(pImgBuf, CB_IMAGE_FRAME_SIZE);

	LEAVE();    

      	exit(0);
}

/**
 * OpenGL Draw Callback.
 * This routine does the actual drawing.
 * 
 */
void cbDraw(void)
{
	char buf[256];
	clock_t c1;
	double elpstime;

	ULONG *pPixel;      
	ULONG i, j;


	// Clear the color and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// A B G R
	// PixelVal = 0xff000000;   // alpha on
	// PixelVal = 0x00ff0000;  // blue
	// PixelVal = 0x0000ff00;  // green
	// PixelVal = 0x000000ff;  // red

	switch (OpMode)
	{
		case SOFTWARE_RATE:	 // create image to draw in software
			//------------------------------------------------------------
			// Manually load the image buffer to display the color bar
			// pattern.  This shows how long it takes brute-force software
			// to accomplish the task beign performed in hardware and DMA.
			//------------------------------------------------------------
			for (i = 0; i < 512; i++)
			{
				PixelVal = Color[(((i>>3) + LoopCnt) & 0x3f)];
				pPixel = pImgBuf + (i * 512);
				for (j = 0; j < 512; j++)
				{
					*pPixel = PixelVal;
					++pPixel;
				}
			}
			break;

		case DMA_RATE:	// Get image data via DMA
			//------------------------------------------------------------
			// Issue a Windows Read system call that causes the driver to
			// setup the SGDMA IP to write (PCIe MWr TLPs) data read from
			// the ColorBar generator and store into the allocated buffer
			// for later display on the screen.
			//------------------------------------------------------------
#if 1
			if (pDrvr->ReadFromDevice(pImgBuf, CB_IMAGE_FRAME_SIZE) == false)
			{
				ERRORSTR("DMA READ ERROR!\nExiting.\n");
				exit(-1);
			}
#endif
			break;


		case BASE_RATE:	 // just update the image Window ASAP
			//------------------------------------------------------------
			// Do nothing (basically).
			// The first time we clear the image to all black, then do
			// nothing in subsequent draw times. This shows the fastest
			// draw time of the OpenGL library and graphics hardware.
			// this is the benchmark the DMA xfer is measured against.
			//------------------------------------------------------------
			if (LoopCnt == 0)
				memset(pImgBuf, 0, CB_IMAGE_FRAME_SIZE);
			break;

		case PAUSE:	 // don't do any transfers or image displays
			Sleep(10);
			break;


	}

	++LoopCnt;
	++ElpsCnt;

	// Option to Slow down the display rate so its easy to see each frame drawn
	if (WaitTime)
	    Sleep(WaitTime);

	//------------------------------------------------------------
	// Draw the image buffer to the screen.
	// Use OpenGL to transfer the 1MB pixel data to the image window.
	// Transfer and draw times are completely system dependent, based
	// on OpenGL library, graphics card hardware, motherboard design,
	// etc.
	//------------------------------------------------------------
	if (OpMode != PAUSE)
	{
	    glDrawPixels(512, 512, GL_RGBA, GL_UNSIGNED_BYTE, pImgBuf);

	    // All done drawing.  Display it.
	    glutSwapBuffers();
	}
	else
	{
	    ElpsCnt = 0;  // Paused so no frames per second
	}




	//------------------------------------------------------------
	// Time how many frames are drawn per second.
	// This is a relative benchmark for DMA performance when compared
	// to software or baseline
	//------------------------------------------------------------
	c1 = clock();
	elpstime = (double)(c1 - StartTime)/(CLOCKS_PER_SEC );

	if (WaitTime)
		elpstime = 1.1f;   // problem is that sleeping kills the process time keeper

	if (elpstime >= 1.0)
	{
		sprintf(buf, "%s : %s   %d FPS", WindowTitle, ModeName[OpMode], ElpsCnt);
		glutSetWindowTitle(buf);
		ElpsCnt = 0;
		StartTime = c1;
	}


}



/**
 * Process a user Key Press.
 * OpenGL calls this back when the user presses a normal key on the keyboard.
 * The Window must have focus.
 */
void cbKeyPressed( unsigned char key, int x, int y)
{
	char buf[256];

   	switch (key) 
	{
      		case 'X': 
      		case 'x': 
      		case 'Q': 
		case 'q': 
		case 27: // Escape
			cleanUpAndExit();
      			break; // exit doesn't return, but anyway...


		case ' ':  // SPACE
			if (OpMode == PAUSE)
			{
				OpMode = PrevMode;
				LoopCnt = 0;
				StartTime = clock();
			}
			else
			{
			    	PrevMode = OpMode;
			    	OpMode = PAUSE;
				StartTime = clock();
			}
			// Return window to previous title
			sprintf(buf, "%s : %s", WindowTitle, ModeName[OpMode]);
			glutSetWindowTitle(buf);
			break;

   		default:
      			//printf ("KP: No action for %d.\n", key);
			break;
	}
}



/**
 * Process a user Function Key Press.
 * OpenGL calls this back when the user presses function and arrow keys on the keyboard (non-ASCII).
 * The Window must have focus.
 */
void cbSpecialKeyPressed( int key, int x, int y)
{
	char buf[256];

	switch (key) 
	{

		case GLUT_KEY_F1:
			WaitTime = 0;
			OpMode = BASE_RATE;
			LoopCnt = 0;
			break;

		case GLUT_KEY_F2:
			WaitTime = 0;
			OpMode = SOFTWARE_RATE;
			LoopCnt = 0;
			break;

		case GLUT_KEY_F3:
			WaitTime = 0;
			OpMode = DMA_RATE;
			LoopCnt = 0;
			break;

		case GLUT_KEY_F4:
			WaitTime = 1001;
			break;

   		default:
      			//printf ("SKP: No action for %d.\n", key);
			break;
	}

	sprintf(buf, "%s : %s", WindowTitle, ModeName[OpMode]);
	glutSetWindowTitle(buf);
}


/**
 * Callback routine to recompute the window size if it gets re-sized by
 * the user.
 * Called by OpenGL. Returns to OpenGL.
 */
void cbReshape( int Width, int Height)
{
	// Make sure denominator is never 0
	if (Height == 0)
		Height = 1;

	glViewport(0, 0, Width, Height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);

	Window_Width  = Width;
	Window_Height = Height;
}


/**
 * Initialize the redering parameters for the window after OpenGL library
 * has been initialized, but before entering the OpenGL control loop.
 */
void setupDrawing(int Width, int Height) 
{

	// Color to clear color buffer to.
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// Depth to clear depth buffer to; type of test.
	glClearDepth(1.0);
	glDepthFunc(GL_LESS); 

	// Enables Smooth Color Shading; try GL_FLAT for (lack of) fun.
	glShadeModel(GL_SMOOTH);

	// Load up the correct perspective matrix; using a callback directly.
	cbReshape(Width, Height);
}



