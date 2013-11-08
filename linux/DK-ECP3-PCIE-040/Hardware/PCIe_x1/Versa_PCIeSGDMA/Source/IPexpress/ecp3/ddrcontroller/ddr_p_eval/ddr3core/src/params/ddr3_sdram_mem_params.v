`define ECP3_8_0

`define sg25E 

`define DATA_WIDTH 16
`define DATA_SIZE_16
`define x8
`define DQS_WIDTH 2
`define DQS_WIDTH_2
`define DMSIZE 2

`define BRST_CNT_EN 
`define DSIZE   `DATA_WIDTH*4
`define USER_DM `DSIZE/8
`define DQSW 8
`define ROW_WIDTH 13
`define ROW_WIDTH_EQ_13 

`define COL_WIDTH 10
`define COL_WIDTH_EQ_10

`define CS_WIDTH 1
`define CS_WIDTH_1 

`ifdef RDIMM
`define CS_WIDTH_BB 2
`define TSTAB 11'd1200
`define DESEL
`else
`define CS_WIDTH_BB `CS_WIDTH
`endif
`define BANK_WIDTH 3
`define BSIZE 3
`define ADDR_WIDTH 26

`define USER_SLOT_SIZE_1
`define WL_DQS_WIDTH (`DQS_WIDTH * `CS_WIDTH)
`define CLKO_WIDTH 1
`define CLKO_WIDTH_1 
`define CKE_WIDTH 1

`define AR_BURST_EN 3'd0
`define AR_BURST_8

`define WrRqDDelay 2'd1
`define MRS0_INIT 16'b0001010100010000
`define MRS1_INIT 16'b0000000000000100
`define MRS2_INIT 16'b0000001000000000
`define MRS3_INIT 16'b0000000000000000

`define TRCD     2'd3
`define TRRD     2'd2
`define TRP      2'd3
`define TWR      2'd3
`define TRAS     4'd8
`define TRC      4'd10
`define TRTP     2'd2
`define TWTR     2'd2
`define TCKESR   2'd3
`define TZQOPER  8'd129
`define TZQS     6'd40
`define TMRD     2'd2
`define TPD      2'd2
`define TXPDLL   3'd5
`define TCKE     2'd2
`define TXPR     5'd24
`define TMOD     3'd6
`define TZQINIT  9'd256
`define TRFC     5'd22
`define TFAW     4'd13
`define TREFI    16'd1560
`define TWLMRD   5'd20
`define TWLDQSEN 4'd13
`define TODTH4   2'd2
`define TODTH8   2'd3
`define TWLO     3'd5
`define TRCD_WIDTH     2
`define TRRD_WIDTH     2
`define TRP_WIDTH      2
`define TWR_WIDTH      2
`define TRAS_WIDTH     4
`define TRC_WIDTH      4
`define TRTP_WIDTH     2
`define TWTR_WIDTH     2
`define TCKESR_WIDTH   2
`define TZQOPER_WIDTH  8
`define TZQS_WIDTH     6
`define TMRD_WIDTH     2
`define TPD_WIDTH      2
`define TXPDLL_WIDTH   3
`define TXPR_WIDTH     5
`define TMOD_WIDTH     3
`define TZQINIT_WIDTH  9
`define TRFC_WIDTH     5
`define TFAW_WIDTH     4
`define TCKE_WIDTH     2
`define TREFI_WIDTH    16
`define TWLMRD_WIDTH   5
`define TWLDQSEN_WIDTH 4
`define TODTH4_WIDTH   2
`define TODTH8_WIDTH   2
`define TWLO_WIDTH     3

`define AL            0
`define CL            3
`define CWL           3
`define CL_WIDTH      4
`define RL_WIDTH      4
`define WL_WIDTH      4
`define AL_WIDTH      2
`define CWL_WIDTH     3

`define NO_WRITE_LEVEL
`define ENB_MEM_RST
 `ifdef x4
`else
  `ifdef NO_WRITE_LEVEL
     `define TRC_DQS0        0
     `define TRC_DQS1        0
     `define TRC_DQS2        0
     `define TRC_DQS3        0
     `define TRC_DQS4        0
     `define TRC_DQS5        0
     `define TRC_DQS6        0
     `define TRC_DQS7        0
     `define TRC_DQS8        0
     `define TRC_DQS9        0
     `define TRC_DQS10       0
     `define TRC_DQS11       0
     `define TRC_DQS12       0
     `define TRC_DQS13       0
     `define TRC_DQS14       0
     `define TRC_DQS15       0
     `define TRC_DQS16       0
     `define TRC_DQS17       0
  `else
     `define TRC_DQS0        100
     `define TRC_DQS1        100
     `define TRC_DQS2        100
     `define TRC_DQS3        100
     `define TRC_DQS4        100
     `define TRC_DQS5        100
     `define TRC_DQS6        100
     `define TRC_DQS7        100
     `define TRC_DQS8        100
     `define TRC_DQS9        100
     `define TRC_DQS10       100
     `define TRC_DQS11       100
     `define TRC_DQS12       100
     `define TRC_DQS13       100
     `define TRC_DQS14       100
     `define TRC_DQS15       100
     `define TRC_DQS16       100
     `define TRC_DQS17       100
  `endif
`endif

 `define PHY_REG
 `define PHY_REG_P2

`ifdef RDIMM
`define READ_PULSE_TAP {`DQS_WIDTH {3'd4}}
`else
`define READ_PULSE_TAP {`DQS_WIDTH {3'd2}}
`endif
`define WL_RTT_NOM 3'b001
`define WL_RNK0_RTT_NOM 3'b010
`define WL_RNK1_RTT_NOM 3'b011
`define DQS_TRC_DEL  0.2
`define DQ_TRC_DEL   0.2
`define DM_TRC_DEL   0.2

`define WL_DQS_PHASE_DLY       0
`define WL_DQS_PHASE_DLY_CNT   8'd0
`define DMY_WIDTH 64
`define DMY_MSK_WIDTH 8
`define LATTICE_MAC
