.PHONY: all kernel iso run clean

all: kernel

kernel:
	make -C kernel all

iso: kernel
	rm -rf iso
	mkdir -p iso/boot/grub
	cp -r boot/* iso/boot
	grub-mkrescue -o os.iso iso

run: iso
	qemu-system-x86_64 --enable-kvm -m 2G -machine q35, -cpu host -d int,cpu_reset,guest_errors -cdrom os.iso -boot d

clean:
	make -C kernel clean
	@rm os.iso
	@rm -rf iso
