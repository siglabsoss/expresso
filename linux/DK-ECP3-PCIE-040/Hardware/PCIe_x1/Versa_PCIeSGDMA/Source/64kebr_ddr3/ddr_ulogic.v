// =============================================================================
//                           COPYRIGHT NOTICE
// Copyright 2010 (c) Lattice Semiconductor Corporation
// ALL RIGHTS RESERVED
// This confidential and proprietary software may be used only as authorised by
// a licensing agreement from Lattice Semiconductor Corporation.
// The entire notice above must be reproduced on all authorized copies and
// copies may only be made to the extent permitted by a licensing agreement from
// Lattice Semiconductor Corporation.
//
// Lattice Semiconductor Corporation        TEL : 1-800-Lattice (USA and Canada)
// 5555 NE Moore Court                            408-826-6000 (other locations)
// Hillsboro, OR 97124                     web  : http://www.latticesemi.com/
// U.S.A                                   email: techsupport@lscc.com
// =============================================================================
//                         FILE DETAILS
// Project          : DDR3 IOPB Demo 
// File             : ddr_ulogic.v 
// Title            : ddr_ulogic
// Dependencies     : 
// Description      : DDR3 user logic block, command and address generation
//					 
//		      
//
// =============================================================================
//                        REVISION HISTORY
// Version          : 1.0
// Author(s)        : Kyoho Lee
// Mod. Date        : June, 2010
// Changes Made     :
//
// Version          : 2.0
// Author(s)        : ARathore
// Mod. Date        : March, 2011
// Changes Made     :State machine changed to work with wishbone signals
// =============================================================================

`timescale 1 ps / 1 ps
`include "ddr3_test_params.v"
module ddr_ulogic 
(
//---------inputs-------------
	sclk,
	reset_n,
	init_done,
	cmd_rdy,
	datain_rdy,
	read_data,
	read_data_valid,

        wb_sel,//frm wishbone
        wb_addr,// from wishbone
        wb_data_wr,//from wishbone
        wb_stb, //frm wishbone
        wb_we,
 
	OdtSel,
	RankSel,
//---------outputs-------------
	ClkSel,
	Odt,
	cmd,
	cmd_valid,
	addr,
	cmd_burst_cnt,
	write_data,
	init_start,
	mem_rst_n,
	data_mask,
	otf_bl_sel,

    st_mach_idle,
 datain_count,
	err_det
	);




// ==============================================================================
// define all inputs / outputs
// ==============================================================================
input						sclk;
input						reset_n;
input						init_done;
input						cmd_rdy;
input						datain_rdy;
input	[`DSIZE-1:0]	                	read_data;
input						read_data_valid;

input   [31:0]				wb_addr;
input   [63:0]				wb_data_wr;
input   [1:0]				wb_sel;
input                       wb_stb;
input                       wb_we;
   

input	[1:0]	         			OdtSel;
input	[1:0]    				RankSel;

output						init_start;
output						mem_rst_n;
output [3:0]    				cmd;
output						cmd_valid;
output [`ADDR_WIDTH-1:0] 			addr;
output	[`DSIZE-1:0]	            write_data;
output	[`DSIZE / 8-1:0]		    data_mask;
output	[4:0]				        cmd_burst_cnt;
output						err_det;
output						otf_bl_sel;
output						ClkSel;
output	[1:0]			         	Odt;

output 	reg			                st_mach_idle;
   output  [4:0]					datain_count;
   

// ==============================================================================
// internal signals
// ==============================================================================
reg		[3:0]				cmdgen;
reg		[3:0]				cmd;
reg							cmd_valid;
reg		[`ADDR_WIDTH-1:0]	addr;

reg		[15:0]				init_cnt;
reg							init_srvcd;
reg							rst_srvcd;
reg							init_start_hit;
reg							init_start_hit_1;
reg							init_start;
reg							mem_rst_n;

reg		[`ADDR_WIDTH-1:0]	addr_rd /* synthesis syn_multstyle="logic" */;
reg		[`ADDR_WIDTH-1:0]	addr_wr /* synthesis syn_multstyle="logic" */;
reg		[8:0]				addr_interval /* synthesis syn_multstyle="logic" */;

reg		[`DSIZE-1:0]		write_data;
`ifdef WrRqDDelay_2
 reg						datain_rdy_d;
`endif
reg							clr_gen;
reg							err_det = 1'b0;
reg							mismatch;
reg							read_data_valid_d;
reg							read_data_valid_d2;
reg							otf_bl_sel;
   reg [1:0] 						Bl_Mode	/* synthesis syn_useioff = 0 */;
   reg							Test_Mode /* synthesis syn_useioff = 0 */;
   reg							MaxCmd_Siz /* synthesis syn_useioff = 0 */;
   reg							CmdBurst_En /* synthesis syn_useioff = 0 */;
   reg [4:0] 						cmd_burst_cnt;	
   reg							clr_cmd_cnt;
   reg [4:0] 						sngl_cmd_cnt;
   
   
   wire [`DSIZE / 8-1:0] 				byte_ok;
   wire [`DATA_WIDTH / 8-1:0] 				dqs_ok;
   reg [`DSIZE / 8-1:0] 				data_mask;
   
   wire [`DSIZE-1:0] 					curr_data;
   wire 						ClkSel;
   wire 						Test_Mode_p;
   wire [1:0] 						Bl_Mode_p;
   wire 						Data_Mode;	
   wire [1:0] 						mr0_bl;
   wire [1:0] 						RankSel;
   wire [1:0] 						Odt;
   wire [4:0] 						CmdBrstCnt;
   wire [4:0] 						SnglCmdCnt;
   wire [`ADDR_WIDTH-1:0] 				curr_addr_wr;
   wire [`ADDR_WIDTH-1:0] 				curr_addr_rd;
   wire 						cmd_gone;
   wire 						Def_Set;
   // ==============================================================================
   // Mode regsiter programming
   // - This is demo purpose only, usual applications do not need reprogramming MRs
   // ==============================================================================
   wire [15:0] 						mr0_init		=	{ `MRS0_INIT };
   wire [15:0] 						mr1_init		=	{ `MRS1_INIT };
   wire [15:0] 						mr2_init		=	{ `MRS2_INIT };
   wire [15:0] 						mr3_init		=	{ `MRS3_INIT };
   wire [15:0] 						mr1_init_odt	=	{mr1_init[15:7],OdtSel[1],mr1_init[5:3],OdtSel[0],mr1_init[1:0]};

   wire [15:0] 						mr0_data		=	{mr0_init[15:9],1'b0,mr0_init[7:2],mr0_bl};	// User BL setting & disable DLL reset
   wire [15:0] 						mr1_data		=	Def_Set	?	mr1_init	:	mr1_init_odt ;
   wire [15:0] 						mr2_data		=	mr2_init;
   wire [15:0] 						mr3_data		=	mr3_init;
   
   wire 						MaxCmd_Siz_p;	 
   wire 						CmdBurst_En_p;
   wire 						wr_done;
   wire 						rd_done;
   
   assign	mr0_bl		= (Bl_Mode_p == 2'b11)	?	2'b00	:	// BL8 Fixed
				  (Bl_Mode_p == 2'b10)	?	2'b10	:	// BC4 Fixed
				  2'b01	;	// OTP BL8/BC4

   // ==============================================================================
   // Test configuration generation
   // ==============================================================================
   assign	Def_Set		= 1'b1     ;
   //	[1] :	Set to the default demo configuration
   //	[0]	:	Allows demo configuration change (RPT, OdtSel,RankSel)
   
   
   assign	Test_Mode_p	= 1'b1;
   //	[1]	:	Continuous Write-then-Read transactions
   //	[0]	:	Single R/W command with a PB input
   
   
   assign	ClkSel 		= 1'b1;
   //	[1]	:	Use the On-Board Oscillator (100MHz)
   //	[0]	:	Use an External Pulse Generator
   
   
   assign	MaxCmd_Siz_p	= 1'b0     ;
   //	[1]	:	Use maximum command size/length (=32)
   //	[0]	:	Use user command burst size (default = 2)
   
   
   assign	CmdBurst_En_p	= 1'b1     ;
   //	[1]	:	Enable command burst mode
   //			(32 burst when MaxCmd_Siz=1, User burst size when MaxCmd_Siz =0)
   //	[0]	:	Use single command mode
   //			(Manual command repetition is still supported by MaxCmd_Siz)

   
   assign	Bl_Mode_p	= 2'b10;
   //	Bl_Mode[1]: 1 = Fixed, 0 = OTP
   //	Bl_Mode[0]: 1 = BL8,   0 = BC4
   //
				//	[11]:	BL8 Fixed
   //	[10]:	BC4 Fixed
   //	[01]:	OTP BL8
   //	[00]:	OTP BC4

   
   // Jumper settings
   // Y: Installed, N: Not installed     
   
   assign	Odt		= {mr1_data[6],mr1_data[2]};
   //  DDR3 memory ODT setting
   //	[YY]:	40 ohms
   //	[YN]:	120 ohms
   //	[NY]:	60 ohms	(DDR3 core default)
   //	[NN]:	No Odt
   
   
   // ==============================================================================
   // state assignments & defines
   // ==============================================================================
`define		IDLE_ST			0
`define		START_GEN		1
`define		CONF_MR0		2
`define		CONF_MR1		3
`define		CONF_MR2		4
`define		CONF_MR3		5
`define		WRITE_CMD		6
`define		READ_CMD		7
`define		PB_WAIT_WR		8
`define		PB_WAIT_RD		9
`define         WAIT_ST                10
`define         WAIT4CMD_FRM_RD         11

//********************************************************************/
   // Init_start generation
   // - in simulation mode, wait for 128 clock cycles
   // - in synthesis mode, wait for +200us for memory reset requirement
//********************************************************************/
   always@(posedge sclk or negedge reset_n) begin
      if (reset_n == 1'b0) begin
	 init_cnt		<=	16'h0;
	 init_srvcd		<=	1'b0;
	 init_start_hit	<=	1'b0;
	 rst_srvcd		<=	1'b0;
	 init_start_hit_1	<=	1'b0;
      end
      else begin
	 init_cnt	<=	init_cnt + 1;
	 if (init_cnt[7] == 1'b1) begin						// takes the first hit
	    rst_srvcd	<=	1'b1;
	    if (!rst_srvcd)
	      init_start_hit	<=	1'b1;
	    else
	      init_start_hit	<=	1'b0;
	 end
	 
`ifdef RTL_SIM
	 if (init_cnt[6] && rst_srvcd == 1'b1)  	begin	// to save the simulation run time
`else
	    if (init_cnt[15] && init_cnt[13] && rst_srvcd == 1'b1)	begin
	       // takes the second hit after 200 us
`endif
	       init_srvcd	<=	1'b1;
	       if (!init_srvcd)
		 init_start_hit_1	<=	1'b1;
	       else
		 init_start_hit_1	<=	1'b0;
		end
	 end
      end
      
      
      always @(posedge sclk or negedge reset_n) begin
	 if (reset_n == 1'b0) begin
	    init_start		<=	1'b0;
	    mem_rst_n	<=	1'b0;
	 end
	 else begin
	    if (init_start_hit) begin
	       mem_rst_n	<=	1'b0;
		end
	    else if (init_start_hit_1) begin
	       mem_rst_n	<=	1'b1;
	       init_start		<=	1'b1;
	    end
	    else if (init_done) begin
	       init_start		<=	1'b0;
	    end
	 end
end
   
   

//********************************************************************/
   //
   //	Address and Command Generation
//
   //********************************************************************/
   
// ==============================================================================
   // Address generator
//  Note: Only 1,2,4,8,16 or 32 of cmd_burst_cnt allowed for non-stop runs
   // ==============================================================================
   assign	cmd_gone	=	cmd_rdy && cmd_valid;
   
assign cmd_gone_wr = datain_count[4] ? 1 : 0;
   //   assign cmd_gone_wr = cmd_rdy && (datain_count == 5'b10000);
   
   
   
   assign	wr_done	=	(cmdgen == `WRITE_CMD) && cmd_gone ;
   assign	rd_done	=	(cmdgen == `READ_CMD)  && cmd_gone;
   
   always @(posedge sclk or negedge reset_n) begin
      if (reset_n == 1'b0) begin
	 addr_rd			<=	{`ADDR_WIDTH{1'b0}};
	 addr_wr			<=	{`ADDR_WIDTH{1'b0}};
	 addr_interval	<=	0 ;
      end
      else if (wb_stb) begin //(clr_gen) begin
	 addr_rd			<=	wb_addr          ; //connect to wishbone
	 addr_wr			<=	wb_addr            ; //connect to wishbone
      end
      else begin
	 
	 
	 if (Bl_Mode[0]) begin
	    if (CmdBrstCnt == 5'b00000)
	      addr_interval	<=	256 ;
	    else
	      addr_interval	<=	CmdBrstCnt * 8 ;
	 end
	 else begin
	    if (CmdBrstCnt == 5'b00000)
	      addr_interval	<=	128 ;
	    else
	      addr_interval	<=	CmdBrstCnt * 4 ;
	 end
	 
      end
   end

   
   
   assign	curr_addr_wr	=	{wb_addr[`ADDR_WIDTH:2],2'd0};//addr_wr;
   assign	curr_addr_rd	=	{wb_addr[`ADDR_WIDTH:2],2'd0};//addr_rd;
   // ==============================================================================
   reg [4:0] datain_count;
   reg 	     wb_stb_dly;
   
   
   always @(posedge sclk or negedge reset_n)
     if (reset_n == 1'b0)
       begin 
	  datain_count<= 4'b0;
	  wb_stb_dly<=1'b0;
       end
     else
       begin
	   wb_stb_dly<=wb_stb;
       if (datain_rdy)
	 datain_count<= datain_count + 1'b1;
       else if (datain_count == 5'b10000 & wb_stb_dly)
	 datain_count<= 4'b0;
       end
      
// ==============================================================================
// Command generation state machine
// ==============================================================================
   always @(posedge sclk or negedge reset_n) begin
      if (reset_n == 1'b0) begin
	 cmdgen			<=	`IDLE_ST;
	 st_mach_idle            <=      1'b0;
	   
	 cmd			<=	4'h0;
	 cmd_burst_cnt	        <=	5'h0;
	 cmd_valid		<=	1'b0;
	 addr			<=	{`ADDR_WIDTH{1'b0}};
	 otf_bl_sel		<=	1'b0;	
	 Bl_Mode 		<=      2'b00;
	 Test_Mode		<=	1'b0;
	 MaxCmd_Siz		<=	1'b0;
	 CmdBurst_En		<=	1'b0;
	 clr_cmd_cnt		<=	1'b0;
	 
	 
      end
	else begin
	   
	   case (cmdgen)

	     `IDLE_ST : begin
		if (init_done) begin
		   cmdgen		<=	`START_GEN;
		   st_mach_idle            <=     1'b0;
		end
		else begin
		   cmdgen		<=	`IDLE_ST;
		   st_mach_idle            <=     1'b0;
		end
	     end				
	     
	     `START_GEN : begin
		cmd_valid	<=	1'b0;
		cmdgen		<=	`CONF_MR0;
		
	     end
	     
	     `CONF_MR0 : begin
		cmd			<=	4'h6;		// Load Mode Register
		cmd_valid	<=	1'b1;
		addr		<=	{{`ADDR_WIDTH-18{1'b0}},2'b00,mr0_data};
		if (cmd_gone) begin
		   cmdgen		<=	`CONF_MR1;
		   cmd_valid	<=	1'b0;
		end
		else
		  cmdgen		<=	`CONF_MR0;
	     end
	     
		`CONF_MR1 : begin
		   cmd_valid	<=	1'b1;
		   addr		<=	{{`ADDR_WIDTH-18{1'b0}},2'b01,mr1_data};
		   if (cmd_gone) begin
		      cmdgen		<=	`CONF_MR2;
		      #1	cmd_valid	<=	1'b0;
		   end
		   else
		     cmdgen		<=	`CONF_MR1;
		end
	     
	     `CONF_MR2 : begin
		cmd_valid	<=	1'b1;
		addr		<=	{{`ADDR_WIDTH-18{1'b0}},2'b10,mr2_data};
		if (cmd_gone) begin
		   cmdgen		<=	`CONF_MR3;
		   cmd_valid	<=	1'b0;
		end
		else
		  cmdgen		<=	`CONF_MR2;
	     end
	     
	     `CONF_MR3 : begin
		cmd_valid	<=	1'b1;									 
		st_mach_idle    <=      1'b1;
		addr		<=	{{`ADDR_WIDTH-18{1'b0}},2'b11,mr3_data};
		if (cmd_gone) begin
		   cmd_valid	<=	1'b0;
		   
		   Bl_Mode		<=	Bl_Mode_p; 				// Sample BL_Mode[0] for OTF
		   Test_Mode	<=	Test_Mode_p;
		   MaxCmd_Siz	<=	MaxCmd_Siz_p;
		   CmdBurst_En	<=	CmdBurst_En_p;
		   
		   if (wb_stb) begin
		      
		      if (wb_we)			     
			cmdgen		<=	`WRITE_CMD;
		      else
			cmdgen		<=	`READ_CMD;			      
		   end
		   else
		     
		     begin
			
			cmdgen		<=	`CONF_MR3;
			  end
		end
		else
		  
		  cmdgen		<=	`CONF_MR3;
		
	     end // case: `CONF_MR3
	     	     
	     		  
	     `WRITE_CMD : 
	       begin
		  cmd		<=	4'h2; 						// Switch to write
		  addr		<=	curr_addr_wr;
		  cmd_burst_cnt   <=      CmdBrstCnt;
		  otf_bl_sel	<=	Bl_Mode[0]; 				// Set OTF burst lengh
		  cmd_valid	<=	1'b1;
		  clr_cmd_cnt	<=	1'b0;
		  if (cmd_gone)
		    begin				// cmd burst mode
		       cmdgen		<=	`WAIT_ST;
		       cmd_valid	<=	1'b0;
		       st_mach_idle    <=      1'b0;
		    end
		  else 
		    begin 
		       cmdgen		<=	`WRITE_CMD;
		       st_mach_idle    <=      1'b0;
		    end 
		  
	       end
	     
	     `WAIT_ST:
	       if (cmd_gone_wr)
		 begin
		    st_mach_idle    <=      1'b1;
    		    if (wb_stb) begin
		       
		       if (wb_we)
			 begin
			    cmd		<=	4'h2; 	
			    cmd_valid	<=	1'b1;
			    addr		<=	curr_addr_wr;				  
			    cmdgen		<=	`WRITE_CMD;
			 end
		       else
			 begin
			    cmd             <= 4'h1;
   		            cmd_valid	<=	1'b1;
			    addr		<=	curr_addr_rd;
			    cmdgen		<=	`READ_CMD;
			    st_mach_idle <=      1'b0;
			    
			   end
		    end
		    else
		      cmdgen		<=	`WAIT_ST;
			 
		 end 
	       else
		      cmdgen		<=	`WAIT_ST;
	     
		
	     
	     
		  `WAIT4CMD_FRM_RD:
		    begin
		       if (cmd_gone)
			 begin
			    st_mach_idle    <=      1'b1;
			 end
    		       if (wb_stb)
			 begin
			    if (wb_we)
			      begin
				 cmd		<=	4'h2; 	
				 cmd_valid	<=	1'b1;
				 addr		<=	curr_addr_wr;			     
				 cmdgen		<=	`WRITE_CMD;
			      end
			    else
			      begin
				 cmd             <= 4'h1;
   				 addr		<=	curr_addr_rd;
				 cmd_valid	<=	1'b1;
				 cmdgen		<=	`READ_CMD;
				 st_mach_idle <=      1'b0;
			      end // else: !if(wb_we)
			    
			 end // if (wb_stb)
		       else
			 cmdgen		<= `WAIT4CMD_FRM_RD;
		    end	  
	     
	     `READ_CMD : 			//7
		    begin					
		       cmd			<=	4'h1; 						// Switch to read
		       addr		<=	curr_addr_rd;
		       cmd_burst_cnt   <=    CmdBrstCnt;//  5'b10000;
		       cmd_valid	<=	1'b1;
		       clr_cmd_cnt	<=	1'b0;
		       if (cmd_gone)
			 
			 begin
			    cmdgen		<=	`WAIT4CMD_FRM_RD;//WAIT_ST
	// go to A
			    cmd_valid	<=	1'b0;
			    Test_Mode	<=	Test_Mode_p;
			    st_mach_idle    <=      1'b1;
			    
			 end// prepare command en/disable control for manual mode
		       else 
			 begin
			    cmdgen		<=	`READ_CMD;
			    st_mach_idle    <=      1'b0;
			 end 
		    end
		  
	     
	   endcase
	end
   end
   

   always @(posedge sclk or negedge reset_n) begin
      if (reset_n == 1'b0) begin
	 sngl_cmd_cnt		<=	5'h0;
      end
	else begin
	   if (clr_cmd_cnt)
	     sngl_cmd_cnt	<=	5'h0;
	   else
	     if ((cmdgen == `READ_CMD || cmdgen == `WRITE_CMD) && cmd_gone)
	       sngl_cmd_cnt		<=	sngl_cmd_cnt + 1;
	end
   end
   
   
   // ==============================================================================
   // Write data insertion timing
   //  - with WrRqDDelay_1 defined: 1 clock delay. Otherwise, two clocks.
   // ==============================================================================
   
   always @(posedge sclk or negedge reset_n) begin
      if (reset_n == 1'b0) begin
	 write_data		<=	{`DSIZE{1'b0}};
	 clr_gen			<=	1'b0;
         data_mask               <=      0;
`ifdef WrRqDDelay_2
	 datain_rdy_d	<=	1'b0;
`endif
      end
      else begin
`ifdef WrRqDDelay_2
	 datain_rdy_d	<=	datain_rdy;
	 if (datain_rdy_d) begin
	    write_data		<=     wb_data_wr   ;
	    //        data_mask               <=     ~wb_sel;
	 end
	 else begin
	    write_data		<=	{`DSIZE{1'b0}};
	    //            data_mask               <=      2'b11;
	 end
`else 	//WrRqDDelay_1
	 if (datain_rdy) begin
	    write_data		<=	wb_data_wr  ;
	    //                data_mask               <=      ~wb_sel;
	 end
	 else begin
	    write_data		<=	{`DSIZE{1'b0}};
	    //	                data_mask               <=      2'b11;
	 end
`endif	
	 
	 if (init_done)
	   clr_gen			<=	1'b1;
	 else
	   clr_gen			<=	1'b0;
      end
   end
   
   
   
   
   
   assign	CmdBrstCnt	= 5'b10000;
   
   assign	SnglCmdCnt	=	MaxCmd_Siz	? 5'h1f		: `UsrCmdBrstCnt-1;
   
   
endmodule

