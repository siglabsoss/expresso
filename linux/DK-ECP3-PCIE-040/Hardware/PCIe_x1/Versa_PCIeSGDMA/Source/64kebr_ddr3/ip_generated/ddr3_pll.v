/* Verilog netlist generated by SCUBA ispLever_v72_PROD_Build (44) */
/* Module Version: 4.2 */
/* M:\isplever7.2\ispfpga\bin\nt\scuba.exe -w -n ddr3_pll -lang verilog -synth synplify -arch ep5c00 -type pll -fin 200 -phase_cntl DYNAMIC -mdiv 1 -ndiv 2 -vdiv 2 -fb_mode INTERNAL -duty50 -kdiv 2 -clkoki 0 -use_rst -noclkok2 -e  */
/* Tue Dec 23 12:27:42 2008 */


`timescale 1 ns / 1 ps
module ddr3_pll (CLK, RESET, DPHASE0, DPHASE1, DPHASE2, DPHASE3, CLKOP, 
    CLKOS, CLKOK, LOCK)/* synthesis syn_noprune=1 */;//pragma attribute ddr3_pll dont_touch true 
    input wire CLK;
    input wire RESET;
    input wire DPHASE0;
    input wire DPHASE1;
    input wire DPHASE2;
    input wire DPHASE3;
    output wire CLKOP;
    output wire CLKOS;
    output wire CLKOK;
    output wire LOCK;

    wire CLKOS_t;
    wire CLKOP_t;
    wire DPHASE3_inv;
    wire CLKFB_t;
    wire scuba_vlo;

    INV INV_0 (.A(DPHASE3), .Z(DPHASE3_inv));

    VLO scuba_vlo_inst (.Z(scuba_vlo));

    defparam PLLInst_0.CLKOK_BYPASS = "DISABLED" ;
    defparam PLLInst_0.CLKOS_BYPASS = "DISABLED" ;
    defparam PLLInst_0.CLKOP_BYPASS = "DISABLED" ;
    defparam PLLInst_0.CLKOK_INPUT = "CLKOP" ;
    defparam PLLInst_0.DELAY_PWD = "DISABLED" ;
    defparam PLLInst_0.DELAY_VAL = 0 ;
    defparam PLLInst_0.CLKOS_TRIM_DELAY = 0 ;
    defparam PLLInst_0.CLKOS_TRIM_POL = "RISING" ;
    defparam PLLInst_0.CLKOP_TRIM_DELAY = 0 ;
    defparam PLLInst_0.CLKOP_TRIM_POL = "RISING" ;
    defparam PLLInst_0.PHASE_DELAY_CNTL = "DYNAMIC" ;
    defparam PLLInst_0.DUTY = 8 ;
    defparam PLLInst_0.PHASEADJ = "0.0" ;
    defparam PLLInst_0.CLKOK_DIV = 2 ;
    defparam PLLInst_0.CLKOP_DIV = 2 ;
    defparam PLLInst_0.CLKFB_DIV = 4 ;
    defparam PLLInst_0.CLKI_DIV = 1 ;
    defparam PLLInst_0.FIN = "100.000000" ;
    defparam PLLInst_0.FEEDBK_PATH = "INTERNAL" ;
    EHXPLLF PLLInst_0 (.CLKI(CLK), .CLKFB(CLKFB_t), .RST(RESET), .RSTK(scuba_vlo), 
        .WRDEL(scuba_vlo), .DRPAI3(DPHASE3), .DRPAI2(DPHASE2), .DRPAI1(DPHASE1), 
        .DRPAI0(DPHASE0), .DFPAI3(DPHASE3_inv), .DFPAI2(DPHASE2), .DFPAI1(DPHASE1), 
        .DFPAI0(DPHASE0), .FDA3(scuba_vlo), .FDA2(scuba_vlo), .FDA1(scuba_vlo), 
        .FDA0(scuba_vlo), .CLKOP(CLKOP_t), .CLKOS(CLKOS_t), .CLKOK(CLKOK), 
        .CLKOK2(), .LOCK(LOCK), .CLKINTFB(CLKFB_t))
             /* synthesis FREQUENCY_PIN_CLKOS="400.000000" */
             /* synthesis FREQUENCY_PIN_CLKOP="400.000000" */
             /* synthesis FREQUENCY_PIN_CLKI="100.000000" */
             /* synthesis FREQUENCY_PIN_CLKOK="200.000000" */;

    assign CLKOS = CLKOS_t;
    assign CLKOP = CLKOP_t;


    //pragma attribute PLLInst_0 FREQUENCY_PIN_CLKOS 400.000000
    //pragma attribute PLLInst_0 FREQUENCY_PIN_CLKOP 400.000000
    //pragma attribute PLLInst_0 FREQUENCY_PIN_CLKI 100.000000
    //pragma attribute PLLInst_0 FREQUENCY_PIN_CLKOK 200.000000

endmodule
