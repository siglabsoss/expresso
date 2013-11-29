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
, input clk_ADC0_DCO_P // synthesis syn_useioff = 1
, input ADC0_D9_D8_P
, input ADC0_D11_D10_P
, input ADC0_D13_D12_P
, input ADC0_D15_D14_P
, input ADC0_OR_P	
, output ADC0_CLK_P
			
			, output LOCK, output [7:0]count3t, output [3:0]count2t
	);
	
	
	wire reset; /* synthesis syn_keep= 1 OPT= "KEEP" */
	
	wire clk_ADC0_DCO_c;
	wire ADC0_D13_D12_P;
	reg ADC0_D15_D14;
	
	
	assign clk_ADC0_DCO_c = clk_ADC0_DCO_P;
	always@(*) begin
		ADC0_D15_D14 <= ADC0_D15_D14_P;
	end
	
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
	
	wire junk_lock;
	wire clk_200Mhz;
	
	wire what_is_this_net;
	
	//ADC0_CLK_P
	
	// 200 mhz is actualyl 100 mhz
	//adc_pll adc_pll_inst (.CLK(clk), .CLKOP(clk_200Mhz), .LOCK(junk_lock));
	

	
	always @(*) begin
		ADC0_CLK_P <= clk;
	end
	
	wire pll_lock;
	wire[15:0] ADC_Q;
	wire ddr_sclk;
	// ADC0_D5_D4_P is a TDQS
	ddr_generic ddr(
		.clk(clk_ADC0_DCO_c),
		.datain({ADC0_D15_D14,ADC0_D13_D12_P,ADC0_D11_D10_P,ADC0_D9_D8_P,ADC0_D7_D6_P,ADC0_D5_D4_P,ADC0_D3_D2_P,ADC0_D1_D0_P}),
		.pll_reset(reset),
		// out
		.pll_lock(pll_lock),
		.sclk(ddr_sclk),
		.q(ADC_Q)
	);
	 
	
	wire ADC0_CSB, ADC0_SCLK, ADC0_SDIO;
	
	/*
	Pop_ADC popAdc( clk_1Mhz, clk, reset, ADC0_CSB, ADC0_SCLK, ADC0_SDIO,
	ADC0_D1_D0_P,
	ADC0_D3_D2_P,
	ADC0_D5_D4_P,
	ADC0_D7_D6_P,
	1'b0,
	ADC0_D9_D8_P,
	ADC0_D11_D10_P,
	ADC0_D13_D12_P,
	ADC0_D15_D14_P,
	ADC0_OR_P
	//ADC_Q
	);*/
	
	reg debug;
	
	always @(posedge ddr_sclk )begin
		if( reset ) begin
			debug <= 1'b1;
		end
		
		
		seg_9 = ADC_Q[8];
		seg_10 = ADC_Q[9];
		seg_11 = ADC_Q[10];
		seg_12 = ADC_Q[11];
		seg_13 = ADC_Q[12];
		seg_14 = ADC_Q[13];
		seg_15 = ADC_Q[14];
		seg_16 = ADC_Q[15];
		
		
		
		seg_1 = ADC_Q[0];
		seg_2 = ADC_Q[1];
		if( ~reset ) begin
			seg_3 = ADC_Q[2];
		end
		seg_4 = ADC_Q[3];
		seg_5 = ADC_Q[4];
		seg_6 = ADC_Q[5];
		seg_7 = ADC_Q[6];
		seg_8 = ADC_Q[7];
	end
	 
	/*
	always @(posedge clk_1Hz )begin
		directionR = direction;
	 
	 seg_8 = seg8;
	 seg_9 = seg9;
	 seg_10 = seg10;
	 seg_11 = seg11;
	 seg_12 = seg12;
	 seg_13 = seg13;
	 seg_14 = seg14;
	 seg_15 = seg15;
	 seg_16 = seg16;
	 //seg_16 = ADC0_D1_D0_P | ADC0_D3_D2_P | ADC0_D5_D4_P | ADC0_D7_D6_P | ADC0_D9_D8_P | ADC0_D11_D10_P | ADC0_D13_D12_P | ADC0_D15_D14_P | ADC0_OR_P;
end*/

	
endmodule


