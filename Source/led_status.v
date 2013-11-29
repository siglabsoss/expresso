// $Id: led_status.v,v 1.2 2008/07/02 19:10:29 jfreed Exp $

module led_status(clk, rstn, invert,
        lock, ltssm_state, dl_up_in, bar1_hit,
        pll_lk, poll, l0, dl_up_out, usr0, usr1, usr2, usr3,
        na_pll_lk, na_poll, na_l0, na_dl_up_out, na_usr0, na_usr1, na_usr2, na_usr3,
        dpn
);

input clk;
input rstn;
input invert;
input lock;
input [3:0] ltssm_state;
input dl_up_in;
input bar1_hit;
output  pll_lk, poll, l0, dl_up_out, usr0, usr1, usr2, usr3;
output  na_pll_lk, na_poll, na_l0, na_dl_up_out, na_usr0, na_usr1, na_usr2, na_usr3;


output dpn;

reg poll_i;
reg l0_i;
reg dl_up_i;
reg dp;
reg [25:0] c;

assign pll_lk = invert ? ~lock : lock;
assign poll = invert ? ~poll_i : poll_i;
assign l0 = invert ? ~l0_i : l0_i;
assign dl_up_out = invert ? ~dl_up_i : dl_up_i;
assign usr0 = invert ? 1'b1 : 1'b0;
assign usr1 = invert ? 1'b1 : 1'b0;
assign usr2 = invert ? 1'b1 : 1'b0;
assign usr3 = invert ? 1'b1 : 1'b0;

assign na_pll_lk = invert ? 1'b1 : 1'b0;
assign na_poll = invert ? 1'b1 : 1'b0;
assign na_l0 = invert ? 1'b1 : 1'b0;
assign na_dl_up_out = invert ? 1'b1 : 1'b0;
assign na_usr0 = invert ? 1'b1 : 1'b0;
assign na_usr1 = invert ? 1'b1 : 1'b0;
assign na_usr2 = invert ? 1'b1 : 1'b0;
assign na_usr3 = invert ? 1'b1 : 1'b0;

assign dpn = ~dp;


always @(posedge clk or negedge rstn)
begin
  if (~rstn)
  begin
    poll_i <= 1'b0;
    l0_i <= 1'b0;
    dl_up_i <= 1'b0;  
    dp <= 1'b0;
    c <= 26'd0;
  end
  else
  begin    
    if (ltssm_state == 4'b0001) // latch Polling
      poll_i <= 1'b1;
      
    l0_i <= (ltssm_state == 4'b0011); // L0
    dl_up_i <= dl_up_in;
    
    if (bar1_hit && ~dp)
    begin
      dp <= 1'b1;
      c <= 26'd1;
    end
    else if (c==26'd0)
      dp <= 1'b0;
    else
      c <= c + 1;
    

  end
end

endmodule

