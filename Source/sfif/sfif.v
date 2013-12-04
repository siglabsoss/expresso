// $Id: sfif.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif(rstn, 
            wb_clk_i, wb_rst_i,
            wb_dat_i, wb_adr_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,            
            wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
            
            clk_125, tx_st, tx_end, tx_nlfy, tx_data,
            tx_req, tx_rdy,
            tx_ca_ph, tx_ca_pd, tx_ca_nph, tx_ca_npd,
            
            rx_st, rx_end, rx_data,
            rx_cr_cplh, rx_cr_cpld,
            
            enabled, sm, debug
			
			, clk_ADC0_DCO_P
			, ADC0_BUS
);

input clk_ADC0_DCO_P;
input [7:0] ADC0_BUS;

input rstn;

input         wb_clk_i;
input         wb_rst_i;
input  [15:0] wb_dat_i;
input  [17:0]  wb_adr_i;
input         wb_cyc_i;
input         wb_lock_i;
input  [1:0]  wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [15:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o; 

input clk_125;
output tx_st, tx_end, tx_nlfy;
output [15:0] tx_data;
output tx_req;
input tx_rdy;
input [8:0] tx_ca_ph;
input [12:0] tx_ca_pd;
input [8:0] tx_ca_nph;
input [12:0] tx_ca_npd;
output rx_cr_cplh;
output [7:0] rx_cr_cpld;
output [2:0] sm;
output [15:0] debug;


input rx_st, rx_end;
input [15:0] rx_data;

output enabled;

wire enable;
wire reset;
wire lpbk;
wire [15:0] tx_cycles;
wire [3:0] c_npd, c_pd;
wire [15:0] ipg_cnt;        
wire [31:0] tx32_data, rx32_data;
wire [63:0] tx64_data, rx64_data;
wire [3:0] tx64_pd; 
wire [4:0] tag, tx64_tag;
wire [3:0] tag_cplds, tx64_tag_cplds;
wire rx64_filter;
reg [31:0] elapsed_cnt, elapsed_cnt_free, tx_tlp_cnt, rx_tlp_cnt, credit_wait_p_cnt, credit_wait_np_cnt, rx_tlp_timestamp;

wire sfif_rstn;

sfif_wbs wbs(.wb_clk_i(wb_clk_i), .wb_rst_i(wb_rst_i), .wb_dat_i(wb_dat_i), .wb_adr_i(wb_adr_i), 
             .wb_cyc_i(wb_cyc_i), .wb_lock_i(wb_lock_i), .wb_sel_i(wb_sel_i), .wb_stb_i(wb_stb_i),
             .wb_we_i(wb_we_i),  .wb_dat_o(wb_dat_o),
             .wb_ack_o(wb_ack_o), .wb_err_o(wb_err_o), .wb_rty_o(wb_rty_o),
             .tx_cycles(tx_cycles), .lpbk(lpbk), .loop(loop), .run(run), .reset(reset), .enable(enable), .rx_filter(rx_filter),
             .c_npd(c_npd), .c_pd(c_pd), .c_nph(c_nph), .c_ph(c_ph), .tag(tag), .tag_cplds(tag_cplds), .mrd(mrd),
             .tx_dwen(tx32_dwen), .tx_nlfy(tx32_nlfy), .tx_end(tx32_end), .tx_st(tx32_st), 
             .tx_ctrl(tx32_ctrl),
             .ipg_cnt(ipg_cnt), .tx_dv(tx32_dv),
             .elapsed_cnt(elapsed_cnt), .tx_tlp_cnt(tx_tlp_cnt), .rx_tlp_cnt(rx_tlp_cnt), .rx_empty(rx_empty),
             .credit_wait_p_cnt(credit_wait_p_cnt), .credit_wait_np_cnt(credit_wait_np_cnt), .rx_tlp_timestamp(rx_tlp_timestamp)
			 
			 //,.clk_ADC0_DCO_P()
		     //,.ADC0_BUS()
			 
			 ,.clk_ADC0_DCO_P(clk_ADC0_DCO_P)
		     ,.ADC0_BUS(ADC0_BUS)
);

assign enabled = enable;

assign sfif_rstn = rstn && ~reset;  // system rstn and sw ctrl reset

/*
sfif_tx_fifo tx (.wb_clk(wb_clk_i), .clk_125(clk_125), .rstn(sfif_rstn), .rprst(rprst), 
           .tx32_st(tx32_st), .tx32_end(tx32_end), .tx32_dwen(tx32_dwen), .tx32_nlfy(tx32_nlfy), .tx32_data(tx32_data), .tx32_dv(tx32_dv),
           .tx32_ctrl(tx32_ctrl), .tx32_nph(c_nph), .tx32_pd(c_pd), .tx32_ph(c_ph),
           .tx32_tag(tag), .tx32_tag_cplds(tag_cplds), .tx32_mrd(mrd),
           .tx64_st(tx64_st), .tx64_end(tx64_end), .tx64_dwen(tx64_dwen), .tx64_nlfy(tx64_nlfy), .tx64_data(tx64_data),           
           .tx64_nph(tx64_nph), .tx64_pd(tx64_pd), .tx64_ph(tx64_ph),
           .tx64_tag(tx64_tag), .tx64_tag_cplds(tx64_tag_cplds),
           .empty(tx_empty), .credit_read(tx_cr_read), .data_read(tx_d_read), .cr_avail(credit_available), .tag_avail(tag_avail),
           .tx64_req(tx64_req), .tx_rdy(tx64_rdy), .tx_val(tx_val)           
);          
*/
sfif_ca ca (.clk_125(clk_125), .rstn(sfif_rstn),
            .cp_ph(tx64_ph), .cp_pd(tx64_pd), .cp_nph(tx64_nph), 
            .ca_ph(tx_ca_ph), .ca_pd(tx_ca_pd), .ca_nph(tx_ca_nph), .ca_npd(tx_ca_npd),
            .credit_available(cr_avail_calc)
);

sfif_tag ta(.clk_125(clk_125), .rstn(sfif_rstn),
            .tx_st(tx64_st), .tag(tx64_tag), .tag_cplds(tx64_tag_cplds), 
            .rx_st(rx64_st), .rx_data(rx64_data), 
            .tag_available(tag_avail)
            
);

           
sfif_ctrl ctrl (.clk_125(clk_125), .rstn(sfif_rstn), 
                .rprst(rprst), .enable(enable), .run(run), .ipg_cnt(ipg_cnt), .tx_cycles(tx_cycles), .loop(loop),
                .tx_empty(tx_empty), .tx_rdy(tx64_rdy), .tx_val(tx_val), .tx_end(tx64_end), .credit_available(credit_available), 
                .tx_cr_read(tx_cr_read), .tx_d_read(tx_d_read),
                .done(done), .sm(sm));
/*                
sfif_rx_fifo rx (.wb_clk(wb_clk_i), .clk_125(clk_125), .rstn(sfif_rstn), 
           .timestamp(elapsed_cnt_free),
           .rx32_data(rx32_data), .rden(rx32_data_read),           
           .rx64_st(rx64_st), .rx64_end(rx64_end), .rx64_dwen(rx64_dwen), .rx64_data(rx64_data), .rx64_filter(rx64_filter), 
           .empty(rx_empty) 
           );                       
*/           
sfif_cr cr (.clk_125(clk_125), .rstn(sfif_rstn),
            .rx_st(rx64_st), .rx_data(rx64_data),
            .cplh_cr(rx_cr_cplh), .cpld_cr(rx_cr_cpld)            
);
                      
           
//assign rx64_st = rx_st;
//assign rx64_end = rx_end;
//assign rx64_dwen = rx_dwen;
//assign rx64_data = rx_data;
bridge_16b_to_64b I_bridge_16b_to_64b(
.rstn             (rstn         ),
.clk_125          (clk_125      ),
.rx_data_16b      (rx_data      ),
.rx_st_16b        (rx_st        ),
.rx_end_16b       (rx_end       ),
.rx_us_req_16b    (1'b0    ),
.rx_malf_tlp_16b  (1'b0  ),
.rx_bar_hit_16b   (7'd0   ),

.rx_data_64b      (rx64_data),
.rx_st_64b        (rx64_st),
.rx_end_64b       (rx64_end),
.rx_dwen_64b      (rx64_dwen),
.rx_us_req_64b    (),
.rx_malf_tlp_64b  (),
.rx_bar_hit_64b   ()
);

assign credit_available = cr_avail_calc;
//assign tx64_rdy  = tx_rdy;
assign tx_req    = 1'b0;
//assign tx_st     = tx64_st;
//assign tx_end    = tx64_end;
//assign tx_dwen   = tx64_dwen; 
//assign tx_nlfy   = tx64_nlfy;
//assign tx_data   = tx64_data;
bridge_64b_to_16b I_bridge_64b_to_16b(
.rstn             (rstn),
.clk_125          (clk_125),
.tx_rdy_64b       (tx64_rdy),
.tx_data_64b      (tx64_data),
.tx_st_64b        (tx64_st),
.tx_end_64b       (tx64_end),
.tx_dwen_64b      (tx64_dwen),

.tx_val           (tx_val),
.tx_rdy_16b       (tx_rdy),
.tx_data_16b      (tx_data),
.tx_st_16b        (tx_st),
.tx_end_16b       (tx_end)
);

assign cpld = (rx64_data[63:56] == 8'b01001010); // CplD type
assign rx64_filter = (rx64_st && ~cpld && rx_filter);           

// Counters
always @(posedge clk_125 or negedge sfif_rstn)
begin
  if (~sfif_rstn)
  begin
    tx_tlp_cnt <= 32'd0;
    rx_tlp_cnt <= 32'd0;
    elapsed_cnt <= 32'd0;    
    elapsed_cnt_free <= 32'd0;
    credit_wait_p_cnt <= 32'd0;
    credit_wait_np_cnt <= 32'd0;
    rx_tlp_timestamp <= 32'd0;
  end
  else
  begin
    if (tx64_end && tx_val && (tx_tlp_cnt!=32'hffffffff))
      tx_tlp_cnt <= tx_tlp_cnt + 1;
      
    if (rx64_st && ~rx64_filter && (rx_tlp_cnt!=32'hffffffff))
      rx_tlp_cnt <= rx_tlp_cnt + 1;
      
    if (run && ~done && (elapsed_cnt!=32'hffffffff))
      elapsed_cnt <= elapsed_cnt + 1;  
    
    if (run && (elapsed_cnt_free!=32'hffffffff))
      elapsed_cnt_free <= elapsed_cnt_free + 1;
      
    if (sm[2:1]==2'b01 && ~credit_available && tx64_ph && (credit_wait_p_cnt!=32'hffffffff))
      credit_wait_p_cnt <= credit_wait_p_cnt + 1;  
      
    if (sm[2:1]==2'b01 && (~credit_available || ~tag_avail) && tx64_nph && (credit_wait_np_cnt!=32'hffffffff))
      credit_wait_np_cnt <= credit_wait_np_cnt + 1;  
    
    
    if (rx64_st && cpld)
      rx_tlp_timestamp <= elapsed_cnt_free;
      
  end
end           

assign req_gated = tx_req & ~done;

//                 15                14      13      12   11    10        9           8          7     6
assign debug = 16'b0; //{credit_available, tag_avail, enable, run, loop, tx_empty, tx_cr_read, tx_d_read, done, req_gated, 6'd0};

 

endmodule

