// $Id: wbs_gpio.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

`define VERSA
module wbs_gpio(wb_clk_i, wb_rst_i,
          wb_dat_i, wb_adr_i, wb_cti_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,          
          wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
          switch_in, led_out, 
          dma_req, dma_ack,
          ca_pd, ca_nph, dl_up,
          int_out, ebr_filter, cb_rst
);
          
input         wb_clk_i;
input         wb_rst_i;

input  [15:0] wb_dat_i;
input  [8:0] wb_adr_i;
input  [2:0] wb_cti_i;
input         wb_cyc_i;
input         wb_lock_i;
input   [1:0] wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [15:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o; 

input [7:0] switch_in;
output [13:0] led_out;
output [4:0] dma_req;
input  [4:0] dma_ack;
input [12:0] ca_pd;
input [8:0] ca_nph;
input dl_up;
output int_out;
output [31:0] ebr_filter;
output cb_rst;

reg [31:0] scratch_pad;
reg cnt_run, cnt_reload;
reg [31:0] cnt, cnt_reload_val;
reg cnt0;
reg [13:0] led_out;
reg [15:0] wb_dat_local;
reg [4:0] dma_req_wr, dma_req_p, dma_req;
reg dma_wr;
reg [4:1] dma_rd;
reg [31:0] dma_wr_cnt, dma_rd_cnt;
reg [7:0] int_test0, int_test1;
reg int_en, int_test_mode;
reg [31:0] int_status, int_mask;
reg int_i, int_out;
reg [12:0] init_ca_pd;
reg [8:0] init_ca_nph;
reg dl_up_p;
reg [31:0] ebr_filter;
reg cb_rst;
reg [15:0] temp_data;
reg wb_ack_o;

reg wr_delayed; //For VERSA



assign rd = ~wb_we_i && wb_cyc_i && wb_stb_i;    
assign wr = wb_we_i && wb_cyc_i && wb_stb_i;

assign wr_use = (!wr_delayed) && wr; //for VERSA
   
// Need to pipeline the write side to allow the read side time to read data
always @(posedge wb_rst_i or posedge wb_clk_i)
begin
  if (wb_rst_i)
  begin
    scratch_pad <= 32'd0;
    led_out <= 14'd0;
    wb_dat_local <= 32'd0;
    cnt_run <= 1'b0;
    cnt_reload <= 1'b0;
    cnt_reload_val <= 32'd0;
    cnt <= 32'd0;
    cnt0 <= 1'b0;
    dma_req <= 5'd0;
    dma_req_wr <= 5'd0;
    dma_req_p <= 5'd0;    
    dma_wr <= 1'b0;
    dma_rd <= 4'b0000;
    
    
    dma_wr_cnt <= 32'd0;
    dma_rd_cnt <= 32'd0;
    
    ebr_filter <= 32'd0;
    cb_rst <= 1'b0;
    
    int_test0 <= 8'd0;
    int_test1 <= 8'd0;
    int_en <= 1'b0;
    int_test_mode <= 1'b0;
    int_status <= 32'd0;
    int_mask <= 32'd0;
    int_i <= 1'b0;
    int_out <= 1'b0;     
    
    init_ca_pd <= 13'd0;
    init_ca_nph <= 9'd0;    
    dl_up_p <= 1'b0;
    temp_data <= 0;		
	
    wr_delayed <= 0;//for VERSA
  end
  else
  begin
    wr_delayed<=wr; 
 
  if (wb_cyc_i)
  begin

     
    

    case (wb_adr_i)
      //9'h000: // 0x0
      //begin
      //  if (rd)
      //  begin
      //    wb_dat_local <= 32'h12043010;      
      //  end
      //end
      9'h000: begin
         if (rd) wb_dat_local <= 16'h3010;
      end
      9'h002: begin
         if (rd) wb_dat_local <= 16'h1204;
      end      
      //9'h004: // 0x4
      //begin
      //  if (rd)    
      //    // bit10 was this way wb_dat_local <= {scratch_pad[7:0], scratch_pad[15:8], scratch_pad[23:16], scratch_pad[31:24]};
      //    wb_dat_local <= scratch_pad;
      //  else if (wr)
      //  begin
      //    scratch_pad[31:24] <= wb_sel_i[0] ? wb_dat_i[31:24] : scratch_pad[31:24];
      //    scratch_pad[23:16] <= wb_sel_i[1] ? wb_dat_i[23:16] : scratch_pad[23:16];
      //    scratch_pad[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8]   : scratch_pad[15:8];
      //    scratch_pad[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0] :     scratch_pad[7:0];      
      //  end
      //end
      9'h004: begin
         if (rd) wb_dat_local <= scratch_pad[15:0];
         else if (wr) begin
            scratch_pad[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : scratch_pad[15:8];
            scratch_pad[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : scratch_pad[7:0];
         end
      end
      9'h006: begin
         if (rd) wb_dat_local <= scratch_pad[31:16];
         else if (wr) begin
            scratch_pad[31:24] <= wb_sel_i[0] ? wb_dat_i[15:8] : scratch_pad[31:24];
            scratch_pad[23:16] <= wb_sel_i[1] ? wb_dat_i[7:0]  : scratch_pad[23:16];
         end
      end   

   
      //9'h008:  // 0x8
      //begin
      //  if (rd)          
      //    wb_dat_local <= {switch_in, 8'h00, led_out};
      //  else if (wr)
      //  begin
      //    led_out[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8] : led_out[15:8];      
      //    led_out[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0]  : led_out[7:0];      
      //  end  
      //end


 
      
	  ////////////////////////////////////////////////////////////////////////////
	
`ifdef VERSA   
	  
	  9'h008: begin
		  
	 
		
         if (rd) wb_dat_local <= {led_out[13:4],led_out[3],led_out[3],led_out[2:1],led_out[0],led_out[0]};
			 
         else if (wr_use) begin
			 	
	        if (led_out[0]&led_out[3]) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]&wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]&wb_dat_i[1])}  : led_out[5:0]; end
		else if  (led_out[0]&(!led_out[3])) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]|wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]&wb_dat_i[1])}  : led_out[5:0]; end
		else if ((!led_out[0])&led_out[3]) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]&wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]|wb_dat_i[1])}  : led_out[5:0]; end
				
	      	else if ((!led_out[0])&(!led_out[3])) begin
	    led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]|wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]|wb_dat_i[1])}  : led_out[5:0]; end	
         end
	  end // case: 9'h008


`else

           
  9'h008: begin
         if (rd) wb_dat_local <= led_out;
         else if (wr) begin
            led_out[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[15:8];
            led_out[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : led_out[7:0];
         end
  end

 `endif
	  /////////////////////////////////////////////////////////////////////////////
	  
      /*9'h008: begin
         if (rd) wb_dat_local <= {led_out[13:4],led_out[3],led_out[3],led_out[2:1],led_out[0],led_out[0]};
         else if (wr) begin
	 if (wb_dat_local[0]&wb_dat_local[1]&wb_dat_local[4]&wb_dat_local[5]) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]&wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]&wb_dat_i[1])}  : led_out[5:0]; end
		else if ((!wb_dat_local[0])&(!wb_dat_local[1])&wb_dat_local[4]&wb_dat_local[5])begin//if (!led_out[0]&led_out[3]) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]&wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]|wb_dat_i[1])}  : led_out[5:0]; end
		else if (wb_dat_local[0]&wb_dat_local[1]&(!wb_dat_local[4])&(!wb_dat_local[5])) begin//if (led_out[0]&(!led_out[3])) begin
            led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]|wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]&wb_dat_i[1])}  : led_out[5:0]; end
				
			else if ((!wb_dat_local[0])&(!wb_dat_local[1])&(!wb_dat_local[4])&(!wb_dat_local[5])) begin// begin
			  led_out[13:6] <= wb_sel_i[0] ? wb_dat_i[15:8] : led_out[13:6];
            led_out[5:0]  <= wb_sel_i[1] ? {wb_dat_i[7:6],(wb_dat_i[5]|wb_dat_i[4]),wb_dat_i[3:2],(wb_dat_i[0]|wb_dat_i[1])}  : led_out[5:0]; end	
         end
      end      */
      9'h00a: begin
         if (rd) wb_dat_local <= {switch_in, 8'h00};
      end            
      //9'h00c: // 0xc
      //begin
      //  if (rd)          
      //    wb_dat_local <= {8'h00, 8'h00, 8'h00, 6'b000000, cnt_reload, cnt_run};
      //  else if (wr)
      //  begin               
      //    cnt_reload  <= wb_sel_i[3] ? wb_dat_i[1]  : cnt_reload;      
      //    cnt_run  <= wb_sel_i[3] ? wb_dat_i[0]  : cnt_run;      
      //  end        
      //end
      9'h00c: begin
         if (rd) wb_dat_local <= {8'h00, 6'b000000, cnt_reload, cnt_run};
         else if (wr) begin
            cnt_reload  <= wb_sel_i[1] ? wb_dat_i[1]  : cnt_reload;
            cnt_run  <= wb_sel_i[1] ? wb_dat_i[0]  : cnt_run;            
         end      
      end
      9'h00e: begin
         if (rd) wb_dat_local <= 0;
      end
      //9'h010: // 0x10
      //begin
      //  if (rd)
      //    wb_dat_local <= cnt;        
      //end
      9'h010: begin
         if (rd) begin wb_dat_local <= cnt[15:0]; temp_data <= cnt[31:16]; end
      end
      9'h012: begin
         if (rd) begin wb_dat_local <= temp_data; end
      end      
      
      //9'h014: // 0x14
      //begin
      //  if (rd)
      //    wb_dat_local <= cnt_reload_val;        
      //  else if (wr)
      //  begin
      //    cnt_reload_val[31:24] <= wb_sel_i[0] ? wb_dat_i[31:24] : cnt_reload_val[31:24];
      //    cnt_reload_val[23:16] <= wb_sel_i[1] ? wb_dat_i[23:16] : cnt_reload_val[23:16];
      //    cnt_reload_val[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8]   : cnt_reload_val[15:8];
      //    cnt_reload_val[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0] :     cnt_reload_val[7:0];      
      //  end        
      //end  
      9'h014: begin
         if (rd) wb_dat_local <= cnt_reload_val[15:0];
         else if (wr) begin
            cnt_reload_val[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : cnt_reload_val[15:8];
            cnt_reload_val[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : cnt_reload_val[7:0];
         end
      end
      9'h016: begin
         if (rd) wb_dat_local <= cnt_reload_val[31:16];
         else if (wr) begin
            cnt_reload_val[31:24] <= wb_sel_i[0] ? wb_dat_i[15:8] : cnt_reload_val[31:24];
            cnt_reload_val[23:16] <= wb_sel_i[1] ? wb_dat_i[7:0]  : cnt_reload_val[23:16];
         end
      end            
      
      //9'h018: // 0x18 DMA Req/Ack
      //begin
      //  if (rd)
      //    wb_dat_local <= {16'd0, 3'd0, dma_ack[4:0], 8'd0};        
      //  else if (wr)
      //  begin          
      //    dma_req[4:0] <= wb_sel_i[3] ? wb_dat_i[4:0] :  5'd0;
      //  end        
      //end   
      9'h018: begin
         if (rd) wb_dat_local <= {3'd0, dma_ack[4:0], 8'd0}; 
         else if (wr) dma_req[4:0] <= wb_sel_i[1] ? wb_dat_i[4:0] :  5'd0;
      end
      9'h01a: begin
         if (rd) wb_dat_local <= 16'd0;
      end                  
      //9'h01c: // DMA Write Counter
      //begin
      //  if (rd)
      //    wb_dat_local <= {dma_wr_cnt};                
      //end   
      9'h01c: begin
         if (rd) begin wb_dat_local <= dma_wr_cnt[15:0]; temp_data <= dma_wr_cnt[31:16]; end
      end
      9'h01e: begin
         if (rd) begin wb_dat_local <= temp_data; end
      end            
      //9'h020: // DMA Read Counter
      //begin
      //  if (rd)
      //    wb_dat_local <= {dma_rd_cnt};                
      //end   
      9'h020: begin
         if (rd) begin wb_dat_local <= dma_rd_cnt[15:0]; temp_data <= dma_rd_cnt[31:16]; end
      end
      9'h022: begin
         if (rd) begin wb_dat_local <= temp_data; end
      end                  
      //9'h024: // Init Credits
      //begin
      //  if (rd)
      //    wb_dat_local <= {3'd0, init_ca_pd, 7'd0, init_ca_nph};                
      //end         
      9'h024: begin
         if (rd) begin wb_dat_local <= {7'd0, init_ca_nph}; end
      end
      9'h026: begin
         if (rd) begin wb_dat_local <= {3'd0, init_ca_pd}; end
      end          
      //9'h028: // EBR Filter Value
      //begin
      //  if (rd)
      //    wb_dat_local <= {ebr_filter};        
      //  else if (wr)
      //  begin          
      //    ebr_filter[31:24] <= wb_sel_i[0] ? wb_dat_i[31:24] : ebr_filter[31:24];
      //    ebr_filter[23:16] <= wb_sel_i[1] ? wb_dat_i[23:16] : ebr_filter[23:16];
      //    ebr_filter[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8]   : ebr_filter[15:8];
      //    ebr_filter[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0] :     ebr_filter[7:0];      
      //  end        
      //end
      9'h028: begin
         if (rd) wb_dat_local <= ebr_filter[15:0];
         else if (wr) begin
            ebr_filter[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : ebr_filter[15:8];
            ebr_filter[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : ebr_filter[7:0];
         end
      end
      9'h02a: begin
         if (rd) wb_dat_local <= ebr_filter[31:16];
         else if (wr) begin
            ebr_filter[31:24] <= wb_sel_i[0] ? wb_dat_i[15:8] : ebr_filter[31:24];
            ebr_filter[23:16] <= wb_sel_i[1] ? wb_dat_i[7:0]  : ebr_filter[23:16];
         end
      end            
      //9'h030: // ColorBar Rst
      //begin
      //  if (rd)
      //    wb_dat_local <= {31'd0, cb_rst};        
      //  else if (wr)
      //  begin                    
      //    cb_rst <= wb_sel_i[3] ? wb_dat_i[0] : cb_rst;      
      //  end       
      //end               
      9'h030: begin
         if (rd) wb_dat_local <= {15'd0, cb_rst};
         else if (wr) begin
            cb_rst <= wb_sel_i[1] ? wb_dat_i[0] : cb_rst;
         end      
      end
      9'h032: begin
         if (rd) wb_dat_local <= 0;
      end             
      //9'h100: // Int Ctrl ID      
      //begin
      //  if (rd)
      //    wb_dat_local <= 32'h12043050;                          
      //end   
      9'h100: begin
         if (rd) wb_dat_local <= 16'h3050;
      end
      9'h102: begin
         if (rd) wb_dat_local <= 16'h1204;
      end            
      //9'h0104: // Int Ctrl
      //begin
      //  if (rd)
      //    wb_dat_local <= {8'd0, int_test1, int_test0, 5'd0, int_en, int_test_mode, int_i};        
      //  else if (wr)
      //  begin          
      //    int_test1 <= wb_sel_i[1] ? wb_dat_i[23:16] : int_test1;
      //    int_test0 <= wb_sel_i[2] ? wb_dat_i[15:8]   : int_test0;
      //    {int_en, int_test_mode} <= wb_sel_i[3] ? wb_dat_i[2:1] : {int_en, int_test_mode};
      //  end        
      //end
      9'h104: begin
         if (rd) wb_dat_local <= {int_test0, 5'd0, int_en, int_test_mode, int_i};
         else if (wr) begin
            int_test0 <= wb_sel_i[0] ? wb_dat_i[15:8] : int_test0;
            {int_en, int_test_mode, int_i} <= wb_sel_i[1] ? wb_dat_i[2:0] : {int_en, int_test_mode, int_i};
         end
      end
      9'h106: begin
         if (rd) wb_dat_local <= {8'd0, int_test1};
         else if (wr) begin
            int_test1 <= wb_sel_i[1] ? wb_dat_i[7:0] : int_test1;
         end
      end              
      //9'h0108: // Int Status
      //begin
      //  if (rd)
      //    wb_dat_local <= {int_status};                
      //end
      9'h108: begin
         if (rd) begin wb_dat_local <= int_status[15:0]; temp_data <= int_status[31:16]; end
      end
      9'h10a: begin
         if (rd) begin int_status <= temp_data; end
      end                        
      //9'h010c: // Int Mask
      //begin
      //  if (rd)
      //    wb_dat_local <= int_mask;        
      //  else if (wr)
      //  begin          
      //    int_mask[31:24] <= wb_sel_i[0] ? wb_dat_i[31:24] : int_mask[31:24];
      //    int_mask[23:16] <= wb_sel_i[1] ? wb_dat_i[23:16] : int_mask[23:16];
      //    int_mask[15:8] <= wb_sel_i[2] ? wb_dat_i[15:8]   : int_mask[15:8];
      //    int_mask[7:0] <= wb_sel_i[3] ? wb_dat_i[7:0] :     int_mask[7:0];      
      //  end        
      //end
      9'h10c: begin
         if (rd) wb_dat_local <= int_mask[15:0];
         else if (wr) begin
            int_mask[15:8] <= wb_sel_i[0] ? wb_dat_i[15:8] : int_mask[15:8];
            int_mask[7:0]  <= wb_sel_i[1] ? wb_dat_i[7:0]  : int_mask[7:0];
         end
      end
      9'h10e: begin
         if (rd) wb_dat_local <= int_mask[31:16];
         else if (wr) begin
            int_mask[31:24] <= wb_sel_i[0] ? wb_dat_i[15:8] : int_mask[31:24];
            int_mask[23:16] <= wb_sel_i[1] ? wb_dat_i[7:0]  : int_mask[23:16];
         end
      end        
        
      default:
        wb_dat_local <= 0;    
    endcase
    
  end // cyc
  else
  begin
    dma_req[4:0] <= 5'd0;  
  end
   
  // DMA Write Counter   
  if (dma_req[0])
    dma_wr <= 1'b1;
  else if (dma_ack[0])
    dma_wr <= 1'b0;
  
  if (dma_req[0])
    dma_wr_cnt <= 32'd0;
  else if (dma_wr && dma_wr_cnt!=32'hffffffff)
    dma_wr_cnt <= dma_wr_cnt + 1;
    
    
  // DMA Read Counter  
  if (dma_req[1])
     dma_rd[1] <= 1'b1;
  else if (dma_ack[1])
     dma_rd[1] <= 1'b0;
  
  if (dma_req[2])
     dma_rd[2] <= 1'b1;
  else if (dma_ack[2])
     dma_rd[2] <= 1'b0;
     
  if (dma_req[3])
     dma_rd[3] <= 1'b1;
  else if (dma_ack[3])
     dma_rd[3] <= 1'b0;
     
  if (dma_req[4])
     dma_rd[4] <= 1'b1;
  else if (dma_ack[4])
     dma_rd[4] <= 1'b0;
  
  
  if (dma_req[1] || dma_req[2] || dma_req[3] || dma_req[4])
    dma_rd_cnt <= 32'd0;
  else if (|dma_rd && dma_rd_cnt!=32'hffffffff)
    dma_rd_cnt <= dma_rd_cnt + 1;
    
  
  // counter
  if (~cnt_reload || cnt == 32'd0)
    cnt <= cnt_reload_val;
  else if (cnt_run)     
    cnt <= cnt - 1;
    
  cnt0 <= cnt==32'd0;
    
    
  // Interrupts
  int_status <= int_test_mode ? {16'd0, int_test1, int_test0} : {26'd0, cnt0, dma_ack[4:0]} ;
  int_i <= |(int_status & int_mask);
  int_out <= int_i && int_en;
  
  dl_up_p <= dl_up;
  if (~dl_up_p && dl_up)
  begin
    init_ca_pd <= ca_pd;
    init_ca_nph <= ca_nph;    
  end
  
  
  
  end //clk
end

assign wb_dat_o = wb_dat_local;

assign wb_err_o = 1'b0;
assign wb_rty_o = 1'b0;

always @(posedge wb_rst_i or posedge wb_clk_i)
   if (wb_rst_i) begin
      wb_ack_o   <= 0;
   end
   else begin
      wb_ack_o   <= wb_cyc_i & wb_stb_i & (~ wb_ack_o);
   end
endmodule

