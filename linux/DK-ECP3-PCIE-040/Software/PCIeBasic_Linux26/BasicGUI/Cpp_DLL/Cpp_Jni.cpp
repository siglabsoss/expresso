/*
 *  COPYRIGHT (c) 2008 by Lattice Semiconductor Corporation
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */
/**  @file Cpp_Jni.cpp */ 

/**
 *
 * \page GUI_page PCIe Basic Demo GUI Description 
 *
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
 * The Java GUI code resides in the directory <B>BasicGUI/DemoUI</B>.  The main Java
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
    private native String CPP_runEBRTest();
    private native String CPP_readEBRMem(String cmd);
    private native String CPP_writeEBRMem(String cmd);
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
 * <B>createBasicDemo()</B> function to be called that constructs the driver interafce
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
 * <li> When the Java GUI exits, the deleteBasicDemo() function is invoked by the DLL
 * and all objects are destroyed and the driver is closed.
 * </ol>
 * <p>
 * Note: This is one possible method to perform the bridge between Java and
 * C. A network server could have been used on the C side and Java could have
 * sent/recvd commands via TCP on a socket.  CORBA could also have been used.
 * 
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
#include "com_latticesemi_lpa_apps_pcie_DemoUI.h"
#include "PCIeAPI.h"         // the APIs
#include "LSCPCIe2_IF.h"     // the driver interface
#include "GPIO.h"            // the GPIO IP module
#include "Mem.h"            // the SFIF IP module
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
Mem         *pEBR;
Mem         *pBAR0;
Mem         *pBAR1;
GPIO        *pGPIO;


bool SimHdw = false;


static char HexChar[16] = {'0', '1', '2', '3', '4', '5', '6', '7', 
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


/**
 * Called by DLL ATTACH_PROCESS to setup all demo objects.
 * This implements what normally is done at the top of main().
 */
bool createBasicDemo(void)
{
 	char boardName[LSC_PCIE_BOARD_NAME_LEN + 1];
 	char demoName[LSC_PCIE_DEMO_NAME_LEN + 1];
 	uint32_t boardNum;


	ENTER();

	DEBUGSTR("Creating the PCIe Basic Demo\n");


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
	strcpy(demoName, "subsys_30101204");
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
			DEBUGSTR("Using View-Only Mode!  No Real Hardware Access!\n");

			pDrvr = NULL;
			pEBR = NULL;
			pBAR0 = NULL;
			pBAR1 = NULL;
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
					  GPIOreg(0),    // its base address
					  pDrvr);        // driver interface to use for register access
			pEBR = new Mem("EBR16k",         // a unique name for the IP module instance
				          EBR_SIZE,
					  EBRreg(0),    // its base address
				          pDrvr);        // driver interface to use for register access

			pBAR0 = new Mem("BAR0",         // a unique name for the IP module instance
				         BAR0_SIZE,
					  BAR0reg(0),    // its base address
				          pDrvr);        // driver interface to use for register access

			pBAR1 = new Mem("BAR1",         // a unique name for the IP module instance
				         BAR1_SIZE,
					  BAR1reg(0),    // its base address
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
bool deleteBasicDemo(void)
{
	ENTER();

	if (!SimHdw)
	{
		if (pDrvr && pBAR0 && pBAR1 && pEBR && pGPIO)
		{
			// delete the device objects open to the device driver
			delete pEBR;
			delete pBAR0;
			delete pBAR1;
			delete pGPIO;
			delete pDrvr;

			pEBR = NULL;
			pBAR0 = NULL;
			pBAR1 = NULL;
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
 *   procedures for variable usage and passing.
 */
/*==========================================================================*/
/*==========================================================================*/
/*==========================================================================*/

/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getVersionStr
 * Signature: ()Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getVersionStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (pDrvr->getDriverVersionStr(retStr))
	    return(env->NewStringUTF(retStr.c_str()));         
	else
		return(env->NewStringUTF("ERROR"));         
}

/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getDrvrResourceStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getDrvrResourceStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (pDrvr->getDriverResourcesStr(retStr))
		return(env->NewStringUTF(retStr.c_str()));
	else
		return(env->NewStringUTF("ERROR"));         
}



/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getPCIResourceStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getPCIResourceStr(JNIEnv *env, jobject obj)
{
	string retStr;

	ENTER();

	if (pDrvr->getPCIResourcesStr(retStr))
		return(env->NewStringUTF(retStr.c_str()));
	else
		return(env->NewStringUTF("ERROR"));         
}



/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getCfgRegsStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getCfgRegsStr(JNIEnv *env, jobject obj)
{
	string regVals, regFields, retStr;
	uint8_t cfgs[256];

	ENTER();

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
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getCapRegStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getCapRegStr(JNIEnv *env, jobject obj)
{
	string regVals, regFields, retStr;
	uint8_t caps[256];

	ENTER();

	if (pDrvr->getPCICapabiltiesStr(regFields))
	{
		pDrvr->getPCIConfigRegs(caps);
		// Call routine to format the memory bytes into a table format in a string
		MemFormat::formatBlockOfBytes(regVals, 0x40, (0x100-0x40), &caps[0x40]);
		retStr = regVals + regFields;
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		return(env->NewStringUTF("ERROR"));         
	}


}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getExtCapRegsStr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getExtCapRegsStr(JNIEnv *env, jobject obj)
{
	char retStr[4096];

	ENTER();
	strcpy(retStr, "NOT IMPLEMENTED: PCIe Extended Capabilities Registers");

	return(env->NewStringUTF(retStr));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_runEBRTest
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1runEBRTest(JNIEnv *env, jobject obj)
{
	ENTER();

	if (pEBR->test())
		return(env->NewStringUTF("PASS"));
	else
		return(env->NewStringUTF("ERRORS!!!"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_readBAR0Mem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1readBAR0Mem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pBAR0, BAR0_SIZE);   // max range

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}

/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_writeBAR0Mem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1writeBAR0Mem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pBAR0, BAR0_SIZE);   // max range

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}

/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_readBAR1Mem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1readBAR1Mem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pBAR1, BAR1_SIZE);   // max range to read

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}

/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_writeBAR1Mem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1writeBAR1Mem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pBAR1, BAR1_SIZE);   // max range to read

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}

  


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_readEBRMem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1readEBRMem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pEBR, EBR_SIZE);   // max range to read

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_writeEBRMem
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1writeEBRMem(JNIEnv *env, jobject obj, jstring cmd)
{
	string retStr;
	const char *str;

	ENTER();

	MemFormat RWMem(pEBR, EBR_SIZE);   // max range to read

	str = env->GetStringUTFChars(cmd, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Cmd: "<<str<<endl;
	DEBUGSTR(str);

	if (RWMem.doCmd((char *)str, retStr))
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF(retStr.c_str()));         
	}
	else
	{
		env->ReleaseStringUTFChars(cmd, str); 
		return(env->NewStringUTF("ERROR"));         
	}
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_clearEBRMem
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1clearEBRMem(JNIEnv *env, jobject obj)
{
	ENTER();

	if (pEBR->clear())
		return(env->NewStringUTF("OK"));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_fillEBRMem
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1fillEBRMem(JNIEnv *env, jobject obj, jint val)
{
	ENTER();

	if (pEBR->fill((uint8_t)val))
		return(env->NewStringUTF("OK"));
	else
		return(env->NewStringUTF("ERROR"));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_loadEBRFromFile
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1loadEBRFromFile(JNIEnv *env, jobject obj, jstring fname)
{
	const char *str;

	ENTER();

	str = env->GetStringUTFChars(fname, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<"Filename: "<<str<<endl;
	DEBUGSTR(str);

	if (pEBR->loadFromFile((char *)str))
	{
		env->ReleaseStringUTFChars(fname, str); 
		return(env->NewStringUTF("OK"));         
	}
	else
	{
		env->ReleaseStringUTFChars(fname, str); 
		return(env->NewStringUTF("ERROR"));         
	}

}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_saveEBRToFile
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1saveEBRToFile(JNIEnv *env, jobject obj, jstring fname)
{
	const char *str;

	ENTER();

	str = env->GetStringUTFChars(fname, NULL);
	if (str == NULL)
	{
		return(NULL);  /* OutofMemory Error */
	}

	//cout<<" Filename: "<<str<<endl;
	DEBUGSTR(str);

	if (pEBR->saveToFile((char *)str))
	{
		env->ReleaseStringUTFChars(fname, str); 
		return(env->NewStringUTF("OK"));         
	}
	else
	{
		env->ReleaseStringUTFChars(fname, str); 
		return(env->NewStringUTF("ERROR"));         
	}

}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_runLEDTest
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1runLEDTest(JNIEnv *env, jobject obj)
{
	ENTER();

	pGPIO->LED16DisplayTest();
	return(env->NewStringUTF("OK"));
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_setLED
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1setLED(JNIEnv *env, jobject obj, jint val)
{
	ENTER();

	pGPIO->setLED16Display((char)val);
	return(env->NewStringUTF("OK"));
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_runLED16SegTest
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1runLED16SegTest(JNIEnv *env, jobject obj)
{
	ENTER();

	pGPIO->LED16DisplayTest();
	return(env->NewStringUTF("OK"));
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_setLED16Seg
 * Signature: (C)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1setLED16Seg__C(JNIEnv *env, jobject obj, jchar val)
{
	ENTER();

	pGPIO->setLED16Display((char)val);
	return(env->NewStringUTF("OK"));

}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_setLED16Seg
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1setLED16Seg__I(JNIEnv *env, jobject obj, jint val)
{
	ENTER();

	pGPIO->setLED16Display((unsigned short)val);
	return(env->NewStringUTF("OK"));

}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getLED16Seg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getLED16Seg(JNIEnv *env, jobject obj)
{
	uint8_t lo, hi;
	char retStr[8];

	ENTER();

	// The 16 Segment LED display is at registers 8 and 9 in BAR1
	lo = pGPIO->read8(0x08);
	hi = pGPIO->read8(0x09);

	// Quick in-place conversion from 4 digit hex value to 4 char string
	retStr[0] = HexChar[(hi>>4) & 0x0f];
	retStr[1] = HexChar[hi & 0x0f];
	retStr[2] = HexChar[(lo>>4) & 0x0f];
	retStr[3] = HexChar[lo & 0x0f];
	retStr[4] = '\0';
	return(env->NewStringUTF(retStr));         
}



/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getDIPsw
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getDIPsw(JNIEnv *env, jobject obj)
{
	char retStr[4];
	uint8_t val;

	ENTER();

	val = pGPIO->getDIPsw();
	
	retStr[0] = HexChar[val>>4];
	retStr[1] = HexChar[val & 0x0f];
	retStr[2] = '\0';
	return(env->NewStringUTF(retStr));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_runCounter
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1runCounter(JNIEnv *env, jobject obj, jint mode)
{
	ENTER();

	if (mode == 1)
	{
		pGPIO->runCounter(true);
		return(env->NewStringUTF("OK"));
	}
	else if (mode == 0)
	{
		pGPIO->runCounter(false);
		return(env->NewStringUTF("OK"));
	}
	else
	{
		return(env->NewStringUTF("ERROR"));         
	}
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_getCounter
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1getCounter(JNIEnv *env, jobject obj)
{
	char retStr[12];
	uint32_t val;

	ENTER();

	val = pGPIO->getCounter();
	
	retStr[0] = HexChar[(val>>28) & 0x0f];
	retStr[1] = HexChar[(val>>24) & 0x0f];
	retStr[2] = HexChar[(val>>20) & 0x0f];
	retStr[3] = HexChar[(val>>16) & 0x0f];
	retStr[4] = HexChar[(val>>12) & 0x0f];
	retStr[5] = HexChar[(val>>8) & 0x0f];
	retStr[6] = HexChar[(val>>4) & 0x0f];
	retStr[7] = HexChar[val & 0x0f];
	retStr[8] = '\0';
	return(env->NewStringUTF(retStr));         
}


/*
 * Class:     com_latticesemi_lpa_apps_pcie_DemoUI
 * Method:    CPP_setReloadCount
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_latticesemi_lpa_apps_pcie_DemoUI_CPP_1setReloadCount(JNIEnv *env, jobject obj, jint cnt)
{
	ENTER();

	pGPIO->setCounterReload((uint32_t)cnt);
	return(env->NewStringUTF("OK"));
}

