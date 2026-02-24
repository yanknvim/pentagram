OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
    /* 命令用メモリ (Instruction Memory) */
    I_MEM (rx) : ORIGIN = 0x00000000, LENGTH = 1K
    /* データ用メモリ (Data Memory) */
    D_MEM (rw) : ORIGIN = 0x00000000, LENGTH = 1K
}

SECTIONS
{
    /* --- 命令セクション (I_MEMへ) --- */
    .text :
    {
        _stext = .;
        *(.text.init)  /* スタートアップコード等を最優先 */
        *(.text .text.*)
        _etext = .;
    } > I_MEM

    /* --- データセクション (D_MEMへ) --- */
    /* ここではAT(LMA)を指定せず、VMA=LMAとしてD_MEMに配置します */
    .data :
    {
        _sdata = .;
        *(.data .data.*)
        *(.sdata .sdata.*) /* RISC-V特有の小さいデータ用セクション */
        _edata = .;
    } > D_MEM

    .rodata :
    {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > D_MEM

    .bss :
    {
        _sbss = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        _ebss = .;
    } > D_MEM

    /* スタック領域の定義 (D_MEMの末尾など) */
    ._stack :
    {
        . = ALIGN(16);
        _stack_top = ORIGIN(D_MEM) + LENGTH(D_MEM);
    } > D_MEM
}
