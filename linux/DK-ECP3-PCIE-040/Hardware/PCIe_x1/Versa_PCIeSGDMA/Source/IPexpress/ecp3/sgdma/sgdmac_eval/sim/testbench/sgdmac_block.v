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

module sgdmac_block # (
	parameter NUM_CHAN       = 16,		// number of channels
	parameter NUM_SUB        = 4,		// number of sub-channels per channel
	parameter DWIDTHA        = 32,		// data width of busA
	parameter DWIDTHB        = 32,		// data width of busB
	parameter AWIDTH         = 32,		// address width, all buses
	parameter PBUFF_SIZE     = 4096,	// packet buffer size
	parameter NUM_BD	     = 256,	// number of buffer descriptors
	parameter FULL_ADDR_SIZE = 0,		// number of address bits to match
	parameter FULL_ADDR      = 0,		// value of match address
	parameter BIGENDIAN		 = 0,	// 1 means Big-Endian byte order
	parameter WEIGHTED		 = 0,	// 0-round robin, 1-weighted round robin
	parameter BUFFER_STATUS  = 0,		// 1 enables buffer status check
	parameter [7:0] SRC_BUS_SIZE = 8'hff,	// source bus size ('hff means size comes from BD)
	parameter [7:0] DST_BUS_SIZE = 8'hff,	// destination bus size ('hff means size comes BD)
	parameter [7:0] SRC_BUS_SEL = 8'hff,	// source bus select ('hff means size comes from BD)
	parameter [7:0] DST_BUS_SEL = 8'hff,	// destination bus select ('hff means size comes from BD)
	parameter [31:0] SRC_BUS_ADDR = 32'hffffffff,	// source bus address ('hffffffff means address comes from BD)
	parameter [31:0] DST_BUS_ADDR = 32'hffffffff,	// destination bus address ('hffffffff means address comes from BD)
	parameter [7:0] LOCK_VALUE = 8'hff,		// lock value ('hff means lock comes from BD)
	parameter [7:0] AUTORETRY_VALUE = 8'hff,	// autoretry value ('hff means autoretry comes from BD)
	parameter [7:0] RETRYTHRESH = 8'hff		// retry threshold ('hff means threshold comes from BD)
)(
	input clk,
	input rstn,
	input rst_core,

	// Master Interface, Bus A
	output wire [AWIDTH-1:0] a_addr,
	output wire [DWIDTHA-1:0] a_wdat,
	input [DWIDTHA-1:0] a_rdat,
	output wire [DWIDTHA/8-1:0] a_sel,
	output wire a_we,
	output wire a_cyc,
	output wire a_stb,
	output wire a_lock,
	output wire [2:0] a_cti,
	input a_ack,
	input a_err,
	input a_retry,
	input a_eod,

	// Master Interface, Bus B
	output wire [AWIDTH-1:0] b_addr,
	output wire [DWIDTHB-1:0] b_wdat,
	input [DWIDTHB-1:0] b_rdat,
	output wire [DWIDTHB/8-1:0] b_sel,
	output wire b_we,
	output wire b_cyc,
	output wire b_stb,
	output wire b_lock,
	output wire [2:0] b_cti,
	input b_ack,
	input b_err,
	input b_retry,
	input b_eod,

	// Slave Interface
	input [AWIDTH-1:0] saddr,
	input [31:0] swdat,
	output wire [31:0] srdat,
	input scyc,
	input sstb,
	input [3:0] ssel,
	input swe,
	output wire sack,
	output wire serr,

	// Interrupt and Control
	input [NUM_CHAN-1:0] dma_req,
	output wire [NUM_CHAN-1:0] dma_ack,
	output wire [NUM_CHAN-1:0] eventx,
	output wire [NUM_CHAN-1:0] errorx,
	output wire [((NUM_CHAN>8)?3:(NUM_CHAN>4)?2:(NUM_CHAN>2)?1:0):0] actchan,
	output wire [((NUM_SUB>4)?2:(NUM_SUB>2)?1:0):0] subchan,
	output wire a_active,
	output wire b_active,
	output wire p_active,
	output wire [15:0] max_burst_size,
	output wire [15:0] auxctrl,
	input [15:0] auxstat
);

	localparam NUM_SELA = DWIDTHA/8;
	localparam NUM_SELB = DWIDTHB/8;

	localparam BD_AWIDTH = (NUM_BD>128)?10:(NUM_BD>64)?9:(NUM_BD>32)?8:(NUM_BD>16)?7:
						   (NUM_BD>8)?6:(NUM_BD>4)?5:(NUM_BD>2)?4:(NUM_BD>1)?3:2;
	localparam PB_AWIDTH = (PBUFF_SIZE>32768)?16:(PBUFF_SIZE>16384)?15:(PBUFF_SIZE>8192)?14:(PBUFF_SIZE>4096)?13:
						   (PBUFF_SIZE>2048)?12:(PBUFF_SIZE>1024)?11:(PBUFF_SIZE>512)?10:(PBUFF_SIZE>256)?9:
						   (PBUFF_SIZE>128)?8:(PBUFF_SIZE>64)?7:(PBUFF_SIZE>32)?6:(PBUFF_SIZE>16)?5:4;


	wire rst = !rstn;

	// buffer descriptor RAM interface
	wire [BD_AWIDTH-1:0] bd_waddr, bd_raddr;
	wire [31:0]	bd_wdat, bd_rdat;
	wire bd_we, bd_re;
	reg bd_rval;

	// packet buffer RAM interface
	wire [PB_AWIDTH-1:0] pb_waddr, pb_raddr;
	wire [DWIDTHA-1:0] pb_wdat, pb_rdat;
	wire pb_write, pb_read;
	reg  pb_rval;

	integer i;

	sgdmac_core #(
		.NUM_CHAN(NUM_CHAN),
		.NUM_SUB(NUM_SUB),
		.DWIDTHA(DWIDTHA),
		.DWIDTHB(DWIDTHB),
		.AWIDTH(32),
		.PBUFF_SIZE(PBUFF_SIZE),
		.NUM_BD(NUM_BD),
		.FULL_ADDR_SIZE(FULL_ADDR_SIZE),
		.FULL_ADDR(FULL_ADDR),
		.BIGENDIAN(BIGENDIAN),
		.WEIGHTED(WEIGHTED),
		.BUFFER_STATUS(BUFFER_STATUS),
		.SRC_BUS_SIZE(SRC_BUS_SIZE),
		.DST_BUS_SIZE(DST_BUS_SIZE),
		.SRC_BUS_SEL(SRC_BUS_SEL),
		.DST_BUS_SEL(DST_BUS_SEL),
		.SRC_BUS_ADDR(SRC_BUS_ADDR),
		.DST_BUS_ADDR(DST_BUS_ADDR),
		.LOCK_VALUE(LOCK_VALUE),
		.AUTORETRY_VALUE(AUTORETRY_VALUE),
		.RETRYTHRESH(RETRYTHRESH)
	) core0 (
		.clk(clk),
		.rstn(rstn),
		.rst_core(rst_core),
		.a_addr(a_addr),
		.a_wdat(a_wdat),
		.a_rdat(a_rdat),
		.a_sel(a_sel),
		.a_we(a_we),
		.a_cyc(a_cyc),
		.a_stb(a_stb),
		.a_lock(a_lock),
		.a_cti(a_cti),
		.a_ack(a_ack),
		.a_err(a_err),
		.a_retry(a_retry),
		.a_eod(a_eod),
		.b_addr(b_addr),
		.b_wdat(b_wdat),
		.b_rdat(b_rdat),
		.b_sel(b_sel),
		.b_we(b_we),
		.b_cyc(b_cyc),
		.b_stb(b_stb),
		.b_lock(b_lock),
		.b_cti(b_cti),
		.b_ack(b_ack),
		.b_err(b_err),
		.b_retry(b_retry),
		.b_eod(b_eod),
		.saddr(saddr),
		.swdat(swdat),
		.srdat(srdat),
		.scyc(scyc),
		.sstb(sstb),
		.ssel(ssel),
		.swe(swe),
		.sack(sack),
		.serr(serr),
		.bd_waddr(bd_waddr),
		.bd_raddr(bd_raddr),
		.bd_wdat(bd_wdat),
		.bd_rdat(bd_rdat),
		.bd_we(bd_we),
		.bd_re(bd_re),
		.bd_rval(bd_rval),
		.pb_waddr(pb_waddr),
		.pb_raddr(pb_raddr),
		.pb_wdat(pb_wdat),
		.pb_rdat(pb_rdat),
		.pb_write(pb_write),
		.pb_read(pb_read),
		.pb_rval(pb_rval),
		.dma_req(dma_req),
		.dma_ack(dma_ack),
		.eventx(eventx),
		.errorx(errorx),
		.actchan(actchan),
		.subchan(subchan),
		.a_active(a_active),
		.b_active(b_active),
		.p_active(p_active),
		.max_burst_size(max_burst_size),
		.auxctrl(auxctrl),
		.auxstat(auxstat)
	);
	
	// buffer descriptor RAM
	pmi_ram_dp #(
		.pmi_wr_addr_depth		(NUM_BD*4),
		.pmi_wr_addr_width		(BD_AWIDTH),
		.pmi_wr_data_width		(32),
		.pmi_rd_addr_depth		(NUM_BD*4),
		.pmi_rd_addr_width		(BD_AWIDTH),
		.pmi_rd_data_width		(32),
		.pmi_regmode			("reg"),
		.pmi_gsr				("enable"),
		.pmi_resetmode			("async"),
		.pmi_init_file			("none"),
		.pmi_init_file_format	("binary"),
		.pmi_family				("SCM"),
		.module_type			("pmi_ram_dp")
	) bdram (
		.Data					(bd_wdat),
		.WrAddress				(bd_waddr),
		.RdAddress				(bd_raddr),
		.WrClock				(clk),
		.RdClock				(clk),
		.WrClockEn				(1'b1),
		.RdClockEn				(1'b1),
		.WE						(bd_we),
		.Reset					(rst),
		.Q						(bd_rdat)
	);
	// bd_rval generation
	reg bd_access;
	always@( posedge clk or posedge rst ) begin
		if( rst ) begin
			bd_access <= 0;
			bd_rval <= 0;
		end
		else begin
			bd_access <= bd_re;
			bd_rval <= bd_access;
		end
	end

	// packet buffer RAM
	pmi_ram_dp #(
		.pmi_wr_addr_depth		(PBUFF_SIZE),
		.pmi_wr_addr_width		(PB_AWIDTH),
		.pmi_wr_data_width		(DWIDTHA),
		.pmi_rd_addr_depth		(PBUFF_SIZE),
		.pmi_rd_addr_width		(PB_AWIDTH),
		.pmi_rd_data_width		(DWIDTHA),
		.pmi_regmode			("reg"),
		.pmi_gsr				("enable"),
		.pmi_resetmode			("async"),
		.pmi_init_file			("none"),
		.pmi_init_file_format	("binary"),
		.pmi_family				("SCM"),
		.module_type			("pmi_ram_dp")
	) pbram (
		.Data					(pb_wdat),
		.WrAddress				(pb_waddr),
		.RdAddress				(pb_raddr),
		.WrClock				(clk),
		.RdClock				(clk),
		.WrClockEn				(1'b1),
		.RdClockEn				(1'b1),
		.WE						(pb_write),
		.Reset					(rst),
		.Q						(pb_rdat)
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

endmodule
