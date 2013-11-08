// $Id: sfif_rx_fifo.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_rx_fifo(wb_clk, clk_125, rstn, 
                    timestamp,
                    rx32_data, rden, empty, 
                    rx64_st, rx64_end, rx64_dwen, rx64_data, rx64_filter
);

input wb_clk;
input clk_125;
input rstn;
input [31:0] timestamp;

input rden;
output [31:0] rx32_data;
output empty;

input rx64_st, rx64_end, rx64_dwen, rx64_filter;
input [63:0] rx64_data;


reg [63:0] fifo_in;
reg [63:0] rx64_data_p;
reg rx64_dwen_p;
reg rx64_end_p, rx64_end_p2;
reg wren;
reg [10:0] wr_addr;
reg [11:0] rd_addr;


// Write Data to FIFO during TLP
always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    fifo_in <= 64'd0;    
    wren <= 1'b0;
    rx64_data_p <= 64'd0;
    rx64_dwen_p <= 1'b0;
    rx64_end_p <= 1'b0;
    rx64_end_p2 <= 1'b0;
  end
  else
  begin  
    rx64_end_p <= rx64_end;
    rx64_end_p2 <= rx64_end_p;
    rx64_dwen_p <= rx64_dwen;
    rx64_data_p <= rx64_data;    
    
    fifo_in <= rx64_st ? {timestamp, timestamp} : rx64_dwen_p ? {32'd0, rx64_data_p[63:32]} : {rx64_data_p[31:0], rx64_data_p[63:32]};
    
    if (rx64_st && ~rx64_filter)
      wren <= 1'b1;
    else if (rx64_end_p2) 
      wren <= 1'b0;
  end
end


pmi_ram_dp #( 
   .pmi_wr_addr_depth    ( 2048 ),
   .pmi_wr_addr_width    ( 11 ),
   .pmi_wr_data_width    ( 64 ),
   .pmi_rd_addr_depth    ( 4096 ),
   .pmi_rd_addr_width    ( 12 ),
   .pmi_rd_data_width    ( 32 ),
   .pmi_regmode          ( "noreg" ),
   .pmi_gsr              ( "enable" ),
   .pmi_resetmode        ( "async" ),
   .pmi_init_file        ( "none" ),
   .pmi_init_file_format ( "binary" ),
   .pmi_family           ( "SC" ),
   .module_type          ( "pmi_ram_dp" )
   ) 
   fifo (
   .WrClockEn    ( 1'b1 ),
   .Data         ( fifo_in ),
   .WrClock      ( clk_125 ),
   .RdClock      ( wb_clk ),
   .WE           ( wren ),
   .RdClockEn    ( rden ), 
   .Reset        ( ~rstn ), 
   .WrAddress    ( wr_addr ),
   .RdAddress    ( rd_addr ),
   .Q            ( rx32_data )
   );

always @(negedge rstn or posedge clk_125)
begin
  if (~rstn)
  begin     
     wr_addr <= 11'd0;     
  end
  else 
  begin          
     if (wren && (wr_addr!=11'b11111111111)) 
     begin
        wr_addr <= wr_addr + 1;
     end     
  end
end


always @(negedge rstn or posedge wb_clk)
begin
  if (~rstn)
  begin     
     rd_addr <= 12'd0;     
  end
  else 
  begin          
     if (rden && ~empty && (rd_addr!=12'hFFF)) 
     begin
        rd_addr <= rd_addr + 1;
     end     
  end
end

assign empty = rd_addr == {wr_addr, 1'b0};

                  


endmodule

