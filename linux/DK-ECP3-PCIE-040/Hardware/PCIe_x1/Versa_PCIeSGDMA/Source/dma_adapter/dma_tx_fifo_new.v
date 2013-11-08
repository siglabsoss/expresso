// $Id: dma_tx_fifo.v,v 1.1.1.1 2008/07/01 17:34:23 jfreed Exp $

module dma_tx_fifo_new(wb_clk, clk_125, rstn,
                    tx_st_in, tx_end_in, tx_dwen_in, tx_data_in, tx_dv,               
                    tx_st, tx_end, tx_data, 
                    tx_req, tx_rdy,                    
                    
                    full, tx_ca_ph, tx_ca_pd, tx_ca_nph, tx_ca_p_recheck                    
);

input wb_clk;
input clk_125;
input rstn;

input tx_st_in, tx_end_in, tx_dwen_in, tx_dv; 
input [63:0] tx_data_in;

output tx_st, tx_end;
output [15:0] tx_data;
output full;
output tx_req;
input tx_rdy;
input [8:0] tx_ca_ph;
input [12:0] tx_ca_pd;
input [8:0] tx_ca_nph;
input tx_ca_p_recheck;

// TLP FIFO
// Hold the generated TLP to be sent.
// For max TLPs of 128 bytes ~= 26 TLPs
//pmi_fifo_dc #(
//    .pmi_data_width_w(67),
//    .pmi_data_width_r(67),
//    .pmi_data_depth_w(512), // singe EBR
//    .pmi_data_depth_r(512),
//    .pmi_full_flag(512),
//    .pmi_empty_flag(0),
//    .pmi_almost_full_flag(256), //  512-20 (a max TLP is 19)
//    .pmi_almost_empty_flag(2),
//    .pmi_regmode("noreg"),
//    .pmi_resetmode("async"),
//    .pmi_family("SC") ,
//    .module_type("pmi_fifo_dc"),
//    .pmi_implementation("EBR")
//)
//fifo (
//  .Data({tx_dwen_in, tx_st_in, tx_end_in, tx_data_in}),
//  .WrClock(wb_clk),
//  .RdClock(clk_125),
//  .WrEn(fifo_wren),
//  .RdEn(fifo_rden),
//  .Reset(~rstn),
//  .RPReset(1'b0),
//  .Q(fifo_out),
//  .Empty(),
//  .Full(),
//  .AlmostEmpty(empty_fifo),
//  .AlmostFull(full)
//); 
localparam c_BUF_DATA_WIDTH = 67;

wire [c_BUF_DATA_WIDTH-1:0] fifo_out;
reg rd_en;
wire fifo_rden;

async_pkt_fifo #(
.c_DATA_WIDTH  (c_BUF_DATA_WIDTH    ),
.c_ADDR_WIDTH  (9    ),
.c_AFULL_FLAG  (256  ),
.c_AEMPTY_FLAG (0     )
)I_async_pkt_fifo(
.WrEop(tx_end_in),
.Data({tx_dwen_in, tx_st_in, tx_end_in, tx_data_in}), 
.WrClock(wb_clk),
.RdClock(clk_125), 
.WrEn(tx_dv), 
.RdEn(fifo_rden),
.Reset(~rstn),

.Q(fifo_out), 
.Empty(),
.AlmostEmpty(AlmostEmpty), 
.AlmostFull(full)
);

localparam e_idle     = 0;
localparam e_wait     = 1;
localparam e_check_ca = 2;
localparam e_req      = 3;
localparam e_xmit     = 4;

reg [2:0] state;
wire [14:0] tx_ca_pd_ext = {tx_ca_pd, 2'd0};
wire [14:0] req_word;
reg [15:0] tx_data;
reg        tx_st;
reg        tx_end;
wire [63:0] tx_data_i;
reg memory_write;
reg [1:0] cnt;
assign tx_data_i = fifo_out[63:0];
assign tx_end_i  = fifo_out[64];
assign tx_st_i  = fifo_out[65]; 
assign tx_dwen = fifo_out[66];

//assign req_word = tx_data_i[32+9:32+0];
assign req_word = {4'd0, tx_data_i[32+9:32+0], 1'b0};
//assign credit_is_ok = memory_write ? ((tx_ca_ph != 0) && (tx_ca_pd_ext >= req_word)) : (tx_ca_nph != 0);
assign credit_is_ok = memory_write ? ((tx_ca_ph > 1) && (tx_ca_pd_ext >= req_word)) : (tx_ca_nph > 1);
assign tx_req = state == e_req;
assign fifo_rden = rd_en && (~ ((state == e_xmit) && tx_end_i));
assign credit_recheck = memory_write && tx_ca_p_recheck;

always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    rd_en    <= 0;
    state    <= e_idle;
    tx_st    <= 0;
    tx_data  <= 0;
    tx_end   <= 0;
    memory_write <= 0;
    cnt      <= 0;
  end
  else 
  begin
       
    case (state)
       e_idle: begin 
          rd_en    <= 0;
          tx_st    <= 0;       
          tx_data  <= 0;       
          tx_end   <= 0;
          rd_en    <= 0;
          if (~AlmostEmpty)
             state <= e_wait;
       end
       e_wait: begin
          if (tx_st_i) begin
             state <= e_check_ca;
             memory_write <= tx_data_i[63:56] == 8'b0100_0000; //32bit address memory write only
             rd_en <= 1'b0;
          end
          else
             rd_en <= ~ rd_en;
       end
       e_check_ca: begin
          rd_en <= 1'b0;
          cnt   <= 0;
          if (credit_is_ok & (~ credit_recheck))
             state <= e_req;
       end
       e_req: begin
          rd_en <= 1'b0;       
          if (credit_recheck)
             state <= e_check_ca;
          else if (tx_rdy )begin
             state <= e_xmit;
             tx_st <= 1'b1;
             tx_data <= tx_data_i[63:48];
             tx_end <= 1'b0;
             cnt    <=  cnt + 1;
          end
       end
       e_xmit: begin
          cnt <= cnt + 1;
          tx_st <= 1'b0;
          case (cnt)
             2'd0: begin tx_data <= tx_data_i[63:48]; tx_end <= 1'b0; end
             2'd1: begin tx_data <= tx_data_i[47:32]; tx_end <= tx_end_i && tx_dwen; end
             2'd2: begin tx_data <= tx_data_i[31:16]; tx_end <= 1'b0; end
             2'd3: begin tx_data <= tx_data_i[15:0];  tx_end <= tx_end_i && (~ tx_dwen); end
          endcase
          
          if (tx_end_i) begin
             if (tx_dwen) begin
                if (cnt == 1) begin 
                   state <= e_idle;
                   rd_en <= 1'b0;
                end
             end
             else if (cnt == 3) begin
                state <= e_idle;
                rd_en <= 1'b0;
             end
          end
          else
             rd_en <= (cnt == 2);
       end
       default:
          state <= e_idle;
    endcase
  end  
end

endmodule



