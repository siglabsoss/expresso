

module wb_slave(clk, 
                rst, 
                adr, 
                din, 
                dout, 
                cyc, 
                stb, 
                sel, 
                cti, 
                we, 
                ack, 
                err, 
                rty, 
                eod,
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
   parameter FULL_ADDR_SIZE = 0;
   parameter FULL_ADDR = 0;
   parameter MEMSIZE = 4096;
   parameter CTLID = 0;
   parameter BIGENDIAN = 0;
   
   localparam ASIZE = (MEMSIZE>32768)? 16 :
                      (MEMSIZE>16384)? 15 :
                      (MEMSIZE>8192) ? 14 :
                      (MEMSIZE>4098) ? 13 :
                      (MEMSIZE>2048) ? 12 :
                      (MEMSIZE>1024) ? 11 :
                      (MEMSIZE>512)  ? 10 :
                      (MEMSIZE>256)  ? 9  :
                      (MEMSIZE>128)  ? 8  :
                      (MEMSIZE>64)   ? 7  :
                      (MEMSIZE>32)   ? 6  :
                      (MEMSIZE>16)   ? 5  :
                      (MEMSIZE>8)    ? 4  :
                      (MEMSIZE>4)    ? 3  :
                      (MEMSIZE>2)    ? 2  : 1;
   localparam NUMBYTE = DWIDTH/8;
   
   // WISHBONE
   input                    clk;
   input                    rst;
   input      [AWIDTH-1:0]  adr;
   input      [DWIDTH-1:0]  din;
   output reg [DWIDTH-1:0]  dout;
   reg        [DWIDTH-1:0]  d_tmp;
   input                    cyc;
   input                    stb;
   input      [NUMBYTE-1:0] sel;
   input      [2:0]	    cti;
   input                    we;
   output reg               ack;
   output reg               err;
   output reg               rty;
   output reg               eod;
   
   // CTL
   input      	     ctl_req;
   input      [7:0]  ctl_id;
   input      [7:0]  ctl_op;
   input      [31:0] ctl_addr;
   input      [31:0] ctl_wdat;
   input      [31:0] ctl_mask;
   output reg        ctl_ack;
   output reg [31:0] ctl_rdat;
   output reg [7:0]  ctl_rtn;

   // INTERNALS
   reg	[7:0]	   mem[0:MEMSIZE-1];
   reg [ASIZE-1:0] laddr;
   reg             burst;
   integer         retrycnt;
   integer         doburst;
   integer         eodcnt;
   integer         mode;
   integer         adrinv;
   localparam RAM = 0, FIFO = 1;
   
   // wb_slave commands
   localparam WBSLAV_CTL_RD = 0,
              WBSLAV_CTL_WR = 1,
              WBSLAV_CTL_VF = 2,
              WBSLAV_MEM_RD = 3,
              WBSLAV_MEM_WR = 4,
              WBSLAV_MEM_VF = 5;
   // config addresses
   localparam WBSLAV_RETRY_VALUE = 0;
   localparam WBSLAV_ENABLE_BURST = 1;
   localparam WBSLAV_EODCNT = 2;
   
   integer i;
   
   // initialization
   initial begin
     // wishbone
     ack = 0;
     err = 0;
     rty = 0;
     eod = 0;
     dout = 0;
     // ctl
     ctl_ack = 0;
     ctl_rdat = 0;
     ctl_rtn = 0;
     // control
     retrycnt = 0;
     burst = 0;
     doburst = 1;
   end

	// combinatorial
   always@* begin
       // CTL
       ctl_ack = 0;
       ctl_rdat = 0;
       ctl_rtn = 0;
       if( ctl_req && (ctl_id == CTLID) ) begin
            case ( ctl_op )
                 WBSLAV_CTL_RD: begin
                    ctl_ack = 1;
                    case ( ctl_addr )
                       WBSLAV_RETRY_VALUE:  ctl_rdat = retrycnt;
                       WBSLAV_ENABLE_BURST: ctl_rdat = doburst;
                       WBSLAV_EODCNT:       ctl_rdat = eodcnt;
                    endcase
                    ctl_rtn = 0;
                    $display("READ: wb_slave %d control[%x] = %x", CTLID, ctl_addr, ctl_rdat );
                 end
                 WBSLAV_CTL_WR: begin
                    ctl_ack = 1;
                    case ( ctl_addr )
                       WBSLAV_RETRY_VALUE:	retrycnt = ctl_wdat;
                       WBSLAV_ENABLE_BURST:	doburst = ctl_wdat;
                       WBSLAV_EODCNT:       eodcnt = ctl_wdat ;
                    endcase
                    ctl_rtn = 0;
                 end
                 WBSLAV_CTL_VF: begin
                    ctl_ack = 1;
                    case ( ctl_addr )
                       WBSLAV_RETRY_VALUE:	ctl_rdat = retrycnt;
                       WBSLAV_ENABLE_BURST:	ctl_rdat = doburst;
                       WBSLAV_EODCNT:       ctl_rdat = eodcnt;
                    endcase
                    ctl_rtn = 0;
                    $display("READ: wb_slave %d control[%x] = %x", CTLID, ctl_addr, ctl_rdat );
                 end
                 WBSLAV_MEM_RD: begin
                    ctl_ack = 1;
                    ctl_rdat = 0;
                    for( i=0; i<4; i=i+1 )
                      ctl_rdat = ctl_rdat + mem[(ctl_addr<<2)+i]*256*i;
                    ctl_rtn = 0;
                    $display("READ: wb_slave %d mem[%x] = %x", CTLID, ctl_addr, ctl_rdat );
                 end
                 WBSLAV_MEM_WR: begin
                    ctl_ack = 1;
                    for( i=0; i<4; i=i+1 )
                       mem[(ctl_addr<<2)+i] = (ctl_wdat >> (8*i));
                    ctl_rtn = 0;
                 end
                 WBSLAV_MEM_VF: begin
                    ctl_ack = 1;
                    ctl_rdat = 0;
                    for( i=3; i>=0; i=i-1 )
                       ctl_rdat = (ctl_rdat<<8) | mem[(ctl_addr<<2)+i];
                    ctl_rtn = 0;
                    if( (ctl_rdat & ctl_mask) === (ctl_wdat & ctl_mask) )
                       $display("VERIFY PASSED: wb_slave %d mem[%x] = %x", CTLID, ctl_addr, ctl_rdat );
                    else begin
                       $display("VERIFY FAILED: wb_slave %d mem[%x] = %x, expected %x", CTLID, ctl_addr, ctl_rdat, ctl_wdat );
                       ctl_rtn = 1;
                    end
                 end
            endcase
       end
   end


	// sequential
   always@( posedge clk or posedge rst ) begin
      if( rst ) begin
         ack <= 0;
         err <= 0;
         rty <= 0;
         eod <= 0;
         eodcnt <= 0;
         dout <= 0;
         burst <= 0;
      end
      else begin
         ack <= 0;
         err <= 0;
         rty <= 0;
         eod <= 0;
         dout <= 0;
         burst <= 0;
         if( stb && ((FULL_ADDR_SIZE>0)?((adr >> (AWIDTH-FULL_ADDR_SIZE)) == FULL_ADDR):cyc)) begin
             if( retrycnt ) begin
                retrycnt <= retrycnt - 1;
                rty <= 1;
             end
             else begin
                case( sel)
                       3  : adrinv = 1;
                      15  : adrinv = 3;
                     255  : adrinv = 7;
                   65535  : adrinv = 15;
                   default: adrinv = 0;
                endcase
                if( we ) begin
                   for( i=0; i<NUMBYTE; i=i+1 ) begin
                       if( sel[i] ) begin
                          if( BIGENDIAN )
                       	    mem[ (adr[ASIZE-1:0] + i) ^ adrinv] <= din >> (8*i);
                       	else
                       	    mem[ adr[ASIZE-1:0] + i ] <= din >> (8*i);
                       end
                   end
                end
                else begin
                   if( eodcnt ) begin
                       eodcnt <= eodcnt - 1;
                       if( eodcnt == 1 )
                          eod <= 1;
                   end
                   
                   // use bus adr at start of burst, local laddr after
                   burst <= doburst & ((cti == 2)||(cti == 1));
                   laddr = (burst)? laddr : adr[ASIZE-1:0];
                   
                   // build response based on sel input
                   d_tmp = 0;
                   for( i=NUMBYTE-1; i>=0; i=i-1 ) begin
                       if( sel[i] ) begin
                          if( BIGENDIAN )
                             d_tmp = (d_tmp<<8) | mem[ (laddr[ASIZE-1:0] + i) ^ adrinv];
                          else
                             d_tmp = (d_tmp<<8) | mem[laddr + i];
                       end
                   end
                   dout <= d_tmp;
                   // increment local address for each active sel
                   for( i=0; i<NUMBYTE; i=i+1 )
                       laddr = laddr + sel[i];
                end
                //ack <= !rty & ((doburst)? (cti != 7) : !ack);
                ack <= !rty & ((doburst & (cti!=7))? 1 : !ack);
             end
         end
      end
   end

endmodule
