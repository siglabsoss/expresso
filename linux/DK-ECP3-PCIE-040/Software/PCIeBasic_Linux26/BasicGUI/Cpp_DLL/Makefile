#==============================================================================
# This makefile would be used to build tests, applications, etc. where a
# single C file, test.c, having main(), possibly calls functions in other
# pre-compiled object files.
#
# The build tools, flags, include paths, etc. are taken from the PCIeAPI
# library so LSC_PCIEAPI_DIR environment variable needs to be defined to
# locate the include files and dll for linking.
#
#===============================================================================


#----------------------------------------------------
# build the DLL using its makefile
# This is for release, no debug info, optimized
#----------------------------------------------------
dll:
	$(MAKE) -f Makefile.dll OPTIONS='-fPIC -O3 -DRELEASE'

#----------------------------------------------------
# build the DLL for debugging  !!! NOT FOR RELEASE !!!
#----------------------------------------------------
dll_debug:
	$(MAKE) -f Makefile.dll OPTIONS='-fPIC -g'


#------------------------------------------------------------------------
# Clean-up all .o files in the directory
#------------------------------------------------------------------------
clean :
	$(MAKE) -f Makefile.dll  clean


