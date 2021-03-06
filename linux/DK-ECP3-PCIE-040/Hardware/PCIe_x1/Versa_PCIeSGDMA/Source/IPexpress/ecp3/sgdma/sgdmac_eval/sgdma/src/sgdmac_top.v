//=============================================================================
// Verilog module generated by IPExpress    01/20/2010    14:16:54          
// Filename: sgdmac_top.v 
// Copyright(c) 2008 Lattice Semiconductor Corporation. All rights reserved.   
//=============================================================================

// =============================================================================
//                           COPYRIGHT NOTICE
// Copyright 2007 (c) Lattice Semiconductor Corporation
// ALL RIGHTS RESERVED
// This confidential and proprietary software may be used only as authorised
// by a licensing agreement from Lattice Semiconductor Corporation.
// The entire notice above must be reproduced on all authorized copies and
// copies may only be made to the extent permitted by a licensing agreement
// from Lattice Semiconductor Corporation.
//
// Lattice Semiconductor Corporation    TEL : 1-800-Lattice (USA and Canada)
// 5555 NE Moore Court                        408-826-6000 (other locations)
// Hillsboro, OR 97124                  web  : http://www.latticesemi.com/
// U.S.A                                email: techsupport@latticesemi.com
// =============================================================================
//                         FILE DETAILS
// Project          : Scatter-Gather DMA Controller
// File             : sgdmac_core.v
// Title            :
// Dependencies     :
// Description      : SGDMAC core top level
// =============================================================================
//
//  REVISION HISTORY
//
// Revision 0.0  2007/03/15 dap
// Initial version
//
// =============================================================================

module sgdmac_top (

    input clk,
    input rstn,
    input rst_core,
    
    // Slave Interface
    input [32-1:0] sgs_addr,
    input [31:0]             sgs_wdat,
    output wire [31:0]       sgs_rdat,
    input                    sgs_cyc,
    input                    sgs_stb,
    input [3:0]              sgs_sel,
    input                    sgs_we,
    output wire              sgs_ack,
    output wire              sgs_err,
    
    // Interrupt and Control
    input       [2-1:0]       dma_req,
    output wire [2-1:0]       dma_ack,
    output wire [2-1:0]       eventx,
    output wire [2-1:0]       errorx,
    output wire [1-1:0] actchan,
    output wire [1-1:0]  subchan,
    output wire                            a_active,
    output wire                            b_active,
    output wire                            p_active,
    output wire [15:0]                     max_burst_size,
    output wire [15:0]                     auxctrl,
    input       [15:0]                     auxstat
);

   // Master Interface, Bus A
   wire [32-1:0]    sga_addr;
   wire [64-1:0]   sga_wdat;
   wire [64-1:0]   sga_rdat;
   wire [8-1:0] sga_sel;
   wire                       sga_we;
   wire                       sga_cyc;
   wire                       sga_stb;
   wire                       sga_lock;
   wire [2:0]                 sga_cti;
   wire                       sga_ack;
   wire                       sga_err;
   wire                       sga_retry;
   wire                       sga_eod;
   
   // Master Interface, Bus B
   wire [32-1:0]    sgb_addr;
   wire [64-1:0]   sgb_wdat;
   wire [64-1:0]   sgb_rdat;
   wire [8-1:0] sgb_sel;
   wire                       sgb_we;
   wire                       sgb_cyc;
   wire                       sgb_stb;
   wire                       sgb_lock;
   wire [2:0]                 sgb_cti;
   wire                       sgb_ack;
   wire                       sgb_err;
   wire                       sgb_retry;
   wire                       sgb_eod;
   
   // buffer descriptor RAM interface
   wire [10-1:0] bd_waddr;
   wire [10-1:0] bd_raddr;
   wire [31:0]	        bd_wdat;
   wire [31:0]	        bd_rdat;
   wire                 bd_we;
   wire                 bd_re;
   reg                  bd_rval;
   
   // packet buffer RAM interface
   wire [1-1:0]     pb_waddr;
   wire [1-1:0]     pb_raddr;
   wire [64-1:0] pb_wdat;
   wire [64-1:0] pb_rdat;
   wire                     pb_write;
   wire                     pb_read;
   reg                      pb_rval;
   
   wire                     rst;
   integer i;
   
   assign rst = ~rstn;
   
    // instantiate core
//=============================================================================
// Verilog module generated by IPExpress    01/20/2010    14:16:54          
// Filename: sgdma_inst.v                                               
// Copyright(c) 2005 Lattice Semiconductor Corporation. All rights reserved.   
//=============================================================================

//--------------------------------------------------------------
// sgdma instance template               
//--------------------------------------------------------------

sgdma sgdma_inst (
    .clk				(clk),
    .rstn				(rstn),
    .rst_core			(rst_core),
    .a_addr			(sga_addr[31:0]),
    .a_wdat			(sga_wdat[63:0]),
    .a_rdat			(sga_rdat[63:0]),
    .a_sel			(sga_sel[7:0]),
    .a_we				(sga_we),
    .a_cyc			(sga_cyc),
    .a_stb			(sga_stb),
    .a_lock			(sga_lock),
    .a_cti			(sga_cti[2:0]),
    .a_ack			(sga_ack),
    .a_err			(sga_err),
    .a_retry			(sga_retry),
    .a_eod			(sga_eod),
    .b_addr			(sgb_addr[31:0]),
    .b_wdat			(sgb_wdat[63:0]),
    .b_rdat			(sgb_rdat[63:0]),
    .b_sel			(sgb_sel[7:0]),
    .b_we				(sgb_we),
    .b_cyc			(sgb_cyc),
    .b_stb			(sgb_stb),
    .b_lock			(sgb_lock),
    .b_cti			(sgb_cti[2:0]),
    .b_ack			(sgb_ack),
    .b_err			(sgb_err),
    .b_retry			(sgb_retry),
    .b_eod			(sgb_eod),
    .b_active			(b_active),
    .saddr                        (sgs_addr[31:0]),
    .swdat			(sgs_wdat[31:0]),
    .srdat			(sgs_rdat[31:0]),
    .scyc				(sgs_cyc),
    .sstb				(sgs_stb),
    .ssel				(sgs_sel[3:0]),
    .swe				(sgs_we),
    .sack				(sgs_ack),
    .serr				(sgs_err),
    .bd_waddr			(bd_waddr[9:0]),
    .bd_raddr			(bd_raddr[9:0]),
    .bd_wdat			(bd_wdat[31:0]),
    .bd_rdat			(bd_rdat[31:0]),
    .bd_we			(bd_we),
    .bd_re			(bd_re),
    .bd_rval			(bd_rval),
    .dma_req			(dma_req[1:0]),
    .dma_ack			(dma_ack[1:0]),
    .eventx			(eventx[1:0]),
    .errorx			(errorx[1:0]),
    .actchan			(actchan[0:0]),
    .subchan			(subchan[0:0]),
    .a_active			(a_active),
    .max_burst_size	(max_burst_size[15:0])
);


    
    GSR GSR_INST(.GSR(rstn));
    PUR PUR_INST(.PUR(1'b1));
    
    // buffer descriptor RAM
    pmi_ram_dp #(
    	.pmi_wr_addr_depth   (256*4),
    	.pmi_wr_addr_width   (10),
    	.pmi_wr_data_width   (32),
    	.pmi_rd_addr_depth   (256*4),
    	.pmi_rd_addr_width   (10),
    	.pmi_rd_data_width   (32),
    	.pmi_regmode         ("reg"),
    	.pmi_gsr             ("disable"),
    	.pmi_resetmode       ("sync"),
    	.pmi_init_file       ("none"),
    	.pmi_init_file_format("binary"),
    	.pmi_family          ("ECP2"),
    	.module_type         ("pmi_ram_dp")
    ) bdram (
    	.Data       (bd_wdat),
    	.WrAddress  (bd_waddr),
    	.RdAddress  (bd_raddr),
    	.WrClock    (clk),
    	.RdClock    (clk),
    	.WrClockEn  (1'b1),
    	.RdClockEn  (1'b1),
    	.WE         (bd_we),
    	.Reset      (rst),
    	.Q          (bd_rdat)
    );
    // bd_rval generation
    reg bd_access;
    always@( posedge clk or posedge rst ) begin
       if( rst ) begin
          bd_access <= 0;
          bd_rval <= 0;
       end
       else begin
          bd_access <= bd_we | bd_re;
          bd_rval <= bd_access;
       end
    end

	// packet buffer RAM
    pmi_ram_dp #(
        .pmi_wr_addr_depth   (0),
        .pmi_wr_addr_width   (1),
        .pmi_wr_data_width   (64),
        .pmi_rd_addr_depth   (0),
        .pmi_rd_addr_width   (1),
        .pmi_rd_data_width   (64),
        .pmi_regmode         ("reg"),
        .pmi_gsr             ("disable"),
        .pmi_resetmode       ("sync"),
        .pmi_init_file       ("none"),
        .pmi_init_file_format("binary"),
        .pmi_family          ("ECP2"),
        .module_type         ("pmi_ram_dp")
    ) pbram (
        .Data      (pb_wdat),
        .WrAddress (pb_waddr),
        .RdAddress (pb_raddr),
        .WrClock   (clk),
        .RdClock   (clk),
        .WrClockEn (1'b1),
        .RdClockEn (1'b1),
        .WE        (pb_write),
        .Reset     (rst),
        .Q         (pb_rdat)
    );
	// packet buffer logic
    reg pb_access;
    always@( posedge clk or posedge rst ) begin
        if( rst ) begin
           pb_access <= 0;
           pb_rval <= 0;
        end
        else begin
           pb_access <= pb_write | pb_read;
           pb_rval <= pb_access;
        end
    end

	// bogus wishbone port terminators
    wbterm #( .DWIDTH(64), 
              .AWIDTH(32) )
    wbterma ( .clk  (clk), 
              .rstn (rstn),
              .cyc  (sga_cyc), 
              .stb  (sga_stb), 
              .we   (sga_we),
              .sel  (sga_sel), 
              .addr (sga_addr), 
              .wdat (sga_wdat),
              .rdat (sga_rdat),
              .lock (sga_lock), 
              .cti  (sga_cti),
              .ack  (sga_ack), 
              .err  (sga_err), 
              .retry(sga_retry), 
              .eod  (sga_eod) );

    wbterm #( .DWIDTH(64), 
              .AWIDTH(32) )
    wbtermb ( .clk  (clk), 
              .rstn (rstn),
              .cyc  (sgb_cyc), 
              .stb  (sgb_stb), 
              .we   (sgb_we),
              .sel  (sgb_sel), 
              .addr (sgb_addr), 
              .wdat (sgb_wdat),
              .rdat (sgb_rdat), 
              .lock (sgb_lock), 
              .cti  (sgb_cti),
              .ack  (sgb_ack), 
              .err  (sgb_err), 
              .retry(sgb_retry), 
              .eod  (sgb_eod) );

endmodule

// bogus wishbone port terminator
module wbterm #(
       parameter DWIDTH = 32,
       parameter AWIDTH = 32
	)(
       input clk,
       input rstn,
       input cyc,
       input stb,
       input we,
       input      [DWIDTH/8-1:0] sel,
       input      [AWIDTH-1:0]   addr,
       input      [DWIDTH-1:0]   wdat,
       output reg [DWIDTH-1:0]   rdat,
       input       lock,
       input [2:0] cti,
       output reg  ack,
       output reg  err,
       output reg  retry,
       output reg  eod
	);

  reg [DWIDTH-1:0] mem ;
  reg [AWIDTH-1:0] addr_r ;
  
  always@( posedge clk or negedge rstn ) begin
    if( !rstn ) begin
       mem <= 0;
       { ack, err, retry, eod } <= 0;
       rdat <= 0;
       addr_r <= 0;
    end
    else begin
       addr_r <= addr;
       { ack, err, retry, eod } <= { lock, cti };
       if ( addr_r == 0 ) 
          rdat <= mem;
       if ( cyc && stb && we ) 
          mem <= wdat;
    end
  end

endmodule
