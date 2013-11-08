//=============================================================================
// Verilog module generated by IPExpress
// Filename: ddr3core_inst.v
// Copyright(c) 2010 Lattice Semiconductor Corporation. All rights reserved.   
//=============================================================================

//------------------------------------------------
// Username module instantiation template
//------------------------------------------------

ddr3core U1_ddr3core (

   // Clock and reset
   .sclk2x             (sclk2x),
   .eclk               (eclk),
   .sclk               (sclk),
   .rst_n              (rst_n),
   .mem_rst_n          (mem_rst_n),  			

   // Input signals from the User I/F  
   .init_start         (init_start),  
   .cmd                (cmd),
   .addr               (addr),
   .cmd_burst_cnt      (cmd_burst_cnt),
   .cmd_valid          (cmd_valid),   
   .ofly_burst_len     (ofly_burst_len), 
   .write_data         (write_data),
   .datain_rdy         (datain_rdy),
   .data_mask          (data_mask),
   .read_pulse_tap     (read_pulse_tap),


   // Output signals to the User I/F
   .cmd_rdy            (cmd_rdy),
   .init_done          (init_done),
   .read_data          (read_data),
   .read_data_valid    (read_data_valid),
   .wl_rst_datapath    (wl_rst_datapath),
   .uddcntln           (uddcntln),
   .dqsbufd_rst        (dqsbufd_rst),
   .dqsdel             (dqsdel),
   .clocking_good      (clocking_good),


   // Memory side signals 
   .em_ddr_reset_n     (em_ddr_reset_n),
   .em_ddr_data        (em_ddr_data),
   .em_ddr_dqs         (em_ddr_dqs),
   .em_ddr_clk         (em_ddr_clk),
   .em_ddr_cke         (em_ddr_cke),
   .em_ddr_ras_n       (em_ddr_ras_n),
   .em_ddr_cas_n       (em_ddr_cas_n),
   .em_ddr_we_n        (em_ddr_we_n),
   .em_ddr_cs_n        (em_ddr_cs_n),
   .em_ddr_odt         (em_ddr_odt),
   .em_ddr_dm          (em_ddr_dm),           
   .em_ddr_ba          (em_ddr_ba),
   .em_ddr_addr        (em_ddr_addr)
);

