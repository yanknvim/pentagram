# AGENTS.md — Pentagram (RISC-V CPU in Veryl)

## Project Overview

Pentagram is an RV32I RISC-V CPU implemented in **Veryl** (a modern HDL transpiling to SystemVerilog).
Single-cycle, Harvard architecture. Separate instruction/data memory, MMIO (LED `0xFE000000`, UART `0xFF000000`).

## Toolchain

- **Veryl** v0.18.0 — HDL compiler (`veryl build` → SystemVerilog in `target/`)
- **just** — Task runner (see `Justfile`)
- **iverilog** v12.0 — Verilog simulator
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

# Full test: build + simulate with iverilog
just test
# Manual test (after build):
iverilog -g2012 -o sim -f pentagram.f tests/tb_core.v && ./sim
```

### Test Programs (C → hex)

Tests in `tests/<name>/` each have a Makefile producing `inst.hex` + `data.hex`:
```bash
cd tests/blink && make      # or: cd tests/rule30 && make
```
Copy `inst.hex` to `tests/test.hex` to load into instruction memory for simulation.

## Project Structure

```
src/                    # Veryl source files
  utils.veryl           # Shared package: Opcode, InstType, ExecCtrl
  core.veryl            # CPU core: PC, pipeline wiring, writeback mux
  decoder.veryl         # Instruction decoder: opcode → immediate + control
  alu.veryl             # ALU: arithmetic/logic operations
  branch.veryl          # Branch comparator
  regfile.veryl         # 32-register file
  inst_mem.veryl        # Instruction memory (loads from hex)
  data_mem.veryl        # Data memory + MMIO
  uart.veryl            # UART transmitter (commented out in data_mem)
  top.veryl             # Top-level wrapper
target/                 # Generated .sv files (git-ignored)
tests/                  # Test programs and testbench
Veryl.toml              # Project config (reset_type = "async_high")
Justfile                # Build commands
pentagram.f             # Verilog file list for iverilog
```

## Code Style Guidelines

### Naming Conventions

| Element       | Convention       | Examples                              |
|---------------|------------------|---------------------------------------|
| Modules       | PascalCase       | `Core`, `DataMem`, `UART`             |
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
module DataMem (
    clk   : input  clock    ,
    rst   : input  reset    ,
    addr  : input  logic<32>,
    data  : output logic<32>,
) {
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

- `Veryl.toml` uses deprecated `source` field (should be `sources`)
- `veryl check` reports 2 `missing_reset_statement` warnings in `uart.veryl`
- UART is commented out in `data_mem.veryl` (LED MMIO active instead)
- Testbench `tests/tb_core.v` referenced in Justfile does not exist in repo
