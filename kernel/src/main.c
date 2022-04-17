#include "stddef.h"
#include "stdint.h"
#include "kerndefs.h"
#include "multiboot2.h"
#include "bootinfo.h"

SECTION(".entry_point")
int main(void *param, uint64_t magic)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        return -1;

    bootinfo_parse_and_store(param, magic);

    return 0;
}
