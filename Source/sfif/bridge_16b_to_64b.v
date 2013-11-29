//bridge from 16bit TLP data interface to 64bit TLP data interface
module bridge_16b_to_64b(
input rstn,
input clk_125,
input [15:0] rx_data_16b,
input rx_st_16b,
input rx_end_16b,
input rx_us_req_16b,
input rx_malf_tlp_16b,
input [6:0] rx_bar_hit_16b,

output reg [63:0] rx_data_64b,
output reg rx_st_64b,
output reg rx_end_64b,
output reg rx_dwen_64b,
output reg rx_us_req_64b,
output reg rx_malf_tlp_64b,
output reg [6:0] rx_bar_hit_64b

);
localparam c_BUF_ADDR_WIDTH = 9;
localparam c_BUF_DATA_WIDTH = 76;

reg [15:0] rx_data_16b_d1   ;
reg [15:0] rx_data_16b_d2   ;
reg [15:0] rx_data_16b_d3   ;
reg [15:0] rx_data_16b_d4   ;
reg  rx_st_16b_d      ;
reg  rx_end_16b_d     ;
reg  rx_us_req_16b_d  ;
reg  rx_malf_tlp_16b_d;
reg [6:0] rx_bar_hit_16b_d ;
reg [1:0] cnt              ;
wire [c_BUF_DATA_WIDTH-1:0] write_data;
reg [c_BUF_ADDR_WIDTH-1:0] write_addr;
reg [c_BUF_ADDR_WIDTH-1:0] write_pointer;
reg [c_BUF_ADDR_WIDTH-1:0] read_addr;
reg  read_en;
reg  [c_BUF_DATA_WIDTH-1:0] read_data;  
reg  read_en_d1;
reg  read_en_d2;
reg  read_valid;
wire [c_BUF_DATA_WIDTH-1:0] Q;
reg  rx_dwen_16b_d;
reg  tlp_receiving;
reg  write_en;
always @(posedge clk_125 or negedge rstn)
   if (!rstn) begin
      rx_data_16b_d1    <= 0;
      rx_data_16b_d2    <= 0;
      rx_data_16b_d3    <= 0;
      rx_data_16b_d4    <= 0;
      rx_st_16b_d       <= 0;
      rx_end_16b_d      <= 0;
      rx_us_req_16b_d   <= 0;
      rx_malf_tlp_16b_d <= 0;
      rx_bar_hit_16b_d  <= 0;
      cnt               <= 0;
      rx_dwen_16b_d     <= 0;
      tlp_receiving     <= 0;
      write_en          <= 0;
   end
   else begin
      rx_data_16b_d1    <= rx_data_16b;
      rx_data_16b_d2    <= rx_data_16b_d1;
      rx_data_16b_d3    <= rx_data_16b_d2;
      rx_data_16b_d4    <= rx_data_16b_d3;
      
      if (rx_st_16b)
         cnt <= 0;
      else if (tlp_receiving)
         cnt <= cnt + 1;
      else
         cnt <= 0;

      if (rx_st_16b)
         rx_st_16b_d <= 1'b1;
      else if (cnt[1:0] == 3)
         rx_st_16b_d <= 1'b0;
      
      rx_end_16b_d      <= rx_end_16b;
      rx_us_req_16b_d   <= rx_us_req_16b;
      rx_malf_tlp_16b_d <= rx_malf_tlp_16b;
      rx_bar_hit_16b_d  <= rx_bar_hit_16b;
      rx_dwen_16b_d     <= rx_end_16b && (~ cnt[1]);
      write_en <= (tlp_receiving && (cnt[1:0] == 2)) || rx_end_16b;
      
      if (rx_st_16b)            
         tlp_receiving  <= 1'b1;      
      else if (rx_end_16b)          
         tlp_receiving  <= 1'b0;      
   end
                       
assign  write_data[63:0]  = {rx_data_16b_d4, rx_data_16b_d3, rx_data_16b_d2, rx_data_16b_d1};    
assign  write_data[64]    = rx_st_16b_d;      
assign  write_data[65]    = rx_end_16b_d;      
assign  write_data[66]    = rx_dwen_16b_d;    
assign  write_data[67]    = rx_us_req_16b_d;  
assign  write_data[68]    = rx_us_req_16b_d;
assign  write_data[75:69] = rx_bar_hit_16b_d;

always @(posedge clk_125 or negedge rstn)
   if (!rstn) begin
      write_addr <= 0;
      write_pointer <= 0;
   end
   else begin
      if (write_en)
         write_addr <= write_addr + 1;
      
      if (write_en && rx_end_16b_d)
         write_pointer <= write_addr + 1;
   end
//*****************************************************************************
pmi_ram_dp 
 #(.pmi_wr_addr_depth (1<<c_BUF_ADDR_WIDTH),
   .pmi_wr_addr_width (c_BUF_ADDR_WIDTH),
   .pmi_wr_data_width (c_BUF_DATA_WIDTH),
   .pmi_rd_addr_depth (1<<c_BUF_ADDR_WIDTH),
   .pmi_rd_addr_width (c_BUF_ADDR_WIDTH),
   .pmi_rd_data_width (c_BUF_DATA_WIDTH),
   .pmi_regmode       ("reg"),
   .pmi_gsr           ("disable"),
   .pmi_resetmode     ("sync"),
//   .pmi_optimization  ("speed"),
   .pmi_init_file     ("none"),
   .pmi_init_file_format ("binary"),
   .pmi_family ("ecp3"),
   .module_type ("pmi_ram_dp")
   )
   I_pmi_ram_dp(
    .Data             (write_data),
    .WrAddress        (write_addr),
    .RdAddress        (read_addr),
    .WrClock          (clk_125),
    .RdClock          (clk_125),
    .WrClockEn        (1'b1),
    .RdClockEn        (1'b1),
    .WE               (write_en),
    .Reset            (1'b0),
    .Q                (Q)
    );/*synthesis syn_black_box */
    
//*****************************************************************************
//read from buffer and send to downstream 
wire [c_BUF_ADDR_WIDTH-1:0] read_addr_inc = read_addr + 1;
always @(posedge clk_125 or negedge rstn)
   if (!rstn) begin
      read_addr  <= 0;
      read_en    <= 1'b0;
      read_data  <= 0;
      read_en_d1 <= 0;
      read_en_d2 <= 0;
      read_valid <= 0;
   end
   else begin
      if (read_en) begin
         if (read_addr_inc == write_pointer)
            read_en   <= 1'b0;
      end
      else begin
         if (read_addr != write_pointer)
            read_en   <= 1'b1;
      end
         
      if (read_en)
         read_addr <= read_addr + 1;
         
      read_en_d1 <= read_en; //EBR out
      read_en_d2 <= read_en_d1; //EBR reg out
      read_valid <= read_en_d2; //
      read_data  <= Q;
   end
   
always @(posedge clk_125 or negedge rstn)
   if (!rstn) begin
      rx_data_64b <= 0;
      rx_st_64b   <= 0;
      rx_end_64b  <= 0;
      rx_dwen_64b <= 0;
      rx_us_req_64b <= 0;
      rx_malf_tlp_64b <= 0;
      rx_bar_hit_64b <= 0;
   end
   else begin
      if (~ read_valid) begin
         rx_data_64b <= 0;
         rx_st_64b   <= 0;
         rx_end_64b  <= 0;
         rx_dwen_64b <= 0;
         rx_us_req_64b <= 0;
         rx_malf_tlp_64b <= 0;
         rx_bar_hit_64b <= 0;
      end
      else begin
         //rx_data_64b <= read_data[63:0];    
         rx_data_64b <= read_data[66] ? {read_data[31:0], read_data[63:32]} : read_data[63:0];    
         rx_st_64b   <= read_data[64];    
         rx_end_64b  <= read_data[65];
         rx_dwen_64b <= read_data[66];
         rx_us_req_64b <= read_data[67];  
         rx_malf_tlp_64b <= read_data[68];      
         rx_bar_hit_64b <= read_data[75:69];       
      end
   end
endmodule