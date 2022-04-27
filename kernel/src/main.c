#include "stddef.h"
#include "stdint.h"
#include "kerndefs.h"
#include "multiboot2.h"
#include "bootinfo.h"
#include "pagealloc.h"

SECTION(".entry_point")
int main(void *param, uint64_t magic)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        return -1;

    bootinfo_parse_and_store(param, magic);

    // Initialize the page allocator
    pagealloc_init();
    // Initialize paging
    // Initialize the GDT and IDT
    // Initialize the PIC, PIT, APIC, IOAPIC, and RTC

    halt();

    return 0;
}
