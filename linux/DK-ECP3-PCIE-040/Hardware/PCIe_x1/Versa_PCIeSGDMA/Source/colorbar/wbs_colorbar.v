// $Id: wbs_colorbar.v,v 1.1.1.1 2008/07/01 17:34:23 jfreed Exp $

module wbs_colorbar(
   wb_clk_i
  ,wb_rst_i  
  ,wb_cyc_i  
  ,wb_cti_i
  ,wb_sel_i
  ,wb_stb_i
  ,wb_we_i
  ,wb_dat_o
  ,wb_ack_o
  ,wb_err_o
  ,wb_rty_o  
);


input         wb_clk_i;
input         wb_rst_i;

input         wb_cyc_i; // Bus Enable Signal
input  [ 7:0] wb_sel_i; // Byte Enables
input         wb_stb_i; // Data Is Valid
input [2:0]   wb_cti_i;
input         wb_we_i;  // Write Enable

output [63:0] wb_dat_o; // Output Data
output        wb_ack_o; // Validate current bus cycle
output        wb_err_o; // Errored Cycle
output        wb_rty_o; // Retry Cycle

reg           wb_ack_p;
reg [63:0] wb_dat_o;

wire [31:0] array [0:63]; 
`ifdef CB_SIM
assign array[ 0] =  32'd0 ;
assign array[ 1] =  32'd1 ;
assign array[ 2] =  32'd2 ;
assign array[ 3] =  32'd3 ;
assign array[ 4] =  32'd4 ;
assign array[ 5] =  32'd5 ;
assign array[ 6] =  32'd6 ;
assign array[ 7] =  32'd7 ;
assign array[ 8] =  32'd8 ;
assign array[ 9] =  32'd9 ;
assign array[10] =  32'd10;
assign array[11] =  32'd11;
assign array[12] =  32'd12;
assign array[13] =  32'd13;
assign array[14] =  32'd14;
assign array[15] =  32'd15;
assign array[16] =  32'd16;
assign array[17] =  32'd17;
assign array[18] =  32'd18;
assign array[19] =  32'd19;
assign array[20] =  32'd20;
assign array[21] =  32'd21;
assign array[22] =  32'd22;
assign array[23] =  32'd23;
assign array[24] =  32'd24;
assign array[25] =  32'd25;
assign array[26] =  32'd26;
assign array[27] =  32'd27;
assign array[28] =  32'd28;
assign array[29] =  32'd29;
assign array[30] =  32'd30;
assign array[31] =  32'd31;
assign array[32] =  32'd32;
assign array[33] =  32'd33;
assign array[34] =  32'd34;
assign array[35] =  32'd35;
assign array[36] =  32'd36;
assign array[37] =  32'd37;
assign array[38] =  32'd38;
assign array[39] =  32'd39;
assign array[40] =  32'd40;
assign array[41] =  32'd41;
assign array[42] =  32'd42;
assign array[43] =  32'd43;
assign array[44] =  32'd44;
assign array[45] =  32'd45;
assign array[46] =  32'd46;
assign array[47] =  32'd47;
assign array[48] =  32'd48;
assign array[49] =  32'd49;
assign array[50] =  32'd50;
assign array[51] =  32'd51;
assign array[52] =  32'd52;
assign array[53] =  32'd53;
assign array[54] =  32'd54;
assign array[55] =  32'd55;
assign array[56] =  32'd56;
assign array[57] =  32'd57;
assign array[58] =  32'd58;
assign array[59] =  32'd59;
assign array[60] =  32'd60;
assign array[61] =  32'd61;
assign array[62] =  32'd62;
assign array[63] =  32'd63;
`else
assign array[ 0] =  32'h000000ff;
assign array[ 1] =  32'h00000ef3;
assign array[ 2] =  32'h00001ce7;
assign array[ 3] =  32'h000028db;
assign array[ 4] =  32'h000034cf;
assign array[ 5] =  32'h000040c3;
assign array[ 6] =  32'h00004cb7;
assign array[ 7] =  32'h000058ab;
assign array[ 8] =  32'h0000649f;
assign array[ 9] =  32'h00007093;
assign array[10] =  32'h00007c87;
assign array[11] =  32'h0000887b;
assign array[12] =  32'h0000946f;
assign array[13] =  32'h0000a063;
assign array[14] =  32'h0000ac57;
assign array[15] =  32'h0000b84b;
assign array[16] =  32'h0000c43f;
assign array[17] =  32'h0000d033;
assign array[18] =  32'h0000dc27;
assign array[19] =  32'h0000e81b;
assign array[20] =  32'h0000f40f;
assign array[21] =  32'h0000ff00;
assign array[22] =  32'h000ef300;
assign array[23] =  32'h001ce700;
assign array[24] =  32'h0028db00;
assign array[25] =  32'h0034cf00;
assign array[26] =  32'h0040c300;
assign array[27] =  32'h004cb700;
assign array[28] =  32'h0058ab00;
assign array[29] =  32'h00649f00;
assign array[30] =  32'h00709300;
assign array[31] =  32'h007c8700;
assign array[32] =  32'h00887b00;
assign array[33] =  32'h00946f00;
assign array[34] =  32'h00a06300;
assign array[35] =  32'h00ac5700;
assign array[36] =  32'h00b84b00;
assign array[37] =  32'h00c43f00;
assign array[38] =  32'h00d03300;
assign array[39] =  32'h00dc2700;
assign array[40] =  32'h00e81b00;
assign array[41] =  32'h00f40f00;
assign array[42] =  32'h00ff0300;
assign array[43] =  32'h00ff0000;
assign array[44] =  32'h00f3000e;
assign array[45] =  32'h00e7001c;
assign array[46] =  32'h00db0028;
assign array[47] =  32'h00cf0034;
assign array[48] =  32'h00c30040;
assign array[49] =  32'h00b7004c;
assign array[50] =  32'h00ab0058;
assign array[51] =  32'h009f0064;
assign array[52] =  32'h00930070;
assign array[53] =  32'h0087007c;
assign array[54] =  32'h007b0088;
assign array[55] =  32'h006f0094;
assign array[56] =  32'h006300a0;
assign array[57] =  32'h005700ac;
assign array[58] =  32'h004b00b8;
assign array[59] =  32'h003f00c4;
assign array[60] =  32'h003300d0;
assign array[61] =  32'h002700dc;
assign array[62] =  32'h001b00e8;
assign array[63] =  32'h000f00f4;
`endif


reg [10:0] j;
reg [5:0] i, k;
reg wb_cti_p;

assign wb_err_o = 1'b0;
assign wb_rty_o = 1'b0;

`ifdef CB_SIM
parameter [10:0] color_repeat = 11'd7;
parameter [5:0] color_rows = 11'd7;
`else
parameter [10:0] color_repeat = 11'd2047; // 256*8=2048
parameter [5:0] color_rows = 6'd63;
`endif


always @(posedge wb_rst_i or posedge wb_clk_i) 
begin
  if (wb_rst_i) 
  begin
    wb_dat_o <= 64'd0;        
    wb_ack_p <= 1'b0;  
    wb_cti_p <= 1'b0;  
    i <= 6'd0;
    k <= 6'd0;
    j <= 11'd0;
  end
  else  
  begin
        
    wb_ack_p <= wb_stb_i;
    
    if (wb_stb_i && wb_cyc_i && (wb_cti_i != 3'b111))// && ~wb_cti_p)
    begin    
`ifdef CB_SIM
      wb_dat_o <= {32'd0, array[i+k]};            
`else
      wb_dat_o <= {array[i+k], array[i+k]};            
`endif
      wb_cti_p <= 1'b0;
    
      
      if (j==color_repeat)  
      begin
        j <= 11'd0;
        i <= i + 1;
      end
      else
        j <= j + 1;
        
      if (i==color_rows && j==color_repeat)
      begin
        i <= 6'd0;        
        k <= k + 1;
      end
    
    end
    else if (wb_stb_i && wb_cyc_i)
    begin    
       wb_cti_p <= (wb_cti_i == 3'b111) & wb_ack_p;       
    end
    
    
  end
end
       
assign wb_ack_o = wb_ack_p ;//&& wb_stb_i;
  

endmodule