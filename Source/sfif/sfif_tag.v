// $Id: sfif_tag.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_tag(clk_125, rstn,
                tx_st, tag, tag_cplds, 
                rx_st, rx_data, 
                tag_available
);

input clk_125;
input rstn;
input tx_st;
input [4:0] tag;
input [3:0] tag_cplds;
input rx_st;
input [63:0] rx_data;
output tag_available;

reg tag_available;
reg [3:0] tag0, tag8 , tag16, tag24;
reg [3:0] tag1, tag9 , tag17, tag25;
reg [3:0] tag2, tag10, tag18, tag26;
reg [3:0] tag3, tag11, tag19, tag27;
reg [3:0] tag4, tag12, tag20, tag28;
reg [3:0] tag5, tag13, tag21, tag29;
reg [3:0] tag6, tag14, tag22, tag30;
reg [3:0] tag7, tag15, tag23, tag31;

reg rx_st_p;
reg debug;


always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    tag_available <= 1'b0;
    tag0 <= 4'd0; tag8  <= 4'd0; tag16 <= 4'd0; tag24 <= 4'd0;
    tag1 <= 4'd0; tag9  <= 4'd0; tag17 <= 4'd0; tag25 <= 4'd0;
    tag2 <= 4'd0; tag10 <= 4'd0; tag18 <= 4'd0; tag26 <= 4'd0;
    tag3 <= 4'd0; tag11 <= 4'd0; tag19 <= 4'd0; tag27 <= 4'd0;
    tag4 <= 4'd0; tag12 <= 4'd0; tag20 <= 4'd0; tag28 <= 4'd0;
    tag5 <= 4'd0; tag13 <= 4'd0; tag21 <= 4'd0; tag29 <= 4'd0;
    tag6 <= 4'd0; tag14 <= 4'd0; tag22 <= 4'd0; tag30 <= 4'd0;
    tag7 <= 4'd0; tag15 <= 4'd0; tag23 <= 4'd0; tag31 <= 4'd0;    
    rx_st_p <= 1'b0;
    
  end
  else
  begin  
    rx_st_p <= rx_st;        
    
    if (rx_st_p)
    begin
      case (rx_data[44:40])
        5'd0:  if (tag0 !=4'd0) tag0  <= tag0  - 1;  
        5'd1:  if (tag1 !=4'd0) tag1  <= tag1  - 1;  
        5'd2:  if (tag2 !=4'd0) tag2  <= tag2  - 1;     
        5'd3:  if (tag3 !=4'd0) tag3  <= tag3  - 1;     
        5'd4:  if (tag4 !=4'd0) tag4  <= tag4  - 1;     
        5'd5:  if (tag5 !=4'd0) tag5  <= tag5  - 1;     
        5'd6:  if (tag6 !=4'd0) tag6  <= tag6  - 1;     
        5'd7:  if (tag7 !=4'd0) tag7  <= tag7  - 1;     
        5'd8:  if (tag8 !=4'd0) tag8  <= tag8  - 1;     
        5'd9:  if (tag9 !=4'd0) tag9  <= tag9  - 1;     
        5'd10: if (tag10!=4'd0) tag10 <= tag10 - 1;      
        5'd11: if (tag11!=4'd0) tag11 <= tag11 - 1;      
        5'd12: if (tag12!=4'd0) tag12 <= tag12 - 1;      
        5'd13: if (tag13!=4'd0) tag13 <= tag13 - 1;      
        5'd14: if (tag14!=4'd0) tag14 <= tag14 - 1;      
        5'd15: if (tag15!=4'd0) tag15 <= tag15 - 1;      
        5'd16: if (tag16!=4'd0) tag16 <= tag16 - 1;      
        5'd17: if (tag17!=4'd0) tag17 <= tag17 - 1;      
        5'd18: if (tag18!=4'd0) tag18 <= tag18 - 1;      
        5'd19: if (tag19!=4'd0) tag19 <= tag19 - 1;      
        5'd20: if (tag20!=4'd0) tag20 <= tag20 - 1;      
        5'd21: if (tag21!=4'd0) tag21 <= tag21 - 1;      
        5'd22: if (tag22!=4'd0) tag22 <= tag22 - 1;      
        5'd23: if (tag23!=4'd0) tag23 <= tag23 - 1;      
        5'd24: if (tag24!=4'd0) tag24 <= tag24 - 1;      
        5'd25: if (tag25!=4'd0) tag25 <= tag25 - 1;      
        5'd26: if (tag26!=4'd0) tag26 <= tag26 - 1;      
        5'd27: if (tag27!=4'd0) tag27 <= tag27 - 1;      
        5'd28: if (tag28!=4'd0) tag28 <= tag28 - 1;      
        5'd29: if (tag29!=4'd0) tag29 <= tag29 - 1;      
        5'd30: if (tag30!=4'd0) tag30 <= tag30 - 1;      
        5'd31: if (tag31!=4'd0) tag31 <= tag31 - 1;      
        default:
        begin
        end 
      endcase    
    
    end 
    else
    begin
      case (tag)
        5'd0:  begin tag_available <= (tag0 ==4'd0); if (tx_st) tag0  <= tag_cplds; end
        5'd1:  begin tag_available <= (tag1 ==4'd0); if (tx_st) tag1  <= tag_cplds; end
        5'd2:  begin tag_available <= (tag2 ==4'd0); if (tx_st) tag2  <= tag_cplds; end   
        5'd3:  begin tag_available <= (tag3 ==4'd0); if (tx_st) tag3  <= tag_cplds; end      
        5'd4:  begin tag_available <= (tag4 ==4'd0); if (tx_st) tag4  <= tag_cplds; end      
        5'd5:  begin tag_available <= (tag5 ==4'd0); if (tx_st) tag5  <= tag_cplds; end      
        5'd6:  begin tag_available <= (tag6 ==4'd0); if (tx_st) tag6  <= tag_cplds; end      
        5'd7:  begin tag_available <= (tag7 ==4'd0); if (tx_st) tag7  <= tag_cplds; end      
        5'd8:  begin tag_available <= (tag8 ==4'd0); if (tx_st) tag8  <= tag_cplds; end      
        5'd9:  begin tag_available <= (tag9 ==4'd0); if (tx_st) tag9  <= tag_cplds; end      
        5'd10: begin tag_available <= (tag10==4'd0); if (tx_st) tag10 <= tag_cplds; end       
        5'd11: begin tag_available <= (tag11==4'd0); if (tx_st) tag11 <= tag_cplds; end       
        5'd12: begin tag_available <= (tag12==4'd0); if (tx_st) tag12 <= tag_cplds; end       
        5'd13: begin tag_available <= (tag13==4'd0); if (tx_st) tag13 <= tag_cplds; end       
        5'd14: begin tag_available <= (tag14==4'd0); if (tx_st) tag14 <= tag_cplds; end       
        5'd15: begin tag_available <= (tag15==4'd0); if (tx_st) tag15 <= tag_cplds; end       
        5'd16: begin tag_available <= (tag16==4'd0); if (tx_st) tag16 <= tag_cplds; end       
        5'd17: begin tag_available <= (tag17==4'd0); if (tx_st) tag17 <= tag_cplds; end       
        5'd18: begin tag_available <= (tag18==4'd0); if (tx_st) tag18 <= tag_cplds; end       
        5'd19: begin tag_available <= (tag19==4'd0); if (tx_st) tag19 <= tag_cplds; end       
        5'd20: begin tag_available <= (tag20==4'd0); if (tx_st) tag20 <= tag_cplds; end       
        5'd21: begin tag_available <= (tag21==4'd0); if (tx_st) tag21 <= tag_cplds; end       
        5'd22: begin tag_available <= (tag22==4'd0); if (tx_st) tag22 <= tag_cplds; end       
        5'd23: begin tag_available <= (tag23==4'd0); if (tx_st) tag23 <= tag_cplds; end       
        5'd24: begin tag_available <= (tag24==4'd0); if (tx_st) tag24 <= tag_cplds; end       
        5'd25: begin tag_available <= (tag25==4'd0); if (tx_st) tag25 <= tag_cplds; end       
        5'd26: begin tag_available <= (tag26==4'd0); if (tx_st) tag26 <= tag_cplds; end       
        5'd27: begin tag_available <= (tag27==4'd0); if (tx_st) tag27 <= tag_cplds; end       
        5'd28: begin tag_available <= (tag28==4'd0); if (tx_st) tag28 <= tag_cplds; end       
        5'd29: begin tag_available <= (tag29==4'd0); if (tx_st) tag29 <= tag_cplds; end       
        5'd30: begin tag_available <= (tag30==4'd0); if (tx_st) tag30 <= tag_cplds; end       
        5'd31: begin tag_available <= (tag31==4'd0); if (tx_st) tag31 <= tag_cplds; end       
        default:
        begin
        end 
      endcase
    end
  end // clk
end



endmodule


