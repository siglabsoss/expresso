/*  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

/** @file Cpp_Jni.cpp */

/**
 * \page GUI_page PCIe Thruput Demo GUI Description 
 * Java JNI to C++ Interface.
 * The Cpp_Jni.cpp file bridges the Java GUI to the C++ classes that implement the PCIe
 * driver interface and device IP modules.  The Java GUI simply provides a Graphical
 * interface to the user.  The Java GUI does not (can not) directly access the PCIe
 * driver APIs.  Java however can make calls to C functions if they are properly
 * declared in the Java source as Java Native Interfaces (JNI) - interfaces to the
 * native language of the Java platform, which is C/C++.
 * To invoke the functions provided by the PCIeAPI library,
 * the Java GUI elements invoke methods, such as runEBRTest(), which are tagged as
 * JNI functions and cause the JVM to
 * search for and invoke the C function found in a loaded DLL.  The Cpp_Jni.cpp file
 * is part of the DLL loaded when the Java GUI starts.  Cpp_Jni provides the bridge
 * between the Java JNI functions and the PCIeAPI library and driver interfaces.
 * <p>
 * The following diagram depicts the flow of execution to invoke driver methods.
 *
 \verbatim
   Java====>JNI/JVM===>DLL(this file)===>APImethod()===>driver
 \endverbatim
 *
 * The file and functions are in C++ so they can access the standard PCIeAPI library
 * methods and driver interface classes which are all in C++.  These are the exact
 * same classes and methods used by the Text Menu program.
 * <p>
 * The Java GUI code resides in the directory <B>SFIF_GUI/DemoUI</B>.  The main Java
 * class is buried under the <b>src/com/.../DemoUI.java</B> path.  It contains the JNI
 * declarations of the functions that need to be implemented here.  A sampling of
 * these JNI declarations follows:
 
 \verbatim
    //==========================================================
    //   JNI Declarations
    //==========================================================
    private native String CPP_getVersionStr();
    private native String CPP_getDrvrResourceStr();
    private native String CPP_getPCIResourceStr();
    private native String CPP_startSFIF(int RunMode);
    private native String CPP_stopSFIF();
 \endverbatim
 *
 * The header file that describes these JNI interface functions in terms of C/C++
 * is auto-generated using the <B>javah</B> utility run in the
 * dist/ directory containing the jar file.  See the Makefile in Cpp_DLL for
 * details of invoking the <B>javah</B> utility
 * See Java JNI tutorial for a description of how the function prototypes
 * are generated and variables passed between Java and C/C++.
 * <p>
 * After running the <b>javah</b> utility, a set of header files are generated.
 * This source code includes the master file that defines the function signature
 * for all the JNI functions declared in the Java source code.  These functions
 * are implemented here.  They have mangled names, and do not match exactly the
 * ones invoked in the Java source.  See the comments of each function for its
 * real name.
 * <p>
 * This file, along with <b>dllmain.cpp</b> are compiled into a DLL.  The DLL can
 * be loaded by the Java Virtual Machine (JVM) when it encounters the Java system
 * call to load a library <B>System.loadLibrary("Cpp_Jni"); </B>
 * Upon loading by the OS, the DllMain() function in dllmain.cpp is invoked first.
 * The function is invoked with a DLL_PROCESS_ATTACH code, which causes the 
 * <B>createSFIFDemo()</B> function to be called that constructs the driver interafce
 * classes and all the device object classes.  This is the exact same attach and 
 * construct sequence used in the Text Menu code.
 * The instances of the classes are global variables so they can be accessed in 
 * each of the JNI method implementations.
 * <p>
 * The code development flow:
 * <ol>
 * <li> Java Source:
 *    <ol>
 *    <li> declare JNI function signatures in Java GUI source
 *    <li> make a system call to load the DLL containing the JNI implementations
 *    <li> use the JNI functions like any other Java method
 *    </ol>
 * <li> Create C++ function signatures of the JNI methods using Java's javah utility
 * <li> Implement these functions (this file)
 * <li> Compile and link into a DLL
 * </ol>
 *
 * The execution flow:
 * <ol>
 * <li> Java GUI is run like any Java class/program
 * <li> At the start of running, a system call is made to load the Cpp_Jni.dll
 * <li> JVM/OS loads the DLL
 * <li> The DllMain() entry is invoked and calls functions to create all the classes
 * <li> There-after Java GUI code calls JNI methods to control the hardware and 
 * thread execution flows into the functions implemented here
 * <li> When the Java GUI exits, the deleteSFIFDemo() function is invoked by the DLL
 * and all objects are destroyed and the driver is closed.
 * </ol>
 * <p>
 * A simulation mode is also provided if the device ID is SIM.  In this case
 * all hardware accesses are simulated with memory arrays contained here, so
 * reads and writes will work, but nothing real will happen.
 * <p>
 * * Note: This is one possible method to perform the bridge between Java and
 * C. A network server could have been used on the C side and Java could have
 * sent/recvd commands via TCP on a socket.  CORBA could also have been used.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>
using namespace std;

// Auto-generated with Java utility javah
#include "com_latticesemi_lpa_apps_sfif_SFIF_UI.h"

#include "PCIeAPI.h"         // the APIs
#include "LSCPCIe2_IF.h"     // the driver interface
#include "GPIO.h"            // the GPIO IP module
#include "SFIF.h"            // the SFIF IP module
#include "MiscUtils.h"
#include "MemFormat.h"
#include "../../MemMap.h"       // IP module base addresses
#include "DebugPrint.h"



//////////////////////////////////////////////
// Required to use Lattice PCIe methods
//////////////////////////////////////////////
using namespace LatticeSemi_PCIe;



/*===== GLOBAL VARIABLES ========*/

LSCPCIe2_IF *pDrvr;
SFIF        *pSFIF;
GPIO        *pGPIO;

uint32_t DmaBufAddr = 0;

uint32_t WriteDmaBuf[FIFO_SIZE/4];
uint32_t ReadDmaBuf[FIFO_SIZE/4];



bool SimHdw = false;



/*================= SIMULATION USE ONLY ===============*/
#include "SimStrings.h"


int Sim_runMode; 
int Sim_trafficMode; 
int Sim_cycles; 
int Sim_ICG; 
int Sim_rdTLPSize; 
int Sim_wrTLPSize; 
int Sim_numRdTLPs; 
int Sim_numWrTLPs;

long Sim_ElapseTime;
long Sim_TxTLPCntr;
long Sim_RxTLPCntr;
long Sim_WrWaitTime;
long Sim_LastCplDTime;
long Sim_RdWaitTime;


bool    Sim_SFIFisRunning = false;

// Buffers Used in Simulation
uint32_t PC_DMA_Buf[16384/4];
uint32_t SFIF_Rx_FIFO[16384/4];

uint32_t PC_DMA_Buf_val = 0;



/*=========== FUNCTION PROTOTYPES =========*/
bool Sim_stopSFIF();
bool Sim_startSFIF(uint32_t mode);
int Sim_getSFIFRegs(long long *elements, int len);
bool Sim_setupSFIF(int runMode, 
				   int trafficMode, 
				   int cycles, 
				   int ICG, 
				   int rdTLPSize, 
				   int wrTLPSize, 
				   int numRdTLPs, 
				   int numWrTLPs);

void Sim_getSFIFParseRxFIFO(string &retStr);
void Sim_getPCSystemBuffer(string &retStr);
void Sim_clrPCSystemBuffer(string &retStr);

bool Sim_formatBlockOfWords(string &ostr, uint32_t startAddr, uint32_t len, uint32_t *data);

int Sim_PC_DMA_Buf_val = 0;

/**
 * Called by DLL ATTACH_PROCESS to setup all demo objects.
 * This implements what normally is done at the top of main().
 */
bool createSFIFDemo(void)
{
 	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
 	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
 	uint32_t boardNum;


	ENTER();

	DEBUGSTR("Creating the SFIF Demo\n");
	SimHdw = false;


	DEBUGSTR("Instantiate the PCIeAPI DLL class\n");
	PCIeAPI theDLL;
	DEBUGPRINT(("PCIeAPI_Lib version info: %s\n", theDLL.getVersionStr()));

#ifndef RELEASE
	// Setup so lots of diag output occurs
	PCIeAPI::setRunTimeCtrl(PCIeAPI::VERBOSE);
	
#endif

	// Get the environment variables needed to open the desired board
	// Pre-load with defaults
	strcpy(boardName, "SIM");
	strcpy(demoName, "subsys_30301204");
	if (!GetPCIeEnvVars(boardName, demoName, boardNum))
	    cout<<"\nEnvVars not found.  Using defaults.\n";

	cout<<"boardName = "<<boardName<<endl;
	cout<<"boardNum = "<<boardNum<<endl;
	cout<<"demoName = "<<demoName<<endl;
    
	try
	{

		// Find out what board the user expects to run on
		if (boardName == NULL)
			strcpy(boardName, "SIM");

		if (strcmp(boardName, "SIM") == 0)
		{
			DEBUGSTR("Using Simualtion Mode!  No Real Hardware Access!\n");

			pDrvr = NULL;
			pSFIF = NULL;
			pGPIO = NULL;
			SimHdw = true;
		}
		else
		{
		    cout<<"Opening LSCPCIe2_IF...\n";
		    pDrvr = new LSCPCIe2_IF(boardName, 
					    demoName, 
				            boardNum);

			// Create Device instances used in the demo
			
			pGPIO = new GPIO("GPIO",         // a unique name for the IP module instance
					  memGPIO(0),    // its base address
					  pDrvr);        // driver interface to use for register access
			pSFIF = new SFIF("SFIF",         // a unique name for the IP module instance
					  memSFIF(0),    // its base address
				          pDrvr);        // driver interface to use for register access

		}

	}
	catch (std::exception &e)
	{
		DEBUGSTR("RUNTIME ERROR!!!...\n");
		DEBUGSTRNL(e.what());
		return(false);
	}
	return(true);
}


/**
 * Called when DLL is exiting to cleanup and shutdown all demo and driver objects.
 * This implements what normally is done at the end of main().
 */
bool deleteSFIFDemo(void)
{
	ENTER();

	if (!SimHdw)
	{
		if (pDrvr && pSFIF && pGPIO)
		{
			// Ensure the SFIF is totally stopped before exiting
			pSFIF->disableSFIF();

			// delete the device objects open to the device driver
			delete pSFIF;
			delete pGPIO;
			delete pDrvr;

			pSFIF = NULL;
			pGPIO = NULL;
			pDrvr = NULL;
		}
	}
	return(true);
}


/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*
 *   IMPLEMENT THE JNI METHODS TO ACCESS AND CONTROL THE FPGA VIA API CLASS
 *
 *   NOTE: Function signatures are created by JNI!  Must follow proper
 *   Java JNI protocol for variable usage and passing.
 */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/
/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getVersionStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getVersionStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
	{
		return(env->NewStringUTF(Sim_DriverVersionStr));         
	}
	else
	{
		pDrvr->getDriverVersionStr(retStr);

		return(env->NewStringUTF(retStr.c_str()));         
	}
}



/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getDrvrResourceStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getDrvrResourceStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
		return(env->NewStringUTF(Sim_DriverResourcesStr));

	if (pDrvr->getDriverResourcesStr(retStr))
		return(env->NewStringUTF(retStr.c_str()));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getPCIResourceStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getPCIResourceStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
		return(env->NewStringUTF(Sim_PCIResourceStr));

	if (pDrvr->getPCIResourcesStr(retStr))
		return(env->NewStringUTF(retStr.c_str()));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getCfgRegsStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getCfgRegsStr(JNIEnv *env, jobject obj)
{
	string regVals, regFields, retStr;
	uint8_t cfgs[256];

	ENTER();

	if (SimHdw)
		return(env->NewStringUTF(Sim_CfgRegStr));


	if (pDrvr->getPCIResourcesStr(regFields))
	{
		pDrvr->getPCIConfigRegs(cfgs);
		// Call routine to format
		MemFormat::formatBlockOfBytes(regVals, 0x00, 0x40, cfgs);
		retStr = regVals + regFields;
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		return(env->NewStringUTF("ERROR"));         
	}
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getCapRegStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getCapRegStr(JNIEnv *env, jobject obj)
{
	string regVals, regFields, retStr;
	uint8_t caps[256];

	ENTER();

	if (SimHdw)
		return(env->NewStringUTF(Sim_CapRegStr));

	if (pDrvr->getPCICapabiltiesStr(regFields))
	{
		pDrvr->getPCIConfigRegs(caps);
		// Call routine to format the memory bytes into a table format in a string
		MemFormat::formatBlockOfBytes(retStr, 0x40, (0x100-0x40), &caps[0x40]);
		retStr = regVals + regFields;
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		return(env->NewStringUTF("ERROR"));         
	}


}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getExtraInfoStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getExtraInfoStr(JNIEnv *env, jobject obj)
{
	string retStr, outs;

	ENTER();

	if (SimHdw)
		return(env->NewStringUTF(Sim_ExtrInfoStr));


	if (pDrvr->getPCIExtraInfoStr(retStr))
	{
	    std::ostringstream oss;

	    oss<<"\nRoot Complex Initial Credits:\n";
	    oss<<"PD_CA (Wr): "<<((pGPIO->read32(0x24))>>16)<<"\n";
	    oss<<"NPH_CA(Rd): "<<((pGPIO->read32(0x24)) & 0xffff)<<"\n";

	    outs = oss.str();

	    retStr.append(outs);
	    return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
	    return(env->NewStringUTF("ERROR"));         
	}
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getPCSysBufStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getPCSysBufStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
	{
		Sim_getPCSystemBuffer(retStr);
	}
	else
	{
	    uint32_t *buf;

	    buf = (uint32_t *)malloc(SYS_DMA_BUF_SIZE);
	    if (buf == NULL)
	    {
		retStr.append("Memory Allocation Error!");
	    }
	    else
	    {
		pDrvr->readSysDmaBuf(buf, SYS_DMA_BUF_SIZE);
		MemFormat::formatBlockOfWords(retStr, DmaBufAddr, SYS_DMA_BUF_SIZE / 4, buf);
		free(buf);
	    }

	}

	return(env->NewStringUTF(retStr.c_str()));         
}



/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_clrPCSysBuf
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1clrPCSysBuf(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
	{
		Sim_clrPCSystemBuffer(retStr);
	}
	else
	{
	    uint32_t *buf;

	    buf = (uint32_t *)malloc(SYS_DMA_BUF_SIZE);
	    if (buf == NULL)
	    {
		    retStr = "Memory Allocation Error!";
	    }
	    else
	    {
		memset(buf, 0, SYS_DMA_BUF_SIZE);
		pDrvr->writeSysDmaBuf(buf, SYS_DMA_BUF_SIZE);

		free(buf);
	    }
	}
	return(env->NewStringUTF(retStr.c_str()));         
}



/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getSFIFRxFIFOStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getSFIFRxFIFOStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (SimHdw)
		Sim_getSFIFParseRxFIFO(retStr);
	else
		pSFIF->getSFIFParseRxFIFO(retStr);

	return(env->NewStringUTF(retStr.c_str()));         
}



/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_setupSFIF
 * Signature: (IIIIIIII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1setupSFIF(JNIEnv *env, 
																					 jobject obj, 
																					 jint runMode, 
																					 jint trafficMode, 
																					 jint cycles, 
																					 jint ICG, 
																					 jint rdTLPSize, 
																					 jint wrTLPSize, 
																					 jint numRdTLPs, 
																					 jint numWrTLPs)
{
	bool ok;

	ENTER();

	if (SimHdw)
	{
		ok = Sim_setupSFIF(runMode, trafficMode, cycles, ICG, rdTLPSize, wrTLPSize, numRdTLPs, numWrTLPs);
	}
	else
	{
		ok = pSFIF->setupSFIF(runMode, trafficMode, cycles, ICG, rdTLPSize, wrTLPSize, numRdTLPs, numWrTLPs);
	}

	if (ok)
		return(env->NewStringUTF("OK"));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_getSFIFRegsArray
 * Signature: ([J)I
 */
JNIEXPORT jint JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1getSFIFRegsArray (JNIEnv *env, 
																						  jobject obj,
																						  jlongArray jla)
{
	int nRead;

	ENTER();


	jsize len = env->GetArrayLength(jla);

	jlong *elements = env->GetLongArrayElements(jla, 0);

	if (SimHdw)
		nRead = Sim_getSFIFRegs((long long *)elements, len);
	else
		nRead = pSFIF->getSFIFRegs((long long *)elements, len);

	env->ReleaseLongArrayElements(jla, elements, 0);

	return(nRead);
}



/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_startSFIF
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1startSFIF(JNIEnv *env, jobject obj, jint mode)
{
	bool ok;

	ENTER();

	if (SimHdw)
		ok = Sim_startSFIF((uint32_t)mode);
	else
		ok = pSFIF->startSFIF((uint32_t)mode);

	if (ok)
		return(env->NewStringUTF("OK"));
	else
		return(env->NewStringUTF("ERROR"));         
}

/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_stopSFIF
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1stopSFIF(JNIEnv *env, jobject obj)
{
	bool ok;

	ENTER();

	if (SimHdw)
		ok = Sim_stopSFIF();
	else
		ok = pSFIF->stopSFIF();

	if (ok)
		return(env->NewStringUTF("OK"));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_sfif_SFIF_UI
 * Method:    CPP_testGPIOAccess
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_latticesemi_lpa_apps_sfif_SFIF_1UI_CPP_1testGPIOAccess(JNIEnv *env, jobject obj)
{

	ENTER();

	if (SimHdw)
		return(0);

	int i = 10;	// fudge factor to get it to run for 100 msec

	uint32_t led = pGPIO->read32(0x08);  // don't have a getLED method yet
	int err = 0;
	while (i)
	{
		// do a burst of 100 GPIO accesses to PCIe board
		for (uint32_t jj = 0; jj < 100; jj++)
		{
			// write Scratch Pad
			pGPIO->setScratchPad(jj);
			// Read GPIO ID register and verify
			if ((pGPIO->getID() != GPIO_ID_VALID) && (pGPIO->getID() != OLD_GPIO_ID_VALID))
				++err;
			// read scratch pad and verify
			if (pGPIO->getScratchPad() != jj)
				++err;
		}

		if ((i % 10) == 0)	// every 100msec show some activity
		{
			led = (led<<1) & 0xffff;
			if (led == 0)
				led = 1;
			pGPIO->setLED16Display((uint16_t)led);
		}

		Sleep(1);
		--i;
	}


	return(err);	

}




/*==================================================================*/
/*==================================================================*/
// SIMULATION FUNCTIONS RUN WHEN HARDWARE NOT PRESENT
// For GUI testing mostly or for demo eval when no board avail
/*==================================================================*/
/*==================================================================*/

void Sim_clrPCSystemBuffer(string &retStr)
{
	ENTER();

	memset(PC_DMA_Buf, 0, 16384);
	retStr.append("OK");

}


void Sim_getPCSystemBuffer(string &retStr)
{
	ENTER();

	// retStr = "PC DMA buffer contents:  0000";
	Sim_formatBlockOfWords(retStr, 0x0000, 16384/4, PC_DMA_Buf);
}


void Sim_getSFIFParseRxFIFO(string &retStr)
{
	ENTER();

	Sim_formatBlockOfWords(retStr, 0x0000, 16384, SFIF_Rx_FIFO);

}


bool Sim_setupSFIF(int runMode, 
				   int trafficMode, 
				   int cycles, 
				   int ICG, 
				   int rdTLPSize, 
				   int wrTLPSize, 
				   int numRdTLPs, 
				   int numWrTLPs)
{

	ENTER();

	if (cycles < 1)
		cycles = 1;
	else if (cycles > 0xffff)
		cycles = 0xffff;

	if (ICG < 1)
		ICG = 1;
	else if (ICG > 0xffff)
		ICG = 0xffff;

	Sim_runMode = runMode; 
	Sim_trafficMode = trafficMode; 
	Sim_cycles = cycles; 
	Sim_ICG = ICG; 
	Sim_rdTLPSize = rdTLPSize; 
	Sim_wrTLPSize = wrTLPSize; 
	Sim_numRdTLPs = numRdTLPs; 
	Sim_numWrTLPs = numWrTLPs;


	++Sim_PC_DMA_Buf_val;
	memset(PC_DMA_Buf, PC_DMA_Buf_val, 16384);

	return(true);

}



/**
 * This returns the SFIF counter statistics.
 * They are only valid if running so startSFIF() should
 * have been called.  The returned values are made up.
 */
int Sim_getSFIFRegs(long long *elements, int len)
{

	ENTER();

	if (Sim_SFIFisRunning)
	{
		Sim_stopSFIF();	 // have to fake stop to update counters
		Sim_SFIFisRunning = true;
	}
	elements[0] = (long long)Sim_ElapseTime;
	elements[1] = (long long)Sim_TxTLPCntr;
	elements[2] = (long long)Sim_RxTLPCntr;
	elements[3] = (long long)Sim_WrWaitTime;
	elements[4] = (long long)Sim_LastCplDTime;
	elements[5] = (long long)Sim_RdWaitTime;


	return(6);

}

bool Sim_startSFIF(uint32_t mode)
{
	ENTER();
	// First reset all the stats counters for this run
	// We can't just let them go continuously because they're 32 bit and 
	// can wrap after only a minute
	Sim_ElapseTime = 0;
	Sim_TxTLPCntr = 0;
	Sim_RxTLPCntr = 0;
	Sim_WrWaitTime = 0;
	Sim_LastCplDTime = 0;
	Sim_RdWaitTime = 0;

	Sim_SFIFisRunning = true;

	return(true);
}


#define TX_FUDGE 4

bool Sim_stopSFIF()
{
	int CplDs;
	int CpldSize;
	uint32_t MWr_clks, MRd_clks,MRdWait_clks;
	float WrTLPSize;
	float RdTLPSize;
	float ICG;
	float numWrTLPs;
	float numRdTLPs;
	float rate;
	float Rd_factor, ICG_factor;
	float frand;


	ENTER();

	Sim_SFIFisRunning = false;

	// Calculate the new counter values now based on the traffic that was to be sent
	// The run-time parameters are from the setupSFIF().


	// This is all bogus, made-up guestimates of a totally fictious system.
	// it is not meant to emulate any real hardware performance, just to
	// produce some numbers.

	if (Sim_runMode == 0)
	{
		ICG = (float)Sim_ICG;
		WrTLPSize = (float)Sim_wrTLPSize;
		RdTLPSize = (float)Sim_rdTLPSize;
		numRdTLPs = (float)Sim_numRdTLPs ;
		numWrTLPs = (float)Sim_numWrTLPs ;

		if (Sim_rdTLPSize < 64)
		{
			CplDs = 1;
			CpldSize = Sim_rdTLPSize;
		}
		else
		{
			CplDs = Sim_rdTLPSize/64;
			CpldSize = 64;
		}

		frand = ((float)(5 - (rand() % 10)) * 0.01)  + 1.0;
		//frand = 1.0;
		cout<<"frand="<<frand<<endl;


		// Thruput mode. it ran for 1 second
		Sim_ElapseTime = (long)(125E6 * frand);	  // 125MHz clock counting for 1 second

		if (Sim_numRdTLPs > 0)
			Sim_LastCplDTime = (long)(125E6 * frand);	// 125MHz clock counting for 1 second
		else
			Sim_LastCplDTime = 0;

		if (ICG > 0.0)
		{
			ICG_factor = (0.5 * (ICG / ((WrTLPSize/8.0) * numWrTLPs)));
			if (ICG_factor > 0.1)
				ICG_factor = 0.1;
		}
		else
		{
			ICG_factor = 0.0;
		}

		if ((WrTLPSize != 0.0) && (numWrTLPs != 0.0))
		{
			Rd_factor = 0.1 * (numRdTLPs / (WrTLPSize / 8.0));
			if (Rd_factor > 0.1)
				Rd_factor = 0.1;
		}
		else
		{
			Rd_factor = 0.1;
		}

		if (Sim_numRdTLPs > 0)
		{
			rate = 600.0 * (0.2 + (RdTLPSize / 512.0) - ICG_factor - (0.1 - Rd_factor));
			if (rate > 750.0)
				rate = 750.0;
			Sim_RxTLPCntr = (long)(rate * 1E6 / CpldSize + 0.5);
			Sim_TxTLPCntr = (Sim_RxTLPCntr / 8);
			Sim_RdWaitTime = ((unsigned int)rand() % 5000) * (Sim_numRdTLPs * Sim_rdTLPSize);
		}
		else
		{
			rate = 0.0;
			Sim_TxTLPCntr = 0;
		}

		if (numWrTLPs > 0.0)
		{
			rate = 650.0 * (0.2 + (WrTLPSize / 128.0) - ICG_factor - Rd_factor);
			if (rate > 820.0)
				rate = 820.0;
			Sim_TxTLPCntr = (long)(rate * 1E6 / WrTLPSize + 0.5);
			Sim_WrWaitTime = ((unsigned int)rand() % 500) * (Sim_numWrTLPs * Sim_wrTLPSize);
		}
		else
		{
			Sim_WrWaitTime = 0;
		}

	}
	else
	{


		MWr_clks = Sim_numWrTLPs * (TX_FUDGE + 2 + Sim_wrTLPSize/8);
		MRd_clks = Sim_numRdTLPs * (TX_FUDGE + 2);
		MRdWait_clks = (Sim_numRdTLPs / 16) * 100;	// assume have to wait after 16 tags have been sent
		Sim_ElapseTime = (long)(MWr_clks  + MRd_clks + MRdWait_clks + Sim_ICG) * Sim_cycles;

		Sim_TxTLPCntr = (Sim_numWrTLPs + Sim_numRdTLPs) * Sim_cycles;
		if (Sim_rdTLPSize < 64)
		{
			CplDs = 1;
			CpldSize = Sim_rdTLPSize;
		}
		else
		{
			CplDs = Sim_rdTLPSize/64;
			CpldSize = 64;
		}

		if (Sim_numRdTLPs)
			Sim_LastCplDTime = Sim_ElapseTime + (long)(CplDs * Sim_numRdTLPs * 10) * Sim_cycles;
		else
			Sim_LastCplDTime =  0;
		Sim_RxTLPCntr = (CplDs * Sim_numRdTLPs) * Sim_cycles;  // based on RCB=64

		Sim_WrWaitTime = Sim_cycles;  // assume hardly no waiting (always reads atleast 1)
		Sim_RdWaitTime = (long)((double)Sim_ElapseTime * ((double)Sim_numRdTLPs/32.0) + 0.5);

	}

	return(true);
}


/**
 *
 *  0000:            0x55667788
 *  0010: 0xa0b0c0d0 0xaabbccdd 0x11223344
 */
bool Sim_formatBlockOfWords(string &ostr, uint32_t startAddr, uint32_t len, uint32_t *data)
{
	uint32_t i, j;
	uint32_t addr;
	char *blank;
	char buf[256];

	if (len == 1)
	{
		sprintf(buf, "%08x: %08x\n", startAddr, *data);
		ostr.append(buf);
		return(true);
	}

	if (len > 2)
	{
		ostr.append("\n");	/* can't fit all on one line so start its own block */
		blank = "          ";
	}
	else
	{
		blank = "";
	}

	addr = (startAddr & 0xfffffff0);
	len = len + (startAddr - addr) / 4;

	j = 0;
	for (i = 0; i < len; i++)
	{
		if ((i % 4) == 0)
		{
			if (i > 0)
				sprintf(buf, "\n%08x:", addr);
			else
				sprintf(buf, "%08x:", startAddr);
			ostr.append(buf);
		}
		else if ((i % 2) == 0)
		{
			ostr.append("  ");	// divide output into 2 cols
		}

		if (addr < startAddr)
		{
			ostr.append(blank);
		}
		else
		{
			sprintf(buf, "  %08x", data[j]);
			ostr.append(buf);
			++j;
		}

		addr = addr + 4;
	}

	ostr.append("\n");

	return(true);
}



