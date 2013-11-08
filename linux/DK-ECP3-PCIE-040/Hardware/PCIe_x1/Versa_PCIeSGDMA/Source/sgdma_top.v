// $Id: sgdma_top.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $
// This file only exists to wrap the Buffer Descriptor Memory into the DMA module

module sgdma_top (
    input clk,
    input rstn,
    output a_cyc,
    output a_stb,
    output a_lock,
    output a_we,
    output [2:0] a_cti,
    output [7:0] a_sel,
    output [31:0] a_addr,
    output [63:0] a_wdat,
    input [63:0] a_rdat,
    input a_ack,
    input a_err,
    input a_retry,
    input a_eod,
    output b_cyc,
    output b_stb,
    output b_lock,
    output b_we,
    output [2:0] b_cti,
    output [7:0] b_sel,
    output [31:0] b_addr,
    output [63:0] b_wdat,
    input [63:0] b_rdat,
    input b_ack,
    input b_err,
    input b_retry,
    input b_eod,
    output b_active,
    input [31:0] saddr,
    input [31:0] swdat,
    output [31:0] srdat,
    input scyc,
    input sstb,
    input [3:0] ssel,
    input swe,
    output sack,
    output serr,
    input [4:0] dma_req,
    output [4:0] dma_ack,
    output [4:0] eventx,
    output [4:0] errorx,
    output [2:0] actchan,
    output [-1:0] subchan,
    output a_active,
    output [15:0] max_burst_size    
    );

wire [9:0] bd_waddr;     
wire [9:0] bd_raddr;     
wire [31:0] bd_wdat;     
wire [31:0] bd_rdat;   
reg bd_access;
reg bd_rval;

wire [31:0] srdat_p;
reg [70:0] to_slv;
reg [33:0] frm_slv;

sgdma dma (
    .clk				(clk),
    .rstn				(rstn),
    .rst_core                   (1'b0),  // not used, don't enable
    .a_addr			(a_addr[31:0]),
    .a_wdat			(a_wdat[63:0]),
    .a_rdat			(a_rdat[63:0]),
    .a_sel			(a_sel[7:0]),
    .a_we			(a_we),
    .a_cyc			(a_cyc),
    .a_stb			(a_stb),
    .a_lock			(a_lock),
    .a_cti			(a_cti[2:0]),
    .a_ack			(a_ack),
    .a_err			(a_err),
    .a_retry			(a_retry),
    .a_eod			(a_eod),
    .b_addr			(b_addr[31:0]),
    .b_wdat			(b_wdat[63:0]),
    .b_rdat			(b_rdat[63:0]),
    .b_sel			(b_sel[7:0]),
    .b_we			(b_we),
    .b_cyc			(b_cyc),
    .b_stb			(b_stb),
    .b_lock			(b_lock),
    .b_cti			(b_cti[2:0]),
    .b_ack			(b_ack),
    .b_err			(b_err),
    .b_retry			(b_retry),
    .b_eod			(b_eod),
    .b_active			(b_active),
    .saddr			(to_slv[70:39]),
    .swdat			(to_slv[38:7]),
    .srdat			(srdat_p[31:0]),
    .scyc			(to_slv[6]),
    .sstb			(to_slv[5]),
    .ssel			(to_slv[4:1]),
    .swe			(to_slv[0]),
    .sack			(sack_p),
    .serr			(serr_p),
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


// DMA Buffer Descriptor Memory			
pmi_ram_dp #(
	.pmi_wr_addr_depth	(1024),
	.pmi_wr_addr_width	(10),
	.pmi_wr_data_width	(32),
	.pmi_rd_addr_depth	(1024),
	.pmi_rd_addr_width	(10),
	.pmi_rd_data_width	(32),
	.pmi_regmode		("reg"),
	.pmi_gsr		("enable"),
	.pmi_resetmode		("async"),
	.pmi_init_file		("none"),
	.pmi_init_file_format	("binary"),
	.pmi_family		("SC"),
	.module_type		("pmi_ram_dp")
) bdram (
	.Data			(bd_wdat),
	.WrAddress		(bd_waddr),
	.RdAddress		(bd_raddr),
	.WrClock		(clk),
	.RdClock		(clk),
	.WrClockEn		(1'b1),
	.RdClockEn		(1'b1),
	.WE			(bd_we),
	.Reset			(~rstn),
	.Q			(bd_rdat)
);

always@( posedge clk or negedge rstn ) begin
  if( ~rstn ) 
  begin
    bd_access <= 1'b0;
    bd_rval <= 1'b0;
  end
  else 
  begin
    bd_access <= bd_we | bd_re;
    bd_rval <= bd_access;
  end
end

// this FF stage is used to provide a unique name for register in the slave
// interface.  This allows me to provide a multicycle from the DMA master to
// the slave interface since the DMA will never talk to itself.
always @(posedge clk or negedge rstn)
begin
  if (~rstn)
  begin
    to_slv  <= 71'd0;
    frm_slv <= 34'd0;     
  end
  else
  begin
    to_slv[70:39] <= saddr[31:0];
    to_slv[38:7]  <= swdat[31:0];
    to_slv[6]     <= scyc;
    to_slv[5]     <= sstb;
    to_slv[4:1]   <= ssel[3:0];
    to_slv[0]     <= swe;
    frm_slv <= {srdat_p[31:0], sack_p, serr_p};    
  end  
end

assign srdat[31:0] = frm_slv[33:2];
assign sack = frm_slv[1];
assign serr = frm_slv[0];



endmodule

