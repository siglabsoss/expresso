// $Id: sfif_tx_fifo.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_tx_fifo(wb_clk, clk_125, rstn, rprst, 
                    tx32_st, tx32_end, tx32_dwen, tx32_nlfy, tx32_data, tx32_dv,
                    tx32_ctrl, 
                    tx32_nph, tx32_pd, tx32_ph, tx32_tag, tx32_tag_cplds, tx32_mrd,
                    tx64_st, tx64_end, tx64_dwen, tx64_nlfy, tx64_data, 
                    tx64_nph, tx64_pd, tx64_ph, tx64_tag, tx64_tag_cplds,
                    tx64_req, tx_rdy, tx_val,                    
                    
                    empty, credit_read, data_read, cr_avail, tag_avail
                    
);

input wb_clk;
input clk_125;
input rstn;
input rprst;

input tx32_st, tx32_end, tx32_dwen, tx32_nlfy, tx32_dv, tx32_ctrl;
input [31:0] tx32_data;
input [3:0] tx32_pd;
input tx32_nph, tx32_ph;
input [4:0] tx32_tag;
input [3:0] tx32_tag_cplds;
input tx32_mrd; 

output tx64_st, tx64_end, tx64_dwen, tx64_nlfy;
output [63:0] tx64_data;
output [3:0] tx64_pd;
output tx64_nph, tx64_ph;
output [4:0] tx64_tag;
output [3:0] tx64_tag_cplds;
output empty;
input credit_read, data_read, cr_avail, tag_avail;
output tx64_req;
input tx_rdy;
input tx_val;

reg tx32_dv_p;
reg tx64_eop_p, tx64_st_p;
wire cr_empty;
wire tx64_eop_i;

reg t;


wire [39:0] fifo_in;
wire [79:0] fifo_out_p;
reg [79:0] fifo_out;
wire [14:0] cr_out;

wire fifo_eop;

reg [10:0] rd_addr;
reg [11:0] wr_addr;
reg [9:0] cr_wr_addr, cr_rd_addr;


reg rden;
reg rden_p;


// Write Data to FIFO on rising edge of tx32_dv (data valid)
always @(posedge wb_clk or negedge rstn)
begin
  if (~rstn)
  begin
    tx32_dv_p <= 1'b0;            
    t <= 1'b0;
  end
  else
  begin
    tx32_dv_p <= tx32_dv;                 
    if (~tx32_dv_p && tx32_dv && tx32_ctrl) t <= ~t;    
    
  end
end

assign wren  = ~tx32_dv_p && tx32_dv && ~tx32_ctrl;     
assign cr_wren  = ~tx32_dv_p && tx32_dv && tx32_ctrl && t;     
assign cr_rden = credit_read || (tx64_st && ~cr_empty);

assign fifo_in = {1'b0, 2'b00, 1'b0, tx32_nlfy, tx32_dwen, tx32_st, tx32_end, tx32_data};

pmi_ram_dp #( 
   .pmi_wr_addr_depth    ( 1024 ),
   .pmi_wr_addr_width    ( 10 ),
   .pmi_wr_data_width    ( 15 ),
   .pmi_rd_addr_depth    ( 1024 ),
   .pmi_rd_addr_width    ( 10 ),
   .pmi_rd_data_width    ( 15 ),
   .pmi_regmode          ( "noreg" ),
   .pmi_gsr              ( "enable" ),
   .pmi_resetmode        ( "async" ),
   .pmi_init_file        ( "none" ),
   .pmi_init_file_format ( "binary" ),
   .pmi_family           ( "SC" ),
   .module_type          ( "pmi_ram_dp" )
   ) 
   cr_fifo (
   .WrClockEn    ( 1'b1 ),
   .Data         ( {tx32_mrd, tx32_tag, tx32_tag_cplds, tx32_pd, tx32_ph} ),
   .WrClock      ( wb_clk ),
   .RdClock      ( clk_125 ),
   .WE           ( cr_wren ),
   .RdClockEn    ( cr_rden ), 
   .Reset        ( ~rstn ), 
   .WrAddress    ( cr_wr_addr ),
   .RdAddress    ( cr_rd_addr ),
   .Q            ( cr_out )
   );
    

pmi_ram_dp #( 
   .pmi_wr_addr_depth    ( 4096 ),
   .pmi_wr_addr_width    ( 12 ),
   .pmi_wr_data_width    ( 40 ),
   .pmi_rd_addr_depth    ( 2048 ),
   .pmi_rd_addr_width    ( 11 ),
   .pmi_rd_data_width    ( 80 ),
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
   .WrClock      ( wb_clk ),
   .RdClock      ( clk_125 ),
   .WE           ( wren ),
   .RdClockEn    ( tx_val ), 
   .Reset        ( ~rstn ), 
   .WrAddress    ( wr_addr ),
   .RdAddress    ( rd_addr ),
   .Q            ( fifo_out_p )
   );

always @(negedge rstn or posedge clk_125)
begin
  if (~rstn)
  begin     
     rd_addr <= 11'd0;     
     cr_rd_addr <= 10'd0;
    
  end
  else 
  begin          
     if (rprst)
     begin
       rd_addr <=11'd0;
       cr_rd_addr <= 10'd0;
     end
     else
     begin
       if (tx_val & rden & ~fifo_eop && (rd_addr!=11'b11111111111)) 
       begin
          rd_addr <= rd_addr + 1;          
       end     
   
       
         
       
       if (cr_rden && (cr_rd_addr!=10'b1111111111)) 
       begin
          cr_rd_addr <= cr_rd_addr + 1;
       end     
     end 
  end
end    

always @(negedge rstn or posedge wb_clk)
begin
  if (~rstn)
  begin     
     wr_addr <= 12'd0;     
     cr_wr_addr <= 10'd0;
  end
  else 
  begin          
     if (wren && (wr_addr!=12'hFFF)) 
     begin
        wr_addr <= wr_addr + 1;
     end     
     
     if (cr_wren && (cr_wr_addr!=10'b1111111111)) 
     begin
        cr_wr_addr <= cr_wr_addr + 1;
     end     
     
     
  end
end    

assign cr_empty = cr_wr_addr == cr_rd_addr;
assign empty = wr_addr[11:1] == rd_addr[10:0];
                
                
// Read Side of FIFO bit orientation.  Due to the 32 to 64 bit bus sizing the fifo_in
// bus will provide two buses on the fifo_out.  Here is how the bits are mapped.

// Bits
//71:40 data
//31:0  data
//32 || 72 end
//33       st (always on bit 33 based on driver
//34 || 74 dwen
//35 || 75 nlfy
//36 || 76 
//37 || 77 
//38 || 78 
//39 || 79 ctrl


// ctrl 
// 0    - ph
// 4:1  - pd
// 8:5  - tag_cplds
// 13:9 - tag
// 14   - mrd
     

assign tx64_data = {fifo_out[31:0], fifo_out[71:40]};
assign tx64_eop_i = fifo_out[32] || fifo_out[72]; 
assign tx64_st_i = fifo_out[33] || fifo_out[73]; 
assign tx64_dwen = fifo_out[34] || fifo_out[74];
assign tx64_nlfy = fifo_out[35] || fifo_out[75];

assign tx64_ph   = cr_out[0]   ;
assign tx64_pd   = cr_out[4:1] ;
assign tx64_tag_cplds = cr_out[8:5]   ;
assign tx64_tag  = cr_out[13:9];
assign tx64_mrd  = cr_out[14];
assign tx64_nph  = cr_out[14];


assign tx64_req = ~empty && data_read && cr_avail && ((tx64_mrd && tag_avail) || ~tx64_mrd);
assign tx64_st = tx64_st_i & ~tx64_st_p;
assign tx64_end = tx64_eop_i & ~tx64_eop_p ;


assign fifo_eop = fifo_out_p[32] || fifo_out_p[72];

// produce pulse for st and end 
always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    tx64_eop_p <= 1'b0;          
    tx64_st_p <= 1'b0;    
    rden <= 1'b0;
     fifo_out <= 80'd0;
     rden_p <= 1'b0;
     
    
  end
  else 
    if (tx_val)
    begin    
      tx64_eop_p <= tx64_eop_i;          
      tx64_st_p <= tx64_st_i;    
      
      if (tx_rdy & ~rden)
        rden <= 1'b1;      
      else if (fifo_eop)
        rden <= 1'b0;
      
      rden_p <= rden;
       if (rden & rden_p)
         fifo_out <= fifo_out_p;
     
      
    end    
    
    
    
   
end


endmodule

