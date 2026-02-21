.section .text.init
.global _start

_start:
    # 1. スタックポインタ(sp)をメモリの末尾にセット
    # (リンカスクリプトで定義した _stack_top を使用)
    la sp, _stack_top

    # 2. 直接 main 関数へジャンプ
    call main

    # 3. 万が一 main から戻ってきたら無限ループ
loop:
    j loop
