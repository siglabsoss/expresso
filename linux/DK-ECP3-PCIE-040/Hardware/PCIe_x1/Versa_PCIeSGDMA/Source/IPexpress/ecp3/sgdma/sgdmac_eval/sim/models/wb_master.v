// wb_master -- wishbone master testbench model

module wb_master(clk, 
                 rst,
                 adr, 
                 din, 
                 dout, 
                 cyc, 
                 stb, 
                 sel, 
                 we, 
                 ack, 
                 eod, 
                 err, 
                 rty,
                 ctl_req, 
                 ctl_id, 
                 ctl_op, 
                 ctl_addr, 
                 ctl_wdat, 
                 ctl_mask,
                 ctl_ack, 
                 ctl_rdat, 
                 ctl_rtn);

   parameter DWIDTH = 32;
   parameter AWIDTH = 32;
   parameter MEMSIZE = 4096;
   parameter CTLID = 0;
   
   localparam NUM_SEL = DWIDTH/8;
   
   input clk;
   input rst;
   
   // WISHBONE
   output reg [AWIDTH-1:0]  adr;
   input      [DWIDTH-1:0]  din;
   output reg [DWIDTH-1:0]  dout;
   output reg               cyc;
   output reg               stb;
   output reg [NUM_SEL-1:0] sel;
   output reg               we;
   input                    ack;
   input                    eod;
   input                    err;
   input                    rty;
   
   // CTL
   input                    ctl_req;
   input  [7:0]             ctl_id;
   input  [7:0]             ctl_op;
   input  [31:0]            ctl_addr;
   input  [31:0]            ctl_wdat;
   input  [31:0]            ctl_mask;
   output reg               ctl_ack;
   output reg [31:0]        ctl_rdat;
   output reg [7:0]         ctl_rtn;
   
   // program execution
   reg	[31:0]	mem[0:MEMSIZE-1];// internal memory
   integer      tcnt;            // transfer count
   integer      twdth;	         // transfer width
   reg          bus_op;	         // do a bus operation

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

	// initialization
   initial begin
     ctl_ack = 0;
     ctl_rdat = 0;
     ctl_rtn = 0;
     bus_op = 0;
   end

   always@* begin
      ctl_ack = 0;
      ctl_rdat = 0;
      ctl_rtn = 0;
      if( ctl_req && (ctl_id == CTLID) ) begin
         wait( !ack );
         case ( ctl_op )
             WBMAST_MEM_RD: begin
                ctl_ack = 1;
                ctl_rdat = mem[ctl_addr];
                ctl_rtn = 0;
                $display("READ: wb_master %d mem[%x] = %x", CTLID, ctl_addr, ctl_rdat );
             end
             WBMAST_MEM_WR: begin
                ctl_ack = 1;
                mem[ctl_addr] = ctl_wdat;
                ctl_rtn = 0;
             end
             WBMAST_MEM_VF: begin
                ctl_ack = 1;
                ctl_rdat = mem[ctl_addr];
                ctl_rtn = 0;
                if( (ctl_rdat & ctl_mask) === (ctl_wdat & ctl_mask) )
                   $display("VERIFY PASSED: wb_master %d mem[%x] = %x", CTLID, ctl_addr, ctl_rdat );
                else 
                   $display("VERIFY FAILED: wb_master %d mem[%x] = %x, expected %x", CTLID, ctl_addr, ctl_rdat, ctl_wdat );
             end
             WBMAST_BUS_WR, WBMAST_BUS_RD, WBMAST_BUS_VF: begin
                bus_op = 1;
                wait( !bus_op );
                ctl_ack = 1;
                ctl_rdat = din;
                ctl_rtn = 0;
                if(ctl_op == WBMAST_BUS_RD)
                   $display("BUS_READ: wb_master %d at %x = %x", CTLID, ctl_addr, ctl_rdat );
                if(ctl_op == WBMAST_BUS_VF) begin
                   if( (ctl_rdat & ctl_mask) === (ctl_wdat & ctl_mask) )
                      $display("VERIFY PASSED: wb_master %d address %x = %x", CTLID, ctl_addr, ctl_rdat );
                   else 
                      $display("VERIFY FAILED: wb_master %d address %x = %x, expected %x", CTLID, ctl_addr, ctl_rdat, ctl_wdat );
                end
             end
         endcase
      end
   end

   always@( posedge clk or posedge rst ) begin
      if( rst ) begin
         cyc  <= 0;
         stb  <= 0;
         adr  <= 0;
         dout <= 0;
         sel  <= 0;
         we   <= 0;
      end
      else begin
        if( bus_op ) begin
           case ( ctl_op )
             WBMAST_BUS_WR, WBMAST_BUS_RD, WBMAST_BUS_VF: begin
               cyc  <= 1;
               stb  <= 1;
               adr  <= ctl_addr;
               dout <= ctl_wdat;
               sel  <= ctl_mask;
               we   <= (ctl_op == WBMAST_BUS_WR);
               if( ack ) begin
                  bus_op <= 0;
                  cyc    <= 0;
                  stb    <= 0;
                  adr    <= 0;
                  dout   <= 0;
                  sel    <= 0;
                  we     <= 0;
               end
             end
           endcase
        end
      end
   end

endmodule
