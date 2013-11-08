if {[file exists work]} {
   file delete -force work 
}
vlib work
vmap work work

#==== compile
vlog -novopt +define+NO_DEBUG+SIM \
-y ../../../models/ecp3 +libext+.v \
-y ../../../models/mem +libext+.v \
-y C:/lscc/diamond/1.1/bin/nt/../../cae_library/simulation/verilog/ecp3 +libext+.v \
-y C:/lscc/diamond/1.1/bin/nt/../../cae_library/simulation/verilog/pmi +libext+.v \
+incdir+../../../testbench/tests/ecp3 \
+incdir+../../src/params \
+incdir+../../../models/mem \
../../src/params/ddr3_sdram_mem_params.v \
../../../../ddr3core_beh.v \
../../../models/ecp3/clk_phase.v \
../../../models/ecp3/clk_stop.v \
../../../models/ecp3/ddr3_pll.v \
../../../models/ecp3/pll_control.v \
../../../models/ecp3/jitter_filter.v \
../../../models/ecp3/ddr3_clks.v \
../../src/rtl/top/ecp3/ddr3_sdram_mem_top_wrapper.v \
../../../testbench/top/ecp3/odt_watchdog.v \
../../../testbench/top/ecp3/monitor.v \
../../../testbench/top/ecp3/test_mem_ctrl.v


#==== run the simulation
vsim -novopt -t 1ps work.test_mem_ctrl -l ddr3core_eval.log\

do wave.do
