#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "kerndefs.h"
#include "bootstrap.h"
#include "multiboot2.h"
#include "bootinfo.h"
#include "pagealloc.h"
#include "mp.h"
#include "vmem.h"

SECTION(".entry_point")
int main(void *param, uint64_t magic)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        return -1;

    update_malloc_free_hooks(bootstrap_malloc, NULL); //Setup bootstrap malloc hooks

    bootinfo_parse_and_store(param, magic); //Parse multiboot2 bootinfo
    pagealloc_init(); //Initialize the page allocator
    //Need the GDT to setup the TLS //Initialize the GDT and IDT
    //mp_tls_setup(); //Setup MP TLS
    //vmem_init(); //Initialize paging
    //Initialize the PIC, PIT, APIC, IOAPIC, and RTC

    halt();

    return 0;
}
