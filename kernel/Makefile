
CC=x86_64-elf-gcc
CFLAGS=-std=c11 -ffreestanding -O2 -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter -fno-plt -mcmodel=large
LDFLAGS= -T linker.ld -ffreestanding -O2 -mno-red-zone -nostdlib -z max-page-size=0x1000 -mcmodel=large -fno-plt

INC_DIRS="-Iinc" "-Isrc"

ASM_SRCS=src/boot.o
OBJS=$(ASM_SRCS) src/bootinfo.o src/memory.o src/bootstrap_alloc.o src/main.o

.PHONY: all clean

all: $(OBJS)
	$(CC) -o ../boot/kernel.bin $(LDFLAGS) $(OBJS) -lgcc

%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f ../boot/kernel.bin
