#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vpentagram_Top.h"

#include <cstdio>
#include <cstdlib>
#include <string>

static vluint64_t sim_time = 0;

int main(int argc, char** argv) {
    // Verilator の初期化 — plusargs (+FILEPATH=xxx) もここで渡される
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    // デフォルトのシミュレーションサイクル数
    vluint64_t max_cycles = 10000;

    // --cycles=N オプションのパース
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--cycles=", 0) == 0) {
            max_cycles = std::stoull(arg.substr(9));
        }
    }

    // VCD ファイル名 (デフォルト: sim.vcd)
    std::string vcd_file = "sim.vcd";
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--vcd=", 0) == 0) {
            vcd_file = arg.substr(6);
        }
    }

    // DUT のインスタンス化
    Vpentagram_Top* dut = new Vpentagram_Top;

    // VCD トレースのセットアップ
    VerilatedVcdC* vcd = new VerilatedVcdC;
    dut->trace(vcd, 99);  // トレース深さ
    vcd->open(vcd_file.c_str());

    // リセットシーケンス (async high reset)
    dut->rst = 1;
    dut->clk = 0;

    // リセットを 5 サイクル保持
    for (int i = 0; i < 10; i++) {
        dut->clk = !dut->clk;
        dut->eval();
        vcd->dump(sim_time++);
    }

    // リセット解除
    dut->rst = 0;

    // メインシミュレーションループ
    for (vluint64_t cycle = 0; cycle < max_cycles; cycle++) {
        // 立ち上がりエッジ
        dut->clk = 1;
        dut->eval();
        vcd->dump(sim_time++);

        // 立ち下がりエッジ
        dut->clk = 0;
        dut->eval();
        vcd->dump(sim_time++);

        if (Verilated::gotFinish()) {
            break;
        }
    }

    // 後処理
    vcd->close();
    dut->final();
    delete dut;
    delete vcd;

    printf("Simulation finished: %lu cycles, VCD -> %s\n",
           (unsigned long)max_cycles, vcd_file.c_str());

    return 0;
}
