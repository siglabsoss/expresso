if {!0} {
    vlib work 
}
vmap work work

#==== compile
vlog -novopt -noincr \
     ../models/*.v \
     ../testbench/*.v \
     -y C:/ispTOOLS8_0/ispcpld/tcltk/lib/ipwidgets/ispipbuilder/../../../../../cae_library/simulation/verilog/ecp3 +libext+.v \
     -y C:/ispTOOLS8_0/ispcpld/tcltk/lib/ipwidgets/ispipbuilder/../../../../../cae_library/simulation/verilog/pmi           +libext+.v 

#==== run the simulation
vsim  -novopt -L work \
      +notimingchecks \
      +nowarnTSCALE \
      sgdmac_test 

view wave
do wave.do
run -all

