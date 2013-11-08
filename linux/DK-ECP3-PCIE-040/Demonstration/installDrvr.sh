#!/bin/sh

# Step 1
# Check that caller is root for privleges
if [ `whoami` != root ]; then
        echo "ERROR! Must be root to install drivers."
        exit 1
fi

# Step 2
# Then move driver modules into proper location in system
INSTALL_DIR=/lib/modules/`uname -r`/extra
echo "Driver module installation directory: $INSTALL_DIR"
install -d $INSTALL_DIR
install ./DMA/Driver/lscdma.ko $INSTALL_DIR
install ./Basic/Driver/lscpcie2.ko $INSTALL_DIR

# Step 3
# rebuild system module dependencies
/sbin/depmod

# Step 4
# Add loading the new drivers to the system init scripts
# check first to see if the lines already exist in the file
if [ ! `sed -n '/modprobe lscdma/=' /etc/rc.d/rc.local` ]; then
	echo "modprobe lscdma" >> /etc/rc.d/rc.local
fi

if [ ! `sed -n '/modprobe lscpcie2/=' /etc/rc.d/rc.local` ]; then
	echo "modprobe lscpcie2" >> /etc/rc.d/rc.local
fi


# Step 5
# Copy over the udev rules so the hardware is detected automajically
# and the /dev/ device nodes are created by the system too.
# if the kernel version is using an older udev version then test
# statements use a singe "=" so convert to older format by replacing
# the "==" with "=" and storing results into /etc/udev/rules.d
# otherwise just copy the rules files.
if [ -n "`sed -n '/KERNEL==/=' /etc/udev/rules.d/50-udev*.rules`" ]; then
	install -m 0644 ./DMA/Driver/9-lscdma.rules /etc/udev/rules.d
	install -m 0644 ./Basic/Driver/9-lscpcie2.rules  /etc/udev/rules.d
else
	echo "Converting udev rules to older format."
	sed 's/KERNEL==/KERNEL=/g' ./Basic/Driver/9-lscpcie2.rules > /etc/udev/rules.d/9-lscpcie2.rules 
	sed 's/KERNEL==/KERNEL=/g' ./DMA/Driver/9-lscdma.rules > /etc/udev/rules.d/9-lscdma.rules
fi

# Step 6
# Manually trigger modprobe so the drivers load right now without
# needing a system restart (they'll automatically load next time)
/sbin/modprobe lscdma
/sbin/modprobe lscpcie2

