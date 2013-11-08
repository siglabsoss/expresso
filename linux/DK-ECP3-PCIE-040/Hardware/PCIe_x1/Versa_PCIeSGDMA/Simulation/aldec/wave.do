add wave -divider {Testbench signals} -color 0,150,0
add wave top_tb/*

add wave -divider {GPIO signals} -color 0,150,0
add wave /I_top/gpio/*

add wave -divider {WishBone/EBR interface signals} -color 0,255,0
add wave  /I_top/ebr/*