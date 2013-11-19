module Pop_ADC(
  input clk_1Mhz
, input adc_clock
, input reset
, output chip_select
, output spi_clock
, output spi_masteroutslaivein
, input ADC0_D1_D0
, input ADC0_D3_D2
, input ADC0_D5_D4
, input ADC0_D7_D6
, input ADC0_DCO
, input ADC0_D9_D8
, input ADC0_D11_D10
, input ADC0_D13_D12
, input ADC0_D15_D14
, input ADC0_OR
//, output [15:0] ADC_Q
);

//wire chip_select;

// ADC Specific stuff

// chip select is active low
//assign chip_select = spi_ssel_o;
//assign spi_clock = spi_sck_o;
//assign spi_masteroutslaivein = spi_mosi_o;


reg [7:0] ADC_Data_In;

always@(*)begin
	ADC_Data_In <= {ADC0_D15_D14,ADC0_D13_D12,ADC0_D11_D10,ADC0_D9_D8,ADC0_D7_D6,ADC0_D5_D4,ADC0_D3_D2,ADC0_D1_D0};
end

//wire ddr_sclk;
wire spi_clock;
wire spi_masteroutslaivein;


//wire[15:0] ADC_Q;
//ddr_generic ddr(.clk(ADC0_DCO), .datain({ADC0_D15_D14,ADC0_D13_D12,ADC0_D11_D10,ADC0_D9_D8,ADC0_D7_D6,ADC0_D5_D4,ADC0_D3_D2,ADC0_D1_D0}), .sclk(ddr_sclk), .q(ADC_Q));


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
	
	//m_count <= m_count + 1;
	
end





endmodule