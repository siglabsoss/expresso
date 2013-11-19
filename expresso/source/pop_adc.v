module Pop_ADC(
  input clk_1Mhz
, input reset
, output chip_select
, output spi_clock
, output spi_masteroutslaivein
, input pin0_p
);

//wire chip_select;

// ADC Specific stuff

// chip select is active low
//assign chip_select = spi_ssel_o;
//assign spi_clock = spi_sck_o;
//assign spi_masteroutslaivein = spi_mosi_o;





// wires for the spiMaster using internal names
wire sclk_i;
wire pclk_i;
wire rst_i;
wire spi_ssel_o;
wire spi_sck_o;
wire spi_mosi_o;
wire spi_miso_i;
wire di_req_o;
wire di_i;
wire wren_i;
wire wr_ack_o;
wire do_valid_o;
wire do_o;
wire sck_ena_o;
wire sck_ena_ce_o;
wire do_transfer_o;
wire wren_o;
wire rx_bit_reg_o;
wire state_dbg_o;
wire core_clk_o;
wire core_n_clk_o;
wire core_ce_o;
wire core_n_ce_o;
wire sh_reg_dbg_o;


// wire things up
assign sclk_i = clk_1Mhz;
assign rst_i = reset;



reg chip_select;




reg [7:0] m_count;

always @(*) begin
	if(reset) begin
		m_count <= 8'h0;
	end

	if(~reset && pin0_p) begin
		m_count <= m_count + 1;
		//m_count <= m_count;
	end
	
	if(~pin0_p) begin
		chip_select = 1'b0;
	end
		
	
end





endmodule