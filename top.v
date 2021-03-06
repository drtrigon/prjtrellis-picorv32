module top(
    input clk_25mhz,
    input [6:0] btn,
    output [7:0] led,
    output ftdi_rxd,
    input ftdi_txd,
    output wifi_gpio0
);

assign wifi_gpio0 = btn[0]; // hold btn0 -> escape to ESP32/OLED control

attosoc soc(
    .clk(clk_25mhz),
    .reset_n(btn[0]),
    .led(led),
    .uart_tx(ftdi_rxd),
    .uart_rx(ftdi_txd)
);

endmodule
