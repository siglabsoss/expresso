#-- Lattice Semiconductor Corporation Ltd.
#-- Synplify OEM project file

#device options
set_option -technology LATTICE-ECP3
set_option -part LFE3_35EA
set_option -package FN484C
set_option -speed_grade -8

#compilation/mapping options
set_option -symbolic_fsm_compiler true
set_option -resource_sharing true

#use verilog 2001 standard option
set_option -vlog_std v2001

#map options
set_option -frequency auto
set_option -maxfan 1000
set_option -auto_constrain_io 0
set_option -disable_io_insertion false
set_option -retiming false; set_option -pipe true
set_option -force_gsr false
set_option -compiler_compatible 0
set_option -dup false

add_file -constraint {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/top.sdc}
set_option -default_enum_encoding default

#simulation options


#timing analysis options



#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#synplifyPro options
set_option -fix_gated_and_generated_clocks 1
set_option -update_models_cp 0
set_option -resolve_multiple_driver 0


#-- add_file options
set_option -include_path {C:/Users/joel/Documents/popwi_expresso/expresso/expresso}
add_file -verilog {C:/lscc/diamond/2.2_x64/cae_library/synthesis/verilog/ecp3.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/32kebr/spram_16384_16.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/32kebr/wbs_32kebr.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/gpio/wbs_gpio.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/bridge_64b_to_16b.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/pmi_ram_dp.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/bridge_16b_to_64b.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_cr.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_rx_fifo.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_ctrl.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_tag.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_ca.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_tx_fifo.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif_wbs.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/sfif/sfif.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_arb/wb_arb.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/sync_logic.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/async_pkt_fifo.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc_cpld_fifo.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc_cpld.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_intf.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc_cr.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc_req_fifo.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc_dec.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/wb_tlc/wb_tlc.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ip_tx_arbiter.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ur_gen/UR_gen.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ip_crpr_arb.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ip_rx_crpr.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ecp3/pcie_top.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/led_status.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../source/ecp3/top_sfif.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../Source/IPExpress/ecp3/pciex1/pcie_bb.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/../../Source/IPExpress/ecp3/pciex1/pcs_pipe_bb.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/impl1/source/pop_data.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/impl1/source/adc_ram.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/tb_sfif_wbs.v}
add_file -verilog {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/ddr_generic.v}

#-- top module name
set_option -top_module top

#-- set result format/file last
project -result_file {C:/Users/joel/Documents/popwi_expresso/expresso/expresso/impl1/top_impl1.edi}

#-- error message log file
project -log_file {top_impl1.srf}

#-- set any command lines input by customer


#-- run Synplify with 'arrange HDL file'
project -run
