
ARMGNU = arm-none-eabi

AOPS = --warn --fatal-warnings -mcpu=cortex-m4
COPS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding  -mcpu=cortex-m4

all : prog01.gcc.thumb.flash.bin prog01.gcc.thumb2.flash.bin

clean:
	rm -f *.bin
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f *.bc
	rm -f *.opt.s
	rm -f *.norm.s

flash.o : flash.s
	$(ARMGNU)-as $(AOPS) flash.s -o flash.o

prog01.gcc.thumb.o : prog01.c
	$(ARMGNU)-gcc $(COPS) -mthumb -c prog01.c -o prog01.gcc.thumb.o

prog01.gcc.thumb2.flash.o : prog01.c
	#$(ARMGNU)-gcc $(COPS) -mthumb -mcpu=cortex-m4 -march=armv7-m -c prog01.c -o prog01.gcc.thumb2.flash.o
	$(ARMGNU)-gcc $(COPS) -mthumb -mcpu=cortex-m4 -c prog01.c -o prog01.gcc.thumb2.flash.o

prog01.gcc.thumb.flash.bin : flash.ld flash.o prog01.gcc.thumb.o
	$(ARMGNU)-ld -o prog01.gcc.thumb.flash.elf -T flash.ld flash.o prog01.gcc.thumb.o
	$(ARMGNU)-objdump -D prog01.gcc.thumb.flash.elf > prog01.gcc.thumb.flash.list
	$(ARMGNU)-objcopy prog01.gcc.thumb.flash.elf prog01.gcc.thumb.flash.bin -O binary

prog01.gcc.thumb2.flash.bin : flash.ld flash.o prog01.gcc.thumb2.flash.o
	$(ARMGNU)-ld -o prog01.gcc.thumb2.flash.elf -T flash.ld flash.o prog01.gcc.thumb2.flash.o
	$(ARMGNU)-objdump -D prog01.gcc.thumb2.flash.elf > prog01.gcc.thumb2.flash.list
	$(ARMGNU)-objcopy prog01.gcc.thumb2.flash.elf prog01.gcc.thumb2.flash.bin -O binary

