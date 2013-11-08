// $Id: sfif_cr.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_cr(clk_125, rstn, 
               rx_st, rx_data,
               cplh_cr, cpld_cr
);                 
                 
input clk_125;
input rstn;
input rx_st;
input [63:0] rx_data;
output cplh_cr;
output [7:0] cpld_cr;

reg cplh_cr;
reg [7:0] cpld_cr;


always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    cplh_cr <= 1'b0;  
    cpld_cr <= 8'd0;  
  end
  else
  begin
    if (rx_st && (rx_data[63:56] == 8'h4a)) // CplD
    begin
      cplh_cr <= 1'b1;
      cpld_cr <= rx_data[41:34]; // get length div 4         
    end    
    else
    begin
      cplh_cr <= 1'b0;
      cpld_cr <= 8'd0;      
    end    
  end
end

endmodule

