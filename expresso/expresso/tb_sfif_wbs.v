module tb_sfif_wbs(
input wb_clk_i
);




reg [15:0] wb_dat_i;
reg [17:0] wb_adr_i;
reg wb_cyc_i;
reg wb_lock_i;
reg [1:0] wb_sel_i;
reg wb_stb_i;
reg wb_we_i;
wire enable;

// outputs
wire wb_dat_o;
wire wb_ack_o;
wire wb_err_o;
wire wb_rty_o;

wire tx_cycles;
wire loop;
wire run;
wire reset;
wire rx_filter;
wire c_npd;
wire c_pd;
wire c_nph;
wire c_ph;
wire tag;
wire tag_cplds;
wire mrd;
wire tx32_dwen;
wire tx32_nlfy;
wire tx32_end;
wire tx32_st;
wire tx32_ctrl;
wire ipg_cnt;
wire tx32_data;
wire tx32_dv;
wire rx32_data;
wire rx32_data_read;
wire elapsed_cnt;
wire tx_tlp_cnt;
wire rx_tlp_cnt;
wire rx_empty;
wire credit_wait_p_cnt;
wire credit_wait_np_cnt;
wire rx_tlp_timestamp;

reg [32:0] t = 0; 
reg wb_rst_i = 1'b1;
reg [4:0] counter = 4'b0;

`define READCYC(mt,maddr) if( t == mt) begin wb_cyc_i <= 1; wb_adr_i <= maddr; wb_stb_i <= 1'b1; end if( t == mt+1 ) begin wb_cyc_i <= 0; end


always@(posedge wb_clk_i)begin
//	clk_125 = ~clk_125;
    wb_rst_i <= 1'b0;
    counter <= counter + 1;
    t <= t + 1;
    
    // reset condition
    if( t == 0 ) begin
        wb_rst_i <= 0;
        wb_dat_i <= 0;
        wb_lock_i <= 0;
        wb_sel_i <= 2'b00;
        wb_stb_i <= 0;
        wb_we_i <= 0;
        wb_cyc_i <= 0;
        wb_adr_i <= 18'h00000;
    end
    
    `READCYC(100, 18'h02000);
    `READCYC(102, 18'h02001);
    `READCYC(104, 18'h02002);
    `READCYC(106, 18'h02003);
    `READCYC(108, 18'h02004);
    `READCYC(110, 18'h02005);
    `READCYC(112, 18'h02006);
    `READCYC(114, 18'h02007);
    `READCYC(116, 18'h02008);
    `READCYC(118, 18'h02009);
    `READCYC(120, 18'h0200a);
    `READCYC(122, 18'h0200b);
    `READCYC(124, 18'h0200c);
    `READCYC(126, 18'h0200d);
    `READCYC(128, 18'h0200e);
    `READCYC(130, 18'h0200f);
    `READCYC(132, 18'h02010);
    `READCYC(134, 18'h02012);
    `READCYC(136, 18'h02013);
    `READCYC(138, 18'h02014);
    
    
    
//    if( t == 102 ) begin
//        wb_cyc_i <= 1;
//        wb_adr_i <= 18'h02001;
//        wb_stb_i <= 1'b1; //somehow part of read (sfif_wbs:85)
//    end
//    if( t == 103 ) begin
//        wb_cyc_i <= 0;
//    end
    
   
    
    
       
end


sfif_wbs wbs(.wb_clk_i(wb_clk_i), .wb_rst_i(wb_rst_i), .wb_dat_i(wb_dat_i), .wb_adr_i(wb_adr_i), 
             .wb_cyc_i(wb_cyc_i), .wb_lock_i(wb_lock_i), .wb_sel_i(wb_sel_i), .wb_stb_i(wb_stb_i),
             .wb_we_i(wb_we_i),  .wb_dat_o(wb_dat_o),
             .wb_ack_o(wb_ack_o), .wb_err_o(wb_err_o), .wb_rty_o(wb_rty_o),
             .tx_cycles(tx_cycles), .lpbk(lpbk), .loop(loop), .run(run), .reset(reset), .enable(enable), .rx_filter(rx_filter),
             .c_npd(c_npd), .c_pd(c_pd), .c_nph(c_nph), .c_ph(c_ph), .tag(tag), .tag_cplds(tag_cplds), .mrd(mrd),
             .tx_dwen(tx32_dwen), .tx_nlfy(tx32_nlfy), .tx_end(tx32_end), .tx_st(tx32_st), 
             .tx_ctrl(tx32_ctrl),
             .ipg_cnt(ipg_cnt), .tx_data(tx32_data), .tx_dv(tx32_dv), .rx_data(rx32_data), .rx_data_read(rx32_data_read),
             .elapsed_cnt(elapsed_cnt), .tx_tlp_cnt(tx_tlp_cnt), .rx_tlp_cnt(rx_tlp_cnt), .rx_empty(rx_empty),
             .credit_wait_p_cnt(credit_wait_p_cnt), .credit_wait_np_cnt(credit_wait_np_cnt), .rx_tlp_timestamp(rx_tlp_timestamp)
);



endmodule