#!/bin/sh
#-----------------------------------------------------------------------
# This script removes the demos.  It undoes everything done by the
# install.sh and installDrvr.sh scripts.
# It should restore the system to the state it was in before installing
# the drivers and links to demo files.
# It does not remove the DevKit directory.
# It only cleans-up settings and files placed in directories outside
# of the DevKit tree.
#-----------------------------------------------------------------------

echo "-------------------------------------------"
echo " Removing Lattice PCIe Demo Installation"
echo "-------------------------------------------"

#=======================================================================
# REMOVE THE DESKTOP ICONS AND LINKS
#=======================================================================
INSTALL_DIR=`pwd`

#------------------- BASIC DEMO -----------------------
DEMO_NAME=Basic
CMD=PCIe${DEMO_NAME}.sh
DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

rm ~/Desktop/.${CMD_FILE}
rm ~/Desktop/$DESKTOP_FILE 


#------------------- THRUPUT DEMO -----------------------
DEMO_NAME=Thruput
CMD=PCIe${DEMO_NAME}.sh
DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

rm ~/Desktop/.${CMD_FILE}
rm ~/Desktop/$DESKTOP_FILE 



#------------------- DMA COLORBARS DEMO -----------------------
DEMO_NAME=ColorBars
CMD=PCIe${DEMO_NAME}.sh
DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

rm ~/Desktop/.${CMD_FILE}
rm ~/Desktop/$DESKTOP_FILE 



#------------------- DMA IMAGEMOVE DEMO -----------------------
DEMO_NAME=ImageMove
CMD=PCIe${DEMO_NAME}.sh
DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

rm ~/Desktop/.${CMD_FILE}
rm ~/Desktop/$DESKTOP_FILE 


#=======================================================================
# UNINSTALL THE DRIVERS AND OTHER SYSTEM STARTUP FILES
# User must have root password to continue
#=======================================================================
echo "Removing driver files requires root password."
su -p -c ./removeDrvr.sh root


echo "Remove Completed."

