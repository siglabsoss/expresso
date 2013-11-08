// $Id: sfif_ca.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_ca(clk_125, rstn, 
               cp_ph, cp_pd, cp_nph, 
               ca_ph, ca_pd, ca_nph, ca_npd,
               credit_available               
);                 
                 
input clk_125;
input rstn;
input cp_ph, cp_nph;
input [3:0]  cp_pd; 
input [8:0]  ca_ph, ca_nph;
input [12:0] ca_pd, ca_npd;

output credit_available;

reg credit_available_p, credit_available_np;

assign credit_available = credit_available_p || credit_available_np;

always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    credit_available_p <= 1'b0;  
    credit_available_np <= 1'b0;  
  end
  else
  begin
    if (cp_ph)
    begin
      if ( (ca_ph[8] || (ca_ph[7:0] > 8'd1)) && (ca_pd[12] || (ca_pd[11:0] >= {cp_pd[3:0],1'b0})) )
        credit_available_p <= 1'b1;
      else
        credit_available_p <= 1'b0;
    end
    else
      credit_available_p <= 1'b0;
    
    if (cp_nph)
    begin
      if (ca_nph[8] || (ca_nph[7:0] > 8'd1)) 
        credit_available_np <= 1'b1;
      else
        credit_available_np <= 1'b0;
    end
    else
      credit_available_np <= 1'b0;
  
  end

end

endmodule

