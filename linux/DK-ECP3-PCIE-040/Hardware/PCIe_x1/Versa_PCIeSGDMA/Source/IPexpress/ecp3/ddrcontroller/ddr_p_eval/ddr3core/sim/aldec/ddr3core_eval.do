cd "F:/abhinav/assign4_ebr2ddr3/ipexpress/ddr3cntrlnew/ddr_p_eval/ddr3core/sim/aldec"
workspace create ddr3_space
design create ddr3_design .
design open ddr3_design
cd "F:/abhinav/assign4_ebr2ddr3/ipexpress/ddr3cntrlnew/ddr_p_eval/ddr3core/sim/aldec"
set sim_working_folder .
vlog -msg 0 +define+NO_DEBUG+SIM \
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


vsim -t 1ps ddr3_design.test_mem_ctrl -lib ddr3_design 

add wave -noupdate -divider {Global Signals}
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/clk_in
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/rst_n
add wave -noupdate -divider {Memory Interface Signals}
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_reset_n
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_cke
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_clk
wave -virtual "em_ddr_cmd" -expand /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_cs_n /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_ras_n /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_cas_n /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_we_n
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_addr
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_ba
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_data
add wave -noupdate -format Literal /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_dqs
add wave -noupdate -format Literal /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_dm
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/em_ddr_odt
add wave -noupdate -divider {Local Interface Signals}
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/mem_rst_n
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/sclk_out
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/init_start
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/init_done
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/wl_err
add wave -noupdate -format Literal -radix octal /test_mem_ctrl/U1_ddr_sdram_mem_top/read_pulse_tap
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/clocking_good
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/addr
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/cmd
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/cmd_valid
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/cmd_rdy
add wave -noupdate -format Literal /test_mem_ctrl/U1_ddr_sdram_mem_top/cmd_burst_cnt
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/ofly_burst_len
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/datain_rdy
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/write_data
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/data_mask
add wave -noupdate -format Literal -radix hexadecimal /test_mem_ctrl/U1_ddr_sdram_mem_top/read_data
add wave -noupdate -format Logic /test_mem_ctrl/U1_ddr_sdram_mem_top/read_data_valid
run -all
