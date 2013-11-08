module bridge_64b_to_16b(
input rstn,
input clk_125,

input [63:0] tx_data_64b,
input tx_st_64b,
input tx_end_64b,
input tx_dwen_64b,
output tx_rdy_64b,
output tx_val,

input tx_rdy_16b,
output reg [15:0] tx_data_16b,
output reg tx_st_16b,
output reg tx_end_16b
);
assign tx_rdy_64b = tx_rdy_16b && (~ tx_end_64b);

reg tx_rdy_16b_d;
reg [1:0] cnt;
reg xmit_processing;
reg tx_st_64b_d;
reg tx_val_reg1;
reg tx_val_reg2;
reg mimic_tx_dwen;
assign tx_val = (tx_val_reg1 & (~ tx_st_64b)) || tx_val_reg2;
assign mimic_tx_dwen_tmp = mimic_tx_dwen && tx_end_64b;
always @(posedge clk_125 or negedge rstn)
   if (!rstn) begin
      tx_rdy_16b_d <= 0; 
      tx_st_64b_d <= 0;
      tx_val_reg1 <= 1'b1;
      tx_val_reg2 <= 1'b0;
      xmit_processing <= 1'b0;
      cnt    <= 0;
      mimic_tx_dwen <= 0;
   end
   else begin
      if (tx_st_64b) begin //only support for 3
         if (tx_data_64b[61]) begin //for 4DW application
            if (tx_data_64b[62]) //with data
               mimic_tx_dwen <= tx_data_64b[32];
            else //without data
               mimic_tx_dwen <= 1'b0;
   end
   else begin
         if (tx_data_64b[62]) //with data
            mimic_tx_dwen <= ~ tx_data_64b[32];
         else //without data
               mimic_tx_dwen <= 1'b1;
         end
      end
            
      tx_rdy_16b_d <= tx_rdy_16b; 
      tx_st_64b_d <= tx_st_64b;
      
      if (tx_st_64b) begin
         xmit_processing <= 1'b1;
         tx_val_reg1 <= 0;
         cnt    <= 0;
      end
      else if (tx_end_64b && (cnt == 3)) begin
         xmit_processing <= 1'b0;
         tx_val_reg1     <= 1'b1;
      end
      
      if (tx_rdy_16b && (~ tx_rdy_16b_d))
         cnt <= 0;   
      else if (xmit_processing || tx_st_64b)
         cnt   <= cnt + 1;
      else
         cnt   <= 0;
         
      tx_val_reg2 <= xmit_processing && (cnt == 2);
   end
   
always @(*) begin
   case (cnt)
      2'd0: begin
         tx_st_16b   = tx_st_64b;
         tx_data_16b = tx_data_64b[63:48];
         tx_end_16b  = 1'b0;
      end
      2'd1: begin
         tx_st_16b   = 1'b0;
         tx_data_16b = tx_data_64b[47:32];
         tx_end_16b  = mimic_tx_dwen_tmp && tx_end_64b;//tx_dwen_64b && tx_end_64b;         
      end
      2'd2: begin
         tx_st_16b   = 1'b0;
         tx_data_16b = tx_data_64b[31:16];
         tx_end_16b  = 1'b0;           
      end
      default: begin
         tx_st_16b   = 1'b0;
         tx_data_16b = tx_data_64b[15:0];
         tx_end_16b  = (~ mimic_tx_dwen_tmp) && tx_end_64b;//(~ tx_dwen_64b) && tx_end_64b;                    
      end
   endcase
end
endmodule