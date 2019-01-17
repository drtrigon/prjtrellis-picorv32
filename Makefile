# 1. source:   fpgarduino
#              https://github.com/f32c/arduino/issues/32
#              https://github.com/emard/prjtrellis-picorv32
# 2. makefile: https://github.com/cliffordwolf/icestorm/tree/master/examples/icezum
#              https://github.com/cliffordwolf/picorv32/blob/master/picosoc/Makefile
#              firmware: https://github.com/emard/prjtrellis-picorv32/tree/master/makefile
# 3. pins:     https://github.com/cliffordwolf/picorv32/issues/92
#              https://github.com/FPGAwars/Alhambra-II-FPGA/blob/master/examples/picorv32/picosoc/demo.pcf
# 4. chipset:  https://github.com/FPGAwars/Alhambra-II-FPGA
#              http://www.clifford.at/icestorm/
#              iCE40-HX4K-TQ144; -d 8k -P tq144:4k, -d hx8k

PROJ = top
PIN_DEF = demo.pcf
DEVICE = hx8k

ICESTORM = $(HOME)/.icestudio/apio/packages/toolchain-icestorm/bin
TOP_MODULE = $(PROJ)
TOP_MODULE_FILE = $(TOP_MODULE).v
VERILOG_FILES = $(TOP_MODULE_FILE) attosoc.v picorv32.v simpleuart.v

#F32C-COMPILER-PATH=~davor/.arduino15/packages/FPGArduino/tools/f32c-compiler/1.0.0/bin
F32C-COMPILER-PATH=../toolchain-fpgarduino/f32c/bin
RISCV32-GCC=$(F32C-COMPILER-PATH)/riscv32-elf-gcc
RISCV32-OBJCOPY=$(F32C-COMPILER-PATH)/riscv32-elf-objcopy

all: $(PROJ).rpt $(PROJ).bin

%.blif: %.v firmware.hex
	$(ICESTORM)/yosys -p 'synth_ice40 -top top -blif $@' $(VERILOG_FILES)

%.asc: $(PIN_DEF) %.blif
#	$(ICESTORM)/arachne-pnr -d $(subst hx,,$(subst lp,,$(DEVICE))) -o $@ -p $^
	$(ICESTORM)/arachne-pnr -d 8k -P tq144:4k -o $@ -p $^

%.bin: %.asc
	icepack $< $@

%.rpt: %.asc
	icetime -d $(DEVICE) -mtr $@ $<

prog: $(PROJ).bin
	iceprog $<

sudo-prog: $(PROJ).bin
	@echo 'Executing prog as root!!!'
	sudo iceprog $<

firmware.elf: sections.lds start.s firmware.c
	$(RISCV32-GCC) -march=rv32i -mabi=ilp32 -Wl,-Bstatic,-T,sections.lds,--strip-debug -ffreestanding -nostdlib -o firmware.elf start.s firmware.c

firmware.bin: firmware.elf
	$(RISCV32-OBJCOPY) -O binary firmware.elf /dev/stdout > firmware.bin

firmware.hex: firmware.bin
	python3 makehex.py $^ 4096 > $@

clean:
	rm -f $(PROJ).blif $(PROJ).asc $(PROJ).rpt $(PROJ).bin firmware.elf firmware.bin firmware.hex

.SECONDARY:
.PHONY: all prog clean
