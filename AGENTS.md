# AGENTS.md — Pentagram (RISC-V CPU in Veryl)

## Agent Rules

- **言語**: ユーザーへの応答はすべて **日本語** で行うこと。コード・コマンド・技術用語はそのままで可。
- **Git 操作の禁止**: `git commit`、`git push`、`git checkout`、`git rebase` 等の git 操作を自動的に実行してはならない。git 操作が必要な場合はユーザーに確認し、明示的な許可を得てから実行すること。

## Project Overview

Pentagram is an RV32I RISC-V CPU implemented in **Veryl** (a modern HDL transpiling to SystemVerilog).
Multi-cycle architecture with a unified memory bus (`MemBus` interface). The CPU uses a Fetch/Data state machine in `Core` to sequence instruction fetch and data (load/store) operations over the shared bus. Memory is accessed through `MemUnit` which handles byte/halfword alignment and bus handshaking.

## Toolchain

- **Veryl** v0.18.0 — HDL compiler (`veryl build` → SystemVerilog in `target/`)
- **just** — Task runner (see `Justfile`)
- **Verilator** v5.044 — Primary simulator (C++ testbench)
- **iverilog** v12.0 — Verilog simulator (legacy, referenced in Justfile but testbench does not exist)
- **riscv32-unknown-elf-gcc** — Cross-compiler for test programs

## Build / Check / Test Commands

```bash
just build                  # transpile all Veryl → SystemVerilog (or: veryl build)
veryl check                 # static analysis (type errors, missing resets, lint)
veryl fmt                   # auto-format all .veryl files
veryl fmt --check           # dry-run format check (exit non-zero if diff)

# Single-file operations
veryl build src/alu.veryl
veryl check src/alu.veryl
veryl fmt src/alu.veryl

# Verilator simulation (primary method):
cd tests && make                                    # build testbench binary
cd tests && make sim FILEPATH=path/to/test.hex      # build + run simulation
cd tests && make sim FILEPATH=test.hex CYCLES=50000 # custom cycle count
cd tests && make clean                              # clean build artifacts
```

### Test Programs (C → hex)

Tests in `tests/<name>/` each have a Makefile producing `inst.hex` + `data.hex`:
```bash
cd tests/blink && make      # or: cd tests/rule30 && make
```
The hex file path is passed to the simulator via `+FILEPATH=` plusarg.

## Project Structure

```
src/                    # Veryl source files
  utils.veryl           # Shared package: Opcode, InstType, ExecCtrl
  core.veryl            # CPU core: PC, Fetch/Data state machine, pipeline wiring, writeback mux
  decoder.veryl         # Instruction decoder: opcode → immediate + control
  alu.veryl             # ALU: arithmetic/logic operations
  branch.veryl          # Branch comparator
  regfile.veryl         # 32-register file
  membus.veryl          # Memory bus interface (MemBus) with master/slave modports
  memunit.veryl         # Memory access unit: byte/halfword alignment, bus handshaking
  memory.veryl          # Unified memory (1024 words, loads from hex via $readmemh)
  uart.veryl            # UART transmitter (standalone module, not connected)
  top.veryl             # Top-level wrapper: Core + Memory connected via MemBus
target/                 # Generated .sv files (git-ignored)
tests/                  # Test programs and testbench
  tb_top.cpp            # Verilator C++ testbench (VCD trace, configurable cycles)
  Makefile              # Verilator build & simulation rules
  blink/                # LED blink test program
  rule30/               # Rule 30 cellular automaton test program
dependencies/           # Veryl standard library (auto-managed)
Veryl.toml              # Project config (reset_type = "async_high")
Justfile                # Build commands (veryl build; iverilog test is stale)
pentagram.f             # SystemVerilog file list for simulation
```

## Code Style Guidelines

### Naming Conventions

| Element       | Convention       | Examples                              |
|---------------|------------------|---------------------------------------|
| Modules       | PascalCase       | `Core`, `Memory`, `UART`, `MemUnit`   |
| Interfaces    | PascalCase       | `MemBus`                              |
| Packages      | snake_case       | `utils`                               |
| Enums/Structs | PascalCase       | `Opcode`, `InstType`, `ExecCtrl`      |
| Enum variants | PascalCase/UPPER | `State::Idle`, `Opcode::LUI`          |
| Struct fields | snake_case       | `is_load`, `is_jump`, `itype`         |
| Signals/Ports | snake_case       | `rs1_data`, `alu_data`, `clk`, `rst`  |
| Constants     | UPPER_SNAKE_CASE | `UART_ADDR`, `BAUD_DIVIDER`           |
| Parameters    | UPPER_SNAKE_CASE | `CLK_FREQ`, `BAUD_RATE`               |

### Imports

Every module file starts with `import utils::*;` — wildcard import of the shared package.

### Module Declaration

Aligned colons, direction, type. Trailing comma on last port. `clock`/`reset` are built-in types:
```veryl
module Memory (
    clk: input   clock        ,
    rst: input   reset        ,
    bus: modport MemBus::slave,
) {
```

### Interface Declaration

```veryl
interface MemBus {
    var valid : logic    ;
    var ready : logic    ;
    // ...
    modport master { valid: output, ready: input, ... }
    modport slave  { ..converse(master) }
}
```

### Module Instantiation

Named connections, aligned colons. Shorthand when port name = signal name:
```veryl
inst regfile: Regfile (
    clk                  ,    // shorthand
    wen     : ctrl.is_reg,    // explicit mapping
);
```

### Variables: `var` vs `let`

- `var` = mutable register/signal: `var pc: logic<32>;`
- `let` = combinational wire: `let rd: logic<5> = instruction[11:7];`
- Column-align colons and types

### Combinational Logic (`always_comb`)

Use `case` for value matching, `switch` for boolean guards:
```veryl
always_comb {
    data = case ctrl.itype {
        InstType::I, InstType::R: case funct3 {
            3'b000: a + b,
            default: 'x,           // don't-care
        },
        default: a + b,
    };
}
```
Ternary: `if condition ? then_val : else_val`

### Sequential Logic (`always_ff`)

All registers MUST have reset values in `if_reset` — `veryl check` enforces this:
```veryl
always_ff {
    if_reset {
        pc = 32'b0;
    } else {
        pc = next_pc;
    }
}
```

### Constants and Literals

```veryl
const UART_ADDR: u32 = 32'hFF000000;
3'b000          // 3-bit binary
32'b0           // 32-bit zero
1'b0 repeat 24  // bit repeat
```

### Enums and Structs (defined in `utils` package)

```veryl
package utils {
    enum Opcode: logic<7> { LUI = 7'b0110111, AUIPC = 7'b0010111 }
    enum InstType { R, I, S, B, U, J }          // auto-encoded
    struct ExecCtrl { itype: InstType, is_lui: logic }
}
```

### Formatting (enforced by `veryl fmt`)

- 4-space indentation
- Aligned colons in port lists, struct fields, variable declarations
- Spaces before trailing commas in port/instantiation lists
- Run `veryl fmt` before committing

## Known Issues

- Justfile の `test` ターゲットは存在しない `tests/tb_core.v` (iverilog) を参照している。実際のテストは `tests/Makefile` (Verilator) を使用すること
- UART モジュールは存在するが、どのモジュールにも接続されていない（スタンドアロン）
