module pop_source(
tx_data
);



output [15:0] tx_data;

reg m_tx_data;


assign tx_data = m_tx_data;


always@(*)
begin
    m_tx_data <= 16'b0001010010000100;
end

endmodule
