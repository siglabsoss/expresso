// $Id: sfif_wbs.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_wbs(wb_clk_i, wb_rst_i,
            wb_dat_i, wb_adr_i, wb_cyc_i, wb_lock_i, wb_sel_i, wb_stb_i, wb_we_i,            
            wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
            tx_cycles, lpbk, loop, run, reset, enable, rx_filter,
            c_npd, c_pd, c_nph, c_ph, tag, tag_cplds, mrd,
            tx_dwen, tx_nlfy, tx_end, tx_st, tx_ctrl,
            ipg_cnt, tx_dv, elapsed_cnt, tx_tlp_cnt, rx_tlp_cnt, rx_empty,
            credit_wait_p_cnt, credit_wait_np_cnt, rx_tlp_timestamp
            , clk_ADC0_DCO_P
			, ADC0_BUS
          
);

input clk_ADC0_DCO_P;
input [7:0] ADC0_BUS;

input         wb_clk_i;
input         wb_rst_i;

input  [15:0] wb_dat_i;
input  [17:0]  wb_adr_i;
input         wb_cyc_i;
input         wb_lock_i;
input   [1:0] wb_sel_i;
input         wb_stb_i;
input         wb_we_i;  
output [15:0] wb_dat_o;
output        wb_ack_o;
output        wb_err_o;
output        wb_rty_o; 

output [15:0] tx_cycles;
output lpbk;
output loop;
output run;
output reset;
output enable;
output rx_filter;

output [3:0] c_npd, c_pd;
output c_nph;
output c_ph;
output [4:0] tag;
output [3:0] tag_cplds;
output mrd;
output tx_dwen;
output tx_nlfy;
output tx_end;
output tx_st;
output tx_dv;
output tx_ctrl;
output [15:0] ipg_cnt;        
input  [31:0] elapsed_cnt, tx_tlp_cnt, rx_tlp_cnt, credit_wait_p_cnt, credit_wait_np_cnt, rx_tlp_timestamp;
input rx_empty;


reg [15:0] wb_dat_o;
reg wb_ack_o;

reg [15:0] tx_cycles;
reg lpbk;
reg loop;
reg run;
reg reset;
reg enable;


reg [3:0] c_npd, c_pd;
reg c_nph;
reg c_ph;
reg [4:0] tag;
reg [3:0] tag_cplds;
reg mrd;
reg tx_dwen;
reg tx_nlfy;
reg tx_end;
reg tx_st;
reg [15:0] ipg_cnt;        
reg tx_dv;
reg tx_ctrl;
reg data_read;
reg rx_filter;
reg dummy;
reg [15:0] temp_data;
assign rd = ~wb_we_i && wb_cyc_i && wb_stb_i;    
assign wr = wb_we_i && wb_cyc_i && wb_stb_i;







// Pop Stuff
parameter SAMPLE_WIDTH = 16;
parameter FIFO_SAMPLES = 2048;
parameter PTR_BITS = 10; // 2^PTR_BITS = FIFO_SAMPLES
parameter POP_START_ADDRESS = 18'h02000;
parameter POP_READ_HEAD = POP_START_ADDRESS - 2;

reg [7:0] pop_counter = 8'h0;


reg [PTR_BITS-1:0] read_head = 10'h0;
reg [PTR_BITS-1:0] write_head = 10'h0;
reg [SAMPLE_WIDTH-1:0] read_cache = 64'h0;
reg [SAMPLE_WIDTH-1:0] read_cache_p = 64'h0;




always @(posedge wb_clk_i)
begin
	
	
	//if( pop_counter == 8'hfe ) begin
	//  pop_counter = 8'h00;
	//end else begin
	  pop_counter <= pop_counter + 1;	
	//end
	if( write_head < FIFO_SAMPLES ) begin
		write_head <= write_head + 1;
	end else begin
		write_head <= 0;
	end
	
end


reg pop_ram_we = 1'b0;
reg pop_ram_wr_clock = 1'b0;
reg pop_ram_wr_clock_en = 1'b0;

reg [SAMPLE_WIDTH-1:0] data_fill = 16'h0;







//assign pop_ram_we = pop_counter != 8'haa;
reg [SAMPLE_WIDTH-1:0] pop_ram_q;
wire [SAMPLE_WIDTH-1:0] pop_ram_q_wire;


// this is what gets passed into the ram
// this is the read head (bytes from the beginning) plus address - address offset, divided by 4
wire [PTR_BITS-1:0] read_head_address;
reg [PTR_BITS-1:0] read_head_address_p;
assign read_head_address = read_head + (wb_adr_i - POP_START_ADDRESS) / 8;
//assign read_head_address_p = read_head_address - 1;
//wire read_head_address = 10'h001;
//wire read_enable = wb_adr_i > POP_START_ADDRESS;



// Pop DDR

wire pll_lock;
wire ddr_sclk;
wire ddr_eclk;
wire[15:0] ADC_Q;
wire[31:0] ADC_Q_WIDE;
wire ddr_eclk;
wire gddl_lock;


//always@(*) begin
//	ADC_Q = ADC_Q_WIDE[15:0];
//end
	

ddr_generic ddr(
		.clk(clk_ADC0_DCO_P),
		.datain(ADC0_BUS),
		.clkdiv_reset(reset),
		.gdll_resetn(1'b1),
		.gdll_uddcntl(1'b0),
		// out
		//.eclk(),
		.sclk(ddr_sclk),
		.gdll_lock(gddl_lock),
		.q(ADC_Q)
		//.q(ADC_Q)
	);
	




// Circular RAM



adc_ram pop_ram (
.WrAddress( write_head ),
.RdAddress( read_head_address_p ),
.Data( ADC_Q),
.WE( 1'b1 ),
.RdClock( wb_clk_i ), 
.RdClockEn( 1'b1 ),
.Reset( wb_rst_i ),
.WrClock( ddr_sclk ),
.WrClockEn( 1'b1 ),
.Q( pop_ram_q_wire )
);




reg [SAMPLE_WIDTH-1:0] read_shift_a = 16'h0;
reg [SAMPLE_WIDTH-1:0] read_shift_b = 16'h0;
reg [SAMPLE_WIDTH-1:0] read_shift_c = 16'h0;


// End pop stuff 








always @(posedge wb_clk_i or posedge wb_rst_i)
begin
  if (wb_rst_i)
  begin
    wb_dat_o <= 0;
    wb_ack_o <= 1'b0;
    tx_dv <= 1'b0;    
    data_read <= 1'b0;
    tx_cycles <= 16'd0;
    ipg_cnt <= 16'd0;
    c_npd <= 4'd0;
    c_pd <= 4'd0;
    c_nph <= 1'b0;
    c_ph <= 1'b0;
    tag <= 5'd0;
    tag_cplds <= 4'd0;
    mrd <= 1'b0;
    tx_dwen <= 1'b0;
    tx_nlfy <= 1'b0;
    tx_end <= 1'b0;    
    tx_st <= 1'b0; 
    tx_ctrl <= 1'b0;
    enable <= 1'b0;
    reset <= 1'b0;
    run <= 1'b0;
    loop <= 1'b0;
    lpbk <= 1'b0;    
    rx_filter <= 1'b0;
    dummy <= 1'b0;
    temp_data <= 0;
	read_head <= 64'h0000;
	read_head_address_p <= 10'h00000;
  end
  else
  begin
    wb_ack_o   <= wb_cyc_i & wb_stb_i & (~ wb_ack_o);
    tx_dv <= 1'b0;
	
	pop_ram_q = pop_ram_q_wire;
    
    if (wb_cyc_i)
    begin
    
	  // This address has 0x1000 added to it as read from the pc.
	  // So a read to 0x2345 will result in 0x3345 in this case statement
      case (wb_adr_i)
	  18'h01034: begin
         if (rd) begin wb_dat_o <= {8'h00, pop_counter}; end
      end  	  
        default:
			
		if( wb_adr_i >= POP_START_ADDRESS && rd ) begin

			read_head_address_p <= ( wb_adr_i - POP_START_ADDRESS ) >> 1;
			wb_dat_o <= read_shift_a;
			read_shift_a <= read_shift_b;
			read_shift_b <= read_shift_c;
			read_shift_c <= pop_ram_q_wire;
			
			
			// This also works... see http://www.sutherland-hdl.com/papers/1996-CUG-presentation_nonblocking_assigns.pdf
			/*
			read_head_address_p <= ( wb_adr_i[15:0] - 16'h2000 ) >> 1;
			
			wb_dat_o <= read_shift_a;
			
			read_shift_a = read_shift_b;
			read_shift_b = read_shift_c;
			read_shift_c = pop_ram_q_wire;
			*/

	
			
		end
		else begin
          //wb_dat_o <= 16'h0000;
		  wb_dat_o <= wb_adr_i[15:0];
		end
		  
		  
      endcase
    
    end // cyc
  end //clk
end


assign wb_err_o = 1'b0;
assign wb_rty_o = 1'b0;


endmodule


