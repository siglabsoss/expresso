/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/** @file ImageMove.cpp */
/**
 * \page ImgMove_page PCIe DMA ImageMove Demo
 *
 * The ImageMove demo (see ImageMove.cpp) demonstrates the Lattice
 *  PCIe IP core and the SGDMA IP core
 * operating on a PCIe Eval Board. It transfers image data from the
 * PC to the Eval Board and then back to the software, which then
 * displays a modified image on the screen.
 *
 *\verbatim
  
    |------------------|
    | Image Filter     |
    | Function         |         |----------------|
    | (XOR pixles)     |<--------| GPIO: mask reg |
    |__________________|         |________________|
        ^         |                             ^
        |         V                             |
     |-----------------|                        |
     | SGDMA IP Core   |<-------------+         |
     |_________________|              |         |
        ^         |                   |         |
        |         V                   |         |
     |-----------------|              |         |
     |  PCIe IP Core   |              |         |
     |_________________|         |----------------|
        ^         |              |      Driver    |
        |         V              |________________|
      |-----|  |-------|              |         |
      | Src |  |  Dest |<--+          |         |
      |_____|  |_______|   |     |----------------|
         ^        |        |_____| Application    |
         |        V              |  (this file)   |
      |----------------|         |________________|
      | OpenGL Windows |
      |________________|
  
  \endverbatim
 *
 * The image is displayed using OpenGL calls. The display rate is therefore
 * also dependent on the OpenGL library and graphics subsystem hardware.
 * Displaying an image is a quick way to illustrate that data has been moved.
 * An image provides a quick, visual way to observe a large transfer of data.
 * <p>
 * Each image is 256KB in size. 
 * <ol>
 * <li> The image source is generated by rotating the triangle shape using
 * OpenGL transform matrix. The resulting image is displayed on the screen.
 * <li> The source image is read from the screen into the source buffer.
 * <li> The source buffer is sent to the Image Filter memory on the Eval Board.
 * The memory is only 64KB in size, so the the image is sent in 4 steps.
 * <li> After a 64KB chunk is transfered to the board, the 64KB chunk is
 * read back, with the pixels modified by the XOR function in the read path.
 * <li> After 4 write/read cycles the desitination buffer contains the modified
 * image and it is displayed on the screen.
 * </ol>
 *
 *
 * Running:
 * <ul>
 * <li>[Esc] - terminates program (or close the window)
 * <li>[F1] - XOR filter set to 0xcc33aa55 (changes resulting display)
 * <li>[F2] - XOR filter set to 0xf0f0f0f0 (changes resulting display)
 * <li>[F3] - XOR filter set to 0x0f0f0f0f (changes resulting display)
 * <li>[F4] - XOR filter set to 0x00000000 (No transform applied to image)
 * <li>[Space] - Pause image movement (usefule to inspect transformed image)
 * </ul>
 *
 * The Application's printf's can go to a console window for diagnostic output.
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


#define IMAGE_FRAME_SIZE (256 * 256 * 4)
#define RUN_DMA 1
#define PAUSE 2


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

ULONG LoopCnt;
ULONG ElpsCnt;
clock_t StartTime;

int SrcWindow_ID;
int DstWindow_ID;
int Window_Width = 256;
int Window_Height = 256;

ULONG *pSrcImgBuf, *pDstImgBuf; 

LSCDMA_IF *pDrvr;

const char* SrcWindowTitle = "LatticeSGDMA: Src Img";
const char* DstWindowTitle = "LatticeSGDMA: Dest Img";

ULONG PixelTransformMask = 0xcc33aa55;

ULONG OpMode = RUN_DMA;


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
	// Allocate 2 buffers to hold the source and destination image buffer.
	// The buffers are 256 x 256 pixels.  Each pixel is 4 bytes, so get 256KB total.
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
	pSrcImgBuf = (ULONG *)AllocAlignedBuffer(IMAGE_FRAME_SIZE);
	if (pSrcImgBuf == NULL)
	{
		ERRORSTR("ERROR! AllocAlignedBuffer failed.  exiting\n");
		exit(-1);
	}

	pDstImgBuf = (ULONG *)AllocAlignedBuffer(IMAGE_FRAME_SIZE);
	if (pDstImgBuf == NULL)
	{
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		ERRORSTR("ERROR! AllocAlignedBuffer failed.  exiting\n");
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
		DEBUGPRINT(("Opening LSCDMA_IF...ImageMove\n"));
		pDrvr = new LSCDMA_IF(boardName, 
						  "_IM",    // function = ImageMove
						  boardNum);
	}
	catch (std::exception &e)
	{
		ERRORSTR("\nError opening driver: ");
		ERRORSTR(e.what());
		ShowLastError();
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);
		printf("Error opening lscdma ImageMove driver.\n");
		exit(-1);
	}


	//=========================================================
	// Verify access to the driver and resources.
	//=========================================================
	const DMAResourceInfo_t *pLinkInfo;
	if (pDrvr->getDriverDMAInfo(&pLinkInfo) == false)
	{
		printf("Error accessing driver.\n");
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);
		exit(-1);
	}




	//===============================================================
	// Verify the IP is accessable over the PCIe/Wishbone bus
	//===============================================================
	regVal = pDrvr->read32(memGPIO(0x00));
	if ((regVal & 0xffff0000) != 0x12040000)
	{
		ERRORSTR("\nError accessing GPIO ID register.");
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);
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

	//------------- DESTINATION IMAGE WINDOW -------------
	glutInitWindowPosition(400, 200);
	glutInitWindowSize(Window_Width, Window_Height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	sprintf(title, "%s :  %d FPS", DstWindowTitle, ElpsCnt);
	DstWindow_ID = glutCreateWindow(title);
	if (DstWindow_ID == GL_FALSE)
	{
		printf("Failed to create Destination Window!\n");
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);
		exit(-1);
	}

	//------------- SOURCE IMAGE WINDOW -------------
	// Set display and drawing mode
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(Window_Width, Window_Height);

	// Open a window 
	sprintf(title, "%s :  %d FPS", SrcWindowTitle, ElpsCnt);
	SrcWindow_ID = glutCreateWindow(title);
	if (SrcWindow_ID == GL_FALSE)
	{
		printf("Failed to create Source Window!\n");
		FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
		FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);
		exit(-1);
	}


	// Assign all the controls to the source window
	// Cursor has to be in Source Window to change pixel mask and exit
	glutSetWindow(SrcWindow_ID);


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
	printf("\n\nLattice PCIe DMA Image Move Demo\n");
	printf("<Esc>|Q|X - terminates program and closes the window\n");
	printf("<F1> - XOR filter set to 0xcc33aa55 (changes resulting display).\n");
	printf("<F2> - XOR filter set to 0xf0f0f0f0 (changes resulting display).\n");
	printf("<F3> - XOR filter set to 0x0f0f0f0f (changes resulting display).\n");
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

	glutDestroyWindow(SrcWindow_ID);
	glutDestroyWindow(DstWindow_ID);


	/* Close all LSC_PCIe objects i.e. driver interface */
	delete pDrvr;
	FreeAlignedBuffer(pSrcImgBuf, IMAGE_FRAME_SIZE);
	FreeAlignedBuffer(pDstImgBuf, IMAGE_FRAME_SIZE);

	LEAVE();    

      	exit(0);
}


/**
 * OpenGL Draw Callback.
 * This routine does the actual drawing.
 */
void cbDraw(void)
{
	char buf[256];
	clock_t c1;
	double elpstime;
	static float theta = 0.0f;
	ULONG *pSrc, *pDst;
	ULONG i, j;


	//-------------------------------------------------
	// FIRST: draw to the Source window the original image
	//-------------------------------------------------

	glutSetWindow(SrcWindow_ID);

	// Clear the color buffer.
   	glClear(GL_COLOR_BUFFER_BIT);


	glPushMatrix();
	glRotatef(theta, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f, 1.0f);
		glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f, -0.5f);
		glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);
	glEnd();
	glPopMatrix ();

	// All done drawing.  Let's show it.
	glFlush();
	glutSwapBuffers();


	//------------------------------------------------------------------------
	// SECOND: Copy the drawn image from the Source window to a memory buffer
	//------------------------------------------------------------------------
	glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pSrcImgBuf);



	//------------------------------------------------------------------------
	// THIRD: Move the source image buffer through the EBR64 and modify
	// the pixel values on the way back out to the destination buffer
	//------------------------------------------------------------------------
#if 1
	// Hardware moves the image

	//---------------------------------------------------
	// Write to the Pixel Mask register in the GPIO block.
	// This 32 bit value is XORed with each pixel when its
	// read back out of the EBR64 memory, thus changing the
	// colors of the pixels between source image and
	// destination image.
	//---------------------------------------------------
	pDrvr->write32(memGPIO(0x28), PixelTransformMask);

	//---------------------------------------------------
	// Have to do the write/read in 4 stages cause the
	// EBR memory on the Eval Board is only 64KB in size
	// and can't hold an entire image frame at once.
	//---------------------------------------------------
	pSrc = pSrcImgBuf;
	pDst = pDstImgBuf;
	for (i = 0; i < 4; i++)
	{
		//-----------------------------------------------------------------------
		// Send 64KB of the source image to the Image Filter Memory on the board
		//-----------------------------------------------------------------------
		if (pDrvr->WriteToDevice(pSrc, IMAGE_FRAME_SIZE/4) == false)
		{
			ERRORSTR("DEVICE WRITE ERROR!\nExiting.\n");
			cleanUpAndExit();
		}

		//-----------------------------------------------------------------------
		// Read back 64KB of the modified image into the destination buffer
		//-----------------------------------------------------------------------
		if (pDrvr->ReadFromDevice(pDst, IMAGE_FRAME_SIZE/4) == false)
		{
			ERRORSTR("DEVICE READ ERROR!\nExiting.\n");
			cleanUpAndExit();
		}

		pSrc = pSrc + ((IMAGE_FRAME_SIZE/4) / sizeof(ULONG));
		pDst = pDst + ((IMAGE_FRAME_SIZE/4) / sizeof(ULONG));

	}


#else // software moves pixels from source buffer to destination buffer
	pSrc = pSrcImgBuf;
	pDst = pDstImgBuf;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{
			*pDst = *pSrc ^ PixelTransformMask;
			++pDst;
			++pSrc;
		}
	}
#endif	 


	//---------------------------------------------------------
	// FOURTH: draw the changed image to the Destination window 
	//---------------------------------------------------------

	glutSetWindow(DstWindow_ID);

	glDrawPixels(256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pDstImgBuf);

	// All done drawing.  Let's show it.
	glFlush();
	glutSwapBuffers();

	theta += 1.0f;

	++LoopCnt;
	++ElpsCnt;

//TODO
#if 0
	if ((LoopCnt % 100) == 0)
	{
		printf(".");
		fflush(stdout);
	}
#endif

	//------------------------------------------------------------
	// Time how many frames are drawn per second.
	// This is a relative benchmark for DMA performance when compared
	// to software or baseline
	//------------------------------------------------------------
	c1 = clock();
	elpstime = (double)(c1 - StartTime)/(CLOCKS_PER_SEC );

	if (elpstime >= 1.0)
	{
		sprintf(buf, "%s: %d FPS", DstWindowTitle, ElpsCnt);
		glutSetWindowTitle(buf);
		ElpsCnt = 0;
		StartTime = c1;
	}

	glutSetWindow(SrcWindow_ID);
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

		case 0x20: // SPACE
			if (OpMode == PAUSE)
				OpMode = RUN_DMA;
			else
				OpMode = PAUSE;
			break;
			

		case 'd':
		case 'D':
			glutPostRedisplay();  // for debugging can step 1 frame per key
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
			PixelTransformMask = 0xcc33aa55;
			break;

		case GLUT_KEY_F2:
			PixelTransformMask = 0xf0f0f0f0;
			break;

		case GLUT_KEY_F3:
			PixelTransformMask = 0x0f0f0f0f;
			break;

		case GLUT_KEY_F4:
			PixelTransformMask = 0x00000000;
			break;


   		default:
      			//printf ("SKP: No action for %d.\n", key);
			break;
	}

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
	gluOrtho2D(-1, 1, -1, 1);

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

	// Enables Smooth Color Shading
	glShadeModel(GL_SMOOTH);

	// Load up the correct perspective matrix; using a callback directly.
	cbReshape(Width, Height);
}


