add wave -divider {Testbench signals} -color 0,150,0
add wave top_tb/*

add wave -divider {Top level signals} -color 0,150,0
add wave /I_top/*

add wave -divider {PCIExpress IPcore signals} -color 0,150,0
add wave /I_top/pcie/*
 
add wave -divider {WishBone/EBR interface signals} -color 0,150,0
add wave  /I_top/ebr/*


