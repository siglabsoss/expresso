
#Begin clock constraint
define_clock -name {my_pll|CLKOK_inferred_clock} {n:my_pll|CLKOK_inferred_clock} -period 6.064 -clockgroup Autoconstr_clkgroup_0 -rise 0.000 -fall 3.032 -route 0.000 
#End clock constraint

#Begin clock constraint
define_clock -name {topcount|clk_ADC0_DCO_P} {p:topcount|clk_ADC0_DCO_P} -period 10000000.000 -clockgroup Autoconstr_clkgroup_1 -rise 0.000 -fall 5000000.000 -route 0.000 
#End clock constraint

#Begin clock constraint
define_clock -name {my_pll|CLKOP_inferred_clock} {n:my_pll|CLKOP_inferred_clock} -period 3.051 -clockgroup Autoconstr_clkgroup_2 -rise 0.000 -fall 1.525 -route 0.000 
#End clock constraint

#Begin clock constraint
define_clock -name {topcount|direction} {p:topcount|direction} -period 10000000.000 -clockgroup Autoconstr_clkgroup_3 -rise 0.000 -fall 5000000.000 -route 0.000 
#End clock constraint

#Begin clock constraint
define_clock -name {topcount|clk} {p:topcount|clk} -period 2.181 -clockgroup Autoconstr_clkgroup_4 -rise 0.000 -fall 1.091 -route 0.000 
#End clock constraint
