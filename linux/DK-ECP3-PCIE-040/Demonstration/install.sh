#!/bin/sh
#-----------------------------------------------------------------------
# This script installs links to the demos into the user's Desktop
# along with icons.  It also installs the driver modules into the
# correct system locations and modifies the system init files to
# start-up the drivers at system boot time.
# The drivers are installed by the installDrvr.sh script, which
# must be executed as root to have permission to modify system
# files and directories.
#
# Any system changes performed by this script can be undone by
# running the remove.sh script.
#-----------------------------------------------------------------------


echo "-------------------------------------------"
echo "       Installing Lattice PCIe Demos"
echo "-------------------------------------------"



#=======================================================================
# INSTALL THE DESKTOP ICONS AND LINKS TO THE DEMO EXECUTABLES
#=======================================================================
INSTALL_DIR=`pwd`

#------------------- BASIC DEMO -----------------------
DEMO_NAME=Basic
DEMO_DIR=${INSTALL_DIR}/${DEMO_NAME}
CMD=PCIe${DEMO_NAME}.sh
ICON=${DEMO_DIR}/logo.png

DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

echo "[Desktop Entry]" > $DESKTOP_FILE
echo "Version=1.0" >> $DESKTOP_FILE
echo "Encoding=UTF-8" >> $DESKTOP_FILE
echo "Name=PCIe${DEMO_NAME}" >> $DESKTOP_FILE
echo "Type=Application" >> $DESKTOP_FILE
echo "Exec=$HOME/Desktop/.$CMD" >> $DESKTOP_FILE
echo "Icon=$ICON" >> $DESKTOP_FILE
echo "TryExec=" >> $DESKTOP_FILE
echo "X-GNOME-DocPath=" >> $DESKTOP_FILE
echo "Terminal=true" >> $DESKTOP_FILE
echo "GenericName[en_US]=Lattice PCIe ${DEMO_NAME} Demo" >> $DESKTOP_FILE
echo "Comment[en_US]=" >> $DESKTOP_FILE

echo "#!/bin/sh" > $CMD_FILE
echo "cd ${DEMO_DIR}" >> $CMD_FILE
echo "./rundemo.sh" >> $CMD_FILE

chmod 755 $CMD_FILE
mv $CMD_FILE ~/Desktop/.${CMD_FILE}
mv $DESKTOP_FILE ~/Desktop


#------------------- THRUPUT DEMO -----------------------
DEMO_NAME=Thruput
DEMO_DIR=${INSTALL_DIR}/${DEMO_NAME}
CMD=PCIe${DEMO_NAME}.sh
ICON=${DEMO_DIR}/logo.png

DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

echo "[Desktop Entry]" > $DESKTOP_FILE
echo "Version=1.0" >> $DESKTOP_FILE
echo "Encoding=UTF-8" >> $DESKTOP_FILE
echo "Name=PCIe${DEMO_NAME}" >> $DESKTOP_FILE
echo "Type=Application" >> $DESKTOP_FILE
echo "Exec=$HOME/Desktop/.$CMD" >> $DESKTOP_FILE
echo "Icon=$ICON" >> $DESKTOP_FILE
echo "TryExec=" >> $DESKTOP_FILE
echo "X-GNOME-DocPath=" >> $DESKTOP_FILE
echo "Terminal=true" >> $DESKTOP_FILE
echo "GenericName[en_US]=Lattice PCIe ${DEMO_NAME} Demo" >> $DESKTOP_FILE
echo "Comment[en_US]=" >> $DESKTOP_FILE

echo "#!/bin/sh" > $CMD_FILE
echo "cd ${DEMO_DIR}" >> $CMD_FILE
echo "./rundemo.sh" >> $CMD_FILE

chmod 755 $CMD_FILE
mv $CMD_FILE ~/Desktop/.${CMD_FILE}
mv $DESKTOP_FILE ~/Desktop


#------------------- DMA COLORBARS DEMO -----------------------
DEMO_NAME=ColorBars
DEMO_DIR=${INSTALL_DIR}/DMA
CMD=PCIe${DEMO_NAME}.sh
ICON=${DEMO_DIR}/logo.png

DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

echo "[Desktop Entry]" > $DESKTOP_FILE
echo "Version=1.0" >> $DESKTOP_FILE
echo "Encoding=UTF-8" >> $DESKTOP_FILE
echo "Name=PCIe${DEMO_NAME}" >> $DESKTOP_FILE
echo "Type=Application" >> $DESKTOP_FILE
echo "Exec=$HOME/Desktop/.$CMD" >> $DESKTOP_FILE
echo "Icon=$ICON" >> $DESKTOP_FILE
echo "TryExec=" >> $DESKTOP_FILE
echo "X-GNOME-DocPath=" >> $DESKTOP_FILE
echo "Terminal=true" >> $DESKTOP_FILE
echo "GenericName[en_US]=Lattice PCIe ${DEMO_NAME} Demo" >> $DESKTOP_FILE
echo "Comment[en_US]=" >> $DESKTOP_FILE

echo "#!/bin/sh" > $CMD_FILE
echo "cd ${DEMO_DIR}" >> $CMD_FILE
echo "./runCB.sh" >> $CMD_FILE

chmod 755 $CMD_FILE
mv $CMD_FILE ~/Desktop/.${CMD_FILE}
mv $DESKTOP_FILE ~/Desktop


#------------------- DMA IMAGEMOVE DEMO -----------------------
DEMO_NAME=ImageMove
DEMO_DIR=${INSTALL_DIR}/DMA
CMD=PCIe${DEMO_NAME}.sh
ICON=${DEMO_DIR}/logo.png

DESKTOP_FILE=PCIe${DEMO_NAME}.desktop
CMD_FILE=$CMD

echo "[Desktop Entry]" > $DESKTOP_FILE
echo "Version=1.0" >> $DESKTOP_FILE
echo "Encoding=UTF-8" >> $DESKTOP_FILE
echo "Name=PCIe${DEMO_NAME}" >> $DESKTOP_FILE
echo "Type=Application" >> $DESKTOP_FILE
echo "Exec=$HOME/Desktop/.$CMD" >> $DESKTOP_FILE
echo "Icon=$ICON" >> $DESKTOP_FILE
echo "TryExec=" >> $DESKTOP_FILE
echo "X-GNOME-DocPath=" >> $DESKTOP_FILE
echo "Terminal=true" >> $DESKTOP_FILE
echo "GenericName[en_US]=Lattice PCIe ${DEMO_NAME} Demo" >> $DESKTOP_FILE
echo "Comment[en_US]=" >> $DESKTOP_FILE

echo "#!/bin/sh" > $CMD_FILE
echo "cd ${DEMO_DIR}" >> $CMD_FILE
echo "./runIM.sh" >> $CMD_FILE

chmod 755 $CMD_FILE
mv $CMD_FILE ~/Desktop/.${CMD_FILE}
mv $DESKTOP_FILE ~/Desktop


#=======================================================================
# INSTALL THE DRIVERS AND OTHER SYSTEM STARTUP FILES
# User must have root password to continue
#=======================================================================
echo "Installing driver files requires root password."
su -p -c ./installDrvr.sh root


echo "Installation Completed."
