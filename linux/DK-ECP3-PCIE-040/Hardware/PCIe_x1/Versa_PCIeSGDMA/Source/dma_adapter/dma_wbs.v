// $Id: dma_wbs.v,v 1.1.1.1 2008/07/01 17:34:23 jfreed Exp $

module dma_wbs(wb_clk_i, wb_rst_i,
            wb_dat_i, wb_adr_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,            
            wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
            burst_len, active_ch, requestor_id,
            dma_req, dma_ack,
            enable,
            c_pd, c_nph, c_ph, tx_dwen, tx_nlfy, tx_end, tx_st, 
            tx_data, tx_dv, tx_cv, tx_full,
            
            ch1_rdy, ch1_size, ch1_rden, ch1_pending,
            ch_data, ch_dv,
            debug
);

input         wb_clk_i;
input         wb_rst_i;

input  [63:0] wb_dat_i;
input  [31:0] wb_adr_i;
input         wb_cyc_i;
input         wb_lock_i;
input   [7:0] wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [63:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o; 
output [1:0]  dma_req;
input [1:0]   dma_ack;

input [15:0] burst_len;
input [1:0] active_ch;
input [15:0] requestor_id;
input enable;

output [4:0]  c_pd;
output c_nph;
output c_ph;
output tx_dwen;
output tx_nlfy;
output tx_end;
output tx_st;
output tx_dv;
output tx_cv;
input tx_full;
output [63:0] tx_data;

input ch1_rdy;
output [11:0] ch1_size;
output ch1_rden;
output ch1_pending;
input [63:0] ch_data;
input ch_dv;

output [31:0] debug;

reg wb_ack_wr;
reg wbm_wait;
reg wb_rty_o;
reg wb_err_o; 


reg [4:0] c_pd;
reg c_nph;
reg c_ph;
reg tx_dwen;
reg tx_nlfy;
reg tx_end;
reg tx_st;

reg [63:0] tx_data;
reg [63:0] wb_dat_i_p, wb_dat_i_p2;
reg wb_stb_i_p;
reg tx_dv;
reg tx_cv;

reg [11:0] ch1_size;
reg ch1_rden_int, ch1_rden_p;

wire wb_ack_rd;

reg [3:0] sm;
parameter IDLE = 3'b000,          
          WRITE = 3'b001,
          WRITE2 = 3'b010,          
          
          READ = 3'b100,
          READ_DATA = 3'b101;          


reg [9:0] len;

reg ch1_pending;

assign rd = ~wb_we_i && wb_cyc_i && wb_stb_i;    
assign wr = wb_we_i && wb_cyc_i && wb_stb_i;
assign wb_ack_o = (wb_ack_wr || wb_ack_rd) && wb_stb_i_p;
assign wb_ack_rd = (ch1_rden | ch1_rden_p) && ch_dv;
                                                     
assign ch1_rden = ch1_rden_int;  // && wb_stb_i;           
assign wb_dat_o = ch_data;                            

assign dma_req = {ch1_rdy, 1'b0};



always @(posedge wb_clk_i or posedge wb_rst_i)
begin
  if (wb_rst_i)
  begin    
    wb_dat_i_p <= 64'd0;
    wb_dat_i_p2 <= 64'd0;
    wb_stb_i_p <= 1'b0;
    wb_ack_wr <= 1'b0;
    wbm_wait <= 1'b0;
    wb_rty_o <= 1'b0;
    wb_err_o <= 1'b0; 
    
    sm <= IDLE;
    len <= 10'd0;
    
    tx_dv <= 1'b0;  
    tx_cv <= 1'b0;      
    c_pd <= 5'd0;
    c_nph <= 1'b0;
    c_ph <= 1'b0;
    tx_dwen <= 1'b0;
    tx_nlfy <= 1'b0;
    tx_end <= 1'b0;    
    tx_st <= 1'b0; 
    tx_data <= 64'd0;
    
    ch1_pending <= 1'b0;
    ch1_size <= 12'd0;
    
    ch1_rden_int <= 1'b0;
    ch1_rden_p <= 1'b0;
       
    
  end
  else
  begin    
    tx_dv <= 1'b0;    
    
    ch1_rden_p <= ch1_rden;

    wb_dat_i_p2 <= wb_dat_i;
    wb_dat_i_p <= wb_dat_i_p2;
    
    wb_stb_i_p <= wb_stb_i;
    
    wb_err_o <= 1'b0; 
    
    case (sm)
      IDLE:
      begin        
      
        if (wb_cyc_i && wb_stb_i)
        begin
          if (wr)
          begin
           if (~tx_full)
           begin
             wb_ack_wr <= wb_stb_i;   
              
             c_ph <= 1'b1;
             c_nph <= 1'b0;
             c_pd <= burst_len[8:4];
             tx_dv <= 1'b1;   
             tx_cv <= 1'b1;     
             
             len[9:0] <= burst_len[11:2];                     
             
             
             tx_data <= { 1'b0,              // R
                      2'b10,             // Fmt
                      5'b00000,          // Type
                      1'b0,              // R
                      3'b000,            // TC
                      4'b0000,           // R             
                      1'b0,              // TD
                      1'b0,              // EP
                      2'b10,             // Attr
                      2'b00,             // R
                      burst_len[11:2],   // Length
                      requestor_id,      // Requestor ID
                      8'd0,              // Tag 0 for writes
                      4'b1111,           // Last DW BE
                      4'b1111            // 1st DW BE
                    };
             tx_st <= 1'b1;
             tx_end <= 1'b0;               
             tx_dwen <= 1'b0;
             
             sm <= WRITE;
        
              
            end    
            else // tx fifo full
            begin
              wb_ack_wr <= 1'b0;                
              tx_end <= 1'b0; 
              tx_dv <= 1'b0;   
              tx_cv <= 1'b0;                                 
              sm <= IDLE;            
            end        
          end // wr  
          else // rd
          begin 
                        
            wb_rty_o <= 1'b0;   
            
            if (ch1_pending && ch1_rdy)
            begin                
              len <= burst_len[11:2]; // divided by 4
              sm <= READ_DATA;                
            end
            else if (ch1_pending && ~ch1_rdy)
            begin
              // wait longer, not ready yet
              wb_rty_o <= 1'b1;   
            end
            else
            begin
              // first request
              wb_rty_o <= 1'b1;   
              ch1_pending <= 1'b1;
              ch1_size <= burst_len[11:0];
              tx_st <= 1'b1;
              tx_end <= 1'b0;               
              tx_dwen <= 1'b0;
              c_nph <= 1'b1;       
              c_ph <= 1'b0;                         
              tx_dv <= 1'b1;   
              tx_cv <= 1'b1;
              sm <= READ;
            end
                
            // common MRd 
            tx_data <= { 1'b0,              // R
                         2'b00,             // Fmt
                         5'b00000,          // Type
                         1'b0,              // R
                         3'b000,            // TC
                         4'b0000,           // R             
                         1'b0,              // TD
                         1'b0,              // EP
                         2'b10,             // Attr
                         2'b00,             // R
                         burst_len[11:2],   // Length
                         requestor_id,      // Requestor ID
                         8'd1,              // Tag 1 for reads
                         4'b1111,           // Last DW BE
                         4'b1111            // 1st DW BE
                       };
            
          end  // rd
        end // cyc      
        else
        begin
          c_ph <= 1'b0;
          c_pd <= 5'd0;
          c_nph <= 1'b0;
        end          
      end
      WRITE:  //       
      begin
        tx_cv <= 1'b0;
        
        tx_data <= {wb_adr_i[31:2],    // Address[31:2]
                    2'b00,             // R
                    wb_dat_i_p2[63:32]  // Data
                    };        
        
        if (len==10'd1)
        begin               
          
          tx_dwen <= 1'b0;
          tx_st <= 1'b0;
          tx_end <= 1'b1;
          tx_dv <= 1'b1;
          wb_ack_wr <= 1'b0;
          sm <= IDLE;
        end
        else if (wb_stb_i)
        begin            
          tx_dwen <= 1'b0;
          tx_st <= 1'b0;
          tx_end <= 1'b0;            
          tx_dv <= 1'b1;
          len <= len - 1;
          sm <= WRITE2;            
        end            
      end
      WRITE2:  // Length > 1
      begin
        if (len==10'd1)
        begin
          tx_data <= {wb_dat_i[31:0], 32'd0};        
          tx_dwen <= 1'b1;
          tx_end <= 1'b1;            
          wb_ack_wr <= 1'b0;
          sm <= IDLE;
        end
        else if (len==10'd2)
        begin
          tx_data <= { wb_dat_i_p2[31:0], wb_dat_i[63:32]};           
          tx_dwen <= 1'b0;
          tx_end <= 1'b1;
          wb_ack_wr <= 1'b0;
          sm <= IDLE;
        end
        else if (wb_stb_i)
        begin
          tx_data <= { wb_dat_i_p2[31:0], wb_dat_i[63:32]};           
          len[9:0] <= len - 2;
        end
        tx_st <= 1'b0;        
        tx_dv <= 1'b1;
      end
      READ:
      begin
        wb_rty_o <= 1'b0;                       
        tx_data <= { wb_adr_i[31:2], // Address
                     2'b00,          // R               
                     32'd0           // Unused
                   };
        tx_st <= 1'b0;
        tx_end <= 1'b1;               
        tx_dwen <= 1'b1;          
        tx_dv <= 1'b1;   
        tx_cv <= 1'b0;
        sm <= IDLE;
      end
      READ_DATA:
      begin
        if (len==10'd0 && ~wbm_wait)
        begin
          ch1_rden_int <= 1'b0;
          if (~wb_stb_i)
          begin
            sm <= IDLE;                
            ch1_pending <= 1'b0;
          end
        end
        else
        begin  // if stb drops low during transfer then rden needs to wait for 1 clock when stb comes
               // back.  This is so the data left on the bus gets ack'd.            
          if (wbm_wait && wb_stb_i_p)
          begin
            ch1_rden_int <= 1'b1;
            wbm_wait <= 1'b0;
          end
          else if (~wbm_wait && wb_stb_i)
          begin
            ch1_rden_int <= 1'b1;                                          
          end  
          else 
          begin
            wbm_wait <= 1'b1;
            ch1_rden_int <= 1'b0;              
          end            
          
          if (ch1_rden) len <= len - 2;
          
        end     
      end
      default:
      begin
        sm <= IDLE;       
      end
    endcase
  
    
  end // clk
end  



endmodule


