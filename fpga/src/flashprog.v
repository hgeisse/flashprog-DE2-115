//
// flashprog.v -- Terasic DE2-115 Flash ROM programmer
//


`timescale 1ns/10ps
`default_nettype none


module flashprog(clk_in,
                 rst_in_n,
                 rs232_rxd,
                 rs232_txd,
                 fl_addr,
                 fl_dq,
                 fl_ce_n,
                 fl_oe_n,
                 fl_we_n,
                 fl_wp_n,
                 fl_rst_n,
                 fl_ry,
                 led_g,
                 led_r,
                 hex7_n,
                 hex6_n,
                 hex5_n,
                 hex4_n,
                 hex3_n,
                 hex2_n,
                 hex1_n,
                 hex0_n);
    // clock and reset
    input clk_in;
    input rst_in_n;
    // serial line
    input rs232_rxd;
    output rs232_txd;
    // flash memory
    output [22:0] fl_addr;
    inout [7:0] fl_dq;
    output fl_ce_n;
    output fl_oe_n;
    output fl_we_n;
    output fl_wp_n;
    output fl_rst_n;
    input fl_ry;
    // programming indicators
    output [8:0] led_g;
    output [17:0] led_r;
    output [6:0] hex7_n;
    output [6:0] hex6_n;
    output [6:0] hex5_n;
    output [6:0] hex4_n;
    output [6:0] hex3_n;
    output [6:0] hex2_n;
    output [6:0] hex1_n;
    output [6:0] hex0_n;

  // clk_rst
  wire clk;				// system clock
  wire rst;				// system reset
  // programming signals
  reg [22:0] addr;			// address to Flash
  reg [7:0] data;			// data to Flash
  reg [3:0] ctrl;			// control to Flash
  wire [7:0] dat;			// data from Flash
  wire rdy;				// ready from Flash
  // serial line
  wire cmd_rdy;
  wire [7:0] cmd_data;
  wire [7:0] snd_data;
  // programming automaton
  reg [3:0] state;
  reg [3:0] next_state;
  reg ack_data;
  reg ld_addr_0;
  reg ld_addr_1;
  reg ld_addr_2;
  reg ld_addr_3;
  reg ld_addr_4;
  reg ld_addr_5;
  reg ld_data_0;
  reg ld_data_1;
  reg ld_ctrl;
  reg snd_stb;
  reg snd_mux;
  // programming indicators
  wire [23:20] aux_addr;

  //--------------------------------------
  // module instances
  //--------------------------------------

  clk_rst clk_rst_1(
    .clk_in(clk_in),
    .rst_in_n(rst_in_n),
    .clk(clk),
    .rst(rst)
  );

  rcvbuf rcvbuf_1(
    .clk(clk),
    .reset(rst),
    .read(ack_data),
    .ready(cmd_rdy),
    .data_out(cmd_data[7:0]),
    .serial_in(rs232_rxd)
  );

  xmtbuf xmtbuf_1(
    .clk(clk),
    .reset(rst),
    .write(snd_stb),
    .ready(),
    .data_in(snd_data[7:0]),
    .serial_out(rs232_txd)
  );

  //--------------------------------------
  // flash memory connections
  //--------------------------------------

  assign fl_addr[22:0] = addr[22:0];
  assign fl_dq[7:0] = (~ctrl[3] & ~ctrl[2]) ? 8'hzz : data[7:0];
  assign fl_ce_n = ctrl[3];
  assign fl_oe_n = ctrl[2];
  assign fl_we_n = ctrl[1];
  assign fl_wp_n = 1'b1;
  assign fl_rst_n = ctrl[0];

  assign dat[7:0] = fl_dq[7:0];
  assign rdy = fl_ry;

  //--------------------------------------
  // programming automaton
  //--------------------------------------

  `define WT_CMD	4'd0
  `define RD_CMD	4'd1
  `define SET_ADDR_0	4'd2
  `define SET_ADDR_1	4'd3
  `define SET_ADDR_2	4'd4
  `define SET_ADDR_3	4'd5
  `define SET_ADDR_4	4'd6
  `define SET_ADDR_5	4'd7
  `define SET_DATA_0	4'd8
  `define SET_DATA_1	4'd9
  `define SET_CTRL	4'd10
  `define GET_DATA	4'd11
  `define GET_READY	4'd12

  always @(posedge clk) begin
    if (rst) begin
      state[3:0] <= `WT_CMD;
    end else begin
      state[3:0] <= next_state[3:0];
    end
  end

  always @(*) begin
    case (state[3:0])
      `WT_CMD:
        begin
          if (~cmd_rdy) begin
            next_state = `WT_CMD;
          end else begin
            next_state = `RD_CMD;
          end
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `RD_CMD:
        begin
          case (cmd_data[7:4])
            4'h0:  next_state = `SET_ADDR_0;	// set address nibble 0
            4'h1:  next_state = `SET_ADDR_1;	// set address nibble 1
            4'h2:  next_state = `SET_ADDR_2;	// set address nibble 2
            4'h3:  next_state = `SET_ADDR_3;	// set address nibble 3
            4'h4:  next_state = `SET_ADDR_4;	// set address nibble 4
            4'h5:  next_state = `SET_ADDR_5;	// set address nibble 5
            4'h6:  next_state = `SET_DATA_0;	// set data nibble 0
            4'h7:  next_state = `SET_DATA_1;	// set data nibble 1
            4'h8:  next_state = `SET_CTRL;	// set ctrl lines
            4'h9:  next_state = `GET_DATA;	// get data byte
            4'hA:  next_state = `GET_READY;	// get ready bit
            4'hB:  next_state = `WT_CMD;	// command ignored
            4'hC:  next_state = `WT_CMD;	// command ignored
            4'hD:  next_state = `WT_CMD;	// command ignored
            4'hE:  next_state = `WT_CMD;	// command ignored
            4'hF:  next_state = `WT_CMD;	// command ignored
          endcase
          ack_data   = 1'b1;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_0:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b1;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_1:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b1;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_2:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b1;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_3:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b1;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_4:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b1;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_ADDR_5:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b1;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_DATA_0:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b1;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_DATA_1:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b1;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `SET_CTRL:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b1;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
      `GET_DATA:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b1;
          snd_mux    = 1'b0;
        end
      `GET_READY:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b1;
          snd_mux    = 1'b1;
        end
      default:
        begin
          next_state = `WT_CMD;
          ack_data   = 1'b0;
          ld_addr_0  = 1'b0;
          ld_addr_1  = 1'b0;
          ld_addr_2  = 1'b0;
          ld_addr_3  = 1'b0;
          ld_addr_4  = 1'b0;
          ld_addr_5  = 1'b0;
          ld_data_0  = 1'b0;
          ld_data_1  = 1'b0;
          ld_ctrl    = 1'b0;
          snd_stb    = 1'b0;
          snd_mux    = 1'b0;
        end
    endcase
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[3:0] <= 4'h0;
    end else begin
      if (ld_addr_0) begin
        addr[3:0] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[7:4] <= 4'h0;
    end else begin
      if (ld_addr_1) begin
        addr[7:4] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[11:8] <= 4'h0;
    end else begin
      if (ld_addr_2) begin
        addr[11:8] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[15:12] <= 4'h0;
    end else begin
      if (ld_addr_3) begin
        addr[15:12] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[19:16] <= 4'h0;
    end else begin
      if (ld_addr_4) begin
        addr[19:16] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      addr[22:20] <= 3'h0;
    end else begin
      if (ld_addr_5) begin
        addr[22:20] <= cmd_data[2:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      data[3:0] <= 4'h0;
    end else begin
      if (ld_data_0) begin
        data[3:0] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
      data[7:4] <= 4'h0;
    end else begin
      if (ld_data_1) begin
        data[7:4] <= cmd_data[3:0];
      end
    end
  end

  always @(posedge clk) begin
    if (rst) begin
                  // ce_n  oe_n  we_n  rst_n
      ctrl[3:0] <= { 1'b1, 1'b1, 1'b1, 1'b1 };
    end else begin
      if (ld_ctrl) begin
        ctrl[3:0] <= cmd_data[3:0];
      end
    end
  end

  assign snd_data[7:0] =
    (snd_mux == 1'b0) ? dat[7:0] : { 7'b0, rdy };

  //--------------------------------------
  // programming indicators
  //--------------------------------------

  assign led_g[8:1] = 8'h00;
  assign led_g[0] = rdy;
  assign led_r[17:4] = 14'h0000;
  assign led_r[3:0] = ctrl[3:0];

  hexdrv hexdrv_0(
    .in(addr[3:0]),
    .out(hex0_n[6:0])
  );

  hexdrv hexdrv_1(
    .in(addr[7:4]),
    .out(hex1_n[6:0])
  );

  hexdrv hexdrv_2(
    .in(addr[11:8]),
    .out(hex2_n[6:0])
  );

  hexdrv hexdrv_3(
    .in(addr[15:12]),
    .out(hex3_n[6:0])
  );

  hexdrv hexdrv_4(
    .in(addr[19:16]),
    .out(hex4_n[6:0])
  );

  assign aux_addr[23:20] = { 1'b0, addr[22:20] };

  hexdrv hexdrv_5(
    .in(aux_addr[23:20]),
    .out(hex5_n[6:0])
  );

  hexdrv hexdrv_6(
    .in(data[3:0]),
    .out(hex6_n[6:0])
  );

  hexdrv hexdrv_7(
    .in(data[7:4]),
    .out(hex7_n[6:0])
  );

endmodule


module hexdrv(in, out);
    input [3:0] in;
    output reg [6:0] out;

  always @(*) begin
    case (in[3:0])
                        // 6543210
      4'h0: out[6:0] = ~7'b0111111;
      4'h1: out[6:0] = ~7'b0000110;
      4'h2: out[6:0] = ~7'b1011011;
      4'h3: out[6:0] = ~7'b1001111;
      4'h4: out[6:0] = ~7'b1100110;
      4'h5: out[6:0] = ~7'b1101101;
      4'h6: out[6:0] = ~7'b1111101;
      4'h7: out[6:0] = ~7'b0000111;
      4'h8: out[6:0] = ~7'b1111111;
      4'h9: out[6:0] = ~7'b1100111;
      4'hA: out[6:0] = ~7'b1110111;
      4'hB: out[6:0] = ~7'b1111100;
      4'hC: out[6:0] = ~7'b0111001;
      4'hD: out[6:0] = ~7'b1011110;
      4'hE: out[6:0] = ~7'b1111001;
      4'hF: out[6:0] = ~7'b1110001;
    endcase
  end

endmodule
