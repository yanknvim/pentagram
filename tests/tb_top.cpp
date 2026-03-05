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
    // --no-vcd オプション
    bool enable_vcd = true;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--vcd=", 0) == 0) {
            vcd_file = arg.substr(6);
        }
        if (arg == "--no-vcd") {
            enable_vcd = false;
        }
    }

    // DUT のインスタンス化
    Vpentagram_Top* dut = new Vpentagram_Top;

    // VCD トレースのセットアップ
    VerilatedVcdC* vcd = nullptr;
    if (enable_vcd) {
        vcd = new VerilatedVcdC;
        dut->trace(vcd, 99);  // トレース深さ
        vcd->open(vcd_file.c_str());
    }

    // リセットシーケンス (async high reset)
    dut->rst = 1;
    dut->clk = 0;

    // リセットを 5 サイクル保持
    for (int i = 0; i < 10; i++) {
        dut->clk = !dut->clk;
        dut->eval();
        if (vcd) vcd->dump(sim_time);
        sim_time++;
    }

    // リセット解除
    dut->rst = 0;

    // メインシミュレーションループ
    bool finished = false;
    vluint64_t cycle;
    for (cycle = 0; cycle < max_cycles; cycle++) {
        // 立ち上がりエッジ
        dut->clk = 1;
        dut->eval();
        if (vcd) vcd->dump(sim_time);
        sim_time++;

        // 立ち下がりエッジ
        dut->clk = 0;
        dut->eval();
        if (vcd) vcd->dump(sim_time);
        sim_time++;

        if (Verilated::gotFinish()) {
            finished = true;
            break;
        }
    }

    // 後処理
    if (vcd) {
        vcd->close();
        delete vcd;
    }
    dut->final();
    delete dut;

    if (finished) {
        // $finish が呼ばれた = tohost に書き込みがあった
        // TOHOST: PASS / TOHOST: FAIL は $display で既に出力済み
        printf("Simulation finished at cycle %lu\n", (unsigned long)cycle);
    } else {
        // サイクル上限に到達 = タイムアウト
        printf("TOHOST: TIMEOUT after %lu cycles\n", (unsigned long)max_cycles);
    }

    // 終了コード: finished なら 0 (テストランナーが stdout から PASS/FAIL を判定)
    //             タイムアウトなら 1
    return finished ? 0 : 1;
}
