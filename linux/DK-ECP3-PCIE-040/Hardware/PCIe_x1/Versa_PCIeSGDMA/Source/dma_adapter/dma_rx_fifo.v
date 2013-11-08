// $Id: dma_rx_fifo.v,v 1.1.1.1 2008/07/01 17:34:23 jfreed Exp $

module dma_rx_fifo(wb_clk, clk_125, rstn, 
                   rx_st, rx_end, rx_data,
                   active_ch,
                   ch1_rdy, ch1_size, ch1_rden, ch1_pending, 
                   ch_data, ch_dv,
                   cplh_cr, cpld_cr, 
                   unexp_cmpl,
                   debug 
);

input wb_clk;
input clk_125;
input rstn;

input rx_st, rx_end;
input [15:0] rx_data;

input [1:0] active_ch;

output ch1_rdy;
input [11:0] ch1_size;
input ch1_rden;
input ch1_pending;
output [63:0] ch_data;
output ch_dv;
output cplh_cr;
output [7:0] cpld_cr;

output unexp_cmpl;

output [15:0] debug;

reg [8:0] wr_addr1, rd_addr1;

reg [1:0] sm;
reg [10:0] len;
reg [11:0] byte_cnt, ch1_bytes;
reg wren;
reg unexp_cmpl;
reg [15:0] rx_data_p1, rx_data_p2, rx_data_p3, rx_data_p4;
reg wren_p;
reg ch1_rdy;
reg ch1_rdy_p;
wire [64:0] fifo_in, ch1_data;

reg cplh_cr;
reg [7:0] cpld_cr;
reg [2:0] actch;
reg [2:0] word_cnt;
assign debug = {8'd0, ch1_pending, ch1_rdy, ch1_rden, rd_addr1[3:0], ch1_data[64]};


always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    sm <= 2'b00;
    len <= 11'd0;
    byte_cnt <= 12'd0;
    ch1_bytes <= 12'd0;
    wren <= 1'b0;
    wren_p <= 1'b0;
    unexp_cmpl <= 1'b0;
    rx_data_p1 <= 0;    
    rx_data_p2 <= 0;
    rx_data_p3 <= 0;
    rx_data_p4 <= 0;
    ch1_rdy <= 1'b0;
    cplh_cr <= 1'b0;
    cpld_cr <= 8'd0;
    actch <= 3'b000;
    ch1_rdy <= 1'b0;
    word_cnt <= 0;
    
  end
  else
  begin
    rx_data_p1 <= rx_data;
    rx_data_p2 <= rx_data_p1;
    rx_data_p3 <= rx_data_p2;
    rx_data_p4 <= rx_data_p3;
    wren_p <= 1'b0;
  
    case (sm)
    2'b00: 
    begin
      unexp_cmpl <= 1'b0;
      word_cnt   <= 0;
      if (rx_st && (rx_data[15:8]==8'h4a))
         sm <= 2'b01;
    end    
    2'b01: // Write Data to FIFO
    begin    
      if (word_cnt == 0) begin 
         word_cnt <= word_cnt + 1;
         byte_cnt[11:0] <= {rx_data[9:0], 2'b00}; // multiply len by 4 to get bytes (assumes all bytes enabled)
         len <= {rx_data[9:0], 1'b0}; //multiply by 2 for 16bit data bus
      end
      else if (word_cnt == 2) begin
         if (rx_data[15:13]!=3'b000) //only process SC
            sm <= 2'b00;
         else
            word_cnt <= word_cnt + 1;
      end
      else if (word_cnt == 4) begin
         actch <= rx_data[10:8]; // Tag field
         if (rx_data[10:8] == 3'b001) begin
            if (ch1_size > 12'd0 && ~ch1_rdy) begin
               ch1_bytes <= byte_cnt + ch1_bytes;      
               sm <= 2'b10; // write
               word_cnt <= 0;
            end
            else begin //unexpected completion for this channel  
               unexp_cmpl <= 1'b1;
               sm <= 2'b00;
            end
         end
         else begin
            unexp_cmpl <= 1'b1;
            sm <= 2'b00;
         end
      end
      else
         word_cnt <= word_cnt + 1;
    end
    2'b10: // write data to FIFO
    begin
      word_cnt <= word_cnt + 1;
      if (len==11'd0) begin
        wren <= 1'b0;
        sm <= 2'b00;
        wren_p <= 1'b1;
      end 
      else begin
        wren <= word_cnt[1:0] == 3;                
        len <= len - 1;
      end
    end
    default:
    begin
      sm <= 2'b00; 
    end
    endcase 
    
    if (ch1_bytes > 12'd0 && ch1_bytes == ch1_size && ch1_pending) begin
       if (wren_p)
          ch1_rdy <= 1'b1;
    end
    else
       ch1_rdy <= 1'b0;
    
    ch1_rdy_p <= ch1_rdy;
    
      
    
    // After a channel is complete, release the credits
    if (ch1_rdy_p && ~ch1_rdy) 
    begin
      cplh_cr <= 1'b1;
      cpld_cr <= ch1_bytes[9:2];      
      ch1_bytes <= 12'd0;      
    end    
    else
    begin
      cplh_cr <= 1'b0;      
    end    
    
  
  end // clk_125
end


//assign fifo_in = {wren, rx_data_p[31:0], rx_data[63:32]};
assign fifo_in = {wren, rx_data_p4, rx_data_p3, rx_data_p2, rx_data_p1};
assign wren1 = wren;

always @(negedge rstn or posedge clk_125)
begin
  if (~rstn)
  begin     
     wr_addr1 <= 9'd0;     
  end
  else 
  begin 
     if (ch1_rdy && ~wren1)
       wr_addr1 <= 9'd0;
     else if (wren1) 
     begin
        wr_addr1 <= wr_addr1 + 1;
     end     
     

  end
end


pmi_ram_dp #( 
   .pmi_wr_addr_depth    ( 512 ),
   .pmi_wr_addr_width    ( 9 ),
   .pmi_wr_data_width    ( 65 ),
   .pmi_rd_addr_depth    ( 512 ),
   .pmi_rd_addr_width    ( 9 ),
   .pmi_rd_data_width    ( 65 ),
   .pmi_regmode          ( "reg" ),
   .pmi_gsr              ( "enable" ),
   .pmi_resetmode        ( "async" ),
   .pmi_init_file        ( "none" ),
   .pmi_init_file_format ( "binary" ),
   .pmi_family           ( "SC" ),
   .module_type          ( "pmi_ram_dp" )
   ) 
   fifo1 (
   .WrClockEn    ( 1'b1 ),
   .Data         ( fifo_in ),
   .WrClock      ( clk_125 ),
   .RdClock      ( wb_clk ),
   .WE           ( wren1 || wren_p ),
   .RdClockEn    ( ch1_rden ), 
   .Reset        ( ~rstn ), 
   .WrAddress    ( wr_addr1 ),
   .RdAddress    ( rd_addr1 ),
   .Q            ( ch1_data )
   );


assign ch_data = ch1_data[63:0];
assign ch_dv = ch1_data[64];      




always @(negedge rstn or posedge wb_clk)
begin
  if (~rstn)
  begin     
     rd_addr1 <= 10'd0;          
  end
  else 
  begin         
  
     if (~ch1_pending)
     begin
       rd_addr1 <= 10'd0;     
     end
     if (ch1_rden) 
     begin
       rd_addr1 <= rd_addr1 + 1;
     end              
     
     
  end
end



endmodule




