#!/bin/sh
#
# This bash script starts the demo in 1 of 2 ways:
# (1) user pre-defines environment variables and they are used
# (2) defaults are used to start the most common setup
# 
# The DEMO_JRE variable points to the local install of Java
# 1.5.0 JRE.  It will only work with Java 1.5.0
#
echo "Lattice PCIe Throughput Demo"

if [ -z $DEMO_JRE ]; then
	DEMO_JRE=../jre1.5.0_14
fi


if [ -z $LSC_PCIE_BOARD ]; then
	export LSC_PCIE_BOARD="ECP3"
fi 

if [ -z $LSC_PCIE_IP_ID ]; then
	export LSC_PCIE_IP_ID="SFIF"
fi

if [ -z $LSC_PCIE_INSTANCE ]; then
	export LSC_PCIE_INSTANCE=1
fi

# Invoke the Java GUI
echo "Starting PCIe Thruput demo on board: $LSC_PCIE_BOARD"
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
$DEMO_JRE/bin/java -classpath .:./lib -jar SFIF_UI.jar


