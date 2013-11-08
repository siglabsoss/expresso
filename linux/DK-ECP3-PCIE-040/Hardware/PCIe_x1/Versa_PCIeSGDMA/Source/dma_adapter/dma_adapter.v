// $Id: dma_adapter.v,v 1.1.1.1 2008/07/01 17:34:23 jfreed Exp $

module dma_adapter(rstn, enable,
            wb_clk_i, wb_rst_i,
            wb_dat_i, wb_adr_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,            
            wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
            dma_req, dma_ack,
            burst_len, active_ch, requestor_id,
            
            clk_125, tx_st, tx_end, tx_data,
            tx_req, tx_rdy,
            tx_ca_ph, tx_ca_pd, tx_ca_nph, 
            
            rx_st, rx_end, rx_data,
            rx_cr_cplh, rx_cr_cpld,
            unexp_cmpl,
            
            debug
);

input rstn;
input enable;

input         wb_clk_i;
input         wb_rst_i;
input  [63:0] wb_dat_i;
input  [31:0]  wb_adr_i;
input         wb_cyc_i;
input         wb_lock_i;
input  [7:0]  wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [63:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o;
output [1:0]  dma_req;
input  [1:0]  dma_ack;

input [15:0] burst_len; 
input [1:0] active_ch;
input [15:0] requestor_id;

input clk_125;
output tx_st, tx_end;
//output [63:0] tx_data;
output [15:0] tx_data;
output tx_req;
input tx_rdy;
input [8:0] tx_ca_ph;
input [12:0] tx_ca_pd;
input [8:0] tx_ca_nph;
output rx_cr_cplh;
output [7:0] rx_cr_cpld;
output unexp_cmpl;

output [31:0] debug;


input rx_st, rx_end;
input [15:0] rx_data;


wire [63:0] tx_data_wb; 
wire [4:0] tx_pd_wb, tx_pd;
wire [11:0] ch1_size;
wire [63:0] ch_data;
wire ch_dv;



wire [63:0] wb_dat_i_flip, wb_dat_o_flip;


// order bytes back to PCIe order
assign wb_dat_i_flip = {wb_dat_i[7:0], wb_dat_i[15:8], wb_dat_i[23:16], wb_dat_i[31:24], 
                    wb_dat_i[39:32], wb_dat_i[47:40], wb_dat_i[55:48], wb_dat_i[63:56]}; 
                    
assign wb_dat_o = {wb_dat_o_flip[7:0], wb_dat_o_flip[15:8], wb_dat_o_flip[23:16], wb_dat_o_flip[31:24], 
                    wb_dat_o_flip[39:32], wb_dat_o_flip[47:40], wb_dat_o_flip[55:48], wb_dat_o_flip[63:56]}; 
                    


dma_wbs wbs(.wb_clk_i(wb_clk_i), .wb_rst_i(~rstn), .wb_dat_i(wb_dat_i_flip), .wb_adr_i(wb_adr_i), 
            .wb_cyc_i(wb_cyc_i), .wb_lock_i(wb_lock_i), .wb_sel_i(wb_sel_i), .wb_stb_i(wb_stb_i),
            .wb_we_i(wb_we_i),  .wb_dat_o(wb_dat_o_flip),
            .wb_ack_o(wb_ack_o), .wb_err_o(wb_err_o), .wb_rty_o(wb_rty_o),
            .burst_len(burst_len), .active_ch(active_ch), .requestor_id(requestor_id),
            .dma_req(dma_req), .dma_ack(dma_ack),
            .enable(enable), 
            .c_pd(tx_pd_wb), .c_nph(tx_nph_wb), .c_ph(tx_ph_wb), .tx_dwen(tx_dwen_wb), .tx_end(tx_end_wb), .tx_st(tx_st_wb),             
            .tx_data(tx_data_wb), .tx_dv(tx_dv_wb), .tx_cv(tx_cv_wb), .tx_full(tx_full),
            .ch1_rdy(ch1_rdy), .ch1_size(ch1_size), .ch1_rden(ch1_rden), .ch1_pending(ch1_pending),
            .ch_data(ch_data), .ch_dv(ch_dv),
            .debug()
);


dma_tx_fifo_new I_dma_tx_fifo_new(
.wb_clk           (wb_clk_i), 
.clk_125          (clk_125), 
.rstn             (rstn),
.tx_st_in         (tx_st_wb), 
.tx_end_in        (tx_end_wb), 
.tx_dwen_in       (tx_dwen_wb), 
.tx_data_in       (tx_data_wb), 
.tx_dv            (tx_dv_wb),
.tx_st            (tx_st), 
.tx_end           (tx_end), 
.tx_data          (tx_data), 
.tx_req           (tx_req), 
.tx_rdy           (tx_rdy),                    
.full             (tx_full), 
.tx_ca_ph         (tx_ca_ph), 
.tx_ca_pd         (tx_ca_pd), 
.tx_ca_nph        (tx_ca_nph), 
.tx_ca_p_recheck  (1'b0)                  
);

dma_rx_fifo rx (.wb_clk(wb_clk_i), .clk_125(clk_125), .rstn(rstn), 
           .rx_st(rx_st), .rx_end(rx_end), .rx_data(rx_data),           
           .active_ch(active_ch),
           .ch1_rdy(ch1_rdy), .ch1_size(ch1_size), .ch1_rden(ch1_rden), .ch1_pending(ch1_pending),
           .ch_data(ch_data), .ch_dv(ch_dv),
           .cplh_cr(rx_cr_cplh), .cpld_cr(rx_cr_cpld),
           .unexp_cmpl(unexp_cmpl)  ,
           .debug()
);                   



 

endmodule


