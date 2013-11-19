module clockDivider(clk, clkDivOut);

input clk;
output clkDivOut;
reg clkDivOut;
parameter periodInCycles = 20000000; 
parameter halfPeriod = periodInCycles/2; 
reg[31:0] countValue; 


always @(posedge clk) begin
			
			if (countValue == periodInCycles -1 ) begin
				countValue = 0; 
				clkDivOut <= 0;
				end 
			else countValue = countValue +1;
		if (countValue == halfPeriod) clkDivOut <= 1; 
		end 
		 
endmodule