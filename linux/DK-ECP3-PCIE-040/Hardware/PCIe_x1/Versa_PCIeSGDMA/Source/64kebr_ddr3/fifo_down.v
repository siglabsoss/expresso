/* Verilog netlist generated by SCUBA Diamond_1.1_Production (517) */
/* Module Version: 5.4 */
/* C:\lscc\diamond\1.1\ispfpga\bin\nt\scuba.exe -w -lang verilog -synth synplify -bus_exp 7 -bb -arch ep5c00 -type ebfifo -depth 64 -width 99 -depth 64 -rdata_width 99 -no_enable -pe -1 -pf -1 -e  */
/* Thu Apr 21 13:20:46 2011 */


`timescale 1 ns / 1 ps
module fifo_down (Data, WrClock, RdClock, WrEn, RdEn, Reset, RPReset, Q, 
    Empty, Full);
    input wire [98:0] Data;
    input wire WrClock;
    input wire RdClock;
    input wire WrEn;
    input wire RdEn;
    input wire Reset;
    input wire RPReset;
    output wire [98:0] Q;
    output wire Empty;
    output wire Full;

    wire invout_1;
    wire invout_0;
    wire w_gdata_0;
    wire w_gdata_1;
    wire w_gdata_2;
    wire w_gdata_3;
    wire w_gdata_4;
    wire w_gdata_5;
    wire wptr_0;
    wire wptr_1;
    wire wptr_2;
    wire wptr_3;
    wire wptr_4;
    wire wptr_5;
    wire wptr_6;
    wire r_gdata_0;
    wire r_gdata_1;
    wire r_gdata_2;
    wire r_gdata_3;
    wire r_gdata_4;
    wire r_gdata_5;
    wire rptr_0;
    wire rptr_1;
    wire rptr_2;
    wire rptr_3;
    wire rptr_4;
    wire rptr_5;
    wire rptr_6;
    wire w_gcount_0;
    wire w_gcount_1;
    wire w_gcount_2;
    wire w_gcount_3;
    wire w_gcount_4;
    wire w_gcount_5;
    wire w_gcount_6;
    wire r_gcount_0;
    wire r_gcount_1;
    wire r_gcount_2;
    wire r_gcount_3;
    wire r_gcount_4;
    wire r_gcount_5;
    wire r_gcount_6;
    wire w_gcount_r20;
    wire w_gcount_r0;
    wire w_gcount_r21;
    wire w_gcount_r1;
    wire w_gcount_r22;
    wire w_gcount_r2;
    wire w_gcount_r23;
    wire w_gcount_r3;
    wire w_gcount_r24;
    wire w_gcount_r4;
    wire w_gcount_r25;
    wire w_gcount_r5;
    wire w_gcount_r26;
    wire w_gcount_r6;
    wire r_gcount_w20;
    wire r_gcount_w0;
    wire r_gcount_w21;
    wire r_gcount_w1;
    wire r_gcount_w22;
    wire r_gcount_w2;
    wire r_gcount_w23;
    wire r_gcount_w3;
    wire r_gcount_w24;
    wire r_gcount_w4;
    wire r_gcount_w25;
    wire r_gcount_w5;
    wire r_gcount_w26;
    wire r_gcount_w6;
    wire empty_i;
    wire rRst;
    wire full_i;
    wire iwcount_0;
    wire iwcount_1;
    wire w_gctr_ci;
    wire iwcount_2;
    wire iwcount_3;
    wire co0;
    wire iwcount_4;
    wire iwcount_5;
    wire co1;
    wire iwcount_6;
    wire co3;
    wire wcount_6;
    wire co2;
    wire scuba_vhi;
    wire ircount_0;
    wire ircount_1;
    wire r_gctr_ci;
    wire ircount_2;
    wire ircount_3;
    wire co0_1;
    wire ircount_4;
    wire ircount_5;
    wire co1_1;
    wire ircount_6;
    wire co3_1;
    wire rcount_6;
    wire co2_1;
    wire rden_i;
    wire cmp_ci;
    wire wcount_r0;
    wire wcount_r1;
    wire rcount_0;
    wire rcount_1;
    wire co0_2;
    wire wcount_r2;
    wire w_g2b_xor_cluster_0;
    wire rcount_2;
    wire rcount_3;
    wire co1_2;
    wire wcount_r4;
    wire wcount_r5;
    wire rcount_4;
    wire rcount_5;
    wire co2_2;
    wire empty_cmp_clr;
    wire empty_cmp_set;
    wire empty_d;
    wire empty_d_c;
    wire wren_i;
    wire cmp_ci_1;
    wire rcount_w0;
    wire rcount_w1;
    wire wcount_0;
    wire wcount_1;
    wire co0_3;
    wire rcount_w2;
    wire r_g2b_xor_cluster_0;
    wire wcount_2;
    wire wcount_3;
    wire co1_3;
    wire rcount_w4;
    wire rcount_w5;
    wire wcount_4;
    wire wcount_5;
    wire co2_3;
    wire full_cmp_clr;
    wire full_cmp_set;
    wire full_d;
    wire full_d_c;
    wire scuba_vlo;

    AND2 AND2_t14 (.A(WrEn), .B(invout_1), .Z(wren_i));

    INV INV_1 (.A(full_i), .Z(invout_1));

    AND2 AND2_t13 (.A(RdEn), .B(invout_0), .Z(rden_i));

    INV INV_0 (.A(empty_i), .Z(invout_0));

    OR2 OR2_t12 (.A(Reset), .B(RPReset), .Z(rRst));

    XOR2 XOR2_t11 (.A(wcount_0), .B(wcount_1), .Z(w_gdata_0));

    XOR2 XOR2_t10 (.A(wcount_1), .B(wcount_2), .Z(w_gdata_1));

    XOR2 XOR2_t9 (.A(wcount_2), .B(wcount_3), .Z(w_gdata_2));

    XOR2 XOR2_t8 (.A(wcount_3), .B(wcount_4), .Z(w_gdata_3));

    XOR2 XOR2_t7 (.A(wcount_4), .B(wcount_5), .Z(w_gdata_4));

    XOR2 XOR2_t6 (.A(wcount_5), .B(wcount_6), .Z(w_gdata_5));

    XOR2 XOR2_t5 (.A(rcount_0), .B(rcount_1), .Z(r_gdata_0));

    XOR2 XOR2_t4 (.A(rcount_1), .B(rcount_2), .Z(r_gdata_1));

    XOR2 XOR2_t3 (.A(rcount_2), .B(rcount_3), .Z(r_gdata_2));

    XOR2 XOR2_t2 (.A(rcount_3), .B(rcount_4), .Z(r_gdata_3));

    XOR2 XOR2_t1 (.A(rcount_4), .B(rcount_5), .Z(r_gdata_4));

    XOR2 XOR2_t0 (.A(rcount_5), .B(rcount_6), .Z(r_gdata_5));

    defparam LUT4_15.initval =  16'h6996 ;
    ROM16X1A LUT4_15 (.AD3(w_gcount_r23), .AD2(w_gcount_r24), .AD1(w_gcount_r25), 
        .AD0(w_gcount_r26), .DO0(w_g2b_xor_cluster_0));

    defparam LUT4_14.initval =  16'h6996 ;
    ROM16X1A LUT4_14 (.AD3(w_gcount_r25), .AD2(w_gcount_r26), .AD1(scuba_vlo), 
        .AD0(scuba_vlo), .DO0(wcount_r5));

    defparam LUT4_13.initval =  16'h6996 ;
    ROM16X1A LUT4_13 (.AD3(w_gcount_r24), .AD2(w_gcount_r25), .AD1(w_gcount_r26), 
        .AD0(scuba_vlo), .DO0(wcount_r4));

    defparam LUT4_12.initval =  16'h6996 ;
    ROM16X1A LUT4_12 (.AD3(w_gcount_r22), .AD2(w_gcount_r23), .AD1(w_gcount_r24), 
        .AD0(wcount_r5), .DO0(wcount_r2));

    defparam LUT4_11.initval =  16'h6996 ;
    ROM16X1A LUT4_11 (.AD3(w_gcount_r21), .AD2(w_gcount_r22), .AD1(w_gcount_r23), 
        .AD0(wcount_r4), .DO0(wcount_r1));

    defparam LUT4_10.initval =  16'h6996 ;
    ROM16X1A LUT4_10 (.AD3(w_gcount_r20), .AD2(w_gcount_r21), .AD1(w_gcount_r22), 
        .AD0(w_g2b_xor_cluster_0), .DO0(wcount_r0));

    defparam LUT4_9.initval =  16'h6996 ;
    ROM16X1A LUT4_9 (.AD3(r_gcount_w23), .AD2(r_gcount_w24), .AD1(r_gcount_w25), 
        .AD0(r_gcount_w26), .DO0(r_g2b_xor_cluster_0));

    defparam LUT4_8.initval =  16'h6996 ;
    ROM16X1A LUT4_8 (.AD3(r_gcount_w25), .AD2(r_gcount_w26), .AD1(scuba_vlo), 
        .AD0(scuba_vlo), .DO0(rcount_w5));

    defparam LUT4_7.initval =  16'h6996 ;
    ROM16X1A LUT4_7 (.AD3(r_gcount_w24), .AD2(r_gcount_w25), .AD1(r_gcount_w26), 
        .AD0(scuba_vlo), .DO0(rcount_w4));

    defparam LUT4_6.initval =  16'h6996 ;
    ROM16X1A LUT4_6 (.AD3(r_gcount_w22), .AD2(r_gcount_w23), .AD1(r_gcount_w24), 
        .AD0(rcount_w5), .DO0(rcount_w2));

    defparam LUT4_5.initval =  16'h6996 ;
    ROM16X1A LUT4_5 (.AD3(r_gcount_w21), .AD2(r_gcount_w22), .AD1(r_gcount_w23), 
        .AD0(rcount_w4), .DO0(rcount_w1));

    defparam LUT4_4.initval =  16'h6996 ;
    ROM16X1A LUT4_4 (.AD3(r_gcount_w20), .AD2(r_gcount_w21), .AD1(r_gcount_w22), 
        .AD0(r_g2b_xor_cluster_0), .DO0(rcount_w0));

    defparam LUT4_3.initval =  16'h0410 ;
    ROM16X1A LUT4_3 (.AD3(rptr_6), .AD2(rcount_6), .AD1(w_gcount_r26), .AD0(scuba_vlo), 
        .DO0(empty_cmp_set));

    defparam LUT4_2.initval =  16'h1004 ;
    ROM16X1A LUT4_2 (.AD3(rptr_6), .AD2(rcount_6), .AD1(w_gcount_r26), .AD0(scuba_vlo), 
        .DO0(empty_cmp_clr));

    defparam LUT4_1.initval =  16'h0140 ;
    ROM16X1A LUT4_1 (.AD3(wptr_6), .AD2(wcount_6), .AD1(r_gcount_w26), .AD0(scuba_vlo), 
        .DO0(full_cmp_set));

    defparam LUT4_0.initval =  16'h4001 ;
    ROM16X1A LUT4_0 (.AD3(wptr_6), .AD2(wcount_6), .AD1(r_gcount_w26), .AD0(scuba_vlo), 
        .DO0(full_cmp_clr));

    defparam pdp_ram_0_0_2.CSDECODE_R = "0b000" ;
    defparam pdp_ram_0_0_2.CSDECODE_W = "0b001" ;
    defparam pdp_ram_0_0_2.GSR = "DISABLED" ;
    defparam pdp_ram_0_0_2.REGMODE = "NOREG" ;
    defparam pdp_ram_0_0_2.DATA_WIDTH_R = 36 ;
    defparam pdp_ram_0_0_2.DATA_WIDTH_W = 36 ;
    PDPW16KC pdp_ram_0_0_2 (.DI0(Data[0]), .DI1(Data[1]), .DI2(Data[2]), 
        .DI3(Data[3]), .DI4(Data[4]), .DI5(Data[5]), .DI6(Data[6]), .DI7(Data[7]), 
        .DI8(Data[8]), .DI9(Data[9]), .DI10(Data[10]), .DI11(Data[11]), 
        .DI12(Data[12]), .DI13(Data[13]), .DI14(Data[14]), .DI15(Data[15]), 
        .DI16(Data[16]), .DI17(Data[17]), .DI18(Data[18]), .DI19(Data[19]), 
        .DI20(Data[20]), .DI21(Data[21]), .DI22(Data[22]), .DI23(Data[23]), 
        .DI24(Data[24]), .DI25(Data[25]), .DI26(Data[26]), .DI27(Data[27]), 
        .DI28(Data[28]), .DI29(Data[29]), .DI30(Data[30]), .DI31(Data[31]), 
        .DI32(Data[32]), .DI33(Data[33]), .DI34(Data[34]), .DI35(Data[35]), 
        .ADW0(wptr_0), .ADW1(wptr_1), .ADW2(wptr_2), .ADW3(wptr_3), .ADW4(wptr_4), 
        .ADW5(wptr_5), .ADW6(scuba_vlo), .ADW7(scuba_vlo), .ADW8(scuba_vlo), 
        .BE0(scuba_vhi), .BE1(scuba_vhi), .BE2(scuba_vhi), .BE3(scuba_vhi), 
        .CEW(wren_i), .CLKW(WrClock), .CSW0(scuba_vhi), .CSW1(scuba_vlo), 
        .CSW2(scuba_vlo), .ADR0(scuba_vlo), .ADR1(scuba_vlo), .ADR2(scuba_vlo), 
        .ADR3(scuba_vlo), .ADR4(scuba_vlo), .ADR5(rptr_0), .ADR6(rptr_1), 
        .ADR7(rptr_2), .ADR8(rptr_3), .ADR9(rptr_4), .ADR10(rptr_5), .ADR11(scuba_vlo), 
        .ADR12(scuba_vlo), .ADR13(scuba_vlo), .CER(rden_i), .CLKR(RdClock), 
        .CSR0(scuba_vlo), .CSR1(scuba_vlo), .CSR2(scuba_vlo), .RST(Reset), 
        .DO0(Q[18]), .DO1(Q[19]), .DO2(Q[20]), .DO3(Q[21]), .DO4(Q[22]), 
        .DO5(Q[23]), .DO6(Q[24]), .DO7(Q[25]), .DO8(Q[26]), .DO9(Q[27]), 
        .DO10(Q[28]), .DO11(Q[29]), .DO12(Q[30]), .DO13(Q[31]), .DO14(Q[32]), 
        .DO15(Q[33]), .DO16(Q[34]), .DO17(Q[35]), .DO18(Q[0]), .DO19(Q[1]), 
        .DO20(Q[2]), .DO21(Q[3]), .DO22(Q[4]), .DO23(Q[5]), .DO24(Q[6]), 
        .DO25(Q[7]), .DO26(Q[8]), .DO27(Q[9]), .DO28(Q[10]), .DO29(Q[11]), 
        .DO30(Q[12]), .DO31(Q[13]), .DO32(Q[14]), .DO33(Q[15]), .DO34(Q[16]), 
        .DO35(Q[17]))
             /* synthesis MEM_LPC_FILE="fifo_down.lpc" */
             /* synthesis MEM_INIT_FILE="" */
             /* synthesis RESETMODE="SYNC" */;

    defparam pdp_ram_0_1_1.CSDECODE_R = "0b000" ;
    defparam pdp_ram_0_1_1.CSDECODE_W = "0b001" ;
    defparam pdp_ram_0_1_1.GSR = "DISABLED" ;
    defparam pdp_ram_0_1_1.REGMODE = "NOREG" ;
    defparam pdp_ram_0_1_1.DATA_WIDTH_R = 36 ;
    defparam pdp_ram_0_1_1.DATA_WIDTH_W = 36 ;
    PDPW16KC pdp_ram_0_1_1 (.DI0(Data[36]), .DI1(Data[37]), .DI2(Data[38]), 
        .DI3(Data[39]), .DI4(Data[40]), .DI5(Data[41]), .DI6(Data[42]), 
        .DI7(Data[43]), .DI8(Data[44]), .DI9(Data[45]), .DI10(Data[46]), 
        .DI11(Data[47]), .DI12(Data[48]), .DI13(Data[49]), .DI14(Data[50]), 
        .DI15(Data[51]), .DI16(Data[52]), .DI17(Data[53]), .DI18(Data[54]), 
        .DI19(Data[55]), .DI20(Data[56]), .DI21(Data[57]), .DI22(Data[58]), 
        .DI23(Data[59]), .DI24(Data[60]), .DI25(Data[61]), .DI26(Data[62]), 
        .DI27(Data[63]), .DI28(Data[64]), .DI29(Data[65]), .DI30(Data[66]), 
        .DI31(Data[67]), .DI32(Data[68]), .DI33(Data[69]), .DI34(Data[70]), 
        .DI35(Data[71]), .ADW0(wptr_0), .ADW1(wptr_1), .ADW2(wptr_2), .ADW3(wptr_3), 
        .ADW4(wptr_4), .ADW5(wptr_5), .ADW6(scuba_vlo), .ADW7(scuba_vlo), 
        .ADW8(scuba_vlo), .BE0(scuba_vhi), .BE1(scuba_vhi), .BE2(scuba_vhi), 
        .BE3(scuba_vhi), .CEW(wren_i), .CLKW(WrClock), .CSW0(scuba_vhi), 
        .CSW1(scuba_vlo), .CSW2(scuba_vlo), .ADR0(scuba_vlo), .ADR1(scuba_vlo), 
        .ADR2(scuba_vlo), .ADR3(scuba_vlo), .ADR4(scuba_vlo), .ADR5(rptr_0), 
        .ADR6(rptr_1), .ADR7(rptr_2), .ADR8(rptr_3), .ADR9(rptr_4), .ADR10(rptr_5), 
        .ADR11(scuba_vlo), .ADR12(scuba_vlo), .ADR13(scuba_vlo), .CER(rden_i), 
        .CLKR(RdClock), .CSR0(scuba_vlo), .CSR1(scuba_vlo), .CSR2(scuba_vlo), 
        .RST(Reset), .DO0(Q[54]), .DO1(Q[55]), .DO2(Q[56]), .DO3(Q[57]), 
        .DO4(Q[58]), .DO5(Q[59]), .DO6(Q[60]), .DO7(Q[61]), .DO8(Q[62]), 
        .DO9(Q[63]), .DO10(Q[64]), .DO11(Q[65]), .DO12(Q[66]), .DO13(Q[67]), 
        .DO14(Q[68]), .DO15(Q[69]), .DO16(Q[70]), .DO17(Q[71]), .DO18(Q[36]), 
        .DO19(Q[37]), .DO20(Q[38]), .DO21(Q[39]), .DO22(Q[40]), .DO23(Q[41]), 
        .DO24(Q[42]), .DO25(Q[43]), .DO26(Q[44]), .DO27(Q[45]), .DO28(Q[46]), 
        .DO29(Q[47]), .DO30(Q[48]), .DO31(Q[49]), .DO32(Q[50]), .DO33(Q[51]), 
        .DO34(Q[52]), .DO35(Q[53]))
             /* synthesis MEM_LPC_FILE="fifo_down.lpc" */
             /* synthesis MEM_INIT_FILE="" */
             /* synthesis RESETMODE="SYNC" */;

    defparam pdp_ram_0_2_0.CSDECODE_R = "0b000" ;
    defparam pdp_ram_0_2_0.CSDECODE_W = "0b001" ;
    defparam pdp_ram_0_2_0.GSR = "DISABLED" ;
    defparam pdp_ram_0_2_0.REGMODE = "NOREG" ;
    defparam pdp_ram_0_2_0.DATA_WIDTH_R = 36 ;
    defparam pdp_ram_0_2_0.DATA_WIDTH_W = 36 ;
    PDPW16KC pdp_ram_0_2_0 (.DI0(Data[72]), .DI1(Data[73]), .DI2(Data[74]), 
        .DI3(Data[75]), .DI4(Data[76]), .DI5(Data[77]), .DI6(Data[78]), 
        .DI7(Data[79]), .DI8(Data[80]), .DI9(Data[81]), .DI10(Data[82]), 
        .DI11(Data[83]), .DI12(Data[84]), .DI13(Data[85]), .DI14(Data[86]), 
        .DI15(Data[87]), .DI16(Data[88]), .DI17(Data[89]), .DI18(Data[90]), 
        .DI19(Data[91]), .DI20(Data[92]), .DI21(Data[93]), .DI22(Data[94]), 
        .DI23(Data[95]), .DI24(Data[96]), .DI25(Data[97]), .DI26(Data[98]), 
        .DI27(scuba_vlo), .DI28(scuba_vlo), .DI29(scuba_vlo), .DI30(scuba_vlo), 
        .DI31(scuba_vlo), .DI32(scuba_vlo), .DI33(scuba_vlo), .DI34(scuba_vlo), 
        .DI35(scuba_vlo), .ADW0(wptr_0), .ADW1(wptr_1), .ADW2(wptr_2), .ADW3(wptr_3), 
        .ADW4(wptr_4), .ADW5(wptr_5), .ADW6(scuba_vlo), .ADW7(scuba_vlo), 
        .ADW8(scuba_vlo), .BE0(scuba_vhi), .BE1(scuba_vhi), .BE2(scuba_vhi), 
        .BE3(scuba_vhi), .CEW(wren_i), .CLKW(WrClock), .CSW0(scuba_vhi), 
        .CSW1(scuba_vlo), .CSW2(scuba_vlo), .ADR0(scuba_vlo), .ADR1(scuba_vlo), 
        .ADR2(scuba_vlo), .ADR3(scuba_vlo), .ADR4(scuba_vlo), .ADR5(rptr_0), 
        .ADR6(rptr_1), .ADR7(rptr_2), .ADR8(rptr_3), .ADR9(rptr_4), .ADR10(rptr_5), 
        .ADR11(scuba_vlo), .ADR12(scuba_vlo), .ADR13(scuba_vlo), .CER(rden_i), 
        .CLKR(RdClock), .CSR0(scuba_vlo), .CSR1(scuba_vlo), .CSR2(scuba_vlo), 
        .RST(Reset), .DO0(Q[90]), .DO1(Q[91]), .DO2(Q[92]), .DO3(Q[93]), 
        .DO4(Q[94]), .DO5(Q[95]), .DO6(Q[96]), .DO7(Q[97]), .DO8(Q[98]), 
        .DO9(), .DO10(), .DO11(), .DO12(), .DO13(), .DO14(), .DO15(), .DO16(), 
        .DO17(), .DO18(Q[72]), .DO19(Q[73]), .DO20(Q[74]), .DO21(Q[75]), 
        .DO22(Q[76]), .DO23(Q[77]), .DO24(Q[78]), .DO25(Q[79]), .DO26(Q[80]), 
        .DO27(Q[81]), .DO28(Q[82]), .DO29(Q[83]), .DO30(Q[84]), .DO31(Q[85]), 
        .DO32(Q[86]), .DO33(Q[87]), .DO34(Q[88]), .DO35(Q[89]))
             /* synthesis MEM_LPC_FILE="fifo_down.lpc" */
             /* synthesis MEM_INIT_FILE="" */
             /* synthesis RESETMODE="SYNC" */;

    FD1P3BX FF_71 (.D(iwcount_0), .SP(wren_i), .CK(WrClock), .PD(Reset), 
        .Q(wcount_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_70 (.D(iwcount_1), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_69 (.D(iwcount_2), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_68 (.D(iwcount_3), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_67 (.D(iwcount_4), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_66 (.D(iwcount_5), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_65 (.D(iwcount_6), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wcount_6))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_64 (.D(w_gdata_0), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_63 (.D(w_gdata_1), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_62 (.D(w_gdata_2), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_61 (.D(w_gdata_3), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_60 (.D(w_gdata_4), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_59 (.D(w_gdata_5), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_58 (.D(wcount_6), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(w_gcount_6))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_57 (.D(wcount_0), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_56 (.D(wcount_1), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_55 (.D(wcount_2), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_54 (.D(wcount_3), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_53 (.D(wcount_4), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_52 (.D(wcount_5), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_51 (.D(wcount_6), .SP(wren_i), .CK(WrClock), .CD(Reset), 
        .Q(wptr_6))
             /* synthesis GSR="ENABLED" */;

    FD1P3BX FF_50 (.D(ircount_0), .SP(rden_i), .CK(RdClock), .PD(rRst), 
        .Q(rcount_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_49 (.D(ircount_1), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_48 (.D(ircount_2), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_47 (.D(ircount_3), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_46 (.D(ircount_4), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_45 (.D(ircount_5), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_44 (.D(ircount_6), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(rcount_6))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_43 (.D(r_gdata_0), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_42 (.D(r_gdata_1), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_41 (.D(r_gdata_2), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_40 (.D(r_gdata_3), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_39 (.D(r_gdata_4), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_38 (.D(r_gdata_5), .SP(rden_i), .CK(RdClock), .CD(rRst), 
        .Q(r_gcount_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_37 (.D(rcount_6), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(r_gcount_6))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_36 (.D(rcount_0), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_0))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_35 (.D(rcount_1), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_1))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_34 (.D(rcount_2), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_2))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_33 (.D(rcount_3), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_3))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_32 (.D(rcount_4), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_4))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_31 (.D(rcount_5), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_5))
             /* synthesis GSR="ENABLED" */;

    FD1P3DX FF_30 (.D(rcount_6), .SP(rden_i), .CK(RdClock), .CD(rRst), .Q(rptr_6))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_29 (.D(w_gcount_0), .CK(RdClock), .CD(Reset), .Q(w_gcount_r0))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_28 (.D(w_gcount_1), .CK(RdClock), .CD(Reset), .Q(w_gcount_r1))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_27 (.D(w_gcount_2), .CK(RdClock), .CD(Reset), .Q(w_gcount_r2))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_26 (.D(w_gcount_3), .CK(RdClock), .CD(Reset), .Q(w_gcount_r3))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_25 (.D(w_gcount_4), .CK(RdClock), .CD(Reset), .Q(w_gcount_r4))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_24 (.D(w_gcount_5), .CK(RdClock), .CD(Reset), .Q(w_gcount_r5))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_23 (.D(w_gcount_6), .CK(RdClock), .CD(Reset), .Q(w_gcount_r6))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_22 (.D(r_gcount_0), .CK(WrClock), .CD(rRst), .Q(r_gcount_w0))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_21 (.D(r_gcount_1), .CK(WrClock), .CD(rRst), .Q(r_gcount_w1))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_20 (.D(r_gcount_2), .CK(WrClock), .CD(rRst), .Q(r_gcount_w2))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_19 (.D(r_gcount_3), .CK(WrClock), .CD(rRst), .Q(r_gcount_w3))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_18 (.D(r_gcount_4), .CK(WrClock), .CD(rRst), .Q(r_gcount_w4))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_17 (.D(r_gcount_5), .CK(WrClock), .CD(rRst), .Q(r_gcount_w5))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_16 (.D(r_gcount_6), .CK(WrClock), .CD(rRst), .Q(r_gcount_w6))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_15 (.D(w_gcount_r0), .CK(RdClock), .CD(Reset), .Q(w_gcount_r20))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_14 (.D(w_gcount_r1), .CK(RdClock), .CD(Reset), .Q(w_gcount_r21))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_13 (.D(w_gcount_r2), .CK(RdClock), .CD(Reset), .Q(w_gcount_r22))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_12 (.D(w_gcount_r3), .CK(RdClock), .CD(Reset), .Q(w_gcount_r23))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_11 (.D(w_gcount_r4), .CK(RdClock), .CD(Reset), .Q(w_gcount_r24))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_10 (.D(w_gcount_r5), .CK(RdClock), .CD(Reset), .Q(w_gcount_r25))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_9 (.D(w_gcount_r6), .CK(RdClock), .CD(Reset), .Q(w_gcount_r26))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_8 (.D(r_gcount_w0), .CK(WrClock), .CD(rRst), .Q(r_gcount_w20))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_7 (.D(r_gcount_w1), .CK(WrClock), .CD(rRst), .Q(r_gcount_w21))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_6 (.D(r_gcount_w2), .CK(WrClock), .CD(rRst), .Q(r_gcount_w22))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_5 (.D(r_gcount_w3), .CK(WrClock), .CD(rRst), .Q(r_gcount_w23))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_4 (.D(r_gcount_w4), .CK(WrClock), .CD(rRst), .Q(r_gcount_w24))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_3 (.D(r_gcount_w5), .CK(WrClock), .CD(rRst), .Q(r_gcount_w25))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_2 (.D(r_gcount_w6), .CK(WrClock), .CD(rRst), .Q(r_gcount_w26))
             /* synthesis GSR="ENABLED" */;

    FD1S3BX FF_1 (.D(empty_d), .CK(RdClock), .PD(rRst), .Q(empty_i))
             /* synthesis GSR="ENABLED" */;

    FD1S3DX FF_0 (.D(full_d), .CK(WrClock), .CD(Reset), .Q(full_i))
             /* synthesis GSR="ENABLED" */;

    FADD2B w_gctr_cia (.A0(scuba_vlo), .A1(scuba_vhi), .B0(scuba_vlo), .B1(scuba_vhi), 
        .CI(scuba_vlo), .COUT(w_gctr_ci), .S0(), .S1());

    CU2 w_gctr_0 (.CI(w_gctr_ci), .PC0(wcount_0), .PC1(wcount_1), .CO(co0), 
        .NC0(iwcount_0), .NC1(iwcount_1));

    CU2 w_gctr_1 (.CI(co0), .PC0(wcount_2), .PC1(wcount_3), .CO(co1), .NC0(iwcount_2), 
        .NC1(iwcount_3));

    CU2 w_gctr_2 (.CI(co1), .PC0(wcount_4), .PC1(wcount_5), .CO(co2), .NC0(iwcount_4), 
        .NC1(iwcount_5));

    CU2 w_gctr_3 (.CI(co2), .PC0(wcount_6), .PC1(scuba_vlo), .CO(co3), .NC0(iwcount_6), 
        .NC1());

    VHI scuba_vhi_inst (.Z(scuba_vhi));

    FADD2B r_gctr_cia (.A0(scuba_vlo), .A1(scuba_vhi), .B0(scuba_vlo), .B1(scuba_vhi), 
        .CI(scuba_vlo), .COUT(r_gctr_ci), .S0(), .S1());

    CU2 r_gctr_0 (.CI(r_gctr_ci), .PC0(rcount_0), .PC1(rcount_1), .CO(co0_1), 
        .NC0(ircount_0), .NC1(ircount_1));

    CU2 r_gctr_1 (.CI(co0_1), .PC0(rcount_2), .PC1(rcount_3), .CO(co1_1), 
        .NC0(ircount_2), .NC1(ircount_3));

    CU2 r_gctr_2 (.CI(co1_1), .PC0(rcount_4), .PC1(rcount_5), .CO(co2_1), 
        .NC0(ircount_4), .NC1(ircount_5));

    CU2 r_gctr_3 (.CI(co2_1), .PC0(rcount_6), .PC1(scuba_vlo), .CO(co3_1), 
        .NC0(ircount_6), .NC1());

    FADD2B empty_cmp_ci_a (.A0(scuba_vlo), .A1(rden_i), .B0(scuba_vlo), 
        .B1(rden_i), .CI(scuba_vlo), .COUT(cmp_ci), .S0(), .S1());

    AGEB2 empty_cmp_0 (.A0(rcount_0), .A1(rcount_1), .B0(wcount_r0), .B1(wcount_r1), 
        .CI(cmp_ci), .GE(co0_2));

    AGEB2 empty_cmp_1 (.A0(rcount_2), .A1(rcount_3), .B0(wcount_r2), .B1(w_g2b_xor_cluster_0), 
        .CI(co0_2), .GE(co1_2));

    AGEB2 empty_cmp_2 (.A0(rcount_4), .A1(rcount_5), .B0(wcount_r4), .B1(wcount_r5), 
        .CI(co1_2), .GE(co2_2));

    AGEB2 empty_cmp_3 (.A0(empty_cmp_set), .A1(scuba_vlo), .B0(empty_cmp_clr), 
        .B1(scuba_vlo), .CI(co2_2), .GE(empty_d_c));

    FADD2B a0 (.A0(scuba_vlo), .A1(scuba_vlo), .B0(scuba_vlo), .B1(scuba_vlo), 
        .CI(empty_d_c), .COUT(), .S0(empty_d), .S1());

    FADD2B full_cmp_ci_a (.A0(scuba_vlo), .A1(wren_i), .B0(scuba_vlo), .B1(wren_i), 
        .CI(scuba_vlo), .COUT(cmp_ci_1), .S0(), .S1());

    AGEB2 full_cmp_0 (.A0(wcount_0), .A1(wcount_1), .B0(rcount_w0), .B1(rcount_w1), 
        .CI(cmp_ci_1), .GE(co0_3));

    AGEB2 full_cmp_1 (.A0(wcount_2), .A1(wcount_3), .B0(rcount_w2), .B1(r_g2b_xor_cluster_0), 
        .CI(co0_3), .GE(co1_3));

    AGEB2 full_cmp_2 (.A0(wcount_4), .A1(wcount_5), .B0(rcount_w4), .B1(rcount_w5), 
        .CI(co1_3), .GE(co2_3));

    AGEB2 full_cmp_3 (.A0(full_cmp_set), .A1(scuba_vlo), .B0(full_cmp_clr), 
        .B1(scuba_vlo), .CI(co2_3), .GE(full_d_c));

    VLO scuba_vlo_inst (.Z(scuba_vlo));

    FADD2B a1 (.A0(scuba_vlo), .A1(scuba_vlo), .B0(scuba_vlo), .B1(scuba_vlo), 
        .CI(full_d_c), .COUT(), .S0(full_d), .S1());

    assign Empty = empty_i;
    assign Full = full_i;
//    GSR GSR_INST (.GSR(1'b1));
//    PUR PUR_INST (.PUR(1'b1));

    // exemplar begin
    // exemplar attribute pdp_ram_0_0_2 MEM_LPC_FILE fifo_down.lpc
    // exemplar attribute pdp_ram_0_0_2 MEM_INIT_FILE 
    // exemplar attribute pdp_ram_0_0_2 RESETMODE SYNC
    // exemplar attribute pdp_ram_0_1_1 MEM_LPC_FILE fifo_down.lpc
    // exemplar attribute pdp_ram_0_1_1 MEM_INIT_FILE 
    // exemplar attribute pdp_ram_0_1_1 RESETMODE SYNC
    // exemplar attribute pdp_ram_0_2_0 MEM_LPC_FILE fifo_down.lpc
    // exemplar attribute pdp_ram_0_2_0 MEM_INIT_FILE 
    // exemplar attribute pdp_ram_0_2_0 RESETMODE SYNC
    // exemplar attribute FF_71 GSR ENABLED
    // exemplar attribute FF_70 GSR ENABLED
    // exemplar attribute FF_69 GSR ENABLED
    // exemplar attribute FF_68 GSR ENABLED
    // exemplar attribute FF_67 GSR ENABLED
    // exemplar attribute FF_66 GSR ENABLED
    // exemplar attribute FF_65 GSR ENABLED
    // exemplar attribute FF_64 GSR ENABLED
    // exemplar attribute FF_63 GSR ENABLED
    // exemplar attribute FF_62 GSR ENABLED
    // exemplar attribute FF_61 GSR ENABLED
    // exemplar attribute FF_60 GSR ENABLED
    // exemplar attribute FF_59 GSR ENABLED
    // exemplar attribute FF_58 GSR ENABLED
    // exemplar attribute FF_57 GSR ENABLED
    // exemplar attribute FF_56 GSR ENABLED
    // exemplar attribute FF_55 GSR ENABLED
    // exemplar attribute FF_54 GSR ENABLED
    // exemplar attribute FF_53 GSR ENABLED
    // exemplar attribute FF_52 GSR ENABLED
    // exemplar attribute FF_51 GSR ENABLED
    // exemplar attribute FF_50 GSR ENABLED
    // exemplar attribute FF_49 GSR ENABLED
    // exemplar attribute FF_48 GSR ENABLED
    // exemplar attribute FF_47 GSR ENABLED
    // exemplar attribute FF_46 GSR ENABLED
    // exemplar attribute FF_45 GSR ENABLED
    // exemplar attribute FF_44 GSR ENABLED
    // exemplar attribute FF_43 GSR ENABLED
    // exemplar attribute FF_42 GSR ENABLED
    // exemplar attribute FF_41 GSR ENABLED
    // exemplar attribute FF_40 GSR ENABLED
    // exemplar attribute FF_39 GSR ENABLED
    // exemplar attribute FF_38 GSR ENABLED
    // exemplar attribute FF_37 GSR ENABLED
    // exemplar attribute FF_36 GSR ENABLED
    // exemplar attribute FF_35 GSR ENABLED
    // exemplar attribute FF_34 GSR ENABLED
    // exemplar attribute FF_33 GSR ENABLED
    // exemplar attribute FF_32 GSR ENABLED
    // exemplar attribute FF_31 GSR ENABLED
    // exemplar attribute FF_30 GSR ENABLED
    // exemplar attribute FF_29 GSR ENABLED
    // exemplar attribute FF_28 GSR ENABLED
    // exemplar attribute FF_27 GSR ENABLED
    // exemplar attribute FF_26 GSR ENABLED
    // exemplar attribute FF_25 GSR ENABLED
    // exemplar attribute FF_24 GSR ENABLED
    // exemplar attribute FF_23 GSR ENABLED
    // exemplar attribute FF_22 GSR ENABLED
    // exemplar attribute FF_21 GSR ENABLED
    // exemplar attribute FF_20 GSR ENABLED
    // exemplar attribute FF_19 GSR ENABLED
    // exemplar attribute FF_18 GSR ENABLED
    // exemplar attribute FF_17 GSR ENABLED
    // exemplar attribute FF_16 GSR ENABLED
    // exemplar attribute FF_15 GSR ENABLED
    // exemplar attribute FF_14 GSR ENABLED
    // exemplar attribute FF_13 GSR ENABLED
    // exemplar attribute FF_12 GSR ENABLED
    // exemplar attribute FF_11 GSR ENABLED
    // exemplar attribute FF_10 GSR ENABLED
    // exemplar attribute FF_9 GSR ENABLED
    // exemplar attribute FF_8 GSR ENABLED
    // exemplar attribute FF_7 GSR ENABLED
    // exemplar attribute FF_6 GSR ENABLED
    // exemplar attribute FF_5 GSR ENABLED
    // exemplar attribute FF_4 GSR ENABLED
    // exemplar attribute FF_3 GSR ENABLED
    // exemplar attribute FF_2 GSR ENABLED
    // exemplar attribute FF_1 GSR ENABLED
    // exemplar attribute FF_0 GSR ENABLED
    // exemplar end

endmodule
