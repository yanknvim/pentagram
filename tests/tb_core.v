`timescale 1ns/1ps

module core_tb;
    reg clk = 1'b0;
    reg rst = 1'b0;

    pentagram_Core DUT (
        .clk(clk),
        .rst(rst)
    );

    initial begin
        $dumpfile("test.vcd");
        $dumpvars(0, DUT);
    end

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        rst = 1'b0;
        #10;
        rst = 1'b1;

        #100;
        $finish;
    end
endmodule
