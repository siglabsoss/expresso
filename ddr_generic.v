/* Verilog netlist generated by SCUBA Diamond_2.2_Production (99) */
/* Module Version: 5.4 */
/* C:\lscc\diamond\2.2_x64\ispfpga\bin\nt64\scuba.exe -w -n ddr_generic -lang verilog -synth synplify -bus_exp 7 -bb -arch ep5c00 -type iol -mode in -io_type LVDS25 -width 8 -freq_in 200 -gear 1 -clk sclk -e  */
/* Tue Nov 19 14:44:48 2013 */


`timescale 1 ns / 1 ps
module ddr_generic (clk, sclk, datain, q)/* synthesis syn_noprune=1 *//* synthesis NGD_DRC_MASK=1 */;// exemplar attribute ddr_generic dont_touch true 
    input wire clk;
    input wire [7:0] datain;
    output wire sclk;
    output wire [15:0] q;

    wire qb7;
    wire qa7;
    wire qb6;
    wire qa6;
    wire qb5;
    wire qa5;
    wire qb4;
    wire qa4;
    wire qb3;
    wire qa3;
    wire qb2;
    wire qa2;
    wire qb1;
    wire qa1;
    wire qb0;
    wire qa0;
    wire sclk_t;
    wire dataini_t7;
    wire dataini_t6;
    wire dataini_t5;
    wire dataini_t4;
    wire dataini_t3;
    wire dataini_t2;
    wire dataini_t1;
    wire dataini_t0;
    wire buf_clk;
    wire buf_dataini7;
    wire buf_dataini6;
    wire buf_dataini5;
    wire buf_dataini4;
    wire buf_dataini3;
    wire buf_dataini2;
    wire buf_dataini1;
    wire buf_dataini0;

    IDDRXD1 Inst_IDDRXD1_1_2 (.D(dataini_t7), .SCLK(sclk_t), .QA(qa7), .QB(qb7))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_1_1 (.D(dataini_t6), .SCLK(sclk_t), .QA(qa6), .QB(qb6))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_1_0 (.D(dataini_t5), .SCLK(sclk_t), .QA(qa5), .QB(qb5))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_0_4 (.D(dataini_t4), .SCLK(sclk_t), .QA(qa4), .QB(qb4))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_0_3 (.D(dataini_t3), .SCLK(sclk_t), .QA(qa3), .QB(qb3))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_0_2 (.D(dataini_t2), .SCLK(sclk_t), .QA(qa2), .QB(qb2))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_0_1 (.D(dataini_t1), .SCLK(sclk_t), .QA(qa1), .QB(qb1))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    IDDRXD1 Inst_IDDRXD1_0_0 (.D(dataini_t0), .SCLK(sclk_t), .QA(qa0), .QB(qb0))
             /* synthesis IDDRAPPS="SCLK_CENTERED" */;

    DELAYC udel_dataini7 (.A(buf_dataini7), .Z(dataini_t7));

    DELAYC udel_dataini6 (.A(buf_dataini6), .Z(dataini_t6));

    DELAYC udel_dataini5 (.A(buf_dataini5), .Z(dataini_t5));

    DELAYC udel_dataini4 (.A(buf_dataini4), .Z(dataini_t4));

    DELAYC udel_dataini3 (.A(buf_dataini3), .Z(dataini_t3));

    DELAYC udel_dataini2 (.A(buf_dataini2), .Z(dataini_t2));

    DELAYC udel_dataini1 (.A(buf_dataini1), .Z(dataini_t1));

    DELAYC udel_dataini0 (.A(buf_dataini0), .Z(dataini_t0));

    IB Inst2_IB (.I(clk), .O(buf_clk))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB7 (.I(datain[7]), .O(buf_dataini7))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB6 (.I(datain[6]), .O(buf_dataini6))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB5 (.I(datain[5]), .O(buf_dataini5))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB4 (.I(datain[4]), .O(buf_dataini4))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB3 (.I(datain[3]), .O(buf_dataini3))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB2 (.I(datain[2]), .O(buf_dataini2))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB1 (.I(datain[1]), .O(buf_dataini1))
             /* synthesis IO_TYPE="LVDS25" */;

    IB Inst1_IB0 (.I(datain[0]), .O(buf_dataini0))
             /* synthesis IO_TYPE="LVDS25" */;

    assign sclk = sclk_t;
    assign q[15] = qb7;
    assign q[14] = qb6;
    assign q[13] = qb5;
    assign q[12] = qb4;
    assign q[11] = qb3;
    assign q[10] = qb2;
    assign q[9] = qb1;
    assign q[8] = qb0;
    assign q[7] = qa7;
    assign q[6] = qa6;
    assign q[5] = qa5;
    assign q[4] = qa4;
    assign q[3] = qa3;
    assign q[2] = qa2;
    assign q[1] = qa1;
    assign q[0] = qa0;
    assign sclk_t = buf_clk;


    // exemplar begin
    // exemplar attribute Inst_IDDRXD1_1_2 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_1_1 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_1_0 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_0_4 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_0_3 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_0_2 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_0_1 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst_IDDRXD1_0_0 IDDRAPPS SCLK_CENTERED
    // exemplar attribute Inst2_IB IO_TYPE LVDS25
    // exemplar attribute Inst1_IB7 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB6 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB5 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB4 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB3 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB2 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB1 IO_TYPE LVDS25
    // exemplar attribute Inst1_IB0 IO_TYPE LVDS25
    // exemplar end

endmodule