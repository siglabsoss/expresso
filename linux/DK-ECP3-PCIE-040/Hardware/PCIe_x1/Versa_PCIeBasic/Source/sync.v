// $Id: sync.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sync(clk, rstn, din, dout);

input clk;
input rstn;
input din;
output dout;

reg din_p;
reg dout;


always @(posedge clk or negedge rstn)
begin
  if (~rstn)
  begin
    dout <= 1'b0;
    din_p <= 1'b0;    
  end
  else
  begin
    din_p <= din;
    dout <= din_p;  
  end
end

endmodule

