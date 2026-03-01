# Verilator testbench Makefile for pentagram Top module
#
# Usage:
#   make                    # ビルドのみ
#   make sim FILEPATH=path/to/test.hex  # ビルド + シミュレーション実行
#   make sim FILEPATH=test.hex CYCLES=50000
#   make clean
#
# VCD 出力: sim.vcd (--vcd= オプションで変更可)

PROJ_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
TARGET_DIR := $(PROJ_ROOT)/target
TB_SRC := $(PROJ_ROOT)/tests/tb_top.cpp
SIM_BIN := $(PROJ_ROOT)/tests/sim

# SystemVerilog ソースファイル (pentagram.f に記載されたもの)
SV_SRCS := \
	$(TARGET_DIR)/utils.sv \
	$(TARGET_DIR)/membus.sv \
	$(TARGET_DIR)/uart.sv \
	$(TARGET_DIR)/regfile.sv \
	$(TARGET_DIR)/memunit.sv \
	$(TARGET_DIR)/memory.sv \
	$(TARGET_DIR)/decoder.sv \
	$(TARGET_DIR)/branch.sv \
	$(TARGET_DIR)/alu.sv \
	$(TARGET_DIR)/core.sv \
	$(TARGET_DIR)/top.sv

# Verilator 設定
VERILATOR := verilator
VERILATOR_FLAGS := \
	--cc \
	--exe \
	--build \
	--trace \
	-Wall \
	-Wno-UNUSEDSIGNAL \
	-Wno-UNUSEDPARAM \
	-Wno-UNDRIVEN \
	-Wno-VARHIDDEN \
	-Wno-DECLFILENAME \
	--top-module pentagram_Top \
	--Mdir $(PROJ_ROOT)/tests/obj_dir \
	-o $(abspath $(SIM_BIN))

# シミュレーション引数
FILEPATH ?=
CYCLES ?= 10000
VCD ?= sim.vcd

SIM_ARGS :=
ifdef FILEPATH
	SIM_ARGS += +FILEPATH=$(FILEPATH)
endif
SIM_ARGS += --cycles=$(CYCLES) --vcd=$(VCD)

.PHONY: all build sim clean

all: build

build: $(SIM_BIN)

$(SIM_BIN): $(SV_SRCS) $(TB_SRC)
	$(VERILATOR) $(VERILATOR_FLAGS) $(SV_SRCS) $(TB_SRC)

sim: $(SIM_BIN)
	@if [ -z "$(FILEPATH)" ]; then \
		echo "Error: FILEPATH is required. Usage: make sim FILEPATH=path/to/test.hex"; \
		exit 1; \
	fi
	$(SIM_BIN) $(SIM_ARGS)

clean:
	rm -rf $(PROJ_ROOT)/tests/obj_dir $(SIM_BIN) $(VCD)
