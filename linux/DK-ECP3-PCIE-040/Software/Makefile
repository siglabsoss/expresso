#=============================================================================
#              Top Level Lattice PCIe Linux DevKit Makefile
#
# This makefile compiles all the sub-projects to produce the drivers, 
# libraries and applications that implement the DevKit demonstrations.
# The main purpose is to descend the project tree and build all the 
# sub-projects so that one can invoke 'make' from a single place and
# rebuild all files for a particular system.
#
# Most likey, at a minimum, the user will need to rebuild the device
# drivers with: "make drivers"
#
# Once all drivers (and possibly demo applications) are rebuilt for
# the platform on which they are installed, run the install.sh script
# in the Demonstration/ directory to instally the drivers, update the
# system and create desktop links to the applications.
#
# For software development and experimentation with the demo source,
# it is advised to also run "make devel" to add environment variables
# to your .bashrc file.  These environment variables are used for
# header file location and library location during building and running.
#=============================================================================


LSC_PCIEAPI_DIR := $(shell pwd)/PCIeAPI_Lib_Linux26/
LD_LIBRARY_PATH := $(shell pwd)/PCIeAPI_Lib_Linux26/:$(LD_LIBRARY_PATH)
export LD_LIBRARY_PATH
export LSC_PCIEAPI_DIR

ifeq ($(JAVA_HOME),)
	JAVA_HOME := $(shell pwd)/../Demonstration/jre1.5.0_14
	export JAVA_HOME
endif

CP = cp

demos:
	$(MAKE) -f Makefile -C ./PCIeAPI_Lib_Linux26/
	$(CP) ./PCIeAPI_Lib_Linux26/liblscpcieapi.so ../Demonstration/Basic
	$(CP) ./PCIeAPI_Lib_Linux26/liblscpcieapi.so ../Demonstration/Thruput
	$(CP) ./PCIeAPI_Lib_Linux26/liblscpcieapi.so ../Demonstration/DMA
	$(MAKE) -f Makefile -C ./PCIeBasic_Linux26/BasicGUI/Cpp_DLL/
	$(CP) ./PCIeBasic_Linux26/BasicGUI/Cpp_DLL/libCpp_Jni.so ../Demonstration/Basic
	$(MAKE) -f Makefile -C ./PCIeSFIF_Linux26/SFIF_GUI/Cpp_DLL/
	$(CP) ./PCIeSFIF_Linux26/SFIF_GUI/Cpp_DLL/libCpp_Jni.so ../Demonstration/Thruput
	$(MAKE) -f Makefile -C ./PCIeDMA_Linux26/ColorBars/
	$(CP) ./PCIeDMA_Linux26/ColorBars/ColorBars ../Demonstration/DMA
	$(MAKE) -f Makefile -C ./PCIeDMA_Linux26/ImageMove/
	$(CP) ./PCIeDMA_Linux26/ImageMove/ImageMove ../Demonstration/DMA

drivers:
	$(MAKE) -f Makefile -C ./lscpcie2_Linux26/drvr/
	$(CP) ./lscpcie2_Linux26/drvr/lscpcie2.ko ../Demonstration/Thruput/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/9-lscpcie2.rules ../Demonstration/Thruput/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/insdrvr.sh ../Demonstration/Thruput/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/rmdrvr.sh ../Demonstration/Thruput/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/lscpcie2.ko ../Demonstration/Basic/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/9-lscpcie2.rules ../Demonstration/Basic/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/insdrvr.sh ../Demonstration/Basic/Driver/
	$(CP) ./lscpcie2_Linux26/drvr/rmdrvr.sh ../Demonstration/Basic/Driver/
	$(MAKE) -f Makefile -C ./lscdma_Linux26/drvr/
	$(CP) ./lscdma_Linux26/drvr/lscdma.ko ../Demonstration/DMA/Driver/
	$(CP) ./lscdma_Linux26/drvr/9-lscdma.rules ../Demonstration/DMA/Driver/
	$(CP) ./lscdma_Linux26/drvr/insdrvr.sh ../Demonstration/DMA/Driver/
	$(CP) ./lscdma_Linux26/drvr/rmdrvr.sh ../Demonstration/DMA/Driver/
	

devel:
	@ echo "Adding LSC_PCIEAPI_DIR and LD_LIBRARY_PATH to .bashrc"
	sed -e '/LSC_PCIEAPI_DIR/d' -e '/LD_LIBRARY_PATH/d' ~/.bashrc > bashrc.tmp
	@ echo "export LSC_PCIEAPI_DIR="$(LSC_PCIEAPI_DIR) >> bashrc.tmp
	@ echo "export LD_LIBRARY_PATH="$(LD_LIBRARY_PATH) >> bashrc.tmp
	mv bashrc.tmp ~/.bashrc

clean:
	$(MAKE) -f Makefile allclean -C ./PCIeAPI_Lib_Linux26/
	$(MAKE) -f Makefile clean -C ./PCIeBasic_Linux26/BasicGUI/Cpp_DLL/
	$(MAKE) -f Makefile clean -C ./PCIeSFIF_Linux26/SFIF_GUI/Cpp_DLL/
	$(MAKE) -f Makefile clean -C ./PCIeDMA_Linux26/ColorBars/
	$(MAKE) -f Makefile clean -C ./PCIeDMA_Linux26/ImageMove/
	$(MAKE) -f Makefile clean -C ./lscpcie2_Linux26/drvr/
	$(MAKE) -f Makefile clean -C ./lscdma_Linux26/drvr/


