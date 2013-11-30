#
# Clocks
#
define_clock   {n:refclkn}  -freq 100 -clockgroup default_clkgroup_0
define_clock   {n:refclkp}  -freq 100 -clockgroup default_clkgroup_1
define_clock   {n:clk_125}  -freq 125 -clockgroup default_clkgroup_3

define_clock   {n:clk_ADC0_DCO_P}  -freq 100 -clockgroup default_clkgroup_0
define_clock   {n:ddr_sclk}        -freq 100 -clockgroup default_clkgroup_0