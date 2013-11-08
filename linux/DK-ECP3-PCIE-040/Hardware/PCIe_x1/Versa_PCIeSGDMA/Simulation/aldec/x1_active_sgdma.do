  set PROJ_DIR C:\Lattice_DevKits\DK-ECP3-PCIE-040\Hardware\PCIe_x1\Versa_PCIeSGDMA\Simulation\aldec
  set PCIE_X1_IP_PATH $PROJ_DIR/../../source/IPexpress/ecp3/pciex1
  set SGDMA_IP_PATH   $PROJ_DIR/../../source/IPexpress/ecp3/sgdma  
  cd $PROJ_DIR

#if {![file exists pcie_design]} {
  #  vlib pcie_design 
#}
#endif

design create pcie_design .
design open pcie_design
adel -all
cd $PROJ_DIR
 
  # Make vlog and vsim commands
  #==============================================================================
  amap pcie_bfm_lib $PROJ_DIR\..\..\testbench\bfm_lspcie_rc\bfm_lspcie_rc.LIB 
  vlib pcie_ep_lib

  vlog -msg 0 +define+DEBUG=0 +define+SIMULATE=1 \
  +incdir+$PCIE_X1_IP_PATH/pcie_eval/src/params \
  -y C:\lscc\diamond\1.3\cae_library\simulation\verilog\ecp3 +libext+.v \
  -y C:\lscc\diamond\1.3\cae_library\simulation\verilog\pmi +libext+.v  \
  $PCIE_X1_IP_PATH/pcie_eval/pcie/src/params/pci_exp_params.v  \
  $PCIE_X1_IP_PATH/pcie_eval/pcie/src/params/pci_exp_ddefines.v  \
  $PCIE_X1_IP_PATH/pcie_eval/models/ecp3/sync1s.v  \
  $PCIE_X1_IP_PATH/pcie_eval/models/ecp3/rx_gear.v  \
  $PCIE_X1_IP_PATH/pcie_eval/models/ecp3/tx_gear.v  \
  $PCIE_X1_IP_PATH/pcie_eval/../pcie.v  \
  $PCIE_X1_IP_PATH/pcie_eval/../pcie_beh.v \
  ../../source/IPexpress/ecp3/pll/pll.v \
  ../../Source/led_status.v \
  ../../Source/ip_rx_crpr.v \
  ../../Source/ip_crpr_arb.v \
  ../../Source/ur_gen/UR_gen.v \
  ../../Source/ip_tx_arbiter.v \
  ../../Source/wb_tlc/wb_tlc.v \
  ../../Source/wb_tlc/wb_intf.v \
  ../../Source/wb_tlc/wb_tlc_cpld.v \
  ../../Source/wb_tlc/wb_tlc_cpld_fifo.v \
  ../../Source/wb_tlc/wb_tlc_cr.v \
  ../../Source/wb_tlc/wb_tlc_dec.v \
  ../../Source/wb_tlc/wb_tlc_req_fifo.v \
  ../../Source/wb_tlc/async_pkt_fifo.v \
  ../../Source/wb_tlc/sync_logic.v \
  ../../Source/64kebr_ddr3/wbs_64kebr_ddr3.v \
  ../../Source/64kebr_ddr3/spram_8192_64.v \
  ../../Source/wb_arb/wb_arb.v \
  ../../Source/gpio/wbs_gpio.v \
  ../../Source/sgdma_top.v \
  ../../Source/sync.v \
  ../../Source/dma_adapter/dma_adapter.v \
  ../../Source/dma_adapter/dma_rx_fifo.v \
  ../../Source/dma_adapter/dma_tx_fifo_new.v \
  ../../Source/dma_adapter/dma_wbs.v \
  ../../Source/colorbar/wbs_colorbar.v \
  $SGDMA_IP_PATH/sgdma.v \
  $SGDMA_IP_PATH/sgdmac_eval/sim/models/sgdmac_beh.v \
  -work pcie_ep_lib

alog -msg 0 -l pcie_bfm_lib ../../Source/ecp3/pcie_top.v 
alog -msg 5 +define+SIMULATE=1 "../../Source/ecp3/top_sgdma.v"

alog -msg 5 -l pcie_design ../../testbench/top_tb.v 

alog -incr -v2k5 +incdir+$PROJ_DIR/../../testbench/bfm_lspcie_rc/src ../../testbench/pcie_vlog_test_case.v -work pcie_bfm_lib

vsim +access +r -t 1ps top_tb -lib pcie_design -L pcie_bfm_lib -L ovi_ecp3 -L pcie_ep_lib 
radix -hexadecimal
do wave.do
run -all

