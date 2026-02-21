# --- 設定 ---
CROSS_COMPILE = riscv32-unknown-elf-
CC      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# RV32I向けフラグ
# -nostartfiles: 標準スタートアップを使用しない
# -ffreestanding: 標準ライブラリを前提としない
# -fno-common: 重複シンボルを許さない（bss配置のため）
CFLAGS  = -march=rv32i -mabi=ilp32 -O2 -nostartfiles -ffreestanding -fno-common
LDFLAGS = -T link.ld

SRCS = crt0.S main.c
TARGET = program.elf

# --- ビルドターゲット ---
all: inst.hex data.hex dump

# 1. ELFのビルド
$(TARGET): $(SRCS) link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(TARGET)


# 1. 命令メモリ用 HEX (32bitごとに1行、アドレス指定なし)
inst.hex: $(TARGET)
	$(OBJCOPY) -O binary --only-section=.text $(TARGET) inst.bin
	hexdump -v -e '1/4 "%08x\n"' inst.bin > inst.hex

# 2. データメモリ用 HEX (32bitごとに1行、アドレス指定なし)
data.hex: $(TARGET)
	$(OBJCOPY) -O binary --remove-section=.text $(TARGET) data.bin
	hexdump -v -e '1/4 "%08x\n"' data.bin > data.hex

# 4. デバッグ用逆アセンブル
dump: $(TARGET)
	$(OBJDUMP) -D $(TARGET) > $(TARGET).dis

clean:
	rm -f *.elf *.hex *.dis
