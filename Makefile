SRCS = $(shell find -name '*.[cS]') vga.o
OBJS = $(addsuffix .o,$(basename $(SRCS)))

CC = gcc
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -nostdinc
LDFLAGS = -melf_i386 -Ttext=0x100000

all: iso

run: iso
	qemu-system-i386 -cdrom test.iso

iso: kernel grub.cfg
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub
	cp kernel isodir/boot/kernel.bin
	grub-mkrescue -o test.iso isodir

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

%.o: %.asm
	nasm -f elf32 -o $@ $^

clean:
	rm $(OBJS)

.PHONY: clean
