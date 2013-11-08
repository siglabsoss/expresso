/*----------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------
--                          COPYRIGHT NOTICE
--Copyright 2003(c) Lattice Semiconductor Corporation
--ALL RIGHTS RESERVED
--This confidential and proprietary software may be used only as authorised by
--a licensing agreement from Lattice Semiconductor Corporation.
--The entire notice above must be reproduced on all authorized copies and
--copies may only be made to the extent permitted by a licensing agreement from
--Lattice Semiconductor Corporation.
--
--Lattice Semiconductor Corporation    TEL : 1-800-Lattice (USA and Canada)
--5555 NE Moore Court                        408-826-6000 (other locations)
--Hillsboro, OR 97124                  web  : http:--www.latticesemi.com/
--U.S.A                                email: techsupport@latticesemi.com
--=============================================================================
--                        FILE DETAILS
--Project      : PCIe Wishbone Slave for DMA burst access
--File         : wbs_64kebr_ddr3.vhd
--Title        : wbs_ebr_ddr3
--Code type    : Register Transfer Level
--Dependencies :
--Description  : This is a bridge between Wishbone and DDR3 controller.
--=============================================================================
--                       REVISION HISTORY
--Version      : 1.0
--Author(s)    : Abhinav Rathore
--Mod. Date    : 4-12-2011
--Changes Made : Initial Creation

-------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------*/
`include "ddr3_test_params.v"

//`define ddr3
module wbs_64kebr_ddr3
  #(parameter init_file = "none")
   (
    wb_clk_i
    ,wb_rst_i
    ,filter  
    ,wb_dat_i
    ,wb_adr_i
    ,wb_cyc_i
    ,wb_cti_i
    ,wb_sel_i
    ,wb_stb_i
    ,wb_we_i
    ,wb_dat_o
    ,wb_ack_o
    ,wb_err_o
    ,wb_rty_o
    
`ifdef ddr3    
    ,clk2ddrcntrl,
    em_ddr_data,
    em_ddr_dqs,
    em_ddr_dm,
    em_ddr_clk,
    em_ddr_cke,
    em_ddr_cs_n,
    em_ddr_odt,
    em_ddr_reset_n,
    em_ddr_ras_n,
    em_ddr_cas_n,
    em_ddr_we_n,
    em_ddr_ba,
    em_ddr_addr,    
    
`endif
    
    
    );
   
   
   input         wb_clk_i;
`ifdef ddr3
   input         clk2ddrcntrl;
`endif
   
   input         wb_rst_i;
   input [31:0]  filter;
   input [63:0]  wb_dat_i; // Data Input
   input [31:0]  wb_adr_i; // Address Input
   input         wb_cyc_i; // Bus Enable Signal
   input [ 2:0]  wb_cti_i; // Indicates a single, burst, or end transaction
   input [ 7:0]  wb_sel_i; // Byte Enables
   input         wb_stb_i; // Data Is Valid
   input         wb_we_i;  // Write Enable
   
   
   
   
   output [63:0] wb_dat_o; // Output Data
   reg [63:0] 	 wb_dat_o;  
   output        wb_ack_o; // Validate current bus cycle
   reg           wb_ack_o;
   output        wb_err_o; // Errored Cycle
   output        wb_rty_o; // Retry Cycle
   
`ifdef ddr3   
   inout [`DATA_WIDTH-1:0] em_ddr_data;
   inout [`DATA_WIDTH / 8-1:0] em_ddr_dqs;
   output [`DATA_WIDTH/8-1:0]  em_ddr_dm;
   output [`CS_WIDTH-1:0]      em_ddr_clk;
   output [`CS_WIDTH-1:0]      em_ddr_cke;
   output 		       em_ddr_ras_n;
   output 		       em_ddr_cas_n;
   output 		       em_ddr_we_n;
   output [`CS_WIDTH-1:0]      em_ddr_cs_n;
   output [`CS_WIDTH -1 :0]    em_ddr_odt;
   output [`ROW_WIDTH-1:0]     em_ddr_addr;
   output [`BANK_WIDTH-1:0]    em_ddr_ba;
   output 		       em_ddr_reset_n;
   
`endif //  `ifdef ddr3
   
   assign wb_err_o = 1'b0;
   assign wb_rty_o = 1'b0;
   
   
`ifdef ddr3
   
   reg [32:0] 		       addr_reg;
   
   reg [2:0] 		       wb_status;
   parameter WB_IDLE   = 3'b000;
   parameter WB_WRITE  = 3'b001;
   parameter WB_REWAIT = 3'b010;
   parameter WB_REWAIT2 = 3'b011;   
   parameter WB_READ   = 3'b100;
   parameter WB_PRE_IDLE   = 3'b101;
   
   wire 		       datain_rdy;
   wire 		       st_mach_idle;
   wire 		       read_data_valid;
   wire [63:0] 		       read_data;
   wire 		       ddr_clk;
   
   reg [63:0] 		       S_DAT_I_ddr_clk;
   reg [31:0] 		       S_ADR_I_ddr_clk;
   reg [1:0] 		       S_SEL_I_ddr_clk;
   reg 			       S_WE_I_ddr_clk;
   reg 			       S_STB_I_ddr_clk;
   
   reg [98:0] 		       down_data;
   reg 			       down_we;
   reg 			       down_re;
   wire [98:0] 		       down_q;
   wire 		       down_empty;
   wire 		       down_full;
   
   reg [63:0] 		       up_data;
   reg 			       up_we;
   reg 			       up_re;
   wire [63:0] 		       up_q;
   wire 		       up_empty;
   wire 		       up_full;
   wire 		       up_re_combi;
   
   wire 		       rst2ddrcntrl;
   
   reg [2:0] 		       wb_cyc_i_d;
   reg 			       clk_en;
   
   
   reg [63:0] 		       wb_dat_o_t;
   reg [3:0] 		       down_we_count;
   reg 			       burst_128;
   reg [2:0] 		       burst_128_count;
   
   
   fifo_down downstream
     (.Data    (down_data),
      .WrClock (wb_clk_i),
      .RdClock (ddr_clk),
      .WrEn    (down_we),
      .RdEn    (down_re_datain_rdy),
      .Reset   (wb_rst_i),
      .RPReset (wb_rst_i),
      .Q       (down_q), 
      .Empty   (down_empty),
      .Full    (down_full));
   
   fifo_up upstream
     (.Data    (up_data),
      .WrClock (ddr_clk),
      .RdClock (wb_clk_i),
      .WrEn    (up_we),
      .RdEn    (up_re_combi),
      .Reset   (wb_rst_i),
      .RPReset (wb_rst_i),
      .Q       (up_q), 
      .Empty   (up_empty),
      .Full    (up_full));
   
   
   ////////////////////////////////////////////////////////////////////////////////
   ddr3_test_top ddr3_test1 (
			     .clk_in         (clk2ddrcntrl),
			     .reset_n        (rst2ddrcntrl),
			     
			     .wb_data_wr     (S_DAT_I_ddr_clk), //from wb
			     .wb_addr        (S_ADR_I_ddr_clk),//frm wb
			     .wb_sel         (S_SEL_I_ddr_clk),
			     .wb_stb         (S_STB_I_ddr_clk),	 
			     .wb_we          (S_WE_I_ddr_clk), 
			     .wb_data_rd     (read_data),
			     .read_data_valid_out	(read_data_valid),
			     .sclk           (ddr_clk),
			     
			     
			     // output to external memory
			     .em_ddr_data    (em_ddr_data),
			     .em_ddr_dqs     (em_ddr_dqs),
			     .em_ddr_dm      (em_ddr_dm),
			     .em_ddr_clk     (em_ddr_clk),
			     .em_ddr_cke     (em_ddr_cke),
			     .em_ddr_cs_n    (em_ddr_cs_n),
			     .em_ddr_odt     (em_ddr_odt),
			     .em_ddr_reset_n (em_ddr_reset_n),
			     .em_ddr_ras_n   (em_ddr_ras_n),
			     .em_ddr_cas_n   (em_ddr_cas_n),
			     .em_ddr_we_n    (em_ddr_we_n),
			     .em_ddr_ba      (em_ddr_ba),
			     .em_ddr_addr    (em_ddr_addr),    
			     .ClkSel         (),
			     .datain_rdy_wb  (datain_rdy),		     
			     .st_mach_idle   (st_mach_idle),
			     .datain_count (datain_count)	      
			     )/* synthesis syn_dspstyle=LOGIC */ /* synthesis syn_useioff = 0 */;
   
   assign    rst2ddrcntrl = ~wb_rst_i ;
   wire [4:0] 		       datain_count;
   
   /////////////////////////////////
   // wb2ddr fifo ddr interface
   /////////////////////////////////
   reg 			       down_re_dly;
   wire 		       down_re_datain_rdy;
   wire 		       down_re_datain;
   
   reg 			       st_mach_idle_dly;
   reg 			       st_mach_idle_dly2;
   
   reg 			       down_re_datain_rdy_dly;
   
   assign  down_re_datain = down_re | datain_rdy;
   
   assign  down_re_datain_rdy = &datain_count[3:0] ? 1'b0:down_re_datain ;
   
   always @(posedge wb_rst_i or posedge ddr_clk)
     if(wb_rst_i) begin
	
	down_re_dly <= 1'b0;
	S_DAT_I_ddr_clk <= 64'h0;
	S_ADR_I_ddr_clk <= 32'h0;
	S_SEL_I_ddr_clk <= 4'h0;
	S_WE_I_ddr_clk  <= 1'b0;
	S_STB_I_ddr_clk <= 1'b0;
	down_re_datain_rdy_dly <=1'b0;

	st_mach_idle_dly<= 1'b0;
	st_mach_idle_dly2<= 1'b0;
     end
   
     else begin
	st_mach_idle_dly<=st_mach_idle;
	st_mach_idle_dly2<= st_mach_idle_dly;
	down_re_dly <= down_re;
	down_re_datain_rdy_dly<=down_re_datain_rdy;
  	
	if (down_re_datain_rdy_dly) begin

	   S_DAT_I_ddr_clk <= down_q[63:0];
	   S_ADR_I_ddr_clk <= down_q[95:64];
	   S_SEL_I_ddr_clk <= down_q[97:96];
	   S_WE_I_ddr_clk  <= down_q[98];
	end
	if (|burst_128_count  && st_mach_idle_dly2 )// && !down_empty)
	  S_STB_I_ddr_clk <= 1'b1;
	else if (st_mach_idle && down_re_dly  && (wb_status==3'b010) )
	  S_STB_I_ddr_clk <= 1'b1;
	else if (!st_mach_idle)
	  S_STB_I_ddr_clk <= 1'b0;
     end
    //===================================================================================

   reg  down_status;
   parameter DOWN_IDLE   = 1'b0;
   parameter DOWN_READ  = 1'b1;
  
   
   always @(posedge wb_rst_i or posedge ddr_clk)
     if(wb_rst_i) 
       begin
	  down_re     <= 1'b0;

	  down_status<= DOWN_IDLE;
	  
       end
     else
       begin
	  case(down_status)
	    DOWN_IDLE:
	      if  (!down_empty  && st_mach_idle )//&& !datain_count_512[2])// && !wb_status[1])
		begin
		   down_re <= 1'b1;
		   down_status <= DOWN_READ;
		end
	      else
		begin
		   down_re <= 1'b0;
		   down_status <= DOWN_IDLE;
		end
	    
	    DOWN_READ:
	      begin
	       down_re <= 1'b0;
	      if (datain_count==5'b01111 || wb_status[2] )// || wb_status[1])
		begin
		   down_status <= DOWN_IDLE;
		end  
	      else
		begin
		   down_status <= DOWN_READ;
		end
	      end // case: DOWN_READ
	    
	    default: down_status <= DOWN_IDLE;
	  endcase // case (down_status)
       end // else: !if(wb_rst_i	  
   
   //===================================================================================

      
   // ddr2wb fifo ddr interface
   always @(posedge wb_rst_i or posedge ddr_clk)
     if(wb_rst_i) begin
	up_we   <= 1'b0;
	up_data <= 64'h0;
     end
     else if (!up_full && read_data_valid) begin
	up_we   <= 1'b1;
	up_data <= read_data;
     end 
     else begin
	up_we   <= 1'b0;
	up_data <= 64'h0;
     end
 

   //=======================================================================================
   reg wb_ack_o_t;
   reg [6:0] stop_write;
   
   assign up_re_combi = (wb_status == 3'b100) ? wb_stb_i : 1'b0;// up_re;
   
   always @(posedge wb_rst_i or posedge wb_clk_i)
     
     if(wb_rst_i) begin
	wb_status   <= WB_IDLE;
	down_we     <= 1'b0;
	//  up_re       <= 1'b0;
	down_data   <= 99'h0;
	wb_ack_o_t    <= 1'b0;
	wb_dat_o_t    <= 64'h0;
	
	stop_write<=7'b0;
	
	
     end 
     else
       begin
	  
	  case(wb_status)
	    
	    WB_IDLE:
	      if (wb_cyc_i && wb_stb_i && wb_we_i && !down_full &&(burst_128_count==3'b0))
		begin
                   down_we   <= 1'b0;
                   down_data <= {wb_we_i,
				 wb_sel_i[1:0],
				 wb_adr_i,
				 //wb_adr_i,wb_adr_i};
                        wb_dat_i};
		   wb_ack_o_t     <= 1'b1;
		   wb_status   <= WB_WRITE;		
		   
		end
	    
	    
	      else if (wb_cyc_i && wb_stb_i && !wb_we_i && !down_full && (burst_128_count==3'b0))
		begin
		   down_we   <= 1'b1;
		   wb_ack_o_t  <= 1'b0;
		   down_data <= {wb_we_i,
				 wb_sel_i[1:0],
				 wb_adr_i,
				 64'h0};
		   wb_status <= WB_REWAIT;
		end
	      else
		begin
		   wb_status   <= WB_IDLE;
		   down_we     <= 1'b0;
		   //up_re       <= 1'b0;
		   down_data   <= 99'h0;
		   wb_ack_o_t    <= 1'b0;
		   
		   stop_write<=7'b0;
		
		end 	 
	    
	    WB_WRITE:
	  
	      if (wb_cyc_i && wb_stb_i && wb_we_i && !down_full )//&& !(&stop_write[4:0]))
		begin
           	   
		   if (stop_write==7'b1000000)//test 12
		     begin
			stop_write<=7'b0;
			wb_status   <= WB_IDLE;
			down_we     <= 1'b0;
			//up_re       <= 1'b0;
			down_data   <= 99'h0;
			wb_ack_o_t    <= 1'b0; 
			
		     end
		   
		   else
		     begin
			stop_write<=stop_write + 1'b1;
			wb_status   <= WB_WRITE;
			down_we   <= 1'b1;
			down_data <= {wb_we_i,
				      wb_sel_i[1:0],
				      wb_adr_i,//wb_adr_i,wb_adr_i};//to test
				 wb_dat_i};
			wb_ack_o_t     <= 1'b1;
		     end
		end
	    
	      else 
		begin
		   wb_status   <= WB_IDLE;
		   down_we     <= 1'b0;
		   //up_re       <= 1'b0;
		   down_data   <= 99'h0;
		   wb_ack_o_t    <= 1'b0;
		   
		   
		   stop_write<=7'b0;
		   
		end 
	    
	    
	    
	    WB_REWAIT:
	      begin
		 
		 down_we   <= 1'b0;
		 wb_ack_o_t  <= 1'b0;
		 if (up_full && wb_stb_i) begin
		    //up_re  <= 1'b1;
		    wb_status  <= WB_REWAIT2;
		 end
		 
	      end
	    
	    WB_REWAIT2:
	      begin
		 if (wb_stb_i)begin
		    wb_status  <= WB_READ;
		    
		 end
	      end
	    
	    WB_READ: 
	      begin
		 if (wb_stb_i && !up_empty)
		   begin
		      wb_ack_o_t    <= 1'b1;
  		      wb_dat_o_t  <= up_q;
		      wb_status   <= WB_READ;
		      //  up_re  <= wb_stb_i;//to emulate SGDMA
		   end
		 else if (up_empty)
		   begin
		      // up_re  <= 1'b0;
		      wb_ack_o_t    <= 1'b1;//test--1									
		      wb_dat_o_t  <= up_q;
		      wb_status   <= WB_PRE_IDLE;	 
		   end
		 
	   	 else
		   begin
		      wb_ack_o_t    <= 1'b0;//test--0
		      wb_dat_o_t  <= up_q ;
		   end
		 
		 
	      end // case: WB_READ
	    
	    WB_PRE_IDLE:
	      begin
		 wb_ack_o_t    <= 1'b0;
		 wb_status   <= WB_IDLE;
		 
	      end
	    default: wb_status <= WB_IDLE;
	  endcase
       end
   
   

////////////////////////////////////////////////////////

   
   always @(posedge wb_rst_i or posedge wb_clk_i)
     
     if(wb_rst_i) begin
	down_we_count   <= 4'b0;
	burst_128 <= 1'b0;
	burst_128_count <= 3'b0;
	
     end
     else
       begin
	  
	  if (down_we)
	    begin
	       down_we_count<= down_we_count + 1'b1;
	       if (down_we_count == 4'b1111)
		 begin
		    if (!burst_128_count[2])
		      begin
			 burst_128_count<= burst_128_count + 1'b1;
			 burst_128 <= 1'b1;
		      end
		    
		    else if (down_empty)
		      begin
			 burst_128_count <= 3'b0;
			 burst_128 <= 1'b0;
		      end
		    else 
		      burst_128 <= 1'b0; 
		 end 
     	       else
		 burst_128 <= 1'b0;
	       
	    end 
	  else if (down_empty)
	    begin
	       down_we_count   <= 4'b0;
	       burst_128_count <= 3'b0;
	       burst_128 <= 1'b0;
	    end
       end // 
   
   
   ////////////////////////////////////////////////////    
   always @(wb_dat_o_t)
     
     wb_dat_o <= wb_dat_o_t ^ {filter, filter};//test
   
   always @ (wb_ack_o_t)
     wb_ack_o<=wb_ack_o_t;
   
   
   /////////////////////////////////////////////////////////////////////////////////// 
   ///////////////////////////////////////////////////////////////////////////////////
   //// `else part is for EBR usage instead of DDR3. This is the default configuration
   //// of this demo
   /////////////////////////////////////////////////////////////////////////////////// 
   /////////////////////////////////////////////////////////////////////////////////// 
`else // !`ifdef ddr3
   
   reg [2:0] wb_cyc_i_d;
   reg [15:0] addr;
   reg [7:0]  byte_en;
   reg [63:0] wdata;
   reg 	      wr_en;
   reg 	      clk_en;
   wire [63:0] wb_dat_o_t;
   always @(posedge wb_clk_i or posedge wb_rst_i)
     if (wb_rst_i) begin
	wb_cyc_i_d <= 0;
	addr <= 0;
	wdata <= 0;
	byte_en <= 0;
	wr_en <= 0;
	clk_en <= 0;
     end
     else begin
	wb_cyc_i_d <= {wb_cyc_i_d[1:0], wb_cyc_i};
	
	if (wb_cyc_i && wb_stb_i && (wb_cti_i == 3'b111))
          wb_ack_o <= 1'b0;
	else if (wb_cyc_i && wb_cyc_i_d[2])
          wb_ack_o <= wb_stb_i;
	else
          wb_ack_o <= 1'b0;
        
        
	if (wb_we_i) begin
           addr  <= wb_adr_i[15:0];
           byte_en <= wb_sel_i;
           wdata <= wb_dat_i;
           wr_en <= wb_cyc_i & wb_ack_o;
	end
	else begin
           byte_en <= 8'hff;
           wr_en   <= 0;
           if (wb_cyc_i && (~ wb_cyc_i_d[0])) begin
              addr  <= wb_adr_i[15:0];
           end
           else if (clk_en)
             addr <= addr + 8;   
	end
	
	if (wb_cyc_i) begin
           if (wb_we_i) //for write
             clk_en <= 1'b1;
           else begin //for read
              if (~ wb_cyc_i_d[2]) //prefech data
		clk_en <= 1'b1;
              else if (wb_stb_i)//get following data
		clk_en <= 1'b1; //wb_stb_i; 
              else
		clk_en <= 1'b0;
           end
	end
	else
          clk_en <= 1'b0;
     end
   
   spram_8192_64 I_spram_8192_64
     (
      .Clock(wb_clk_i), 
      .ClockEn(clk_en), 
      .Reset(1'b0), 
      .ByteEn(byte_en), 
      .WE(wr_en), 
      .Address(addr[15:3]), 
      .Data(wdata), 
      .Q(wb_dat_o_t)
      );
   
   always @(posedge wb_clk_i or posedge wb_rst_i)
     if (wb_rst_i)
       wb_dat_o <= 0;
     else begin
	if (clk_en)
          wb_dat_o <= wb_dat_o_t ^ {filter, filter};
     end
   
   
`endif // !`ifdef ddr3
   
   
endmodule