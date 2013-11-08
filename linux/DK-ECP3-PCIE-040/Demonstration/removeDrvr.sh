#!/bin/sh

# Step 1
# Check that caller is root for privleges
if [ `whoami` != root ]; then
        echo "ERROR! Must be root to remove driver files."
        exit 1
fi

# Step 2
# Then remove driver modules from install locations
INSTALL_DIR=/lib/modules/`uname -r`/extra
echo "Remove driver modules from directory: $INSTALL_DIR"
rm $INSTALL_DIR/lscdma.ko
rm $INSTALL_DIR/lscpcie2.ko

# Step 3
# rebuild system module dependencies to remove our deleted driver 
/sbin/depmod

# Step 4
# Remove installation steps from local init script
sed '/modprobe lscdma/d' /etc/rc.d/rc.local > rm.tmp
sed '/modprobe lscpcie2/d' rm.tmp > /etc/rc.d/rc.local
rm rm.tmp

# Step 5
# remove the udev rules
rm /etc/udev/rules.d/9-lscdma.rules
rm /etc/udev/rules.d/9-lscpcie2.rules

