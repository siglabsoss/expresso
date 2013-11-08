cd "C:/JoeM/PCI_Express/DevKits/ECP3_PCIe/DK-ECP3-PCIE-020.bld/Hardware/PCIe_x1/ecp3-95E_PCIeSGDMA_SBx1/ispLeverGenCore/ecp3/sgdma/sgdmac_eval/sim/aldec"
workspace create sgdmac_test
design create sgdmac_test . 
design open sgdmac_test     
cd "C:/JoeM/PCI_Express/DevKits/ECP3_PCIe/DK-ECP3-PCIE-020.bld/Hardware/PCIe_x1/ecp3-95E_PCIeSGDMA_SBx1/ispLeverGenCore/ecp3/sgdma/sgdmac_eval/sim/aldec"

#==== compile
vlog \
     ../models/*.v \
     ../testbench/*.v 

#==== run the simulation
vsim  -lib sgdmac_test \
      -L ovi_ecp3 \
      -L pmi_work \
      +notimingchecks \
      sgdmac_test.sgdmac_test 

do wave.do
run -all

