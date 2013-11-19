module Pop_ADC(
input clk_1Mhz
, input reset
);



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






spi_master spiMaster(
  sclk_i
, pclk_i
, rst_i
, spi_ssel_o
, spi_sck_o
, spi_mosi_o
, spi_miso_i
, di_req_o
, di_i
, wren_i
, wr_ack_o
, do_valid_o
, do_o
, sck_ena_o
, sck_ena_ce_o
, do_transfer_o
, wren_o
, rx_bit_reg_o
, state_dbg_o
, core_clk_o
, core_n_clk_o
, core_ce_o
, core_n_ce_o
, sh_reg_dbg_o
);

defparam spiMaster.SPI_2X_CLK_DIV = 2;


reg [7:0] m_count;

always @(*) begin
	if(reset) begin
		m_count <= 8'h0;
	end

	if(~reset) begin
		m_count <= m_count + 1;
	end
end





endmodule