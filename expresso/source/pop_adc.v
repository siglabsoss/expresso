module Pop_ADC(
input clk
, input reset
);



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