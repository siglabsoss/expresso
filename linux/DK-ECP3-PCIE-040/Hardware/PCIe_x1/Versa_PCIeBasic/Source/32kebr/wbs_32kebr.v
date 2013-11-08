/*-----------------------------------------------------------------------------------
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
--File         : wbs_32kebr.vhd
--Title        : wbs_32kebr
--Code type    : Register Transfer Level
--Dependencies :
--Description  : This is burst capable 64x4K RAM with wishbone slave interface.
--=============================================================================
--                       REVISION HISTORY
--Version      : 1.0
--Author(s)    : Jaime Freed
--Mod. Date    : 5-29-2007
--Changes Made : Initial Creation
--
--Version      : 2.0
--Author(s)    : Gary Landes
--Mod. Date    : 6-10-2007
--Changes Made : Updated to 64-bit data size and added burst input capability
-------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------*/

module wbs_32kebr
   #(parameter init_file = "none")
   (
   wb_clk_i
  ,wb_rst_i
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
);


input         wb_clk_i;
input         wb_rst_i;

input  [15:0] wb_dat_i; // Data Input
input  [31:0] wb_adr_i; // Address Input
input         wb_cyc_i; // Bus Enable Signal
input  [ 2:0] wb_cti_i; // Indicates a single, burst, or end transaction
input  [ 1:0] wb_sel_i; // Byte Enables
input         wb_stb_i; // Data Is Valid
input         wb_we_i;  // Write Enable

output [15:0] wb_dat_o; // Output Data
output        wb_ack_o; // Validate current bus cycle
reg           wb_ack_o;
output        wb_err_o; // Errored Cycle
output        wb_rty_o; // Retry Cycle

// Synchronous Signals


// assign wb_ack_local = (ebr_wr || ebr_wr_p) ? wb_stb_i : wb_stb_dly; // pipe ack if read since it takes a clock to get data from ebr
assign wb_err_o = 1'b0;
assign wb_rty_o = 1'b0;

reg [2:0] wb_cyc_i_d;
reg [14:0] addr;
reg [1:0] byte_en;
reg [15:0] wdata;
reg wr_en;
reg rd_en;
always @(posedge wb_clk_i or posedge wb_rst_i)
   if (wb_rst_i) begin
      wb_cyc_i_d <= 0;
      addr <= 0;
      wdata <= 0;
      byte_en <= 0;
      wr_en <= 0;
      rd_en <= 0;
   end
   else begin
      wb_cyc_i_d <= {wb_cyc_i_d[1:0], wb_cyc_i};
      
      if (wb_cyc_i_d[1] && (~ wb_cyc_i_d[2]))
         wb_ack_o <= 1'b1;
      else if (wb_cyc_i && wb_stb_i && (wb_cti_i == 3'b111))
         wb_ack_o <= 1'b0;
      else if (~ wb_cyc_i)
         wb_ack_o <= 1'b0;
         
         
      if (wb_we_i) begin
         addr  <= wb_adr_i[14:0];
         byte_en <= wb_sel_i;
         wdata <= wb_dat_i;
         wr_en <= wb_cyc_i & wb_stb_i & wb_ack_o;
      end
      else begin
         byte_en <= 2'b11;
         wr_en   <= 0;
         if (wb_cyc_i && (~ wb_cyc_i_d[0])) begin
            addr  <= wb_adr_i[14:0];
            rd_en <= 1'b1;
         end
         else if (~ wb_cyc_i)
            rd_en <= 1'b0;
         else if (rd_en)
            addr <= addr + 2;   
      end
   end

spram_16384_16 I_spram_16384_16 (
.Clock(wb_clk_i), 
.ClockEn(1'b1), 
.Reset(1'b0), 
.ByteEn(byte_en), 
.WE(wr_en), 
.Address(addr[14:1]), 
.Data(wdata), 
.Q(wb_dat_o)
);



endmodule