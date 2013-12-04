// $Id: wb_arb.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module wb_arb #(
parameter c_DATA_WIDTH = 64,
parameter S0_BASE = 32'h0000,
parameter S1_BASE = 32'h0000,
parameter S2_BASE = 32'h0000,
parameter S3_BASE = 32'h0000
)
(clk, rstn,
              
              m0_dat_i, m0_dat_o, m0_adr_i, m0_sel_i, m0_we_i, m0_cyc_i, m0_cti_i,
	      m0_stb_i, m0_ack_o, m0_err_o, m0_rty_o, 

              m1_dat_i, m1_dat_o, m1_adr_i, m1_sel_i, m1_we_i, m1_cyc_i, m1_cti_i,
	      m1_stb_i, m1_ack_o, m1_err_o, m1_rty_o, 
	      
              s0_dat_i, s0_dat_o, s0_adr_o, s0_sel_o, s0_we_o, s0_cyc_o, s0_cti_o,
              s0_stb_o, s0_ack_i, s0_err_i, s0_rty_i,                
              
              s1_dat_i, s1_dat_o, s1_adr_o, s1_sel_o, s1_we_o, s1_cyc_o, s1_cti_o,
              s1_stb_o, s1_ack_i, s1_err_i, s1_rty_i,
              
              s2_dat_i, s2_dat_o, s2_adr_o, s2_sel_o, s2_we_o, s2_cyc_o, s2_cti_o,
              s2_stb_o, s2_ack_i, s2_err_i, s2_rty_i,
              
              s3_dat_i, s3_dat_o, s3_adr_o, s3_sel_o, s3_we_o, s3_cyc_o, s3_cti_o,
              s3_stb_o, s3_ack_i, s3_err_i, s3_rty_i
);

input clk;
input rstn;

input	[c_DATA_WIDTH-1:0]	m0_dat_i;
output	[c_DATA_WIDTH-1:0]	m0_dat_o;
input	[31:0]	m0_adr_i;
input	[c_DATA_WIDTH/8-1:0]	m0_sel_i;
input   [2:0]   m0_cti_i;
input	m0_we_i;
input	m0_cyc_i;
input	m0_stb_i;
output	m0_ack_o;
output	m0_err_o;
output	m0_rty_o;

input	[c_DATA_WIDTH-1:0]	m1_dat_i;
output	[c_DATA_WIDTH-1:0]	m1_dat_o;
input	[31:0]	m1_adr_i;
input	[c_DATA_WIDTH/8-1:0]	m1_sel_i;
input   [2:0]   m1_cti_i;
input	m1_we_i;
input	m1_cyc_i;
input	m1_stb_i;
output	m1_ack_o;
output	m1_err_o;
output	m1_rty_o;

input	[c_DATA_WIDTH-1:0]	s0_dat_i;
output	[c_DATA_WIDTH-1:0]	s0_dat_o;
output	[31:0]	s0_adr_o;
output	[c_DATA_WIDTH/8-1:0]	s0_sel_o;
output  [2:0]   s0_cti_o;
output	s0_we_o;
output	s0_cyc_o;
output	s0_stb_o;
input	s0_ack_i;
input	s0_err_i;
input	s0_rty_i;

input	[c_DATA_WIDTH-1:0]	s1_dat_i;
output	[c_DATA_WIDTH-1:0]	s1_dat_o;
output	[31:0]	s1_adr_o;
output	[c_DATA_WIDTH/8-1:0]	s1_sel_o;
output  [2:0]   s1_cti_o;
output	s1_we_o;
output	s1_cyc_o;
output	s1_stb_o;
input	s1_ack_i;
input	s1_err_i;
input	s1_rty_i;

input	[c_DATA_WIDTH-1:0]	s2_dat_i;
output	[c_DATA_WIDTH-1:0]	s2_dat_o;
output	[31:0]	s2_adr_o;
output	[c_DATA_WIDTH/8-1:0]	s2_sel_o;
output  [2:0]   s2_cti_o;
output	s2_we_o;
output	s2_cyc_o;
output	s2_stb_o;
input	s2_ack_i;
input	s2_err_i;
input	s2_rty_i;

input	[c_DATA_WIDTH-1:0]	s3_dat_i;
output	[c_DATA_WIDTH-1:0]	s3_dat_o;
output	[31:0]	s3_adr_o;
output	[c_DATA_WIDTH/8-1:0]	s3_sel_o;
output  [2:0]   s3_cti_o;
output	s3_we_o;
output	s3_cyc_o;
output	s3_stb_o;
input	s3_ack_i;
input	s3_err_i;
input	s3_rty_i;





reg [1:0] rr;
reg [c_DATA_WIDTH-1:0] m_dat, s_dat;
reg s_ack;
reg s_err;
reg s_rty;   
reg m_cyc;
reg m_stb;
reg m_we;
reg [31:0] m_adr;
reg [c_DATA_WIDTH/8-1:0] m_sel;
reg [2:0] m_cti;

reg s0_cyc_o;
reg s0_stb_o;

reg s1_cyc_o;
reg s1_stb_o;

reg s2_cyc_o;
reg s2_stb_o;

reg s3_cyc_o;
reg s3_stb_o;


reg [c_DATA_WIDTH-1:0] m0_dat_o;
reg m0_ack_o;
reg m0_err_o;
reg m0_rty_o;

reg [c_DATA_WIDTH-1:0] m1_dat_o;
reg m1_ack_o;
reg m1_err_o;
reg m1_rty_o;

reg s0_sel, s1_sel, s2_sel, s3_sel;    


// Both the PCIe and SGDMA place valid addr on the bus 1 clock cycle befor cyc and stb.  This allows
// the arbiter to register the slave select lines.
always @(posedge clk or negedge rstn)
begin
  if (~rstn)
  begin
    s0_sel <= 1'b0;
    s1_sel <= 1'b0;
    s2_sel <= 1'b0;
    s3_sel <= 1'b0;
  end
  else
  begin
    s0_sel <= ~rr[0] ? (m0_adr_i[31:12] >= S0_BASE[31:12]) : 1'b0; // M0 (PCIe) only talks to S0 and S1
    s1_sel <= ~rr[0] ? (m0_adr_i[31:12] >= S1_BASE[31:12]) : 1'b0;
    s2_sel <= 1'b0; //(m_adr[31:12] >= S2_BASE[31:12]);
    s3_sel <= (m_adr[31:12] >= S3_BASE[31:12]);  
  end
end



// Broadcast signals
assign s3_dat_o = m_dat;
assign s3_adr_o = m_adr;  
assign s3_sel_o = m_sel;
assign s3_cti_o = m_cti;
assign s3_we_o  = m_we;
assign s2_dat_o = m_dat;
assign s2_adr_o = m_adr;  
assign s2_sel_o = m_sel;
assign s2_cti_o = m_cti;
assign s2_we_o  = m_we;
assign s1_dat_o = m_dat;
assign s1_adr_o = m0_adr_i; // M0 (PCIe) only talks to S1 and S0
assign s1_sel_o = m0_sel_i;
assign s1_cti_o = m0_cti_i;
assign s1_we_o  = m0_we_i;
assign s0_dat_o = m0_dat_i;
assign s0_adr_o = m0_adr_i;
assign s0_sel_o = m0_sel_i;
assign s0_cti_o = m0_cti_i;
assign s0_we_o  = m0_we_i;


// Slave mux
//always @(posedge clk or negedge rstn)
always @(*)
begin
    casex ({s0_sel, s1_sel, s2_sel, s3_sel})
    4'b0000: // no match
    begin
      s0_cyc_o <= 1'b0;
      s0_stb_o <= 1'b0;    
      s1_cyc_o <= 1'b0;
      s1_stb_o <= 1'b0;        
      s2_cyc_o <= 1'b0;
      s2_stb_o <= 1'b0;  
      s3_cyc_o <= 1'b0;       
      s3_stb_o <= 1'b0;             
      s_dat <= 0;
      s_ack <= 1'b0;
      s_err <= 1'b0;
      s_rty <= 1'b0;        
    end
    4'bxxx1: // s3
    begin
      s0_cyc_o <= 1'b0;
      s0_stb_o <= 1'b0;
      s1_cyc_o <= 1'b0;
      s1_stb_o <= 1'b0;
      s2_cyc_o <= 1'b0;
      s2_stb_o <= 1'b0;      
      s3_cyc_o <= m_cyc;
      s3_stb_o <= m_stb;        
      s_dat <= s3_dat_i;
      s_ack <= s3_ack_i;
      s_err <= s3_err_i;
      s_rty <= s3_rty_i;        
    end
    4'bxx10: // s2
    begin
      s0_cyc_o <= 1'b0;
      s0_stb_o <= 1'b0;
      s1_cyc_o <= 1'b0;
      s1_stb_o <= 1'b0;
      s2_cyc_o <= m_cyc;
      s2_stb_o <= m_stb;    
      s3_cyc_o <= 1'b0;
      s3_stb_o <= 1'b0;      
      s_dat <= s2_dat_i;
      s_ack <= s2_ack_i;
      s_err <= s2_err_i;
      s_rty <= s2_rty_i;        
    end
    4'bx100: // s1
    begin
      s0_cyc_o <= 1'b0;
      s0_stb_o <= 1'b0;
      s1_cyc_o <= m0_cyc_i; // M0 (PCIe) only talks to S1
      s1_stb_o <= m0_stb_i;    
      s2_cyc_o <= 1'b0;
      s2_stb_o <= 1'b0;
      s3_cyc_o <= 1'b0;
      s3_stb_o <= 1'b0;
      s_dat <= s1_dat_i;
      s_ack <= s1_ack_i;
      s_err <= s1_err_i;
      s_rty <= s1_rty_i;        
    end
    4'b1000: // s0
    begin
      s0_cyc_o <= m0_cyc_i;  // M0 (PCIe) only talks to S1
      s0_stb_o <= m0_stb_i;    
      s1_cyc_o <= 1'b0;
      s1_stb_o <= 1'b0;    
      s2_cyc_o <= 1'b0;
      s2_stb_o <= 1'b0;
      s3_cyc_o <= 1'b0;
      s3_stb_o <= 1'b0;
      s_dat <= s0_dat_i;
      s_ack <= s0_ack_i;
      s_err <= s0_err_i;
      s_rty <= s0_rty_i;       
    end
    default:
    begin
      s0_cyc_o <= 0;  // M0 (PCIe) only talks to S1
      s0_stb_o <= 0;    
      s1_cyc_o <= 1'b0;
      s1_stb_o <= 1'b0;    
      s2_cyc_o <= 1'b0;
      s2_stb_o <= 1'b0;
      s3_cyc_o <= 1'b0;
      s3_stb_o <= 1'b0;
      s_dat <= 0;
      s_ack <= 0;
      s_err <= 0;
      s_rty <= 0;         
    end
    endcase  
end

// Master mux
//always @(posedge clk or negedge rstn)
always @(*)
begin
    case (rr)
    2'b00:
    begin    
      m_dat <= m0_dat_i;
      m_cyc <= m0_cyc_i;
      m_stb <= m0_stb_i;
      m_we <= m0_we_i;
      m_adr <= m0_adr_i;
      m_sel <= m0_sel_i;
      m_cti <= m0_cti_i;
      m0_dat_o <= s_dat;
      m0_ack_o <= s_ack;
      m0_err_o <= s_err;
      m0_rty_o <= s_rty;      
      m1_dat_o <= 0;
      m1_ack_o <= 1'b0;
      m1_err_o <= 1'b0;
      m1_rty_o <= 1'b0;      
    end
    2'b01:
    begin
      m_dat <= m1_dat_i;
      m_cyc <= m1_cyc_i;
      m_stb <= m1_stb_i;
      m_we <= m1_we_i;
      m_adr <= m1_adr_i;
      m_sel <= m1_sel_i;
      m_cti <= m1_cti_i;
      m0_dat_o <= 0;
      m0_ack_o <= 1'b0;
      m0_err_o <= 1'b0;
      m0_rty_o <= 1'b0;        
      m1_dat_o <= s_dat;
      m1_ack_o <= s_ack;
      m1_err_o <= s_err;
      m1_rty_o <= s_rty;      
    end
    default:
    begin
      m_dat <= 0;
      m_cyc <= 0;
      m_stb <= 0;
      m_we <= 0;
      m_adr <= 0;
      m_sel <= 0;
      m_cti <= 0;
      m0_dat_o <= 0;
      m0_ack_o <= 1'b0;
      m0_err_o <= 1'b0;
      m0_rty_o <= 1'b0;        
      m1_dat_o <= 0;
      m1_ack_o <= 0;
      m1_err_o <= 0;
      m1_rty_o <= 0;      
    end
    endcase  
end

    

always @(posedge clk or negedge rstn)
begin
  if (~rstn)
  begin
    rr <= 2'b00;
  end
  else
  begin
    case (rr)
    2'b00:
      if (~m0_cyc_i && m1_cyc_i)
        rr <= 2'b01;      
    2'b01:
      if (~m1_cyc_i && m0_cyc_i)
        rr <= 2'b00;      
    default:
    begin
      rr <= 2'b00;    
    end
    endcase
  end // clk
end


 
endmodule

