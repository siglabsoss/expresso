// $Id: sfif_ctrl.v,v 1.1.1.1 2008/07/01 17:34:22 jfreed Exp $

module sfif_ctrl(clk_125, rstn, 
                 rprst, enable, run, ipg_cnt, tx_cycles, loop,
                 tx_empty, tx_rdy, tx_val, tx_end, credit_available,
                 tx_cr_read, tx_d_read, done, sm
              
);                 
                 
input clk_125;
input rstn;
input enable;
input run;
input [15:0] ipg_cnt;
input [15:0] tx_cycles;
input loop;
input tx_empty;
input tx_rdy;
input tx_val;
input tx_end;
input credit_available;

output rprst;
output tx_cr_read, tx_d_read;
output done;
output [2:0] sm;

reg rprst;
reg tx_cr_read, tx_d_read;
reg [15:0] ipg;
reg [15:0] cycles;
reg tx_end_p;

reg [2:0] sm;
parameter IDLE = 3'b000,          
          CREDIT = 3'b010,
          SEND = 3'b011,
          RESET = 3'b101,
          DONE = 3'b110;          
          
assign done = (sm==DONE);          

always @(posedge clk_125 or negedge rstn)
begin
  if (~rstn)
  begin
    rprst <= 1'b0;
    sm <= IDLE;  
    tx_cr_read <= 1'b0;
    tx_d_read <= 1'b0;
    ipg <= 16'd0;
    cycles <= 16'd0;
    tx_end_p <= 1'b0;
  end
  else
  begin
    tx_end_p <= tx_end;    
    
    case (sm)    
    IDLE:
    begin
      if (enable && run && ~tx_empty)      
      begin
        sm <= CREDIT;                          
        tx_cr_read <= 1'b1;                
        cycles <= tx_cycles;
      end
    end    
    CREDIT:
    begin
      tx_cr_read <= 1'b0;
      if (credit_available)
      begin
        tx_d_read <= 1'b1;
        sm <= SEND;
      end      
    end
    SEND:
    begin
      if (tx_rdy) // the packet is being sent
      begin
        tx_d_read <= 1'b1; // this is only a gate to read data
      end        
      else if (tx_end && ~tx_empty && tx_val) // go send next TLP
      begin
        tx_d_read <= 1'b1;
        sm <= SEND;
      end
      else if (tx_end && tx_empty && tx_val) // all TLPs sent
      begin
        sm <= RESET;
        rprst <= 1'b1;
        tx_d_read <= 1'b0;
        ipg <= ipg_cnt;
      end      
    end    
    RESET:
    begin
      if (ipg>16'd0)
        ipg <= ipg - 1;
      else if (loop || (cycles>16'd1))
      begin
        if (rprst)
          rprst <= 1'b0;
        else
        begin
          rprst <= 1'b0;
          tx_cr_read <= 1'b1;
          sm <= CREDIT;
          if (cycles>16'd1) cycles <= cycles - 1;
        end
      end
      else
        sm <= DONE;
    end          
    DONE:
    begin
      sm <= DONE;    
    end
    default:
    begin      
    end    
    endcase
  
  end
end

endmodule


