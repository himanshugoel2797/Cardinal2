
CC=x86_64-elf-gcc
CFLAGS=-std=c11 -ffreestanding -O2 -Wall -Wextra -Werror -Wpedantic -fno-plt
LDFLAGS= -T linker.ld -ffreestanding -O2 -mno-red-zone -nostdlib -z max-page-size=0x1000 -mcmodel=large -fno-plt

INC_DIRS="-Iinc" "-Isrc"

ASM_SRCS=src/boot.o
OBJS=$(find src/ -name '*.c' -print | sed 's/\.c$/\.o/') $(ASM_SRCS)

.PHONY: all clean

all: $(OBJS)
	$(CC) -o ../boot/kernel.bin $(OBJS) $(LDFLAGS) -lgcc

%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f ../boot/kernel.bin
