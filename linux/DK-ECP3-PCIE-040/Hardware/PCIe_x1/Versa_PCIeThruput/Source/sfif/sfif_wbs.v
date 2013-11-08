// $Id: sfif_wbs.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_wbs(wb_clk_i, wb_rst_i,
            wb_dat_i, wb_adr_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,            
            wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
            tx_cycles, lpbk, loop, run, reset, enable, rx_filter,
            c_npd, c_pd, c_nph, c_ph, tag, tag_cplds, mrd,
            tx_dwen, tx_nlfy, tx_end, tx_st, tx_ctrl,
            ipg_cnt, tx_data, tx_dv, rx_data, elapsed_cnt, tx_tlp_cnt, rx_tlp_cnt, rx_empty, rx_data_read,
            credit_wait_p_cnt, credit_wait_np_cnt, rx_tlp_timestamp
          
);

input         wb_clk_i;
input         wb_rst_i;

input  [15:0] wb_dat_i;
input  [5:0]  wb_adr_i;
input         wb_cyc_i;
input         wb_lock_i;
input   [1:0] wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [15:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o; 

output [15:0] tx_cycles;
output lpbk;
output loop;
output run;
output reset;
output enable;
output rx_filter;

output [3:0] c_npd, c_pd;
output c_nph;
output c_ph;
output [4:0] tag;
output [3:0] tag_cplds;
output mrd;
output tx_dwen;
output tx_nlfy;
output tx_end;
output tx_st;
output tx_dv;
output tx_ctrl;
output [15:0] ipg_cnt;        
output [31:0] tx_data;
input  [31:0] rx_data, elapsed_cnt, tx_tlp_cnt, rx_tlp_cnt, credit_wait_p_cnt, credit_wait_np_cnt, rx_tlp_timestamp;
input rx_empty;
output rx_data_read;


reg [15:0] wb_dat_o;
reg wb_ack_o;

reg [15:0] tx_cycles;
reg lpbk;
reg loop;
reg run;
reg reset;
reg enable;


reg [3:0] c_npd, c_pd;
reg c_nph;
reg c_ph;
reg [4:0] tag;
reg [3:0] tag_cplds;
reg mrd;
reg tx_dwen;
reg tx_nlfy;
reg tx_end;
reg tx_st;
reg [15:0] ipg_cnt;        
reg [31:0] tx_data;
reg tx_dv;
reg tx_ctrl;
reg rx_data_read, data_read;
reg rx_filter;
reg dummy;
reg [15:0] temp_data;
assign rd = ~wb_we_i && wb_cyc_i && wb_stb_i;    
assign wr = wb_we_i && wb_cyc_i && wb_stb_i;


always @(posedge wb_clk_i or posedge wb_rst_i)
begin
  if (wb_rst_i)
  begin
    wb_dat_o <= 0;
    wb_ack_o <= 1'b0;
    tx_dv <= 1'b0;    
    rx_data_read <= 1'b0;
    data_read <= 1'b0;
    tx_cycles <= 16'd0;
    ipg_cnt <= 16'd0;
    c_npd <= 4'd0;
    c_pd <= 4'd0;
    c_nph <= 1'b0;
    c_ph <= 1'b0;
    tag <= 5'd0;
    tag_cplds <= 4'd0;
    mrd <= 1'b0;
    tx_dwen <= 1'b0;
    tx_nlfy <= 1'b0;
    tx_end <= 1'b0;    
    tx_st <= 1'b0; 
    tx_data <= 32'd0;
    tx_ctrl <= 1'b0;
    enable <= 1'b0;
    reset <= 1'b0;
    run <= 1'b0;
    loop <= 1'b0;
    lpbk <= 1'b0;    
    rx_filter <= 1'b0;
    dummy <= 1'b0;
    temp_data <= 0;
  end
  else
  begin
    wb_ack_o   <= wb_cyc_i & wb_stb_i & (~ wb_ack_o);
    tx_dv <= 1'b0;    
    
    // issue a read to the rx_fifo once the rx data has been read
    if (data_read && ~wb_cyc_i)
    begin
      rx_data_read <= 1'b1;
      data_read <= 1'b0;
    end
    else
      rx_data_read <= 1'b0;
      
  
    if (wb_cyc_i)
    begin
    
      case (wb_adr_i)
        //6'b000000: // 0x0  Control
        //begin
        //  if (rd)
        //  begin
        //    wb_dat_o <= {tx_cycles, 9'd0, rx_filter, rx_empty, lpbk, loop, run, reset, enable};
        //  end
        //  else if (wr)
        //  begin
        //    tx_cycles[15:8] <= wb_sel_i[0] ? wb_dat_i[31:24] : tx_cycles[15:8];
        //    tx_cycles[7:0]  <= wb_sel_i[1] ? wb_dat_i[23:16] : tx_cycles[7:0];
        //    {rx_filter, lpbk, loop, run, reset, enable} <= wb_sel_i[3] ? {wb_dat_i[6],wb_dat_i[4:0]} : {rx_filter, lpbk, loop, run, reset, enable};
        //  end          
        //end
      6'h00: begin
         if (rd) wb_dat_o <= {9'd0, rx_filter, rx_empty, lpbk, loop, run, reset, enable};
         else if (wr) begin
            {rx_filter, lpbk, loop, run, reset, enable} <= wb_sel_i[1] ? {wb_dat_i[6],wb_dat_i[4:0]} : {rx_filter, lpbk, loop, run, reset, enable};
         end
      end
      6'h02: begin
         if (rd) wb_dat_o <= tx_cycles;
         else if (wr) begin
            tx_cycles[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : tx_cycles[15:8];
            tx_cycles[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : tx_cycles[7:0];
         end
      end              
        
        //6'b000100: // 0x4 Tx TLP Control
        //begin
        //  if (rd)                
        //    wb_dat_o <= {6'd0, tag, tag_cplds, mrd, tx_ctrl, 1'b0, c_npd, c_nph, c_pd, c_ph, tx_dwen, tx_nlfy, tx_end, tx_st};
        //  else if (wr)
        //  begin                                                                        
        //    {tag[4:3]}                                         <= wb_sel_i[0] ? wb_dat_i[31:24] : {tag[4:3]};                                      
        //    {tag[2:0],tag_cplds,mrd}                           <= wb_sel_i[1] ? wb_dat_i[23:16] : {tag[2:0], tag_cplds, mrd};
        //    {tx_ctrl, dummy, c_npd, c_nph, c_pd[3]}            <= wb_sel_i[2] ? wb_dat_i[15:8]  : {tx_ctrl, 1'b0, c_npd, c_nph, c_pd[3]};
        //    {c_pd[2:0], c_ph, tx_dwen, tx_nlfy, tx_end, tx_st} <= wb_sel_i[3] ? wb_dat_i[7:0]   : {c_pd[2:0], c_ph, tx_dwen, tx_nlfy, tx_end, tx_st};            
        //  end
        //end
      6'h04: begin
         if (rd) wb_dat_o <= {tx_ctrl, 1'b0, c_npd, c_nph, c_pd, c_ph, tx_dwen, tx_nlfy, tx_end, tx_st};
         else if (wr) begin
            {tx_ctrl, dummy, c_npd, c_nph, c_pd[3]}            <= wb_sel_i[0] ? wb_dat_i[15:8]  :{tx_ctrl, 1'b0, c_npd, c_nph, c_pd[3]};
            {c_pd[2:0], c_ph, tx_dwen, tx_nlfy, tx_end, tx_st} <= wb_sel_i[1] ? wb_dat_i[7:0]   : {c_pd[2:0], c_ph, tx_dwen, tx_nlfy, tx_end, tx_st};
         end
      end
      6'h06: begin
         if (rd) wb_dat_o <= {6'd0, tag, tag_cplds, mrd};
         else if (wr) begin
            {tag[4:3]}               <= wb_sel_i[0] ? wb_dat_i[15:8] : {tag[4:3]};
            {tag[2:0],tag_cplds,mrd} <= wb_sel_i[1] ? wb_dat_i[7:0] : {tag[2:0], tag_cplds, mrd};
         end
      end                      
        //6'b001000:  // 0x8  Tx IPG
        //begin
        //  if (rd)            
        //    wb_dat_o <= {16'd0, ipg_cnt};
        //  else if (wr)
        //  begin
        //    ipg_cnt[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8] : ipg_cnt[15:8];      
        //    ipg_cnt[7:0]  <= wb_sel_i[3] ? wb_dat_i[7:0]  : ipg_cnt[7:0];      
        //  end  
        //end  
      6'h08: begin
         if (rd) wb_dat_o <= ipg_cnt;
         else if (wr) begin
            ipg_cnt[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : ipg_cnt[15:8]; 
            ipg_cnt[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : ipg_cnt[7:0];  
         end
      end
      6'h0a: begin
         if (rd) wb_dat_o <= 0;
      end                      
        //6'b001100: // 0xc Tx Data
        //begin
        //  if (rd)            
        //    wb_dat_o <= tx_data;
        //  else if (wr)
        //  begin
        //    tx_data[31:24] <= wb_sel_i[0] ? wb_dat_i[31:24] : tx_data[31:24];
        //    tx_data[23:16] <= wb_sel_i[1] ? wb_dat_i[23:16] : tx_data[23:16];
        //    tx_data[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8]   : tx_data[15:8];
        //    tx_data[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0] :     tx_data[7:0];      
        //    tx_dv <= 1'b1;
        //  end        
        //end
      6'h0c: begin
         if (rd) wb_dat_o <= tx_data[15:0];
         else if (wr) begin
            tx_data[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : tx_data[15:8];
            tx_data[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : tx_data[7:0];
         end
      end
      6'h0e: begin
         if (rd) wb_dat_o <= tx_data[31:16];
         else if (wr) begin
            tx_data[31:24] <= wb_sel_i[0] ? wb_dat_i[15:8] : tx_data[31:24];
            tx_data[23:16] <= wb_sel_i[1] ? wb_dat_i[7:0]  : tx_data[23:16];
            tx_dv <= 1'b1;
         end
      end      
        //6'b010000: // 0x10 Rx TLP
        //begin
        //  if (rd)
        //  begin
        //    wb_dat_o <= rx_data;
        //    data_read <= 1'b1;            
        //  end
        //end
      6'h10: begin
         if (rd) begin wb_dat_o <= rx_data[15:0]; temp_data <= rx_data[31:16]; data_read <= 1'b1; end
      end
      6'h12: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end      
        //6'b010100: // 0x14 Elapsed Count
        //begin
        //  if (rd)
        //    wb_dat_o <= elapsed_cnt;                  
        //end     
      6'h14: begin
         if (rd) begin wb_dat_o <= elapsed_cnt[15:0]; temp_data <= elapsed_cnt[31:16]; end
      end
      6'h16: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end            
        
        //6'b011000: // 0x18 Tx TLP Count
        //begin
        //  if (rd)
        //    wb_dat_o <= tx_tlp_cnt;                  
        //end       
      6'h18: begin
         if (rd) begin wb_dat_o <= tx_tlp_cnt[15:0]; temp_data <= tx_tlp_cnt[31:16]; end
      end
      6'h1a: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end                     
        //6'b011100: // 0x1C Rx TLP Count
        //begin
        //  if (rd)
        //    wb_dat_o <= rx_tlp_cnt;                  
        //end             
      6'h1c: begin
         if (rd) begin wb_dat_o <= rx_tlp_cnt[15:0]; temp_data <= rx_tlp_cnt[31:16]; end
      end
      6'h1e: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end                             
        //6'b100000: // 0x20 Credit Wait P Count
        //begin
        //  if (rd)
        //    wb_dat_o <= credit_wait_p_cnt;                  
        //end                     
      6'h20: begin
         if (rd) begin wb_dat_o <= credit_wait_p_cnt[15:0]; temp_data <= credit_wait_p_cnt[31:16]; end
      end
      6'h22: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end                        
        //6'b100100: // 0x24 Rx TLP Timestamp
        //begin
        //  if (rd)
        //    wb_dat_o <= rx_tlp_timestamp;                  
        //end          
      6'h24: begin
         if (rd) begin wb_dat_o <= rx_tlp_timestamp[15:0]; temp_data <= rx_tlp_timestamp[31:16]; end
      end
      6'h26: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end             
        //6'b101000: // 0x28 Credit Wait NP Count
        //begin
        //  if (rd)
        //    wb_dat_o <= credit_wait_np_cnt;                  
        //end                     
      6'h28: begin
         if (rd) begin wb_dat_o <= credit_wait_np_cnt[15:0]; temp_data <= credit_wait_np_cnt[31:16]; end
      end
      6'h2a: begin
         if (rd) begin wb_dat_o <= temp_data; end
      end            
        default:
          wb_dat_o <= 0;    
      endcase
    
    end // cyc
  end //clk
end


assign wb_err_o = 1'b0;
assign wb_rty_o = 1'b0;


endmodule


