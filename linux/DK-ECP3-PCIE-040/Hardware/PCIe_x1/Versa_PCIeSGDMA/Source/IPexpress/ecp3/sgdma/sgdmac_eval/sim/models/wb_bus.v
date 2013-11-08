
// wb_bus -- WISHBONE interconnect model with built-in arbiter

module wb_bus(
  clk_i, 
  rst_i,
  
  // Master Interfaces
  m_dat_i, 
  m_dat_o, 
  m_adr_i, 
  m_sel_i, 
  m_cti_i, 
  m_we_i, 
  m_cyc_i,
  m_stb_i, 
  m_ack_o, 
  m_err_o, 
  m_rty_o,
   m_eod_o,
  
  // Slave Interfaces
  s_dat_i, 
  s_dat_o, 
  s_adr_o, 
  s_sel_o, 
  s_cti_o, 
  s_we_o, 
  s_cyc_o,
  s_stb_o, 
  s_ack_i, 
  s_err_i, 
  s_rty_i, 
  s_eod_i
);

parameter DWIDTH   = 32;           // Data bus Width
parameter AWIDTH   = 32;           // Address bus Width
parameter NUM_MAST = 4;	           // number of masters
parameter NUM_SLAV = 4;	           // number of slaves
parameter SLAVADDR0 =  'hffffffff, // slave start addresses
          SLAVADDR1 =  'hffffffff,
          SLAVADDR2 =  'hffffffff,
          SLAVADDR3 =  'hffffffff,
          SLAVADDR4 =  'hffffffff,
          SLAVADDR5 =  'hffffffff,
          SLAVADDR6 =  'hffffffff,
          SLAVADDR7 =  'hffffffff,
          SLAVADDR8 =  'hffffffff,
          SLAVADDR9 =  'hffffffff,
          SLAVADDR10 = 'hffffffff,
          SLAVADDR11 = 'hffffffff,
          SLAVADDR12 = 'hffffffff,
          SLAVADDR13 = 'hffffffff,
          SLAVADDR14 = 'hffffffff,
          SLAVADDR15 = 'hffffffff;

localparam NUM_SEL  = DWIDTH/8;		// number of byte selects
localparam MWIDTH   = ((NUM_MAST-1)>>3)? 4 : 
                      ((NUM_MAST-1)>>2)? 3 : 
                      ((NUM_MAST-1)>>1)? 2 : 1;

  input	clk_i, rst_i;
  
  // Master Interfaces
  input       [NUM_MAST*DWIDTH-1:0]  m_dat_i;  // master to wishbone write data
  output reg  [DWIDTH-1:0]           m_dat_o;  // wishbone to masters read data
  input       [NUM_MAST*AWIDTH-1:0]  m_adr_i;  // master to wishbone address
  input       [NUM_MAST*NUM_SEL-1:0] m_sel_i;  // master to wishbone byte selects
  input       [NUM_MAST*3-1:0]       m_cti_i;  // master to wishbone cycle type indicator
  input       [NUM_MAST-1:0]         m_we_i;   // master to wishbone write enables
  input       [NUM_MAST-1:0]         m_cyc_i;  // master to wishbone cyc's
  input       [NUM_MAST-1:0]         m_stb_i;  // master to wishbone strobes
  output reg  [NUM_MAST-1:0]         m_eod_o;  // wishbone to master end-of-data
  output reg  [NUM_MAST-1:0]         m_ack_o;  // wishbone to master acknowledge
  output reg  [NUM_MAST-1:0]         m_err_o;  // wishbone to master error
  output reg  [NUM_MAST-1:0]         m_rty_o;  // wishbone to master retry
  
  // Slave Interfaces
  input	      [NUM_SLAV*DWIDTH-1:0]  s_dat_i;  // slave to wishbone read data
  output reg  [DWIDTH-1:0]           s_dat_o;  // wishbone to slaves write data
  output reg  [AWIDTH-1:0]           s_adr_o;  // wishbone to slaves address
  output reg  [NUM_SEL-1:0]          s_sel_o;  // wishbone to slaves byte selects
  output reg  [2:0]                  s_cti_o;  // wishbone to slaves cycle type indicator
  output reg  		             s_we_o;   // wishbone to slaves write enable
  output reg  [15:0]                 s_cyc_o;  // wishbone to slaves cyc
  output reg                         s_stb_o;  // wishbone to slaves strobe
  input       [NUM_SLAV-1:0]         s_eod_i;  // slave to wishbone end-of-data
  input       [NUM_SLAV-1:0]         s_ack_i;  // slave to wishbone acknowledge
  input       [NUM_SLAV-1:0]         s_err_i;  // slave to wishbone error flag
  input       [NUM_SLAV-1:0]         s_rty_i;  // slave to wishbone retry
  integer slavndx;                             // selected slave index
  
  // arbiter
  reg         [NUM_MAST-1:0]         gnt;      // grants from arbiter to masters
  reg         [MWIDTH-1:0]           gindex;   // index variable for round-robin
  
  reg                                scyc;     // internal slave cycle signal
  
  integer a,m,s;


 // arbiter
 always@( posedge clk_i or posedge rst_i ) begin
    if( rst_i ) begin
        gindex = 0;
        gnt    = 0;
    end
    else begin
        if( m_cyc_i[gindex] == 0 ) begin
            gnt = 0;
            for( a=0; a<NUM_MAST; a=a+1 ) begin
                gindex = (gindex < (NUM_MAST - 1))? gindex + 1 : 0 ;
                if( gnt == 0 ) begin
                    if( m_cyc_i[gindex] ) begin
                       gnt[gindex] = 1;
                    end
                end
            end
        end
        else begin
            gnt[gindex] = 1;
        end
    end
 end

 // logic
 always@* begin
  // driven by selected master -- only one master receives gnt
  s_dat_o = 0;
  s_adr_o = 0;
  s_sel_o = 0;
  s_we_o  = 0;
  s_cyc_o = 0;
  s_stb_o = 0;
  for( m=0; m<NUM_MAST; m=m+1 ) begin
     if( gnt[m] ) begin
         scyc    = m_cyc_i[m];
         s_stb_o = m_stb_i[m];
         s_sel_o = m_sel_i >> (m*NUM_SEL);
         s_cti_o = m_cti_i >> (m*3);
         s_we_o  = m_we_i[m];
         s_adr_o = m_adr_i >> (m*AWIDTH);
         s_dat_o = m_dat_i >> (m*DWIDTH);
     end
  end

  // slave address decoding
  if( s_adr_o >= SLAVADDR15 )
      s_cyc_o[15] = scyc;
  else if( s_adr_o >= SLAVADDR14 )
      s_cyc_o[14] = scyc;
  else if( s_adr_o >= SLAVADDR13 )
      s_cyc_o[13] = scyc;
  else if( s_adr_o >= SLAVADDR12 )
      s_cyc_o[12] = scyc;
  else if( s_adr_o >= SLAVADDR11 )
      s_cyc_o[11] = scyc;
  else if( s_adr_o >= SLAVADDR10 )
      s_cyc_o[10] = scyc;
  else if( s_adr_o >= SLAVADDR9 )
      s_cyc_o[9] = scyc;
  else if( s_adr_o >= SLAVADDR8 )
      s_cyc_o[8] = scyc;
  else if( s_adr_o >= SLAVADDR7 )
      s_cyc_o[7] = scyc;
  else if( s_adr_o >= SLAVADDR6 )
      s_cyc_o[6] = scyc;
  else if( s_adr_o >= SLAVADDR5 )
      s_cyc_o[5] = scyc;
  else if( s_adr_o >= SLAVADDR4 )
      s_cyc_o[4] = scyc;
  else if( s_adr_o >= SLAVADDR3 )
      s_cyc_o[3] = scyc;
  else if( s_adr_o >= SLAVADDR2 )
      s_cyc_o[2] = scyc;
  else if( s_adr_o >= SLAVADDR1 )
      s_cyc_o[1] = scyc;
  else if( s_adr_o >= SLAVADDR0 )
      s_cyc_o[0] = scyc;

  // driven by responding slave -- only one slave will respond
  m_dat_o = 0;
  m_ack_o  = 0;
  m_eod_o  = 0;
  m_err_o  = 0;
  m_rty_o  = 0;
  for( s=0; s<NUM_SLAV; s=s+1 ) begin
     m_dat_o  = m_dat_o | (s_dat_i >> (s*DWIDTH));
     for( m=0; m<NUM_MAST; m=m+1 ) begin
        if( gnt[m] ) begin
         m_ack_o[m]  = m_ack_o[m] | s_ack_i[s];
         m_eod_o[m]  = m_eod_o[m] | s_eod_i[s];
         m_err_o[m]  = m_err_o[m] | s_err_i[s];
         m_rty_o[m]  = m_rty_o[m] | s_rty_i[s];
        end
     end
  end
   
 end

endmodule

