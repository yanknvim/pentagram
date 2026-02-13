build:
    veryl build

test: build
    iverilog -g2012 -o sim -f pentagram.f tests/tb_core.v
    ./sim
