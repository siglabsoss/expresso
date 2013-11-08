#!/bin/bash
#                     setup
#   (For setting up and using the Lattice PCIe API Library Environment)
#
#   Installation batch program to setup the correct environment variables
#   for the PCIeAPI_Lib.  These environment variables are used by the
#   makefiles to locate include files and DLLs needed when
#   building and running applications that use the PCIeAPI_Lib.
#   This file must be run from the top of the PCIeAPI_Lib directory.
#
#   USAGE:  setup
#   displays values of required environment variables that need to be
#   entered at the shell building will be done from.
#
#   Optionally user can run make devel from top level to permanently
#   set these in .bashrc
#
#   Environment Variables:
#   LSC_PCIEAPI_DIR = full path to the PCIeAPI library directory tree
#   LD_LIBRARY_PATH = path to PCIeAPI library for running the demos
#
# ===========================================================================


#============================================================================
#  Get the path values for the environment variables
#============================================================================
LSC_PCIEAPI_DIR=`pwd`/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`

echo "PCIeAPI_Lib Environment Variables"
echo "copy and paste the following lines at shell prompt:"
echo "export LSC_PCIEAPI_DIR=$LSC_PCIEAPI_DIR"
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

