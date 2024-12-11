MINOS_PATH  ?=../../
INPUTS_PATH ?=../inputs/
DAY:=day3
EXE:=build/$(DAY)

CC:=gcc
CFLAGS:=-g -static -nostdlib -march=x86-64 -ffreestanding -Werror -Wno-unused-function -Wall -fno-stack-protector -mno-3dnow -fPIC -I../../user/libc/include/ -I../../libs/std/include/
crt:=$(wildcard $(MINOS_PATH)bin/user/crt/*.o)
libc:=$(wildcard $(MINOS_PATH)bin/user/libc/*.o)
std:=$(wildcard $(MINOS_PATH)bin/std/*.o)

all: $(EXE) copy

run: $(EXE) copy
	cd $(MINOS_PATH) && ./build bruh -f initrd iso

copy: $(EXE) $(INPUTS_PATH)$(DAY)_input.txt $(INPUTS_PATH)$(DAY)_input_test.txt
	cp $< $(MINOS_PATH)initrd/user/
	mkdir -p $(MINOS_PATH)initrd/inputs/
	cp $(INPUTS_PATH)$(DAY)_input.txt      $(MINOS_PATH)initrd/inputs/$(DAY)_input.txt
	cp $(INPUTS_PATH)$(DAY)_input_test.txt $(MINOS_PATH)initrd/inputs/$(DAY)_input_test.txt
$(EXE): main.c
	mkdir -p build/
	$(CC) $(CFLAGS) $(crt) $(libc) $(std) $< -o $@
