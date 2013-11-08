//`timescale 100 ps/100 ps
`timescale 1ns / 1 ps
module top_tb;

reg rstn;
reg clk_100;
initial begin
   clk_100 = 1'b0;
   rstn = 1'b0;
   #2010;
   rstn = 1'b1;
  end
always   #50 clk_100 <= ~clk_100 ;
   
PUR  PUR_INST (.PUR (1'b1));
GSR  GSR_INST (.GSR (1'b1));

		
		

top I_top  (   
   .rstn        (rstn        ),
   .FLIP_LANES  (1'b1        ), 
   .LED_INV     (1'b1        ),
   
    
   .refclkp     (clk_100     ), 
   .refclkn     (~clk_100    ),
   .hdinp       (1'h1       ), 
   .hdinn       (1'h0       ),                   
   .hdoutp      (hdoutp      ), 
   .hdoutn      (hdoutn      ), 

   .pll_lk      (), 
   .poll        (), 
   .l0          (), 
   .dl_up       (), 
   .usr0        (), 
   .usr1        (), 
   .usr2        (), 
   .usr3        (),
   .dip_switch  (8'hEF), 
   .led_out     (), 
   .dp          (),
   .TP          () 
 // .la          ()  
   );         

endmodule