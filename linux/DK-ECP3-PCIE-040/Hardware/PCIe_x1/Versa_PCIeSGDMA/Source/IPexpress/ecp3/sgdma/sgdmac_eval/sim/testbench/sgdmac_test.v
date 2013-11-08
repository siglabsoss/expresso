// sgdmac_test -- testbench for SGDMAC core
//

`timescale 1 ns/ 100 ps

module sgdmac_test;

	localparam NUM_CHAN    = 16;
	localparam NUM_SUB     = 0;
	localparam DWIDTHA     = 32;
	localparam DWIDTHB     = 32;
	localparam AWIDTHA     = 32;
	localparam AWIDTHB     = 32;
	localparam PBUFF_SIZE  = 4096;
	localparam NUM_BD      = 256;
	localparam BIGENDIAN   = 1;
	localparam WEIGHTED    = 1;
	localparam BUFFER_STATUS = 1;
	localparam [7:0] SRC_BUS_SIZE = 8'hff;
	localparam [7:0] DST_BUS_SIZE = 8'hff;
	localparam [7:0] SRC_BUS_SEL = 8'hff;
	localparam [7:0] DST_BUS_SEL = 8'hff;
	localparam [31:0] SRC_BUS_ADDR = 32'hffffffff;
	localparam [31:0] DST_BUS_ADDR = 32'hffffffff;
	localparam [7:0] LOCK_VALUE = 8'hff;
	localparam [7:0] AUTORETRY_VALUE = 8'hff;
	localparam [7:0] RETRYTHRESH = 8'hff;

	localparam NUM_MASTA = 2;					// number of masters on bus A
	localparam NUM_MASTB = 1;					// number of masters on bus B
	localparam NUM_SLAVA = 2;					// number of slaves on bus A
	localparam NUM_SLAVB = 1;					// number of slaves on bus B

	localparam NUM_SELA = DWIDTHA/8;
	localparam NUM_SELB = DWIDTHB/8;

	reg clk, rst;
	
	wire [DWIDTHA-1:0]            ma_wdat [0:NUM_MASTA-1];	// WISHBONE A-Bus master write data buses
	reg  [DWIDTHA*NUM_MASTA-1:0]  ma_wdat_tmp;		// concatenated ma_wdat's
	wire [DWIDTHA-1:0]            ma_rdat;			// WISHBONE A-Bus master read data bus
	wire [AWIDTHA-1:0]            ma_adr  [0:NUM_MASTA-1];	// WISHBONE A-Bus master address buses
	reg  [AWIDTHA*NUM_MASTA-1:0]  ma_adr_tmp;		// concatenated ma_adr's
	wire [NUM_SELA-1:0]	      ma_sel  [0:NUM_MASTA-1];	// WISHBONE A-Bus master byte select buses
	reg  [NUM_SELA*NUM_MASTA-1:0] ma_sel_tmp;		// concatenated ma_sel's
	wire [2:0]	              ma_cti  [0:NUM_MASTA-1];	// WISHBONE A-Bus master byte select buses
	reg  [3*NUM_MASTA-1:0]        ma_cti_tmp;		// concatenated ma_cti's
	wire [NUM_MASTA-1:0]          ma_we;			// WISHBONE A-Bus master write enables
	wire [NUM_MASTA-1:0]	      ma_cyc;			// WISHBONE A-Bus master cyc's
	wire [NUM_MASTA-1:0]	      ma_stb;			// WISHBONE A-Bus master strobes
	wire [NUM_MASTA-1:0]	      ma_lock;			// WISHBONE A-Bus master locks
	wire [NUM_MASTA-1:0]	      ma_ack;			// WISHBONE A-Bus master acks
	wire [NUM_MASTA-1:0]	      ma_eod;			// WISHBONE A-Bus master eods
	wire [NUM_MASTA-1:0]	      ma_err;			// WISHBONE A-Bus master errs
	wire [NUM_MASTA-1:0]	      ma_rty;			// WISHBONE A-Bus master retries

	wire [DWIDTHB-1:0]            mb_wdat [0:NUM_MASTB-1];	// WISHBONE B-Bus master write data buses
	reg  [DWIDTHB*NUM_MASTB-1:0]  mb_wdat_tmp;	        // concatenated mb_wdat's
	wire [DWIDTHB-1:0]            mb_rdat;			// WISHBONE B-Bus master read data bus
	wire [AWIDTHB-1:0]            mb_adr  [0:NUM_MASTB-1];	// WISHBONE B-Bus master address buses
	reg  [AWIDTHB*NUM_MASTB-1:0]  mb_adr_tmp;	        // concatenated mb_adr's
	wire [NUM_SELB-1:0]	      mb_sel  [0:NUM_MASTB-1];	// WISHBONE B-Bus master byte select buses
	reg  [NUM_SELB*NUM_MASTB-1:0] mb_sel_tmp;	        // concatenated mb_sel's
	wire [2:0]	              mb_cti  [0:NUM_MASTB-1];	// WISHBONE B-Bus master byte select buses
	reg  [3*NUM_MASTB-1:0]        mb_cti_tmp;		// concatenated mb_cti's
	wire [NUM_MASTB-1:0]          mb_we;			// WISHBONE B-Bus master write enables
	wire [NUM_MASTB-1:0]	      mb_cyc;			// WISHBONE B-Bus master cyc's
	wire [NUM_MASTB-1:0]	      mb_stb;			// WISHBONE B-Bus master strobes
	wire [NUM_MASTB-1:0]	      mb_lock;			// WISHBONE B-Bus master locks
	wire [NUM_MASTB-1:0]	      mb_ack;			// WISHBONE B-Bus master acks
	wire [NUM_MASTB-1:0]	      mb_eod;			// WISHBONE B-Bus master eods
	wire [NUM_MASTB-1:0]	      mb_err;			// WISHBONE B-Bus master errs
	wire [NUM_MASTB-1:0]	      mb_rty;			// WISHBONE B-Bus master retries

	wire [DWIDTHA-1:0]            sa_rdat [0:NUM_SLAVA-1];	// WISHBONE A-Bus slave read data buses
	reg  [DWIDTHA*NUM_SLAVA-1:0]  sa_rdat_tmp;	        // concatenated sa_rdat's
	wire [DWIDTHA-1:0]            sa_wdat;			// WISHBONE A-Bus slave write data bus
	wire [AWIDTHA-1:0]            sa_adr;			// WISHBONE A-Bus slave address bus
	wire [NUM_SELA-1:0]           sa_sel;			// WISHBONE A-Bus slave byte select buses
	wire [2:0]                    sa_cti;			// WISHBONE A-Bus slave cycle type indicator
	wire                          sa_we;			// WISHBONE A-Bus slave write enables
	wire [NUM_SLAVA-1:0]	      sa_cyc;			// WISHBONE A-Bus slave cyc's
	wire                          sa_stb;			// WISHBONE A-Bus slave strobes
	wire [NUM_SLAVA-1:0]	      sa_ack;			// WISHBONE A-Bus slave acks
	wire [NUM_SLAVA-1:0]	      sa_eod;			// WISHBONE A-Bus slave eods
	wire [NUM_SLAVA-1:0]	      sa_err;			// WISHBONE A-Bus slave errs
	wire [NUM_SLAVA-1:0]	      sa_rty;			// WISHBONE A-Bus slave retries

	wire [DWIDTHB-1:0]            sb_rdat [0:NUM_SLAVB-1];	// WISHBONE B-Bus slave read data buses
	reg  [DWIDTHB*NUM_SLAVB-1:0]  sb_rdat_tmp;	        // concatenated sb_rdat's
	wire [DWIDTHB-1:0]            sb_wdat;			// WISHBONE B-Bus slave write data bus
	wire [AWIDTHB-1:0]            sb_adr;			// WISHBONE B-Bus slave address bus
	wire [NUM_SELB-1:0]	      sb_sel;			// WISHBONE B-Bus slave byte select buses
	wire [2:0]	              sb_cti;			// WISHBONE B-Bus slave cycle type indicator
	wire                          sb_we;			// WISHBONE B-Bus slave write enables
	wire [NUM_SLAVB-1:0]          sb_cyc;			// WISHBONE B-Bus slave cyc's
	wire               	      sb_stb;			// WISHBONE B-Bus slave strobes
	wire [NUM_SLAVB-1:0]	      sb_ack;			// WISHBONE B-Bus slave acks
	wire [NUM_SLAVB-1:0]	      sb_eod;			// WISHBONE B-Bus slave eods
	wire [NUM_SLAVB-1:0]	      sb_err;			// WISHBONE B-Bus slave errs
	wire [NUM_SLAVB-1:0]	      sb_rty;			// WISHBONE B-Bus slave retries

	reg  		              ctl_req;   // testbench control request
	wor		              ctl_ack;   // testbench control acknowledge
	reg [7:0]	              ctl_id;    // testbench control client id
	reg [7:0]	              ctl_op;    // testbench control opcode
	reg [31:0]	              ctl_addr;  // testbench control address
	reg [31:0]	              ctl_wdat;  // testbench control write data
	reg [31:0]	              ctl_mask;  // testbench control mask
	wor [31:0]	              ctl_rdat;  // testbench control read data
	wor [7:0]	              ctl_rtn;   // testbench control return code

	reg rst_core;
	reg [15:0] dma_req;
	wire [15:0] dma_ack;
	wire [15:0] auxctrl, auxstat;
	wire [((NUM_CHAN>8)?3:(NUM_CHAN>4)?2:(NUM_CHAN>2)?1:0):0] actchan;
	wire [((NUM_SUB>4)?2:(NUM_SUB>2)?1:0):0] subchan;
	wire a_active, b_active, p_active;
	wire [15:0] max_burst_size;
	wire [NUM_CHAN-1:0] eventx, errorx;

	integer testnum;
	integer testfail;

	integer i,j,c;

	// wb_master commands
	localparam WBMAST_CTL_RD = 0,
			   WBMAST_CTL_WR = 1,
			   WBMAST_CTL_VF = 2,
			   WBMAST_MEM_RD = 3,
			   WBMAST_MEM_WR = 4,
			   WBMAST_MEM_VF = 5,
			   WBMAST_BUS_RD = 6,
			   WBMAST_BUS_WR = 7,
			   WBMAST_BUS_VF = 8;
	// wb_master config addresses
	localparam WBMAST_TCNT  = 0,
			   WBMAST_TWDTH = 1,
			   WBMAST_AINCR = 2;

	// wb_slave commands
	localparam WBSLAV_CTL_RD = 0,
			   WBSLAV_CTL_WR = 1,
			   WBSLAV_CTL_VF = 2,
			   WBSLAV_MEM_RD = 3,
			   WBSLAV_MEM_WR = 4,
			   WBSLAV_MEM_VF = 5;
	// wb_slave config addresses
	localparam WBSLAV_RETRY_VALUE = 0;
	localparam WBSLAV_ENABLE_BURST = 1;
	localparam WBSLAV_EODCNT = 2;

	// ctl id's
	localparam WBMAST0       = 0;
	localparam WBSLAV0       = 1;
	localparam WBSLAV1       = 2;

	// wishbone addresses
	localparam SLAV0_BASE	 = 'h01000000;
	localparam SLAV1_BASE	 = 'h01000000;
	localparam SGDMAC_BASE	 = 'h02000000;
	// SGDMAC global addresses
	localparam IPID_ADDR	 = 0;
	localparam IPVER_ADDR	 = 4;
	localparam GCONTROL_ADDR = 8;
	localparam GSTATUS_ADDR	 = 'hC;
	localparam GEVENT_ADDR	 = 'h10;
	localparam GERROR_ADDR	 = 'h14;
	localparam GARBITER_ADDR = 'h18;
	localparam GAUX_ADDR	 = 'h1C;
	// SGDMAC channel addresses
	localparam CHAN_BASE_ADDR= 'h200 + SGDMAC_BASE;
	localparam CONTROLN_ADDR = 0;
	localparam STATUSN_ADDR  = 4;
	localparam CURSRCN_ADDR  = 8;
	localparam CURDSTN_ADDR  = 'hC;
	localparam CURXFRN_ADDR  = 'h10;
	localparam PBOFFSETN_ADDR= 'h14;
	// SGDMAC buffer descriptor addresses
	localparam BD_BASE_ADDR  = 'h400 + SGDMAC_BASE;
	localparam CONFIG0_ADDR  = 0;
	localparam CONFIG1_ADDR  = 4;
	localparam SRCADDR_ADDR  = 8;
	localparam DSTADDR_ADDR  = 'hC;

	// field offsets
	localparam VENDORID_OFF  = 16;
	localparam IPNUM_OFF     = 0;
	localparam MAJOR_OFF     = 24;
	localparam MINOR_OFF     = 16;
	localparam NUMCHAN_OFF   = 12;
	localparam NUMSUB_OFF    = 8;
	localparam BBUS_OFF      = 0;
	localparam PBUF_OFF      = 1;
	localparam ENDIAN_OFF    = 2;
	localparam CHENABLE_OFF  = 0;
	localparam CHMASK_OFF    = 16;
	localparam CHACTIVE_OFF  = 0;
	localparam RSTCORE_OFF   = 29;
	localparam AENABLE_OFF   = 29;
	localparam BENABLE_OFF   = 30;
	localparam GENABLE_OFF   = 31;
	localparam CHEVENT_OFF   = 0;
	localparam CHEVMSK_OFF   = 16;
	localparam CHERR_OFF     = 0;
	localparam CHERRMSK_OFF  = 16;
	localparam SHARE0_OFF    = 0;
	localparam SHARE1_OFF    = 4;
	localparam SHARE2_OFF    = 8;
	localparam SHARE3_OFF    = 12;
	localparam CHARBMSK_OFF  = 16;
	localparam AUXCTRL_OFF   = 0;
	localparam AUXSTAT_OFF   = 16;
	localparam PRIGRP_OFF    = 6;
	localparam ERRMASK_OFF   = 8;
	localparam BDBASE_OFF    = 16;
	localparam ENABLED_OFF   = 0;
	localparam REQUEST_OFF   = 1;
	localparam XFERCOMP_OFF  = 2;
	localparam EOD_OFF       = 3;
	localparam CLRCOMP_OFF   = 4;
	localparam RTRYCNT_OFF   = 7;
	localparam STATE_OFF     = 12;
	localparam ERRORS_OFF    = 16;
	localparam XFRCNT_OFF    = 0;
	localparam CURR_BD_OFF   = 16;
	localparam EOL_OFF       = 0;
	localparam SPLIT_OFF     = 1;
	localparam LOCK_OFF      = 2;
	localparam AUTORETRY_OFF = 3;
	localparam RETRYTHRESH_OFF = 4;
	localparam SRC_BUS_OFF   = 8;
	localparam SRCBUS_SIZE_OFF = 10;
	localparam SRCINCR_OFF   = 13;
	localparam DST_BUS_OFF   = 16;
	localparam DSTBUS_SIZE_OFF = 18;
	localparam DSTINCR_OFF   = 21;
	localparam SUBCHAN_OFF   = 24;
	localparam BD_NEXT_OFF   = 29;
	localparam BD_STATUS_OFF = 30;
	localparam BD_STATUS_EN_OFF = 31;
	localparam XFER_SIZE_OFF = 0;
	localparam BURST_SIZE_OFF = 16;


	task command;
		input integer idx, opx, addrx, wdatx, maskx;
		begin
			ctl_req  = 1;
			ctl_id   = idx;
			ctl_op   = opx;
			ctl_addr = addrx;
			ctl_wdat = wdatx;
			ctl_mask = maskx;
			wait(ctl_ack);
			ctl_req = 0;
			wait(!ctl_ack);
		end
	endtask

	initial begin

		testnum = 0;
		testfail = 0;
		rst_core = 0;
		dma_req = 0;
		ctl_req = 0;
		clk = 0;
		rst = 1;
		#300 rst = 0;

		`include "transfer_tests.v"

		// end of test sequence
		$display("########################");
		if( testfail )
			$display("FAILURES ENCOUNTERED");
		else
			$display("ALL TESTS PASSED");
		$display("########################");
		$stop;

	end

	// clock and reset
	always clk = #50 ~clk ;

	// testfail
	always@(*) testfail = testfail | (ctl_rtn > 0);

	// A-bus concatenations
	always@* begin
		ma_wdat_tmp = 0;
		ma_adr_tmp  = 0;
		ma_sel_tmp  = 0;
		ma_cti_tmp  = 0;
		for( i=NUM_MASTA-1; i>=0; i=i-1 ) begin
			ma_wdat_tmp = (ma_wdat_tmp<<DWIDTHA) | ma_wdat[i];
			ma_adr_tmp  = (ma_adr_tmp<<AWIDTHA)  | ma_adr[i];
			ma_sel_tmp  = (ma_sel_tmp<<NUM_SELA) | ma_sel[i];
			ma_cti_tmp  = (ma_cti_tmp<<3) | ma_cti[i];
		end
		sa_rdat_tmp = 0;
		for( i=NUM_SLAVA-1; i>=0; i=i-1 ) begin
			sa_rdat_tmp = (sa_rdat_tmp<<DWIDTHA) | sa_rdat[i];
		end
	end

	// A-Bus
	wb_bus #(
		.DWIDTH(DWIDTHA),
		.AWIDTH(AWIDTHA),
		.NUM_MAST(NUM_MASTA),
		.NUM_SLAV(NUM_SLAVA),
		.SLAVADDR0('h01000000),
		.SLAVADDR1('h02000000)
	) wba (
		.clk_i(clk),
		.rst_i(rst),
		.m_dat_i(ma_wdat_tmp),
		.m_dat_o(ma_rdat),
		.m_adr_i(ma_adr_tmp),
		.m_sel_i(ma_sel_tmp),
		.m_cti_i(ma_cti_tmp),
		.m_we_i(ma_we),
		.m_cyc_i(ma_cyc),
		.m_stb_i(ma_stb),
		.m_ack_o(ma_ack),
		.m_err_o(ma_err),
		.m_rty_o(ma_rty),
		.m_eod_o(ma_eod),
		.s_dat_i(sa_rdat_tmp),
		.s_dat_o(sa_wdat),
		.s_adr_o(sa_adr),
		.s_sel_o(sa_sel),
		.s_cti_o(sa_cti),
		.s_we_o(sa_we),
		.s_cyc_o(sa_cyc),
		.s_stb_o(sa_stb),
		.s_ack_i(sa_ack),
		.s_err_i(sa_err),
		.s_rty_i(sa_rty),
		.s_eod_i(sa_eod)
	);

	// wishbone master 
	wb_master #(
		.DWIDTH(DWIDTHA),
		.AWIDTH(AWIDTHA),
		.MEMSIZE(NUM_CHAN*PBUFF_SIZE),
		.CTLID(WBMAST0)
	) wbm0 (
		.clk(clk),
		.rst(rst),
		.adr(ma_adr[0]),
		.din(ma_rdat),
		.dout(ma_wdat[0]),
		.cyc(ma_cyc[0]),
		.stb(ma_stb[0]),
		.sel(ma_sel[0]),
		.we(ma_we[0]),
		.ack(ma_ack[0]),
		.eod(ma_eod[0]),
		.err(ma_err[0]),
		.rty(ma_rty[0]),
		.ctl_req(ctl_req),
		.ctl_ack(ctl_ack),
		.ctl_id(ctl_id),
		.ctl_op(ctl_op),
		.ctl_wdat(ctl_wdat),
		.ctl_addr(ctl_addr),
		.ctl_mask(ctl_mask),
		.ctl_rdat(ctl_rdat),
		.ctl_rtn(ctl_rtn)
	);

	// wishbone slave 0 
	wb_slave #(
		.DWIDTH(DWIDTHA),
		.AWIDTH(AWIDTHA),
		.FULL_ADDR_SIZE(0),
		.FULL_ADDR(0),
		.MEMSIZE(NUM_CHAN*PBUFF_SIZE),
		.BIGENDIAN(BIGENDIAN),
		.CTLID(WBSLAV0)
	) wbs0 (
		.clk(clk),
		.rst(rst),
		.adr(sa_adr),
		.din(sa_wdat),
		.dout(sa_rdat[0]),
		.cyc(sa_cyc[0]),
		.stb(sa_stb),
		.sel(sa_sel),
		.cti(sa_cti),
		.we(sa_we),
		.ack(sa_ack[0]),
		.eod(sa_eod[0]),
		.err(sa_err[0]),
		.rty(sa_rty[0]),
		.ctl_req(ctl_req),
		.ctl_ack(ctl_ack),
		.ctl_id(ctl_id),
		.ctl_op(ctl_op),
		.ctl_wdat(ctl_wdat),
		.ctl_addr(ctl_addr),
		.ctl_mask(ctl_mask),
		.ctl_rdat(ctl_rdat),
		.ctl_rtn(ctl_rtn)
	);

	sgdmac_block #(
		.NUM_CHAN(NUM_CHAN),
		.NUM_SUB(NUM_SUB),
		.DWIDTHA(DWIDTHA),
		.DWIDTHB(DWIDTHB),
		.AWIDTH(AWIDTHA),
		.PBUFF_SIZE(4096),
		.NUM_BD(256),
		.FULL_ADDR_SIZE(0),
		.FULL_ADDR(0),
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
	) dmac (
		.clk(clk),
		.rstn(!rst),
		.rst_core(rst_core),
		.a_addr(ma_adr[1]),
		.a_wdat(ma_wdat[1]),
		.a_rdat(ma_rdat),
		.a_sel(ma_sel[1]),
		.a_we(ma_we[1]),
		.a_cyc(ma_cyc[1]),
		.a_stb(ma_stb[1]),
		.a_lock(ma_lock[1]),
		.a_cti(ma_cti[1]),
		.a_ack(ma_ack[1]),
		.a_err(ma_err[1]),
		.a_retry(ma_rty[1]),
		.a_eod(ma_eod[1]),
		.b_addr(mb_adr[0]),
		.b_wdat(mb_wdat[0]),
		.b_rdat(mb_rdat),
		.b_sel(mb_sel[0]),
		.b_we(mb_we[0]),
		.b_cyc(mb_cyc[0]),
		.b_stb(mb_stb[0]),
		.b_lock(mb_lock[0]),
		.b_cti(mb_cti[0]),
		.b_ack(mb_ack[0]),
		.b_err(mb_err[0]),
		.b_retry(mb_rty[0]),
		.b_eod(mb_eod[0]),
		.saddr(sa_adr),
		.swdat(sa_wdat),
		.srdat(sa_rdat[1]),
		.scyc(sa_cyc[1]),
		.sstb(sa_stb),
		.ssel(sa_sel),
		.swe(sa_we),
		.sack(sa_ack[1]),
		.serr(sa_err[1]),
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
	assign sa_eod[1] = 0;
	assign sa_rty[1] = 0;

	// B-bus concatenations
	always@* begin
		mb_wdat_tmp = 0;
		mb_adr_tmp  = 0;
		mb_sel_tmp  = 0;
		for( j=NUM_MASTB-1; j>=0; j=j-1 ) begin
			mb_wdat_tmp = (mb_wdat_tmp<<DWIDTHB) | mb_wdat[j];
			mb_adr_tmp  = (mb_adr_tmp<<AWIDTHB)  | mb_adr[j];
			mb_sel_tmp  = (mb_sel_tmp<<NUM_SELB) | mb_sel[j];
			mb_cti_tmp  = (mb_cti_tmp<<3) | mb_cti[j];
		end
		sa_rdat_tmp = 0;
		for( j=NUM_SLAVB-1; j>=0; j=j-1 ) begin
			sb_rdat_tmp = (sb_rdat_tmp<<DWIDTHB) | sb_rdat[j];
		end
	end

	// B-Bus
	wb_bus #(
		.DWIDTH(DWIDTHB),
		.AWIDTH(AWIDTHB),
		.NUM_MAST(NUM_MASTB),
		.NUM_SLAV(NUM_SLAVB),
		.SLAVADDR0('h01000000)
	) wbb (
		.clk_i(clk),
		.rst_i(rst),
		.m_dat_i(mb_wdat_tmp),
		.m_dat_o(mb_rdat),
		.m_adr_i(mb_adr_tmp),
		.m_sel_i(mb_sel_tmp),
		.m_cti_i(mb_cti_tmp),
		.m_we_i(mb_we),
		.m_cyc_i(mb_cyc),
		.m_stb_i(mb_stb),
		.m_ack_o(mb_ack),
		.m_err_o(mb_err),
		.m_rty_o(mb_rty),
		.m_eod_o(mb_eod),
		.s_dat_i(sb_rdat_tmp),
		.s_dat_o(sb_wdat),
		.s_adr_o(sb_adr),
		.s_sel_o(sb_sel),
		.s_cti_o(sb_cti),
		.s_we_o(sb_we),
		.s_cyc_o(sb_cyc),
		.s_stb_o(sb_stb),
		.s_ack_i(sb_ack),
		.s_err_i(sb_err),
		.s_rty_i(sb_rty),
		.s_eod_i(sb_eod)
	);

	// wishbone slave 1 
	wb_slave #(
		.DWIDTH(DWIDTHB),
		.AWIDTH(AWIDTHB),
		.FULL_ADDR_SIZE(0),
		.FULL_ADDR(0),
		.MEMSIZE(NUM_CHAN*PBUFF_SIZE),
		.BIGENDIAN(BIGENDIAN),
		.CTLID(WBSLAV1)
	) wbs1 (
		.clk(clk),
		.rst(rst),
		.adr(sb_adr),
		.din(sb_wdat),
		.dout(sb_rdat[0]),
		.cyc(sb_cyc[0]),
		.stb(sb_stb),
		.sel(sb_sel),
		.cti(sb_cti),
		.we(sb_we),
		.ack(sb_ack[0]),
		.eod(sb_eod[0]),
		.err(sb_err[0]),
		.rty(sb_rty[0]),
		.ctl_req(ctl_req),
		.ctl_ack(ctl_ack),
		.ctl_id(ctl_id),
		.ctl_op(ctl_op),
		.ctl_wdat(ctl_wdat),
		.ctl_addr(ctl_addr),
		.ctl_mask(ctl_mask),
		.ctl_rdat(ctl_rdat),
		.ctl_rtn(ctl_rtn)
	);

endmodule
