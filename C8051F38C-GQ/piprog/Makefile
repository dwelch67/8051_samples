
ARMGNU ?= arm-none-eabi

COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 

all : piprog.bin

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img
	rm -f *.bc
	rm -f *.clang.s
	rm -f hextoc

hextoc : hextoc.c
	gcc -O2 hextoc.c -o hextoc

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

rom.h : blinker.s hextoc
	as31 blinker.s
	./hextoc blinker.s.hex

piprog.o : piprog.c rom.h
	$(ARMGNU)-gcc $(COPS) -c piprog.c -o piprog.o

periph.o : periph.c 
	$(ARMGNU)-gcc $(COPS) -c periph.c -o periph.o

piprog.bin : loader vectors.o periph.o piprog.o 
	$(ARMGNU)-ld vectors.o periph.o piprog.o -T loader -o piprog.elf
	$(ARMGNU)-objdump -D piprog.elf > piprog.list
	$(ARMGNU)-objcopy piprog.elf -O ihex piprog.hex
	$(ARMGNU)-objcopy piprog.elf -O binary piprog.bin


