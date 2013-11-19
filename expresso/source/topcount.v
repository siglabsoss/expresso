module topcount 
(input  clk, reset,direction, output reg seg_1,seg_2,seg_3,seg_4,seg_5,seg_6,seg_7,seg_8,
			seg_9, seg_10,seg_11,seg_12,seg_13,seg_14,seg_15,seg_16
			

, output ADC0_CSB
, output ADC0_SCLK
, output ADC0_SDIO
, input ADC0_D1_D0_P
, input ADC0_D3_D2_P
, input ADC0_D5_D4_P
, input ADC0_D7_D6_P
, input ADC0_DCO_P
, input ADC0_D9_D8_P
, input ADC0_D11_D10_P
, input ADC0_D13_D12_P
, input ADC0_D15_D14_P
, input ADC0_OR_P	
, output ADC0_CLK_P
			
			, output LOCK, output [7:0]count3t, output [3:0]count2t
	);
	
	
	
	wire ADC0_D1_D0_P;
	
	reg g_trash2;
	
	//assign ADC0_D1_D0_P = g_trash2;
	
	reg ADC0_CLK_P;


	wire [3:0] countt;
	wire [3:0] count2t;
	wire [7:0] count3t;
	reg directionR;
	wire CLKOP, clk_1Hz, CLKOK, clk_1Mhz;
	
	wire seg1, seg2, seg3, seg4, seg5, seg6, seg7, 
	seg8,seg9, seg10,seg11,seg12,seg13,seg14,seg15,seg16;
	
	// clk is 100 mhz, clkop is 500mhz
	// CLKOK is actually 50 mhz
	my_pll my_pll_inst (.CLK(clk), .CLKOK(CLKOK), .CLKOP(CLKOP),.LOCK(LOCK));
	
	wire junk_lock;
	wire clk_200Mhz;
	
	//ADC0_CLK_P
	
	// 200 mhz is actualyl 100 mhz
	adc_pll adc_pll_inst (.CLK(clk), .CLKOP(clk_200Mhz), .LOCK(junk_lock));
	
	clockDivider clockDivider_inst( CLKOK, clk_1Hz );
	// set the clock divider parameter
	defparam clockDivider_inst.periodInCycles = 2000000;
	
	// clk is 100mhz
	clockDivider clockDivider2( clk, clk_1Mhz);
	
	// divide by 100 for 1mhz
	defparam clockDivider2.periodInCycles = 100;
	
	// how many mhz does this give us?
	//clockDivider clockDivider3( CLKOK, clk_200Mhz);
	//defparam clockDivider3.periodInCycles = 2;
	
	always @(*) begin
		ADC0_CLK_P <= clk_200Mhz;
	end
	
	
	count8 counter1 (CLKOP,reset, directionR,count3t);
	count4 counter2 (clk,directionR,reset,count2t);
	count4 counter3 (clk_1Hz,directionR,reset,countt);
	
	
	LEDtest my_LEDtest( direction, seg1, seg2, seg3, seg4, seg5, seg6, seg7, 
	seg8,seg9, seg10,seg11,seg12,seg13,seg14,seg15,seg16, countt);
	
	
	Pop_ADC popAdc( clk_1Mhz, reset, ADC0_CSB, ADC0_SCLK, ADC0_SDIO, ADC0_D1_D0_P );
	 
	
	always @(posedge clk_1Hz )begin
		directionR = direction;
	 seg_1 = seg1;
	 seg_2 = seg2;
	 seg_3 = ADC0_D1_D0_P | ADC0_D3_D2_P | ADC0_D5_D4_P | ADC0_D7_D6_P | ADC0_DCO_P | ADC0_D9_D8_P | ADC0_D11_D10_P | ADC0_D13_D12_P | ADC0_D15_D14_P | ADC0_OR_P;
	 //seg_3 = seg3;
	 seg_4 = seg4;
	 seg_5 = seg5;
	 seg_6 = seg6;
	 seg_7 = seg7;
	 seg_8 = seg8;
	 seg_9 = seg9;
	 seg_10 = seg10;
	 seg_11 = seg11;
	 seg_12 = seg12;
	 seg_13 = seg13;
	 seg_14 = seg14;
	 seg_15 = seg15;
	 seg_16 = seg16;
	 //g_trash2 = ;
end

	reg g_trash;
	always @(*)begin
		g_trash = ADC0_D1_D0_P;
		
		if( g_trash != ADC0_D1_D0_P ) begin
			ADC0_CLK_P = 1;
		end
	end
	
endmodule


