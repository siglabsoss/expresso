// $Id: top_basic.v,v 1.2 2008/07/02 19:10:29 jfreed Exp $


  `timescale 1ns / 1 ps
module top  (   
   rstn,
   FLIP_LANES, LED_INV,
   
    
   refclkp, refclkn,
   hdinp, hdinn,                   
   hdoutp, hdoutn, 

   pll_lk, poll, l0, dl_up, usr0, usr1, usr2, usr3,
 //  na_pll_lk, na_poll, na_l0, na_dl_up, na_usr0, na_usr1, na_usr2, na_usr3,   // cngd frm org
   dip_switch, led_out, dp,
   TP);
  // la  
     
      
   //);

input rstn;
input refclkp, refclkn;
input hdinp, hdinn;
output hdoutp, hdoutn;

// These two inputs are set in the .lpf file
input FLIP_LANES; // Flip PCIe lanes
input LED_INV;  // LED polarity inverted


output pll_lk, poll, l0, dl_up, usr0, usr1, usr2, usr3;
         // na_pll_lk, na_poll, na_l0, na_dl_up, na_usr0, na_usr1, na_usr2, na_usr3;   // cngd frm org
input [7:0] dip_switch;
output [13:0] led_out; // cngd frm org
output dp;
output [15:0] TP;
//output [33:0] la;// cngd frm org

reg  [19:0] rstn_cnt;
reg  core_rst_n;
wire [13:0] led_out_int;

wire [3:0]               phy_ltssm_state; 
wire dl_up_int;

wire [15:0] rx_data, tx_data,  tx_dout_wbm, tx_dout_ur;
wire [6:0] rx_bar_hit;

wire [7:0] pd_num, pd_num_ur, pd_num_wb;

wire [15:0] pcie_dat_i, pcie_dat_o;
wire [31:0] pcie_adr;
wire [1:0] pcie_sel;  
wire [2:0] pcie_cti;
wire pcie_cyc;
wire pcie_we;
wire pcie_stb;
wire pcie_ack;



wire [15:0] gpio_dat_i, gpio_dat_o;
wire [31:0] gpio_adr;
wire [1:0] gpio_sel;
wire [2:0] gpio_cti;
wire gpio_cyc;
wire gpio_we;
wire gpio_stb;
wire gpio_ack;

wire [15:0] ebr_dat_i, ebr_dat_o;
wire [31:0] ebr_adr;
wire [1:0] ebr_sel;
wire [2:0] ebr_cti;
wire ebr_cyc;
wire ebr_we;
wire ebr_stb;
wire ebr_ack;

wire [7:0] bus_num ; 
wire [4:0] dev_num ; 
wire [2:0] func_num ;


wire [8:0] tx_ca_ph ;
wire [12:0] tx_ca_pd  ;
wire [8:0] tx_ca_nph ;
wire [12:0] tx_ca_npd ;
wire [8:0] tx_ca_cplh;
wire [12:0] tx_ca_cpld ;
wire clk_125;
wire tx_eop_wbm;
//=============================================================================
// Reset management
//=============================================================================
always @(posedge clk_125 or negedge rstn) begin
   if (!rstn) begin
       rstn_cnt   <= 20'd0 ;
       core_rst_n <= 1'b0 ;
   end
   else begin
      if (rstn_cnt[19])            // 4ms in real hardware
         core_rst_n <= 1'b1 ;
      // synthesis translate_off
         else if (rstn_cnt[7])     // 128 clocks 
            core_rst_n <= 1'b1 ;
      // synthesis translate_on
      else 
         rstn_cnt <= rstn_cnt + 1'b1 ;
   end
end
// LED Assignments
led_status led(.clk(clk_125), .rstn(core_rst_n), .invert(LED_INV),
        .lock(1'b1), .ltssm_state(phy_ltssm_state), .dl_up_in(dl_up_int), .bar1_hit(rx_bar_hit[1] | rx_bar_hit[0]),
        .pll_lk(pll_lk), .poll(poll), .l0(l0), .dl_up_out(dl_up), 
        .usr0(usr0), .usr1(usr1), .usr2(usr2), .usr3(usr3),
        .na_pll_lk(na_pll_lk), .na_poll(na_poll), .na_l0(na_l0), .na_dl_up_out(na_dl_up), 
        .na_usr0(na_usr0), .na_usr1(na_usr1), .na_usr2(na_usr2), .na_usr3(na_usr3), .dpn(dp)
);


pcie_top pcie(   
   .refclkp                    ( refclkp ),    
   .refclkn                    ( refclkn ),   
   .sys_clk_125                ( clk_125 ),
   .ext_reset_n                ( rstn ),
   .rstn                       ( core_rst_n ),    
   .flip_lanes                 ( FLIP_LANES ),
                               
   .hdinp0                     ( hdinp ), 
   .hdinn0                     ( hdinn ), 
   .hdoutp0                    ( hdoutp ), 
   .hdoutn0                    ( hdoutn ), 
                               
   .msi                        (  8'd0 ),
   .inta_n                     (  1'b1 ),
   
   // This PCIe interface uses dynamic IDs. 
   .vendor_id                  (16'h1204),       
   .device_id                  (16'hec30),       
   .rev_id                     (8'h00),          
   .class_code                 (24'h000000),      
   .subsys_ven_id              (16'h1204),   
   .subsys_id                  (16'h3010),       
   .load_id                    (1'b1),         
   
   
   // Inputs
   .force_lsm_active           ( 1'b0 ), 
   .force_rec_ei               ( 1'b0 ),     
   .force_phy_status           ( 1'b0 ), 
   .force_disable_scr          ( 1'b0 ),
                               
   .hl_snd_beacon              ( 1'b0 ),
   .hl_disable_scr             ( 1'b0 ),
   .hl_gto_dis                 ( 1'b0 ),
   .hl_gto_det                 ( 1'b0 ),
   .hl_gto_hrst                ( 1'b0 ),
   .hl_gto_l0stx               ( 1'b0 ),
   .hl_gto_l1                  ( 1'b0 ),
   .hl_gto_l2                  ( 1'b0 ),
   .hl_gto_l0stxfts            ( 1'b0 ),
   .hl_gto_lbk                 ( 1'd0 ),
   .hl_gto_rcvry               ( 1'b0 ),
   .hl_gto_cfg                 ( 1'b0 ),
   .no_pcie_train              ( 1'b0 ),    

   // Power Management Interface
   .tx_dllp_val                ( 2'd0 ),
   .tx_pmtype                  ( 3'd0 ),
   .tx_vsd_data                ( 24'd0 ),

   .tx_req_vc0                 ( tx_req ),    
   .tx_data_vc0                ( tx_data ),    
   .tx_st_vc0                  ( tx_st ), 
   .tx_end_vc0                 ( tx_end ), 
   .tx_nlfy_vc0                ( 1'b0 ), 
   .ph_buf_status_vc0       ( 1'b0 ),
   .pd_buf_status_vc0       ( 1'b0 ),
   .nph_buf_status_vc0      ( 1'b0 ),
   .npd_buf_status_vc0      ( 1'b0 ),
   .ph_processed_vc0        ( ph_cr ),
   .pd_processed_vc0        ( pd_cr ),
   .nph_processed_vc0       ( nph_cr ),
   .npd_processed_vc0       ( npd_cr ),
   .pd_num_vc0              ( pd_num ),
   .npd_num_vc0             ( 8'd1 ),   
   

   // From User logic
   .cmpln_tout                 ( 1'b0 ),       
   .cmpltr_abort_np            ( 1'b0 ),
   .cmpltr_abort_p             ( 1'b0 ),
   .unexp_cmpln                ( 1'b0 ),
   .ur_np_ext                  ( 1'b0 ),       
   .ur_p_ext                   ( 1'b0 ),
   .np_req_pend                ( 1'b0 ),     
   .pme_status                 ( 1'b0 ),      
         
   .tx_dllp_sent               (  ),
   .rxdp_pmd_type              (  ),
   .rxdp_vsd_data              (  ),
   .rxdp_dllp_val              (  ),
  
 
   .phy_ltssm_state            ( phy_ltssm_state ),     
   .phy_ltssm_substate         ( ),     
   .phy_pol_compliance         ( ),   

   .tx_rdy_vc0                 ( tx_rdy),  
   .tx_ca_ph_vc0               ( tx_ca_ph),
   .tx_ca_pd_vc0               ( tx_ca_pd),
   .tx_ca_nph_vc0              ( tx_ca_nph),
   .tx_ca_npd_vc0              ( tx_ca_npd ), 
   .tx_ca_cplh_vc0             ( tx_ca_cplh ),
   .tx_ca_cpld_vc0             ( tx_ca_cpld ),
   .tx_ca_p_recheck_vc0        ( tx_ca_p_recheck ),
   .tx_ca_cpl_recheck_vc0      ( tx_ca_cpl_recheck ),
   .rx_data_vc0                ( rx_data),    
   .rx_st_vc0                  ( rx_st),     
   .rx_end_vc0                 ( rx_end),   
   .rx_us_req_vc0              ( rx_us_req ), 
   .rx_malf_tlp_vc0            ( rx_malf_tlp ), 
   .rx_bar_hit                 ( rx_bar_hit ), 
   .mm_enable                  (  ),
   .msi_enable                 (  ),

   // From Config Registers
   .bus_num                    ( bus_num  ),           
   .dev_num                    ( dev_num  ),           
   .func_num                   ( func_num  ),  
   .pm_power_state             (  ) , 
   .pme_en                     (  ) , 
   .cmd_reg_out                (  ),
   .dev_cntl_out               (  ),  
   .lnk_cntl_out               (  ),  

   .tx_rbuf_empty              (  ),   // Transmit retry buffer is empty
   .tx_dllp_pend               (  ),    // DLPP is pending to be transmitted
   .rx_tlp_rcvd                (  ),     // Received a TLP   

   // Datal Link Control SM Status
   .dl_inactive                (  ),
   .dl_init                    (  ),
   .dl_active                  (  ),
   .dl_up                      ( dl_up_int )   
);
reg rx_st_d;
reg tx_st_d;
reg [15:0] tx_tlp_cnt;
reg [15:0] rx_tlp_cnt;
always @(posedge clk_125 or negedge core_rst_n)
   if (!core_rst_n) begin
      tx_st_d <= 0;
      rx_st_d <= 0;
      tx_tlp_cnt <= 0;
      rx_tlp_cnt <= 0;
   end
   else begin
      tx_st_d <= tx_st;
      rx_st_d <= rx_st;
      if (tx_st_d) tx_tlp_cnt <= tx_tlp_cnt + 1;
      if (rx_st_d) rx_tlp_cnt <= rx_tlp_cnt + 1;
         
   end

ip_rx_crpr cr (.clk(clk_125), .rstn(core_rst_n), .rx_bar_hit(rx_bar_hit),
               .rx_st(rx_st), .rx_end(rx_end), .rx_din(rx_data),
               .pd_cr(pd_cr_ur), .pd_num(pd_num_ur), .ph_cr(ph_cr_ur), .npd_cr(npd_cr_ur), .nph_cr(nph_cr_ur)               
);

ip_crpr_arb crarb(.clk(clk_125), .rstn(core_rst_n), 
            .pd_cr_0(pd_cr_ur), .pd_num_0(pd_num_ur), .ph_cr_0(ph_cr_ur), .npd_cr_0(npd_cr_ur), .nph_cr_0(nph_cr_ur),
            .pd_cr_1(pd_cr_wb), .pd_num_1(pd_num_wb), .ph_cr_1(ph_cr_wb), .npd_cr_1(1'b0), .nph_cr_1(nph_cr_wb),               
            .pd_cr(pd_cr), .pd_num(pd_num), .ph_cr(ph_cr), .npd_cr(npd_cr), .nph_cr(nph_cr)               
);

UR_gen ur (.clk(clk_125), .rstn(core_rst_n),  
            .rx_din(rx_data), .rx_sop(rx_st), .rx_eop(rx_end), .rx_us(rx_us_req), .rx_bar_hit(rx_bar_hit),
             .tx_rdy(tx_rdy_ur), .tx_ca_cpl_recheck(1'b0), .tx_ca_cplh(tx_ca_cplh),
             .tx_req(tx_req_ur), .tx_dout(tx_dout_ur), .tx_sop(tx_sop_ur), .tx_eop(tx_eop_ur),
             .comp_id({bus_num, dev_num, func_num})
);
         
ip_tx_arbiter #(.c_DATA_WIDTH (16))
           tx_arb(.clk(clk_125), .rstn(core_rst_n), .tx_val(1'b1),
                  .tx_req_0(tx_req_wbm), .tx_din_0(tx_dout_wbm), .tx_sop_0(tx_sop_wbm), .tx_eop_0(tx_eop_wbm), .tx_dwen_0(1'b0),                 
                  .tx_req_1(1'b0), .tx_din_1(16'd0), .tx_sop_1(1'b0), .tx_eop_1(1'b0), .tx_dwen_1(1'b0),
                  .tx_req_2(1'b0), .tx_din_2(16'd0), .tx_sop_2(1'b0), .tx_eop_2(1'b0), .tx_dwen_2(1'b0),
                  .tx_req_3(tx_req_ur), .tx_din_3(tx_dout_ur), .tx_sop_3(tx_sop_ur), .tx_eop_3(tx_eop_ur), .tx_dwen_3(1'b0),
                  .tx_rdy_0(tx_rdy_wbm), .tx_rdy_1(), .tx_rdy_2( ), .tx_rdy_3(tx_rdy_ur),
                  .tx_req(tx_req), .tx_dout(tx_data), .tx_sop(tx_st), .tx_eop(tx_end), .tx_dwen(),
                  .tx_rdy(tx_rdy)
                  
);                           

wb_tlc wb_tlc(.clk_125(clk_125), .wb_clk(clk_125), .rstn(core_rst_n),
              .rx_data(rx_data), .rx_st(rx_st), .rx_end(rx_end), .rx_bar_hit(rx_bar_hit),
              .wb_adr_o(pcie_adr), .wb_dat_o(pcie_dat_o), .wb_cti_o(pcie_cti), .wb_we_o(pcie_we), .wb_sel_o(pcie_sel), .wb_stb_o(pcie_stb), .wb_cyc_o(pcie_cyc), .wb_lock_o(), 
              .wb_dat_i(pcie_dat_i), .wb_ack_i(pcie_ack),
              .pd_cr(pd_cr_wb), .pd_num(pd_num_wb), .ph_cr(ph_cr_wb), .npd_cr(npd_cr_wb), .nph_cr(nph_cr_wb),
              .tx_rdy(tx_rdy_wbm),
              .tx_req(tx_req_wbm), .tx_data(tx_dout_wbm), .tx_st(tx_sop_wbm), .tx_end(tx_eop_wbm), .tx_ca_cpl_recheck(1'b0), .tx_ca_cplh(tx_ca_cplh), .tx_ca_cpld(tx_ca_cpld),
              .comp_id({bus_num, dev_num, func_num}),
              .debug()
);


wb_arb #(.c_DATA_WIDTH(16),
         .S0_BASE     (32'h0000),
         .S1_BASE     (32'h4000),
         .S2_BASE     (32'h1000),
         .S3_BASE     (32'h5000)
) wb_arb (
    .clk(clk_125),
    .rstn(core_rst_n),
	
    // PCIe Master 
    .m0_dat_i(pcie_dat_o), 
    .m0_dat_o(pcie_dat_i), 
    .m0_adr_i(pcie_adr),
    .m0_sel_i(pcie_sel), 
    .m0_we_i(pcie_we), 
    .m0_cyc_i(pcie_cyc),
    .m0_cti_i(pcie_cti),
    .m0_stb_i(pcie_stb), 
    .m0_ack_o(pcie_ack), 
    .m0_err_o(), 
    .m0_rty_o(), 
    
    // DMA Master
    .m1_dat_i(16'd0), 
    .m1_dat_o(), 
    .m1_adr_i(32'd0),
    .m1_sel_i(2'd0), 
    .m1_we_i( 1'b0), 
    .m1_cyc_i(1'b0),
    .m1_cti_i(3'd0),
    .m1_stb_i(1'b0), 
    .m1_ack_o(), 
    .m1_err_o(), 
    .m1_rty_o(),         

    // GPIO 32-bit
    .s0_dat_i(gpio_dat_o), 
    .s0_dat_o(gpio_dat_i), 
    .s0_adr_o(gpio_adr), 
    .s0_sel_o(gpio_sel), 
    .s0_we_o (gpio_we), 
    .s0_cyc_o(gpio_cyc),
    .s0_cti_o (gpio_cti),
    .s0_stb_o(gpio_stb), 
    .s0_ack_i(gpio_ack), 
    .s0_err_i(gpio_err),
    .s0_rty_i(gpio_rty), 
    
    // DMA Slave
    .s1_dat_i(16'd0), 
    .s1_dat_o(), 
    .s1_adr_o(), 
    .s1_sel_o(), 
    .s1_we_o (), 
    .s1_cyc_o(),
    .s1_cti_o(),
    .s1_stb_o(), 
    .s1_ack_i(1'b0), 
    .s1_rty_i(1'b0),
    .s1_err_i(1'b0),
    
    // EBR
    .s2_dat_i(ebr_dat_o), 
    .s2_dat_o(ebr_dat_i), 
    .s2_adr_o(ebr_adr), 
    .s2_sel_o(ebr_sel), 
    .s2_we_o (ebr_we), 
    .s2_cyc_o(ebr_cyc),
    .s2_cti_o(ebr_cti),
    .s2_stb_o(ebr_stb), 
    .s2_ack_i(ebr_ack), 
    .s2_err_i(ebr_err),
    .s2_rty_i(ebr_rty),
    
    // Not used
    .s3_dat_i(16'd0), 
    .s3_dat_o(), 
    .s3_adr_o(), 
    .s3_sel_o(), 
    .s3_we_o (), 
    .s3_cyc_o(),
    .s3_cti_o(),
    .s3_stb_o(), 
    .s3_ack_i(1'b0),
    .s3_rty_i(1'b0), 
    .s3_err_i(1'b0)
);


             
wbs_gpio gpio(.wb_clk_i(clk_125), .wb_rst_i(~core_rst_n),
          .wb_dat_i(gpio_dat_i), .wb_adr_i(gpio_adr[8:0]), .wb_cti_i(gpio_cti), .wb_cyc_i(gpio_cyc), .wb_lock_i(1'b0), .wb_sel_i(gpio_sel), .wb_stb_i(gpio_stb), .wb_we_i(gpio_we),          
          .wb_dat_o(gpio_dat_o), .wb_ack_o(gpio_ack), .wb_err_o(gpio_err), .wb_rty_o(gpio_rty), 
          .switch_in(dip_switch), .led_out(led_out_int), 
          .dma_req(dma_req_gpio), .dma_ack(5'd0), 
          .ca_pd(tx_ca_pd), .ca_nph(tx_ca_nph), .dl_up(dl_up_int),
          .int_out()
);
assign led_out = ~led_out_int[13:0];          
                    
wbs_32kebr ebr(.wb_clk_i(clk_125), .wb_rst_i(~core_rst_n),
          .wb_dat_i(ebr_dat_i), .wb_adr_i(ebr_adr), .wb_cyc_i(ebr_cyc), .wb_sel_i(ebr_sel), .wb_stb_i(ebr_stb), .wb_we_i(ebr_we), .wb_cti_i(ebr_cti),
          .wb_dat_o(ebr_dat_o), .wb_ack_o(ebr_ack), .wb_err_o(ebr_err), .wb_rty_o(ebr_rty)          
);                          
  


// DEBUG OUPUT OPTIONS
//assign la = 34'd0; // cngd frm org
assign TP = 16'd0;
                  
                



endmodule

